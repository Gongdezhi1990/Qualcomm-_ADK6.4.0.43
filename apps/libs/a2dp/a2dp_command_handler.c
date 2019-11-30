/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_command_handler.c

DESCRIPTION

NOTES

*/



/****************************************************************************
    Header files
*/

#include "a2dp_command_handler.h"
#include "a2dp_data_block.h"
#include "a2dp_packet_handler.h"
#include "a2dp_process_command.h"
#include "a2dp_caps_parse.h"
#include "a2dp_api.h"
#include "a2dp_l2cap_handler.h"
#include "a2dp_codec_handler.h"

#include <print.h>
#include <stdlib.h>
#include <string.h>
#include <sink.h>


static uint8 getReceivedMessageType (remote_device *device)
{
    return *device->signal_conn.connection.active.received_packet & 0x03;
}

/*****************************************************************************/
static bool sendSetConfiguration(remote_device *device)
{
    uint8 *configured_service_caps = (uint8 *)PanicNull( blockGetBase( (uint8)device->bitfields.device_id, data_block_configured_service_caps ) );
    uint16 configured_service_caps_size = blockGetSize( (uint8)device->bitfields.device_id, data_block_configured_service_caps );

    return a2dpSendCommand(device, avdtp_set_configuration, configured_service_caps, configured_service_caps_size);
}

/*****************************************************************************/
static bool sendDelayReport (remote_device *device, uint16 delay)
{
    uint8 delay_report[2];

    /* Update locally cached copy of current delay before sending */
    device->media_conn[0].status.delay = delay;
    
    delay_report[1] = (uint8)delay;
    delay_report[0] = (uint8)(delay >> 8);

    return a2dpSendCommand(device, avdtp_delayreport, delay_report, 2);
}

/*****************************************************************************/
static void setDelayReportPending (remote_device *device, bool state)
{
    device->media_conn[0].status.conn_info.delay_pending = state;
}

/*****************************************************************************/
static bool isDelayReportPending (remote_device *device)
{
    return device->media_conn[0].status.conn_info.delay_pending;
}

/****************************************************************************/
void a2dpSetStreamState(remote_device *device, avdtp_stream_state state)
{
    PRINT(("a2dpSetStreamState %d: O:%d N:%d\n", device->bitfields.device_id, device->signal_conn.status.stream_state, state));

    device->signal_conn.status.stream_state = state;

    /* Cancel the watchdog timeouts */
    (void) MessageCancelAll((Task)&a2dp->task, A2DP_INTERNAL_CLIENT_RSP_TIMEOUT_BASE + device->bitfields.device_id);
    (void) MessageCancelAll((Task)&a2dp->task, A2DP_INTERNAL_REMOTE_CMD_TIMEOUT_BASE + device->bitfields.device_id);
    (void) MessageCancelAll((Task)&a2dp->task, A2DP_INTERNAL_WATCHDOG_BASE + device->bitfields.device_id);

    /* do we need to restart the watchdog? */
    switch (state)
    {
        case avdtp_stream_idle:
        case avdtp_stream_configured:
        case avdtp_stream_open:
        case avdtp_stream_streaming:
            /* These states are stable and therefore
               do not require the watchdog. */
            break;

        /*
           From Generic Audio/Video Distribution Profile, Table 4-1
           Some signals use TGAVDP100 and some don't.  For those
           that don't we apply our own timeout to prevent lock-up.
        */
        case avdtp_stream_configuring:
        case avdtp_stream_local_opening:
        case avdtp_stream_local_starting:
        case avdtp_stream_remote_starting:
        case avdtp_stream_local_suspending:
        case avdtp_stream_local_closing:
        case avdtp_stream_remote_closing:
        case avdtp_stream_reconfiguring:
        case avdtp_stream_local_aborting:
        case avdtp_stream_remote_aborting:
            MessageSendLater((Task)&a2dp->task, A2DP_INTERNAL_WATCHDOG_BASE + device->bitfields.device_id, 0, WATCHDOG_TGAVDP100);
            break;

        case avdtp_stream_discovering:
        case avdtp_stream_reading_caps:
        case avdtp_stream_processing_caps:
        case avdtp_stream_reconfig_reading_caps:
        case avdtp_stream_remote_opening:
            MessageSendLater((Task)&a2dp->task, A2DP_INTERNAL_WATCHDOG_BASE + device->bitfields.device_id, 0, WATCHDOG_GENERAL);
            break;
    }
}


/****************************************************************************/
void a2dpHandleInternalWatchdogTimeout(MessageId msg_id)
{
    PanicFalse((uint16)(msg_id-A2DP_INTERNAL_WATCHDOG_BASE) < A2DP_MAX_REMOTE_DEVICES);

    {    
        remote_device *device = &a2dp->remote_conn[(uint16)(msg_id-A2DP_INTERNAL_WATCHDOG_BASE)];

        if ((device->signal_conn.status.stream_state == avdtp_stream_local_aborting) ||
            (device->signal_conn.status.stream_state == avdtp_stream_remote_aborting))
        {   /* The watchdog fired while we were aborting, kill. */
            a2dpStreamReset(device);
            
            if ( !a2dpDisconnectAllMedia(device) )
            {   /* No media channels to disconnect, check if signalling needs closing */
                a2dpSignallingDisconnectPending(device);
            }
        }
        else
        {   /* Abort */
            a2dpStreamAbort(device);
        }
    }
}

static const sep_data_type * findLocalCodecInfo (remote_device *device, uint8 seid)
{
    sep_data_type *sep_list = (sep_data_type *)PanicNull( blockGetBase((uint8)device->bitfields.device_id, data_block_sep_list) );
    unsigned sep_list_size = blockGetSize((uint8)device->bitfields.device_id, data_block_sep_list) / sizeof(sep_data_type);
    
    while (sep_list_size--)
    {
        const sep_config_type *sep_config = sep_list->sep_config;
        
        if (sep_config->seid == seid)
        {
            return sep_list;
        }
        
        sep_list++;
    }
    
    return NULL;
}

#if 0   /* TODO: Not currently used */
static uint8 findLocalCodecType (remote_device *device, uint8 seid)
{
    sep_data_type *sep_list = (sep_data_type *)PanicNull( blockGetBase(device->bitfields.device_id, data_block_sep_list) );
    uint16 sep_list_size = blockGetSize(device->bitfields.device_id, data_block_sep_list) / sizeof(sep_data_type);
    
    while (sep_list_size--)
    {
        const sep_config_type *sep_config = sep_list->sep_config;
        
        if (sep_config->seid == seid)
        {
            const uint8 * local_codec = sep_config->caps;
            uint16 local_codec_size = sep_config->size_caps;
            
            /* Should never fail since local codec caps will always be present */
            PanicFalse( a2dpFindCodecSpecificInformation(&local_codec, &local_codec_size) );
            
            return local_codec[AVDTP_SERVICE_CAPS_MEDIA_CODEC_TYPE_OFFSET];
        }
        
        sep_list++;
    }
    
    return AVDTP_MEDIA_CODEC_UNDEFINED;
}
#endif

static bool buildPreferredList (remote_device *device, const uint8 *seid_list, uint16 seid_list_size)
{
    unsigned i;
    uint8 available_seps = 0;
    PRINT(("buildPreferredList seid_list=%p seid_list_size=%u   ", seid_list, seid_list_size));
    
    blockRemove((uint8)device->bitfields.device_id, data_block_list_preferred_local_seids);
    
    if ((seid_list_size == 0) || (seid_list == NULL))
    {   /* Build preferred list from local seid list */
        sep_data_type *sep_list = (sep_data_type *)PanicNull( blockGetBase((uint8)device->bitfields.device_id, data_block_sep_list) );
        unsigned sep_list_size = blockGetSize((uint8)device->bitfields.device_id, data_block_sep_list) / sizeof(sep_data_type);
        
        /* Find available seps */
        for (i=0; i<sep_list_size; i++)
        {
            if (!sep_list[i].in_use)
            {
                available_seps++;
            }
        }
        
        if (available_seps)
        {
            sep_info *preferred_list = (sep_info *)PanicNull( blockAdd((uint8)device->bitfields.device_id, data_block_list_preferred_local_seids, 
                                                                       available_seps, sizeof(sep_info)) );
            
            PRINT(("preferred list = "));
            
            while (sep_list_size--)
            {
                if (!sep_list->in_use)
                {
                    const sep_config_type *sep_config = sep_list->sep_config;
                    const uint8 * local_codec = sep_config->caps;
                    uint16 local_codec_size = sep_config->size_caps;
                    
                    /* Should never fail since local codec caps will always be present */
                    PanicFalse( a2dpFindCodecSpecificInformation(&local_codec, &local_codec_size) );
                    
                    preferred_list->seid = sep_config->seid;
                    preferred_list->role = sep_config->role;
                    preferred_list->codec = local_codec[AVDTP_SERVICE_CAPS_MEDIA_CODEC_TYPE_OFFSET];

                    PRINT(("[s:%u,r:%u,c:%u] ", preferred_list->seid, preferred_list->role, preferred_list->codec));
                    preferred_list++;
                }
                
                sep_list++;
            }
            
            return TRUE;
        }
    }
    else
    {   /* Build preferred list from supplied seid list */
        /* Find available seps */
        for (i=0; i<seid_list_size; i++)
        {
            const sep_data_type *sep_data = findLocalCodecInfo(device, seid_list[i]);
            
            if (sep_data && !sep_data->in_use)
            {
                available_seps++;
            }
        }
        
        if (available_seps)
        {
            sep_info *preferred_list = (sep_info *)PanicNull( blockAdd((uint8)device->bitfields.device_id, data_block_list_preferred_local_seids, available_seps, sizeof(sep_info)) );
            
            PRINT(("preferred list = "));
            
            while (seid_list_size--)
            {
                const sep_data_type *sep_data = findLocalCodecInfo(device, *seid_list);
                
                if (sep_data && !sep_data->in_use)
                {
                    const sep_config_type *sep_config = sep_data->sep_config;
                    const uint8 * local_codec = sep_config->caps;
                    uint16 local_codec_size = sep_config->size_caps;
                    
                    /* Should never fail since local codec caps will always be present */
                    PanicFalse( a2dpFindCodecSpecificInformation(&local_codec, &local_codec_size) );
                    
                    preferred_list->seid = *seid_list;
                    preferred_list->role = sep_config->role;
                    preferred_list->codec = local_codec[AVDTP_SERVICE_CAPS_MEDIA_CODEC_TYPE_OFFSET];
                    
                    PRINT(("[s:%u,r:%u,c:%u] ", preferred_list->seid, preferred_list->role, preferred_list->codec));
                    preferred_list++;
                }
                
                seid_list++;
            }
            
            return TRUE;
        }
    }
    
    PRINT(("\n"));
    return FALSE;
}


/*****************************/
/****  Inbound responses  ****/
/*****************************/

static bool doesCodecMatch (sep_info *local_sep, sep_info *remote_sep)
{
    PRINT(("doesCodecMatch -  local_sep:%p = seid:0x%X role:%u codec:0x%X\n", (void*)local_sep, local_sep->seid, local_sep->role, local_sep->codec));
    PRINT(("doesCodecMatch - remote_sep:%p = seid:0x%X role:%u codec:0x%X\n", (void*)remote_sep, remote_sep->seid, remote_sep->role, remote_sep->codec));

    if ((local_sep->codec == remote_sep->codec) && (local_sep->codec != AVDTP_MEDIA_CODEC_UNDEFINED) && (local_sep->role != remote_sep->role))
    {
        return TRUE;
    }
    
    return FALSE;
}

static bool haveCodecType (sep_info *sep)
{
    if (sep->codec != AVDTP_MEDIA_CODEC_UNDEFINED)
    {
        return TRUE;
    }
    
    return FALSE;
}

static bool haveCodecCaps (remote_device *device, sep_info *sep)
{
    const uint8 *service_caps;
    
    if ((service_caps = blockGetBase( (uint8)device->bitfields.device_id, data_block_configured_service_caps )) != NULL)
    {
        if ( a2dpFindCodecSpecificInformation(&service_caps, 0) )
        {
            if (service_caps[3] == sep->codec)
            {
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

static bool selectNextSeid (remote_device *device)
{
    sep_info *remote_sep;
    sep_info *local_sep;
    
    PRINT(("selectNextSeid - "));
    
    if ((remote_sep = (sep_info *)blockSetCurrent( (uint8)device->bitfields.device_id, data_block_list_discovered_remote_seids, DATA_BLOCK_INDEX_NEXT )) == NULL)
    {
        remote_sep = (sep_info *)PanicNull( blockSetCurrent( (uint8)device->bitfields.device_id, data_block_list_discovered_remote_seids, 0 ) );
        if ((local_sep = (sep_info *)blockSetCurrent( (uint8)device->bitfields.device_id, data_block_list_preferred_local_seids, DATA_BLOCK_INDEX_NEXT )) == NULL)
        /* && != AVDTP_MEDIA_CODEC_UNDEFINED */
        {
            PRINT(("Sep lists exhausted\n"));

            /* Incohesive but a result of optimisation.  If either selectNextSeid or continueStreamConnect fail then we can't proceed any */
            /* further, so we need to revert to the idle stream state and inform the app.  However, continueStreamConnect only fails if   */
            /* its call to selectNextSeid fails...                                                                                        */
            a2dpStreamReset(device);
            a2dpMediaOpenCfm(device, NULL, a2dp_operation_fail);
           
            return FALSE;
        }
        
        device->local_sep = *local_sep;
    }
    
    device->remote_sep = *remote_sep;
    
    PRINT(("local_sep = codec:0x%X seid:%u, remote_sep = codec:0x%X seid:%u\n", device->local_sep.codec, device->local_sep.seid, device->remote_sep.codec, device->remote_sep.seid));
    return TRUE;
}

static void issueDiscover (remote_device *device, const uint8 *seid_list, uint16 seid_list_size)
{
    if (buildPreferredList(device, seid_list, seid_list_size))
    {
        if( a2dpSendCommand(device, avdtp_discover, NULL, AVDTP_NO_PAYLOAD) )
        {
            a2dpSetStreamState(device, avdtp_stream_discovering);
        }
        else
        {
            a2dpStreamAbort(device);
            a2dpMediaOpenCfm(device, NULL, a2dp_operation_fail);
        }
    }
    else
    {
        a2dpStreamAbort(device);
        a2dpMediaOpenCfm(device, NULL, a2dp_invalid_parameters);
    }
}

static void issueGetCapabilities (remote_device *device)
{
    uint8 signal_id;
    
    PRINT(("issueGetCapabilities\n"));
    
    if (device->bitfields.profile_version < avdtp_version_13)
    {   /* Use legacy command, to request basic capabilities, for backwards compatibility */
        signal_id = avdtp_get_capabilities;
    }
    else
    {   /* Request full capabilities */
        signal_id = avdtp_get_all_capabilities;
    }
    
    if ( a2dpSendCommand(device, signal_id, NULL, AVDTP_NO_PAYLOAD) )
    {
        a2dpSetStreamState(device, avdtp_stream_reading_caps);
    }
    else
    {
        a2dpStreamAbort(device);
        a2dpMediaOpenCfm(device, NULL, a2dp_operation_fail);
    }
}

static bool issueSetConfiguration (remote_device *device)
{
    const uint8 *remote_service_caps;
    uint16 remote_service_caps_size;
    bool result = FALSE;
    const sep_data_type *sep_data = NULL;

    PRINT(("issueSetConfiguration - "));
    
    switch ( a2dpSelectConfigurationParameters(device) )
    {
    case CONFIGURATION_BY_CLIENT:       
        /* Issue request to client (app) to select an appropriate configuration */
        PRINT(("CONFIGURATION_BY_CLIENT\n"));

        sep_data = findLocalCodecInfo(device, device->local_sep.seid);
        remote_service_caps = sep_data->sep_config->caps;
        remote_service_caps_size = sep_data->sep_config->size_caps;

        a2dpCodecConfigureInd(device, (uint8)device->local_sep.seid, remote_service_caps_size, remote_service_caps);     /* TODO: Make multi-stream aware */
        a2dpSetStreamState(device, avdtp_stream_processing_caps);
        result = TRUE;
        break;
        
    case CONFIGURATION_SELECTED:        
        /* Library has managed to select a suitable configuation */
        PRINT(("CONFIGURATION_SELECTED\n"));
        if ( sendSetConfiguration(device) )
        {
            a2dpSetStreamState(device, avdtp_stream_configuring);
        }
        else
        {
            a2dpStreamAbort(device);
            a2dpMediaOpenCfm(device, NULL, a2dp_operation_fail);
        }
        result = TRUE;
        break;
        
    case CONFIGURATION_NOT_SELECTED:    
        PRINT(("CONFIGURATION_NOT_SELECTED\n"));
        break;
    }
    
    return result;
}

static void issueClientSetConfiguration(remote_device *device)
{
    PRINT(("issueClientSetConfiguration\n"));
    
    if ( sendSetConfiguration(device) )
    {
        a2dpSetStreamState(device, avdtp_stream_configuring);
    }
    else
    {
        a2dpStreamAbort(device);
        a2dpMediaOpenCfm(device, NULL, a2dp_operation_fail);
    }
}

static void requestDelayReport (remote_device *device)
{
    PRINT(("requestDelayReport\n"));

    a2dpMediaAvSyncDelayInd(device, (uint8)device->local_sep.seid);
    a2dpSetStreamState(device, avdtp_stream_configured);
    MessageSendLater((Task)&a2dp->task, A2DP_INTERNAL_CLIENT_RSP_TIMEOUT_BASE + device->bitfields.device_id, 0, WATCHDOG_TGAVDP100);
}

static void waitDelayReport (remote_device *device)
{
    PRINT(("waitDelayReport\n"));

    a2dpSetStreamState(device, avdtp_stream_configured);
    MessageSendLater((Task)&a2dp->task, A2DP_INTERNAL_REMOTE_CMD_TIMEOUT_BASE + device->bitfields.device_id, 0, WATCHDOG_TGAVDP100);
}

static void issueOpen (remote_device *device)
{
    PRINT(("issueOpen\n"));
    
    if ( a2dpSendCommand(device, avdtp_open, NULL, AVDTP_NO_PAYLOAD) )
    {
        a2dpSetStreamState(device, avdtp_stream_local_opening);
    }
    else
    {
        a2dpStreamAbort(device);
        a2dpMediaOpenCfm(device, NULL, a2dp_operation_fail);
    }
}

static bool continueStreamConnect (remote_device *device)
{
    PRINT(("continueStreamConnect(%p) stream_state=%u\n", (void*)device, device->signal_conn.status.stream_state));
    
    /* Attempt Stream Connect */
    do
    {
        switch (device->signal_conn.status.stream_state)
        {
        case avdtp_stream_discovering:
            /* AVDTP_DISCOVER was successful, request capabilities of first remote seid */
            issueGetCapabilities(device);
            return TRUE;
                    
        case avdtp_stream_reading_caps:
            /* AVDTP_GET_CAPABILITIES was successful */
            if( doesCodecMatch(&device->local_sep, &device->remote_sep) )
            {   /* We will have the codec caps because we have just asked for them */
                if ( issueSetConfiguration(device) )
                {
                    return TRUE;
                }
            }
            
            /* Codecs do not match.  Retrun to idle state to ensure next remote codecs caps are obtained */
            a2dpSetStreamState(device, avdtp_stream_idle);
            break;
                
        case avdtp_stream_configuring:
        {   /* AVDTP_SET_CONFIGURATION was successful */
            uint8 *configured_caps = blockGetBase((uint8)device->bitfields.device_id, data_block_configured_service_caps);
            uint16 size_configured_caps = blockGetSize((uint8)device->bitfields.device_id, data_block_configured_service_caps);
            
            /* Issue an Av Sync delay notification, if delay reporting has been configured and local device is configured as a sink */
            if (a2dpIsServiceSupported(AVDTP_SERVICE_DELAY_REPORTING, configured_caps, size_configured_caps))
            {
                if (device->local_sep.role == a2dp_sink)
                {
                    requestDelayReport(device);
                }
                else
                {
                    waitDelayReport(device);
                }
            }
            else
            {
                issueOpen(device);
            }
            return TRUE;
        }      
        case avdtp_stream_configured:
            /* AVDTP_DELAYREPORT was successful, attempt to open the stream */
            issueOpen(device);
            return TRUE;
            
        case avdtp_stream_idle:
            /* Last AVDTP command was rejected for some reason */
            if ( !haveCodecType(&device->remote_sep) )
            {   /* We know nothing about this remote seid, request its caps */
                issueGetCapabilities(device);
                return TRUE;
            }
            
            if( doesCodecMatch(&device->local_sep, &device->remote_sep) )
            {   /* Local (the one we want to use) and remote (the one we are prodding) endpoints support the same codec */
                if ( !haveCodecCaps(device, &device->remote_sep) )
                {   /* We don't have detailed caps stored, request the info */
                    issueGetCapabilities(device);
                    return TRUE;
                }
                else
                {   /* We have already cached the detailed caps for the remote seid, try to set something up */
                    if ( issueSetConfiguration(device) )
                    {
                        return TRUE;
                    }
                }
            }
            break;

        default:
            /* Should not be able to get here */
            PRINT(("UNEXPECTED A2DP LIBRARY STATE: continueStreamConnect %u\n",device->signal_conn.status.stream_state));
            a2dpStreamAbort(device);
            break;
        }
    } while ( selectNextSeid(device) );
    
    return FALSE;
}

/****************************************************************************/
void a2dpHandleInternalClientRspTimeout(MessageId msg_id)
{
    PanicFalse((uint16)(msg_id-A2DP_INTERNAL_CLIENT_RSP_TIMEOUT_BASE) < A2DP_MAX_REMOTE_DEVICES);

    {    
        remote_device *device = &a2dp->remote_conn[(uint16)(msg_id-A2DP_INTERNAL_CLIENT_RSP_TIMEOUT_BASE)];

        switch (device->signal_conn.status.stream_state)
        {
        case avdtp_stream_idle:
        case avdtp_stream_discovering:
        case avdtp_stream_reading_caps:
        case avdtp_stream_processing_caps:
        case avdtp_stream_configuring:
            break;
            
        case avdtp_stream_configured:
            /* Will have been waiting for client to call a2dpMediaAvSyncDelayResponse(), which has not been done */
            /* We have no idea of any AV Delay, so assume zero */
            sendDelayReport(device, 0);
            
            /* Stream Establishment process will continue on receiving an AVDTP_DELAYREPORT_RSP */
            break;
            
        case avdtp_stream_local_opening:
        case avdtp_stream_remote_opening:
        case avdtp_stream_open:
        case avdtp_stream_streaming:
        case avdtp_stream_local_starting:
        case avdtp_stream_remote_starting:
        case avdtp_stream_local_suspending:
        case avdtp_stream_local_closing:
        case avdtp_stream_remote_closing:
        case avdtp_stream_reconfig_reading_caps:
        case avdtp_stream_reconfiguring:
        case avdtp_stream_local_aborting:
        case avdtp_stream_remote_aborting:
            break;
        }
        
    }
}


/****************************************************************************/
void a2dpHandleInternalRemoteCmdTimeout(MessageId msg_id)
{
    PanicFalse((uint16)(msg_id-A2DP_INTERNAL_REMOTE_CMD_TIMEOUT_BASE) < A2DP_MAX_REMOTE_DEVICES);

    {    
        remote_device *device = &a2dp->remote_conn[(uint16)(msg_id-A2DP_INTERNAL_REMOTE_CMD_TIMEOUT_BASE)];

        switch (device->signal_conn.status.stream_state)
        {
        case avdtp_stream_idle:
        case avdtp_stream_discovering:
        case avdtp_stream_reading_caps:
        case avdtp_stream_processing_caps:
        case avdtp_stream_configuring:
            break;
            
        case avdtp_stream_configured:
            /* Will have been waiting for remote device to issue an AV Sync Delay report, which has not been done */
            /* Continue the Stream Establishment process */
            continueStreamConnect(device);
            break;
            
        case avdtp_stream_local_opening:
        case avdtp_stream_remote_opening:
        case avdtp_stream_open:
        case avdtp_stream_streaming:
        case avdtp_stream_local_starting:
        case avdtp_stream_remote_starting:
        case avdtp_stream_local_suspending:
        case avdtp_stream_local_closing:
        case avdtp_stream_remote_closing:
        case avdtp_stream_reconfig_reading_caps:
        case avdtp_stream_reconfiguring:
        case avdtp_stream_local_aborting:
        case avdtp_stream_remote_aborting:
            break;
        }
        
    }
}


void a2dpHandleDiscoverResponse (remote_device *device)
{
    if (device->signal_conn.status.stream_state != avdtp_stream_discovering)
    {   /* Only expecting to receive this message in the appropriate state - ignore it */
        return;
    }
    
    if ( (getReceivedMessageType(device) != avdtp_message_type_response_accept) || !a2dpProcessDiscoverResponse(device) )
    {
        a2dpStreamReset(device);
        a2dpMediaOpenCfm(device, NULL, a2dp_operation_fail);
    }
    else
    {   /* Process discover reponse fn will have generated a new list_discovered_remote_seids data block */
        /* Ensure we are pointing to the first seid in both lists */
        device->local_sep = *(sep_info *)PanicNull( blockSetCurrent( (uint8)device->bitfields.device_id, data_block_list_preferred_local_seids, 0 ) );
        device->remote_sep = *(sep_info *)PanicNull( blockSetCurrent( (uint8)device->bitfields.device_id, data_block_list_discovered_remote_seids, 0 ) );
        PRINT(("local_sep = codec:0x%X seid:%u, remote_sep = codec:0x%X seid:%u\n", device->local_sep.codec, device->local_sep.seid, device->remote_sep.codec, device->remote_sep.seid));
    
        continueStreamConnect( device );
    }
}


void a2dpHandleGetCapabilitiesResponse (remote_device *device)
{
    if (device->signal_conn.status.stream_state != avdtp_stream_reading_caps)
    {   /* Only expecting to receive this message in the appropriate state - ignore it */
        return;
    }
    
    if ((getReceivedMessageType(device) == avdtp_message_type_response_accept) && a2dpProcessGetCapabilitiesResponse(device))
    {
        continueStreamConnect( device );
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_idle);
        
        if ( selectNextSeid(device) )
        {
            continueStreamConnect( device );
        }
    }
}


void a2dpHandleCodecConfigureResponse(const A2DP_INTERNAL_CODEC_CONFIGURE_RSP_T *rsp)
{
    remote_device *device = rsp->device;
    
    if (device->signal_conn.status.stream_state != avdtp_stream_processing_caps)
    {   /* Only expecting to receive this message in the appropriate state - ignore it */
        return;
    }
    
    if (rsp->accept && a2dpProcessCodecConfigureResponse(device, rsp->local_seid, rsp->codec_service_caps, rsp->size_codec_service_caps))
    {
        issueClientSetConfiguration(device);
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_idle);
        
        if ( selectNextSeid(device) )
        {
            continueStreamConnect( device );
        }
    }
}


void a2dpHandleSetConfigurationResponse (remote_device *device)
{
    if (device->signal_conn.status.stream_state != avdtp_stream_configuring)
    {   /* Only expecting to receive this message in the appropriate state - ignore it */
        return;
    }
    
    if (getReceivedMessageType(device) == avdtp_message_type_response_accept)
    {
        /* Mark the current SEP as in use */
        device->bitfields.sep_configured_locally = TRUE;
        ((sep_data_type *)PanicNull( blockGetCurrent( (uint8)device->bitfields.device_id, data_block_sep_list ) ))->in_use |= A2DP_SEP_IN_USE;
        continueStreamConnect( device );
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_idle);
        
        if ( selectNextSeid(device) )
        {
            continueStreamConnect( device );
        }
    }
}

void a2dpHandleReconfigureResponse (remote_device *device)
{
    if (device->signal_conn.status.stream_state != avdtp_stream_reconfiguring)
    {   /* Only expecting to receive this message in the appropriate state - ignore it */
        return;
    }
    
    if (getReceivedMessageType(device) == avdtp_message_type_response_accept)
    {
        a2dpProcessReconfigureResponse(device);
        a2dpSetStreamState(device, avdtp_stream_open);
        a2dpMediaReconfigureCfm(device, &device->media_conn[0], a2dp_success);
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_open);
        a2dpMediaReconfigureCfm(device, &device->media_conn[0], a2dp_rejected_by_remote_device);
   }
}

void a2dpHandleOpenResponse (remote_device *device)
{
    if (device->signal_conn.status.stream_state != avdtp_stream_local_opening)
    {   /* Only expecting to receive this message in the appropriate state - ignore it */
        return;
    }
    
    if (getReceivedMessageType(device) == avdtp_message_type_response_accept)
    {
        a2dpMediaConnectReq(device);   /* TODO: Add support for different flush timeout values */
    }
    else
    {
        a2dpStreamAbort(device);
        a2dpMediaOpenCfm(device, NULL, a2dp_operation_fail);
    }
}

void a2dpHandleStartResponse (remote_device *device)
{
    if ( (device->signal_conn.status.stream_state != avdtp_stream_open) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_starting) &&
         (device->signal_conn.status.stream_state != avdtp_stream_streaming) )
    {   /* Only expecting to receive this message in the appropriate state - inform app */
        a2dpMediaStartCfm(device, &device->media_conn[0], a2dp_wrong_state);
        return;
    }
    
    /* If we are already streaming, don't worry about any error response from the remote device */
    if ((getReceivedMessageType(device) == avdtp_message_type_response_accept) || (device->signal_conn.status.stream_state == avdtp_stream_streaming))
    {
        /* end point is now streaming */
        a2dpSetStreamState(device, avdtp_stream_streaming);
        /* tell application the good news! */
        a2dpMediaStartCfm(device, &device->media_conn[0], a2dp_success);
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_open);
        a2dpMediaStartCfm(device, &device->media_conn[0], a2dp_rejected_by_remote_device);
    }
}

void a2dpHandleSuspendResponse (remote_device *device)
{
    if ( (device->signal_conn.status.stream_state != avdtp_stream_local_suspending) &&
         (device->signal_conn.status.stream_state != avdtp_stream_open) )
    {   /* Only expecting to receive this message in the appropriate state - inform app */
        a2dpMediaSuspendCfm(device, &device->media_conn[0], a2dp_wrong_state);
        return;
    }
    
    /* If we have already suspended, don't worry about any error response from the remote device */
    if ((getReceivedMessageType(device) == avdtp_message_type_response_accept) || (device->signal_conn.status.stream_state == avdtp_stream_open))
    {
        /* end point has now suspended */
        a2dpSetStreamState(device, avdtp_stream_open);
        /* tell application the good news! */
        a2dpMediaSuspendCfm(device, &device->media_conn[0], a2dp_success);
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_streaming);
        a2dpMediaSuspendCfm(device, &device->media_conn[0], a2dp_rejected_by_remote_device);
    }
}

void a2dpHandleCloseResponse (remote_device *device)
{
    signalling_channel *signalling = &device->signal_conn;
    
    if ((signalling->status.stream_state != avdtp_stream_open) &&
        (signalling->status.stream_state != avdtp_stream_local_starting) &&
        (signalling->status.stream_state != avdtp_stream_remote_starting) &&
        (signalling->status.stream_state != avdtp_stream_local_suspending) &&
        (signalling->status.stream_state != avdtp_stream_streaming) &&
        (signalling->status.stream_state != avdtp_stream_reconfig_reading_caps) &&
        (signalling->status.stream_state != avdtp_stream_reconfiguring) &&
        (signalling->status.stream_state != avdtp_stream_remote_closing) &&
        (signalling->status.stream_state != avdtp_stream_local_closing))
    {   /* Only expecting to receive this message in the appropriate state - inform app */
        a2dpMediaCloseInd(device, &device->media_conn[0], a2dp_wrong_state);
        return;
    }
    
    if (getReceivedMessageType(device) == avdtp_message_type_response_accept)
    {
        /* Mark the current SEP as no longer being in use */
        ((sep_data_type *)PanicNull( blockGetCurrent( (uint8)device->bitfields.device_id, data_block_sep_list ) ))->in_use &= ~A2DP_SEP_IN_USE;
        
        /* Reset the local and remote SEIDs */
        blockSetCurrent( (uint8)device->bitfields.device_id, data_block_sep_list, 0 );
        device->remote_sep.codec = AVDTP_MEDIA_CODEC_UNDEFINED;
        device->local_sep.codec = AVDTP_MEDIA_CODEC_UNDEFINED;
        
        /* Remove stream specific data */
        blockRemove( (uint8)device->bitfields.device_id, data_block_configured_service_caps );
        blockRemove( (uint8)device->bitfields.device_id, data_block_list_discovered_remote_seids );
        
        a2dpSetStreamState(device, avdtp_stream_local_closing);
        a2dpMediaDisconnectReq(device->media_conn[0].connection.active.sink); /* TODO: Make multi-stream aware */
    }
    else
    {   /* Don't take no for an answer - if our request is rejected then abort to force a close */
        a2dpStreamAbort(device);
    }
}

void a2dpHandleAbortResponse (remote_device *device)
{
    /* If we get a response then it means the remote device has accepted our abort request so, reset the stream state */
    a2dpStreamReset(device);
    
    if ( !a2dpDisconnectAllMedia(device) )
    {   /* No media channels to disconnect, check if signalling needs closing */
        a2dpSignallingDisconnectPending(device);
    }
}

void a2dpHandleGetAllCapabilitiesResponse (remote_device *device)
{
    if (device->signal_conn.status.stream_state != avdtp_stream_reading_caps)
    {   /* Only expecting to receive this message in the appropriate state - ignore it */
        return;
    }
    
    if ((getReceivedMessageType(device) == avdtp_message_type_response_accept) && a2dpProcessGetAllCapabilitiesResponse(device))
    {
        continueStreamConnect( device );
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_idle);
        
        if ( selectNextSeid(device) )
        {
            continueStreamConnect( device );
        }
    }
}

void a2dpHandleDelayReportResponse (remote_device *device)
{
    if ( (device->signal_conn.status.stream_state != avdtp_stream_configured) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_opening) &&
         (device->signal_conn.status.stream_state != avdtp_stream_remote_opening) &&
         (device->signal_conn.status.stream_state != avdtp_stream_open) &&
         (device->signal_conn.status.stream_state != avdtp_stream_streaming) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_starting) &&
         (device->signal_conn.status.stream_state != avdtp_stream_remote_starting) )
    {   /* Only expecting to receive this message in the appropriate state - ignore it */
        return;
    }

    if (device->signal_conn.status.stream_state == avdtp_stream_configured)
    {   /* A media stream is in process of being set up */
        if (getReceivedMessageType(device) == avdtp_message_type_response_accept)
        {
            a2dpMediaAvSyncDelayCfm (device, (uint8)device->local_sep.seid, a2dp_success);
            
            if (device->bitfields.sep_configured_locally)
            {   /* If we configured the sep locally then we are the initiator of the stream establishment process */
                continueStreamConnect( device );
            }
        }
        else
        {
            a2dpMediaAvSyncDelayCfm (device, (uint8)device->local_sep.seid, a2dp_rejected_by_remote_device);

            a2dpStreamAbort(device);
            a2dpMediaOpenCfm(device, NULL, a2dp_operation_fail);
        }
    }
    else if (device->signal_conn.status.stream_state == avdtp_stream_streaming)
    {   /* A media stream is streaming, thus there should be no other AVDTP commands that are likely to occur */
        if (isDelayReportPending(device))
        {
            setDelayReportPending(device, FALSE);
            a2dpStreamDelayReport(device, device->media_conn[0].status.delay);
        }
    }
}


/*****************************/
/****  Inbound commands  ****/
/*****************************/

void a2dpHandleDiscoverCommand (remote_device *device)
{
    uint16 error_code;
    uint16 payload_size;
    
    if ( (error_code = a2dpProcessDiscoverCommand(device, &payload_size))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_discover, error_code);
    }
    else
    {
        a2dpSendAccept(device, avdtp_discover, payload_size);
    }
}

void a2dpHandleGetCapabilitiesCommand (remote_device *device)
{
    uint16 error_code;
    uint16 payload_size;
    
    if ( (error_code = a2dpProcessGetCapabilitiesCommand(device, &payload_size))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_get_capabilities, error_code);
    }
    else
    {
        a2dpSendAccept(device, avdtp_get_capabilities, payload_size);
    }
}

void a2dpHandleSetConfigurationCommand (remote_device *device)
{
    uint16 error_code;
    signalling_channel *signalling = &device->signal_conn;
    
    if ( (signalling->status.stream_state != avdtp_stream_idle) &&
         (signalling->status.stream_state != avdtp_stream_discovering) &&
         (signalling->status.stream_state != avdtp_stream_reading_caps) &&
         (signalling->status.stream_state != avdtp_stream_processing_caps) &&
         (signalling->status.stream_state != avdtp_stream_configuring) &&
         (signalling->status.stream_state != avdtp_stream_configured) )
    {   /* TP/SIG/SMG/BI-08-C requires us to process an AVDTP_SET_CONFIGURATION command in the Configured state */
        a2dpSendReject(device, avdtp_set_configuration, avdtp_bad_state);
    }
    else if ( (error_code = a2dpProcessSetConfigurationCommand(device))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_set_configuration, error_code);
    }
    else
    {
        uint8 *configured_caps = blockGetBase((uint8)device->bitfields.device_id, data_block_configured_service_caps);
        uint16 size_configured_caps = blockGetSize((uint8)device->bitfields.device_id, data_block_configured_service_caps);
            
        device->bitfields.sep_configured_locally = FALSE;
        a2dpSetStreamState(device, avdtp_stream_configured);
        a2dpSendAccept(device, avdtp_set_configuration, 0);

        if (a2dpIsServiceSupported(AVDTP_SERVICE_DELAY_REPORTING, configured_caps, size_configured_caps) && (device->local_sep.role == a2dp_sink))
        {   /* As we are operating as a sink, issue Av Sync delay to source */
            a2dpMediaAvSyncDelayInd(device, (uint8)device->local_sep.seid);
        }
    }
}

void a2dpHandleGetConfigurationCommand (remote_device *device)
{
    uint16 error_code;
    uint16 payload_size;
    
    if ( (error_code = a2dpProcessGetConfigurationCommand(device, &payload_size))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_get_configuration, error_code);
    }
    else
    {
        a2dpSendAccept(device, avdtp_get_configuration, payload_size);
    }
}

void a2dpHandleReconfigureCommand (remote_device *device)
{
    uint16 error_code;
    signalling_channel *signalling = &device->signal_conn;
    
    /* should not receive RECONFIGURE command in states other than
     * avdtp_stream_open & avdtp_stream_remote_opening & avdtp_stream_local_opening */
    if ((signalling->status.stream_state != avdtp_stream_open) &&
        (signalling->status.stream_state != avdtp_stream_remote_opening) &&
        (signalling->status.stream_state != avdtp_stream_local_opening))
    {
        a2dpSendReject(device, avdtp_reconfigure, avdtp_bad_state);
    }
    else if ( (error_code = a2dpProcessReconfigureCommand(device))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_reconfigure, error_code);
    }
    else
    {
        a2dpSendAccept(device, avdtp_reconfigure, 0);
        a2dpMediaReconfigureInd(device, &device->media_conn[0]);
    }
}

void a2dpHandleOpenCommand (remote_device *device)
{
    uint16 error_code;
    signalling_channel *signalling = &device->signal_conn;
    
    if (signalling->status.stream_state != avdtp_stream_configured)
    {
        a2dpSendReject(device, avdtp_open, avdtp_bad_state);
    }
    else if ( (error_code = a2dpProcessOpenCommand(device))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_open, error_code);
    }
    else
    {
        /* The call to a2dpProcessOpenCommand will have already validated this, so it should not panic */
        const sep_config_type *sep_config = ((sep_data_type *)PanicNull( blockGetCurrent( (uint8)device->bitfields.device_id, data_block_sep_list )))->sep_config;
        
        a2dpSetStreamState(device, avdtp_stream_remote_opening);
        a2dpMediaOpenInd(device, (uint8)sep_config->seid);
    }
}

bool a2dpHandleCloseCommand (remote_device *device)
{
    uint16 error_code;
    
    if ( (device->signal_conn.status.stream_state == avdtp_stream_remote_opening) ||
         (device->signal_conn.status.stream_state == avdtp_stream_local_opening) )
    {   /* Return FALSE to indicate that processing of AVDTP_CLOSE command will be deferred until later */
        return FALSE;
    }
    else if ( (device->signal_conn.status.stream_state != avdtp_stream_open) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_starting) &&
         (device->signal_conn.status.stream_state != avdtp_stream_remote_starting) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_suspending) &&
         (device->signal_conn.status.stream_state != avdtp_stream_reconfig_reading_caps) &&
         (device->signal_conn.status.stream_state != avdtp_stream_reconfiguring) &&
         (device->signal_conn.status.stream_state != avdtp_stream_streaming) &&
	     (device->signal_conn.status.stream_state != avdtp_stream_local_closing) )
    {
        a2dpSendReject(device, avdtp_close, avdtp_bad_state);
    }
    else if ( (error_code = a2dpProcessCloseCommand(device))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_close, error_code);
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_remote_closing);
        a2dpSendAccept(device, avdtp_close, AVDTP_NO_PAYLOAD);
        /* App is informed on l2cap media channel close */
    }
    
    return TRUE;
}

bool a2dpHandleStartCommand (remote_device *device)
{
    uint16 error_code;

    if ( (device->signal_conn.status.stream_state == avdtp_stream_remote_opening) ||
         (device->signal_conn.status.stream_state == avdtp_stream_local_opening) )
    {   /* Return FALSE to indicate that processing of AVDTP_START command will be deferred until later */
        return FALSE;
    }
    else if ( (device->signal_conn.status.stream_state != avdtp_stream_open) &&
              (device->signal_conn.status.stream_state != avdtp_stream_local_starting) &&
              (device->signal_conn.status.stream_state != avdtp_stream_remote_starting) )
    {
        a2dpSendReject(device, avdtp_start, avdtp_bad_state);
    }
    else if ( (error_code = a2dpProcessStartCommand(device))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_start, error_code);
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_remote_starting);
        a2dpMediaStartInd(device, &device->media_conn[0]);
    }

    return TRUE;
}

void a2dpHandleSuspendCommand (remote_device *device)
{
    uint16 error_code;
    
    if ( (device->signal_conn.status.stream_state != avdtp_stream_streaming) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_suspending) )
    {
        a2dpSendReject(device, avdtp_suspend, avdtp_bad_state);
    }
    else if ( (error_code = a2dpProcessSuspendCommand(device))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_suspend, error_code);
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_open);
        a2dpSendAccept(device, avdtp_suspend, AVDTP_NO_PAYLOAD);
        a2dpMediaSuspendInd(device, &device->media_conn[0]);
    }
}

void a2dpHandleAbortCommand (remote_device *device)
{
    if ( a2dpProcessAbortCommand(device) )
    {   /* If the supplied seid is recognised, we must abort */
        a2dpSendAccept(device, avdtp_abort, AVDTP_NO_PAYLOAD);
        a2dpDisconnectAllMedia(device);
        a2dpStreamReset(device);
    }
}

void a2dpHandleGetAllCapabilitiesCommand (remote_device *device)
{
    uint16 error_code;
    uint16 payload_size;
    

    /* If the connected device supports less than AVDTP 1.3 than process the GetCapabilities command otherwise process GetAllCapabilities */
    
    if(device->bitfields.profile_version < avdtp_version_13) 
    {
        error_code = a2dpProcessGetCapabilitiesCommand(device, &payload_size);
    }
    else
    {
        error_code = a2dpProcessGetAllCapabilitiesCommand(device, &payload_size);
    }
    
    if (error_code != (uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_get_all_capabilities, error_code);
    }
    else
    {
        a2dpSendAccept(device, avdtp_get_all_capabilities, payload_size);
    }
}

void a2dpHandleDelayReportCommand (remote_device *device)
{
    uint16 error_code;
    
    if ( (device->signal_conn.status.stream_state != avdtp_stream_configured) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_opening) &&
         (device->signal_conn.status.stream_state != avdtp_stream_remote_opening) &&
         (device->signal_conn.status.stream_state != avdtp_stream_open) &&
         (device->signal_conn.status.stream_state != avdtp_stream_streaming) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_starting) &&
         (device->signal_conn.status.stream_state != avdtp_stream_remote_starting) )
    {
        a2dpSendReject(device, avdtp_delayreport, avdtp_bad_state);
    }
    else if ( (error_code = a2dpProcessDelayReportCommand(device))!=(uint16)avdtp_ok )
    {
        a2dpSendReject(device, avdtp_delayreport, error_code);
    }
    else
    {
        a2dpSendAccept(device, avdtp_delayreport, AVDTP_NO_PAYLOAD);
        a2dpMediaAvSyncDelayUpdatedInd(device, (uint8)device->local_sep.seid, device->media_conn[0].status.delay);
    }
    
    if ((device->signal_conn.status.stream_state == avdtp_stream_configured) && (device->bitfields.sep_configured_locally))
    {   /* If we configured the sep locally then we are the initiator of the stream establishment process */
        continueStreamConnect( device );
    }
}


/*****************************/
/****  Outbound commands  ****/
/*****************************/

bool a2dpStreamClose (remote_device *device)
{
    signalling_channel *signalling = &device->signal_conn;
    media_channel *media = &device->media_conn[0];      /* TODO; Make multi-stream aware */
    
    if (signalling->status.connection_state != avdtp_connection_connected)
    {
        return FALSE;
    }
    else if (media->status.conn_info.connection_state != avdtp_connection_connected)
    {
        return FALSE;
    }
    else if ((signalling->status.stream_state != avdtp_stream_open) &&
             (signalling->status.stream_state != avdtp_stream_streaming) &&
             (signalling->status.stream_state != avdtp_stream_local_starting) &&
             (signalling->status.stream_state != avdtp_stream_remote_starting) &&
             (signalling->status.stream_state != avdtp_stream_local_suspending))
    {
        return FALSE;
    }
    else
    {
        if ( a2dpSendCommand(device, avdtp_close, NULL, AVDTP_NO_PAYLOAD) )    /* TODO: Support multiple streams */
        {
            a2dpSetStreamState(device, avdtp_stream_local_closing);
        }
        else
        {
            a2dpStreamAbort(device);
        }
        
        return TRUE;
    }
}


void a2dpStreamEstablish (const A2DP_INTERNAL_MEDIA_OPEN_REQ_T *req)
{
    signalling_channel *signalling = &((remote_device *)PanicNull(req->device))->signal_conn;
    
    if ( signalling->status.connection_state != avdtp_connection_connected )
    {
        a2dpMediaOpenCfm(req->device, NULL, a2dp_no_signalling_connection);
    }
    else if (signalling->status.stream_state != avdtp_stream_idle)
    {
        a2dpMediaOpenCfm(req->device, NULL, a2dp_wrong_state);
    }
    else
    {   /* Signalling channel open - start discovery */
        issueDiscover(req->device, req->seid_list, req->size_seid_list);
    }
}


void a2dpStreamOpenResponse (const A2DP_INTERNAL_MEDIA_OPEN_RES_T *res)
{
    signalling_channel *signalling = &((remote_device *)PanicNull(res->device))->signal_conn;
    
    if ( signalling->status.connection_state != avdtp_connection_connected )
    {
        a2dpMediaOpenCfm(res->device, NULL, a2dp_no_signalling_connection);
    }
    else if (signalling->status.stream_state != avdtp_stream_remote_opening)
    {
        a2dpMediaOpenCfm(res->device, NULL, a2dp_wrong_state);
    }
    else
    {
        if ( res->accept )
        {
            a2dpSendAccept(res->device, avdtp_open, AVDTP_NO_PAYLOAD);
            /* App is informed on L2cap media connect */
        }
        else
        {
            a2dpSendReject(res->device, avdtp_open, avdtp_bad_state);
            a2dpStreamReset(res->device);
        }
    }
}


void a2dpStreamStart (const A2DP_INTERNAL_MEDIA_START_REQ_T *req)
{
    signalling_channel *signalling = &((remote_device *)PanicNull(req->device))->signal_conn;
    media_channel *media= (media_channel *)PanicNull(req->media);
    
    if (signalling->status.connection_state != avdtp_connection_connected)
    {
        a2dpMediaStartCfm(req->device, media, a2dp_no_signalling_connection);
    }
    else if (media->status.conn_info.connection_state != avdtp_connection_connected)
    {
        a2dpMediaStartCfm(req->device, media, a2dp_no_media_connection);
    }
    else if (signalling->status.stream_state != avdtp_stream_open)
    {
        a2dpMediaStartCfm(req->device, media, a2dp_wrong_state);
    }
    else
    {
        if( a2dpSendCommand(req->device, avdtp_start, NULL, AVDTP_NO_PAYLOAD) ) /* TODO: Support multiple streams */
        {
            a2dpSetStreamState(req->device, avdtp_stream_local_starting);
        }
        else
        {
            a2dpStreamAbort(req->device);
            a2dpMediaStartCfm(req->device, media, a2dp_operation_fail);
        }
    }
}

void a2dpStreamStartResponse (const A2DP_INTERNAL_MEDIA_START_RES_T *res)
{
    remote_device *device = PanicNull(res->device);

    if ( (device->signal_conn.status.stream_state != avdtp_stream_open) &&
         (device->signal_conn.status.stream_state != avdtp_stream_remote_starting) &&
         (device->signal_conn.status.stream_state != avdtp_stream_streaming) )
    {   /* If the stream state has changed, in the time taken for the local client to respond, send a rejection and inform app */
        a2dpSendReject(device, avdtp_start, avdtp_bad_state);
        a2dpMediaStartCfm(device, &device->media_conn[0], a2dp_wrong_state);
        return;
    }
    
    /* If we are already streaming, don't worry about any rejection response from the local client */
    if (res->accept || (device->signal_conn.status.stream_state == avdtp_stream_streaming))
    {
        /* end point is now streaming */
        a2dpSetStreamState(device, avdtp_stream_streaming);
        a2dpSendAccept(device, avdtp_start, AVDTP_NO_PAYLOAD);
        /* tell application the good news! */
        a2dpMediaStartCfm(device, &device->media_conn[0], a2dp_success);
    }
    else
    {
        a2dpSetStreamState(device, avdtp_stream_open);
        a2dpSendReject(device, avdtp_start, avdtp_bad_state);
        a2dpMediaStartCfm(device, &device->media_conn[0], a2dp_operation_fail);
    }
}

void a2dpStreamRelease (const A2DP_INTERNAL_MEDIA_CLOSE_REQ_T *req)
{
    signalling_channel *signalling = &((remote_device *)PanicNull(req->device))->signal_conn;
    media_channel *media = (media_channel *)PanicNull(req->media);
    
    if (signalling->status.connection_state != avdtp_connection_connected)
    {
        a2dpMediaCloseInd(req->device, media, a2dp_no_signalling_connection);
    }
    else if (media->status.conn_info.connection_state != avdtp_connection_connected)
    {
        a2dpMediaCloseInd(req->device, media, a2dp_no_media_connection);
    }
    else if ( !((signalling->status.stream_state == avdtp_stream_open) ||
                (signalling->status.stream_state == avdtp_stream_streaming) ||
                (signalling->status.stream_state == avdtp_stream_local_starting) ||
                (signalling->status.stream_state == avdtp_stream_remote_starting) ||
                (signalling->status.stream_state == avdtp_stream_local_suspending) ||
                (signalling->status.stream_state == avdtp_stream_reconfig_reading_caps) ||
                (signalling->status.stream_state == avdtp_stream_reconfiguring))
            )
    {
        a2dpMediaCloseInd(req->device, media, a2dp_wrong_state);
    }
    else
    {
        if ( a2dpSendCommand(req->device, avdtp_close, NULL, AVDTP_NO_PAYLOAD) )    /* TODO: Support multiple streams */
        {
            a2dpSetStreamState(req->device, avdtp_stream_local_closing);
        }
        else
        {
            a2dpStreamAbort(req->device);
            a2dpMediaCloseInd(req->device, media, a2dp_operation_fail);
        }
    }
}

void a2dpStreamSuspend (const A2DP_INTERNAL_MEDIA_SUSPEND_REQ_T *req)
{
    signalling_channel *signalling = &((remote_device *)PanicNull(req->device))->signal_conn;
    media_channel *media= (media_channel *)PanicNull(req->media);
    
    if (signalling->status.connection_state != avdtp_connection_connected)
    {
        a2dpMediaSuspendCfm(req->device, media, a2dp_no_signalling_connection);
    }
    else if (media->status.conn_info.connection_state != avdtp_connection_connected)
    {
        a2dpMediaSuspendCfm(req->device, media, a2dp_no_media_connection);
    }
    else if (signalling->status.stream_state != avdtp_stream_streaming)
    {
        a2dpMediaSuspendCfm(req->device, media, a2dp_wrong_state);
    }
    else
    {
        if ( a2dpSendCommand(req->device, avdtp_suspend, NULL, AVDTP_NO_PAYLOAD) )  /* TODO: Support multiple streams */
        {
            a2dpSetStreamState(req->device, avdtp_stream_local_suspending);
        }
        else
        {
            a2dpStreamAbort(req->device);
            a2dpMediaSuspendCfm(req->device, media, a2dp_operation_fail);
        }
    }
}

void a2dpStreamReconfigure (const A2DP_INTERNAL_MEDIA_RECONFIGURE_REQ_T *req)
{
    signalling_channel *signalling = &((remote_device *)PanicNull(req->device))->signal_conn;
    media_channel *media= (media_channel *)PanicNull(req->media);
    
    if (signalling->status.connection_state != avdtp_connection_connected)
    {
        a2dpMediaReconfigureCfm(req->device, media, a2dp_no_signalling_connection);
    }
    else if (media->status.conn_info.connection_state != avdtp_connection_connected)
    {
        a2dpMediaReconfigureCfm(req->device, media, a2dp_no_media_connection);
    }
    else if (signalling->status.stream_state != avdtp_stream_open)
    {
        a2dpMediaReconfigureCfm(req->device, media, a2dp_wrong_state);
    }
    else
    {
        if ( a2dpSendCommand(req->device, avdtp_reconfigure, req->service_caps, req->service_caps_size) )  /* TODO: Support multiple streams */
        {
            /* Store reference to the reconfigure caps for later use */
            req->device->reconfig_caps = PanicUnlessMalloc(req->service_caps_size);
            memmove(req->device->reconfig_caps, req->service_caps, req->service_caps_size);
            req->device->reconfig_caps_size = req->service_caps_size;
            
            a2dpSetStreamState(req->device, avdtp_stream_reconfiguring);
        }
        else
        {
            a2dpStreamAbort(req->device);
            a2dpMediaReconfigureCfm(req->device, media, a2dp_operation_fail);
        }
    }
}


void a2dpStreamDelayReport (remote_device *device, uint16 delay)
{
    if ( (device->signal_conn.status.stream_state != avdtp_stream_configured) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_opening) &&
         (device->signal_conn.status.stream_state != avdtp_stream_remote_opening) &&
         (device->signal_conn.status.stream_state != avdtp_stream_open) &&
         (device->signal_conn.status.stream_state != avdtp_stream_streaming) &&
         (device->signal_conn.status.stream_state != avdtp_stream_local_starting) &&
         (device->signal_conn.status.stream_state != avdtp_stream_remote_starting) )
    {   /* Can only issue a delay report in an appropriate state */
        return;
    }
    
    if (device->signal_conn.status.connection_state == avdtp_connection_connected)
    {
        if ( !sendDelayReport(device, delay) )
        {
            setDelayReportPending(device, TRUE);
        }
    }
}


void a2dpStreamAbort (remote_device *device)
{
    if ( (device->signal_conn.status.stream_state == avdtp_stream_local_aborting) ||
         (device->signal_conn.status.stream_state == avdtp_stream_remote_aborting) )
    {   /* Already aborting */
        return;
    }

    if (device->signal_conn.status.connection_state == avdtp_connection_connected)
    {
        if ( a2dpSendCommand(device, avdtp_abort, NULL, AVDTP_NO_PAYLOAD) )
        {
            a2dpSetStreamState(device, avdtp_stream_local_aborting);
        }
        else
        {   /* Unable to issue Abort command, so just reset */
            a2dpDisconnectAllMedia(device);
            a2dpStreamReset(device);
        }
    }
}


void a2dpStreamReset (remote_device *device)
{
    sep_data_type *current_sep;
    
    /* Mark current SEP as not in use anymore */
    if ( (current_sep = (sep_data_type *)blockGetCurrent( (uint8)device->bitfields.device_id, data_block_sep_list ))!=NULL )
    {
        current_sep->in_use &= ~A2DP_SEP_IN_USE;
    }
        
    /* Reset the local and remote SEIDs */
    blockSetCurrent( (uint8)device->bitfields.device_id, data_block_sep_list, 0 );
    device->remote_sep.codec = AVDTP_MEDIA_CODEC_UNDEFINED;
    device->local_sep.codec = AVDTP_MEDIA_CODEC_UNDEFINED;

    /* Remove stream specific data */
    blockRemove( (uint8)device->bitfields.device_id, data_block_configured_service_caps );
    blockRemove( (uint8)device->bitfields.device_id, data_block_list_discovered_remote_seids );

    /* Cancel the watchdog timeout */
    (void) MessageCancelAll((Task)&a2dp->task, A2DP_INTERNAL_WATCHDOG_BASE + device->bitfields.device_id);

    /* Clear transaction logs */
    device->signal_conn.status.pending_issued_transaction = FALSE;
    device->signal_conn.status.pending_received_transaction = FALSE;
    device->signal_conn.connection.active.bitfields.issued_transaction_label = 0;
    device->signal_conn.connection.active.bitfields.received_transaction_label = 0;

    a2dpSetStreamState(device, avdtp_stream_idle);
}


#if 0
bool a2dpChangeParameters (remote_device *device)
{
    return FALSE;
}
#endif

#if 0
bool a2dpSecurityControl (remote_device *device)
{
    return FALSE;
}
#endif


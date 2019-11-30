/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
     Interface to handle Peer State machine to ensure PTS qualification for A2DP Sorce functionality
*/

#include "sink_peer_qualification.h"
#include "sink_a2dp.h"
#include "sink_debug.h"

#include <source.h>
#include <string.h>
#include <stdlib.h>
#include <vm.h>


#ifdef DEBUG_A2DP_QUALIFICATION
#define A2DP_QUALIFICATION_DEBUG(x) DEBUG(x)
#else
#define A2DP_QUALIFICATION_DEBUG(x) 
#endif

#define SERVICE_CAPS(caps) ((caps)[0])
#define CAPS_SIZE(caps)    ((caps)[1] + 2)

/*************************************************************************
NAME    
    findDelayReportServiceCaps
    
DESCRIPTION
    Utility function to find if the configured AG's Cap has delay reporting support.

**************************************************************************/
static uint16 findDelayReportServiceCaps (const uint8 *codec_caps, uint16 size_codec_caps)
{
    while (codec_caps && (size_codec_caps > 1))
    {
        uint16 size;

        if ((SERVICE_CAPS(codec_caps) < AVDTP_SERVICE_MEDIA_TRANSPORT) || (SERVICE_CAPS(codec_caps) > AVDTP_SERVICE_DELAY_REPORTING))
        {
            return 0;
        }

        if ((size = CAPS_SIZE(codec_caps)) > size_codec_caps)
        {
            return 0;
        }

        if (SERVICE_CAPS(codec_caps) == AVDTP_SERVICE_DELAY_REPORTING)
        {
            return CAPS_SIZE(codec_caps);
        }

        size_codec_caps -= size;
        codec_caps += size;
    }

    return 0;
}

/*************************************************************************
NAME    
    peerQualificationReplaceDelayReportServiceCaps
    
DESCRIPTION
    Utility function to replace PTS configured caps with AG's configured caps by removing delay 
    reporting, provided AG supports one.

**************************************************************************/
bool peerQualificationReplaceDelayReportServiceCaps (uint8 *dest_service_caps, uint16 *size_dest_service_caps, const uint8 *src_service_caps, uint16 size_src_service_caps)
{
    uint16 size;
    if (!src_service_caps || !size_src_service_caps || !peerTwsQualificationEnabled())
    {
        return FALSE;
    }
    else
    {
        if((size = findDelayReportServiceCaps(src_service_caps, size_src_service_caps)) != 0) 
        {
            if((dest_service_caps != NULL) && (size_dest_service_caps != NULL))
            {
                /* size of dest_service_caps is always allocated to size_src_service_caps */
                memset(dest_service_caps, 0, size_src_service_caps);
            	*size_dest_service_caps = size_src_service_caps - size;
                memcpy(dest_service_caps, src_service_caps, (size_src_service_caps -size));
            	return TRUE;
        	}
    	}
    }
    
    return FALSE;
}

/*************************************************************************
NAME    
    findMediaTransportServiceCaps
    
DESCRIPTION
    Utility function to find if the configured AG's Cap has media transport support.

**************************************************************************/
static uint16 findMediaTransportServiceCaps (const uint8 *codec_caps, uint16 size_codec_caps)
{
    while (codec_caps && (size_codec_caps > 1))
    {
        uint16 size;

        if ((SERVICE_CAPS(codec_caps) < AVDTP_SERVICE_MEDIA_TRANSPORT) || (SERVICE_CAPS(codec_caps) > AVDTP_SERVICE_DELAY_REPORTING))
        {
            return 0;
        }

        if ((size = CAPS_SIZE(codec_caps)) > size_codec_caps)
        {
            return 0;
        }

        if (SERVICE_CAPS(codec_caps) == AVDTP_SERVICE_MEDIA_TRANSPORT)
        {
            return CAPS_SIZE(codec_caps);
        }

        size_codec_caps -= size;
        codec_caps += size;
    }

    return 0;
}

/*************************************************************************
NAME    
    peerQualificationReplaceMediaTransportServiceCaps
    
DESCRIPTION
    Utility function to replace PTS configured caps with AG's configured caps by removing media 
    transport, provided AG supports one.

**************************************************************************/
static bool peerQualificationReplaceMediaTransportServiceCaps (uint8 *dest_service_caps, uint16 *size_dest_service_caps, const uint8 *src_service_caps, uint16 size_src_service_caps)
{
    uint16 size;
    if (!src_service_caps || !size_src_service_caps || !peerTwsQualificationEnabled())
    {
        return FALSE;
    }
    else
    {
        if((size = findMediaTransportServiceCaps(src_service_caps, size_src_service_caps)) != 0) 
        {
            if((dest_service_caps != NULL) && (size_dest_service_caps != NULL))
            {
                /* size of dest_service_caps is always allocated to size_src_service_caps */
                memset(dest_service_caps, 0, size_src_service_caps);
            	*size_dest_service_caps = size_src_service_caps - size;
                memcpy(dest_service_caps, (src_service_caps+size), (size_src_service_caps -size));
            	return TRUE;
        	}
    	}
    }
    
    return FALSE;
}

/*************************************************************************
NAME    
    a2dpIssueSinkReconfigureRequest
    
DESCRIPTION
    Issues a request to reconfigure the relay channel to the currently connected Source

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssueSinkReconfigureRequest (void)
{
    uint16 index;
    a2dp_index_t a2dp_index;
    a2dp_codec_settings* codec_settings = NULL;
    uint16 size_dest_service_caps = 0;
    uint8 *find_media_transport = NULL;
    uint8 *config_caps = NULL;
    uint16 size_of_conf_caps = 0;
    uint16 size_of_transport = 0;
    bool result = FALSE;

    if (a2dpGetConnectedDeviceIndex(&index))
    {
        A2DP_QUALIFICATION_DEBUG(("a2dpIssueSinkReconfigureRequest index=%u\n",index));
        if (findCurrentA2dpSource( &a2dp_index ))
        {
            codec_settings = A2dpCodecGetSettings(getA2dpLinkDataDeviceId(a2dp_index), getA2dpLinkDataStreamId(a2dp_index));
        }

        if(codec_settings)
        {
            /* Allocate the memory for the dest_service_caps, atleast equal to the configured caps */
            uint8 *reconfig_caps = PanicUnlessMalloc(codec_settings->size_configured_codec_caps);
            setA2dpReconfigServiceCaps(reconfig_caps);
            
            find_media_transport = PanicUnlessMalloc(codec_settings->size_configured_codec_caps);
            config_caps = codec_settings->configured_codec_caps;
            size_of_conf_caps = codec_settings->size_configured_codec_caps;

             /* If at all there is media_transport in the configured caps, then remove it and copy it to dest_caps */
             if(peerQualificationReplaceMediaTransportServiceCaps(find_media_transport, &size_of_transport, config_caps, size_of_conf_caps))
             {
                memcpy(getA2dpReconfigServiceCaps(), find_media_transport, size_of_transport);
                size_dest_service_caps = size_of_transport;
                config_caps = find_media_transport;
                size_of_conf_caps = size_of_transport;
             }
             /* Remove any delay_reporting, and update the dest_caps */
             peerQualificationReplaceDelayReportServiceCaps(getA2dpReconfigServiceCaps(), &size_dest_service_caps, config_caps, size_of_conf_caps);
            
            free(find_media_transport);
            find_media_transport = NULL;

            /* If the configured caps has no media_transport or delay_reporting use it as is for reconfiguration */
            if(size_dest_service_caps == 0)
            {
                free(getA2dpReconfigServiceCaps());
                setA2dpReconfigServiceCaps(NULL);
                result = A2dpMediaReconfigureRequest(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index), codec_settings->size_configured_codec_caps, codec_settings->configured_codec_caps);
            }
            else
            {
                /* reconfig_caps will be freed in response to A2DP_MEDIA_RECONFIGURE_CFM */
                result = A2dpMediaReconfigureRequest(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index), size_dest_service_caps, getA2dpReconfigServiceCaps());
            }
            free(codec_settings);
        }
    }

    return result;
}

/*************************************************************************
NAME    
    a2dpGetConnectedDeviceIndex
    
DESCRIPTION
    Attempts to obtain the index into a2dp_link_data structure for a currently 
    connected source device.
    
RETURNS
    TRUE if successful, FALSE otherwise
    
**************************************************************************/
bool a2dpGetConnectedDeviceIndex (uint16* index)
{
    uint8 i;
    
    /* go through A2dp connection looking for device_id match */
    for_all_a2dp(i)
    {
        /* if the a2dp link is connected check its device id */
        if(getA2dpStatusFlag(CONNECTED, i))
        {
            /* if a device_id match is found return its value and a
               status of successful match found */
            *index = i;
            return TRUE;
        }
    }
    /* no matches found so return not successful */    
    return FALSE;
}

/*************************************************************************
NAME    
    a2dpIssueSuspendRequest
    
DESCRIPTIONDESCRIPTION
    Issues a request to suspend the relay channel to the currently connected Source

RETURNS
    
**************************************************************************/
void a2dpIssueSuspendRequest (void)
{
    uint16 index;

    if (a2dpGetConnectedDeviceIndex(&index))
    {
        A2DP_QUALIFICATION_DEBUG(("a2dpIssueSuspendRequest index=%u\n",index));

        /* Ensure suspend state is set as streaming has now ceased */
        a2dpSetSuspendState(index, a2dp_local_suspended);

        A2dpMediaSuspendRequest(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index));
    }
}

#ifdef ENABLE_PEER

/*************************************************************************
NAME    
    a2dpIssuePeerReconfigureRequest
    
DESCRIPTION
    Issues a request to reconfigure the relay channel to the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerReconfigureRequest (void)
{
    uint16 peer_id;
    a2dp_index_t a2dp_index;
    a2dp_codec_settings* codec_settings = NULL;
    uint16 size_dest_service_caps = 0;
    uint8 *find_media_transport = NULL;
    uint8 *config_caps = NULL;
    uint16 size_of_conf_caps = 0;
    uint16 size_of_transport = 0;
    bool result = FALSE;

    if (a2dpGetPeerIndex(&peer_id))
    {
        A2DP_QUALIFICATION_DEBUG(("issuePeerReconfigureRequest peer=%u\n",peer_id));
        if (findCurrentA2dpSource( &a2dp_index ))
        {
            codec_settings = A2dpCodecGetSettings(getA2dpLinkDataDeviceId(a2dp_index), getA2dpLinkDataStreamId(a2dp_index));
        }

        if(codec_settings)
        {
            /* Allocate the memory for the dest_service_caps, atleast equal to the configured caps */
            uint8 *reconfig_caps = PanicUnlessMalloc(codec_settings->size_configured_codec_caps);
            setA2dpReconfigServiceCaps(reconfig_caps);
            
            find_media_transport = PanicUnlessMalloc(codec_settings->size_configured_codec_caps);
            config_caps = codec_settings->configured_codec_caps;
            size_of_conf_caps = codec_settings->size_configured_codec_caps;

             /* If at all there is media_transport in the configured caps, then remove it and copy it to dest_caps */
             if(peerQualificationReplaceMediaTransportServiceCaps(find_media_transport, &size_of_transport, config_caps, size_of_conf_caps))
             {
                memcpy(getA2dpReconfigServiceCaps(), find_media_transport, size_of_transport);
                size_dest_service_caps = size_of_transport;
                config_caps = find_media_transport;
                size_of_conf_caps = size_of_transport;
             }
             /* Remove any delay_reporting, and update the dest_caps */
             peerQualificationReplaceDelayReportServiceCaps(getA2dpReconfigServiceCaps(), &size_dest_service_caps, config_caps, size_of_conf_caps);
            
            free(find_media_transport);
            find_media_transport = NULL;

            /* If the configured caps has no media_transport or delay_reporting use it as is for reconfiguration */
            if(size_dest_service_caps == 0)
            {
                free(getA2dpReconfigServiceCaps());
                setA2dpReconfigServiceCaps(NULL);
                result = A2dpMediaReconfigureRequest(getA2dpLinkDataDeviceId(peer_id), getA2dpLinkDataStreamId(peer_id), codec_settings->size_configured_codec_caps, codec_settings->configured_codec_caps);
            }
            else
            {
                /* reconfig_caps will be freed in response to A2DP_MEDIA_RECONFIGURE_CFM */
                result = A2dpMediaReconfigureRequest(getA2dpLinkDataDeviceId(peer_id), getA2dpLinkDataStreamId(peer_id), size_dest_service_caps, getA2dpReconfigServiceCaps());
            }
            free(codec_settings);
        }
    }
    
    return result;
}

/*************************************************************************
NAME    
    peerQualificationAdvanceRelayState
    
DESCRIPTION
    This function is to bypass Peer state machine so that we can handle PTS request/reponse.

**************************************************************************/
bool peerQualificationAdvanceRelayState (RelayEvent relay_event)
{
    bool retVal = FALSE;
    if(!peerTwsQualificationEnabled())
    {
        return retVal;
    }
    
    switch (relay_event)
    {
        case RELAY_EVENT_CONNECTED:
            A2DP_QUALIFICATION_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_CONNECTED\n"));
            updateCurrentState( RELAY_STATE_IDLE );
            /* for some TC's PTS expects SRC to open the channel.
                With this flag, ADK shall trigger opening of the media channel,
                provided AG is already connected and is streaming */
            if(peerOpenRequestForQualificationEnabled())
            {
                if (a2dpIssuePeerOpenRequest())
                {
                    A2DP_QUALIFICATION_DEBUG(("PEER: Issue PEER OPEN REQ\n"));
                    updateCurrentState( RELAY_STATE_OPENING );
                }
            }
            retVal = TRUE;
            break;
            
        case RELAY_EVENT_OPEN:
            A2DP_QUALIFICATION_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_OPEN\n"));
            updateCurrentState( RELAY_STATE_IDLE );
            retVal = TRUE;
            break;
            
        
        case RELAY_EVENT_SUSPENDED:
            A2DP_QUALIFICATION_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_SUSPENDED\n"));
            updateCurrentState( RELAY_STATE_OPEN );
            if (peerTargetRelayState() < peerCurrentRelayState())
            {   /* reconfigure relay stream */
                if(a2dpIssuePeerReconfigureRequest())
                {
                    A2DP_QUALIFICATION_DEBUG(("PEER: Issue PEER RECONFIGURE REQ\n"));
                    updateCurrentState(RELAY_STATE_IDLE);
                }
            }
            retVal = TRUE;
            break;

        case RELAY_EVENT_NOT_OPENED:
        {
            retVal = TRUE;
        }
        break;
        default:
            retVal = FALSE;
            break;
    }
    return retVal;
}

/*************************************************************************
NAME    
    handlePeerQualificationReconfigureCfm
    
DESCRIPTION
    handle a successful confirm of a media channel reconfiguration

RETURNS
    
**************************************************************************/
void handlePeerQualificationReconfigureCfm(uint8 DeviceId, uint8 StreamId, a2dp_status_code status)
{
    UNUSED(StreamId);

    if(!peerTwsQualificationEnabled())
        return;
        
    if(getA2dpReconfigServiceCaps() != NULL)
    {
        free(getA2dpReconfigServiceCaps());
        setA2dpReconfigServiceCaps(NULL);
    }
    
    /* ensure successful confirm status */
    if (status == a2dp_success)
    {
        uint16 Id;
        bdaddr bd_addr;

        /*Get the A2DP index from the BD Address corresponding to the DeviceId */
        if(A2dpDeviceGetBdaddr(DeviceId, &bd_addr) && getA2dpIndexFromBdaddr(&bd_addr , &Id))
        {
            if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
            {
                peerAdvanceRelayState(RELAY_EVENT_START);
            }
        }
    }
   else
   {
        uint16 Id;
        if (getA2dpIndex(DeviceId, &Id))
        {
            if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
            {
                peerAdvanceRelayState(RELAY_EVENT_NOT_OPENED);
            }
        }
    }
}

/*************************************************************************
NAME    
    handlePeerQualificationEnablePeerOpen
    
DESCRIPTION
    Utility function to toggle the "Enable Peer Open" flag. This flag is used to send stream open to
    PTS. For some TC's PTS expects the SRC to open the channel.
    
**************************************************************************/
void handlePeerQualificationEnablePeerOpen(void)
{
    if(peerTwsQualificationEnabled())
    {
        if(peerOpenRequestForQualificationEnabled())
        {
            peerSetTwsQualEnablePeerOpen(FALSE);
        }
        else
        {
            peerSetTwsQualEnablePeerOpen(TRUE);
            /*Unlock the peer state machine to initiate a2dp media connection*/
            peerSetUnlockPeerStateMachine(1);
        }
    }
}

#endif /* ENABLE_PEER */


/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ba_receiver.c

DESCRIPTION
    Implementation for Broadcast Audio Receiver
    
NOTES

*/

#include "sink_ba_receiver.h"
#include "sink_debug.h"
#include "sink_utils.h"
#include "sink_main_task.h"
#include "sink_gatt.h"
#include "sink_statemanager.h"
#include "sink_configmanager.h"
#include "sink_ble.h"
#include "sink_ba_ble_gap.h"
#include "sink_a2dp.h"
#include "sink_malloc_debug.h"
#include "sink_devicemanager.h"

#include <stdlib.h>
#include <vmtypes.h>
#include <scm.h>
#include <csb.h>
#include <audio.h>
#include <broadcast_stream_service_record.h>
#include <gatt_broadcast_server_uuids.h>
#include <audio_plugin_music_params.h>

#ifdef ENABLE_BROADCAST_AUDIO

#include "sink_broadcast_audio_config_def.h"
#include <broadcast_context.h>
#include <broadcast_cmd.h>

#include <audio_plugin_music_variants.h>

#ifdef DEBUG_BA_RECEIVER
#define DEBUG_RECEIVER(x) DEBUG(x)
#else
#define DEBUG_RECEIVER(x) 
#endif

#define RECEIVE_SUPERVISION_TIMEOUT (0x200)

#define SYNC_SCAN_TIMEOUT           (0x4000)
#define SYNC_SCAN_WINDOW            (0x200)
#define SYNC_SCAN_INTERVAL          (0x202)


#define CHECK_BA_UUID(msb, lsb) (((msb == (UUID_BROADCAST_SERVICE >> 8)) && \
                                                     (lsb == (UUID_BROADCAST_SERVICE & 0XFF))) || \
                                                     ((msb == (UUID_BROADCAST_SERVICE_2 >> 8)) && \
                                                     (lsb == (UUID_BROADCAST_SERVICE_2 & 0XFF))))

#define BA_RECEIVER_SAMPLE_RATE (44100)

/* Macro to check whether existing stored broadcaster did not connect any BR/EDR profiles */
#define SINK_ATTRIBUTES_NOT_AVAILABLE(attributes) (attributes.profiles == sink_none)

/* Macro to check whether existing stored broadcaster did not connect any LE services */
#define GATT_ATTRIBUTES_NOT_AVAILABLE(attributes) ((attributes.gatt_client.gatt == 0x0) && \
                                                     (attributes.gatt_client.battery_local == 0x0) && \
                                                     (attributes.gatt_client.battery_remote== 0x0) && \
                                                     (attributes.gatt_client.battery_peer == 0x0 ) && \
                                                     (attributes.gatt_client.heart_rate == 0x0))

/* \brief Application data */
typedef struct _receiverData_t {
    /* Receiver task. */
    Task task;
    /* The broadcaster's (i.e. the source of audio) address. */
    bdaddr   srcAddr;
    /* The local volume offset. */
    int16    volume;
    /* The broadcast system-wide volume. */
    int16    global_volume;
    /* The actual volume. */
    uint16   actual_volume;
    /* Parameters for sync train discovery */
    receiver_sync_params* sync_params;
    /* Remember the connection ID, so we can disconnect after
        using the Broadcast service for association */
    uint16 broadcast_server_cid;

    uint8* bssr;
}receiverData_t;

/* Reference to receiver for use in module */
static receiverData_t *receiver = NULL;
#define RECEIVER receiver

/*-------------------------Static Function Definitions------------------------*/

/***************************************************************************
NAME
    receiverGetVolume
 
DESCRIPTION
    Utility function to get the Receiver Volume (in Steps) from PS store

PARAMS
    void

RETURNS
    int16 volume
*/
static int16 receiverGetVolume(void)
{
    receiver_volume_config_def_t* rw_config_data = NULL;
    uint16 volume = 0;
    if (configManagerGetReadOnlyConfig(RECEIVER_VOLUME_CONFIG_BLK_ID, (const void **)&rw_config_data))
    {
       volume = rw_config_data->receiver_default_local_volume;
       configManagerReleaseConfig(RECEIVER_VOLUME_CONFIG_BLK_ID);
    }
    return volume;
}

/***************************************************************************
NAME
    receiverSetVolume
 
DESCRIPTION
    Utility function to set the Receiver Volume in PS store

PARAMS
    int16 volume

RETURNS
    void
*/
static void receiverSetVolume(int16 volume)
{
    receiver_volume_config_def_t* rw_config_data = NULL;
    if (configManagerGetWriteableConfig(RECEIVER_VOLUME_CONFIG_BLK_ID, (void **)&rw_config_data, 0))
    {
        /* convert the volume into steps before writting into PS Key */
        rw_config_data->receiver_default_local_volume = (volume/RECEIVER_VOLUME_STEP_SIZE);
        configManagerUpdateWriteableConfig(RECEIVER_VOLUME_CONFIG_BLK_ID);
    }
}

/***************************************************************************
NAME
    receiverLoadStoredBroadcasterAddress
 
DESCRIPTION
    Utility function to retrieve the Broadcaster BD Address from PS store

PARAMS
    void

RETURNS
    bool TRUE if retrieved BD Address is Valid else FALSE 
*/
static bool receiverLoadStoredBroadcasterAddress(void)
{
    receiver_master_address_config_def_t* rw_config_data = NULL;
    bool result = FALSE;

    if(configManagerGetReadOnlyConfig(RECEIVER_MASTER_ADDRESS_CONFIG_BLK_ID, (const void **)&rw_config_data))
    {
       RECEIVER->srcAddr.lap = (((uint32)rw_config_data->broadcaster_bd_addr.lap_msb<< 16) | ((uint32) (rw_config_data->broadcaster_bd_addr.lap_lsb) & 0x0000FFFFUL));
       RECEIVER->srcAddr.nap = rw_config_data->broadcaster_bd_addr.nap;
       RECEIVER->srcAddr.uap = rw_config_data->broadcaster_bd_addr.uap;
       configManagerReleaseConfig(RECEIVER_MASTER_ADDRESS_CONFIG_BLK_ID);
    }

    /* In config refactor we always initiailze all the config variables so below check is to identify PS BD
    Address is Valid */
    if((RECEIVER->srcAddr.lap != 0x0000) && (RECEIVER->srcAddr.nap != 0x0000) && (RECEIVER->srcAddr.uap != 0x0000))
    {
        result = TRUE;
    }
    return result;
}

/***************************************************************************
NAME
    receiverConvertToBdAddress
 
DESCRIPTION
    Utility function to convert passed receiver_master_address_config_def_t into bdaddr

PARAMS
    bdaddr* addr
    receiver_master_address_config_def_t* data
RETURNS
    void
*/
static void receiverConvertToBdAddress(bdaddr* addr, receiver_master_address_config_def_t* data)
{
    if(addr)
    {
       addr->lap = (((uint32)data->broadcaster_bd_addr.lap_msb<< 16) | 
                        ((uint32) (data->broadcaster_bd_addr.lap_lsb) & 0x0000FFFFUL));
       addr->nap = data->broadcaster_bd_addr.nap;
       addr->uap = data->broadcaster_bd_addr.uap;
    }
}

/***************************************************************************
NAME
    receiverCheckIfBroadcasterCanBeRemoved
 
DESCRIPTION
    Utility function to check whether passed broadcaster BD address was used to connect any BR/EDR profiles or LE
    services. If for this BD address we find any BR/EDR or LE attributes we will maintain it in PDL otherwise clear it.

PARAMS
    bdaddr* addr

RETURNS
    bool returns TRUE if passed master BD address was removed from PS store else FALSE
*/
static bool receiverCheckIfBroadcasterCanBeRemoved(bdaddr* addr)
{
    bool result = FALSE;

    if(addr)
    {
        sink_attributes attributes;

        DEBUG_RECEIVER(("Receiver: Check whether existing stored Broadcaster was solely used for purpose of broadcasting\n"));

        deviceManagerClearAttributes(&attributes); 

        if(deviceManagerGetAttributes(&attributes, (const bdaddr *) addr) &&
           SINK_ATTRIBUTES_NOT_AVAILABLE(attributes) && GATT_ATTRIBUTES_NOT_AVAILABLE(attributes))
        {
            DEBUG_RECEIVER(("Receiver: The existing stored Broadcaster was solely used for purpose of broadcasting\n"));
            DEBUG_RECEIVER(("Hence can be removed from PDL\n"));
            ConnectionSmDeleteAuthDeviceReq(TYPED_BDADDR_PUBLIC, addr);
            result = TRUE;
        }
        else
        {
            DEBUG_RECEIVER(("Receiver: The existing stored Broadcaster was used to connect BR/EDR profiles or LE services too\n"))
            DEBUG_RECEIVER(("So will continue to remain PDL\n"));
        }
    }
    return result;
}

/***************************************************************************
NAME
    receiverSetBroadcasterAddress
 
DESCRIPTION
    Utility function to set the Broadcaster BD Address in PS store

PARAMS
    addr

RETURNS
    bool TRUE if the passed pointer is Valid & BD Address is stored in PS store successfully else FALSE 
*/
static bool receiverSetBroadcasterAddress(bdaddr* addr)
{
    bool result = FALSE;
    receiver_master_address_config_def_t* rw_config_data = NULL;
    if (configManagerGetWriteableConfig(RECEIVER_MASTER_ADDRESS_CONFIG_BLK_ID, (void **)&rw_config_data, 0))
    {
       if(addr)
       {
            bdaddr temp_addr;
            receiverConvertToBdAddress(&temp_addr, rw_config_data);

            if(BdaddrIsSame((const bdaddr *)addr, (const bdaddr *) &temp_addr))
            {
                DEBUG_RECEIVER(("Receiver: Same Broadcaster Address already exists in PS Store. PS write unncessary\n"));
                configManagerReleaseConfig(RECEIVER_MASTER_ADDRESS_CONFIG_BLK_ID);
                return TRUE;
            }
            else
            {
                receiverCheckIfBroadcasterCanBeRemoved(&temp_addr);
                DEBUG_RECEIVER(("Update new Broadcaster Address in PS Store\n"));
                rw_config_data->broadcaster_bd_addr.lap_lsb = (uint16)(addr->lap & 0x0000FFFFUL);
                rw_config_data->broadcaster_bd_addr.lap_msb = (uint16)((addr->lap>>16) & 0x0000FFFFUL);
                rw_config_data->broadcaster_bd_addr.nap = addr->nap;
                rw_config_data->broadcaster_bd_addr.uap = addr->uap;
                result = TRUE;
            }
       }
       configManagerUpdateWriteableConfig(RECEIVER_MASTER_ADDRESS_CONFIG_BLK_ID);
    }
    return result;
}

/***************************************************************************
NAME
    receiverResetBroadcasterAddress
 
DESCRIPTION
    Utility function to Reset the Broadcaster BD Address in PS store

PARAMS
    void

RETURNS
    void
*/
static void receiverResetBroadcasterAddress(void)
{
    receiver_master_address_config_def_t* rw_config_data = NULL;
    if(configManagerGetWriteableConfig(RECEIVER_MASTER_ADDRESS_CONFIG_BLK_ID, (void **)&rw_config_data, 0))
    {
        DEBUG_RECEIVER(("Reset the Broadcaster BD Address in PS Store\n"));
        if(rw_config_data)
            memset(rw_config_data, 0x00, sizeof(*rw_config_data));
        configManagerUpdateWriteableConfig(RECEIVER_MASTER_ADDRESS_CONFIG_BLK_ID);
    }
}

/***************************************************************************
NAME
    receiverGetBssr
 
DESCRIPTION
    Utility function to get Broadcaster Stream Service Records from PS Store

PARAMS
    data

RETURNS
    uint16 length of Broadcaster Stream Service Records Data
*/
static uint16 receiverGetBssr(uint8* data)
{
    receiver_bssr_config_def_t* rw_config_data = NULL;
    uint16 size = 0;
    size = configManagerGetReadOnlyConfig(RECEIVER_BSSR_CONFIG_BLK_ID, (const void **)&rw_config_data);
    if (size)
    {
       if(data)
       {
           memcpy(data,rw_config_data,size);
       }
       configManagerReleaseConfig(RECEIVER_BSSR_CONFIG_BLK_ID);
    }
    return size;
}

/***************************************************************************
NAME
    receiverSetBssr
 
DESCRIPTION
    Utility function to set Broadcaster Stream Service Records in PS Store

PARAMS
    data BSSR data
    size size of BSSR data
RETURNS
    bool TRUE if passed data pointer is Valid & data is stored in PS store successfully else FALSE
*/
static bool receiverSetBssr(uint8 *data, uint16 size)
{
    bool result = FALSE;
    receiver_bssr_config_def_t* rw_config_data = NULL;
    if (configManagerGetWriteableConfig(RECEIVER_BSSR_CONFIG_BLK_ID, (void **)&rw_config_data, size))
    {
        if(data)
        {
            if(!memcmp(rw_config_data, data, size))
            {
                DEBUG_RECEIVER(("BSSR matches with PS Store BSSR data. PS write unncessary\n"));
                configManagerReleaseConfig(RECEIVER_BSSR_CONFIG_BLK_ID);
                return TRUE;
            }
            DEBUG_RECEIVER(("Update PS Store with new BSSR data\n"));
            memcpy(rw_config_data, data, size);
            result = TRUE;
        }
        configManagerUpdateWriteableConfig(RECEIVER_BSSR_CONFIG_BLK_ID);
    }
    return result;
}

/***************************************************************************
NAME
    receiverConfigLoadKey
 
DESCRIPTION
    Utility function to load the security key.

PARAMS
    key_dest Destination to copy Fixed IV to
    key_length length of Key
    
RETURNS
    bool TRUE if the loaded security is valid else FALSE.
*/
static bool receiverConfigLoadKey(uint16* key_dest)
{
    receiver_writeable_keys_config_def_t* rw_config_data = NULL;
    bool ret_val = FALSE;
    
    if (configManagerGetReadOnlyConfig(RECEIVER_WRITEABLE_KEYS_CONFIG_BLK_ID, (const void **)&rw_config_data))
    {
        /* if we loaded something, but the first words is 0x0000, return FALSE
        this is just initialised data not a real key */
        if (rw_config_data->receiver_keys.sec[0] != 0x0000)
        {
            /* get data from PS and copy section we need to destination */
            memmove(key_dest, &rw_config_data->receiver_keys.sec, sizeof(rw_config_data->receiver_keys.sec));
            ret_val = TRUE;
        }
        configManagerReleaseConfig(RECEIVER_WRITEABLE_KEYS_CONFIG_BLK_ID);
    }
    return ret_val;
}

/***************************************************************************
NAME
    receiverConfigLoadFixedIv
 
DESCRIPTION
    Utility function to Load the fixed IV.

PARAMS
    fixed_iv_dest Destination to copy Fixed IV to
    
RETURNS
    bool TRUE if the loaded Fixed IV is valid else FALSE.
*/
static bool receiverConfigLoadFixedIv(uint16* fixed_iv_dest)
{
    receiver_writeable_keys_config_def_t* rw_config_data = NULL;
    bool ret_val = FALSE;

    if (configManagerGetReadOnlyConfig(RECEIVER_WRITEABLE_KEYS_CONFIG_BLK_ID, (const void **)&rw_config_data))
    {
        /* if we loaded something, but the first words is 0x0000, return FALSE
        this is just initialised data not a real fixed IV */
        if (rw_config_data->receiver_keys.fiv[0] != 0x0000)
        {
            /* get data from PS and copy section we need to destination */
            memmove(fixed_iv_dest, &rw_config_data->receiver_keys.fiv, sizeof(rw_config_data->receiver_keys.fiv));
            ret_val = TRUE;
        }
        configManagerReleaseConfig(RECEIVER_WRITEABLE_KEYS_CONFIG_BLK_ID);
    }
    return ret_val;
}

/***************************************************************************
NAME
    receiverConfigLoadKey
 
DESCRIPTION
    Utility function to load the Receiver Configuration like encryption & fixed IV

PARAMS
    void
RETURNS
    void
*/
static void receiverLoadConfigKey(void)
{
    broadcast_encr_config_t encr;

    memset(&encr, 0x0,sizeof(encr));

    if (!receiverConfigLoadFixedIv(&encr.fixed_iv[0]))
    {
        /* must have a FixedIV in PS */
        Panic();
    }

    /* try and load key and invariant IV from PS */
    receiverConfigLoadKey(&encr.seckey[0]);
    BroadcastContextSetEncryptionConfig(&encr);
}

/***************************************************************************
NAME
    getReceiverPlugin
 
DESCRIPTION
    Utility function to get Receiver audio plugin
PARAMS
    void
    
RETURNS
    Task
*/
static Task getReceiverPlugin(void)
{
    return (Task)&csr_ba_receiver_decoder_plugin.data;
}

/***************************************************************************
NAME
    baPopulateConnectParameters
 
DESCRIPTION
    Utility function to set the audio connect parameters for the
    broadcast receiver plugin.

PARAMS
    connect_parameters Audio connect parameters to populate.
    
RETURNS
    bool
*/
bool baPopulateConnectParameters(audio_connect_parameters *connect_parameters)
{
    A2dpPluginConnectParams *audioConnectParams = getAudioPluginConnectParams();

    /* EQ mode and Audio enhancements */
    audioConnectParams->mode_params = getAudioPluginModeParams();

    connect_parameters->mode = AUDIO_MODE_CONNECTED;
    connect_parameters->audio_plugin = getReceiverPlugin();
    connect_parameters->rate = BA_RECEIVER_SAMPLE_RATE;
    connect_parameters->params = audioConnectParams;
    connect_parameters->app_task = &theSink.task;
    connect_parameters->volume = 0x0a;/* TODO */
    connect_parameters->audio_sink = StreamSinkFromSource(BroadcastContextGetReceiverSource());

    DEBUG_RECEIVER(("BA: populating rx_source %p rx_sink %p\n",
                     (void *)BroadcastContextGetReceiverSource(),
                     (void *)StreamSinkFromSource(BroadcastContextGetReceiverSource())));

    return TRUE;
}

/***************************************************************************
NAME
    receiverDisconnectAudio
 
DESCRIPTION
    Utility function to Disconnect BA Receiver Audio

    This utility function posts internal reciever audio disconnect
    message when plugin is in not in loading state. This avoids a situation
    where receiver source is set to NULL while reciever plugin was in loading 
    state which results into panic

PARAMS
    void

RETURNS
    void
*/
static void receiverDisconnectAudio(Task task)
{
    MessageSendConditionallyOnTask (task, RECEIVER_INTERNAL_MSG_AUDIO_DISCONNECT, NULL, AudioBusyPtr());
}

/***************************************************************************
NAME
    receiverHandleBlStartRecvCfm
 
DESCRIPTION
    Utility function to handle broadcast library BROADCAST_START_RECEIVER_CFM messages

PARAMS
    task Handler for Receiver Broadcast related Messages
    cfm BROADCAST_START_RECEIVER_CFM_T Message
    
RETURNS
    void
*/
static void receiverHandleBlStartRecvCfm(Task task, BROADCAST_START_RECEIVER_CFM_T* cfm)
{
    UNUSED(task);

    if (cfm->status == broadcast_success)
    {
        if (cfm->mode == broadcast_mode_auto)
        {
            uint16 new_bssr_length = 0;
            broadcast_encr_config_t* encr = NULL;

            /* Notify main task that CSB stream is started on receiver */
            MessageSend(sinkGetMainTask(), EventSysBaRxCsbStart, 0);

            encr = BroadcastContextGetEncryptionConfig();

            DEBUG_RECEIVER(("Receiver: BROADCAST_START_RX_CFM AUTO: setup streams, source id=%x\n", (unsigned)(cfm->source)));
            /* Record the reciever source */
            BroadcastContextSetReceiverSource(cfm->source);
            new_bssr_length = receiverGetBssr(NULL);

            if(!RECEIVER->bssr)
            {
                /* For very first time allocate memroy for BSSR data */
                RECEIVER->bssr = mallocPanic(new_bssr_length);
            }
            else
            {
                uint16 bssr_length = BroadcastContextGetBssrConfigLength();
                if(bssr_length != new_bssr_length)
                {
                    void *new_bssr = NULL;
                    /* If memory for BSSR data is already allocated then better to realloc
                    with new size. This is required in case new broadcaster has different BSSR
                    data length */
                    DEBUG_RECEIVER(("Receiver: Realloc BSSR data of new size\n"));
                    new_bssr = realloc(RECEIVER->bssr, new_bssr_length);

                    if(NULL == new_bssr)
                    {
                        DEBUG_RECEIVER(("Receiver: Realloc failed for BSSR data allocation\n"));
                        Panic();
                    }
                    RECEIVER->bssr = new_bssr;
                }
                else
                {
                    DEBUG_RECEIVER(("Receiver: No need to realloc BSSR data with new size as it is same size as previous allocation\n"));
                }
            }

            receiverGetBssr(RECEIVER->bssr);
            BroadcastContextSetBssrConfig(RECEIVER->bssr, new_bssr_length);
            if(encr)
            {
                receiverConfigLoadKey(&encr->seckey[0]);
                receiverConfigLoadFixedIv(&encr->fixed_iv[0]);
            }

            audioUpdateAudioRouting();
        }
        else if(cfm->mode == broadcast_mode_synctrain_only)
        {
            uint16 i;
            BROADCAST *ba_receiver;
            DEBUG_RECEIVER(("Receiver: BROADCAST_START_RX_CFM STO Instant:%08lx ", cfm->csb_params.next_csb_clock));
            for (i=0;i<10;i++)
            {
                DEBUG_RECEIVER(("%02x ", cfm->csb_params.afh_map[i]));
            }
            DEBUG_RECEIVER(("\n"));

            /* First get the handle */
            ba_receiver = BroadcastContextGetBroadcastHandle();
            CsbReceiverSetAfhMap(&RECEIVER->srcAddr, BroadcastGetLtAddr(ba_receiver), &cfm->csb_params.afh_map, cfm->csb_params.next_csb_clock);
        }
        else
        {
            DEBUG_RECEIVER(("Receiver: BROADCAST_START_RX_CFM BO: doing nothing atm\n"));
        }
    }
    else
    {
        /* There is a race condition with start/stop in the broadcast library
           that means we may get a stray BROADCAST_START_RX_CFM after calling
           BroadcastStopReceiver.
           Don't try to re-start the broadcast receiver if we are powered off. */
        if (deviceLimbo != stateManagerGetState()
            && cfm->status != broadcast_bad_param)
        {
            /* Only re-start if we get an error that isn't broadcast_bad_param.
               Reason: broadcast_bad_param means we tried to start the receiver
               in broadcast_mode_auto but it was already in that mode. */

            DEBUG_RECEIVER(("Receiver: BROADCAST_START_RX_CFM FAILED: auto start again mode 0x%x status 0x%x\n", 
                             cfm->mode, cfm->status));

            /* try and find the broadcast again */
            BroadcastStartReceiver(cfm->broadcast, broadcast_mode_auto, RECEIVE_SUPERVISION_TIMEOUT);
        }
    }
}

/***************************************************************************
NAME
    receiverHandleBlConfigureCfm
 
DESCRIPTION
    Utility function to handle broadcast library BROADCAST_CONFIGURE_CFM messages
    
PARAMS
    task Handler for Receiver Broadcast related Messages
    cfm BROADCAST_CONFIGURE_CFM_T Message
    
RETURNS
    void
*/
static void receiverHandleBlConfigureCfm(Task task, BROADCAST_CONFIGURE_CFM_T* cfm)
{
    UNUSED(task);

    if (cfm->status == broadcast_success)
    {
        DEBUG_RECEIVER(("Receiver: BROADCAST_CONFIG_CFM: Receiver configured. Start receiver auto\n"));
        BroadcastStartReceiver(cfm->broadcast, broadcast_mode_auto, RECEIVE_SUPERVISION_TIMEOUT);
    }
}

/***************************************************************************
NAME
    receiverHandleBlStopRecvCfm
 
DESCRIPTION
    Utility function to handle broadcast library BROADCAST_STOP_RECEIVER_CFM messages
    
PARAMS
    task Handler for Receiver Broadcast related Messages
    cfm BROADCAST_STOP_RECEIVER_CFM_T
    
RETURNS
    void
*/
static void receiverHandleBlStopRecvCfm(Task task, BROADCAST_STOP_RECEIVER_CFM_T* cfm)
{
    UNUSED(task);

    DEBUG_RECEIVER(("Receiver: BROADCAST_STOP_RX_CFM status 0x%x\n", cfm->status));

    if (cfm->status == broadcast_success)
    {
        DEBUG_RECEIVER(("Receiver: BROADCAST_STOP_RX_CFM: destroy streams\n"));
        BroadcastContextSetInvalidVariantIV();
        /* Disconnect the BA audio */
        receiverDisconnectAudio(task);
    }
    baBroadcastlibStoppedCfm();
}

/***************************************************************************
NAME
    receiverHandleBlStopRecvCfm
 
DESCRIPTION
    Utility function to Setup the broadcast library receiver for finding the sync train
    
PARAMS
    void
    
RETURNS
    void
*/
static void receiverconfigureReceiver(void)
{
    RECEIVER->sync_params->bd_addr = RECEIVER->srcAddr;
    RECEIVER->sync_params->sync_scan_timeout = SYNC_SCAN_TIMEOUT;
    RECEIVER->sync_params->sync_scan_window = SYNC_SCAN_WINDOW;
    RECEIVER->sync_params->sync_scan_interval = SYNC_SCAN_INTERVAL;
    BroadcastConfigureReceiver(BroadcastContextGetBroadcastHandle(), RECEIVER->sync_params);
}

/***************************************************************************
NAME
    receiverHandleScmPowerOff
 
DESCRIPTION
    Utility function to handle SCM Afh Update message
    
PARAMS
    None    
    
RETURNS
    None
*/
static void receiverHandleScmAfhUpdate(BROADCAST_CMD_AFH_FULL_UPDATE_IND_T *ind)
{
    AfhMap afh_map;
    uint16 index;
    uint32 instant;
    BROADCAST *ba_receiver;
    uint16 instant_offset = SCM_AFH_UPDATE_MESSAGE_HEADER_LEN +
                            SCM_AFH_UPDATE_MESSAGE_MAP_LEN;
    
    DEBUG_RECEIVER(("Receiver:SCM: RX AFH Update Msg\n"));
    /* extract the AFH map and instant to apply the map */
    instant = (((uint32)ind->data[instant_offset + 0] << 24) |
               ((uint32)ind->data[instant_offset + 1] << 16) |
               ((uint32)ind->data[instant_offset + 2] << 8)  | 
                (uint32)ind->data[instant_offset + 3]);

    DEBUG_RECEIVER(("Receiver:SCM: New AFH Map: Clock:%08lx ", instant));
    for (index = SCM_AFH_UPDATE_MESSAGE_HEADER_LEN;
         index < SCM_AFH_UPDATE_MESSAGE_HEADER_LEN + SCM_AFH_UPDATE_MESSAGE_MAP_LEN;
         index++)
    {
        afh_map[index - SCM_AFH_UPDATE_MESSAGE_HEADER_LEN] = ind->data[index];
        DEBUG_RECEIVER(("%02x ", afh_map[index - SCM_AFH_UPDATE_MESSAGE_HEADER_LEN]));
    }
    DEBUG_RECEIVER(("\n"));

    /* First get the broadcast handle */
    ba_receiver = BroadcastContextGetBroadcastHandle();
    /* request the firmware use the new map at the instant */
    CsbReceiverSetAfhMap(&RECEIVER->srcAddr, BroadcastGetLtAddr(ba_receiver), &afh_map, instant);

    free(ind->data);
}

/***************************************************************************
NAME
    receiverLoadConfig
 
DESCRIPTION
    Utility function to load the Receiver Config
    
PARAMS
    void
    
RETURNS
    void
*/
static void receiverLoadConfig(void)
{
    receiverLoadStoredBroadcasterAddress();
    receiverLoadConfigKey();
}

/***************************************************************************
NAME
    receiverHandleBlInitCfm
 
DESCRIPTION
    Utility function to handle broadcast library BROADCAST_INIT_CFM messages
    
PARAMS
    task Handler for Receiver Broadcast related Messages
    cfm BROADCAST_INIT_CFM message
    
RETURNS
    void
*/
static void receiverHandleBlInitCfm(Task task, BROADCAST_INIT_CFM_T* cfm)
{
    UNUSED(task);

    if (cfm->status == broadcast_success)
    {
        DEBUG_RECEIVER(("Receiver: BROADCAST_INIT_CFM\n"));

        /* remember the receiver instance of the broadcast library
         * for future API calls */
        BroadcastContextSetBroadcastHandle(cfm->broadcast);

        BroadcastCmdScmInit(task, broadcast_cmd_role_receiver);
    }
}

/***************************************************************************
NAME
    receiverHandleBlStopRecvInd
 
DESCRIPTION
    Utility function to handle broadcast library BROADCAST_STOP_RECEIVER_IND messages
    
PARAMS
    task Handler for Receiver Broadcast related Messages
    ind BROADCAST_STOP_RECEIVER_IND message
      
RETURNS
    void
*/
static void receiverHandleBlStopRecvInd(Task task, BROADCAST_STOP_RECEIVER_IND_T* ind)
{
    UNUSED(task);

    /* Notify main task that receiver encountered CSB link loss */
    MessageSend(sinkGetMainTask(), EventSysBaRxCsbLinkLoss, 0);

    DEBUG_RECEIVER(("Receiver: BROADCAST_STOP_RX_IND: destroy streams, start rx auto\n"));
    BroadcastContextSetInvalidVariantIV();
    /* Respond to the indication so the broadcast library will let us start again */
    BroadcastReceiverTimeoutResponse(ind->broadcast);

    /* Disconnect the BA audio */
    receiverDisconnectAudio(task);

    /* try and find the broadcast again */
    DEBUG_RECEIVER(("Receiver: BROADCAST_STOP_RECEIVER_IND: Start receiver auto\n"));
    sinkReceiverScanForVariantIv(task);
    MessageSend(task, RECEIVER_INTERNAL_MSG_START_RECEIVER, NULL);
}

/***************************************************************************
NAME
    receiverConfigStoreKey
 
DESCRIPTION
    Store the security key.
 
PARAMS
    key  Security Key
RETURNS
    void
*/
static void receiverConfigStoreKey(uint16* key)
{
    receiver_writeable_keys_config_def_t* rw_config_data = NULL;

    DEBUG_RECEIVER(("Receiver: receiverConfigStoreKey(): Security Key\n"));

    if (configManagerGetWriteableConfig(RECEIVER_WRITEABLE_KEYS_CONFIG_BLK_ID, (void **)&rw_config_data, 0))
    {
        memmove(&rw_config_data->receiver_keys.sec, key, sizeof(rw_config_data->receiver_keys.sec));
        configManagerUpdateWriteableConfig(RECEIVER_WRITEABLE_KEYS_CONFIG_BLK_ID);
    }
}

/***************************************************************************
NAME
    receiverHandleBcmdInitCfm
 
DESCRIPTION
    Utility function to handle Broadcast Cmd library BROADCAST_CMD_INIT_CFM messages
    
PARAMS
    task Handler for receiver BCMD related Messages
    cfm BROADCAST_CMD_INIT_CFM message
    
RETURNS
    void
*/
static void receiverHandleBcmdInitCfm(Task task, BROADCAST_CMD_INIT_CFM_T* cfm)
{
    UNUSED(task);

    DEBUG_RECEIVER(("Receiver:receiverHandleBcmdInitCfm(), Status %d\n",cfm->status));

    if (cfm->status == broadcast_cmd_success)
    {
        sinkBroadcastAudioHandleInitCfm();
    }
}

/***************************************************************************
NAME
    receiverParseVariantIV
 
DESCRIPTION
    Utility function to parse the received advert data to extract the variant IV
    
PARAMS
    data Pointer to the advertising data
    data_length Indicates the length of the data array
    broadcast_encr_config_t* reference to encryption config
RETURNS
    TRUE if variant IV is found else FALSE
*/
static bool receiverParseVariantIV(const uint8 *data, uint16 data_length, broadcast_encr_config_t* encr)
{
    ble_ad_type ad_type;
    uint16 ad_data_size;

    /* Loop through the advert data, and extrat each ad-type to parse */
    while(data && data_length)
    {
        /* first octet has the data size of the ad_type */
        ad_data_size = data[0];
        if(ad_data_size)
        {
            /* The next octet has the ad_type */
            ad_type = data[1];
            switch(ad_type)
            {
                case ble_ad_type_service_data:
                {
                    /* We expect service_data to have atleast 5 octets
                         1 to specify ad_type, followed by 2 octets for BA UUID and then
                         followed by 2 octets of Variant IV */
                    if((ad_data_size >= 5) && CHECK_BA_UUID(data[3], data[2]))
                    {
                        /* variant IV shall be immediately after BA UUID */
                        encr->variant_iv = ((uint16)(((uint16)data[5] << 8) | ((uint16)data[4])));
                        return TRUE;
                    }
                }
                break;
                /* Ignore */
                case ble_ad_type_flags:
                default:
                break;
            }
        }

        /* Could be a superfluous data_length! */
        if((ad_data_size + 1) <=  data_length)
        {
            /* Increase the pointer to the next ad_type. Please note we need to 
                add 1 to consider the placeholder of data_size */
            data += (ad_data_size + 1);
            /* At the same time need to decrease the length as we don't expect to 
                read more than required */
            data_length -= (ad_data_size + 1);
        }
        else
        {
            /* Looks like a invalid packet, just igore it*/
            data_length = 0;
        }
    }
    return FALSE;
}

/*--------------------Interfaces to BA Receiver Module----------------------*/

/****************************************************************************/
void sinkReceiverHandleBroadcasterDisconnectInd(GATT_MANAGER_DISCONNECT_IND_T* ind)
{
    if (ind->status == gatt_manager_status_success)
    {
        DEBUG_RECEIVER(("Receiver:GATTMAN: Disconnect Ind\n"));
        /* no longer connected, stop association indication and forget CID */
        MessageSend(sinkGetMainTask(), EventSysBAAssociationEnd, 0);
        RECEIVER->broadcast_server_cid = INVALID_CID;
    }
    else
    {
        DEBUG_RECEIVER(("Receiver:GATTMAN: Disconnect failed\n"));
    }
}

/****************************************************************************/
void sinkReceiverHandleBroadcasterConnectCfm(GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM_T* cfm)
{
    if (cfm->status == gatt_manager_status_success)
    {
        DEBUG_RECEIVER(("Receiver:GATTMAN A Broadcast Service server connected, discovering services\n"));

        /* remember the connection ID for later disconnection */
        RECEIVER->broadcast_server_cid = cfm->cid;
    }
}

/***************************************************************************
NAME
    sinkReceiverHandleAssocRecvCfm
 
DESCRIPTION
    Utility function to handle the association data received from broadcaster
    
PARAMS
    assoc_data pointer to association data
       
RETURNS
    void
*/
void sinkReceiverHandleAssocRecvCfm(broadcast_assoc_data* assoc_data)
{
    if (assoc_data)
    {
        /* check with broadcast library if the broadcast version matches and we can go ahead */
        if(broadcast_success == BroadcastSetAssocInfo(BroadcastContextGetBroadcastHandle(), assoc_data))
        {
            DEBUG_RECEIVER(("Receiver: Broadcast Identifier [%04x %04x]\n", assoc_data->broadcast_identifier[0],
                                                                     assoc_data->broadcast_identifier[1]));
            if (assoc_data->device_status_len == BROADCAST_DEVICE_STATUS_MAX_LEN &&
                assoc_data->seckey_len == BROADCAST_KEY_MAX_LEN &&
                BroadcastContextStreamCanBeReceived(assoc_data->stream_service_records,
                                    assoc_data->stream_service_records_len,
                                    assoc_data->device_status[1]))
            {
                broadcast_encr_config_t* encr = NULL;

                encr = BroadcastContextGetEncryptionConfig();
                /* Association completed, save the BSSR and broadcaster address in PS */ 
                if (receiverSetBssr(assoc_data->stream_service_records, assoc_data->stream_service_records_len) &&
                    receiverSetBroadcasterAddress(&assoc_data->broadcast_addr))
                {
                    /* also store in theApp for local use */
                    RECEIVER->srcAddr = assoc_data->broadcast_addr;
                    if(encr)
                    {
                        /* copy the encryption information */
                        memcpy_pack(&encr->seckey[0], &assoc_data->seckey[0], assoc_data->seckey_len / 2);
                        /* save the security key in PS */
                        receiverConfigStoreKey(&encr->seckey[0]);
                    }
                    /* Start scaning for non-conn IV, this will automatically taken care in BLE-statemachine
                        on receiving the disconnect indication */
                }
            }
            else
            {
                DEBUG_RECEIVER(("Receiver: cannot associate with broadcaster because of un-compactible SSR's\n"));
            }
        }
        else
        {
            DEBUG_RECEIVER(("Receiver: could not associate with broadcaster are we have different version\n"));
        }
    }
}

/*******************************************************************************/
bool sinkReceiverIsBroadcasterCid(uint16 cid)
{
    if(RECEIVER)
        return (cid == RECEIVER->broadcast_server_cid) ? TRUE : FALSE;
    return FALSE;
}

/*******************************************************************************/
void sinkReceiverScanForVariantIv(Task task)
{
    UNUSED(task);

    /* Trigger scanning for variant IV */
    sinkBleStartBroadcastEvent();
}

/*******************************************************************************/
void sinkReceiverHandleIVAdvert(const CL_DM_BLE_ADVERTISING_REPORT_IND_T* ind)
{
    /* only take notice of Variant IV adverts coming from our broadcaster or not in association state*/
    if((!BA_RECEIVER_MODE_ACTIVE) || 
       (!BdaddrIsSame(&RECEIVER->srcAddr, &ind->permanent_taddr.addr)) || 
       (gapBaGetAssociationInProgress()))
    {
        return;
    }
    /* Receiver can start receiving Broadcast Messages even in Limbo state  & go ahead with connecting 
    Audio plugin so don't initialize Broadcast library in Limbo Mode */
    if(deviceLimbo != stateManagerGetState())
    {
        broadcast_encr_config_t* encr = NULL;

        encr = BroadcastContextGetEncryptionConfig();

        /* only process this advert if we've not already seen one and recorded the IV */
        if (!encr->variant_iv)
        {
            /* got IV advert, save the IV in our encryption settings */
            if(receiverParseVariantIV(ind->advertising_data, ind->size_advertising_data, encr))
            {
              DEBUG_RECEIVER(("Receiver:BLE: Got Variant IV advert %x\n", encr->variant_iv));
              BroadcastContextSetValidVariantIV();
              /* stop further scanning, should be handled in GAP state machine */
              sinkBleStopBroadcastEvent();
              /* we only currently support IV discovery at startup, so now configure the Broadcast
               * library for receiver */
              receiverconfigureReceiver();
            }
            /* Missed the variant IV in the advert, just wait for the correct advert */
        }
        else
        {
            DEBUG_RECEIVER(("Receiver:BLE: Received extra ADVERTISING_REPORT_INDs\n"));
        }
    }
}

/*******************************************************************************/
void sinkReceiverInit(Task task)
{
    if(!RECEIVER)
    {
        /* Init memory for receiver */
        RECEIVER = (receiverData_t*)PanicUnlessMalloc(sizeof(receiverData_t));
        memset(RECEIVER,0,sizeof(receiverData_t));

        RECEIVER->task = task;

        /* Set default volume */
        RECEIVER->volume = (RECEIVER_VOLUME_STEP_SIZE * receiverGetVolume());

        PanicNotNull(RECEIVER->sync_params);
        RECEIVER->sync_params = PanicUnlessMalloc(sizeof(*RECEIVER->sync_params));

        sinkBroadcastAcquireVolumeTable();

        BroadcastInit(task, broadcast_role_receiver);

        /* Try and load stored configuration */
        receiverLoadConfig();
    }
}

/***************************************************************************
NAME
    receiverHandleBlDeInitCfm
 
DESCRIPTION
    Utility function to handle broadcast library BROADCAST_DESTROY_CFM messages
    
PARAMS
    task Handler for Receiver Broadcast related Messages
    cfm BROADCAST_DESTROY_CFM message
    
RETURNS
    void
*/
static void receiverHandleBlDeInitCfm(Task task, BROADCAST_DESTROY_CFM_T* cfm)
{
    UNUSED(task);

    if (cfm->status == broadcast_success)
    {

        if(!sinkReceiverIsAssociated())
            /*We are exiting receiver role so if receiver is not associated till now cancel the 
            BA Receiver not associated filter*/
            MessageSend(sinkGetMainTask(), EventSysBACancelReceiverAssociationFilter, NULL );

        /* Free Receiver Instance */
        free(RECEIVER);
        RECEIVER = NULL;

        /* so stop scanning the IV */
        sinkBroadcastAudioStopIVAdvertsAndScan();
        BroadcastContextSetInvalidVariantIV();
        sinkBroadcastAudioHandleDeInitCfm(sink_ba_appmode_receiver);
    }
}

/*******************************************************************************/
void sinkReceiverHandleBroadcastMessage(Task task, MessageId id, Message message)
{
    switch (id)
    {
        case BROADCAST_INIT_CFM:
            receiverHandleBlInitCfm(task,(BROADCAST_INIT_CFM_T*)message);
            break;

        case BROADCAST_DESTROY_CFM:
            receiverHandleBlDeInitCfm(task,(BROADCAST_DESTROY_CFM_T*)message);
            break;

        case BROADCAST_CONFIGURE_CFM:
            receiverHandleBlConfigureCfm(task, (BROADCAST_CONFIGURE_CFM_T*)message);
            break;

        case BROADCAST_STATUS_IND:
            break;

        case BROADCAST_START_RECEIVER_CFM:
            receiverHandleBlStartRecvCfm(task, (BROADCAST_START_RECEIVER_CFM_T*)message);
            break;

        case BROADCAST_STOP_RECEIVER_CFM:
            receiverHandleBlStopRecvCfm(task, (BROADCAST_STOP_RECEIVER_CFM_T*)message);
            break;

        case BROADCAST_STOP_RECEIVER_IND:
            receiverHandleBlStopRecvInd(task, (BROADCAST_STOP_RECEIVER_IND_T*)message);
            break;

        default:
            DEBUG_RECEIVER(("Receiver: BM: Unhandled msg I0x%x\n", id));
            break;
    }

}

/***************************************************************************/
void sinkReceiverInternalDisconnectAudio(void)
{
    /* Register this module as disconnected by setting the receiver source to NULL. */
    BroadcastContextSetReceiverSource(NULL);

    /* Call the audio framework to update the routing. 
       It will disconnect the receiver plugin if required. */
    audioUpdateAudioRouting();
}

/*******************************************************************************/
void sinkReceiverHandleBcmdMessage(Task task, MessageId id, Message message)
{
    switch (id)
    {
        case BROADCAST_CMD_INIT_CFM:
            receiverHandleBcmdInitCfm(task, (BROADCAST_CMD_INIT_CFM_T *)message);
            break;

        case BROADCAST_CMD_AFH_FULL_UPDATE_IND:
            receiverHandleScmAfhUpdate((BROADCAST_CMD_AFH_FULL_UPDATE_IND_T *)message);
            break;

        default:
            DEBUG_RECEIVER(("Receiver: SCM: Unhandled msg I0x%x\n", id));
            break;
    }

}

/*******************************************************************************/
void sinkReceiverStartAssociation(void)
{
    DEBUG_RECEIVER(("Receiver: Association Button Pressed\n"));

    if (RECEIVER)
    {
        DEBUG_RECEIVER(("Receiver: Starting association\n"));

        /* Need to disconnect the broadcast audio input plugin before stopping
           the broadcast receiver. This order is important to avoid the plugin
           trying to read data from an invalid broadcast receive stream.

           Start here by disconnecting the broadcast audio input.

           The broadcast receiver should be stopped when the plugin
           has completed disconnecting.*/
        MessageCancelAll(RECEIVER->task, RECEIVER_INTERNAL_MSG_START_RECEIVER);
        receiverDisconnectAudio(RECEIVER->task);

        DEBUG_RECEIVER(("Sending stop receiver\n"));
        MessageSendConditionally(RECEIVER->task,
                                 RECEIVER_INTERNAL_MSG_STOP_RECEIVER,
                                 NULL, BroadcastContextGetBroadcastBusy());
    }
}

/*******************************************************************************/
void sinkReceiverPowerOff(Task task)
{
    UNUSED(task);
    DEBUG_RECEIVER(("Receiver: MSG_POWER_OFF\n"));

    BroadcastContextSetInvalidVariantIV();
    /* Need to disconnect the broadcast audio input plugin before stopping
       the broadcast receiver. This order is important to avoid the plugin
       trying to read data from an invalid broadcast receive stream.

       Start here by disconnecting the broadcast audio input.

       The broadcast audio input plugin should be stopped when the plugin
       has completed disconnecting.*/
    MessageCancelAll(task, RECEIVER_INTERNAL_MSG_START_RECEIVER);
    receiverDisconnectAudio(task);

    DEBUG_RECEIVER(("Sending stop receiver\n"));
    MessageSendConditionally(task,
                             RECEIVER_INTERNAL_MSG_STOP_RECEIVER,
                             NULL, BroadcastContextGetBroadcastBusy());
}

/*******************************************************************************/
void sinkReceiverWriteVolumePsKey(void)
{
    DEBUG_RECEIVER(("Receiver: Write Volume PS Key\n"));
    receiverSetVolume(RECEIVER->volume);
}

/*******************************************************************************/
int16 sinkReceiverGetGlobalVolume(void)
{
    return RECEIVER->global_volume;
}

/*******************************************************************************/
int16 sinkReceiverGetLocalVolume(void)
{
    return RECEIVER->volume;
}

/*******************************************************************************/
void sinkReceiverSetGlobalVolume(int16 volume)
{
    RECEIVER->global_volume = volume;
}

/*******************************************************************************/
void sinkReceiverSetLocalVolume(int16 volume)
{
    RECEIVER->volume = volume;
}

/*******************************************************************************/
uint16 sinkReceiverGetActualVolume(void)
{
    return RECEIVER->actual_volume;
}

/*******************************************************************************/
void sinkReceiverSetActualVolume(uint16 volume)
{
    RECEIVER->actual_volume = volume;
}

/*******************************************************************************/
void sinkReceiverResetVariantIv(void)
{
    broadcast_encr_config_t * encr = BroadcastContextGetEncryptionConfig();
    encr->variant_iv = 0;
    BroadcastContextSetInvalidVariantIV();
}

/*******************************************************************************/
bool sinkReceiverIsAssociated(void)
{
    return receiverLoadStoredBroadcasterAddress();
}

/*******************************************************************************/
void sinkReceiverHandlePowerOn(Task receiver_task)
{
    DEBUG_RECEIVER(("Receiver: Power On\n"));
    /* see if we have a stored broadcaster in PS,
       use it to try and receive audio from our associated broadcaster */
    if (sinkReceiverIsAssociated())
    {
        sinkReceiverScanForVariantIv(receiver_task);
    }
    else
    {
        /* No known broadcaster, await user association
            Indicate on LED that BA Receiver is not yet associated*/
        MessageSend(sinkGetMainTask() , EventSysBAReceiverNotAssociated, NULL );
        /* Also start association on receiver */
        MessageSend(sinkGetMainTask() , EventUsrBAStartAssociation, NULL);
    }

    /* Check for any available audio source to play, as we might not have 
	   CSB stream at all */
    audioUpdateAudioRouting();
}

/*******************************************************************************/
void sinkReceiverDeInit(void)
{
    if(RECEIVER)
    {
        DEBUG_RECEIVER(("BA: sinkReceiverDeInit\n"));

        /* Disconnect any routed Receiver Audio */
        receiverDisconnectAudio(RECEIVER->task);

        /* Use a conditional message so that the plugin can disable scm,
           destroy the packetiser etc. before we destroy scm and the
           broadcast receiver. */
        MessageSendConditionally(RECEIVER->task,
                                 RECEIVER_INTERNAL_MSG_DESTROY_RECEIVER,
                                 0, BroadcastContextGetBroadcastBusy());
        MessageCancelAll(RECEIVER->task, RECEIVER_INTERNAL_MSG_START_RECEIVER);
    }
}

/*******************************************************************************/
void sinkReceiverUpStreamMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(message);

    switch(id)
    {
        case AUDIO_BA_RECEIVER_START_SCAN_VARIANT_IV:
        {
            if(!gapBaGetAssociationInProgress())
            {
                /* Scan for Variant IV only if we ar e not in association mode */
                sinkReceiverScanForVariantIv(task);
            }
        }
        break;

        case AUDIO_BA_RECEIVER_RESET_BD_ADDRESS:
        {
            bdaddr* lAddr = NULL;
            /* This should not happen. But if for some reason it has happened,
               we need to reassociate to read the BSSR. This could be performed
               automatically, however, this application will just delete the
               association from ps and panic. The user will have to manually
               reassociate. */
            lAddr = mallocPanic(sizeof(bdaddr));
            memset(lAddr,0,sizeof(bdaddr));
            receiverSetBroadcasterAddress(lAddr);
            free(lAddr);
        }
        break;

        case AUDIO_BA_RECEIVER_INDICATE_STREAMING_STATE:
        {
            /* TODO once extended states framework is ready, decide on how to 
            indicate this streaming state on LED*/
        }
        break;

        case AUDIO_BA_RECEIVER_AFH_CHANNEL_MAP_CHANGE_PENDING:
        {
            BroadcastStartReceiver(BroadcastContextGetBroadcastHandle(), broadcast_mode_synctrain_only, 0x0000);
        }
        break;

        case AUDIO_BA_RECEIVER_RESET_SCM_RECEIVER:
        {
            if(BroadcastContextGetReceiverScm())
                ScmReceiverReset(BroadcastContextGetReceiverScm());
        }
        break;

        default:
            break;
    }
}

/*******************************************************************************/
bool sinkReceiverIsStreaming(void)
{
    return AudioIsControlPlugin(getReceiverPlugin());
}

bool sinkReceiverIsRoutable(void)
{
    /* The check for state != deviceLimbo is for the power-off use case.
       In that situation even if we have a valid receiver source, we want
       to disconnect the receiver plugin as soon as possible to avoid
       rendering audio after any 'power off' tone or prompt. */
    return ((BroadcastContextGetReceiverSource() != 0)
            && (deviceLimbo != stateManagerGetState()));
}

bool baAudioSinkMatch(Sink sink)
{
    Sink csb_sink;
    csb_sink = NULL;

    csb_sink = StreamSinkFromSource(BroadcastContextGetReceiverSource());

    DEBUG_RECEIVER(("BA: baAudioSinkMatch sink %p rx_source %p rx_sink %p\n",
                    (void *)sink,
                    (void *)BroadcastContextGetReceiverSource(),
                    (void *)csb_sink));

    if(csb_sink)
    {
        return sink == csb_sink;
    }
    /* NULL csb_sink is invalid, so don't route the BA audio */
    return TRUE;
}

bool baAudioIsRouted(void)
{
    if(baAudioSinkMatch(sinkAudioGetRoutedAudioSink()))
    {
        return TRUE;
    }
    return FALSE;
}

void baAudioPostDisconnectConfiguration(void)
{
    DEBUG_RECEIVER(("BA: baAudioPostDisconnectConfiguration\n"));
}

void sinkReceiverStartReceiver(void)
{
    if(BroadcastContextGetVariantIVIsSet())
    {
        DEBUG_RECEIVER(("BA: Receiver audio is routable \n"));
        BroadcastStartReceiver(BroadcastContextGetBroadcastHandle(), broadcast_mode_auto, RECEIVE_SUPERVISION_TIMEOUT);
    }
    else
    {
        DEBUG_RECEIVER(("BA: Receiver audio is not routable as Variant IV is not set\n"));
    }
}

void sinkReceiverInternalStartReceiver(Task task)
{
    if(BroadcastContextGetVariantIVIsSet())
    {
        DEBUG_RECEIVER(("BA: Receiver audio is routable \n"));
        BroadcastStartReceiver(BroadcastContextGetBroadcastHandle(), broadcast_mode_auto, RECEIVE_SUPERVISION_TIMEOUT);
    }
    else
    {
        DEBUG_RECEIVER(("BA: Variant IV is not set repost internal start receiver\n"));
        MessageSendConditionally(task, RECEIVER_INTERNAL_MSG_START_RECEIVER, 
            NULL, BroadcastContextGetVariantIVReference());
    }
}

void sinkReceiverStopReceiver(void)
{
    DEBUG_RECEIVER(("BA: sinkReceiverStopReceiver\n"));
    BroadcastStopReceiver(BroadcastContextGetBroadcastHandle());
}

void sinkReceiverDestroyReceiver(void)
{
    DEBUG_RECEIVER(("BA: sinkReceiverDestroyReceiver\n"));

    /* Stop the Broadcast Receive */
    sinkReceiverStopReceiver();

    /* Free the sync train params */
    free(RECEIVER->sync_params);
    RECEIVER->sync_params = NULL;

    free(RECEIVER->bssr);
    RECEIVER->bssr = NULL;

    /* Destroy the scm library. */
    BroadcastCmdScmDestroy();

    /* De-initialize Broadcast library */
    /* This should only be destroyed when switching roles */
    BroadcastDestroy(BroadcastContextGetBroadcastHandle());

    sinkBroadcastReleaseVolumeTable();

    DEBUG_RECEIVER(("BA Receiver: Destroyed\n"));
}

void sinkReceiverDeleteBroadcasterInfo(void)
{
    DEBUG_RECEIVER(("BA: sinkReceiverDeleteBroadcasterInfo\n"));
    /* Receiver is trying to become broadcaster
       remove the previous broadcaster info, else we end up with
       security issues */
    if(receiverCheckIfBroadcasterCanBeRemoved(&RECEIVER->srcAddr))
    {
        receiverResetBroadcasterAddress();
    }
}
#endif /* ENABLE_BROADCAST_AUDIO */


/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ba_broadcaster.c

DESCRIPTION
    This is an internal submodule for broadcast audio. This module implements
    Broadcaster functionalities of broadcast audio. It provides interface to other
    sub modules to access(set/get) broadcaster behaviours.
    Major functionalities handled in this module are
    1.Broadcaster Association
    2.Security key/random number generation for association
    3.Handle Broadcaster configuration
    4.Handles all messages form broadcaster
    5.Broadcaster connection
    
NOTES
    This module interfaces need to be exposed to sink_ba or sink_ba_pluging modules
    only. It is restricted to access these interfaces from any other modules

*/

#include "sink_ba_broadcaster.h"
#include "sink_debug.h"

#ifdef ENABLE_BROADCAST_AUDIO

#include "sink_ba_broadcaster_encryption.h"
#include "sink_ba_broadcaster_scm.h"
#include "sink_ba_broadcaster_packetiser.h"
#include "sink_ba.h"
#include "sink_main_task.h"
#include "sink_gatt_server_ba.h"
#include "sink_ble.h"
#include "sink_configmanager.h"
#include "sink_broadcast_audio_config_def.h"

#include <broadcast_context.h>
#include <broadcast_cmd.h>
#include <broadcast.h>
#include <afh_channel_map.h>
#include <a2dp.h>

#include <vm.h>
#include <vmtypes.h>
#include <panic.h>

#include <stdlib.h>

/* Debugging defines */
#ifdef DEBUG_BA_BROADCASTER
#define DEBUG_BROADCASTER(x) DEBUG(x)
#else
#define DEBUG_BROADCASTER(x)
#endif

#ifdef AFH_LOGGING
#define AFH_LOGGING_DEBUG(x)   DEBUG(x)
#else
#define AFH_LOGGING_DEBUG(x)
#endif

/*! Offset to apply to the instant for an AFH update to be applied on broadcaster
 *  and receiver, in BT slots when using sync train method; 1 second. */
#define AFH_UPDATE_INSTANT_OFFSET   (3200)


/*---------------------CSB Defines--------------------------*/
/* Time in slots before we'll receive a supervision timeout
 * for failure to transmit a CSB packet. */
#define CSB_SUPERVISION_TIMEOUT         0x2000
/* Permitted packets to use for CSB. */
#define CSB_PACKET_TYPE                 8

/*---------------------SYNC TRAIN related Defines ----------*/
/* Default sync train timeout of 30s (in BT slots) */
#define SYNC_TRAIN_TIMEOUT              (0xBB80)
/* Don't timeout the sync train, broadcast library will
 * automatically restart it for us, and always keep it running */
#define SYNC_TRAIN_NO_TIMEOUT           0
/* Minimum interval for sync train packets. */
#define SYNC_TRAIN_INTERVAL_MIN         0xA0 
/* Maximum interval for sync train packets. */
#define SYNC_TRAIN_INTERVAL_MAX         0xC0
/* Service data in the sync train, must be 0x00 */
#define SYNC_TRAIN_SERVICE_DATA_WORD    0x00

/*-----------------Data structure used for Broadcaster------*/ 

/* Broadcaster data, this single structure handles all
   data related to broadcaster.
*/
typedef struct _brodcasterdata_t{
    /* Reference to an instance of the broadcast library, configured  as a broadcaster */
    BROADCAST* broadcaster;

    /* Parameters for configuring a broadcast stream */
    broadcaster_csb_params csb_params;
    /* Parameters for configuring a sync train, advertising the broadcast stream */
    broadcaster_sync_params sync_params;

    /* Lock during handling of adverts/connecting/disconnecting BLE link */
    unsigned        connecting:1;

    /*! Flag indicating we have requested a local change in the
     * AFH channel map, and not yet been notified it has been
     * applied. */
    unsigned afh_local_map_change_in_progress:1;
    
    /* Flag to record whether Broadcaster is started */
    unsigned broadcaster_started:1;

    /*! Spare bit fields */
    unsigned __SPARE__:13;

    /* Broadcaster task. */
    Task task;

}brodcasterdata_t;


/* Reference to broadcaster data for use in this module */
static brodcasterdata_t *gBroadcaster = NULL;
#define BROADCASTER   gBroadcaster

static uint8 broadcasterLoadStoredLtAddress(void);
static bool broadcasterIsContinuousSyncTrainSet(void);
static void BroadcasterMessageHandler(Task task, MessageId id, Message message);
static void handleBroadcastIsReadyForAudio(void);

/********************** External Interface Definitions*************************/

/*--------------------------Interfaces to BA Module---------------------------*/
bool sinkBroadcasterInit(Task  appTask,Task task)
{
    if(!BROADCASTER)
    {
        /* Init memory for broadcaster */
        BROADCASTER = (brodcasterdata_t*)PanicUnlessMalloc(sizeof(brodcasterdata_t));
        memset(BROADCASTER,0,sizeof(brodcasterdata_t));

        task->handler = BroadcasterMessageHandler;

        /* Store the broadcaster task */
        BROADCASTER->task = appTask;

        /* Update the broadcaster server database with the stream ID */
        switch(sinkBroadcastAudioGetEcScheme())
        {
            case EC_SCHEME_2_5:
            default:
                sinkGattBAServerUpdateStreamID(CELT_STREAM_ID_EC_2_5);
                BroadcastContextSetStreamId(CELT_STREAM_ID_EC_2_5);
                BroadcastContextSetEcScheme(EC_SCHEME_2_5);
                break;

            case EC_SCHEME_3_9:
                sinkGattBAServerUpdateStreamID(CELT_STREAM_ID_EC_3_9);
                BroadcastContextSetStreamId(CELT_STREAM_ID_EC_3_9);
                BroadcastContextSetEcScheme(EC_SCHEME_3_9);
                break;
        }

        setupEncryption();

        /* got new IV, so reset prev_ttp_ext */
        BroadcastContextSetTtpExtension(0);

        sinkBroadcastAcquireVolumeTable();

        /* Init Broadcast Lib with broadcaster Role */
        BroadcastInit(task, broadcast_role_broadcaster);

        return TRUE;
    }
    return FALSE;
}

static void setupCsbParams(broadcaster_csb_params *csb_params)
{
    csb_params->lpo_allowed = TRUE;
    csb_params->packet_type = CSB_PACKET_TYPE;
    csb_params->interval_min = CSB_INTERVAL_SLOTS;
    csb_params->interval_max = CSB_INTERVAL_SLOTS;
    csb_params->supervision_timeout = CSB_SUPERVISION_TIMEOUT;
}

static void setupSyncParams(broadcaster_sync_params *sync_params)
{
    sync_params->interval_min = SYNC_TRAIN_INTERVAL_MIN;
    sync_params->interval_max = SYNC_TRAIN_INTERVAL_MAX;
    sync_params->service_data = SYNC_TRAIN_SERVICE_DATA_WORD;

    if(broadcasterIsContinuousSyncTrainSet())
    {
       sync_params->sync_train_timeout = SYNC_TRAIN_NO_TIMEOUT;
    }
    else
    {
        sync_params->sync_train_timeout = SYNC_TRAIN_TIMEOUT;
    }
}

/******************************************************************************
 * Broadcast LIB Message Handlers
 ******************************************************************************/

static void broadcasterHandleBlInitCfm(Task task,BROADCAST_INIT_CFM_T* cfm)
{
    if (cfm->status == broadcast_success)
    {
        DEBUG_BROADCASTER(("BA: Broadcast Lib Init\n"));

        setupCsbParams(&BROADCASTER->csb_params);
        setupSyncParams(&BROADCASTER->sync_params);


        BROADCASTER->broadcaster = cfm->broadcast;
            
        BroadcastConfigureBroadcaster(BROADCASTER->broadcaster, 
                                      broadcasterLoadStoredLtAddress(), 
                                      &BROADCASTER->csb_params, &BROADCASTER->sync_params);
        BroadcastCmdScmInit(task, broadcast_cmd_role_broadcaster);
    }


}

/******************************************************************************/
static void broadcasterHandleBlConfigureCfm(Task task,BROADCAST_CONFIGURE_CFM_T* cfm)
{
    UNUSED(task);

    if (cfm->status == broadcast_success)
    {
        DEBUG_BROADCASTER(("BA: Broadcast Configure Success\n"));
    }
}

/*****************************************************************************/
void sinkBroadcasterHandlePowerOn(Task broadcaster_task)
{
    DEBUG_BROADCASTER(("Broadcaster: Power On \n"));
    /* Attempt to start the broadcast */
    MessageSend(broadcaster_task, BROADCASTER_INTERNAL_MSG_START_BROADCAST, NULL);
}

/*******************************************************************************/
void sinkBroadcasterStartBroadcast(broadcast_mode mode)
{
    if(!BROADCASTER->broadcaster_started)
    {
        DEBUG_BROADCASTER(("APP:BROADCASTER_INTERNAL_MSG_START_BROADCAST calling BroadcastStartBroadcast()\n"));
        BROADCASTER->broadcaster_started = TRUE;
        BroadcastStartBroadcast(BROADCASTER->broadcaster, mode);
    }
}

/******************************************************************************/
static void broadcasterHandleBlStartBroadcastCfm(Task task,BROADCAST_START_BROADCAST_CFM_T* cfm)
{
    UNUSED(task);
    DEBUG_BROADCASTER(("BA: BROADCAST_START_BROADCAST_CFM_T: Status %x, mode %x\n", cfm->status , cfm->mode));

    if (cfm->status == broadcast_success)
    {
        if (cfm->mode == broadcast_mode_auto)
        {
            DEBUG_BROADCASTER(("BA: BRD_START_CFM: Auto started\n"));
#ifndef BROADCAST_PACKETISER_IS_CONTROLLED_BY_APP
            afhChannelMapInit();
#endif
            BroadcastContextSetSink(cfm->sink);
            /* Trigger advertising non-conn IV adverts */
            sinkBleStartBroadcastEvent();

            if(sinkBroadcasterIsPacketiserControlledByApp())
            {
                sinkBroadcasterInitErasureCoding();
                sinkBroadcasterInitPacketiser(sinkBroadcastAudioGetHandler());
            }
            else
            {
                handleBroadcastIsReadyForAudio();
            }
        }
    }
}

static void handleBroadcastIsReadyForAudio(void)
{
    /* start receiving AFH map updates from firmware */
#ifndef DISABLE_AFH
    CsbEnableNewAfhMapEvent(TRUE);
#endif
    audioUpdateAudioRouting();
}

/******************************************************************************/
/*! @brief Count the number of set bits in the AFH channel map. 
    AFH channel map is 10 bytes packed into 5 uint16s.
 */
static int broadcastercountNumberAfhChannelsInMap(AfhMap afh_map)
{
    int map_index;
    int afh_channel_count = 0;

    for (map_index = 0; map_index < (int)(sizeof(AfhMap)/sizeof(uint16)); map_index++)
    {
        uint16 v=afh_map[map_index], c;
        for (c=0;v;c++)
            v &= v-1;
        afh_channel_count += c;
    }
    return afh_channel_count;
}

static void printChannelMap(AfhMap *channel_map)
{
    uint16 map_index;

    for (map_index = 0; map_index < sizeof(*channel_map)/sizeof(uint16); map_index++)
    {
            /* build AfhMap structure for local AFH update trap */
           AFH_LOGGING_DEBUG(("%02x ", (channel_map[map_index] & 0xFF)));
    }
    AFH_LOGGING_DEBUG(("\n"));
}

/******************************************************************************/
static void broadcasterHandleBlAfhUpdateInd(Task task, BROADCAST_AFH_UPDATE_IND_T* ind)
{
    uint32 apply_instant = ind->clock + AFH_UPDATE_INSTANT_OFFSET;
    uint16 afh_channel_count = broadcastercountNumberAfhChannelsInMap(ind->channel_map);

    UNUSED(task);

    AFH_LOGGING_DEBUG(("Broadcaster: New AFH Map: VMTime:%08lx Clock:%08lx New Clock:%08lx ChannelCount:%d ", 
                                    VmGetClock(), ind->clock, apply_instant, afh_channel_count));

    /* only send AFH update if we don't already have one in-flight to receivers */
    if ((!BROADCASTER->afh_local_map_change_in_progress)
        && (afh_channel_count >= 20))
    {
        printChannelMap(&ind->channel_map);

        AFH_LOGGING_DEBUG(("Broadcaster:AFH: Sending this AFH update\n"));

        BROADCASTER->afh_local_map_change_in_progress = CsbTransmitterSetAfhMap(BroadcastGetLtAddr(BROADCASTER->broadcaster),
                                                                                &ind->channel_map, apply_instant);
        if(BROADCASTER->afh_local_map_change_in_progress)
        {
            afhChannelMapChangeIsPending();
        }
        else
        {
            DEBUG_BROADCASTER(("Broadcaster:AFH: Failure to set local AFH map! Channel Count %d\n",
                                                                afh_channel_count));
        }
    }
}

/******************************************************************************/
static void broadcasterHandleBlAfhChannelMapChangeInd(Task task, BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND_T *ind)
{
    UNUSED(task);
    UNUSED(ind);

    DEBUG_BROADCASTER(("APP:BL: AFH Channel map changed\n"));
    BROADCASTER->afh_local_map_change_in_progress = FALSE;
}



/*******************************************************************************/
void sinkBroadcasterStopBroadcast(broadcast_mode mode)
{
    if(sinkBroadcasterIsPacketiserControlledByApp())
    {
        sinkBroadcasterDestroyPacketiser();
        sinkBroadcasterDestroyErasureCoding();
    }

    DEBUG_BROADCASTER(("BA: sinkBroadcasterStopBroadcast()\n"));
    BROADCASTER->broadcaster_started = FALSE;
    BroadcastStopBroadcast(BROADCASTER->broadcaster, mode);
}

/***************************************************************************
NAME
    broadcasterBlBroadcastStoppedHandling

DESCRIPTION
    Standard Action for Broadcaster Stop

PARAMS
    void
RETURNS
    void
*/
static void broadcasterBlBroadcastStoppedHandling(Task task)
{
    /* no longer streaming, so stop advertising the IV */
    sinkBroadcastAudioStopIVAdvertsAndScan();

    /* stop receiving AFH map updates from firmware, as we can't send them
       over SCM */
    CsbEnableNewAfhMapEvent(FALSE);
    MessageCancelAll(task, BROADCAST_AFH_UPDATE_IND);

    /* sink is no longer valid */
    BroadcastContextSetSink(NULL);

}

/******************************************************************************/
static void broadcasterHandleBlStopBroadcastCfm(Task task, BROADCAST_STOP_BROADCAST_CFM_T* cfm)
{
    if (cfm->status == broadcast_success)
    {
        if (cfm->mode == broadcast_mode_auto)
        {
            DEBUG_BROADCASTER(("Broadcaster: BRD_STOP_CFM: Broadcast Auto Stopped\n"));
            /* standard actions on broadcast stopping */
            broadcasterBlBroadcastStoppedHandling(task);
        }
    }
    baBroadcastlibStoppedCfm();
}

/******************************************************************************/
static void broadcasterHandleBlStopBroadcastInd(Task task, BROADCAST_STOP_BROADCAST_IND_T* ind)
{
    /* broadcast timed out, it's no longer running try and restart it */
    DEBUG_BROADCASTER(("Broadcaster: BRD_STOP_IND: Broadcast Timed Out, restarting...\n"));

    /* respond to the timeout */
    BroadcastBroadcasterTimeoutResponse(ind->broadcast);

    if(sinkBroadcasterIsPacketiserControlledByApp())
    {
        sinkBroadcasterDestroyPacketiser();
        sinkBroadcasterDestroyErasureCoding();
    }

    /* standard actions on broadcast stopping */
    broadcasterBlBroadcastStoppedHandling(task);

    /* request broadcast restart */
    MessageSend(task, BROADCASTER_INTERNAL_MSG_START_BROADCAST, NULL);
}



/*****************************************************************************/
void sinkBroadcasterDeInit(void)
{
    DEBUG_BROADCASTER(("Broadcaster: sinkBroadcasterDeInit\n"));

    if(BROADCASTER)
    {
        /* Use a conditional message so that the plugin can disable scm,
           destroy the packetiser etc. before we destroy scm and the
           broadcast receiver. */
        MessageSendConditionally(BROADCASTER->task,
                                 BROADCASTER_INTERNAL_MSG_DESTROY_BROADCASTER,
                                 0, BroadcastContextGetBroadcastBusy());
    }
}

void sinkBroadcasterDestroyBroadcaster(void)
{
    DEBUG_BROADCASTER(("Broadcaster: sinkBroadcasterDestroyBroadcaster\n"));

    /* Stop the Broadcast */
    sinkBroadcasterStopBroadcast(broadcast_mode_auto);

    /* Free the SCMB memory which was allocated as part of init */
    BroadcastCmdScmDestroy();

    /* De-initialize Broadcast library */
    BroadcastDestroy(BROADCASTER->broadcaster);

    sinkBroadcastReleaseVolumeTable();
}

/******************************************************************************/
static void broadcasterHandleBlDeInitCfm(Task task, BROADCAST_DESTROY_CFM_T* cfm)
{
    UNUSED(task);

    if (cfm->status == broadcast_success)
    {
        /* De-initialize the Broadcaster Global instance */
        free(BROADCASTER);
        BROADCASTER = NULL;
        BroadcastContextSetEncryptionConfig(NULL);
        sinkBroadcastAudioHandleDeInitCfm(sink_ba_appmode_broadcaster);
    }
}

void sinkBroadcasterHandleBroadcastMessage(Task task, MessageId id, Message message)
{
    switch (id)
    {
        case BROADCAST_INIT_CFM:
            broadcasterHandleBlInitCfm(task,(BROADCAST_INIT_CFM_T*)message);
            break;

        case BROADCAST_DESTROY_CFM:
            broadcasterHandleBlDeInitCfm(task, (BROADCAST_DESTROY_CFM_T*)message);
            break;

        case BROADCAST_CONFIGURE_CFM:
            broadcasterHandleBlConfigureCfm(task,(BROADCAST_CONFIGURE_CFM_T*)message);
            break;

        case BROADCAST_START_BROADCAST_CFM:
            broadcasterHandleBlStartBroadcastCfm(task,(BROADCAST_START_BROADCAST_CFM_T*)message);
            break;

        case BROADCAST_STATUS_IND:
            DEBUG_BROADCASTER(("Broadcaster: BROADCAST_STATUS_IND: %x\n", ((BROADCAST_STATUS_IND_T*)message)->status ));
            break;

        case BROADCAST_STOP_BROADCAST_CFM:
            broadcasterHandleBlStopBroadcastCfm(task,(BROADCAST_STOP_BROADCAST_CFM_T*)message);
            break;
        
        case BROADCAST_STOP_BROADCAST_IND:
            broadcasterHandleBlStopBroadcastInd(task,(BROADCAST_STOP_BROADCAST_IND_T*)message);
            break;

        case BROADCAST_AFH_UPDATE_IND:
            broadcasterHandleBlAfhUpdateInd(task,(BROADCAST_AFH_UPDATE_IND_T*)message);
            break;

        case BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND:
            broadcasterHandleBlAfhChannelMapChangeInd(task,(BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND_T*)message);
            break;

        default:
            DEBUG_BROADCASTER(("Broadcaster: Unhandled Broadcast message 0x%x\n", id));
            break;
    }
}

/***************************************************************************
NAME
    BroadcasterMessageHandler

DESCRIPTION
    Application Message Handler for Broadcaster Role.

PARAMS
    task Application Task
    id Message ID
    Message Message to application Broadcaster task

RETURNS
    void
*/
static void BroadcasterMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    /* No need to handle any messages if we are not in proper role */
    if(!BA_BROADCASTER_MODE_ACTIVE)
    {
        /* We could have received this for switching to standalone mode */
        if(id == BROADCASTER_INTERNAL_MSG_STOP_BLE_BROADCAST)
            sinkBleStopBroadcastEvent();
        return;
    }

   /* handle Broadcast messages */
    if ((id >= BROADCAST_MESSAGE_BASE) && (id < BROADCAST_MESSAGE_TOP))
    {
        sinkBroadcasterHandleBroadcastMessage(sinkBroadcastAudioGetHandler(), id, message);
        return;
    }

    /* handle SCM messages */
    if ((id >= BROADCAST_CMD_MESSAGE_BASE) && (id < BROADCAST_CMD_MESSAGE_TOP))
    {
        sinkBroadcasterHandleBcmdMessage(sinkBroadcastAudioGetHandler(), id, message);
        return;
    }

    if(sinkBroadcasterIsItPacketiserMessage(id))
    {
        bp_msg_handler_result_t event  = sinkBroadcasterHandlePacketiserMessages(id, message);
        switch(event)
        {
            case BROADCASTER_BP_INIT_SUCCESS:
                handleBroadcastIsReadyForAudio();
                break;

            default:
                break;
        }
        return;
    }

    switch(id)
    {
        case BROADCASTER_INTERNAL_MSG_START_BROADCAST:
            sinkBroadcasterStartBroadcast(broadcast_mode_auto);
        break;

        case BROADCASTER_INTERNAL_MSG_STOP_BROADCASTER:
            sinkBroadcasterStopBroadcast(broadcast_mode_auto);
        break;

        case BROADCASTER_INTERNAL_MSG_DESTROY_BROADCASTER:
            sinkBroadcasterDestroyBroadcaster();
        break;

        default:
            DEBUG_BROADCASTER(("BA:Error!!! BroadcasterMessageHandler() Unknown Message\n" ));
        break;

    }
}

/***************************************************************************
NAME
    broadcasterLoadStoredLtAddress

DESCRIPTION
    Utility function to retrieve the Broadcaster LT Address from configuration

PARAMS
    void

RETURNS
    bool TRUE if retrieved LT Address is Valid else FALSE
*/
static uint8 broadcasterLoadStoredLtAddress(void)
{
    ba_config_def_t* rw_config_data = NULL;
    uint8 lt_addr;

    if(configManagerGetReadOnlyConfig(BA_CONFIG_BLK_ID, (const void **)&rw_config_data))
    {
       lt_addr = (uint8)(rw_config_data->broadcaster_lt_addr);
       configManagerReleaseConfig(BA_CONFIG_BLK_ID);

       return lt_addr;
    }
    return LT_ADDR_DEFAULT;
}

/***************************************************************************
NAME
    broadcasterIsContinuousSyncTrainSet

DESCRIPTION
    Check whether continuous sync train is Enabled for Broadcaster.

PARAMS
    void

RETURNS
    bool TRUE if Sync Train is Enabled else FALSE
*/
static bool broadcasterIsContinuousSyncTrainSet(void)
{
    ba_config_def_t* ro_config_data = NULL;
    bool result = FALSE;

    if (configManagerGetReadOnlyConfig(BA_CONFIG_BLK_ID, (const void **)&ro_config_data))
    {
        if(ro_config_data->continuous_sync_train)
            result = TRUE;
        configManagerReleaseConfig(BA_CONFIG_BLK_ID);
    }
    return result;
}

/*******************************************************************************/
void sinkBroadcasterSetMasterRole( uint16 device_id)
{
    /* Make sure we are master of link */
    ConnectionSetRole(sinkGetMainTask(), A2dpSignallingGetSink(device_id), hci_role_master);
}

#endif /* ENABLE_BROADCAST_AUDIO*/


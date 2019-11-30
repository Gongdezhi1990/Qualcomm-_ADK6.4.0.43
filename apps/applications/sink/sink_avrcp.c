/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
\file    
\ingroup sink_app
\brief   
    Handles AVRCP control channel functionality. By default it supports AVRCP 1.5 mandatory features, operating as Category 1 Controller
    and Category 2 Target device.  When Peer device support is included, the application will operate as a Category 1 Target also.
    
    In order to support advanced optional features, additional defines need to be specified:
        - Defining ENABLE_AVRCP_NOW_PLAYING in sink_avrcp.h will enable Now Playing functionality of AVRCP 1.5 specification 
        - Defining ENABLE_AVRCP_PLAYER_APP_SETTINGS in sink_avrcp.h will enable Player Application Settings functionality of AVRCP 1.5 specification 
        - Browsing is handled separately in sink_avrcp_browsing and enabled by defining ENABLE_AVRCP_BROWSING in project properties
*/

#ifdef ENABLE_AVRCP
#include "sink_audio.h"
#include "sink_main_task.h"
#include "sink_anc.h"
#include "sink_avrcp.h"
#include "sink_avrcp_browsing.h"
#include "sink_avrcp_browsing_channel.h"
#include "sink_avrcp_qualification.h"
#include "sink_debug.h"
#include "sink_statemanager.h"
#include "sink_link_policy.h"
#include "sink_partymode.h"
#include "sink_a2dp.h"
#include "sink_peer.h"
#include "sink_events.h"
#include "sink_led_manager.h"
#include "sink_ba.h"
#include "sink_music_processing.h"
#include "sink_powermanager.h"
#include "sink_tones.h"
#include "sink_usb.h"
#include "sink_wired.h"
#include "sink_devicemanager.h"

#include "sink_auto_power_off.h"

/* Include config store and definition headers */
#include "config_definition.h"
#include "sink_avrcp_config_def.h"
#include <config_store.h>

#include "sink_display.h"
#include "display_plugin_if.h"

#include <gain_utils.h>
#include <audio.h>

#include <bdaddr.h>
#include <source.h>
#include <string.h>
#include <vm.h>
#include <vmal.h>


#ifdef DEBUG_AVRCP
#define AVRCP_DEBUG(x) DEBUG(x)
#else
#define AVRCP_DEBUG(x) 
#endif


typedef struct
{
    TaskData avrcp_ctrl_handler[MAX_AVRCP_CONNECTIONS];
    SinkAvrcpCleanUpTask        dataCleanUpTask[MAX_AVRCP_CONNECTIONS];
    unsigned active_avrcp:2;
    unsigned avrcp_manual_connect:1;
    unsigned avrcp_initialised:1;
    unsigned play_displayed:1;
    unsigned media_info_displayed:1;
    unsigned unused:10;
    uint16 extensions[MAX_AVRCP_CONNECTIONS];
    uint16 features[MAX_AVRCP_CONNECTIONS];
    uint16 event_capabilities[MAX_AVRCP_CONNECTIONS];
    bool connected[MAX_AVRCP_CONNECTIONS];
    AVRCP *avrcp[MAX_AVRCP_CONNECTIONS];
    bool pending_cmd[MAX_AVRCP_CONNECTIONS];
    uint16 cmd_queue_size[MAX_AVRCP_CONNECTIONS];
    bdaddr bd_addr[MAX_AVRCP_CONNECTIONS];
    uint16 registered_events[MAX_AVRCP_CONNECTIONS];
    uint16 play_status[MAX_AVRCP_CONNECTIONS];
    uint16 absolute_volume[MAX_AVRCP_CONNECTIONS];
    bdaddr avrcp_play_addr;
    bool   link_active[MAX_AVRCP_CONNECTIONS];
    uint8 *vendor_data[MAX_AVRCP_CONNECTIONS];
    uint16 browsing_channel[MAX_AVRCP_CONNECTIONS];   
    bool is_peer[MAX_AVRCP_CONNECTIONS];
#ifdef ENABLE_AVRCP_BROWSING
    TaskData avrcp_browsing_handler[MAX_AVRCP_CONNECTIONS];
    uint16 uid_counter[MAX_AVRCP_CONNECTIONS];
    uint16 media_player_features[MAX_AVRCP_CONNECTIONS];
    uint16 media_player_id[MAX_AVRCP_CONNECTIONS];
    uint16 browsing_scope[MAX_AVRCP_CONNECTIONS];
#endif
    Source last_source;
} avrcp_data;

static avrcp_data gAvrcpData;

/* Global data for AVRCP */
#define AVRCP_DATA   gAvrcpData

static bool isAvrcpPlaybackStatusSupported(uint16 Index);
static void sinkAvrcpSetActiveConnectionFromIndex(uint16 new_index);
static void sinkAvrcpFlushAndResetStandardCommands(uint16 index);
static void sinkAvrcpResetVendorCommands(uint16 index);
static void sinkAvrcpSetLocalVolume(const uint16 Index, const uint16 a2dp_step);
static void sinkAvrcpUpdateActiveConnection(void);

/*********************** Const data ***************************/

/* media attributes to retrieve from TG - this is only limited track data that can be filtered down to only return the required information */
const uint8 avrcp_retrieve_media_attributes_basic[AVRCP_NUMBER_MEDIA_ATTRIBUTES_BASIC * 4] = {
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_TITLE,
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_ARTIST,
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_ALBUM};

/* media attributes to retrieve from TG - this is all the track data that can be requested */
const uint8 avrcp_retrieve_media_attributes_full[AVRCP_NUMBER_MEDIA_ATTRIBUTES_FULL * 4] = {
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_TITLE,
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_ARTIST,
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_ALBUM,
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_NUMBER,
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_TOTAL_NUMBER,
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_GENRE,
                                                        0, 0, 0, AVRCP_MEDIA_ATTRIBUTE_PLAYING_TIME};

#ifdef ENABLE_PEER
static const avrcp_controls avrcp_map_opid_to_ctrl[opid_backward-opid_power+1][2] =
{
    { AVRCP_CTRL_NOP,            AVRCP_CTRL_NOP              },    /* opid_power        */
    { AVRCP_CTRL_NOP,            AVRCP_CTRL_NOP              },    /* opid_volume_up    */
    { AVRCP_CTRL_NOP,            AVRCP_CTRL_NOP              },    /* opid_volume_down  */
    { AVRCP_CTRL_NOP,            AVRCP_CTRL_NOP              },    /* opid_mute         */
    { AVRCP_CTRL_PLAY_PRESS,     AVRCP_CTRL_PLAY_RELEASE     },    /* opid_play         */
    { AVRCP_CTRL_STOP_PRESS,     AVRCP_CTRL_STOP_RELEASE     },    /* opid_stop         */
    { AVRCP_CTRL_PAUSE_PRESS,    AVRCP_CTRL_PAUSE_RELEASE    },    /* opid_pause        */
    { AVRCP_CTRL_NOP,            AVRCP_CTRL_NOP              },    /* opid_record       */
    { AVRCP_CTRL_REW_PRESS,      AVRCP_CTRL_REW_RELEASE      },    /* opid_rewind       */
    { AVRCP_CTRL_FF_PRESS,       AVRCP_CTRL_FF_RELEASE       },    /* opid_fast_forward */
    { AVRCP_CTRL_NOP,            AVRCP_CTRL_NOP              },    /* opid_eject        */
    { AVRCP_CTRL_FORWARD_PRESS,  AVRCP_CTRL_FORWARD_RELEASE  },    /* opid_forward      */
    { AVRCP_CTRL_BACKWARD_PRESS, AVRCP_CTRL_BACKWARD_RELEASE }     /* opid_backward     */
};
#endif

#define sinkAvrcpConvertAvrcpVolumeToA2dpStep(avrcp_volume)     (((avrcp_volume * sinkVolumeGetMaxVolumeStep(audio_output_group_main)) / AVRCP_MAX_ABS_VOL))
#define sinkAvrcpConvertA2dpStepToAvrcpVolume(a2dp_step)        (((a2dp_step * AVRCP_MAX_ABS_VOL)/sinkVolumeGetMaxVolumeStep(audio_output_group_main)))

/*********************** Helper Functions ***************************/
static void sendVolumeEvent(sinkEvents_t event)
{
    if (sinkTonesIsPlayLocalVolumeTone())
    {
        sinkSend(event);
    }
}

static bool getA2dpIndexFromAvrcp (uint16 avrcpIndex, a2dp_index_t *a2dpIndex)
{
    if (SinkA2dpIsInitialised() && sinkAvrcpIsInitialised() &&  AVRCP_DATA.connected[avrcpIndex])
    {
        for_all_a2dp(*a2dpIndex)
        {
            if (getA2dpStatusFlag(CONNECTED, *a2dpIndex) && 
                BdaddrIsSame(getA2dpLinkBdAddr(*a2dpIndex), & AVRCP_DATA.bd_addr[avrcpIndex]))
            {
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

static void updateDisplay (void)
{
    uint16 activeIndex = sinkAvrcpGetActiveConnection();
    bool nowPlaying = (sinkavrcp_play_status_playing == AVRCP_DATA.play_status[activeIndex]);
    bool trackActive = nowPlaying || (sinkavrcp_play_status_paused == AVRCP_DATA.play_status[activeIndex])
                            || (sinkavrcp_play_status_fwd_seek == AVRCP_DATA.play_status[activeIndex])
                            || (sinkavrcp_play_status_rev_seek == AVRCP_DATA.play_status[activeIndex]);
    if (nowPlaying)
    {
        uint16 index;
#ifdef ENABLE_PEER
        uint16 srcIndex;
#endif
        uint16 avrcpIndex;
        /* check whether the a2dp connection is present and streaming data and that the audio is routed */
        if( !(sinkAudioIsAudioRouted()
                && getA2dpIndexFromSink(sinkAudioGetRoutedAudioSink(), &index)
                && (A2dpMediaGetState(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)) == a2dp_stream_streaming)
#ifdef ENABLE_PEER
                && a2dpGetSourceIndex(&srcIndex)
                && (srcIndex == index)
#endif
                && sinkAvrcpGetIndexFromBdaddr(getA2dpLinkBdAddr(index), &avrcpIndex, TRUE) ) )
            nowPlaying = FALSE;
    }
    if (nowPlaying != AVRCP_DATA.play_displayed)
    {
        AVRCP_DATA.play_displayed = nowPlaying;
        if (nowPlaying)
            displayShowText(DISPLAYSTR_A2DPSTREAMING, strlen(DISPLAYSTR_A2DPSTREAMING), DISPLAY_TEXT_SCROLL_SCROLL, 500, 1000, FALSE, 15, SINK_TEXT_TYPE_PLAY_STATE);
        else
            displayRemoveText(SINK_TEXT_TYPE_PLAY_STATE);
    }
    if ((!trackActive) && AVRCP_DATA.media_info_displayed)
    {
        displayRemoveText(SINK_TEXT_TYPE_MEDIA_INFO);
        AVRCP_DATA.media_info_displayed = FALSE;
    }
}

static void setPlayStatus (uint16 index, SinkAvrcp_play_status newStatus)
{
    uint16 activeIndex = sinkAvrcpGetActiveConnection();

    AVRCP_DEBUG(("AVRCP: play_status[%d] %d -> %d\n", index, AVRCP_DATA.play_status[index], newStatus));
    AVRCP_DATA.play_status[index] = newStatus;
    if (activeIndex == index)
        updateDisplay();
}

static void updateActiveConnection (uint16 newConnection)
{
    if (newConnection<MAX_AVRCP_CONNECTIONS && newConnection!=AVRCP_DATA.active_avrcp)
    {
        AVRCP_DEBUG(("AVRCP: update active connection %d -> %d\n", AVRCP_DATA.active_avrcp, newConnection));
        AVRCP_DATA.active_avrcp = newConnection;
        updateDisplay();
    }
}

static void setAvrcpIsPeer(uint16 avrcpIndex, bool is_peer)
{
    AVRCP_DATA.is_peer[avrcpIndex] = is_peer;
}

static bool getAvrcpIsPeer(uint16 avrcpIndex)
{
    return AVRCP_DATA.is_peer[avrcpIndex];
}

/*************************************************************************
NAME    
    getNextConnectedAvrcpIndex
    
DESCRIPTION
    Returns the next connected AVRCP index if found.

**************************************************************************/
static bool getNextConnectedAvrcpIndex(uint16 *IndexToUse , uint16 currentIndex)
{
    if (sinkAvrcpIsInitialised())
    {
        for_all_avrcp(*IndexToUse)
        {
            if ((*IndexToUse != currentIndex) && AVRCP_DATA.connected[*IndexToUse])
            {
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

#ifdef ENABLE_PEER
/*************************************************************************
NAME    
    sinkAvrcpIsAvrcpIndexPeer
    
DESCRIPTION

    Determine if the supplied AVRCP connection index is that of Peer device

**************************************************************************/
bool sinkAvrcpIsAvrcpIndexPeer (uint16 avrcpIndex, uint16 *peerIndex)
{
    uint16 a2dpIndex;
    
    if (SinkA2dpIsInitialised() && sinkAvrcpIsInitialised() && AVRCP_DATA.connected[avrcpIndex])
    {
        for_all_a2dp(a2dpIndex)
        {
            if (getA2dpStatusFlag(CONNECTED, a2dpIndex) && 
                (getA2dpPeerRemoteDevice(a2dpIndex) == remote_device_peer) &&
                BdaddrIsSame(getA2dpLinkBdAddr(a2dpIndex), & AVRCP_DATA.bd_addr[avrcpIndex]))
            {
                if (peerIndex)
                {
                    *peerIndex = a2dpIndex;
                }
                
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

/*************************************************************************
NAME    
    getAvAvrcpIndex
    
DESCRIPTION

    Obtain index of AVRCP connection to a connected AV Source (AG)

**************************************************************************/
static bool getAvAvrcpIndex (uint16 *avIndex)
{
    uint16 a2dpIndex;
    uint16 avrcp_index;
    
    if (SinkA2dpIsInitialised() && sinkAvrcpIsInitialised())
    {
        for_all_a2dp(a2dpIndex)
        {
            if (getA2dpStatusFlag(CONNECTED, a2dpIndex) && (getA2dpPeerRemoteDevice(a2dpIndex) == remote_device_nonpeer))
            {
                for_all_avrcp(avrcp_index)
                {
                    if ( AVRCP_DATA.connected[avrcp_index] && (BdaddrIsSame(getA2dpLinkBdAddr(a2dpIndex), &AVRCP_DATA.bd_addr[avrcp_index])))
                    {
                        if(avIndex)
                        {
                            *avIndex = avrcp_index;
                        }                        
                        return TRUE;
                    }
                }
            }
        }
    }
    
    return FALSE;
}

/*************************************************************************
NAME    
    avrcpGetPeerIndex
    
DESCRIPTION

    Obtain index of AVRCP connection to a connected Peer device

**************************************************************************/
bool avrcpGetPeerIndex (uint16 *peer_index)
{
    uint16 a2dp_index;
    uint16 avrcp_index;
    
    if (SinkA2dpIsInitialised() && sinkAvrcpIsInitialised())
    {
        for_all_a2dp(a2dp_index)
        {
            if (getA2dpStatusFlag(CONNECTED, a2dp_index) && (getA2dpPeerRemoteDevice(a2dp_index) == remote_device_peer))
            {
                for_all_avrcp(avrcp_index)
                {
                    if (AVRCP_DATA.connected[avrcp_index] && (BdaddrIsSame(getA2dpLinkBdAddr(a2dp_index), &AVRCP_DATA.bd_addr[avrcp_index])))
                    {
                        if (peer_index)
                        {
                            *peer_index = avrcp_index;
                        }
                        
                        return TRUE;
                    }
                }
            }
        }
    }
    
    return FALSE;
}

/*************************************************************************
NAME    
    getValidAvrcpIndex
    
DESCRIPTION
    Returns the valid AVRCP index if found.

**************************************************************************/

static bool getValidAvrcpIndex(uint16 *IndexToUse , uint16 currentIndex)
{
    uint16 peerIndex;
    bool control_peer_audio = TRUE;
    
    if (peerIsSingleDeviceOperationEnabled())
    {
        /* For TWS Single Device Mode it is only valid to send an AVRCP command to the remote peer device
           for A2DP and USB audio */
        control_peer_audio = peerGetRemotePeerAudioConnectionStatus() & (A2DP_AUDIO | USB_AUDIO);
    }
    
    if( AVRCP_DATA.connected[currentIndex] && 
       ((!sinkAvrcpIsAvrcpIndexPeer(currentIndex, &peerIndex)) || control_peer_audio))
    {
        /* Use this index as we have made sure that either it's a true AV device or a remote device with a true Av connected*/
        *IndexToUse = currentIndex;
    }
    else
    {
        /* Find another suitable index and update the active connection */
        if(!getNextConnectedAvrcpIndex(IndexToUse , currentIndex))
        {
            /* Couldn't find next valid index */
            AVRCP_DEBUG(("AVRCP: Unable to find IndexToUse\n"));
            return FALSE;
        }
    }
    
    AVRCP_DEBUG(("AVRCP: IndexToUse=%u\n",*IndexToUse));
    return TRUE;
}

#ifdef ENABLE_PEER
/*************************************************************************
NAME
    avrcpUpdatePeerLowBatteryStatus

DESCRIPTION
    Updates the peer master device of the current low battery condition

**************************************************************************/
void avrcpUpdatePeerLowBatteryStatus (bool low_battery_limit_applied)
{
    uint16 peerAvrcpIndex;

    /* If a peer is connected, then send the avrcp vendor dependant command to the peer device updating the audio connection status */
    if(avrcpGetPeerIndex(&peerAvrcpIndex) && AVRCP_DATA.connected[peerAvrcpIndex])
    {

        /* Inform remote peer of change to local devices status */
        sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex,
                                                AVRCP_PEER_CMD_TWS_VOLUME_LOW_BAT_LIMIT,
                                                AVRCP_PAYLOAD_PEER_CMD_TWS_VOLUME_LOW_BAT,
                                                (const uint8 *)&low_battery_limit_applied );
    }
}
#endif

/*************************************************************************
NAME    
    updatePeerSourceConnectionStatus
    
DESCRIPTION
    Updates the Audio connection status to the peer device 

**************************************************************************/
static void updatePeerSourceConnectionStatus (AudioSrc audio_source, uint8 is_connected, const bdaddr * bd_addr)
{
    uint16 peerAvrcpIndex;
        
    /* If a peer is connected, then send the avrcp vendor dependant command to the peer device updating the audio connection status */
    if(avrcpGetPeerIndex (&peerAvrcpIndex) && AVRCP_DATA.connected[peerAvrcpIndex])
    {
        audio_src_conn_state_t  audioSrcStatus;
        
        memset(&audioSrcStatus, 0, sizeof(audio_src_conn_state_t));
        audioSrcStatus.src = audio_source;
        audioSrcStatus.isConnected = is_connected;

        if (bd_addr)
        {   
            /* Convert the BD Addess from the type bdaddr to byte_aligned_bd_addr_t for sending */
            /* it across to the peer device through the vendor unique AVRCP passthrough command */
            audioSrcStatus.bd_addr.lap[0] = bd_addr->lap & 0xFF;
            audioSrcStatus.bd_addr.lap[1] = (bd_addr->lap >>8)  & 0xFF;
            audioSrcStatus.bd_addr.lap[2] = (bd_addr->lap >>16) & 0xFF;
            audioSrcStatus.bd_addr.uap = bd_addr->uap;
            audioSrcStatus.bd_addr.nap[0] = bd_addr->nap & 0xFF;
            audioSrcStatus.bd_addr.nap[1] = (bd_addr->nap >> 8) & 0xFF ;
        }
   
        /* Inform remote peer of change to local devices status */
        sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex, 
                                                AVRCP_PEER_CMD_AUDIO_CONNECTION_STATUS, 
                                                sizeof(audio_src_conn_state_t), 
                                                (const uint8 *)&audioSrcStatus );
    }
}



/*************************************************************************
NAME    
    sinkAvrcpUpdatePeerWirelessSourceConnected
    
DESCRIPTION
    Informs a remote Peer that a local wireless audio source has connected

**************************************************************************/
void sinkAvrcpUpdatePeerWirelessSourceConnected (AudioSrc audio_source, const bdaddr * bd_addr)
{
    updatePeerSourceConnectionStatus( audio_source, TRUE, bd_addr);
}

/*************************************************************************
NAME    
    sinkAvrcpUpdatePeerWiredSourceConnected
    
DESCRIPTION
    Informs a remote Peer that a local wired audio source has connected

**************************************************************************/
void sinkAvrcpUpdatePeerWiredSourceConnected (AudioSrc audio_source)
{
    updatePeerSourceConnectionStatus( audio_source, TRUE, NULL);
}

/*************************************************************************
NAME    
    sinkAvrcpUpdatePeerSourceDisconnected
    
DESCRIPTION
    Informs a remote Peer that a local audio source has disconnected

**************************************************************************/
void sinkAvrcpUpdatePeerSourceDisconnected (AudioSrc audio_source)
{
    updatePeerSourceConnectionStatus( audio_source, FALSE, NULL);
}


#endif

/*************************************************************************
NAME    
    avrcpSendControlMessage
    
DESCRIPTION
    Queue up message to send an AVRCP command based on pending_cmd flag.
    The flag is used so that a response to an AVRCP command must be received 
    before another AVRCP command can be sent.

**************************************************************************/
void avrcpSendControlMessage(avrcp_controls control, uint16 Index)
{
    if (control != AVRCP_CTRL_NOP)
    {
        if( (control == AVRCP_CTRL_FF_RELEASE)  || (control == AVRCP_CTRL_REW_RELEASE) )
        {
            /* Check the playstatus of the currently active connection before issuing these commands */
            if( ! ((AVRCP_DATA.play_status[Index] & sinkavrcp_play_status_fwd_seek) ||    
                   (AVRCP_DATA.play_status[Index] & sinkavrcp_play_status_rev_seek) ))
            {
                /*If there is no suitable AVRCP device to process this command then just return */
                if (!isAvrcpPlaybackStatusSupported(Index))
                {
                    setPlayStatus(Index, sinkavrcp_play_status_error);
                    return;
                }                
            }
        }

        {
            MAKE_AVRCP_MESSAGE(AVRCP_CONTROL_SEND);
            message->control = control;
            message->index = Index;
            MessageSendConditionally(&AVRCP_DATA.avrcp_ctrl_handler[Index], AVRCP_CONTROL_SEND, 
                message, (uint16 *) &AVRCP_DATA.pending_cmd[Index]);
        }
        
        if (AVRCP_DATA.cmd_queue_size[Index] < AVRCP_MAX_PENDING_COMMANDS)
            AVRCP_DATA.cmd_queue_size[Index]++;
        
        AVRCP_DEBUG(("    AVRCP pending commands %d\n", AVRCP_DATA.cmd_queue_size[Index])); 
    }
}


/*************************************************************************
NAME    
    sendAvrcpPassthroughCmd
    
DESCRIPTION
    Sends an AVRCP PASSTHROUGH command. Sets the pending_cmd flag so that the device waits
    for a response before sending another command.

**************************************************************************/
static void sendAvrcpPassthroughCmd(avc_operation_id op_id, uint8 state, uint16 Index)
{
    AVRCP_DATA.pending_cmd[Index] = TRUE;
    if (AVRCP_DATA.cmd_queue_size[Index])
        AVRCP_DATA.cmd_queue_size[Index]--;  
    /* Send a key press */
    AvrcpPassthroughRequest(AVRCP_DATA.avrcp[Index], subunit_panel, 0, state, op_id, 0, 0);   
}

/*************************************************************************
NAME    
    sendAvrcpGroupCmd
    
DESCRIPTION
    Sends an AVRCP Group Navigation command. Sets the pending_cmd flag so that the device waits
    for a response before sending another command.

**************************************************************************/
static void sendAvrcpGroupCmd(bool forward, uint16 Index, bool state)
{
    AVRCP_DATA.pending_cmd[Index] = TRUE;
    if (AVRCP_DATA.cmd_queue_size[Index])
        AVRCP_DATA.cmd_queue_size[Index]--;
    /* Send appropriate group command */
    if (forward)
        AvrcpNextGroupRequest(AVRCP_DATA.avrcp[Index], state);
    else
        AvrcpPreviousGroupRequest(AVRCP_DATA.avrcp[Index], state);
}

/*************************************************************************
NAME    
    sendAvrcpVendorOpId
    
DESCRIPTION
    Sends an AVRCP PASSTHROUGH REQUEST command with vendor opid. Sets the pending_cmd flag so that the device waits
    for a response before sending another command.

**************************************************************************/
static void sendAvrcpVendorOpId(uint16 cmd_id, uint16 size_data, const uint8 *data, uint16 Index)
{
    Source vendor_source;

    /* Need to retain this data until AVRCP library informs us that it has been sent */
    AVRCP_DATA.vendor_data[Index] = (uint8 *)malloc(VENDOR_CMD_TOTAL_SIZE(size_data));

    if (!AVRCP_DATA.vendor_data[Index])
    {
        return;
    }
        
    AVRCP_DATA.vendor_data[Index][0] = (uint8)(cmd_id >> 8);
    AVRCP_DATA.vendor_data[Index][1] = (uint8)(cmd_id & 0xFF);
        
    if(size_data != 0)
    {
       memcpy(&AVRCP_DATA.vendor_data[Index][2], data, size_data); 
    }
        
    vendor_source = StreamRegionSource( AVRCP_DATA.vendor_data[Index], VENDOR_CMD_TOTAL_SIZE(size_data) );

    AVRCP_DATA.pending_cmd[Index] = TRUE;
    AVRCP_DATA.last_source = vendor_source;
    AvrcpPassthroughRequest(AVRCP_DATA.avrcp[Index], 
                                subunit_panel, 
                                0, 
                                0, 
                                opid_vendor_unique, 
                                VENDOR_CMD_TOTAL_SIZE(size_data), 
                                vendor_source);
}

/*************************************************************************
NAME    
    sendAvrcpAbortContinuingCmd
    
DESCRIPTION
    Sends an AVRCP Abort Continuation command. There is no response so this command is just sent directly.

**************************************************************************/
static void sendAvrcpAbortContinuingCmd(uint16 Index, uint16 pdu_id)
{
    AvrcpAbortContinuingResponseRequest(AVRCP_DATA.avrcp[Index], pdu_id);
    
    /* An AVRCP fragmented response has been aborted. This will need to be handled if expecting more data to arrive */
}

/*************************************************************************
NAME    
    getAvrcpQueueSpace
    
DESCRIPTION
    Gets the free space in the AVRCP command queue

RETURNS
    Returns the number of AVRCP commands that can be queued.
**************************************************************************/
uint16 getAvrcpQueueSpace(uint16 Index)
{
    return AVRCP_MAX_PENDING_COMMANDS - AVRCP_DATA.cmd_queue_size[Index];
}

/*************************************************************************
NAME    
    getNumOfAvrcpCmdQueued
    
DESCRIPTION
    Gets the numbers of commands in the AVRCP command queue

RETURNS
    Returns the number of AVRCP commands that are being queued.
**************************************************************************/
uint16 getNumOfAvrcpCmdQueued(uint16 Index)
{
    return AVRCP_DATA.cmd_queue_size[Index];
}

/*************************************************************************
NAME    
    isAvrcpCategory1MetadataEnabled
    
DESCRIPTION
    Checks that the remote device has Category 1 Metadata enabled. This covers the Metadata 
    that was introduced in the AVRCP 1.3 specification.

RETURNS
    Returns TRUE if Category 1 Metadata enabled, FALSE otherwise.
**************************************************************************/
static bool isAvrcpCategory1MetadataEnabled(uint16 Index)
{
    if ((AVRCP_DATA.features[Index] & AVRCP_CATEGORY_1) && (AVRCP_DATA.extensions[Index] & AVRCP_VERSION_1_3))
        return TRUE;

    return FALSE;
}

/*************************************************************************
NAME    
    isAvrcpPlaybackStatusSupported
    
DESCRIPTION
    Checks that the remote device supports playback status changed notifications.

RETURNS
    Returns TRUE if supported, FALSE otherwise.
**************************************************************************/
static bool isAvrcpPlaybackStatusSupported(uint16 Index)
{
    if (AVRCP_DATA.registered_events[Index] & (1 << avrcp_event_playback_status_changed))
        return TRUE;

    return FALSE;
}

/*************************************************************************
NAME    
    isAvrcpGroupNavigationEnabled
    
DESCRIPTION
    Checks that the remote device has Group Navigation enabled.

RETURNS
    Returns TRUE if Group Navigation enabled, FALSE otherwise.
**************************************************************************/
static bool isAvrcpGroupNavigationEnabled(uint16 Index)
{
    if ((AVRCP_DATA.features[Index] & AVRCP_GROUP_NAVIGATION) == AVRCP_GROUP_NAVIGATION)
    {
        if(sinkAvrcpBrowsingIsSupported(Index))
        {
            if(sinkAvrcpBrowsingIsGroupNavigationSupported(Index))
                return TRUE;
        }
        else
        {
            return TRUE;
        }
    }
    return FALSE;
}

#ifdef ENABLE_AVRCP_PLAYER_APP_SETTINGS
/*************************************************************************
NAME    
    isAvrcpPlayerApplicationSettingsSupported
    
DESCRIPTION
    Checks that both ends support Player Application Settings.

RETURNS
    Returns TRUE if supported, FALSE otherwise.
**************************************************************************/
static bool isAvrcpPlayerApplicationSettingsSupported(uint16 Index)
{

    if ((AVRCP_DATA.features[Index] & AVRCP_PLAYER_APPLICATION_SETTINGS) == AVRCP_PLAYER_APPLICATION_SETTINGS)
        return TRUE;

    return FALSE;
}
#endif    


/*********************** AVRCP Local Message Handling ***************************/


/*************************************************************************
NAME    
    avrcpControlHandler
    
DESCRIPTION
    Handles all application created AVRCP messages associated with a certain AVRCP connection.

**************************************************************************/
static void avrcpControlHandler(Task task, MessageId id, Message message)
{    
    UNUSED(task);

    if (id == AVRCP_CREATE_CONNECTION)
    {
        uint16 Index;
        bdaddr addr = ((const AVRCP_CREATE_CONNECTION_T *)message)->bd_addr;

        AVRCP_DEBUG(("AVRCP_CREATE_CONNECTION :\n"));
        if (sinkAvrcpGetIndexFromBdaddr(&addr, &Index, FALSE))
        {
            /* reset bdaddr so the connection can be made to this device */
            BdaddrSetZero(&AVRCP_DATA.bd_addr[Index]);
        }
        sinkAvrcpConnect(&addr, DEFAULT_AVRCP_NO_CONNECTION_DELAY);
    }
    else if (id == AVRCP_CONTROL_SEND)
    {
        uint16 Index = ((const AVRCP_CONTROL_SEND_T *)message)->index;
        Sink sinkAG1,sinkAG2;
    
        /* obtain any sco sinks */
        HfpLinkGetAudioSink(hfp_primary_link, &sinkAG1);
        HfpLinkGetAudioSink(hfp_secondary_link, &sinkAG2);
            
        /* ignore if not connected */
        if (!AVRCP_DATA.connected[Index])
            return;

        /*Need not do power mode settings if within AVRCP_ACTIVE_MODE_INTERVAL
         assuming user is continuously sending controls */
        if(!(AVRCP_DATA.link_active[Index]))
        {   
            /*Check for active AVRCP alone scenarios and change to active mode*/        
            if((!sinkAG1 && !sinkAG2)&&
            (A2dpMediaGetState(getA2dpLinkDataDeviceId(a2dp_primary), getA2dpLinkDataStreamId(a2dp_primary))!= a2dp_stream_streaming)&&            
            (A2dpMediaGetState(getA2dpLinkDataDeviceId(a2dp_secondary), getA2dpLinkDataStreamId(a2dp_secondary))!= a2dp_stream_streaming))
            {
                uint16 *new_message = PanicUnlessNew(uint16);
                
                /* set AVRCP link policy settings */
                linkPolicyUseAvrcpSettings(AvrcpGetSink(AVRCP_DATA.avrcp[Index]));
                AVRCP_DATA.link_active[Index] =  TRUE;
                *new_message = Index;
                MessageSendLater(&theSink.task, EventSysResetAvrcpMode, new_message, D_SEC(AVRCP_ACTIVE_MODE_INTERVAL));
            }
        }

        
        AVRCP_DEBUG(("AVRCP_CONTROL_SEND :\n"));
        
        switch (((const AVRCP_CONTROL_SEND_T *)message)->control)
        {
            /* passthrough commands */
            case AVRCP_CTRL_PAUSE_PRESS:
                AVRCP_DEBUG(("  Sending Pause Pressed\n"));
                sendAvrcpPassthroughCmd(opid_pause, 0, Index);
                break;
            case AVRCP_CTRL_PAUSE_RELEASE:
                AVRCP_DEBUG(("  Sending Pause Released\n"));
                sendAvrcpPassthroughCmd(opid_pause, 1, Index);
                break;
            case AVRCP_CTRL_PLAY_PRESS:
                AVRCP_DEBUG(("  Sending Play Pressed\n"));
                sendAvrcpPassthroughCmd(opid_play, 0, Index);
                break;
            case AVRCP_CTRL_PLAY_RELEASE:
                AVRCP_DEBUG(("  Sending Play Released\n"));
                sendAvrcpPassthroughCmd(opid_play, 1, Index);
                break;
            case AVRCP_CTRL_FORWARD_PRESS:
                AVRCP_DEBUG(("  Sending Forward Pressed\n"));
                sendAvrcpPassthroughCmd(opid_forward, 0, Index);
                break;
            case AVRCP_CTRL_FORWARD_RELEASE:
                AVRCP_DEBUG(("  Sending Forward Released\n"));
                sendAvrcpPassthroughCmd(opid_forward, 1, Index);
                break;
            case AVRCP_CTRL_BACKWARD_PRESS:
                AVRCP_DEBUG(("  Sending Backward Pressed\n"));
                sendAvrcpPassthroughCmd(opid_backward, 0, Index);
                break;
            case AVRCP_CTRL_BACKWARD_RELEASE:
                AVRCP_DEBUG(("  Sending Backward Released\n"));
                sendAvrcpPassthroughCmd(opid_backward, 1, Index);
                break;
            case AVRCP_CTRL_STOP_PRESS:
                AVRCP_DEBUG(("  Sending Stop Pressed\n"));
                sendAvrcpPassthroughCmd(opid_stop, 0, Index);
                break;
            case AVRCP_CTRL_STOP_RELEASE:
                AVRCP_DEBUG(("  Sending Stop Released\n"));
                sendAvrcpPassthroughCmd(opid_stop, 1, Index);
                break;
            case AVRCP_CTRL_FF_PRESS:
                AVRCP_DEBUG(("  Sending FF Pressed\n"));
                sendAvrcpPassthroughCmd(opid_fast_forward, 0, Index);
                break;
            case AVRCP_CTRL_FF_RELEASE:
                AVRCP_DEBUG(("  Sending FF Released\n"));
                sendAvrcpPassthroughCmd(opid_fast_forward, 1, Index);
                break;
            case AVRCP_CTRL_REW_PRESS:
                AVRCP_DEBUG(("  Sending REW Pressed\n"));
                sendAvrcpPassthroughCmd(opid_rewind, 0, Index);
                break;
            case AVRCP_CTRL_REW_RELEASE:
                AVRCP_DEBUG(("  Sending REW Released\n"));
                sendAvrcpPassthroughCmd(opid_rewind, 1, Index);
                break;
            case AVRCP_CTRL_NEXT_GROUP_PRESS:
                AVRCP_DEBUG(("  Sending Next Group Press\n"));
                sendAvrcpGroupCmd(TRUE, Index, 0);
                break;
            case AVRCP_CTRL_NEXT_GROUP_RELEASE:
                AVRCP_DEBUG(("  Sending Next Group Release\n"));
                sendAvrcpGroupCmd(TRUE, Index, 1);
                break;
            case AVRCP_CTRL_PREVIOUS_GROUP_PRESS:
                AVRCP_DEBUG(("  Sending Previous Group Press\n"));
                sendAvrcpGroupCmd(FALSE, Index, 0);
                break;
            case AVRCP_CTRL_PREVIOUS_GROUP_RELEASE:
                AVRCP_DEBUG(("  Sending Previous Group Release\n"));
                sendAvrcpGroupCmd(FALSE, Index, 1);
                break;
#ifdef ENABLE_PEER
            case AVRCP_CTRL_VOLUME_UP_PRESS:
                AVRCP_DEBUG(("  Sending Volume Up Pressed\n"));
                sendAvrcpPassthroughCmd(opid_volume_up, 0, Index);
                break;
            case AVRCP_CTRL_VOLUME_UP_RELEASE:
                AVRCP_DEBUG(("  Sending Volume Up Released\n"));
                sendAvrcpPassthroughCmd(opid_volume_up, 1, Index);
                break;
            case AVRCP_CTRL_VOLUME_DOWN_PRESS:
                AVRCP_DEBUG(("  Sending Volume Down Pressed\n"));
                sendAvrcpPassthroughCmd(opid_volume_down, 0, Index);
                break;
            case AVRCP_CTRL_VOLUME_DOWN_RELEASE:
                AVRCP_DEBUG(("  Sending Volume Down Released\n"));
                sendAvrcpPassthroughCmd(opid_volume_down, 1, Index);
                break;
             case AVRCP_CTRL_POWER_OFF:
                AVRCP_DEBUG(("  Sending Power Off\n"));
                sendAvrcpPassthroughCmd(opid_power, 1, Index);
                break;                
#endif
            case AVRCP_CTRL_ABORT_CONTINUING_RESPONSE:
                AVRCP_DEBUG(("  Sending Abort Continuing Response\n"));
                sendAvrcpAbortContinuingCmd(Index, 
                            ((const AVRCP_CTRL_ABORT_CONTINUING_RESPONSE_T *)message)->pdu_id);
                break;
            default:
                break;
        }
    }
    else if (id == AVRCP_VENDOR_UNIQUE_ID)
    {
        AVRCP_DEBUG(("  Sending Vendor Unique Id Request "));
        sendAvrcpVendorOpId(((const SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ_T*)message)->cmd_id, 
                                              ((const SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ_T*)message)->size_data, 
                                              ((const SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ_T*)message)->data, 
                                              ((const SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ_T*)message)->avrcp_index);
    }
}


/*********************** AVRCP Library Message Handling Functions ***************************/

/******************/
/* INITIALISATION */
/******************/

/*************************************************************************
NAME    
    sinkAvrcpInitComplete
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_INIT_CFM from the AVRCP library.

**************************************************************************/
static void sinkAvrcpInitComplete(const AVRCP_INIT_CFM_T *msg)
{   
    /* check for successful initialisation of AVRCP libraray */
    if(msg->status == avrcp_success)
    {
        AVRCP_DEBUG(("  AVRCP Init Success [SDP handle 0x%lx]\n", msg->sdp_handle));
    }
    else
    {
        AVRCP_DEBUG(("    AVRCP Init Failed [Status %d]\n", msg->status));
        Panic();
    }
}

/******************************/
/* CONNECTION / DISCONNECTION */
/******************************/

/*************************************************************************
NAME    
    handleAvrcpConnectCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_CONNECT_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpConnectCfm(const AVRCP_CONNECT_CFM_T *msg)
{
    uint16 avrcpIndex = 0;
    uint16 a2dpIndex = 0;
    uint16 Index = 0;

#ifdef ENABLE_PEER
    uint16 avAvrcpIndex = 0;
    uint16 peerAvrcpIndex = 0;
#endif

    if(msg->status == avrcp_success)
    {
        AVRCP_DEBUG(("  AVRCP Connect Success [avrcp %p][bdaddr %x:%x:%lx][sink %p]\n",
                     (void*)msg->avrcp,
                     msg->bd_addr.nap, msg->bd_addr.uap, msg->bd_addr.lap,
                     (void*)msg->sink));

        /* If the device is off then disconnect */
        if (stateManagerGetState() == deviceLimbo)
        {
               AvrcpDisconnectRequest(msg->avrcp);
        }
        else
        {
            uint16 a2dp_volume;
            sink_attributes *attributes = PanicUnlessMalloc(sizeof(sink_attributes));

            /* Ensure the underlying ACL is encrypted */       
            ConnectionSmEncrypt(&theSink.task , msg->sink , TRUE);

            deviceManagerClearAttributes(attributes);
            if(deviceManagerGetAttributes(attributes, &msg->bd_addr))
            {
                 /* Make sure we store this device */
                attributes->profiles |= sink_avrcp;
                deviceManagerStoreAttributes(attributes, &msg->bd_addr);
            }
            free(attributes);

            /* Get the supported profile extensions */
            AvrcpGetProfileExtensions(msg->avrcp);
            
            /* find the instance to use for this connection */
            if (!sinkAvrcpGetIndexFromBdaddr(&msg->bd_addr, &avrcpIndex, FALSE))
            {
                /* address not in the list
                 * NB we don't expect this to happen because address will have
                 * been set in sinkAvrcpConnect()
                 */
                uint16 i = 0;
                AVRCP_DEBUG(("  can't find bdaddr\n"));
                for_all_avrcp(i)
                {
                    if (!AVRCP_DATA.connected[i])
                    {
                        AVRCP_DEBUG(("  found free %d\n", i));
                        break;
                    }
                }
                avrcpIndex = i; /* replace the disconnected device */
            }
            
            AVRCP_DEBUG(("  found index %d\n", avrcpIndex));
                
            if ((avrcpIndex < MAX_AVRCP_CONNECTIONS) && !AVRCP_DATA.connected[avrcpIndex])
            {
                AVRCP_DATA.connected[avrcpIndex] = TRUE;
                AVRCP_DATA.bd_addr[avrcpIndex] = msg->bd_addr;
                AVRCP_DATA.play_status[avrcpIndex] = sinkavrcp_play_status_stopped;
                AVRCP_DATA.avrcp[avrcpIndex] = msg->avrcp;
                 
                sinkAvrcpUpdateActiveConnection();
                       
                if (!sinkA2dpGetA2dpVolumeFromBdaddr(&AVRCP_DATA.bd_addr[avrcpIndex], &a2dp_volume))
                {
                    a2dp_volume = sinkA2dpGetDefaultVolumeInSteps();
                }
                
                sinkAvrcpSetLocalVolume(avrcpIndex, a2dp_volume);  
            
                /* set play status depending on streaming state */          
                for_all_a2dp(a2dpIndex)
                {            
                    if (getA2dpStatusFlag(CONNECTED, a2dpIndex) && BdaddrIsSame(getA2dpLinkBdAddr(a2dpIndex), &AVRCP_DATA.bd_addr[avrcpIndex]))
                    {
                        setAgAvrcpSupportStatus(a2dpIndex, avrcp_support_supported);
                        /* check whether the a2dp connection is streaming data */
                        if (A2dpMediaGetState(getA2dpLinkDataDeviceId(a2dpIndex), getA2dpLinkDataStreamId(a2dpIndex)) == a2dp_stream_streaming)
                        {
                            sinkAvrcpSetPlayStatusbyAddress(getA2dpLinkBdAddr(a2dpIndex), avrcp_play_status_playing);

                            /* AG audio may get routed before the AVRCP connection takes place.
                             * In that case, ensure the active avrcp link is set correctly once the AVRCP link connects */
                            if (sinkAudioIsAudioRouted() && getA2dpIndexFromSink(sinkAudioGetRoutedAudioSink(), &Index) && (Index == a2dpIndex))
                            {
                                sinkAvrcpSetActiveConnectionFromIndex(avrcpIndex);
                            }
                        }
#ifdef ENABLE_PEER                        
                        if (getA2dpPeerRemoteDevice(a2dpIndex) == remote_device_peer)
                        {
                            sinkA2dpHandlePeerAvrcpConnectCfm(a2dpIndex, TRUE);
                            sinkAncSychroniseStateWithPeer();
                            peerAvrcpConnect();
                            setAvrcpIsPeer(avrcpIndex, TRUE);
                        }
#endif                        
                        break;
                    } 
                }
                
#ifdef ENABLE_PEER  
                /* Find if there is a true AV device (non peer) device connected, if yes then send the notification to the peer*/
                if(avrcpGetPeerIndex (&peerAvrcpIndex))
                {                   
                    if(getAvAvrcpIndex(&avAvrcpIndex))
                    {
                        sinkAvrcpUpdatePeerWirelessSourceConnected(A2DP_AUDIO, &msg->bd_addr);
                    }
                    /* Check if any USB speaker class is connected and notify the peer. 
					   For USB connection enumerations then a headset would also be classed as a speaker device. */
                    if(usbSpeakerClassIsEnumerated())
                    {
                        /*If the USB source has connected then notify this to the peer device */
                        sinkAvrcpUpdatePeerWiredSourceConnected(USB_AUDIO);
                    }
                    /*Check if any Analog audio source is connected and notify the peer*/                    
                    if(analogAudioConnected())
                    {
                        /*If the Analogue wired audio has connected then notify this to the peer device */
                        sinkAvrcpUpdatePeerWiredSourceConnected(ANALOG_AUDIO);
                    }
                }
#endif                 
                
            }
        }        
    }
    else
    {
        AVRCP_DEBUG(("  AVRCP Connect Fail [status %d]\n", msg->status));
        
        if (sinkAvrcpGetIndexFromBdaddr(&msg->bd_addr, &avrcpIndex, FALSE))
        {
            BdaddrSetZero(&AVRCP_DATA.bd_addr[avrcpIndex]);
        }
        
        for_all_a2dp(a2dpIndex)
        {            
            /* test to see if A2DP connection exists so that AVRCP status can be updated */
            if (getA2dpStatusFlag(CONNECTED, a2dpIndex) && BdaddrIsSame(getA2dpLinkBdAddr(a2dpIndex), &msg->bd_addr))
            {
                if (getAgAvrcpSupportStatus(a2dpIndex) == avrcp_support_unknown)
                {
                    /* try AVRCP connection again */
                    setAgAvrcpSupportStatus(a2dpIndex, avrcp_support_second_attempt);
                    sinkAvrcpConnect(&msg->bd_addr, DEFAULT_AVRCP_2ND_CONNECTION_DELAY);
                    AVRCP_DEBUG(("      AVRCP 2nd attempt\n"));
                    return;
                }
                else
                {
                    /* give up on connecting AVRCP */
                    setAgAvrcpSupportStatus(a2dpIndex, avrcp_support_unsupported);
                    AVRCP_DEBUG(("      AVRCP unsupported\n"));
                }
                
#ifdef ENABLE_PEER                
                if (getA2dpPeerRemoteDevice(a2dpIndex) == remote_device_peer)
                {
                    sinkA2dpHandlePeerAvrcpConnectCfm(a2dpIndex, FALSE);
                }
#endif                
                break;
            } 
        }
    }
    
    if (AVRCP_DATA.avrcp_manual_connect && BdaddrIsSame(&AVRCP_DATA.avrcp_play_addr, &msg->bd_addr))
    {
        if (msg->status == avrcp_success)
        {
            AVRCP_DEBUG(("AVRCP manual connect - send play\n"));
            sinkAvrcpPlay();
        }
        AVRCP_DATA.avrcp_manual_connect = FALSE;
        BdaddrSetZero(&AVRCP_DATA.avrcp_play_addr);
    }    
}

/*************************************************************************
NAME    
    handleAvrcpConnectInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_CONNECT_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpConnectInd(const AVRCP_CONNECT_IND_T *msg)
{
    uint8 i = 0;
    bool accept = FALSE;    
    
    for_all_avrcp(i)
    {
        AVRCP_DEBUG(("  index %d\n", i));
        if (!BdaddrIsZero(&AVRCP_DATA.bd_addr[i]))
        {           
            AVRCP_DEBUG(("  bdaddr not zero [bdaddr inst %x:%x:%lx] [bdaddr inc %x:%x:%lx]\n", 
                         AVRCP_DATA.bd_addr[i].nap,
                         AVRCP_DATA.bd_addr[i].uap,
                         AVRCP_DATA.bd_addr[i].lap,
                         msg->bd_addr.nap,
                         msg->bd_addr.uap,
                         msg->bd_addr.lap));
            if (BdaddrIsSame(&AVRCP_DATA.bd_addr[i], &msg->bd_addr))
            {          
                AVRCP_DEBUG(("  bdaddr same\n"));
                if (!AVRCP_DATA.connected[i])
                {    
                    AVRCP_DEBUG(("  not connected\n"));
                    /* if an AVRCP connection has been queued, remove our attempt from the queue and accept this connection */
                    accept = TRUE;
                    MessageCancelAll(&AVRCP_DATA.avrcp_ctrl_handler[i], AVRCP_CREATE_CONNECTION);
                    break;
                }
            }
        }
        else
        {
            AVRCP_DEBUG(("  bdaddr zero\n"));
            /* there is space for a new connection so accept it and store device address */
            accept = TRUE; 
            AVRCP_DATA.bd_addr[i] = msg->bd_addr;
            break;
        }
    }
    
    AVRCP_DEBUG(("  AVRCP Connect Response [accept %d]\n", accept));

    if(accept)
    {
        AvrcpConnectResponse(&theSink.task, msg->connection_id, msg->signal_id, accept, &AVRCP_DATA.bd_addr[i]);
    }
    else
    {
        AvrcpConnectResponse(&theSink.task, msg->connection_id, msg->signal_id, accept, NULL);
    }
}


/*************************************************************************
NAME    
    resetAvrcpConnection
    
DESCRIPTION
    Set AVRCP instance back to disconnected state

**************************************************************************/
static void resetAvrcpConnection(uint16 index)
{
    /* delete all messages for this connection */
    sinkAvrcpFlushAndResetStandardCommands(index);
    sinkAvrcpResetVendorCommands(index);

    if(getAvrcpIsPeer(index))
    {
        peerAvrcpDisconnect();
        setAvrcpIsPeer(index, FALSE);
    }
        
    AVRCP_DATA.connected[index] = FALSE;
    BdaddrSetZero(&AVRCP_DATA.bd_addr[index]);
    AVRCP_DATA.registered_events[index] = 0;
    AVRCP_DATA.features[index] = 0;
    AVRCP_DATA.event_capabilities[index] = 0;
    setPlayStatus(index, sinkavrcp_play_status_stopped);
}


/*************************************************************************
NAME    
    handleAvrcpDisconnectInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_DISCONNECT_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpDisconnectInd(const AVRCP_DISCONNECT_IND_T *msg)
{
    uint16 Index;
    
#ifdef ENABLE_PEER    
    uint16 peerAvrcpIndex;
#endif    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
#ifdef ENABLE_PEER
        /* Find if there is a true AV device (non peer) device connected, if yes then send the notification to the peer*/
        if(SinkA2dpIsInitialised())
        {
            if (sinkAvrcpIsAvrcpIndexPeer(Index, &peerAvrcpIndex))
            {
                peerSetRemotePeerAudioConnectionStatus(0);
            }
        }
#endif

        sinkAvrcpBrowsingChannelDisconnectRequest(AVRCP_DATA.avrcp[Index]);
        if (sinkAvrcpBrowsingChannelIsDisconnected(Index))
        {
            AVRCP_DATA.avrcp[Index] = NULL;
        }

        resetAvrcpConnection(Index);
              
        sinkAvrcpUpdateActiveConnection();             
    }    
}

/*****************/
/* AV/C COMMANDS */
/*****************/

/*************************************************************************
NAME    
    handleAvrcpPassthroughCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_PASSTHROUGH_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpPassthroughCfm(const AVRCP *avrcp)
{
    /* 
        Clearing the pending flag should allow another
        pending event to be delivered to controls_handler 
    */
    uint16 index;
    
    if (sinkAvrcpGetIndexFromInstance(avrcp, &index))
    {
        uint8 *vendor_data = AVRCP_DATA.vendor_data[index];
        AVRCP_DATA.pending_cmd[index] = FALSE;
        
        if (vendor_data)
        {
#ifdef ENABLE_PEER
            uint16 cmd_id = vendor_data[0];
            cmd_id = (cmd_id << 8) | vendor_data[1];
            if (cmd_id == AVRCP_PEER_CMD_TWS_AUDIO_ROUTING)
            {
                uint16 a2dp_index;
                uint16 latency_ms = AUDIO_LATENCY_TWS_NORMAL_MS;
                
                if (a2dpGetSourceIndex(&a2dp_index))
                {
                    latency_ms = getA2dpLinkDataLatency(a2dp_index) / 10;
                }
                
                PEER_DEBUG(("TWS routing m=%u s=%u in %u ms\n",
                       vendor_data[2],
                       vendor_data[3],
                       latency_ms));
                
                sinkCancelAndSendLater(EventSysTwsAudioRoutingChanged, latency_ms);
            }
#endif /* def ENABLE_PEER */
            /* Free vendor data */
            sinkAvrcpResetVendorCommands(index);
            
        }
        
    }
}

#ifdef ENABLE_PEER
#ifdef PEER_TWS
/*************************************************************************
NAME
    sinkAvrcpUpdateTWSPeerVolumeLimitReached

DESCRIPTION
    Updates peer slave device about maximum or minimum volume indication
    arrival at peer master device.

RETURNS
    void
**************************************************************************/
void sinkAvrcpUpdateTWSPeerVolumeLimitReached(volume_max_min max_min)
{
    uint16 peerAvrcpIndex;

    /* If a peer is connected, then send the avrcp vendor dependant command to the peer 
       device signalling max or min volume limit */
    if(avrcpGetPeerIndex (&peerAvrcpIndex) )
    {
        /* Inform remote peer about mute toggle */
        sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex, 
                                                AVRCP_PEER_CMD_VOLUME_LIMIT, 
                                                AVRCP_PAYLOAD_PEER_CMD_VOLUME_LIMIT, 
                                                (const uint8 *)&max_min );
    }
}
#endif  /* PEER_TWS */
/*************************************************************************
NAME
    handlePeerVolumeLimitIndication

DESCRIPTION
    Handler function to process volume max or volume min indication from
    peer master.

RETURNS
    void
**************************************************************************/
static void handlePeerVolumeLimitIndication(AVRCP_PASSTHROUGH_IND_T *ind)
{
    uint16 avrcpIndex = 0;
    uint16 peerIndex = 0;
    
    if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_VOLUME_LIMIT))
    {
        if (sinkAvrcpGetIndexFromInstance(ind->avrcp, &avrcpIndex) && sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex))
        {
             peerHandleVolumeLimitCmd(ind->op_data[2]);
             AVRCP_DEBUG(("   peer %s volume\n", ind->op_data[2]==0?"min":"max" ));
        } 
        AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
    } 
}
/*************************************************************************
NAME    
    handleAvrcpVendorUniquePassthroughInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_PASSTHROUGH_IND from the AVRCP library 
    with an opid_vendor_unique operation id.

**************************************************************************/
static bool handleAvrcpVendorUniquePassthroughInd (AVRCP_PASSTHROUGH_IND_T *ind)
{
    AVRCP_DEBUG(("handleAvrcpVendorUniquePassthroughInd\n"));    

    if (ind->size_op_data >= VENDOR_CMD_HDR_SIZE)
    {
        uint16 avrcpIndex;
        uint16 peerIndex;
        uint16 cmd_id = (ind->op_data[0] << 8) + ind->op_data[1];
        
        switch (cmd_id)
        {
        case AVRCP_PEER_CMD_NOP:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_NOP\n"));
            AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
            return TRUE;
            
        case AVRCP_PEER_CMD_PEER_STATUS_CHANGE:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_PEER_STATUS_CHANGE\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_PEER_STATUS_CHANGE))
            {
                if (sinkAvrcpGetIndexFromInstance(ind->avrcp, &avrcpIndex) && sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex))
                {
                    peerHandleStatusChangeCmd( (PeerStatusChange)ind->op_data[2] );
                    AVRCP_DEBUG(("   peer status change = 0x%X\n",ind->op_data[2]));
                }
                
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;
            
        case AVRCP_PEER_CMD_TWS_AUDIO_ROUTING:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_TWS_AUDIO_ROUTING\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_TWS_AUDIO_ROUTING))
            {
                if (sinkAvrcpGetIndexFromInstance(ind->avrcp, &avrcpIndex) && sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex))
                {
                    peerHandleAudioRoutingCmd( (PeerTwsAudioRouting)ind->op_data[2], (PeerTwsAudioRouting)ind->op_data[3] );
                    AVRCP_DEBUG(("   peer audio routing = 0x%X 0x%X\n",ind->op_data[2],ind->op_data[3]));
                }
                
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;
            
        case AVRCP_PEER_CMD_TWS_VOLUME_LOW_BAT_LIMIT:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_TWS_VOLUME_LOW_BAT_LIMIT\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_TWS_VOLUME_LOW_BAT))
            {
                if (sinkAvrcpGetIndexFromInstance(ind->avrcp, &avrcpIndex) && sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex))
                {
                    peerHandleVolumeLowBattCmd( ind->op_data[2] );
                    AVRCP_DEBUG(("   peer slave volume max limit = %u\n",ind->op_data[2]));
                }
                
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;
        case AVRCP_PEER_CMD_AUDIO_CONNECTION_STATUS:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_AUDIO_CONNECTION_STATUS\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_AUDIO_CONNECTION_STATUS))
            {
                audio_src_conn_state_t new_state;
                
                memcpy(&new_state , &ind->op_data[2] , AVRCP_PAYLOAD_PEER_CMD_AUDIO_CONNECTION_STATUS);               
                
                if(new_state.isConnected)
                {
                    peerSetRemotePeerAudioConnectionStatus(peerGetRemotePeerAudioConnectionStatus() | new_state.src);
                    
                    if(new_state.src == A2DP_AUDIO)
                    { 
                        /*Pack the received BD address back into the bdaddr format defined in bdaddr_.h */
                        bdaddr* address;
                        address = peerGetRemotePeerAgBdAddress();
                        address->lap = ((uint32)new_state.bd_addr.lap[2] << 16) | ((uint16)new_state.bd_addr.lap[1] << 8) | new_state.bd_addr.lap[0];
                        address->uap = new_state.bd_addr.uap;   
                        address->nap = (new_state.bd_addr.nap[1] << 8) | new_state.bd_addr.nap[0] ;
                         
                        peerHandleRemoteAgConnected();  
                    }
                }
                else
                {
                    peerSetRemotePeerAudioConnectionStatus(peerGetRemotePeerAudioConnectionStatus() & ~(new_state.src));
                    
                    if(new_state.src == A2DP_AUDIO)
                    {
                        BdaddrSetZero(peerGetRemotePeerAgBdAddress());
                    }
                    
                    /* Clear and restart the auto switchoff timer */
                    AVRCP_DEBUG(("AVRCP : reset auto off\n"));
                    sinkStartAutoPowerOffTimer();
                }
                
                AvrcpVendorDependentResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break; 

        case AVRCP_PEER_CMD_UPDATE_AUDIO_ENHANCEMENT_SETTINGS:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_AUDIO_ENHANCEMENT_SETTINGS\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_UPDATE_AUDIO_ENHANCEMENT_SETTINGS))
            {
                if(SinkA2dpIsInitialised())
                {
                    sinkMusicProcessingStoreFromMsgPayload(&ind->op_data[2]);

                    AVRCP_DEBUG(("   music_mode_enhancements = 0x%X \n", getMusicProcessingBypassFlags()));

                     /* Send the message to DSP to use these audio enhancement parameters */
                    AudioSetMode(AUDIO_MODE_CONNECTED, getAudioPluginModeParams());
                }

                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;

    case AVRCP_PEER_CMD_UPDATE_TRIM_VOLUME:     
        AVRCP_DEBUG(("AVRCP_PEER_CMD_UPDATE_TRIM_VOLUME\n"));
        if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_UPDATE_TRIM_VOLUME))
        {
            if (sinkAvrcpGetIndexFromInstance(ind->avrcp, &avrcpIndex) && sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex))
            {
                peerUpdateTrimVolume((PeerTrimVolChangeCmd) ind->op_data[2] );                
            }
            
            AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
            return TRUE;
        }
        break;

        case AVRCP_PEER_CMD_UPDATE_USER_EQ_SETTINGS:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_UPDATE_USER_EQ_SETTINGS\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_UPDATE_USER_EQ_SETTINGS))
            {
                user_eq_bank_t *PEQ = NULL;
                PEQ = sinkAudioGetPEQ();
                
                if(PEQ)
                {
                    uint8 *user_eq_params = &ind->op_data[VENDOR_CMD_HDR_SIZE];

                    sinkAudioPeerUpdatePEQ(user_eq_params);
                    handleA2DPUserEqBankUpdate();
                }
                
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;

        case AVRCP_PEER_CMD_REQUEST_USER_EQ_SETTINGS:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_REQUEST_USER_EQ_SETTINGS\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_REQUEST_USER_EQ_SETTINGS))
            {
                /*This pull command is sent by a TWS peer device acting as a sink, to receive the current user EQ settings. 
                    So just send the AVRCP_PEER_CMD_UPDATE_USER_EQ_SETTINGS with curent EQ settings as a response. */
                peerSendUserEqSettings();
                
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;

        case AVRCP_PEER_CMD_UPDATE_MUTE:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_TOGGLE_MUTE\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_UPDATE_MUTE))
            {
                VolumeUpdateMuteState(audio_mute_group_main, (bool)ind->op_data[2]);
                
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;

        case AVRCP_PEER_CMD_UPDATE_LED_INDICATION_ON_OFF: 
            AVRCP_DEBUG(("AVRCP_PEER_CMD_UPDATE_LED_INDICATION_ON_OFF\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_UPDATE_LED_INDICATION_ON_OFF))
            {
                if(ind->op_data[2])
                {
                    LedManagerEnableLEDS();
                }
                else
                {
                    LedManagerDisableLEDS();                    
                }
                
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;
            

        case AVRCP_PEER_CMD_UPDATE_BATTERY_LEVEL:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_UPDATE_BATTERY_LEVEL\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_UPDATE_BATTERY_LEVEL))
            {
                if(!peerUpdateBatteryLevel((ind->op_data[2] << 8) | ind->op_data[3]))
                {
                    AVRCP_DEBUG(("AVRCP_PEER_CMD_UPDATE_BATTERY_LEVEL Peer battery not supported\n"));
                }
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;

        case AVRCP_PEER_CMD_SLAVE_EQ_ENABLE_REQUEST:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_SLAVE_EQ_ENABLE_REQUEST"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_SLAVE_EQ_ENABLE_REQUEST))
            {
                if (sinkAvrcpGetIndexFromInstance(ind->avrcp, &avrcpIndex) && sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex))
                {
                    peerUpdateEqEnable(ind->op_data[2]);
                    AVRCP_DEBUG(("   action %d \n", ind->op_data[2] ));
                }
                
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;

        case AVRCP_PEER_CMD_SLAVE_EQ_NEXT_BANK_REQUEST:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_SLAVE_EQ_NEXT_BANK_REQUEST\n"));
            if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_SLAVE_EQ_NEXT_BANK_REQUEST))
            {
                if (sinkAvrcpGetIndexFromInstance(ind->avrcp, &avrcpIndex) && sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex))
                {
                    /* issue next EQ bank command, DSP actions and returns selected bank
                       which is passed to peer slave device */
                    MessageSend(&theSink.task, EventUsrSelectNextUserEQBank, 0);
                }
                
                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;

        case AVRCP_PEER_CMD_TIME_STAMPED_COMMAND:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_TIME_STAMPED_COMMAND\n"));
            if (ind->size_op_data >= VENDOR_CMD_TOTAL_SIZE(AVRCP_MINIMUM_PAYLOAD_PEER_CMD_TIME_STAMPED_COMMAND))
            {
                if (sinkAvrcpGetIndexFromInstance(ind->avrcp, &avrcpIndex) && sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex))
                {
                    peerReceiveSychronisedEvent(&ind->op_data[2]);
                }

                AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                return TRUE;
            }
            break;
            
         case AVRCP_PEER_CMD_VOLUME_LIMIT:
            AVRCP_DEBUG(("AVRCP_PEER_CMD_VOLUME_LIMIT\n"));
            handlePeerVolumeLimitIndication(ind);
            break;

         case AVRCP_PEER_CMD_UPDATE_ANC_EVENT:
        	 AVRCP_DEBUG(("AVRCP_PEER_CMD_UPDATE_ANC_EVENT\n"));
             if (ind->size_op_data == VENDOR_CMD_TOTAL_SIZE(AVRCP_PAYLOAD_PEER_CMD_UPDATE_ANC_EVENT))
             {
                 if (sinkAvrcpGetIndexFromInstance(ind->avrcp, &avrcpIndex) && sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex))
                 {
                	 peerHandleAncEvent( ind->op_data[2] );
                	 AVRCP_DEBUG(("   peer ANC event = %u\n",ind->op_data[2]));
                 }

                 AvrcpPassthroughResponse(ind->avrcp, avctp_response_accepted);
                 return TRUE;
             }
             break;

        default:
            break;
        }
    }
    
    return FALSE;
}
#endif

#ifdef ENABLE_PEER
/*************************************************************************
NAME    
    handlePeerPassthroughOperation
    
DESCRIPTION
     Passthrough operation is for a Peer and in the range we forward to AV Source

**************************************************************************/
static void handlePeerPassthroughOperation(AVRCP_PASSTHROUGH_IND_T *msg, uint16 avrcpIndex)
{   
    /* Passthrough operation is for a Peer and in the range we forward to AV Source */
    uint8 opid = msg->opid - opid_power;
    uint8 state = !!msg->state;
    a2dp_index_t a2dpIndex = a2dp_invalid;
    uint16 avIndex;
    
    /* The device should accept these commands as it supports AVRCP TG category 1. */
    AvrcpPassthroughResponse(msg->avrcp, avctp_response_accepted);

    if (getA2dpIndexFromAvrcp (avrcpIndex, &a2dpIndex) &&
        ((peerIsSingleDeviceOperationEnabled() && (getA2dpPeerRemoteFeatures(a2dpIndex) & remote_features_tws_a2dp_sink)) ||
         (peerShareMeControlEnabled() && (getA2dpPeerRemoteFeatures(a2dpIndex) & remote_features_shareme_a2dp_sink))))
    {   /* Received AVRCP commands routed to AV Source */
        if (getAvAvrcpIndex(&avIndex))
        {   /* True AV Source exists to receive command */
            AVRCP_DEBUG(("   relaying passthrough to AV [%d][%d]\n", msg->opid,state));
            avrcpSendControlMessage(avrcp_map_opid_to_ctrl[opid][state], avIndex);
        }
    }
    else
    {   /* AVRCP command routing not enabled, so handle ones that affect relay stream state locally */
        if (msg->opid == opid_play)
        {   /* Mark relay as availiable */
            peerHandleStatusChangeCmd( PEER_STATUS_CHANGE_RELAY_AVAILABLE );
        }
        else if ((msg->opid == opid_pause) || (msg->opid == opid_stop))
        {   /* Mark relay as unavailiable */
            peerHandleStatusChangeCmd( PEER_STATUS_CHANGE_RELAY_UNAVAILABLE );
        }
    }
}
#endif
/*************************************************************************
NAME    
    handleAvrcpPassthroughInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_PASSTHROUGH_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpPassthroughInd(AVRCP_PASSTHROUGH_IND_T *msg)
{
    uint16 avrcpIndex;
    a2dp_index_t a2dpIndex;
    
    /* Acknowledge the request */    
    if ((msg->opid == opid_volume_up) || (msg->opid == opid_volume_down))
    {        
        AVRCP_DEBUG(("Recieved %d passthrough command\n", msg->opid));
        /* The device should accept volume up commands as it supports AVRCP TG category 2. */
        AvrcpPassthroughResponse(msg->avrcp, avctp_response_accepted);
    
        /* Adjust the local volume only if it is a press command and the A2DP is streaming */
        if (!msg->state && sinkAvrcpGetIndexFromInstance(msg->avrcp, &avrcpIndex))
        {                 
            for_all_a2dp(a2dpIndex)
            {
                if(BdaddrIsSame(getA2dpLinkBdAddr(a2dpIndex), &AVRCP_DATA.bd_addr[avrcpIndex]))
                {
#ifdef ENABLE_PEER
                    if (getA2dpPeerRemoteDevice(a2dpIndex) == remote_device_peer)
                    {   /* Volume change request from a Peer - we could have any type of source */
                        if (msg->opid == opid_volume_up)
                        {
                            AVRCP_DEBUG(("   received vol up from Peer\n"));
                            sinkVolumeModifyAndUpdateRoutedAudioMainVolume(increase_volume);
                        }
                        else
                        {
                            AVRCP_DEBUG(("   received vol down from Peer\n"));
                            sinkVolumeModifyAndUpdateRoutedAudioMainVolume(decrease_volume);
                        }
                    }
                    else
#endif
                    {
                        /* Volume change request from an A2DP source */
                        if(sinkAudioIsAudioRouted() && (sinkAudioGetRoutedAudioSink() == A2dpMediaGetSink(getA2dpLinkDataDeviceId(a2dpIndex), getA2dpLinkDataStreamId(a2dpIndex))))
                        {
                            if(msg->opid == opid_volume_up)
                            {
                                AVRCP_DEBUG(("   received vol up\n"));
                                sinkVolumeModifyAndUpdateRoutedA2DPAudioMainVolume(increase_volume);
                            }
                            else
                            {
                                AVRCP_DEBUG(("   received vol down\n"));
                                sinkVolumeModifyAndUpdateRoutedA2DPAudioMainVolume(decrease_volume);
                            }
                        }
                    }
                }
            }
        }
    }
#ifdef ENABLE_PEER
    else if (msg->opid == opid_vendor_unique)
    {
        if (!handleAvrcpVendorUniquePassthroughInd(msg))
        {   /* An invalid vendor unique command has been received */
            AVRCP_DEBUG(("   received invalid vendor unique cmd\n"));
            AvrcpPassthroughResponse(msg->avrcp, avctp_response_not_implemented);
        }
    }
    else
    {
        uint16 peerIndex;
        PeerStatus localPeerStatus;
        
        if (!sinkAvrcpGetIndexFromInstance(msg->avrcp, &avrcpIndex) || 
            !sinkAvrcpIsAvrcpIndexPeer(avrcpIndex, &peerIndex) || 
            (msg->opid < opid_power) || 
            (msg->opid > opid_backward))
        {   /* Only handle AVRCP passthrough commands, in the expected range, if received from Peer */
            AVRCP_DEBUG(("   received invalid passthrough [%d]\n", msg->opid));
            AvrcpPassthroughResponse(msg->avrcp, avctp_response_not_implemented);
        }
        else if((msg->opid == opid_power) && getA2dpIndexFromAvrcp (avrcpIndex, &a2dpIndex) && peerIsSingleDeviceOperationEnabled())
        { 
             /*We have received a power off command from the peer device so just turn off the device*/
            AVRCP_DEBUG((" received power off passthrough command"));
             
             /* The device should accept these commands as it supports AVRCP TG category 1. */
            AvrcpPassthroughResponse(msg->avrcp, avctp_response_accepted);
            localPeerStatus = getA2dpPeerStatus(a2dpIndex, LOCAL_PEER);
            localPeerStatus |= PEER_STATUS_POWER_OFF;
            setA2dpPeerStatus(a2dpIndex, LOCAL_PEER,  localPeerStatus);

            MessageSend(&theSink.task, EventUsrPowerOff, 0);
        }
        else
        {   /* Passthrough operation is for a Peer and in the range we forward to AV Source */
            handlePeerPassthroughOperation(msg, avrcpIndex);
        }
    }
#else
    else
    {
        AVRCP_DEBUG(("   received invalid passthrough [%d]\n", msg->opid));
        /* The device won't accept any other commands. */
        AvrcpPassthroughResponse(msg->avrcp, avctp_response_not_implemented);
    }
#endif
}

/*************************************************************************
NAME    
    handleAvrcpUnitInfoInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_UNITINFO_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpUnitInfoInd(AVRCP_UNITINFO_IND_T *msg)
{
    /* Device is a CT and TG, so send the correct response to UnitInfo requests. */
    uint32 company_id = 0xffffff; /* IEEE RAC company ID can be used here */
    AvrcpUnitInfoResponse(msg->avrcp, TRUE, subunit_panel, 0, company_id);    
}

/*************************************************************************
NAME    
    handleAvrcpSubUnitInfoInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_SUBUNITINFO_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpSubUnitInfoInd(AVRCP_SUBUNITINFO_IND_T *msg)
{
    /* Device is a CT and TG, so send the correct response to SubUnitInfo requests. */
    uint8 page_data[4];
    page_data[0] = 0x48; /* subunit_type: panel; max_subunit_ID: 0 */
    page_data[1] = 0xff;
    page_data[2] = 0xff;
    page_data[3] = 0xff;
    AvrcpSubUnitInfoResponse(msg->avrcp, TRUE, page_data);    
}

/*************************************************************************
NAME    
    handleSinkAvrcpVendorUniquePassthroughReq
    
DESCRIPTION
    Functionality as a result of receiving SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ from the application.

**************************************************************************/
static void handleSinkAvrcpVendorUniquePassthroughReq (const SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ_T *req)
{
    sinkAvrcpVendorUniquePassthroughRequest( req->avrcp_index, req->cmd_id, req->size_data, req->data);
}

/*************************************************************************
NAME    
    handleAvrcpVendorDependentInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_VENDORDEPENDENT_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpVendorDependentInd(AVRCP_VENDORDEPENDENT_IND_T *msg)
{
    /*
        Reject all vendor requests.
    */
    AvrcpVendorDependentResponse(msg->avrcp, avctp_response_not_implemented);
}

/******************/
/* AVRCP Metadata */
/******************/
/*************************************************************************
NAME    
    handleGetCapsInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_GET_CAPS_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpGetCapsInd(const AVRCP_GET_CAPS_IND_T *msg)
{
    AVRCP_DEBUG(("Request for AVRCP capability id %d\n", msg->caps));

    switch (msg->caps)
    {
        case avrcp_capability_company_id:
            /* FALLTHROUGH */

        case avrcp_capability_event_supported:
            /* In all case we just want to respond with only the mandatory 
               capabilities, which will be inserted by the AVRCP library. */
            AvrcpGetCapsResponse(msg->avrcp, avctp_response_stable, msg->caps, 0, 0);
            break;
        
        default:
            /* This should never happen, doing nothing here will cause the library
               to correctly return an error. */
            AVRCP_DEBUG(("AVRCP capability id not handled\n"));
            AvrcpGetCapsResponse(msg->avrcp, avrcp_response_rejected_invalid_param, msg->caps, 0, 0);
            break;
    }
}

/*************************************************************************
NAME    
    handleAvrcpGetCapsCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_GET_CAPS_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpGetCapsCfm(const AVRCP_GET_CAPS_CFM_T *msg)
{
    uint16 Index;    
    uint16 caps_returned = 0;
    const uint8 *lSource = SourceMap(msg->caps_list);  
    uint16 size_source = SourceSize(msg->caps_list);
    
    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {        
        caps_returned = (msg->size_caps_list < msg->number_of_caps) ? msg->size_caps_list : msg->number_of_caps;
        caps_returned = (size_source < caps_returned) ? size_source : caps_returned;
        
        if (lSource)
        {
            AVRCP_DEBUG(("   success; packet[%d] caps[%d] num_caps[%d] size_caps[%d] size_source[%d]\n", msg->metadata_packet_type, msg->caps, msg->number_of_caps, msg->size_caps_list, size_source));              
            
#ifdef DEBUG_AVRCP   
            {
                uint16 i;
                for (i = 0; i < caps_returned; i++)
                {
                    AVRCP_DEBUG(("        capability[%d] = 0x%x\n", i, lSource[i]));
                }
            }
#endif       
            if (msg->caps == avrcp_capability_event_supported)
            {       
                uint16 j;
                /* now know what optional events are supported by the TG so store these for future reference */
                for (j = 0; j < caps_returned; j++)
                {
                    AVRCP_DATA.event_capabilities[Index] |= (1 << lSource[j]);
                }
            }    
            /* else if (msg->caps == avrcp_capability_company_id)
               {
               
               }
            */
        }
        else
        {
            AVRCP_DEBUG(("   fail; no source\n"));
        }                
    }
    else
    {
        AVRCP_DEBUG(("   fail; status %d\n", msg->status));
    }      
    
    /* finished processing the source */
    AvrcpSourceProcessed(msg->avrcp);  
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        /* Get notified of change in playback status */
        AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_playback_status_changed, 0);
    
        /* other optional notifications can be registered here as it is known what events are supported by the remote device */
    
        if (AVRCP_DATA.event_capabilities[Index] & (1 << avrcp_event_playback_pos_changed))
        {
            /* Get notified of change in playback position */
            AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_playback_pos_changed, AVRCP_PLAYBACK_POSITION_TIME_INTERVAL);
        }

       if(peerTwsQualificationEnabled())
       {
           if (AVRCP_DATA.event_capabilities[Index] & (1 << avrcp_event_volume_changed))
           {
               /* Get notified of change in volume indication*/
               AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_volume_changed, 0);
           }
        }        
    }
}

/*************************************************************************
NAME    
    handleAvrcpRegisterNotificationInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_REGISTER_NOTIFICATION_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpRegisterNotificationInd(const AVRCP_REGISTER_NOTIFICATION_IND_T *msg)
{
    uint16 Index;
    
    AVRCP_DEBUG(("   event_id [%d]\n", msg->event_id));
    
    switch (msg->event_id)
    {
        case avrcp_event_volume_changed:
            if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
            {
                AVRCP_DATA.registered_events[Index] |= (1 << msg->event_id);
                AVRCP_DEBUG(("   (avrcp_event_volume_changed)  registered events [%d] index [%d]\n", AVRCP_DATA.registered_events[Index], Index));
                AvrcpEventVolumeChangedResponse(AVRCP_DATA.avrcp[Index], 
                                        avctp_response_interim,
                                        AVRCP_DATA.absolute_volume[Index]);
            }
            break;
#ifdef ENABLE_PEER
        case avrcp_event_playback_status_changed:
            if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
            {
                AVRCP_DATA.registered_events[Index] |= (1 << msg->event_id);
                AVRCP_DEBUG(("   (avrcp_event_playback_status_changed)  registered events [%d] index [%d]\n", AVRCP_DATA.registered_events[Index], Index));
                AvrcpEventPlaybackStatusChangedResponse(AVRCP_DATA.avrcp[Index], 
                                                        avctp_response_interim,
                                                        (avrcp_play_status)AVRCP_DATA.play_status[Index]);
            }
            break;
#endif
        /* AVRCP TG for Addressed Player */
        case avrcp_event_addressed_player_changed:
        case avrcp_event_track_changed:
            handleAvrcpQualificationRegisterNotificationInd(msg);
            break;
        default:
            /* other registrations should not be received */
            break;
    }
}

/*************************************************************************
NAME
    sinkAvrcpSetA2dpVolumeFromAvrcpVolume

DESCRIPTION
    Sets the A2DP volume for the connection to the device with the address specified.

RETURNS
    Returns TRUE if the volume was set, FALSE otherwise.

**************************************************************************/
static bool sinkAvrcpSetA2dpVolumeFromAvrcpVolume(const bdaddr *bd_addr, const uint8 avrcp_volume)
{
    uint16 volume_step_previous,previous_a2dp_main_volume=0;
    volume_info *a2dp_volume_info = NULL;

    if(sinkA2dpGetA2dpVolumeFromBdaddr(bd_addr, &volume_step_previous))
    {
        /* convert AVRCP absolute volume to A2DP volume level, the AVRCP volume range is
           0 to 127, convert this to the headset configured range max steps */
        uint16 volume_step = sinkAvrcpConvertAvrcpVolumeToA2dpStep(avrcp_volume);
        uint16 volume_step_max = sinkVolumeGetMaxVolumeStep(audio_output_group_main);
        uint16 a2dp_index;

        if(volume_step > volume_step_max)
        {
            volume_step = volume_step_max;
        }

        getA2dpIndexFromBdaddr(bd_addr, &a2dp_index);

        /* keep a record of the previous volume */
        a2dp_volume_info =  sinkA2dpGetA2dpVolumeInfoAtIndex(a2dp_index);
        previous_a2dp_main_volume = a2dp_volume_info->main_volume;

        /* update the main volume with the volume */
        sinkA2dpSetA2dpVolumeAtIndex(a2dp_index, volume_step);

        if(sinkAudioIsAudioRouted() && a2dpAudioSinkMatch(a2dp_index, sinkAudioGetRoutedAudioSink()))
        {
            if(powerManagerGetChargingDeficitAction() == battery_low_output_action_limit_max_volume)
            {
                unsigned cdl_as_step_level = sinkVolumeGetChargeDeficitMaxVolumeLimit();

                /* check whether the volume limit is in effect and prevent it from increasing
                                  above limit is output is limited due to low power */
                if(cdl_as_step_level != 0U)
                {
                    if(a2dp_volume_info->main_volume >= (int16)cdl_as_step_level)
                    {
                        /* limit in volume to calculated step level */
                        sinkA2dpSetA2dpVolumeAtIndex(a2dp_index, cdl_as_step_level);

                        /* now at max volume limit, play tone */
                        /* flush out any queued messages */
                        MessageCancelAll(&theSink.task, EventSysVolumeMax);
                        sendVolumeEvent(EventSysVolumeMax);
#ifdef ENABLE_AVRCP
                        if(avrcpAvrcpIsEnabled())
                        {
                            AVRCP_DEBUG(("VOL: avrcp abs vol old[%d] new[%d]\n",
                                         previous_a2dp_main_volume, 
                                         a2dp_volume_info->main_volume));

                            /* update absolute volume control change */
                            sinkAvrcpUpdateVolume(a2dp_volume_info->main_volume);
                        }
#endif
                    }
                }
            }

            if(previous_a2dp_main_volume != volume_step)
            {
                if(volume_step == volume_step_max)
                {
                    sendVolumeEvent(EventSysVolumeMax);
                }
                else if(volume_step == VOLUME_STEP_MIN)
                {
                    sendVolumeEvent(EventSysVolumeMin);
                }
            }
            /* after limit checking the volume, set the new volume level */
            VolumeSetNewMainVolume(sinkA2dpGetA2dpVolumeInfoAtIndex(a2dp_index),
                                                    volume_step_previous);
        }
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
NAME
    handleAvrcpSetAbsVolInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_SET_ABSOLUTE_VOLUME_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpSetAbsVolInd(const AVRCP_SET_ABSOLUTE_VOLUME_IND_T *msg)
{
    uint16 Index;
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {        
        AvrcpSetAbsoluteVolumeResponse(AVRCP_DATA.avrcp[Index], 
                                   avctp_response_accepted, 
                                   msg->volume);

        sinkAvrcpSetA2dpVolumeFromAvrcpVolume(&AVRCP_DATA.bd_addr[Index], msg->volume);
    }
}

/*************************************************************************
NAME    
    handleAvrcpGetPlayStatusCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_GET_PLAY_STATUS_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpGetPlayStatusCfm(const AVRCP_GET_PLAY_STATUS_CFM_T *msg)
{
    uint16 Index;
                
    AVRCP_DEBUG(("   play status cfm [%d] [%d]\n", msg->status, msg->play_status));
    
    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if ((msg->play_status == avrcp_play_status_fwd_seek) || (msg->play_status == avrcp_play_status_rev_seek))
            setPlayStatus(Index, (SinkAvrcp_play_status)(1 << msg->play_status));
        else
            setPlayStatus(Index, (SinkAvrcp_play_status)msg->play_status);
    }
}

#ifdef ENABLE_PEER
/*************************************************************************
NAME    
    avrcpUpdatePeerPlayStatus
    
DESCRIPTION
    Used to update Peer device with current AV source play status.

**************************************************************************/
void avrcpUpdatePeerPlayStatus (avrcp_play_status play_status)
{
    uint16 peer_id;
    
    if (avrcpGetPeerIndex(&peer_id))
    {   /* Forward AV Source play status to Peer device */
         if ((play_status == avrcp_play_status_fwd_seek) || (play_status == avrcp_play_status_rev_seek))
         {
            setPlayStatus(peer_id, (SinkAvrcp_play_status)(1 << play_status));
         }
         else
         {
            setPlayStatus(peer_id, (SinkAvrcp_play_status)play_status);
         }
        
        AVRCP_DEBUG(("AVRCP: Update peer play status [%d] index[%d]\n", AVRCP_DATA.play_status[peer_id], peer_id));
        AvrcpEventPlaybackStatusChangedResponse(AVRCP_DATA.avrcp[peer_id], 
                                                avctp_response_changed, 
                                                play_status);
    }
}
#endif

/*************************************************************************
NAME    
    handleAvrcpPlayStatusChangedInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpPlayStatusChangedInd(const AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND_T *msg)
{
    uint16 Index;
    a2dp_index_t a2dp_link;
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {                   
        AVRCP_DEBUG(("   play status ind [%d] [%d] index[%d]\n", msg->response, msg->play_status, Index));
        if ((msg->response == avctp_response_changed) || (msg->response == avctp_response_interim))
        {
            if ((msg->play_status == avrcp_play_status_fwd_seek) || (msg->play_status == avrcp_play_status_rev_seek))
            {
                setPlayStatus(Index, (SinkAvrcp_play_status)(1 << msg->play_status));
            }
            else
            {
                setPlayStatus(Index, (SinkAvrcp_play_status)msg->play_status);
            }
          
            /* store that this command is supported by remote device */
            AVRCP_DATA.registered_events[Index] |= (1 << avrcp_event_playback_status_changed);
        
            if (msg->response == avctp_response_changed)
            {                      
                /* re-register to receive notifications */
                AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_playback_status_changed, 0);
            }

#ifdef ENABLE_PEER
            if((msg->response == avctp_response_changed) && (!sinkAvrcpIsAvrcpIndexPeer(Index, NULL)) && (peerCurrentRelaySource() == RELAY_SOURCE_A2DP))
            {
                avrcpUpdatePeerPlayStatus( msg->play_status);
            }
#endif

            /* Do a now playing request on an interim or changed notification, but don't unless Playing */
            if (AVRCP_DATA.play_status[Index] == sinkavrcp_play_status_playing)
            {
                a2dp_index_t a2dpIndex;
                
                if ((msg->response == avctp_response_changed) && getA2dpIndexFromAvrcp(Index, &a2dpIndex))
                {
                    a2dpSetPlayingState(a2dpIndex, TRUE);
#ifdef ENABLE_PEER
                    PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE PLAYING");
                    
                    AVRCP_DEBUG(("AVRCP: handleAvrcpPlayStatusChangedInd - playing; cancel pause-suspend timeout a2dp_idx %u\n", a2dpIndex));
                    a2dpCancelPauseSuspendTimeout(a2dpIndex);
#endif
                }
#ifdef ENABLE_PARTYMODE
               if((BdaddrIsSame(getA2dpLinkBdAddr(a2dp_primary),&AVRCP_DATA.bd_addr[Index])))
               {
                   /* Primary AG has started the audio streaming so cancel the timer */
                   MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice1));
               }
               else if((BdaddrIsSame(getA2dpLinkBdAddr(a2dp_secondary),&AVRCP_DATA.bd_addr[Index])))
               {
                   /* Secondary AG has started the audio streaming so cancel the timer */
                   MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice2));
               }
#endif
#ifdef ENABLE_AVRCP_NOW_PLAYING                                
                if (Index == sinkAvrcpGetActiveConnection())
                    sinkAvrcpRetrieveNowPlayingNoBrowsingRequest(Index, FALSE);
#endif
                /* when using the Soundbar manual audio routing and subwoofer support, 
                   check to see if the a2dp audio is being routed, if not check whether
                   an esco subwoofer channel is currently in use, if this is the case it 
                   will be necessary to pause this a2dp stream to prevent disruption
                   of the subwoofer esco link due to link bandwidth limitations */
                if(getA2dpIndexFromAvrcp(Index, &a2dp_link))
                {
#ifdef ENABLE_SUBWOOFER
                    suspendWhenSubwooferStreamingLowLatency(a2dp_link);
#endif
                }
            }
            else if (AVRCP_DATA.play_status[Index] == sinkavrcp_play_status_stopped ||
                     AVRCP_DATA.play_status[Index] == sinkavrcp_play_status_paused)
            {
                a2dp_index_t a2dpIndex;

                if ((msg->response == avctp_response_changed) && getA2dpIndexFromAvrcp(Index, &a2dpIndex))
                {
#ifdef ENABLE_PEER
                    uint16 peerIndex;
#ifdef DEBUG_AVRCP
					bool is_peer = a2dpGetPeerIndex(&peerIndex);
#endif
                    /* When in extended TWS mode we need to suspend the local
                       AG a2dp stream as soon as possible after it is paused.
                       This is to reduce audio dropouts if the AG on the peer
                       device starts streaming before the local AG stream has
                       been suspended.
                       This is a workaround for a performance limitation
                       when handling > 1 a2dp stream as slave. */
                    AVRCP_DEBUG(("AVRCP:    is_peer %u peer_index %u a2dp_index %u playing %u\n",
                        is_peer, peerIndex, a2dpIndex,
                        (getA2dpStatusFlag(PLAYING, a2dpIndex))));
                    
                    if (a2dpGetPeerIndex(&peerIndex)
                        && !a2dpIsIndexPeer(a2dpIndex)
                        && getA2dpStatusFlag(PLAYING, a2dpIndex)
                        && sinkAvrcpIsAvrcpAudioSwitchingEnabled())
                    {
                        a2dpStartPauseSuspendTimeout(a2dpIndex);
                    }
                
#endif
                    if (sinkAvrcpIsAvrcpAudioSwitchingEnabled())
                    {
                        a2dpSetPlayingState(a2dpIndex, FALSE);
#ifdef ENABLE_PEER
                        if (AVRCP_DATA.play_status[Index] == sinkavrcp_play_status_paused)
                        {
                            /* if we are not in single device mode, we also need to make the link available */
                            if(!peerIsSingleDeviceOperationEnabled() && !a2dpIsIndexPeer(a2dpIndex))
                            {
                                peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_RELAY_AVAILABLE);
                            }
                            
                            PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE PAUSED");                        
                        }
                        else
                        {
                            PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE STOPPED");
                        }
#endif
                    }
                    else
                    {
                        setA2dpStatusFlag(PLAYING, a2dpIndex, FALSE);
                    }
                }
            }     
            
            if (sinkAvrcpIsAvrcpAudioSwitchingEnabled() && 
                (msg->response == avctp_response_changed) &&
                AVRCP_DATA.play_status[Index] != sinkavrcp_play_status_fwd_seek &&
                AVRCP_DATA.play_status[Index] != sinkavrcp_play_status_rev_seek)
            {
                
                if(AVRCP_DATA.play_status[Index] == sinkavrcp_play_status_playing)
                {
                    audioUpdateAudioRouting();
                }
                else audioUpdateAudioRoutingAfterDisconnect();
            }
            
#ifdef ENABLE_PARTYMODE
            if (sinkAvrcpIsAvrcpAudioSwitchingEnabled() && 
                (msg->response == avctp_response_changed) &&
                ((AVRCP_DATA.play_status[Index] == sinkavrcp_play_status_paused)||
                 (AVRCP_DATA.play_status[Index] == sinkavrcp_play_status_stopped)))
            {
                /* notify party mode that current track being played has ended */
                sinkPartyModeTrackChangeIndication(Index);          
            }
#endif
        }     
        else
        {
            /* assume not supported by remote device */
            AVRCP_DATA.registered_events[Index] &= ~(1 << avrcp_event_playback_status_changed);
        }
    }
}

#ifdef ENABLE_AVRCP_NOW_PLAYING
/*************************************************************************
NAME    
    handleAvrcpGetElementAttributesCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_GET_ELEMENT_ATTRIBUTES_CFM from the AVRCP library.
    This will return the currently now playing track information which can be displayed.

**************************************************************************/
static void handleAvrcpGetElementAttributesCfm(const AVRCP_GET_ELEMENT_ATTRIBUTES_CFM_T *msg)
{
    uint16 Index;

    if ((msg->status == avrcp_success) && (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index)))
    {
        uint16 i = 0;
        uint32 attribute_id = 0;
        uint16 attribute_length = 0;
        uint16 header_end = 0;
        const uint8 *lSource = SourceMap(msg->attributes);
        uint16 source_size = SourceSize(msg->attributes);
        uint16 size_attributes = (source_size < msg->size_attributes) ? source_size : msg->size_attributes;

        if ((lSource) && (msg->number_of_attributes > 0))
        {
            AVRCP_DEBUG(("   success; packet[%d] num_attr[%d] size_att[%d] size_source[%d]\n", msg->metadata_packet_type, msg->number_of_attributes, msg->size_attributes, source_size));

            header_end = i + AVRCP_GET_ELEMENT_ATTRIBUTES_CFM_HEADER_SIZE;
            while (size_attributes >= header_end) /* check there is room left for data to exist */
            {
            #ifdef DEBUG_AVRCP
                uint16 charset_id;
            #endif
                attribute_id = ((uint32)lSource[i] << 24) | ((uint32)lSource[i + 1] << 16) | ((uint32)lSource[i + 2] << 8) | lSource[i + 3];
                attribute_length = (lSource[i + 6] << 8) | lSource[i + 7];

            #ifdef DEBUG_AVRCP
                charset_id = (lSource[i + 4] << 8) | lSource[i + 5];
                AVRCP_DEBUG(("        attribute = 0x%lx\n", attribute_id));
                AVRCP_DEBUG(("        charset_id = 0x%x\n", charset_id));
                AVRCP_DEBUG(("        attribute_length = 0x%x\n", attribute_length));
            #endif

                if ((attribute_length + header_end) > size_attributes)
                    attribute_length = size_attributes - header_end;
                
#ifdef DEBUG_AVRCP    
                {
                    uint16 j;
                    AVRCP_DEBUG(("        attribute text : "));
                    for (j = i; j < (i + attribute_length); j++)
                    {                    
                        AVRCP_DEBUG(("%c", lSource[j + 8]));                    
                    }
                        AVRCP_DEBUG(("\n"));
                }
#endif                               
                
                AVRCP_DEBUG(("AVRCP NOW PLAYING:\n"));
                
                if (attribute_length)
                    sinkAvrcpDisplayMediaAttributes(attribute_id, attribute_length, &lSource[i + 8]);

                i = header_end + attribute_length;
                header_end = i + AVRCP_GET_ELEMENT_ATTRIBUTES_CFM_HEADER_SIZE;
            }
                    
            /* finished processing the source */
            AvrcpSourceProcessed(msg->avrcp);  
            
            if ((msg->metadata_packet_type == AVRCP_PACKET_TYPE_END) || (msg->metadata_packet_type == AVRCP_PACKET_TYPE_CONTINUE))
            {                
                /* cancel sending an Abort */
                MessageCancelAll(&AVRCP_DATA.avrcp_ctrl_handler[Index], AVRCP_CTRL_ABORT_CONTINUING_RESPONSE);
            }
            /* If the response is fragmented then request the next chunk of data */
            if ((msg->metadata_packet_type == AVRCP_PACKET_TYPE_START) || (msg->metadata_packet_type == AVRCP_PACKET_TYPE_CONTINUE))
            {                
                MAKE_AVRCP_MESSAGE(AVRCP_CTRL_ABORT_CONTINUING_RESPONSE);                
                message->pdu_id = AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID;
                AvrcpRequestContinuingResponseRequest(msg->avrcp, AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID);
                MessageSendLater(&AVRCP_DATA.avrcp_ctrl_handler[Index], AVRCP_CTRL_ABORT_CONTINUING_RESPONSE, message, AVRCP_ABORT_CONTINUING_TIMEOUT);
            }            
        }
        else
        {
            AVRCP_DEBUG(("   fail; no source; num attributes zero\n"));
        }
    }
    else
    {
        AVRCP_DEBUG(("   fail; status %d\n", msg->status));
    }
}


/*************************************************************************
NAME    
    handleAvrcpNowPlayingContentChangedInd
    
DESCRIPTION
    Notification that Now Playing Content has changed.

**************************************************************************/
static void handleAvrcpNowPlayingContentChangedInd(const AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED_IND_T *msg)
{
    uint16 Index;
                
    AVRCP_DEBUG(("   now playing content changed ind [%d]\n", msg->response));
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if ((msg->response == avctp_response_changed) || (msg->response == avctp_response_interim))
        {         
            /* store that this command is supported by remote device */         
            if (msg->response == avctp_response_changed)
            {
                /* re-register to receive notifications */
                AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_now_playing_content_changed, 0);   
                              
                if (sinkAvrcpGetActiveConnection() == Index)
                {
                    /* TODO now playing content changed on active connection, may need to update display */
                }                         
            }            
        }     
        else
        {
            /* assume not supported by remote device */
            AVRCP_DATA.registered_events[Index] &= ~(1 << avrcp_event_now_playing_content_changed);
        }
    }
}

#endif /* ENABLE_AVRCP_NOW_PLAYING */

/*************************************************************************
NAME    
    handleAvrcpTrackChangedInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_EVENT_TRACK_CHANGED_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpTrackChangedInd(const AVRCP_EVENT_TRACK_CHANGED_IND_T *msg)
{
    uint16 Index;
                
    AVRCP_DEBUG(("   track changed ind [%x] [0x%lx][0x%lx]\n", msg->response, msg->track_index_high, msg->track_index_low));
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if ((msg->response == avctp_response_changed) || (msg->response == avctp_response_interim))
        {         
            /* store that this command is supported by remote device */
            AVRCP_DATA.registered_events[Index] |= (1 << avrcp_event_track_changed);
        

            if (msg->response == avctp_response_changed)
            {
                AVRCP_DEBUG(("TrackChanged- Response change\n"));
                /* re-register to receive notifications */
                AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_track_changed, 0);          
                
#ifdef ENABLE_AVRCP_NOW_PLAYING                   
                /* request now playing information */
                if ((msg->track_index_high != 0xffffffffu) || (msg->track_index_low != 0xffffffffu))
                {
                    if (Index == sinkAvrcpGetActiveConnection())
                        sinkAvrcpRetrieveNowPlayingRequest(msg->track_index_high, msg->track_index_low, FALSE);
                }     
#endif /* ENABLE_AVRCP_NOW_PLAYING */                 
                
#ifdef ENABLE_PARTYMODE
                /* notify party mode that current track being played has ended */
                sinkPartyModeSkipTrackIndication(Index);          
#endif                
            }                 
        }     
        else
        {
            /* assume not supported by remote device */
            AVRCP_DATA.registered_events[Index] &= ~(1 << avrcp_event_track_changed);
        }
    }
}


/*************************************************************************
NAME    
    handleAvrcpPlaybackPosChangedInd
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND from the AVRCP library.

**************************************************************************/
static void handleAvrcpPlaybackPosChangedInd(const AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND_T *msg)
{
    uint16 Index;
                
    AVRCP_DEBUG(("   playback pos changed ind [%d] playback_pos[0x%lx]\n", msg->response, msg->playback_pos));
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if ((msg->response == avctp_response_changed) || (msg->response == avctp_response_interim))
        {                    
            if (msg->response == avctp_response_changed)
            {
                a2dp_index_t a2dpIndex;

                if (getA2dpIndexFromAvrcp(Index, &a2dpIndex) )        
                {
                    a2dpPauseNonRoutedSource(a2dpIndex);
                }
                
                /* re-register to receive notifications */
                AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_playback_pos_changed, AVRCP_PLAYBACK_POSITION_TIME_INTERVAL);                 
            }
        }     
        else
        {
            /* assume not supported by remote device */
            AVRCP_DATA.event_capabilities[Index] &= ~(1 << avrcp_event_playback_pos_changed);
        }
    }
}

#ifdef ENABLE_AVRCP_PLAYER_APP_SETTINGS
/*************************************************************************
NAME
    handleAvrcpListAppAttributeCfm

DESCRIPTION
    Functionality as a result of receiving AVRCP_LIST_APP_ATTRIBUTE_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpListAppAttributeCfm(const AVRCP_LIST_APP_ATTRIBUTE_CFM_T *msg)
{
    uint16 Index;
    const uint8 *lSource = SourceMap(msg->attributes);
    
    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if (lSource)
        {
            AVRCP_DEBUG(("   success; packet[%d] num_attr[%d] size_att[%d]\n", msg->metadata_packet_type, msg->number_of_attributes, msg->size_attributes));
            
        #ifdef DEBUG_AVRCP
            {
                uint16 source_size = SourceSize(msg->attributes);
                uint16 size_attributes = (source_size < msg->size_attributes) ? source_size : msg->size_attributes;
                uint16 attributes_returned = (size_attributes < msg->number_of_attributes) ? size_attributes : msg->number_of_attributes;
                uint16 i;
                for (i = 0; i < attributes_returned; i++)
                {
                    AVRCP_DEBUG(("        attribute[%d] = 0x%x\n", i, lSource[i]));
                }
            }
        #endif

        }
        else
        {
            AVRCP_DEBUG(("   fail; no source\n"));
        }
    }
    else
    {
        AVRCP_DEBUG(("   fail; status %d\n", msg->status));
    }

    /* TODO there are attributes_returned attribute IDs contained in lSource - can now retrieve text for these attributes (sinkAvrcpListPlayerAttributesTextRequest) */

    /* finished processing the source */
    AvrcpSourceProcessed(msg->avrcp);  
}


/*************************************************************************
NAME    
    handleAvrcpGetAppAttributeValueTextCfm
    
DESCRIPTION
    Functionality as a result of receiving attribute text and attribute values text.

**************************************************************************/
static void handleAvrcpGetAppAttributeValueTextCfm(uint16 pdu_id, const AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM_T *msg)
{
    uint16 Index;
    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        uint16 i = 0;
        uint16 attribute_length = 0;
        uint16 header_end = 0;
        const uint8 *lSource = SourceMap(msg->attributes);
        uint16 source_size = SourceSize(msg->attributes);
        uint16 size_attributes = (source_size < msg->size_attributes) ? source_size : msg->size_attributes;

        if (lSource)
        {
            AVRCP_DEBUG(("   success; packet[%d] num_attr[%d] size_att[%d] source_size[%d]\n", msg->metadata_packet_type, msg->number_of_attributes, msg->size_attributes, source_size));

            if ((msg->metadata_packet_type == AVRCP_PACKET_TYPE_SINGLE) || (msg->metadata_packet_type == AVRCP_PACKET_TYPE_START))
            {
                /* TODO prepare display for msg->number_of_attributes number of attributes with text to be returned in following responses */
            }

            header_end = i + AVRCP_GET_APP_ATTRIBUTES_TEXT_CFM_HEADER_SIZE;

            while (size_attributes >= header_end) /* check there is room left for header to string data to exist */
            {
            #ifdef DEBUG_AVRCP
                uint16 attribute_id = lSource[i];
                uint16 charset_id = (lSource[i + 1] << 8) | lSource[i + 2];
                attribute_length = lSource[i + 3];
                AVRCP_DEBUG(("        attribute = 0x%x\n", attribute_id));
                AVRCP_DEBUG(("        charset_id = 0x%x\n", charset_id));
                AVRCP_DEBUG(("        attribute_length = 0x%x\n", attribute_length));
            #else
                attribute_length = lSource[i + 3];
            #endif
                if ((attribute_length + header_end) > size_attributes)
                {
                    attribute_length = size_attributes - header_end;
                    AVRCP_DEBUG(("        new attr length = %d\n", attribute_length));
                }

                if (attribute_length)
                {
#ifdef DEBUG_AVRCP    
                    {
                        uint16 j;
                        AVRCP_DEBUG(("        attribute text : "));
                        for (j = i; j < (i + attribute_length); j++)
                        {                    
                            AVRCP_DEBUG(("%c", lSource[j + 4]));                    
                        }
                        AVRCP_DEBUG(("\n"));
                    }
#endif                               

                    /* TODO display attributes that have text associated with them - the text is of length attribute_length and starts at lSource[i + 4] */                
                    
                }      

                i = header_end + attribute_length;
                header_end = i + AVRCP_GET_APP_ATTRIBUTES_TEXT_CFM_HEADER_SIZE;                      
            }
                   
            if ((msg->metadata_packet_type == AVRCP_PACKET_TYPE_END) || (msg->metadata_packet_type == AVRCP_PACKET_TYPE_CONTINUE))
            {                
                /* cancel sending an Abort */
                MessageCancelAll(&AVRCP_DATA.avrcp_ctrl_handler[Index], AVRCP_CTRL_ABORT_CONTINUING_RESPONSE);
            }
            /* If the response is fragmented then request the next chunk of data */
            if ((msg->metadata_packet_type == AVRCP_PACKET_TYPE_START) || (msg->metadata_packet_type == AVRCP_PACKET_TYPE_CONTINUE))
            {
                MAKE_AVRCP_MESSAGE(AVRCP_CTRL_ABORT_CONTINUING_RESPONSE);
                message->pdu_id = pdu_id;
                AvrcpRequestContinuingResponseRequest(msg->avrcp, pdu_id);
                MessageSendLater(&AVRCP_DATA.avrcp_ctrl_handler[Index], AVRCP_CTRL_ABORT_CONTINUING_RESPONSE, message, AVRCP_ABORT_CONTINUING_TIMEOUT);                
            }
            else if ((msg->metadata_packet_type == AVRCP_PACKET_TYPE_SINGLE) || (msg->metadata_packet_type == AVRCP_PACKET_TYPE_END))
            {
                /* TODO this is the last packet in response so display of text is complete */
            }
        }
        else
        {
            AVRCP_DEBUG(("   fail; no source\n"));
        }
    }
    else
    {
        AVRCP_DEBUG(("   fail; status %d\n", msg->status));
    }

    if (!success)
    {
        /* TODO response is not valid so update display based on avrcp_status */
    }

    /* finished processing the source */
    AvrcpSourceProcessed(msg->avrcp);
}


/*************************************************************************
NAME    
    handleAvrcpListAppValueCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_LIST_APP_VALUE_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpListAppValueCfm(const AVRCP_LIST_APP_VALUE_CFM_T *msg)
{
    uint16 Index;
    const uint8 *lSource = SourceMap(msg->values);
    uint16 source_size = SourceSize(msg->values);
    uint16 attributes_returned = msg->number_of_values;
    uint16 size_values = (source_size < msg->size_values) ? source_size : msg->size_values;

    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if (lSource)
        {
            AVRCP_DEBUG(("   success; packet[%d] num_attr[%d] size_att[%d] source_size[%d]\n", msg->metadata_packet_type, msg->number_of_values, msg->size_values, source_size));
            if (size_values < attributes_returned)
                attributes_returned = size_values;

#ifdef DEBUG_AVRCP
            {
                uint16 j;
                for (j = 0; j < attributes_returned; j++)
                {
                    AVRCP_DEBUG(("        value_id[%d] = 0x%x\n", j, lSource[j]));
                }
            }
#endif
        }
        else
        {
            AVRCP_DEBUG(("   fail; no source\n"));
        }
    }
    else
    {
        AVRCP_DEBUG(("   fail; status %d\n", msg->status));
    }
    
    /* TODO there are attributes_returned number of values returned in lSource, can now retrieve text for these values (sinkAvrcpListPlayerValueTextRequest) */
    
    /* finished processing the source */
    AvrcpSourceProcessed(msg->avrcp); 
}


/*************************************************************************
NAME    
    handleAvrcpGetAppValueCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_GET_APP_VALUE_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpGetAppValueCfm(const AVRCP_GET_APP_VALUE_CFM_T * msg)
{
    uint16 Index;

    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        const uint8 *lSource = SourceMap(msg->values);
        uint16 source_size = SourceSize(msg->values);
        uint16 attributes_returned = msg->number_of_values;
        uint16 size_values = (source_size < msg->size_values) ? source_size : msg->size_values;
        uint16 i = 0;

        if (lSource)
        {
            AVRCP_DEBUG(("   success; packet[%d] num_attr[%d] size_att[%d] source_size[%d]\n", msg->metadata_packet_type, msg->number_of_values, msg->size_values, source_size));
            if (size_values < (attributes_returned * AVRCP_GET_APP_VALUE_CFM_DATA_SIZE))
                attributes_returned = size_values / AVRCP_GET_APP_VALUE_CFM_DATA_SIZE;

            while ((size_values - i) >= AVRCP_GET_APP_VALUE_CFM_DATA_SIZE)
            {
            #ifdef DEBUG_AVRCP
                uint8 attribute_id = lSource[i];
                uint8 value_id = lSource[i + 1];
                AVRCP_DEBUG(("        attribute = 0x%x\n", attribute_id));
                AVRCP_DEBUG(("        value_id = 0x%x\n", value_id));
            #endif

                i += AVRCP_GET_APP_VALUE_CFM_DATA_SIZE;

                /* TODO store {attribute_id, value_id} pair for display */
            }
        }
        else
        {
            AVRCP_DEBUG(("   fail; no source\n"));
        }
    }
    else
    {
        AVRCP_DEBUG(("   fail; status %d\n", msg->status));
    }

    /* finished processing the source */
    AvrcpSourceProcessed(msg->avrcp);
}


/*************************************************************************
NAME    
    handleAvrcpSetAppValueCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_SET_APP_VALUE_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpSetAppValueCfm(const AVRCP_SET_APP_VALUE_CFM_T *msg)
{
    UNUSED(msg);
    AVRCP_DEBUG((" handleAvrcpSetAppValueCfm  status %d\n", msg->status));

    /* setting of value is now complete */
}

/*************************************************************************
NAME    
    sinkAvrcpPlayerAppSettingsChangedInd
    
DESCRIPTION
    Notification of change in Player Application Settings

**************************************************************************/
static void sinkAvrcpPlayerAppSettingsChangedInd(const AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND_T *msg)
{
    uint16 Index;
                
    AVRCP_DEBUG(("   Player App Settings changed ind [%d]\n", msg->response));
    
    if (sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        if ((msg->response == avctp_response_changed) || (msg->response == avctp_response_interim))
        {                           
            if (msg->response == avctp_response_changed)
            {
                /* re-register to receive notifications */
                AvrcpRegisterNotificationRequest(msg->avrcp, avrcp_event_player_app_setting_changed, 0);   

                if (sinkAvrcpGetActiveConnection() == Index) /* update display if this is the active connection */
                {
                    /* TODO active connection has had its Player Application Settings changed, may need to update display */                    
                }

            }
        }     
        else
        {
            /* assume not supported by remote device */
            AVRCP_DATA.event_capabilities[Index] &= ~(1 << avrcp_event_player_app_setting_changed);
        }
    }
    if (msg->attributes)
        AvrcpSourceProcessed(msg->avrcp);
}
#endif


/********************/
/* LIBRARY SPECIFIC */
/********************/ 
            
/*************************************************************************
NAME    
    handleAvrcpGetExtensionsCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_GET_EXTENSIONS_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpGetExtensionsCfm(const AVRCP_GET_EXTENSIONS_CFM_T *msg)
{
    uint16 Index;
    
    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        AVRCP_DATA.extensions[Index] = msg->extensions; 
        AVRCP_DEBUG(("   extensions 0x%x\n", msg->extensions));
        /* now retrieve supported features */
        AvrcpGetSupportedFeatures(msg->avrcp);
    }    
}

/*************************************************************************
NAME    
    handleAvrcpGetSupportedFeaturesCfm
    
DESCRIPTION
    Functionality as a result of receiving AVRCP_GET_SUPPORTED_FEATURES_CFM from the AVRCP library.

**************************************************************************/
static void handleAvrcpGetSupportedFeaturesCfm(const AVRCP_GET_SUPPORTED_FEATURES_CFM_T *msg)
{
    uint16 Index;
    
    if ((msg->status == avrcp_success) && sinkAvrcpGetIndexFromInstance(msg->avrcp, &Index))
    {
        AVRCP_DATA.features[Index] = msg->features;
        
        AVRCP_DEBUG(("   features 0x%x\n", msg->features));
        
        if (isAvrcpCategory1MetadataEnabled(Index))
        {            
            AVRCP_DEBUG(("   Category 1 Metadata enabled\n"));
            
            /* Get the TG supported event capabilities */
            AvrcpGetCapsRequest(msg->avrcp, avrcp_capability_event_supported);

#if defined (ENABLE_AVRCP_NOW_PLAYING) || defined (ENABLE_PARTYMODE)
            /* Get Now Playing information for display, by registering Track Changed event */
            /* Get Track change information in AVRCP Party mode, by registering Track Changed event */
            AvrcpRegisterNotificationRequest(AVRCP_DATA.avrcp[Index], avrcp_event_track_changed, 0);
#endif
            if (sinkAvrcpBrowsingMultipleMediaPlayersIsSupported(Index))
            {
                /* Get currently active media player */
                AvrcpRegisterNotificationRequest(AVRCP_DATA.avrcp[Index], avrcp_event_addressed_player_changed, 0);
            }
        }
        else
        {
            AVRCP_DEBUG(("   No Metadata enabled\n"));           
        }                
    }
}

/*********************** Main Functions ***************************/

/*************************************************************************
NAME    
    sinkAvrcpInit
    
DESCRIPTION
    Initialise the AVRCP library.

**************************************************************************/
void sinkAvrcpInit(void)
{
    avrcp_init_params config;
    uint16 i;

    AVRCP_DEBUG(("AVRCP: Avrcp Init\n"));
    if(avrcpAvrcpIsEnabled())
    {     
         /* Initialise AVRCP library */ 
        config.device_type = avrcp_target_and_controller;
        config.supported_controller_features = AVRCP_CATEGORY_1; /* mainly operates as controller */
        config.supported_target_features = AVRCP_CATEGORY_2; /* only target for volume commands */
#ifdef ENABLE_PEER
        config.supported_target_features |= AVRCP_CATEGORY_1; /* Peer devices also a target for category 1 commands (play, pause etc) */
#endif
        config.profile_extensions = AVRCP_VERSION_1_6; /* operate as AVRCP 1.6 device */    
    
    
#ifdef ENABLE_AVRCP_BROWSING
        config.profile_extensions |= AVRCP_BROWSING_SUPPORTED; /* indicate Browsing support */        
#endif    
   
   
        AvrcpInit(&theSink.task, &config);   
    }
    else
    {
        config.device_type = 0;
        config.supported_controller_features = 0; 
        config.supported_target_features = 0; 
        config.profile_extensions = 0;      
    }

    /* initialise avrcp data memory */
    memset(&AVRCP_DATA, 0, sizeof(AVRCP_DATA));

    /* Initialise local message handler */  
    for_all_avrcp(i)            
        AVRCP_DATA.avrcp_ctrl_handler[i].handler = avrcpControlHandler;
    /* Initialise free up message handler */  
    for_all_avrcp(i)            
    {
        (AVRCP_DATA.dataCleanUpTask[i].task).handler = sinkAvrcpDataCleanUp;
        AVRCP_DATA.dataCleanUpTask[i].data = NULL;
    }

    /* initialise Browsing variables */
    sinkAvrcpBrowsingChannelInit(TRUE, 0);

    AVRCP_DATA.avrcp_initialised = TRUE;
}

/****************************************************************************
*NAME    
*    sinkAvrcpDataCleanUp
*
*DESCRIPTION
*    handler function to free the allocated data. 
******************************************************************************/
void sinkAvrcpDataCleanUp(Task task, MessageId id, Message message)
{
    SinkAvrcpCleanUpTask *cleanTask = (SinkAvrcpCleanUpTask *) task;

    UNUSED(message); /* Not used as we are just discarding messages */

    switch (id)
    {
    case MESSAGE_SOURCE_EMPTY:
        {
            /* Free the previously stored data ptr. */
            if (cleanTask->data)
            {
                free(cleanTask->data);
                AVRCP_DEBUG(("avrcpDataCleanUp: Cleanup Stored Data \n"));
            }
            cleanTask->data = NULL;
        }
        break;

    default:
        AVRCP_DEBUG((" AVRCP handleUnexpected in "
                   " SinkAvrcpCleanUpTask- MsgId 0x%x\n",id));
        break;
    }
}

/*************************************************************************
NAME    
    updateNextActiveAvrcpConnection
    
DESCRIPTION
    Updates the next active AVRCP connection.

**************************************************************************/
static void updateNextActiveAvrcpConnection(void)
{   
    uint16 index, next_index;
    index = sinkAvrcpGetActiveConnection();
    if(getNextConnectedAvrcpIndex(&next_index, index))
    {
        sinkAvrcpSetActiveConnectionFromIndex(next_index);
    }
}

/*************************************************************************
NAME    
    sinkAvrcpDisconnect
    
DESCRIPTION
    Disconnect the AVRCP connection associated with the specified device.

**************************************************************************/
void sinkAvrcpDisconnect(const bdaddr *bd_addr)
{
    uint16 Index = 0;
    
    AVRCP_DEBUG(("AVRCP: Avrcp Disconnect\n"));
    if (sinkAvrcpGetIndexFromBdaddr(bd_addr, &Index, TRUE))
    {
        AVRCP_DEBUG(("    Disconnect [%x:%x:%lx] index[%d]\n", bd_addr->uap, bd_addr->nap, bd_addr->lap, Index));
        AvrcpDisconnectRequest(AVRCP_DATA.avrcp[Index]);
        if(sinkAvrcpGetActiveConnection() == Index)
        {
            updateNextActiveAvrcpConnection();
        }
    }
}


/*************************************************************************
NAME    
    sinkAvrcpDisconnectAll
    
DESCRIPTION
    Disconnect all AVRCP connections.

**************************************************************************/
void sinkAvrcpDisconnectAll(bool disconnect_peer)
{
    uint8 i;

#ifndef ENABLE_PEER
    UNUSED(disconnect_peer);
#endif

    AVRCP_DEBUG(("AVRCP: Avrcp Disconnect All\n"));
    if (sinkAvrcpIsInitialised())
    {
        /* loop for all AVRCP connections */
        for_all_avrcp(i)
        {
#ifdef ENABLE_PEER            
            if (AVRCP_DATA.connected[i] && ( !sinkAvrcpIsAvrcpIndexPeer(i, NULL) || disconnect_peer))
#else
            if (AVRCP_DATA.connected[i])
#endif                
            {              
                AvrcpDisconnectRequest(AVRCP_DATA.avrcp[i]);
                BdaddrSetZero(&AVRCP_DATA.bd_addr[i]);
            }
            AVRCP_DATA.avrcp_manual_connect = FALSE;
            BdaddrSetZero(&AVRCP_DATA.avrcp_play_addr);
        }
    }
}


/*************************************************************************
NAME    
    sinkAvrcpAclClosed
    
DESCRIPTION
    The ACL to the device specified has closed, so perform any clean up that is required

**************************************************************************/
void sinkAvrcpAclClosed(bdaddr bd_addr)
{
    uint16 Index;
    
    /* It is possible that a disconnect crossover can fail to inform the application of the AVRCP disconnection.
       Clean up any active connections if the ACL has disappeared */
    if (sinkAvrcpGetIndexFromBdaddr(&bd_addr, &Index, FALSE))
    {
        AVRCP_DEBUG(("AVRCP: Reset AVRCP connection as ACL closed %d\n", Index));
        resetAvrcpConnection(Index);
        sinkAvrcpBrowsingChannelInit(FALSE, Index);
    }
}

/*************************************************************************
NAME    
    sinkAvrcpConnect
    
DESCRIPTION
    Create an AVRCP connection with with the specified device.

**************************************************************************/
void sinkAvrcpConnect(const bdaddr *bd_addr, uint16 delay_time)
{
    uint16 connections = 0;
    uint16 i;
    bool bdaddr_set = FALSE;
    uint16 Index = 0;


    if ( !sinkAvrcpIsInitialised() || !bd_addr || BdaddrIsZero(bd_addr) )
    {   /* Avrcp data structure not initialised or invalid connection address */
        AVRCP_DEBUG(("AVRCP: Avrcp Connect ***unable to action*** - bd_addr=%p avrcp_initialised=%p\n", (void*) bd_addr, (void*) sinkAvrcpIsInitialised()));
        return;
    }

    AVRCP_DEBUG(("AVRCP: Avrcp Connect [%x:%x:%lx] delay[%d]\n", bd_addr->nap, bd_addr->uap, bd_addr->lap, delay_time));

    for_all_avrcp(i)
    {
        if (!BdaddrIsZero(&AVRCP_DATA.bd_addr[i]))
        {            
            /* count the number of currently active connections, and don't allow a futher connection to the same device */
            connections++;
            if (BdaddrIsSame(&AVRCP_DATA.bd_addr[i], bd_addr))
            {         
                return;
            }
        }
        else if (!bdaddr_set)
        {            
            /* store the address of the attempted connection */
            AVRCP_DATA.bd_addr[i] = *bd_addr;
            Index = i;
            bdaddr_set = TRUE;                        
        }
    }
        
    if ((connections < MAX_AVRCP_CONNECTIONS) && bdaddr_set)
    {
        if (!delay_time)
        {        
            AVRCP_DEBUG(("   Avrcp connect request\n"));
            AvrcpConnectRequest(&theSink.task, bd_addr);
        }
        else
        {
            MAKE_AVRCP_MESSAGE(AVRCP_CREATE_CONNECTION);        
            message->bd_addr = *bd_addr;
            AVRCP_DEBUG(("   Avrcp delayed connect [%d]\n", delay_time));
            MessageSendLater(&AVRCP_DATA.avrcp_ctrl_handler[Index], AVRCP_CREATE_CONNECTION, message, delay_time);
        }
    }
}

/*************************************************************************
NAME    
    sinkAvrcpManualConnect
    
DESCRIPTION
    Create an manual AVRCP connection with  the specified device

**************************************************************************/
void sinkAvrcpManualConnect(void)
{
    uint16 i;
    for_all_a2dp(i)
    {
        /* see if an a2dp link is connected without AVRCP */
        if (getA2dpStatusFlag(CONNECTED, i))
        {                
            if (getAgAvrcpSupportStatus(i) != avrcp_support_unsupported)
            {
                const bdaddr *a2dpBdaddr = getA2dpLinkBdAddr(i);
                setAgAvrcpSupportStatus(i, avrcp_support_unknown);
                AVRCP_DATA.avrcp_manual_connect = TRUE;
                AVRCP_DATA.avrcp_play_addr = *a2dpBdaddr;
                sinkAvrcpConnect(a2dpBdaddr, DEFAULT_AVRCP_NO_CONNECTION_DELAY);
                AVRCP_DEBUG(("AVRCP: Retry AVRCP connection\n"));
            }
            else
            {
                /* TODO if AVRCP can't be connected, control stopping and starting of A2DP audio here */
                AVRCP_DEBUG(("AVRCP: AVRCP not supported\n"));
            }
            return;
        }
    }
    AVRCP_DEBUG(("AVRCP: Establish SLC connections\n"));
    AVRCP_DATA.avrcp_manual_connect = TRUE;
    MessageSend(&theSink.task, EventUsrEstablishSLC, 0 );
}

#ifdef ENABLE_PEER        
/*************************************************************************
NAME    
    sinkAvrcpVolumeUp
    
DESCRIPTION
    Send an AVRCP_VOLUME_UP to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpVolumeUp (void)
{
    uint16 Index;
    
    AVRCP_DEBUG(("AVRCP: Avrcp Volume Up\n"));
    
    /* Check if the provided index is of a true AV device or a remote device with a true Av conncted , otherwise find the next valid index*/
    if (avrcpGetPeerIndex(&Index))
    {
        if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 2))
        {
            avrcpSendControlMessage(AVRCP_CTRL_VOLUME_UP_PRESS, Index);
            avrcpSendControlMessage(AVRCP_CTRL_VOLUME_UP_RELEASE, Index);
        }
    }
}
#endif

#ifdef ENABLE_PEER        
/*************************************************************************
NAME    
    sinkAvrcpVolumeDown
    
DESCRIPTION
    Send an AVRCP_VOLUME_DOWN to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpVolumeDown (void)
{
    uint16 Index;
    
    AVRCP_DEBUG(("AVRCP: Avrcp Volume Up\n"));
    
    /* Check if the provided index is of a true AV device or a remote device with a true Av conncted , otherwise find the next valid index*/
    if (avrcpGetPeerIndex(&Index))
    {
        if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 2))
        {
            avrcpSendControlMessage(AVRCP_CTRL_VOLUME_DOWN_PRESS, Index);
            avrcpSendControlMessage(AVRCP_CTRL_VOLUME_DOWN_RELEASE, Index);
        }
    }
}
#endif

#ifdef ENABLE_PEER        
/*************************************************************************
NAME    
    sinkAvrcpPowerOff
    
DESCRIPTION
    Send an AVRCP_POWER to the peer device.
    
**************************************************************************/
void sinkAvrcpPeerPowerOff (void)
{
    uint16 Index;
    
    AVRCP_DEBUG(("AVRCP: Avrcp Power off\n"));
    
    if (avrcpGetPeerIndex(&Index))
    {
        if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1))
        {
            avrcpSendControlMessage(AVRCP_CTRL_POWER_OFF, Index);           
        }
    }
}
#endif


/*************************************************************************
NAME    
    sinkAvrcpPlay
    
DESCRIPTION
    Send an AVRCP_PLAY to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpPlay(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    uint16 IndexToUse = 0;

    AVRCP_DEBUG(("AVRCP: Avrcp Play\n"));

#ifdef ENABLE_PEER
    /* Check if the provided index is of a true AV device or a remote device with a true Av conncted , otherwise find the next valid index*/
    if(getValidAvrcpIndex(&IndexToUse , Index))
    {
        if(IndexToUse != Index)
        {
            /* We have a new index so update the active connection*/
            sinkAvrcpSetActiveConnectionFromBdaddr( &(AVRCP_DATA.bd_addr[IndexToUse]));
        }
#else
     if (AVRCP_DATA.connected[Index])
     {
        IndexToUse = Index;
#endif        
        if (getAvrcpQueueSpace(IndexToUse) >= 2)
        {
            avrcpSendControlMessage(AVRCP_CTRL_PLAY_PRESS, IndexToUse);
            avrcpSendControlMessage(AVRCP_CTRL_PLAY_RELEASE, IndexToUse);
            if (!isAvrcpPlaybackStatusSupported(IndexToUse))
            {
                setPlayStatus(IndexToUse, sinkavrcp_play_status_playing);
            }
#ifdef ENABLE_PEER            
            /* If single device operation is disabled and the IndexToUse is a peer device then mark the peer relay as available */
            if( !peerIsSingleDeviceOperationEnabled() && sinkAvrcpIsAvrcpIndexPeer(IndexToUse, NULL) )
            {
                peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_RELAY_AVAILABLE);
            }
#endif
        }
     }
    else
    {
        /* Couldn't find a valid index to route this comand so connect manually*/        
        sinkAvrcpManualConnect();
    }
}


/*************************************************************************
NAME    
    sinkAvrcpPause
    
DESCRIPTION
    Send an AVRCP_PAUSE to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpPause(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();     
    uint16 IndexToUse = 0;
    
    AVRCP_DEBUG(("AVRCP: Avrcp Pause\n"));
    
#ifdef ENABLE_PEER        
    /* Check if the provided index is of a true AV device or a remote device with a true Av conncted , otherwise find the next valid index*/
    if(getValidAvrcpIndex(&IndexToUse , Index))
    {
        if(IndexToUse != Index)
        {
            /* We have a new index so update the active connection*/
            sinkAvrcpSetActiveConnectionFromBdaddr( &(AVRCP_DATA.bd_addr[IndexToUse]));
        }
#else
     if (AVRCP_DATA.connected[Index])
     {
        IndexToUse = Index;
#endif 
        if ( (getAvrcpQueueSpace(IndexToUse) >= 2) && 
              (AVRCP_DATA.play_status[IndexToUse] == sinkavrcp_play_status_playing))
        {
            avrcpSendControlMessage(AVRCP_CTRL_PAUSE_PRESS, IndexToUse);
            avrcpSendControlMessage(AVRCP_CTRL_PAUSE_RELEASE, IndexToUse);
            if (!isAvrcpPlaybackStatusSupported(IndexToUse))
            {
                setPlayStatus(IndexToUse, sinkavrcp_play_status_paused);
            }
#ifdef ENABLE_PEER            
            /* If single device operation is disabled and the IndexToUse is a peer device then mark the peer relay as unavailable */
            if( !peerIsSingleDeviceOperationEnabled() && sinkAvrcpIsAvrcpIndexPeer(IndexToUse, NULL) )
            {
                peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_RELAY_UNAVAILABLE);
            }
#endif
        }
    }
    else
    {
        sinkAvrcpManualConnect();
    }
}

/*************************************************************************
NAME    
    sinkAvrcpPlayPause
    
DESCRIPTION
    Send a AVRCP_PLAY / AVRCP_PAUSE based on play status, to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpPlayPause(void)
{
    a2dp_index_t  a2dpIndex = 0;
    uint16 Index = sinkAvrcpGetActiveConnection();
    uint16 IndexToUse = 0;

    AVRCP_DEBUG(("AVRCP: Avrcp PlayPause\n"));

#ifdef ENABLE_PEER        
    /* Check if the provided index is of a true AV device or a remote device with a true Av conncted , otherwise find the next valid index*/
    if(getValidAvrcpIndex(&IndexToUse , Index))
    {
        if(IndexToUse != Index)
        {
            /* We have a new index so update the active connection*/
            sinkAvrcpSetActiveConnectionFromBdaddr( &(AVRCP_DATA.bd_addr[IndexToUse]));
        }
#else
     if (AVRCP_DATA.connected[Index])
     {

        IndexToUse = Index;
#endif /* ENABLE_PEER */
        getA2dpIndexFromAvrcp(Index, &a2dpIndex);
        
        if ((AVRCP_DATA.play_status[IndexToUse] != sinkavrcp_play_status_playing)
                || sinkA2dpIsA2dpLinkSuspended(a2dpIndex))
        {
            sinkAvrcpPlay();
        }
        else
        {
            sinkAvrcpPause();            
        }
    }
    else
    {
       sinkAvrcpManualConnect();
    }
}


/*************************************************************************
NAME    
    sinkAvrcpStop
    
DESCRIPTION
    Send a AVRCP_STOP to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpStop(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Stop\n"));
    
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 2))
    {
        avrcpSendControlMessage(AVRCP_CTRL_STOP_PRESS, Index);
        avrcpSendControlMessage(AVRCP_CTRL_STOP_RELEASE, Index);
        if (!isAvrcpPlaybackStatusSupported(Index))
            setPlayStatus(Index, sinkavrcp_play_status_stopped);
    }
}


/*************************************************************************
NAME    
    sinkAvrcpSkipForward
    
DESCRIPTION
    Send a AVRCP_FORWARD to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpSkipForward(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Skip Fwd\n"));
    
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 2))
    {
        avrcpSendControlMessage(AVRCP_CTRL_FORWARD_PRESS, Index);
        avrcpSendControlMessage(AVRCP_CTRL_FORWARD_RELEASE, Index);
    }
}


/*************************************************************************
NAME    
    sinkAvrcpSkipBackward
    
DESCRIPTION
    Send a AVRCP_BACKWARD to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpSkipBackward(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Skip Bwd\n"));
   
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 2))
    {
        avrcpSendControlMessage(AVRCP_CTRL_BACKWARD_PRESS, Index);
        avrcpSendControlMessage(AVRCP_CTRL_BACKWARD_RELEASE, Index);
    }
}


/*************************************************************************
NAME    
    sinkAvrcpFastForwardPress
    
DESCRIPTION
    Send a AVRCP_FAST_FORWARD pressed to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpFastForwardPress(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp FFWD Press\n"));
    
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1))
    {
        avrcpSendControlMessage(AVRCP_CTRL_FF_PRESS, Index);
        if (!isAvrcpPlaybackStatusSupported(Index))
        {
            /* use bitmask for fwd and rev so with AVRCP 1.0 devices can store previous play status before this operation */
            AVRCP_DATA.play_status[Index] |=  sinkavrcp_play_status_fwd_seek;
            AVRCP_DATA.play_status[Index] &= ~sinkavrcp_play_status_rev_seek;
        }
    }
}


/*************************************************************************
NAME    
    sinkAvrcpFastForwardRelease
    
DESCRIPTION
    Send a AVRCP_FAST_FORWARD released to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpFastForwardRelease(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp FFWD Release\n"));
   
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1))
    {
        avrcpSendControlMessage(AVRCP_CTRL_FF_RELEASE, Index);
        if (!isAvrcpPlaybackStatusSupported(Index))
            AVRCP_DATA.play_status[Index] &= ~sinkavrcp_play_status_fwd_seek;
    }
}


/*************************************************************************
NAME    
    sinkAvrcpRewindPress
    
DESCRIPTION
    Send a AVRCP_REWIND pressed to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpRewindPress(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp REW Press\n"));
   
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1))
    {
        avrcpSendControlMessage(AVRCP_CTRL_REW_PRESS, Index);
        if (!isAvrcpPlaybackStatusSupported(Index))
        {
            AVRCP_DATA.play_status[Index] |=  sinkavrcp_play_status_rev_seek;
            AVRCP_DATA.play_status[Index] &= ~sinkavrcp_play_status_fwd_seek;
        }
    }
}


/*************************************************************************
NAME    
    sinkAvrcpRewindRelease
    
DESCRIPTION
    Send a AVRCP_REWIND released to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpRewindRelease(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp REW Release\n"));
  
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1))
    {
        avrcpSendControlMessage(AVRCP_CTRL_REW_RELEASE, Index);
        if (!isAvrcpPlaybackStatusSupported(Index))
            AVRCP_DATA.play_status[Index] &= ~sinkavrcp_play_status_rev_seek;
    }
}


/*************************************************************************
NAME    
    sinkAvrcpNextGroupPress
    
DESCRIPTION
    Send a Next Group Press command to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpNextGroupPress(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Next Group Press\n"));
  
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1) && isAvrcpGroupNavigationEnabled(Index))
    {
        avrcpSendControlMessage(AVRCP_CTRL_NEXT_GROUP_PRESS, Index);
    }
}

/*************************************************************************
NAME    
    sinkAvrcpNextGroupRelease
    
DESCRIPTION
    Send a Next Group Release command to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpNextGroupRelease(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Next Group Release\n"));
  
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1) && isAvrcpGroupNavigationEnabled(Index))
    {
        avrcpSendControlMessage(AVRCP_CTRL_NEXT_GROUP_RELEASE, Index);        
    }
}


/*************************************************************************
NAME    
    sinkAvrcpPreviousGroupPress
    
DESCRIPTION
    Send a Previous Group Press command to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpPreviousGroupPress(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Previous Group Press\n"));
  
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1) && isAvrcpGroupNavigationEnabled(Index))
    {
        avrcpSendControlMessage(AVRCP_CTRL_PREVIOUS_GROUP_PRESS, Index); 
    }
}

/*************************************************************************
NAME    
    sinkAvrcpPreviousGroupRelease
    
DESCRIPTION
    Send a Previous Group Release command to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpPreviousGroupRelease(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Previous Group Release\n"));
  
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1) && isAvrcpGroupNavigationEnabled(Index))
    {
        avrcpSendControlMessage(AVRCP_CTRL_PREVIOUS_GROUP_RELEASE, Index); 
    }
}

/****************************************************************************
* NAME    
* sinkAvrcpSourceFromData    
*
* DESCRIPTION
*  Create a Source from the passed memory location.
*
* RETURNS
*    void
*******************************************************************************/
Source sinkAvrcpSourceFromData(SinkAvrcpCleanUpTask *avrcp_cleanup, uint8 *data, uint16 length)
{
    /* Create a source from the data */
    Source src = StreamRegionSource(data, length);

    /* The clean-up task clears this so should be NULL at this point */
    PanicNotNull(avrcp_cleanup->data);

    /* Register a task for freeing the data and store a ptr to it */
    avrcp_cleanup->data = data;
    MessageStreamTaskFromSink(StreamSinkFromSource(src), &avrcp_cleanup->task);

    return src;
}


/*************************************************************************
NAME    
    sinkAvrcpGetShuffleType
    
DESCRIPTION
    Map the user requested shuffle type to spec value

**************************************************************************/
uint8 sinkAvrcpGetShuffleType(avrcp_shuffle_t type)
{
    uint16 retval = 0;

    switch(type)
    {
        case AVRCP_SHUFFLE_OFF:            
            AVRCP_DEBUG(("AVRCP_PLAYER_VALUE_SHUFFLE_OFF\n"));
            retval = AVRCP_PLAYER_VALUE_SHUFFLE_OFF;
            break;
            
        case AVRCP_SHUFFLE_ALL_TRACK:            
            AVRCP_DEBUG(("AVRCP_PLAYER_VALUE_SHUFFLE_ALL\n"));
            retval = AVRCP_PLAYER_VALUE_SHUFFLE_ALL;
            break;

        case AVRCP_SHUFFLE_GROUP:            
            AVRCP_DEBUG(("AVRCP_PLAYER_VALUE_SHUFFLE_GROUP\n"));
            retval = AVRCP_PLAYER_VALUE_SHUFFLE_GROUP;
            break;
        
        default:
            /* Not supported. */
            break;
    }
    
    return retval;
}

/*************************************************************************
NAME    
    sinkAvrcpShuffle
    
DESCRIPTION
    Send a Shuffle command to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpShuffle(avrcp_shuffle_t type)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Shuffle \n"));
  
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1) )
    {
#ifdef ENABLE_AVRCP_PLAYER_APP_SETTINGS

        if ((isAvrcpPlayerApplicationSettingsSupported(Index)) &&
            (AVRCP_DATA.event_capabilities[Index] & (1 << avrcp_event_player_app_setting_changed))
            && (type != 0))           
        {   
            Source src;    

            uint8 *data = (uint8 *) PanicUnlessMalloc(AVRCP_PLAYER_APP_SETTINGS_DATA_LEN);/*Freed by cleanup task*/

            data[0] = AVRCP_PLAYER_ATTRIBUTE_SHUFFLE;
            data[1] = sinkAvrcpGetShuffleType(type);

            AVRCP_DEBUG(("AVRCP: Avrcp Shuffle Command Sent \n"));
            src = sinkAvrcpSourceFromData(&AVRCP_DATA.dataCleanUpTask[Index], data, AVRCP_PLAYER_APP_SETTINGS_DATA_LEN);
            AvrcpSetAppValueRequest(AVRCP_DATA.avrcp[Index], AVRCP_PLAYER_APP_SETTINGS_DATA_LEN, src);
        }
#else
            UNUSED(type);
#endif  /* ENABLE_AVRCP_PLAYER_APP_SETTINGS */
    }
}


/*************************************************************************
NAME    
    sinkAvrcpGetRepeatType
    
DESCRIPTION
    Map the user requested repeat type to spec value

**************************************************************************/
uint8 sinkAvrcpGetRepeatType(avrcp_repeat_t type)
{
    uint16 retval = 0;

    switch(type)
    {
        case AVRCP_REPEAT_OFF:
            AVRCP_DEBUG(("AVRCP_PLAYER_VALUE_REPEAT_MODE_OFF\n"));
            retval = AVRCP_PLAYER_VALUE_REPEAT_MODE_OFF;
            break;
            
        case AVRCP_REPEAT_SINGLE_TRACK:
            AVRCP_DEBUG(("AVRCP_PLAYER_VALUE_REPEAT_MODE_SINGLE\n"));
            retval = AVRCP_PLAYER_VALUE_REPEAT_MODE_SINGLE;
            break;

        case AVRCP_REPEAT_ALL_TRACK:            
            AVRCP_DEBUG(("AVRCP_PLAYER_VALUE_REPEAT_MODE_ALL\n"));
            retval = AVRCP_PLAYER_VALUE_REPEAT_MODE_ALL;
            break;

        case AVRCP_REPEAT_GROUP:            
            AVRCP_DEBUG(("AVRCP_PLAYER_VALUE_REPEAT_MODE_GROUP\n"));
            retval = AVRCP_PLAYER_VALUE_REPEAT_MODE_GROUP;
            break;
            
        default:
            /* Not supported. */
            break;
    }
    
    return retval;
}

/*************************************************************************
NAME    
    sinkAvrcpRepeat
    
DESCRIPTION
    Send a Repeat command to the device with the currently active AVRCP connection.

**************************************************************************/
void sinkAvrcpRepeat(avrcp_repeat_t type)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Repeat \n"));
  
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 1) )
    {
#ifdef ENABLE_AVRCP_PLAYER_APP_SETTINGS
        if ((isAvrcpPlayerApplicationSettingsSupported(Index)) &&
            (AVRCP_DATA.event_capabilities[Index] & (1 << avrcp_event_player_app_setting_changed))
            && (type != 0))           
        {   
            Source src;    

            uint8 *data = (uint8 *) PanicUnlessMalloc(AVRCP_PLAYER_APP_SETTINGS_DATA_LEN);/*Freed by cleanup task*/

            data[0] = AVRCP_PLAYER_ATTRIBUTE_REPEAT_MODE;
            data[1] = sinkAvrcpGetRepeatType(type);
            
            AVRCP_DEBUG(("AVRCP: Avrcp Repeat Command Sent \n"));
            src = sinkAvrcpSourceFromData(&AVRCP_DATA.dataCleanUpTask[Index], data, AVRCP_PLAYER_APP_SETTINGS_DATA_LEN);
            AvrcpSetAppValueRequest(AVRCP_DATA.avrcp[Index], AVRCP_PLAYER_APP_SETTINGS_DATA_LEN, src);
        }
#else
    UNUSED(type);
#endif  /* ENABLE_AVRCP_PLAYER_APP_SETTINGS */
    }
}

/*************************************************************************
NAME    
    sinkAvrcpToggleActiveConnection
    
DESCRIPTION
    Toggles between active AVRCP connections.

**************************************************************************/
void sinkAvrcpToggleActiveConnection(void)
{
    uint16 start_index = sinkAvrcpGetActiveConnection();
    uint16 search_index = start_index + 1;
    
    while (search_index != start_index)
    {
        if (search_index >= MAX_AVRCP_CONNECTIONS)
            search_index = 0;
        if (AVRCP_DATA.connected[search_index])
            break;
        search_index++;
    }
    
    AVRCP_DEBUG(("AVRCP: Toggle Active connection[%d -> %d]\n", start_index,
                search_index))

    /* switch to new connection if required */
    sinkAvrcpSetActiveConnectionFromIndex(search_index);
}


/*************************************************************************
NAME    
    sinkAvrcpUpdateVolume
    
DESCRIPTION
    Update the absolute volume (AVRCP 1.5) and notify
    the remote device if this was a local change of volume, and it requested to be notified. 

**************************************************************************/
void sinkAvrcpUpdateVolume(const int16 volume)
{
    if(avrcpAvrcpIsEnabled())
    {
        uint16 Index = sinkAvrcpGetActiveConnection();

        if (AVRCP_DATA.connected[Index])
        {
            sinkAvrcpSetLocalVolume(Index, volume);
        }
    }
}

/*************************************************************************
NAME    
    sinkAvrcpSetLocalVolume
    
DESCRIPTION
    The volume has been locally set so update the absolute volume (AVRCP 1.5) and notify 
    the remote device if this was a local change of volume, and it requested to be notified.

**************************************************************************/
static void sinkAvrcpSetLocalVolume(const uint16 Index, const uint16 a2dp_step)
{
    /* convert digital volume into avrcp scaled volume 0 to 127 */
    AVRCP_DATA.absolute_volume[Index] = sinkAvrcpConvertA2dpStepToAvrcpVolume(a2dp_step);

    if (AVRCP_DATA.registered_events[Index] & (1 << avrcp_event_volume_changed))
    {
        AVRCP_DEBUG(("  Notify remote device [local:%d][abs:%d]\n", a2dp_step, AVRCP_DATA.absolute_volume[Index]));
        AvrcpEventVolumeChangedResponse(AVRCP_DATA.avrcp[Index], 
                                        avctp_response_changed,
                                        AVRCP_DATA.absolute_volume[Index]);  
        /* reset registered volume notification */
        AVRCP_DATA.registered_events[Index] &= ~(1 << avrcp_event_volume_changed);
    }
}


/*************************************************************************
NAME    
    sinkAvrcpGetIndexFromInstance
    
DESCRIPTION
    Retrieve the correct AVRCP connection index based on the AVRCP library instance pointer.
   
RETURNS
    Returns TRUE if the AVRCP connection was found, FASLE otherwise.
    The actual connection index is returned in the Index variable.

**************************************************************************/
bool sinkAvrcpGetIndexFromInstance(const AVRCP *avrcp, uint16 *Index)
{
    uint8 i;
    
    /* go through Avrcp connections looking for device_id match */
    for_all_avrcp(i)
    {
        /* if the avrcp link is connected check for its AVRCP pointer */
        if (AVRCP_DATA.connected[i])
        {
            /* if a device_id match is found return its value and a
               status of successful match found */
            if (avrcp && (AVRCP_DATA.avrcp[i] == avrcp))
            {
                *Index = i;
                AVRCP_DEBUG(("AVRCP: getIndex = %d\n", i)); 
                return TRUE;
            }
        }
    }
    /* no matches found so return not successful */    
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpGetIndexFromBdaddr
    
DESCRIPTION
    Retrieve the correct AVRCP connection index based on the Bluetooth address.
   
RETURNS
    Returns TRUE if the AVRCP connection was found, FALSE otherwise.
    The actual connection index is returned in the Index variable.

**************************************************************************/
bool sinkAvrcpGetIndexFromBdaddr(const bdaddr *bd_addr, uint16 *Index, bool require_connection)
{
    uint8 i;
    
    /* go through Avrcp connections looking for device_id match */
    for_all_avrcp(i)
    {
        if ((require_connection && AVRCP_DATA.connected[i]) || !require_connection)
        {        
            if (BdaddrIsSame(&AVRCP_DATA.bd_addr[i], bd_addr))
            {
                *Index = i;
                AVRCP_DEBUG(("AVRCP: getIndex = %d\n", i)); 
                return TRUE;
            }
        }
    }
    /* no matches found so return not successful */    
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpGetActiveConnectionFromCurrentIndex
    
DESCRIPTION
    Retrieve the Bluetooth address from the current AVRCP connection index
   
RETURNS
    none
    The bd_addr is returned in bd_addr if present in the avrcp_link_data.

**************************************************************************/
void sinkAvrcpGetActiveConnectionFromCurrentIndex(bdaddr *bd_addr)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    uint16 IndexToUse = 0;

    AVRCP_DEBUG(("AVRCP: sinkAvrcpGetActiveConnectionFromCurrentIndex\n"));

#ifdef ENABLE_PEER        
    /* Check if the provided index is of a true AV device or a remote device with a true Av conncted , otherwise find the next valid index*/
    if(getValidAvrcpIndex(&IndexToUse , Index))
    {
        if(IndexToUse != Index)
        {
            /* We have a new index so update the active connection*/
            sinkAvrcpSetActiveConnectionFromBdaddr( &(AVRCP_DATA.bd_addr[IndexToUse]));
        }
#else
     if (AVRCP_DATA.connected[Index])
     {
          IndexToUse = Index;
#endif /* ENABLE_PEER */

          bd_addr->lap = AVRCP_DATA.bd_addr[IndexToUse].lap;
          bd_addr->nap = AVRCP_DATA.bd_addr[IndexToUse].nap;
          bd_addr->uap = AVRCP_DATA.bd_addr[IndexToUse].uap;
     }
}


/*************************************************************************
NAME    
    sinkAvrcpSetActiveConnectionFromBdaddr
    
DESCRIPTION
    Sets the active AVRCP connection based on Bluetooth address.

**************************************************************************/
void sinkAvrcpSetActiveConnectionFromBdaddr(const bdaddr *bd_addr)
{
    uint16 Index;
    
    if (sinkAvrcpGetIndexFromBdaddr(bd_addr, &Index, TRUE))
    {
        sinkAvrcpSetActiveConnectionFromIndex(Index);
    }
}

/*************************************************************************
NAME    
    sinkAvrcpSetActiveConnectionFromIndex
    
DESCRIPTION
    Sets the active AVRCP connection based on Bluetooth address.

**************************************************************************/
static void sinkAvrcpSetActiveConnectionFromIndex(uint16 new_index)
{
    if(AVRCP_DATA.active_avrcp != new_index)
    {
        AVRCP_DEBUG(("AVRCP: Set Active connection[%d]\n", new_index));
        /* set new active connection */
        updateActiveConnection(new_index);
#ifdef ENABLE_AVRCP_NOW_PLAYING     
        /* sources have been switched, update the current playing track */
        sinkAvrcpRetrieveNowPlayingNoBrowsingRequest(new_index, FALSE);
#endif            
    }
}

/*************************************************************************
NAME    
    sinkAvrcpFlushAndResetStandardCommands
    
DESCRIPTION
    Flushes any pending messages and resets queued non-vendor specific commands

**************************************************************************/
static void sinkAvrcpFlushAndResetStandardCommands(uint16 index)
{
    /* Remove pending messages */
    MessageFlushTask(&AVRCP_DATA.avrcp_ctrl_handler[index]);
    sinkAvrcpBrowsingFlushHandlerTask(index);
    sinkAvrcpBrowsingChannelDisconnectRequest(AVRCP_DATA.avrcp[index]);
    
    /* Reset the pending message count */
    AVRCP_DATA.cmd_queue_size[index] = 0;

    /* reset the pending flag unless pending command is vendor specific */
        AVRCP_DATA.pending_cmd[index] = FALSE;
}

/*************************************************************************
NAME    
    sinkAvrcpResetVendorCommands
    
DESCRIPTION
    Frees any memory associated with vendor specific commands

**************************************************************************/
static void sinkAvrcpResetVendorCommands(uint16 index)
{
    if (AVRCP_DATA.vendor_data[index])
    {
        /* Remove pending PassThrough requests of AVRCP lib task from message queue, which will use vendor_data */
        AvrcpPassthroughRequestCancelAll(AVRCP_DATA.avrcp[index]);

        /* 
         * There is a memory buffer still pointed to that
         * now needs freeing
         */
        free(AVRCP_DATA.vendor_data[index]);
        AVRCP_DATA.vendor_data[index] = NULL;
    }
}
        
/*************************************************************************
NAME    
    sinkAvrcpGetActiveConnection
    
DESCRIPTION
    Gets the active AVRCP connection.

**************************************************************************/
uint16 sinkAvrcpGetActiveConnection(void)
{
    return AVRCP_DATA.active_avrcp;
}
        
/*************************************************************************
NAME    
    sinkAvrcpSetActiveConnection
    
DESCRIPTION
    Sets the active AVRCP connection.

**************************************************************************/
void  sinkAvrcpSetActiveConnection(uint8 active_avrcp)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpSetActiveConnection(active_avrcp = %x)\n",active_avrcp));
    updateActiveConnection(active_avrcp);
}

static bool isAvrcpIndexValid(uint16 index)
{
    bool is_valid = FALSE;
    if(index < MAX_AVRCP_CONNECTIONS)
    {
        is_valid = TRUE;
    }
    return is_valid;
}

/*************************************************************************
NAME    
    sinkAvrcpIsConnected
    
DESCRIPTION
    Checks AVRCP is connected for a particular Index.

    index - Connection Index
    Return TRUE if connection exists, FALSE if connection does not exist

**************************************************************************/
bool sinkAvrcpIsConnected(uint16 index)
{
    if(isAvrcpIndexValid(index))
    {
    AVRCP_DEBUG(("AVRCP: sinkAvrcpIsConnected(index = %d, Connected: %x) \n",index,AVRCP_DATA.connected[index]));

    return (AVRCP_DATA.connected[index]) ? TRUE: FALSE;
}
    return FALSE;
}

/*************************************************************************
NAME    
    sinkAvrcpIsManualConnectEnabled
    
DESCRIPTION
    Checks AVRCP manual connect is enabled.

    Return TRUE if enabled, FALSE if disabled

**************************************************************************/
bool sinkAvrcpIsManualConnectEnabled(void)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpIsManualConnectEnabled(), Enabled: %x \n",AVRCP_DATA.avrcp_manual_connect));

    return  (AVRCP_DATA.avrcp_manual_connect)? TRUE : FALSE;
}

/*************************************************************************
NAME    
    sinkAvrcpEnableManualConnect
    
DESCRIPTION
    Set AVRCP manual connect Flag.

    enable-Enable/Disable avrcp manual connect

**************************************************************************/
void sinkAvrcpEnableManualConnect(bool enable)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpEnableManualConnect(enable = %d) \n",enable));

    AVRCP_DATA.avrcp_manual_connect = enable;
}

/*************************************************************************
NAME    
    sinkAvrcpGetLinkBdAddr
    
DESCRIPTION
    Interface to get the avrcp bd address for index link.

     Return a pointer to the bd address of link

**************************************************************************/
bdaddr *sinkAvrcpGetLinkBdAddr(uint16 index)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpGetLinkBdAddr(index = %d) \n",index));

    return &AVRCP_DATA.bd_addr[index];
}

/*************************************************************************
NAME    
    sinkAvrcpGetPlayStatus
    
DESCRIPTION
    Interface to get the avrcpPlay Status.

     Return avrcp_play_status type

**************************************************************************/
avrcp_play_status sinkAvrcpGetPlayStatus(uint16 index)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpGetPlayStatus(index = %d) \n",index));

    return AVRCP_DATA.play_status[index];
}

/*************************************************************************
NAME    
    sinkAvrcpSetPlayStatusbyIndex
    
DESCRIPTION
    Interface to Set the avrcpPlay Status.

     index-Index to the link where play status need to be set
     play_status - Play status of type avrcp_play_status 

**************************************************************************/
void sinkAvrcpSetPlayStatusbyIndex(uint16 index,avrcp_play_status play_status)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpGetPlayStatus(index = %d,avrcp_play_status=%d) \n",index,play_status));

    setPlayStatus(index, play_status);
}

/*************************************************************************
NAME    
    sinkAvrcpUpdateLinkActiveStatus
    
DESCRIPTION
    Interface to update the active status of a link .

    Index-Index to the link where active status need to be updated
    status - Active status of the indexed link

**************************************************************************/
void sinkAvrcpUpdateLinkActiveStatus(uint16 index,bool status)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpUpdateLinkActiveStatus(index =%d,status =%d) \n",index,status));

    AVRCP_DATA.link_active[index] =  status;
}

/*************************************************************************
NAME    
    sinkAvrcpSetPlayAddress
    
DESCRIPTION
    Interface to set play address of AVRCP .

    play_addr-Address that has to be set

**************************************************************************/
void sinkAvrcpSetPlayAddress( bdaddr play_addr)
{
     AVRCP_DATA.avrcp_play_addr = play_addr;
}

/*************************************************************************
NAME    
    sinkAvrcpGetProfileInstance
    
DESCRIPTION
    Interface to get the AVRCP profile instance .

    Index-index of the profile instace

    Return a pointer to AVRCP profile instance

**************************************************************************/
AVRCP* sinkAvrcpGetProfileInstance(uint16 index)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpGetProfileInstance(index=%d) \n",index));

    return AVRCP_DATA.avrcp[index];
}

/*************************************************************************
NAME    
    sinkAvrcpResetProfileInstance
    
DESCRIPTION
    Reset avrcp instance with NULL .

    index-index of the instace to reset

**************************************************************************/
void sinkAvrcpResetProfileInstance(uint16 index)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpResetProfileInstance(Index = %d) \n",index));

    AVRCP_DATA.avrcp[index] = NULL;
}
/*************************************************************************
NAME    
    sinkAvrcpUpdateAbsoluteVolume
    
DESCRIPTION
    Interface to Update the absolute volume .

    index-Index for the instace on which Absolute Volume need to be updated

**************************************************************************/
void sinkAvrcpUpdateAbsoluteVolume(uint16 index, uint16 volume)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpUpdateAbsoluteVolume(Index = %d,volume = %d) \n",index,volume));

    AVRCP_DATA.absolute_volume[index] = volume;
}

/*************************************************************************
NAME    
    sinkAvrcpResetEventCapabilities
    
DESCRIPTION
    Interface to update the event capability.

    Index-index for the instance on which event capability need to be updated

**************************************************************************/
void sinkAvrcpResetEventCapabilities(uint16 index,avrcp_supported_events event)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpResetEventCapabilities(Index = %d,event = %d) \n",index,event));

    AVRCP_DATA.event_capabilities[index] &= ~(1 << event);
}

/*************************************************************************
NAME    
    sinkAvrcpUpdateRegisteredEvent
    
DESCRIPTION
    Interface to Update supported event ID.

    Index-index for the instance on which event ID need to be updated

**************************************************************************/
void sinkAvrcpUpdateRegisteredEvent(uint16 index,avrcp_supported_events eventid)
{
    AVRCP_DATA.registered_events[index] |= (1 <<eventid);
    AVRCP_DEBUG((" AVRCP:sinkAvrcpUpdateRegisteredEvent(),Registered events [%d] index [%d]\n", AVRCP_DATA.registered_events[index], index));
}

/*************************************************************************
NAME    
    sinkAvrcpResetRegisteredEvent
    
DESCRIPTION
    Interface to reset a registered event flag

    Index-index for the instance
    
**************************************************************************/
void sinkAvrcpResetRegisteredEvent(uint16 index,avrcp_supported_events eventid)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpResetRegisteredEvent(Index = %d,event = %d) \n",index,eventid));

    AVRCP_DATA.registered_events[index] &= ~(1 << eventid);
}

/*************************************************************************
NAME    
    sinkAvrcpGetFeatures
    
DESCRIPTION
    Interface to get AVRCP features

    Index-index for the instace

    Return AVRCP feature,uint16 value
    
**************************************************************************/
uint16 sinkAvrcpGetFeatures(uint16 index)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpGetFeatures(Index = %x),Features=  0x%x \n",index,AVRCP_DATA.features[index]));

    return AVRCP_DATA.features[index];
}
/*************************************************************************
NAME    
    sinkAvrcpIsInitialised
    
DESCRIPTION
    Check if sink_avrcp module has been initialised.

RETURNS
    TRUE if sinkAvrcpInit() has been called, FALSE otherwise.

**************************************************************************/
bool sinkAvrcpIsInitialised(void)
{
    return (AVRCP_DATA.avrcp_initialised ? TRUE : FALSE);
}

/*************************************************************************
NAME    
    sinkAvrcpIsAvrcpAudioSwitchingEnabled
    
DESCRIPTION
    Check AVRCP audio switching enabled.

    Return TRUE : Enabled, FALSE: Disabled

**************************************************************************/
bool sinkAvrcpIsAvrcpAudioSwitchingEnabled(void)
{
    bool audioswitching_enabled = FALSE;
    avrcp_readonly_config_def_t *read_config_data = NULL;
    
    if (configManagerGetReadOnlyConfig(AVRCP_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
        audioswitching_enabled = read_config_data->EnableAvrcpAudioSwitching;
        configManagerReleaseConfig(AVRCP_READONLY_CONFIG_BLK_ID);
    }
    AVRCP_DEBUG(("AVRCP: sinkAvrcpIsAvrcpAudioSwitchingEnabled(),Enabled= %d \n",audioswitching_enabled));

    return (audioswitching_enabled)? TRUE: FALSE;
}

#ifdef ENABLE_AVRCP_BROWSING
/*************************************************************************
NAME    
    sinkAvrcpGetBrowsingUIDCounter
    
DESCRIPTION
    Get AVRCP Browsing UID counter value

    index: Link Index on which browsing type is requested
    Return a uin16 value

**************************************************************************/
uint16 sinkAvrcpGetBrowsingUIDCounter(uint16 index)
{
    return AVRCP_DATA.uid_counter[index];    
}

/*************************************************************************
NAME    
    sinkAvrcpGetBrowsingMediaPlayerFeatures
    
DESCRIPTION
    Get AVRCP Browsing MediaPlayer Features value

    index: Link Index on which browsing type is requested
    Return a uin16 value

**************************************************************************/
uint16 sinkAvrcpGetBrowsingMediaPlayerFeatures(uint16 index)
{
    return AVRCP_DATA.media_player_features[index];    
}

/*************************************************************************
NAME    
    sinkAvrcpGetBrowsingMediaPlayerId
    
DESCRIPTION
    Get AVRCP Browsing MediaPlayer Id value.

    index: Link Index on which browsing type is requested
    Return a uin16 value

**************************************************************************/
uint16 sinkAvrcpGetBrowsingMediaPlayerId(uint16 index)
{
    return AVRCP_DATA.media_player_id[index];    
}

/*************************************************************************
NAME    
    sinkAvrcpGetBrowsingScope
    
DESCRIPTION
    Get AVRCP Browsing Scope value.

    index: Link Index on which browsing type is requested
    Return a uin16 value

**************************************************************************/
uint16 sinkAvrcpGetBrowsingScope(uint16 index)
{
    return AVRCP_DATA.browsing_scope[index];    
}

/*************************************************************************
NAME    
    sinkAvrcpSetBrowsingUIDCounter
    
DESCRIPTION
    Set AVRCP Browsing UID Counter value
    index: AVRCP link index
    info: Data need to be set

**************************************************************************/
void sinkAvrcpSetBrowsingUIDCounter(uint16 index,uint16 info)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpSetBrowsingUIDCounter(index=%d,info = %d) \n",index,info));

    AVRCP_DATA.uid_counter[index] = info;
}

/*************************************************************************
NAME    
    sinkAvrcpSetBrowsingMediaPlayerFeatures
    
DESCRIPTION
    Set AVRCP Browsing MediaPlayer Features value
    index: AVRCP link index
    info: Data need to be set

**************************************************************************/
void sinkAvrcpSetBrowsingMediaPlayerFeatures(uint16 index,uint16 info)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpSetBrowsingMediaPlayerFeatures(index=%d,info = %d) \n",index,info));

    AVRCP_DATA.media_player_features[index] = info;
}

/*************************************************************************
NAME    
    sinkAvrcpSetBrowsingMediaPlayerId
    
DESCRIPTION
    Set AVRCP Browsing MediaPlayer Id value
    index: AVRCP link index
    info: Data need to be set

**************************************************************************/
void sinkAvrcpSetBrowsingMediaPlayerId(uint16 index,uint16 info)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpSetBrowsingMediaPlayerId(index=%d,info = %d) \n",index,info));

    AVRCP_DATA.media_player_id[index] = info;
}

/*************************************************************************
NAME    
    sinkAvrcpSetBrowsingScope
    
DESCRIPTION
    Set AVRCP Browsing Scope value.
    index: AVRCP link index
    info: Data need to be set

**************************************************************************/
void sinkAvrcpSetBrowsingScope(uint16 index,uint16 info)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpSetBrowsingScope(index=%d,info = %d) \n",index,info));

    AVRCP_DATA.browsing_scope[index] = info;
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingScopeBusy
    
DESCRIPTION
    index: AVRCP link index

    Return address of browsing scope flag

**************************************************************************/
uint16 *sinkAvrcpBrowsingScopeBusy(uint16 index)
{
    return &AVRCP_DATA.browsing_scope[index];
}

/*************************************************************************
NAME    
    sinkAvrcpGetBrowsingHandler
    
DESCRIPTION
    index: AVRCP link index

    Return browsing handler

**************************************************************************/
TaskData *sinkAvrcpGetBrowsingHandler(uint16 index)
{
    return &AVRCP_DATA.avrcp_browsing_handler[index];
}

/*************************************************************************
NAME    
    sinkAvrcpUpdateBrowsingHandler
    
DESCRIPTION
    index: AVRCP link index for 
    *task: Pointer to Browsing Hanlder 

**************************************************************************/
void sinkAvrcpUpdateBrowsingHandler(uint16 index,TaskData *task)
{
    AVRCP_DATA.avrcp_browsing_handler[index].handler = task->handler;
}

#endif /* ENABLE_AVRCP_BROWSING */

/*************************************************************************
NAME    
    sinkAvrcpGetBrowsingChannel
    
DESCRIPTION
    index: AVRCP link index

    Return browsing channel as per index 

**************************************************************************/
uint16 sinkAvrcpGetBrowsingChannel(uint16 index)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpGetBrowsingChannel(index=%d): Channel=%d \n",index,AVRCP_DATA.browsing_channel[index]));

    return AVRCP_DATA.browsing_channel[index];
}

/*************************************************************************
NAME    
    sinkAvrcpSetBrowsingChannel
    
DESCRIPTION
    index: AVRCP link index
    value: Value need to be set for the channel

**************************************************************************/
void sinkAvrcpSetBrowsingChannel(uint16 index,uint16 value)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpSetBrowsingChannel(index=%d,Value = %d)\n",index,value));

    AVRCP_DATA.browsing_channel[index] = value;
}

/*************************************************************************
NAME    
    sinkAvrcpBrowsingChannelBusy
    
DESCRIPTION
    index: AVRCP link index
    Return address of the browsing channel

**************************************************************************/
uint16 *sinkAvrcpBrowsingChannelBusy(uint16 index)
{
    return &AVRCP_DATA.browsing_channel[index];
}

/*************************************************************************
NAME    
    sinkAvrcpGetExtensions
    
DESCRIPTION
    index: AVRCP link index
    Return requested link extensions

**************************************************************************/
uint16 sinkAvrcpGetExtensions(uint16 index)
{
    AVRCP_DEBUG(("AVRCP: sinkAvrcpGetExtensions(index=%d)Extension = %d\n",index, AVRCP_DATA.extensions[index]));

    return AVRCP_DATA.extensions[index];
}

/*************************************************************************
NAME    
    sinkAvrcpGetCleanUpTask
    
DESCRIPTION
    index: AVRCP link index
    Return avrcp cleau up task data as per index

**************************************************************************/
SinkAvrcpCleanUpTask *sinkAvrcpGetCleanUpTask(uint16 index)
{
    return &AVRCP_DATA.dataCleanUpTask[index];

}
/*************************************************************************
NAME    
    sinkAvrcpUpdateActiveConnection
    
DESCRIPTION
    Updates the active AVRCP connection based on what is currently connected.

**************************************************************************/
static void sinkAvrcpUpdateActiveConnection(void)
{
    uint8 i = 0;
    uint16 activeIndex = sinkAvrcpGetActiveConnection();
   
    if(AVRCP_DATA.connected[activeIndex] && 
        ((AVRCP_DATA.play_status[activeIndex] != sinkavrcp_play_status_stopped) &&
        (AVRCP_DATA.play_status[activeIndex] != sinkavrcp_play_status_paused)))
    {
        return;
    }

    /* Since the current AVRCP active index is not connected or playing, find a valid one */
    for_all_avrcp(i)
    {
        if(AVRCP_DATA.connected[i])
        {
            if(!peerAvrcpUpdateActiveConnection(i))
            {
                /* We have found a non-peer device, search no further */
                updateActiveConnection(i);
                break;
            }
        }
    }
    AVRCP_DEBUG(("AVRCP: sinkAvrcpUpdateActiveConnection, active_index = %d\n", sinkAvrcpGetActiveConnection()));
}

/*************************************************************************
NAME    
    sinkAvrcpGetNumberConnections
    
DESCRIPTION
    Retrieves the number of active AVRCP connections

**************************************************************************/
uint16 sinkAvrcpGetNumberConnections(void)
{
    uint8 i = 0;
    uint16 connected = 0;
    
    for_all_avrcp(i)
    {
        if (AVRCP_DATA.connected[i])
            connected++;
    }
    
    return connected;
}


/*************************************************************************
NAME    
    sinkAvrcpSetPlayStatusbyAddress
    
DESCRIPTION
    Sets the play status but only for AVRCP 1.0 devices. For AVRCP 1.3 onwards, the remote
    device will notify of changes in play status.

**************************************************************************/
void sinkAvrcpSetPlayStatusbyAddress(const bdaddr *bd_addr, avrcp_play_status play_status)
{
    uint16 Index;
    
    if (sinkAvrcpGetIndexFromBdaddr(bd_addr, &Index, TRUE))
    {
        if (!isAvrcpPlaybackStatusSupported(Index))
        {
            AVRCP_DEBUG(("AVRCP: update play status %d\n", play_status));
            setPlayStatus(Index, (SinkAvrcp_play_status)play_status);
        }
    }
}


/*************************************************************************
NAME    
    sinkAvrcpCheckManualConnectReset
    
DESCRIPTION
    Checks if the manual AVRCP connect state needs resetting.

**************************************************************************/
bool sinkAvrcpCheckManualConnectReset(const bdaddr *bd_addr)
{
    if (avrcpAvrcpIsEnabled() && 
        AVRCP_DATA.avrcp_manual_connect)
    {
        if (BdaddrIsZero(&AVRCP_DATA.avrcp_play_addr) || 
                ((bd_addr != NULL) && 
                 !BdaddrIsZero(&AVRCP_DATA.avrcp_play_addr) && 
                 BdaddrIsSame(&AVRCP_DATA.avrcp_play_addr, bd_addr))
        )
        {
            AVRCP_DATA.avrcp_manual_connect = FALSE;
            BdaddrSetZero(&AVRCP_DATA.avrcp_play_addr);
            return TRUE;
        }
    }
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpDisplayMediaAttributes
    
DESCRIPTION
    Displays the Media Attribute text (title, artist etc.) for the supplied attribute ID.
    The text is contained in data with length attribute_length.

**************************************************************************/
void sinkAvrcpDisplayMediaAttributes(uint32 attribute_id, uint16 attribute_length, const uint8 *data)
{        
    switch (attribute_id)
    {
        case AVRCP_MEDIA_ATTRIBUTE_TITLE:
        {
            AVRCP_DEBUG(("  Title: "));
            displayShowText((char*)data, attribute_length, DISPLAY_TEXT_SCROLL_SCROLL, 500, 2000, FALSE, 0, SINK_TEXT_TYPE_MEDIA_INFO);
            AVRCP_DATA.media_info_displayed = TRUE;
        }
            break;
        case AVRCP_MEDIA_ATTRIBUTE_ARTIST:
            AVRCP_DEBUG(("  Artist: "));
            break;
        case AVRCP_MEDIA_ATTRIBUTE_ALBUM:
            AVRCP_DEBUG(("  Album: "));
            break;
        case AVRCP_MEDIA_ATTRIBUTE_NUMBER:
            AVRCP_DEBUG(("  Number: "));
            break;
        case AVRCP_MEDIA_ATTRIBUTE_TOTAL_NUMBER:
            AVRCP_DEBUG(("  Total Number: "));
            break;
        case AVRCP_MEDIA_ATTRIBUTE_GENRE:
            AVRCP_DEBUG(("  Genre: "));
            break;
        case AVRCP_MEDIA_ATTRIBUTE_PLAYING_TIME:
            AVRCP_DEBUG(("  Playing time (ms): "));
            break;
        default:            
            break;
    }
  
#ifdef DEBUG_AVRCP  
    {
        uint16 i;
        for (i = 0; i < attribute_length; i++)
        {
            AVRCP_DEBUG(("%c", data[i]));        
        }        
        AVRCP_DEBUG(("\n")); 
    }
#else
    UNUSED(attribute_length);
    UNUSED(data);
#endif    
}


#ifdef ENABLE_AVRCP_NOW_PLAYING
/*************************************************************************
NAME    
    sinkAvrcpRetrieveNowPlayingRequest
    
DESCRIPTION
    Retrieves Now Playing Track from remote device.
    
        track_index_high - upper 32 bits of unique track UID
        track_index_high - lower 32 bits of unique track UID
        full_attributes - TRUE if all media attributes should be returned, FALSE if only the basic media attributes should be returned

**************************************************************************/
void sinkAvrcpRetrieveNowPlayingRequest(uint32 track_index_high, uint32 track_index_low, bool full_attributes)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    AVRCP_DEBUG(("AVRCP: Avrcp Retrieve Now Playing\n"));
  
    if (AVRCP_DATA.connected[Index])
    {
        if (sinkAvrcpBrowsingIsSupported(Index) && ((track_index_high != 0x0) || (track_index_low != 0x0)))
        {
            /* open browsing channel and use Browsing:GetItemAttributes */
            sinkAvrcpBrowsingRetrieveNowPlayingTrackRequest(Index, track_index_high, track_index_low, full_attributes);
        }
        else
        {
            /* use GetElementAttributes command */
            sinkAvrcpRetrieveNowPlayingNoBrowsingRequest(Index, full_attributes);
        }
    }
}


/*************************************************************************
NAME    
    sinkAvrcpRetrieveNowPlayingNoBrowsingRequest
    
DESCRIPTION
    Retrieves Now Playing track from remote device without using browsing channel.
    
        Index - AVRCP connection instance, usually the active one
        full_attributes - TRUE if all media attributes should be returned, FALSE if only the basic media attributes should be returned

**************************************************************************/
void sinkAvrcpRetrieveNowPlayingNoBrowsingRequest(uint16 Index, bool full_attributes)
{
    uint16 size_media_attributes;
    Source src_media_attributes;    
    
    if (full_attributes)
    {
        size_media_attributes = sizeof(avrcp_retrieve_media_attributes_full);
        src_media_attributes = StreamRegionSource(avrcp_retrieve_media_attributes_full, size_media_attributes);    
    }
    else
    {
        size_media_attributes = sizeof(avrcp_retrieve_media_attributes_basic);
        src_media_attributes = StreamRegionSource(avrcp_retrieve_media_attributes_basic, size_media_attributes);    
    }
    
    AvrcpGetElementAttributesRequest(AVRCP_DATA.avrcp[Index],
                                     0, 
                                     0, 
                                     size_media_attributes, 
                                     src_media_attributes);
    
    AVRCP_DEBUG(("AVRCP: Avrcp Retrieve Now Playing no Browsing, size:%d\n", size_media_attributes));
}

#endif /* ENABLE_AVRCP_NOW_PLAYING */


#ifdef ENABLE_AVRCP_PLAYER_APP_SETTINGS
/*************************************************************************
NAME    
    sinkAvrcpListPlayerAttributesRequest
    
DESCRIPTION
    Retrieves the list of Player Attributes from the remote device.

**************************************************************************/
bool sinkAvrcpListPlayerAttributesRequest(void)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    if (isAvrcpPlayerApplicationSettingsSupported(Index))
    {    
        AVRCP_DEBUG(("AVRCP: Avrcp List Player Attributes\n"));
        
        /* register to get notifications of changes in application settings */
        AvrcpRegisterNotificationRequest(AVRCP_DATA.avrcp[Index], avrcp_event_player_app_setting_changed, 0);   
    
        /* retrieve the attributes */
        AvrcpListAppAttributeRequest(AVRCP_DATA.avrcp[Index]);
        
        return TRUE;
    }
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpListPlayerAttributesTextRequest
    
DESCRIPTION
    Retrieves the list of Player Attributes Text from the remote device.
    
        size_attributes - number of IDs contained in attributes
        attributes - the IDs to return the associated text for

**************************************************************************/
bool sinkAvrcpListPlayerAttributesTextRequest(uint16 size_attributes, Source attributes)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    if (isAvrcpPlayerApplicationSettingsSupported(Index))
    {    
        AVRCP_DEBUG(("AVRCP: Avrcp List Player Attributes Text\n"));
 
        AvrcpGetAppAttributeTextRequest(AVRCP_DATA.avrcp[Index], size_attributes, attributes);    
                
        return TRUE;       
    }
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpListPlayerValuesRequest
    
DESCRIPTION
    Retrieves the list of Player Values for the specified Attribute from the remote device.
    
        attribute_id - return all the possible values for this attribute ID 

**************************************************************************/
bool sinkAvrcpListPlayerValuesRequest(uint8 attribute_id)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    if (isAvrcpPlayerApplicationSettingsSupported(Index))
    {    
        AVRCP_DEBUG(("AVRCP: Avrcp List Player Values\n"));
                
        AvrcpListAppValueRequest(AVRCP_DATA.avrcp[Index], attribute_id);
        
        return TRUE;
    }
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpGetPlayerValueRequest
    
DESCRIPTION
    Retrieves the current Player Value for the specified Attributes from the remote device.
    
        size_attributes - number of attribute IDs contained in attributes
        attributes - the IDs to get the associated value for

**************************************************************************/
bool sinkAvrcpGetPlayerValueRequest(uint16 size_attributes, Source attributes)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    if (isAvrcpPlayerApplicationSettingsSupported(Index))
    {    
        AVRCP_DEBUG(("AVRCP: Avrcp Get Player Value\n"));
        
        AvrcpGetAppValueRequest(AVRCP_DATA.avrcp[Index], size_attributes, attributes);
        
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
NAME    
    sinkAvrcpListPlayerValueTextRequest
    
DESCRIPTION
    Retrieves the current Player Value Text for the specified Attributes from the remote device.
    
        attribute_id - the attribute ID for which to retrieve the value text for
        size_values - number of value IDs contained in values
        values - the value IDs to get the associated text for

**************************************************************************/
bool sinkAvrcpListPlayerValueTextRequest(uint16 attribute_id, uint16 size_values, Source values)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    if (isAvrcpPlayerApplicationSettingsSupported(Index))
    {    
        AVRCP_DEBUG(("AVRCP: Avrcp Get Player Value Text\n"));
        
        AvrcpGetAppValueTextRequest(AVRCP_DATA.avrcp[Index], attribute_id, size_values, values);
        
        return TRUE;
    }
    return FALSE;
}


/*************************************************************************
NAME    
    sinkAvrcpSetPlayerValueRequest
    
DESCRIPTION
    Sets the current Player Value for the specified Attributes on the remote device.
    
        size_attributes - number of {attribute, value} pairs contained in attributes
        attributes - the {attribute, value} pairs to set the value for

**************************************************************************/
bool sinkAvrcpSetPlayerValueRequest(uint16 size_attributes, Source attributes)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    if (isAvrcpPlayerApplicationSettingsSupported(Index))
    {    
        AVRCP_DEBUG(("AVRCP: Avrcp Set Player Value\n"));
        
        AvrcpSetAppValueRequest(AVRCP_DATA.avrcp[Index], size_attributes, attributes);
        
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
NAME    
    sinkAvrcpInformBatteryStatusRequest
    
DESCRIPTION
    Informs the TG of the battery status
    
        status - current status of battery

**************************************************************************/
bool sinkAvrcpInformBatteryStatusRequest(avrcp_battery_status status)
{
    uint16 Index = sinkAvrcpGetActiveConnection();
    
    if (isAvrcpPlayerApplicationSettingsSupported(Index))
    {    
        AVRCP_DEBUG(("AVRCP: Avrcp Inform Battery Status\n"));
        
        AvrcpInformBatteryStatusRequest(AVRCP_DATA.avrcp[Index], status);
        
        return TRUE;
    }
    return FALSE;
}
#endif /* ENABLE_AVRCP_PLAYER_APP_SETTINGS */





/*************************************************************************
NAME    
    sinkAvrcpPauseRequest
    
DESCRIPTION
    attemtps to pause the media stream of the passed in avrcp connection index
    
RETURNS
    success or failure status

**************************************************************************/
bool sinkAvrcpPlayPauseRequest(uint16 Index, avrcp_remote_actions action)
{
    bool status = FALSE;
    
    AVRCP_DEBUG(("AVRCP: Avrcp Play or Pause to %x\n",Index));

    /* ensure avrcp is connected */
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 2))
    {
        /* pause requested ? */
        if(action == AVRCP_PAUSE)
        {
            AVRCP_DEBUG(("AVRCP: Avrcp Pause to %x\n",Index));
            avrcpSendControlMessage(AVRCP_CTRL_PAUSE_PRESS, Index);
            avrcpSendControlMessage(AVRCP_CTRL_PAUSE_RELEASE, Index);
            if (!isAvrcpPlaybackStatusSupported(Index))
                setPlayStatus(Index, sinkavrcp_play_status_paused);
            /* paused successfully */        
            status = TRUE;
        }
        /* play requested ? */
        else if(action == AVRCP_PLAY)
        {          
            AVRCP_DEBUG(("AVRCP: Avrcp Play to %x\n",Index));
            avrcpSendControlMessage(AVRCP_CTRL_PLAY_PRESS, Index);
            avrcpSendControlMessage(AVRCP_CTRL_PLAY_RELEASE, Index);
            if (!isAvrcpPlaybackStatusSupported(Index))
                setPlayStatus(Index, sinkavrcp_play_status_playing);
            /* resume play successfully */        
            status = TRUE;
        }
    }
    return status;
}


/*************************************************************************
NAME    
    sinkAvrcpStopRequest
    
DESCRIPTION
    attemtps to stop the media stream of the passed in avrcp connection index
    
RETURNS
    success or failure status

**************************************************************************/
bool sinkAvrcpStopRequest(uint16 Index)
{
    bool status = FALSE;

    /* ensure avrcp is connected */
    if (AVRCP_DATA.connected[Index] && (getAvrcpQueueSpace(Index) >= 2))
    {
        AVRCP_DEBUG(("AVRCP: Avrcp Stop to %x\n",Index));
        avrcpSendControlMessage(AVRCP_CTRL_STOP_PRESS, Index);
        avrcpSendControlMessage(AVRCP_CTRL_STOP_RELEASE, Index);
        
        if (!isAvrcpPlaybackStatusSupported(Index))
        {
            setPlayStatus(Index, sinkavrcp_play_status_stopped);
        }

        /* stopped successfully */        
        status = TRUE;
    }
    return status;
}


/*************************************************************************
NAME    
    sinkAvrcpVendorUniquePassthroughRequest
    
DESCRIPTION
    attemtps to pause the media stream of the passed in avrcp connection index
    
RETURNS
    FALSE if command could not be sent, TRUE otherwise

**************************************************************************/
bool sinkAvrcpVendorUniquePassthroughRequest (uint16 avrcp_index, uint16 cmd_id, uint16 size_data, const uint8 *data)
{
#ifdef ENABLE_PEER
    uint16 a2dp_index;
    
    if ( !a2dpGetPeerIndex(&a2dp_index) || !(getA2dpPeerRemoteFeatures(a2dp_index) & remote_features_peer_avrcp_target) )
    {   /* The Peer does not accept custom commands */
        AVRCP_DEBUG(("AVRCP: No peer target\n"));
        return FALSE;
    }
#endif
    
    if ( !avrcpAvrcpIsEnabled() || !(sinkAvrcpIsInitialised() && AVRCP_DATA.connected[avrcp_index]) )
    {   /* We don't support AVRCP */
        AVRCP_DEBUG(("AVRCP: No AVRCP\n"));
        return FALSE;
    }

    AVRCP_DEBUG(("AVRCP: sinkAvrcpVendorUniquePassthroughRequest avrcp_index=%u  cmd_id=%u  size_data=%u  data=%p\n",avrcp_index,cmd_id,size_data,(void*)data));
    {
        MAKE_AVRCP_MESSAGE_WITH_LEN( SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ, size_data);
        message->avrcp_index = avrcp_index;
        message->cmd_id = cmd_id;
        message->size_data = size_data;
        memcpy(message->data, data, size_data);

        if ( !AVRCP_DATA.vendor_data[avrcp_index] )
        {
            AVRCP_DEBUG(("AVRCP:    Issuing...\n"));
            MessageSendConditionally(&AVRCP_DATA.avrcp_ctrl_handler[avrcp_index], AVRCP_VENDOR_UNIQUE_ID, message, (uint16 *)&AVRCP_DATA.pending_cmd[avrcp_index]);
        }
        else
        {
            AVRCP_DEBUG(("AVRCP:    Requeueing...\n"));
            MessageSendConditionally( &theSink.task, SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ, message, (uint16 *)&AVRCP_DATA.pending_cmd[avrcp_index]);
        }
    }
    return TRUE;
}


/*************************************************************************
NAME    
    sinkAvrcpHandleMessage
    
DESCRIPTION
    Handles AVRCP library messages.

**************************************************************************/
void sinkAvrcpHandleMessage(Task task, MessageId id, Message message)
{
    AVRCP_DEBUG(("AVRCP_MSG id=%x : \n", id));
    AVRCP_DEBUG(("Allocations=%d : \n", VmGetAvailableAllocations()));
    
    switch (id)
    {
     
/******************/
/* INITIALISATION */
/******************/
        
        case AVRCP_INIT_CFM:
            AVRCP_DEBUG(("AVRCP_INIT_CFM :\n"));
            sinkAvrcpInitComplete((const AVRCP_INIT_CFM_T *) message);
            break;

/******************************/
/* CONNECTION / DISCONNECTION */
/******************************/            
            
        case AVRCP_CONNECT_CFM:
            AVRCP_DEBUG(("AVRCP_CONNECT_CFM :\n"));
            handleAvrcpConnectCfm((const AVRCP_CONNECT_CFM_T *) message);
            break;
        case AVRCP_CONNECT_IND:
            AVRCP_DEBUG(("AVRCP_CONNECT_IND :\n"));
            handleAvrcpConnectInd((const AVRCP_CONNECT_IND_T *) message);
            break;
        case AVRCP_DISCONNECT_IND:
            AVRCP_DEBUG(("AVRCP_DISCONNECT_IND :\n"));
            handleAvrcpDisconnectInd((const AVRCP_DISCONNECT_IND_T *) message);
            break;
            
/*****************/
/* AV/C COMMANDS */
/*****************/            
        
        case AVRCP_PASSTHROUGH_CFM:
            AVRCP_DEBUG(("AVRCP_PASSTHROUGH_CFM :\n"));
            if (((const AVRCP_PASSTHROUGH_CFM_T *) message)->status == avrcp_device_not_connected)
            {
                if (AVRCP_DATA.last_source != NULL)
                {
                    AVRCP_DEBUG(("Consume stale passthrough request: %04x\n", AVRCP_DATA.last_source));
                    SourceEmpty(AVRCP_DATA.last_source);
                    AVRCP_DATA.last_source = NULL;
                }
            }
            handleAvrcpPassthroughCfm(((const AVRCP_PASSTHROUGH_CFM_T *) message)->avrcp);
            break;    
        case AVRCP_PASSTHROUGH_IND:
            AVRCP_DEBUG(("AVRCP_PASSTHROUGH_IND :\n"));
            handleAvrcpPassthroughInd((AVRCP_PASSTHROUGH_IND_T *) message);
            break;
        case AVRCP_UNITINFO_IND:
            AVRCP_DEBUG(("AVRCP_UNITINFO_IND :\n"));
            handleAvrcpUnitInfoInd((AVRCP_UNITINFO_IND_T *) message);
            break;
        case AVRCP_SUBUNITINFO_IND:
            AVRCP_DEBUG(("AVRCP_SUBUNITINFO_IND :\n"));
            handleAvrcpSubUnitInfoInd((AVRCP_SUBUNITINFO_IND_T *) message);
            break;
        case AVRCP_VENDORDEPENDENT_IND:
            AVRCP_DEBUG(("AVRCP_VENDORDEPENDENT_IND :\n"));
            handleAvrcpVendorDependentInd((AVRCP_VENDORDEPENDENT_IND_T *) message);
            break;
        case AVRCP_NEXT_GROUP_CFM:
            AVRCP_DEBUG(("AVRCP_NEXT_GROUP_CFM :\n"));
            handleAvrcpPassthroughCfm(((const AVRCP_NEXT_GROUP_CFM_T *) message)->avrcp); /* treat as general Passthrough cmd */
            break;    
        case AVRCP_PREVIOUS_GROUP_CFM:
            AVRCP_DEBUG(("AVRCP_PREVIOUS_GROUP_CFM :\n"));
            handleAvrcpPassthroughCfm(((const AVRCP_PREVIOUS_GROUP_CFM_T *) message)->avrcp); /* treat as general Passthrough cmd */
            break; 
            
/******************/
/* AVRCP Metadata */
/******************/            
            
        case AVRCP_GET_CAPS_IND:
            AVRCP_DEBUG(("AVRCP_GET_CAPS_IND :\n"));
            handleAvrcpGetCapsInd((const AVRCP_GET_CAPS_IND_T *) message);
            break;
        case AVRCP_GET_CAPS_CFM:
            AVRCP_DEBUG(("AVRCP_GET_CAPS_CFM :\n"));
            handleAvrcpGetCapsCfm((const AVRCP_GET_CAPS_CFM_T *) message);
            break;
        case AVRCP_REGISTER_NOTIFICATION_IND:
            AVRCP_DEBUG(("AVRCP_REGISTER_NOTIFICATION_IND :\n"));
            handleAvrcpRegisterNotificationInd((const AVRCP_REGISTER_NOTIFICATION_IND_T *) message);
            break;
        case AVRCP_REGISTER_NOTIFICATION_CFM:
            AVRCP_DEBUG(("AVRCP_REGISTER_NOTIFICATION_CFM :\n"));            
            break;
        case AVRCP_SET_ABSOLUTE_VOLUME_IND:
            AVRCP_DEBUG(("AVRCP_SET_ABSOLUTE_VOLUME_IND :\n"));
            handleAvrcpSetAbsVolInd((const AVRCP_SET_ABSOLUTE_VOLUME_IND_T *) message);
            break;
        case AVRCP_GET_PLAY_STATUS_CFM:
            AVRCP_DEBUG(("AVRCP_GET_PLAY_STATUS_CFM :\n"));
            handleAvrcpGetPlayStatusCfm((const AVRCP_GET_PLAY_STATUS_CFM_T *) message);
            break;
        case AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND :\n"));
            handleAvrcpPlayStatusChangedInd((const AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND_T *) message);
            break;                    
        case AVRCP_EVENT_TRACK_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_TRACK_CHANGED_IND :\n"));
            handleAvrcpTrackChangedInd((const AVRCP_EVENT_TRACK_CHANGED_IND_T *) message);
            break;
        case AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND :\n"));
            handleAvrcpPlaybackPosChangedInd((const AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND_T *) message);
            break;
        case AVRCP_EVENT_VOLUME_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_VOLUME_CHANGED_IND :\n"));
            handleAvrcpVolumeChangedInd((const AVRCP_EVENT_VOLUME_CHANGED_IND_T *) message);            
            break;
        case AVRCP_SET_ABSOLUTE_VOLUME_CFM:
            AVRCP_DEBUG(("AVRCP_SET_ABSOLUTE_VOLUME_CFM :\n"));
            handleAvrcpSetAbsoluteVolumeCfm((const AVRCP_SET_ABSOLUTE_VOLUME_CFM_T *) message);
            break;
        case AVRCP_EVENT_TRACK_REACHED_END_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_TRACK_REACHED_END_IND :\n")); /* notification not currently used */
            break;
        case AVRCP_EVENT_TRACK_REACHED_START_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_TRACK_REACHED_START_IND :\n")); /* notification not currently used */
            break;
        case AVRCP_EVENT_BATT_STATUS_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_BATT_STATUS_CHANGED_IND :\n")); /* notification not currently used */
            break;
        case AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND :\n")); /* notification not currently used */
            break;        
        case AVRCP_REQUEST_CONTINUING_RESPONSE_CFM:
            AVRCP_DEBUG(("AVRCP_REQUEST_CONTINUING_RESPONSE_CFM :\n"));
            break;    
        case AVRCP_ABORT_CONTINUING_RESPONSE_CFM:
            AVRCP_DEBUG(("AVRCP_ABORT_CONTINUING_RESPONSE_CFM :\n"));
            break;    
#ifdef ENABLE_AVRCP_NOW_PLAYING            
        case AVRCP_GET_ELEMENT_ATTRIBUTES_CFM:
            AVRCP_DEBUG(("AVRCP_GET_ELEMENT_ATTRIBUTES_CFM :\n"));
            handleAvrcpGetElementAttributesCfm((const AVRCP_GET_ELEMENT_ATTRIBUTES_CFM_T *) message);
            break;
        case AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED_IND :\n"));
            handleAvrcpNowPlayingContentChangedInd((const AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED_IND_T *) message);
            break;
        case AVRCP_ADD_TO_NOW_PLAYING_CFM:
            AVRCP_DEBUG(("AVRCP_ADD_TO_NOW_PLAYING_CFM %d :\n", ((const AVRCP_ADD_TO_NOW_PLAYING_CFM_T *)message)->status));            
            break;
#endif /* ENABLE_AVRCP_NOW_PLAYING */            
#ifdef ENABLE_AVRCP_PLAYER_APP_SETTINGS            
        case AVRCP_LIST_APP_ATTRIBUTE_CFM:
            AVRCP_DEBUG(("AVRCP_LIST_APP_ATTRIBUTE_CFM :\n"));
            handleAvrcpListAppAttributeCfm((const AVRCP_LIST_APP_ATTRIBUTE_CFM_T *) message);
            break;
        case AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM:
            AVRCP_DEBUG(("AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM :\n"));
            handleAvrcpGetAppAttributeValueTextCfm(AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID, (const AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM_T *) message);
            break;
        case AVRCP_LIST_APP_VALUE_CFM:
            AVRCP_DEBUG(("AVRCP_LIST_APP_VALUE_CFM :\n"));
            handleAvrcpListAppValueCfm((const AVRCP_LIST_APP_VALUE_CFM_T *) message);
            break;
        case AVRCP_GET_APP_VALUE_CFM:
            AVRCP_DEBUG(("AVRCP_GET_APP_VALUE_CFM :\n"));
            handleAvrcpGetAppValueCfm((const AVRCP_GET_APP_VALUE_CFM_T *) message);
            break;
        case AVRCP_GET_APP_VALUE_TEXT_CFM:
            AVRCP_DEBUG(("AVRCP_GET_APP_VALUE_TEXT_CFM :\n"));
            handleAvrcpGetAppAttributeValueTextCfm(AVRCP_GET_APP_VALUE_TEXT_PDU_ID, (const AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM_T *) message); /* AVRCP_GET_APP_VALUE_TEXT_CFM_T is same structure as AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM_T */
            break;
        case AVRCP_SET_APP_VALUE_CFM:
            AVRCP_DEBUG(("AVRCP_SET_APP_VALUE_CFM :\n"));
            handleAvrcpSetAppValueCfm((const AVRCP_SET_APP_VALUE_CFM_T *) message);
            break;
        case AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND :\n"));
            sinkAvrcpPlayerAppSettingsChangedInd((const AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND_T *)message);            
            break;                           
#endif /* ENABLE_AVRCP_PLAYER_APP_SETTINGS */            

/************/
/* BROWSING */
/************/
        case AVRCP_BROWSE_CONNECT_IND:
            AVRCP_DEBUG(("AVRCP_BROWSE_CONNECT_IND :\n"));
            sinkAvrcpBrowsingChannelConnectInd((const AVRCP_BROWSE_CONNECT_IND_T *) message);
            break;
        case AVRCP_BROWSE_DISCONNECT_IND:
            AVRCP_DEBUG(("AVRCP_BROWSE_DISCONNECT_IND :\n"));
            sinkAvrcpBrowsingChannelDisconnectInd((const AVRCP_BROWSE_DISCONNECT_IND_T *) message);
            break;
        case AVRCP_BROWSE_CONNECT_CFM:
            AVRCP_DEBUG(("AVRCP_BROWSE_CONNECT_CFM :\n"));
            sinkAvrcpBrowsingChannelConnectCfm((const AVRCP_BROWSE_CONNECT_CFM_T *) message);
            break;
        case AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM:
            AVRCP_DEBUG(("AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM :\n"));
            sinkAvrcpBrowsingGetItemAttributesCfm((const AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_T *) message);
            break;
        case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND :\n"));
            sinkAvrcpBrowsingAddressedPlayerChangedInd((const AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND_T *) message);
            break;
        case AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM:
            AVRCP_DEBUG(("AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM :\n"));            
            sinkAvrcpBrowsingGetFolderItemsCfm((const AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_T *) message);
            break;

        case AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM:
            AVRCP_DEBUG(("AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM :\n"));
            sinkAvrcpBrowsingGetNumberOfItemsCfm((const AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM_T *) message);
            break;
        case AVRCP_SET_ADDRESSED_PLAYER_CFM:
            AVRCP_DEBUG(("AVRCP_SET_ADDRESSED_PLAYER_CFM %d : Opening of Player started \n", ((const AVRCP_SET_ADDRESSED_PLAYER_CFM_T *)message)->status));
            break;
        case AVRCP_BROWSE_SET_PLAYER_CFM:
            AVRCP_DEBUG(("AVRCP_BROWSE_SET_PLAYER_CFM :\n"));
            sinkAvrcpBrowsingSetBrowsedPlayerCfm((const AVRCP_BROWSE_SET_PLAYER_CFM_T *) message);
            break;
        case AVRCP_PLAY_ITEM_CFM:
            AVRCP_DEBUG(("AVRCP_PLAY_ITEM_CFM %d :\n", ((const AVRCP_PLAY_ITEM_CFM_T *)message)->status));
            break;
        case AVRCP_BROWSE_CHANGE_PATH_CFM:
            AVRCP_DEBUG(("AVRCP_BROWSE_CHANGE_PATH_CFM %d :\n", ((AVRCP_BROWSE_CHANGE_PATH_CFM_T *)message)->status));
            sinkAvrcpBrowsingChangePathCfm((const AVRCP_BROWSE_CHANGE_PATH_CFM_T *) message);
            break;
        case AVRCP_EVENT_UIDS_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_UIDS_CHANGED_IND :\n"));
            sinkAvrcpBrowsingUIDsChangedInd((AVRCP_EVENT_UIDS_CHANGED_IND_T *)message);
            break;
        case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND:
            AVRCP_DEBUG(("AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND :\n"));
            sinkAvrcpBrowsingAvailablePlayersChangedInd((const AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND_T *)message);
            break;
        case AVRCP_BROWSE_SEARCH_CFM:
            AVRCP_DEBUG(("AVRCP_BROWSE_SEARCH_CFM :\n"));
            sinkAvrcpBrowsingSearchCfm((const AVRCP_BROWSE_SEARCH_CFM_T *)message);
            break;
            
/********************/
/* LIBRARY SPECIFIC */
/********************/ 
            
        case AVRCP_GET_EXTENSIONS_CFM:
            AVRCP_DEBUG(("AVRCP_GET_EXTENSIONS_CFM :\n"));
            handleAvrcpGetExtensionsCfm((const AVRCP_GET_EXTENSIONS_CFM_T *) message);
            break;
        case AVRCP_GET_SUPPORTED_FEATURES_CFM:
            AVRCP_DEBUG(("AVRCP_GET_SUPPORTED_FEATURES_CFM :\n"));
            handleAvrcpGetSupportedFeaturesCfm((const AVRCP_GET_SUPPORTED_FEATURES_CFM_T *) message);
            break;
            
/************************/
/* APPLICATION SPECIFIC */
/************************/ 

        case SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ:
            AVRCP_DEBUG(("SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ :\n"));
            handleSinkAvrcpVendorUniquePassthroughReq((const SINK_AVRCP_VENDOR_UNIQUE_PASSTHROUGH_REQ_T *) message);
            break;

            
/************************/
/* AVRCP TG COMMANDS TO BE HANDLED IN PTS*/
/************************/ 
        /* Have all the fall through cases for PTS here */
        case AVRCP_BROWSE_GET_FOLDER_ITEMS_IND:
        case AVRCP_SET_ADDRESSED_PLAYER_IND:
        case AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_IND:
        case AVRCP_GET_PLAY_STATUS_IND:
        case AVRCP_GET_ELEMENT_ATTRIBUTES_IND:
            handleAvrcpQualificationTestCaseInd(task, id, message);
            break;

/*************/
/* UNHANDLED */
/*************/  
    
        default:
            break;
    }
}

/*************************************************************************
NAME    
    avrcpAvrcpIsEnabled

    DESCRIPTION
    Interface to check AVRCP is enabled or not
    
    Return: TRUE if avrcp is enabled, FALSE if AVRCP is disabled or config is not able to retrieve

**************************************************************************/
bool avrcpAvrcpIsEnabled(void)
{
    bool avrcp_enabled = FALSE;
    avrcp_readonly_config_def_t *read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(AVRCP_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
        avrcp_enabled = read_config_data->avrcp_enabled;
        configManagerReleaseConfig(AVRCP_READONLY_CONFIG_BLK_ID);
    }

    AVRCP_DEBUG(("AVRCP: avrcpAvrcpIsEnabled() Enabled = %d\n", avrcp_enabled));

    return (avrcp_enabled)? TRUE : FALSE;
}

/****************************************************************************
NAME    
    sinkAvrcpCheckStateMatch
    
DESCRIPTION
    helper function to check an avrcp play status for a given a2dp link priority
    
RETURNS
    true is the avrcp play state matches that passed in
    false if state does not match or no matching avrcp link is found for a2dp profile index
*/
bool sinkAvrcpCheckStateMatch(const bdaddr *bd_addr, avrcp_play_status play_status)
{
    uint16 i;
    bool status = FALSE;
    /* does the device support AVRCP and is AVRCP currently connected to this device? */
    for_all_avrcp(i)
    {    
        /* ensure media is streaming and the avrcp channel is that requested to be paused */
        if (sinkAvrcpIsInitialised())
        {
            if(getA2dpStatusFlag(CONNECTED, i) && (BdaddrIsSame(bd_addr, &AVRCP_DATA.bd_addr[i])) &&
               (sinkAvrcpGetPlayStatus(i) == play_status))
            {
                status = TRUE;
                break;
            }
        }
    }
    return status;
}

static void rescheduleForwardRewindRepeatEvent(MessageId forward_rewind_event)
{
    MessageSendLater( &theSink.task , forward_rewind_event , 0 , AVRCP_FF_REW_REPEAT_INTERVAL);
}

/****************************************************************************
NAME
    sinkAvrcpProcessEventAvrcp

DESCRIPTION
    Event handler for AVRCP related usr/sys events

RETURNS
    TRUE if the event was handled, otherwise FALSE
*/
bool sinkAvrcpProcessEventAvrcp(MessageId avrcp_event, Message message)
{
    bool indicate_event = TRUE;

    switch(avrcp_event)
    {
        case EventUsrAvrcpPlayPause:
            AVRCP_DEBUG(("HS : EventUsrAvrcpPlayPause\n")) ;
            MessageCancelAll (&theSink.task, EventUsrAvrcpFastForwardPress);
            MessageCancelAll (&theSink.task, EventUsrAvrcpRewindPress);
            sinkAvrcpPlayPause();
            break;

        case EventUsrAvrcpPlay:
            AVRCP_DEBUG(("HS : EventUsrAvrcpPlay\n")) ;
            MessageCancelAll (&theSink.task, EventUsrAvrcpFastForwardPress);
            MessageCancelAll (&theSink.task, EventUsrAvrcpRewindPress);
            sinkAvrcpPlay();
            break;

        case EventUsrAvrcpPause:
            AVRCP_DEBUG(("HS : EventUsrAvrcpPause\n")) ;
            MessageCancelAll (&theSink.task, EventUsrAvrcpFastForwardPress);
            MessageCancelAll (&theSink.task, EventUsrAvrcpRewindPress);
            sinkAvrcpPause();
            break;

        case EventUsrAvrcpStop:
            AVRCP_DEBUG(("HS : EventUsrAvrcpStop\n")) ;
            MessageCancelAll (&theSink.task, EventUsrAvrcpFastForwardPress);
            MessageCancelAll (&theSink.task, EventUsrAvrcpRewindPress);
            sinkAvrcpStop();
            break;

        case EventUsrAvrcpSkipForward:
            AVRCP_DEBUG(("HS : EventUsrAvrcpSkipForward\n")) ;
            sinkAvrcpSkipForward();
            break;

        case EventUsrAvrcpSkipBackward:
            AVRCP_DEBUG(("HS : EventUsrAvrcpSkipBackward\n")) ;
            sinkAvrcpSkipBackward();
            break;

        case EventUsrAvrcpFastForwardPress:
            AVRCP_DEBUG(("HS : EventUsrAvrcpFastForwardPress\n")) ;
            sinkAvrcpFastForwardPress();
            rescheduleForwardRewindRepeatEvent(EventUsrAvrcpFastForwardPress);
            break;

        case EventUsrAvrcpFastForwardRelease:
            AVRCP_DEBUG(("HS : EventUsrAvrcpFastForwardRelease\n")) ;
            MessageCancelAll (&theSink.task, EventUsrAvrcpFastForwardPress);
            sinkAvrcpFastForwardRelease();
            break;

        case EventUsrAvrcpRewindPress:
            AVRCP_DEBUG(("HS : EventUsrAvrcpRewindPress\n")) ;
            rescheduleForwardRewindRepeatEvent(EventUsrAvrcpRewindPress);
            sinkAvrcpRewindPress();
            break;

        case EventUsrAvrcpRewindRelease:
            AVRCP_DEBUG(("HS : EventUsrAvrcpRewindRelease\n")) ;
            MessageCancelAll (&theSink.task, EventUsrAvrcpRewindPress);
            sinkAvrcpRewindRelease();
            break;

        case EventUsrAvrcpToggleActive:
            AVRCP_DEBUG(("HS : EventUsrAvrcpToggleActive\n"));
            if (sinkAvrcpGetNumberConnections() > 1)
            {
                sinkAvrcpToggleActiveConnection();
            }
            else
            {
                indicate_event = FALSE;
            }
            break;

        case EventUsrAvrcpNextGroupPress:
            AVRCP_DEBUG(("HS : EventUsrAvrcpNextGroupPress\n"));
            sinkAvrcpNextGroupPress();
            break;

        case EventUsrAvrcpNextGroupRelease:
            AVRCP_DEBUG(("HS : EventUsrAvrcpNextGroupRelease\n"));
            sinkAvrcpNextGroupRelease();
            break;

        case EventUsrAvrcpPreviousGroupPress:
            AVRCP_DEBUG(("HS : EventUsrAvrcpPreviousGroupPress\n"));
            sinkAvrcpPreviousGroupPress();
            break;

        case EventUsrAvrcpPreviousGroupRelease:
            AVRCP_DEBUG(("HS : EventUsrAvrcpPreviousGroupRelease\n"));
            sinkAvrcpPreviousGroupRelease();
            break;

        case EventUsrAvrcpShuffleOff:
            AVRCP_DEBUG(("HS : EventUsrAvrcpShuffleOff\n"));
            sinkAvrcpShuffle(AVRCP_SHUFFLE_OFF);
            break;

        case EventUsrAvrcpShuffleAllTrack:
            AVRCP_DEBUG(("HS : EventUsrAvrcpShuffleAllTrack\n"));
            sinkAvrcpShuffle(AVRCP_SHUFFLE_ALL_TRACK);
            break;

        case EventUsrAvrcpShuffleGroup:
            AVRCP_DEBUG(("HS : EventUsrAvrcpShuffleGroup\n"));
            sinkAvrcpShuffle(AVRCP_SHUFFLE_GROUP);
            break;

        case EventUsrAvrcpRepeatOff:
            AVRCP_DEBUG(("HS : EventUsrAvrcpRepeatOff\n"));
            sinkAvrcpRepeat(AVRCP_REPEAT_OFF);
            break;

        case EventUsrAvrcpRepeatSingleTrack:
            AVRCP_DEBUG(("HS : EventUsrAvrcpRepeatSingleTrack\n"));
            sinkAvrcpRepeat(AVRCP_REPEAT_SINGLE_TRACK);
            break;

        case EventUsrAvrcpRepeatAllTrack:
            AVRCP_DEBUG(("HS : EventUsrAvrcpRepeatAllTrack\n"));
            sinkAvrcpRepeat(AVRCP_REPEAT_ALL_TRACK);
            break;

        case EventUsrAvrcpRepeatGroup:
            AVRCP_DEBUG(("HS : EventUsrAvrcpRepeatGroup\n"));
            sinkAvrcpRepeat(AVRCP_REPEAT_GROUP);
            break;

        case EventSysSetActiveAvrcpConnection:
            sinkAvrcpSetActiveConnectionFromBdaddr(&((const UpdateAvrpcMessage_t *)message)->bd_addr);
            break;

        case EventSysResetAvrcpMode:
            {
                uint16 index = *(const uint16 *) message;
                sinkAvrcpUpdateLinkActiveStatus(index,FALSE);
                indicate_event = FALSE ;
            }
            break;



    }
    return indicate_event;
}

bool sinkAvrcpIsAvrcpLinkInPlayingState(uint16 avrcp_index)
{
    bool is_playing = FALSE;
    switch(sinkAvrcpGetPlayStatus(avrcp_index))
    {
        case avrcp_play_status_playing:
        case avrcp_play_status_fwd_seek:
        case avrcp_play_status_rev_seek:
            is_playing = TRUE;
            break;
        default:
            break;
    }
    return is_playing;
}

void sinkAvrcpSetPlayingStateToPausedOnSuspend(uint16 Id)
{
    if (avrcpAvrcpIsEnabled())
    {
        /* assume device is paused for AVRCP 1.0 devices */
        sinkAvrcpSetPlayStatusbyAddress(getA2dpLinkBdAddr(Id), avrcp_play_status_paused);
    }
}

#else /* ENABLE_AVRCP*/
static const int avrcp_disabled;
#endif /* ENABLE_AVRCP*/


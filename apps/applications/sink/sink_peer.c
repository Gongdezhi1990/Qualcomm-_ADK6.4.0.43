/*
Copyright (c) 2011 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

/****************************************************************************
    Header files
*/
#include "sink_peer.h"

#ifdef ENABLE_PEER

#include "sink_a2dp.h"
#include "sink_anc.h"
#include "sink_configmanager.h"
#include "sink_devicemanager.h"
#include "sink_config.h"
#include "sink_wired.h"
#include "sink_usb.h"
#include "sink_slc.h"
#include "sink_auth.h"
#include "sink_pio.h"
#include "sink_avrcp.h"
#include "sink_device_id.h"
#include "sink_link_policy.h"
#include "sink_powermanager.h"
#include "sink_peer_qualification.h"
#include "sink_audio.h"
#include "sink_audio_routing.h"
#include "sink_music_processing.h"
#include "sink_inquiry.h"
#include "sink_volume.h"
#include "sink_main_task.h"
#include "sink_malloc_debug.h"
#include "sink_devicemanager.h"

#include <bdaddr.h>
#include <ps.h>
#include <panic.h>
#include <sdp_parse.h>
#include <audio_plugin_music_variants.h>
#include <stdlib.h>
#include <string.h>
#include <tws_synchronised_control.h>
#include <audio_data_types.h>
#include <byte_utils.h>
#include <stdio.h>
#include <audio_plugin_output_variants.h>
#include <audio_config.h>
#include <audio.h>

#include "config_definition.h"
#include "sink_peer_config_def.h"
#include <config_store.h>

#ifdef SBC_ENCODER_CONFORMANCE
#include "sink_sbc.h"
#endif

#include "sink_debug.h"

#ifdef DEBUG_PEER_SM_1
    #define PEER_DEBUG_1(x) DEBUG(x)
#else
    #define PEER_DEBUG_1(x)
#endif

#define IS_FM_SOURCE_AVAILABLE_TO_PEER          (PEER_GDATA.audioSourcesAvailableToPeer & (1<<0))
#define IS_ANALOGUE_SOURCE_AVAILABLE_TO_PEER    (PEER_GDATA.audioSourcesAvailableToPeer & (1<<1))
#define IS_SPDIF_SOURCE_AVAILABLE_TO_PEER       (PEER_GDATA.audioSourcesAvailableToPeer & (1<<2))
#define IS_I2S_SOURCE_AVAILABLE_TO_PEER         (PEER_GDATA.audioSourcesAvailableToPeer & (1<<3))
#define IS_USB_SOURCE_AVAILABLE_TO_PEER         (PEER_GDATA.audioSourcesAvailableToPeer & (1<<4))
#define IS_A2DP1_SOURCE_AVAILABLE_TO_PEER       (PEER_GDATA.audioSourcesAvailableToPeer & (1<<5))
#define IS_A2DP2_SOURCE_AVAILABLE_TO_PEER       (PEER_GDATA.audioSourcesAvailableToPeer & (1<<6))
#define IS_BA_SOURCE_AVAILABLE_TO_PEER          (PEER_GDATA.audioSourcesAvailableToPeer & (1<<7))


#ifdef DEBUG_PEER_SM
    #define PEER_SM_DEBUG(x) DEBUG(x)
    
static const char * const av_source_str[] = 
{
    "SOURCE_NULL",
    "SOURCE_ANALOGUE",
    "SOURCE_USB",
    "SOURCE_A2DP"
};

static const char * const relay_state_str[] = 
{
    "STATE_IDLE",
    "STATE_CLOSING",
    "STATE_OPENING",
    "STATE_OPEN",
    "STATE_SUSPENDING",
    "STATE_STARTING",
    "STATE_STREAMING"
};

static const char * const relay_event_str[] = 
{
    "EVENT_DISCONNECTED",
    "EVENT_CONNECTED",
    "EVENT_CLOSED",
    "EVENT_OPEN",
    "EVENT_OPENING",
    "EVENT_NOT_OPENED",
    "EVENT_OPENED",
    "EVENT_CLOSE",
    "EVENT_SUSPENDED",
    "EVENT_START",
    "EVENT_STARTING",
    "EVENT_NOT_STARTED",
    "EVENT_STARTED",
    "EVENT_SUSPEND"
};

#else
    #define PEER_SM_DEBUG(x)
#endif


/* Unsigned Integer with Size Index 1 (implicit, 2 bytes) */
#define DATA_EL_UINT_16(value)  (0x09),(((value)>>8)&0xFF),((value)&0xFF)

/* Data Element Sequence with (count) uint16s, Size Index 5 (explicit) */
#define DATA_EL_UINT_16_LIST(count) (0x35),(3 * (count))

/* UUID with Size Index 4 (implicit, 16 bytes) */
#define DATA_EL_UUID_128(value)  (0x1C),value

/* Data Element Sequence with (count) UUID128s, Size Index 5 (explicit) */
#define DATA_EL_UUID_128_LIST(count) (0x35),(17 * (count))


/* Supported features are those Peer features offered by the local device */
/* Compatible feature are those Peer features that need to be supported by the remote device in order to be available for use */
#ifdef PEER_AS
#define SUPPORTED_SHAREME_A2DP_FEATURES  (remote_features_shareme_a2dp_source | remote_features_shareme_a2dp_sink  )
#define COMPATIBLE_SHAREME_A2DP_FEATURES (remote_features_shareme_a2dp_sink   | remote_features_shareme_a2dp_source)
#else
#define SUPPORTED_SHAREME_A2DP_FEATURES  (0)
#define COMPATIBLE_SHAREME_A2DP_FEATURES (0)
#endif

#ifdef PEER_TWS
#define SUPPORTED_TWS_A2DP_FEATURES  (remote_features_tws_a2dp_source | remote_features_tws_a2dp_sink  )
#define COMPATIBLE_TWS_A2DP_FEATURES (remote_features_tws_a2dp_sink   | remote_features_tws_a2dp_source | remote_features_peer_mono_channel_supported )
#else
#define SUPPORTED_TWS_A2DP_FEATURES  (0)
#define COMPATIBLE_TWS_A2DP_FEATURES (0)
#endif

#ifdef PEER_AVRCP
#define SUPPORTED_PEER_AVRCP_FEATURES  (remote_features_peer_avrcp_target     | remote_features_peer_avrcp_controller)
#define COMPATIBLE_PEER_AVRCP_FEATURES (remote_features_peer_avrcp_controller | remote_features_peer_avrcp_target    )
#else
#define SUPPORTED_PEER_AVRCP_FEATURES  (0)
#define COMPATIBLE_PEER_AVRCP_FEATURES (0)
#endif

#define PEER_DEVICE_UUID128 0x00, 0x00, 0x10, 0x02, 0xD1, 0x02, 0x11, 0xE1, 0x9B, 0x23, 0x00, 0x02, 0x5B, 0x00, 0xA5, 0xA5     /* QTIL Peer Device service UUID */
#define PEER_DEVICE_SUPPORTED_FEATURES  (SUPPORTED_SHAREME_A2DP_FEATURES | SUPPORTED_TWS_A2DP_FEATURES | SUPPORTED_PEER_AVRCP_FEATURES)
#define PEER_DEVICE_COMPATIBLE_FEATURES (COMPATIBLE_SHAREME_A2DP_FEATURES | COMPATIBLE_TWS_A2DP_FEATURES | COMPATIBLE_PEER_AVRCP_FEATURES)


#define BCD_MAJOR_VERSION 0xFF00
#define BCD_MINOR_VERSION 0x00F0
#define BCD_PATCH_VERSION 0x000F

#define PEER_DEVICE_MIN_SUPPORTED_BUFFER_COMPATIBILITY_VERSION          0x0320                /* versions greater than this support 350ms internal buffering delay */
#define PEER_DEVICE_MIN_SUPPORTED_SLAVE_DELAY_COMPATIBILITY_VERSION     0x0310
#define PEER_DEVICE_MIN_SUPPORTED_AUDIO_ROUTING_COMPATIBILITY_VERSION   0x0330

#define PEER_DEVICE_SYNCHRONISED_EVENTS_MINIMUM_SUPPORTED_VERSION       0x0400

/* First Battery Level Sending Delay 1 second */
#define FIRST_BATTERY_LEVEL_SENDING_DELAY    1000u

/* Subsequent Battery Level Sending Delay 60 seconds */
#define BATTERY_LEVEL_SENDING_DELAY          60000u

/* Internal message base */
#define PEER_INTERNAL_MSG_BASE               0x0000u

#ifdef PEER_TWS
static const uint16 tws_audio_routing[4] =
{
    (PEER_TWS_ROUTING_STEREO << 2) | (PEER_TWS_ROUTING_STEREO),  /* Master stereo, Slave stereo */
    (  PEER_TWS_ROUTING_LEFT << 2) | ( PEER_TWS_ROUTING_RIGHT),
    ( PEER_TWS_ROUTING_RIGHT << 2) | (  PEER_TWS_ROUTING_LEFT),
    (  PEER_TWS_ROUTING_DMIX << 2) | (  PEER_TWS_ROUTING_DMIX)

};
#endif

typedef struct
{
    unsigned :1;
    unsigned current_state:3;
    unsigned current_source:2;
    unsigned target_state:3;
    unsigned target_source:2;
    unsigned required_state:3;
    unsigned required_source:2;
} peer_states_t;

typedef enum
{
    EQ_TOGGLE,
    EQ_ON,
    EQ_OFF
} PeerEqEnableRelayType;

typedef enum
{
    ANC_ENABLE,
    ANC_DISABLE,
    ANC_SET_MODE1,
    ANC_SET_MODE2,
    ANC_SET_MODE3,
    ANC_SET_MODE4,
    ANC_SET_MODE5,
    ANC_SET_MODE6,
    ANC_SET_MODE7,
    ANC_SET_MODE8,
    ANC_SET_MODE9,
    ANC_SET_MODE10
} PeerAncEventType;

#define ROUTING_MODE_MASTER_INDEX   0
#define ROUTING_MODE_SLAVE_INDEX    1

/* Sink Peer global data */
typedef struct  __sink_peer_global_data_t
{
    unsigned PeerUseDeviceId:1;
    unsigned remote_peer_audio_conn_status:3;
    unsigned PeerSource:2;                   /* Initial source used for new TWS connections */
    unsigned tws_qual_enable_peer_open:1;    /* Flag to indicate App to trigger opening of Peer media channel */
    unsigned UnlockPeerStateMachine:1;       /* Allow peer to try establishing a connection */
    
    unsigned audioSourcesAvailableToPeer:8;
    
    uint16   peer_battery_level;
    int16    device_trim_slave; 
    int16    device_trim_master;
    bdaddr  remote_peer_ag_bd_addr;
    peer_states_t  peer;
    PeerRemoteRenderingMode remote_peer_rendering_mode;
}sink_peer_global_data_t;

/* Internal messages */
typedef enum __peer_internal_msg
{
    PEER_INTERNAL_MSG_SEND_BATTERY_LEVEL = PEER_INTERNAL_MSG_BASE,
    PEER_INTERNAL_MSG_STATE_MACHINE_UNLOCK,
    PEER_INTERNAL_MSG_DELAY_SOURCE_STREAM,
    PEER_INTERNAL_MSG_TOP

} peer_internal_msg_t;

#define USER_EQ_PARAMS_SIZE     ((NUM_USER_EQ_BANDS * USER_EQ_BAND_PARAMS_SIZE) + USER_EQ_PARAM_PRE_GAIN_SIZE)

typedef enum
{
    SYNCHRONISED_PEER_EVENT_MUTE,
    SYNCHRONISED_PEER_EVENT_POWER,
    SYNCHRONISED_PEER_EVENT_UPDATE_EQ,
    SYNCHRONISED_PEER_EVENT_UPDATE_AUDIO_ENHANCEMENTS,
    SYNCHRONISED_PEER_EVENT_UPDATE_VOLUME,
    SYNCHRONISED_PEER_EVENT_UPDATE_TRIM,
    SYNCHRONISED_PEER_EVENT_UPDATE_ANC_EVENT
} synchronised_peer_event_t;

typedef enum
{
    SCALED_VOLUME_PERCENTAGE_LO_OFFSET,
    SCALED_VOLUME_PERCENTAGE_HI_OFFSET
} scaled_volume_percentage_offsets_t;

typedef enum
{
    PEER_TRIM_LOW_OFFSET,
    PEER_TRIM_HIGH_OFFSET
} peer_trim_offsets_t;

#define SYNCHRONISED_PEER_EVENT_MUTE_PAYLOAD_SIZE 1
#define SYNCHRONISED_PEER_EVENT_POWER_PAYLOAD_SIZE 1
#define SYNCHRONISED_PEER_EVENT_ANC_PAYLOAD_SIZE 1
#define SYNCHRONISED_PEER_EVENT_UPDATE_EQ_PAYLOAD_SIZE     USER_EQ_PARAMS_SIZE
#define SYNCHRONISED_PEER_EVENT_UPDATE_AUDO_ENHANCEMENTS_PAYLOAD_SIZE     MUSIC_PROCESSING_PAYLOAD_SIZE
#define SYNCHRONISED_PEER_EVENT_UPDATE_VOLUME_PAYLOAD_SIZE SCALED_VOLUME_PERCENTAGE_SIZE
#define SYNCHRONISED_PEER_EVENT_UPDATE_TRIM_PAYLOAD_SIZE    2

#define TIMESTAMP_LENGTH                    (sizeof(tws_timestamp_t))
#define SYNCHRONISED_PEER_EVENT_ID_LENGTH   (1)

#define SYNCHRONISED_COMMAND_LATENCY_IN_MILLISECONDS    50

/* Internal message handler */
static void peerInternalMessageHandler  ( Task task, MessageId id, Message message );

/* Internal message handler TaskData */
static const TaskData peer_internal_message_task = {peerInternalMessageHandler};

static const uint8 peer_device_service_record_template[] =
{
    /* DataElUint16, ServiceClassIDList */
    DATA_EL_UINT_16(UUID_SERVICE_CLASS_ID_LIST),
        /* DataElSeq, 1 UUID128 */
        DATA_EL_UUID_128_LIST(1),
            /* DataElUuid128, 128-bit (16 byte) UUID */
            DATA_EL_UUID_128(PEER_DEVICE_UUID128),
            
#define PEER_DEVICE_OFFSET_UUID128_ATTR_VAL (6)

            
    /* DataElUint16, Supported Features */
    DATA_EL_UINT_16(UUID_SUPPORTED_FEATURES),
        /* DataElUint16, Supported Features Bitmask */
        DATA_EL_UINT_16(PEER_DEVICE_SUPPORTED_FEATURES),

#define PEER_DEVICE_OFFSET_SUPPORTED_FEATURES (27)
        
    /* DataElUint16, Supported Codecs */
#ifdef PEER_TWS
    DATA_EL_UINT_16(UUID_SUPPORTED_CODECS_TWS),
#else
    DATA_EL_UINT_16(UUID_SUPPORTED_CODECS_SHAREME),
#endif
    /* DataElUint16, codecs bitmask */
            DATA_EL_UINT_16(0),
        
#define PEER_DEVICE_OFFSET_CODEC_BITS (32)


    /* DataElUint16, Legacy Peer Version */
    DATA_EL_UINT_16(UUID_LEGACY_VERSION),
        /* DataElUint16, Peer Device support version number */
        DATA_EL_UINT_16(0),
        
#define PEER_DEVICE_OFFSET_MINIMUM_SUPPORTED_PEER_VERSION    38


    /* DataElUint16, Current Peer Version */
    DATA_EL_UINT_16(UUID_CURRENT_VERSION),
        /* DataElUint16, Peer Device support version number */
        DATA_EL_UINT_16(0)

#define PEER_DEVICE_OFFSET_PEER_VERSION    44

};

#define PEER_DEVICE_SERVICE_RECORD_SIZE (sizeof peer_device_service_record_template)


/* DataElSeq(0x35), Length(0x11), 128-bit UUID(0x1C) */
static const uint8 peer_device_search_pattern[] = {0x35, 0x11, 0x1C, PEER_DEVICE_UUID128};

static const uint8 peer_device_attr_list[] =
{
    DATA_EL_UINT_16_LIST(5),
        DATA_EL_UINT_16(UUID_SUPPORTED_FEATURES),       /* Supported Features */
        DATA_EL_UINT_16(UUID_SUPPORTED_CODECS_SHAREME), /* Supported ShareMe Codecs */
        DATA_EL_UINT_16(UUID_SUPPORTED_CODECS_TWS),     /* Supported TWS Codecs */
        DATA_EL_UINT_16(UUID_LEGACY_VERSION),           /* Legacy Peer Version */
        DATA_EL_UINT_16(UUID_CURRENT_VERSION)           /* Current Peer Version */
};
/* Prototype and task structure for Peer Device credentials specific SDP searches */
static void handleCredentialsMessage (Task task, MessageId id, Message message);
static const struct TaskData credentials_task = { handleCredentialsMessage };

static sink_peer_global_data_t gPeerData;

#define PEER_GDATA gPeerData    
#define PEER_STATES_GDATA PEER_GDATA.peer

#define isPeerStateStable()             (PEER_STATES_GDATA.current_state == PEER_STATES_GDATA.required_state)
#define isPeerStateStableAndStreaming() (isPeerStateStable() && PEER_STATES_GDATA.required_state == RELAY_STATE_STREAMING)
#define SIZE_OF_UUID_IN_WORDS           8

/* Local functions */
static bool determineRequiredState (void);

static bool PeerUseCsrUuid(void);

static RelaySource peerTwsSource(void);

static bool peerAllowedToEstablishConnection(void);

static PeerPairingMode peerTwsPairingMode(void);

static PeerPairingMode peerShareMePairingMode(void);

static Task getRelayPlugin(uint8 seid);

static audio_plugin_output_mode_t peerGetForwardingMode(void);

#ifdef PEER_TWS
static PeerAncEventType getAncEventPayload(MessageId anc_event);

static bool peerSlaveRequestEqEnableChange(PeerEqEnableRelayType action);
static bool peerSlaveRequestEqNextBank(void);
static void peerUpdateTWSDeviceTrim(const volume_direction direction , const tws_device_type tws_device);
static void setPeerTwsSlaveTrim(int16 slave_trim);
static bool peerTrimsAreHandledThroughSynchronisedEvents(void);
#endif

static uint16 peerGetPeerVersion(void);
static uint16 peerGetMinimumSupportedPeerVersion(void);
static void peerUpdateMonoSupportInfo(uint8 *supported_features);
static bool peerCheckForRenderingModeCompatablity(void);
static bool peerCheckForVersionCompatablity(uint16 version);
static PeerRemoteRenderingMode peerGetRemoteRenderingMode(void);

#ifdef PEER_TWS
/*************************************************************************
NAME    
    getPeerRequiredStates
    
DESCRIPTION
    Function to get required peer state.

RETURNS
    RelayState
    
**************************************************************************/
RelayState getPeerRequiredStates(void)
{
    return PEER_STATES_GDATA.required_state;
}


static PeerTwsAudioRouting getMasterRoutingModeAtIndex(unsigned routing_mode_index)
{
    return ((tws_audio_routing[routing_mode_index] >> 2) & 0x3);
}

static PeerTwsAudioRouting getSlaveRoutingModeAtIndex(unsigned routing_mode_index)
{
    return (tws_audio_routing[routing_mode_index] & 0x3);
}

/*************************************************************************
NAME    
    peerChangeAudioRouting
    
DESCRIPTION
    Handle the User Event EventUsrChangeAudioRouting

RETURNS
    TRUE if TWS Audio Routing was changed, FALSE otherwise
    
**************************************************************************/
static bool peerChangeAudioRouting(void)
{
    if ((peerGetPermittedRoutingModes() != 0) && !PeerImplementsTwsForcedDownmix())
    {
        uint16 current_routing = ((getTWSRoutingMode(tws_master) & 0x3) << 2) | (getTWSRoutingMode(tws_slave) & 0x3);
        uint16 distance = 16;
        uint16 index = 0;
        uint16 i;
    
        /* Find entry in tws_audio_routing table which is closest to current routing mode */
        for(i = 0; i < sizeof(tws_audio_routing); i++)
        {
            if (tws_audio_routing[i] < current_routing)
            {
                if ((current_routing - tws_audio_routing[i]) < distance)
                {
                    distance = current_routing - tws_audio_routing[i];
                    index = i;
                }
            }
            else
            {
                if ((tws_audio_routing[i] - current_routing) < distance)
                {
                    distance = tws_audio_routing[i] - current_routing;
                    index = i;
                }
            }
        }

        do{
            /* Select next routing mode in table */
            index = (index + 1) % sizeof(tws_audio_routing);
        }while(!((1 << index) & peerGetPermittedRoutingModes()));
    
        peerSetPeerAudioRouting(getMasterRoutingModeAtIndex(index), getSlaveRoutingModeAtIndex(index));
        return TRUE;
    }
    
    return FALSE;
}

/*************************************************************************
NAME    
    peerGetTwsForceDownmixPio
    
DESCRIPTION
    Returns the input PIO configured to force the device into Downmix
    routing mode
    
RETURNS
    PIO number
    
**************************************************************************/
static uint8 peerGetTwsForcedDownmixPio(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    uint8 pio = PIN_INVALID;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        pio = read_data->PeerTwsForceDownmixPio;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    
    return pio;
}

/*************************************************************************
NAME    
    peerGetTwsForcedDownmixMode
    
DESCRIPTION
    Gets the TWS Forced Downmix mode based on the configuration and
    state of the controlling PIO
    
RETURNS
    Forced Downmix routing mode.
    
**************************************************************************/
PeerTwsForcedDownmixMode peerGetTwsForcedDownmixMode(void)
{
    PeerTwsForcedDownmixMode mode = PEER_TWS_FORCED_DOWNMIX_MODE_DISABLED;
    uint8 pio;
    
    pio = peerGetTwsForcedDownmixPio();
    
    if (pio != PIN_INVALID)
    {
        bool state = PioGetPio(pio) ^ PioIsInverted(pio);
        mode = state ? PEER_TWS_FORCED_DOWNMIX_MODE_ON : PEER_TWS_FORCED_DOWNMIX_MODE_OFF;
    }
    
    return mode;
}
/*************************************************************************
NAME    
    PeerImplementsTwsForcedDownmix
    
DESCRIPTION
    Determines if the device implements TWS Forced Downmix routing mode.
    
RETURNS
    TRUE if Forced Downmix is implemented, FALSE otherwise
    
**************************************************************************/
bool PeerImplementsTwsForcedDownmix(void)
{
    return peerGetTwsForcedDownmixMode() != PEER_TWS_FORCED_DOWNMIX_MODE_DISABLED;
}

/*************************************************************************
NAME    
    peerGetStartRoutingModes
    
DESCRIPTION
    Determines starting TWS Master and Slave routing modes constrained by
    Fixed Routing and Forced Downmix configuration
    
RETURNS
    void
    
**************************************************************************/
static void peerGetStartRoutingModes(PeerTwsAudioRouting *master_routing,
                                     PeerTwsAudioRouting *slave_routing)
{
    switch (peerGetTwsForcedDownmixMode())
    {
    case PEER_TWS_FORCED_DOWNMIX_MODE_DISABLED:
    /*  Forced Downmix feature not used, return current routing  */
        *master_routing = getTWSRoutingMode(tws_master);
        *slave_routing = getTWSRoutingMode(tws_slave);
        break;
        
    case PEER_TWS_FORCED_DOWNMIX_MODE_ON:
    /*  Force Master and Slave into Downmix routing mode  */
        *master_routing = PEER_TWS_ROUTING_DMIX;
        *slave_routing = PEER_TWS_ROUTING_DMIX;
        break;

    case PEER_TWS_FORCED_DOWNMIX_MODE_OFF:
    /*  Use default or fixed routing  */
        switch (peerGetFixedRoutingMode())
        {
        case PEER_FIXED_ROUTING_NONE:
            *master_routing = sinkAudioGetTwsMasterAudioRouting();
            *slave_routing = sinkAudioGetTwsSlaveAudioRouting();
            break;
            
        case PEER_FIXED_ROUTING_LEFT:
            *master_routing = PEER_TWS_ROUTING_LEFT;
            *slave_routing = PEER_TWS_ROUTING_RIGHT;
            break;
            
        case PEER_FIXED_ROUTING_RIGHT:
            *master_routing = PEER_TWS_ROUTING_RIGHT;
            *slave_routing = PEER_TWS_ROUTING_LEFT;
            break;
        }
        break;
    }
    
    PEER_DEBUG(("peerGetStartRoutingModes: mas=%u sla=%u\n",
                *master_routing,
                *slave_routing));
}

/*************************************************************************
NAME    
    PeerDetermineTwsForcedDownmixMode
    
DESCRIPTION
    Select and apply appropriate Forced Downmix mode based on configured PIO
    
RETURNS
    TRUE if a Forced Downmix mode is in effect (On or Off) after the
    function call, FALSE if the Forced Downmix feature is not in use.
    
**************************************************************************/
bool PeerDetermineTwsForcedDownmixMode(void)
{
    bool dmix_implemented = PeerImplementsTwsForcedDownmix();
    
    if (dmix_implemented)
    {
        PeerTwsAudioRouting master_routing;
        PeerTwsAudioRouting slave_routing;
        
        peerGetStartRoutingModes(&master_routing, &slave_routing);
        peerSetPeerAudioRouting(master_routing, slave_routing);
    }
    
    return dmix_implemented;
}
#endif /* PEER_TWS */

/*************************************************************************
NAME    
    peerPauseIfStreaming
    
DESCRIPTION
    Handle the User Event EventUsrPeerSessionConnDisc

RETURNS
    None
    
**************************************************************************/
void peerPauseIfStreaming(void)
{
    uint16 index;
    uint16 srcIndex;
    uint16 avrcpIndex;
    /* check whether the a2dp connection is present and streaming data and that the audio is routed, if thats true then pause the stream */
    if(sinkAudioIsAudioRouted() && getA2dpIndexFromSink(sinkAudioGetRoutedAudioSink(), &index)
        && (A2dpMediaGetState(getA2dpLinkDataDeviceId( index), getA2dpLinkDataStreamId(index)) == a2dp_stream_streaming)
        && a2dpGetSourceIndex(&srcIndex) && (srcIndex == index)
        && sinkAvrcpGetIndexFromBdaddr(getA2dpLinkBdAddr(index), &avrcpIndex, TRUE))
    {
        /* cancel any queued ff or rw requests and then pause the streaming*/
        MessageCancelAll (&theSink.task, EventUsrAvrcpFastForwardPress);
        MessageCancelAll (&theSink.task, EventUsrAvrcpRewindPress);
        sinkAvrcpPlayPauseRequest(avrcpIndex,AVRCP_PAUSE);
    }
}

/*************************************************************************
NAME    
    peerSetPeerUseDeviceId
    
DESCRIPTION
    Set the Peer Use Device Id

RETURNS
    None
    
**************************************************************************/
void peerSetPeerUseDeviceId(uint8 value)
{
    PEER_GDATA.PeerUseDeviceId = value;
}

/*************************************************************************
NAME    
    peerSetTwsQualEnablePeerOpen
    
DESCRIPTION
    Set the Peer Tws Qual Enable Peer Open bit

RETURNS
    None
    
**************************************************************************/
void peerSetTwsQualEnablePeerOpen(uint8 value)
{
    PEER_GDATA.tws_qual_enable_peer_open = value;
}

/*************************************************************************
NAME    
    peerSetUnlockPeerStateMachine
    
DESCRIPTION
    Set the Peer Unlock Peer State Machine bit

RETURNS
    None
    
**************************************************************************/
void peerSetUnlockPeerStateMachine(uint8 value)
{
    PEER_GDATA.UnlockPeerStateMachine = value;
}

/*************************************************************************
NAME    
    peerSetPeerSource
    
DESCRIPTION
    Set the Peer source

RETURNS
    None
    
**************************************************************************/
void peerSetPeerSource(uint8 new_source)
{
    PEER_GDATA.PeerSource = new_source;
}

/*************************************************************************
NAME    
    peerSetRemoteRenderingMode
    
DESCRIPTION
    Set the Remote peer rendering mode from inquiry results
    
RETURNS
    None
    
**************************************************************************/
void peerSetRemoteRenderingMode(PeerRemoteRenderingMode rendering_mode)
{
    PEER_GDATA.remote_peer_rendering_mode = rendering_mode;
}

/*************************************************************************
NAME    
    peerGetRemoteRenderingMode
    
DESCRIPTION
    Returns the remote device rendering mode
    
RETURNS
    rendering mode of the remote device
    
**************************************************************************/
static PeerRemoteRenderingMode peerGetRemoteRenderingMode(void)
{
    PEER_DEBUG(("PEER : Rendering mode returned as 0x%x\n",PEER_GDATA.remote_peer_rendering_mode));
    return PEER_GDATA.remote_peer_rendering_mode;
}

/*************************************************************************
NAME    
    expandUuid128
    
DESCRIPTION
    Expands a 128-bit UUID stored as eight 16-bit words to sixteen 8-bit words

RETURNS
    TRUE if non zero UUID read from PS, FALSE otherwise
    
**************************************************************************/
static bool expandUuid128 (uint16 *packed_uuid,uint8 *unpacked_uuid)
{
    bool non_zero = FALSE;
    uint16 *read_ptr = packed_uuid + 8;
    uint8 *write_ptr = unpacked_uuid + 16;
    
    while (--read_ptr >= packed_uuid)
    {
        uint16 read_data = *read_ptr;
        
        PEER_DEBUG((" %X",read_data));
        
        *(--write_ptr) = (read_data & 0xFF);
        *(--write_ptr) = ((read_data >> 8) & 0xFF);
        
        if (read_data)
        {
            non_zero = TRUE;
        }
        
    }
    
    PEER_DEBUG(("   non_zero=%u\n",non_zero));
    return non_zero;
}

static void expandUint16(uint16 data, uint8 *unpacked_data)
{
    *unpacked_data = (data >> 8);
    *(unpacked_data + 1) = (data & 0x00FF);
}

/*************************************************************************
NAME    
    RegisterPeerDeviceServiceRecord
    
DESCRIPTION
    Registers a Peer Device SDP record

RETURNS
    None
    
**************************************************************************/
static void RegisterPeerDeviceServiceRecord(sink_peer_readonly_config_def_t *read_data)
{
    uint8 *service_record = (uint8 *)mallocPanic(PEER_DEVICE_SERVICE_RECORD_SIZE);
    uint16 codecs;

    memmove(service_record, peer_device_service_record_template, PEER_DEVICE_SERVICE_RECORD_SIZE);
    if (!read_data->PeerUseQtilUuid)
    {
        uint16 uuid[SIZE_OF_UUID_IN_WORDS]={0};
        /* Update 128-bit UUID from PS or readonly config data */
        memcpy(&uuid[0], read_data->service_record,sizeof(uuid));
        expandUuid128(uuid, &service_record[PEER_DEVICE_OFFSET_UUID128_ATTR_VAL]);
    }

    /*Update mono support info */
    peerUpdateMonoSupportInfo(&service_record[PEER_DEVICE_OFFSET_SUPPORTED_FEATURES]);
    
    /* Update codec bitmaps */
    if (peerGetLocalSupportedCodecs(&codecs))
    {
#ifdef PEER_AS
        codecs &= ~(1 << AAC_CODEC_BIT); 
#endif
        PEER_DEBUG(("Local peer codecs 0x%04X\n", codecs));
        expandUint16(codecs, &service_record[PEER_DEVICE_OFFSET_CODEC_BITS]);
    }

    expandUint16(peerGetMinimumSupportedPeerVersion(), &service_record[PEER_DEVICE_OFFSET_MINIMUM_SUPPORTED_PEER_VERSION]);

    expandUint16(peerGetPeerVersion(), &service_record[PEER_DEVICE_OFFSET_PEER_VERSION]);

    /* Malloc'd block is passed to f/w and unmapped from VM space */
    ConnectionRegisterServiceRecord(&theSink.task, PEER_DEVICE_SERVICE_RECORD_SIZE, (uint8 *)service_record);
}

/*************************************************************************
NAME    
    peerInitPeerStatus
    
DESCRIPTION
    Inits peer data structure and sets the initial required state

RETURNS
    None
    
**************************************************************************/
static void peerInitPeerStatus (void)
{
    PEER_DEBUG(("peerInitPeerStatus\n"));
    
    memset(&PEER_STATES_GDATA, 0, sizeof(PEER_STATES_GDATA));
    determineRequiredState();
#ifdef ENABLE_PEER_BATTERY_LEVEL
    PEER_GDATA.peer_battery_level = BATTERY_LEVEL_INVALID;
#endif
}

#if PEER_TWS
/*************************************************************************
NAME    
    isRemotePeerAbleToRelay
    
DESCRIPTION
    Utility function to determine whether remote peer status is either :
        1) in a call state or "don't relay" state 
        AND 
        2) device does not have single device operation enabled. 

RETURNS
    TRUE if peer is able to relay, FALSE otherwise
    
**************************************************************************/
static bool isRemotePeerAbleToRelay(void)
{
    uint16 peer_index;
    if(a2dpGetPeerIndex(&peer_index))
    {
        if((getA2dpPeerStatus(peer_index, REMOTE_PEER) & (PEER_STATUS_IN_CALL | PEER_STATUS_DONT_RELAY))
            && (peerIsSingleDeviceOperationEnabled() == FALSE))
        {
            return FALSE;
        }
    }
    return TRUE;
}
#endif /* PEER_TWS*/

/*************************************************************************
NAME    
    handleCredentialsSdpServiceSearchAttributeCfm
    
DESCRIPTION
    Handles search results returned from requesting Peer credentials (Device Id and/or Peer SDP records)

RETURNS
    None
    
**************************************************************************/
static void handleCredentialsSdpServiceSearchAttributeCfm (const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
    /* Not inquiring thus we are the acceptor of an incoming A2DP signalling channel connect request */
    uint16 index;
    
    if (getA2dpIndexFromBdaddr(&cfm->bd_addr, &index))
    {
        bool peer_sdp_record_obtained = FALSE;

        if (peerUseDeviceIdRecord() && (getA2dpPeerRemoteDevice(index) == remote_device_unknown))
        {   /* Response is due to a request for the Device Id service record */
            if ( (cfm->status==sdp_response_success) && !cfm->error_code )
            {   /* Response obtained, examine it */
                setA2dpPeerRemoteDevice(index, CheckRemoteDeviceId(cfm->attributes, (uint8)cfm->size_attributes));
            }
            else
            {
                setA2dpPeerRemoteDevice(index, remote_device_nonpeer);
            }
            setA2dpPeerRemoteFeatures(index, remote_features_none);
        }
        else
        {   /* Response is due to a request for the Peer Device service record */
        
            peer_sdp_record_obtained = TRUE;
                
            if ( (cfm->status==sdp_response_success) && !cfm->error_code )
            {   /* Response obtained, examine it */
                uint16 remote_peer_version = peerGetRemoteVersionFromServiceRecord(cfm->attributes, cfm->size_attributes);
                remote_features remote_peer_features = peerGetRemoteSupportedFeaturesFromServiceRecord(cfm->attributes, cfm->size_attributes);

                peerSetRemoteSupportedCodecsFromServiceRecord(cfm->attributes, cfm->size_attributes);
                peerSetRemoteRenderingMode(peerCheckRemoteRenderingMode(remote_peer_features));
                setA2dpPeerRemoteFeatures(index, peerDetermineCompatibleRemoteFeatures(remote_peer_features, remote_peer_version));
                setA2dpPeerRemoteDevice(index, remote_device_peer);
                setA2dpLinkDataPeerVersion(index, remote_peer_version);
                setPeerDSPRequiredBufferingLevel(index, getPeerDSPBufferingRequired(remote_peer_version));
            }
            else
            {
                setA2dpPeerRemoteFeatures(index, remote_features_none);
                setA2dpPeerRemoteDevice(index, remote_device_nonpeer);
                setA2dpLinkDataPeerVersion(index, 0);
                setPeerDSPRequiredBufferingLevel(index, PEER_BUFFER_250MS);
            }
        }

        if ((getA2dpPeerRemoteDevice(index) == remote_device_peer) && !peer_sdp_record_obtained)
        {   /* Remote device is a Peer, check to see what features it supports */
            RequestRemotePeerServiceRecord((Task)&credentials_task, &cfm->bd_addr);
        }
        else
        {   /* A non Peer device or a Peer with known features */
            if ((getA2dpPeerRemoteDevice(index) != remote_device_peer) ||        /* A non-Peer device */
                ((getA2dpPeerRemoteFeatures(index) != remote_features_none) &&   /* A Peer with matching features? */
                 peerIsCompatibleDevice(getA2dpLinkDataPeerVersion(index)) ) )     /* A Peer with compatible version number? */
            {   /* Accept the connection */
                PEER_DEBUG(("PEER: Device compatible - accepting A2DP Signalling\n"));
                issueA2dpSignallingConnectResponse(&cfm->bd_addr, TRUE);
            }
            else
            {   /* Reject the connection */
                PEER_DEBUG(("PEER: Device NOT compatible - REJECTING A2DP Signalling\n"));
                issueA2dpSignallingConnectResponse(&cfm->bd_addr, FALSE);
            }
        }
    }
}


/****************************************************************************
NAME    
    handleCredentialsMessage
    
DESCRIPTION
    Task function to handle responses to Peer device specific credentials requests
    
RETURNS
    void
*/
static void handleCredentialsMessage (Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch (id)
    {
    case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
        PEER_DEBUG(("PEER [Cred]: CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM\n"));
        handleCredentialsSdpServiceSearchAttributeCfm ((const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T*) message);
        break;
    
    default:
        PEER_DEBUG(("PEER [Cred]: Unexpected message, id=0x%X\n", id));
        break;
    }
}


/****************************************************************************
NAME
    peerSetLowPowerLinkPolicyInConnectedState
    
DESCRIPTION
    Function to set link policy in connected state to enter low power mode
    
RETURNS
    void
*/
static void peerSetLowPowerLinkPolicyInConnectedState(void)
{
    uint16 peer_id;
   
    if (a2dpGetPeerIndex(&peer_id))
    {
        linkPolicyUseA2dpSettings( getA2dpLinkDataDeviceId(peer_id),
                                               getA2dpLinkDataStreamId(peer_id),
                                               A2dpSignallingGetSink(getA2dpLinkDataDeviceId(peer_id)) );
    }
}


/****************************************************************************
NAME    
    peerCredentialsRequest
    
DESCRIPTION
    Request Peer device credentials (device id and/or peer device SDP record)
    from the specified device
    
RETURNS
    TRUE if credentials were requested, FALSE otherwise
*/
bool peerCredentialsRequest (bdaddr *device_addr)
{
    if (RequestRemoteDeviceId((Task)&credentials_task, device_addr))
    {
        PEER_DEBUG(("PEER: Requesting device id record\n"));
        return TRUE;
    }
    
    if (RequestRemotePeerServiceRecord((Task)&credentials_task, device_addr))
    {
        PEER_DEBUG(("PEER: Requesting Peer Device service record\n"));
        return TRUE;
    }
    
    return FALSE;
}


/*************************************************************************
NAME    
    peerRequestServiceRecord
    
DESCRIPTION
    Issues a request to obtain the attributes of a Peer Device SDP record from
    the specified device

RETURNS
    TRUE if a search requested, FALSE otherwise
    
**************************************************************************/
bool RequestRemotePeerServiceRecord (Task task, const bdaddr *bd_addr)
{
    if (PeerUseCsrUuid())
    {
        PEER_DEBUG(("Requesting PEER SDP record using QTIL UUID\n"));
        ConnectionSdpServiceSearchAttributeRequest(task, bd_addr, 32, sizeof(peer_device_search_pattern), peer_device_search_pattern, sizeof(peer_device_attr_list), peer_device_attr_list);
        return TRUE;
    }
    else
    {
        uint16 uuid[SIZE_OF_UUID_IN_WORDS] = {0};
        sink_peer_readonly_config_def_t *read_data = NULL;
    
        uint8 *search_pattern = (uint8 *)mallocPanic(sizeof(peer_device_search_pattern));
        (void)configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data); /* Get 128 bit UUID from PS or readonly config*/
        memcpy(&uuid[0], read_data->service_record,sizeof(uuid));
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);

        if ( expandUuid128(uuid,&search_pattern[3]) )
        {
            PEER_DEBUG(("Requesting PEER SDP record using custom UUID\n"));
            memcpy(search_pattern, peer_device_search_pattern, 3);          /* Get Data El Seq info from peer_device_search_pattern */
    
            ConnectionSdpServiceSearchAttributeRequest(task, bd_addr, 32, sizeof(peer_device_search_pattern), search_pattern, sizeof(peer_device_attr_list), peer_device_attr_list);
            return TRUE;
        }
        else
        {
            PEER_DEBUG(("Not requesting PEER SDP record\n"));
        }
        free(search_pattern);
    }
    
    return FALSE;
}


/*************************************************************************
NAME    
    peerSetRemoteSupportedCodecsFromServiceRecord
    
DESCRIPTION
    Extracts and stores the list of optional codecs supported by the remote peer device
    from the supplied Peer Device SDP record

RETURNS
    None
**************************************************************************/
void peerSetRemoteSupportedCodecsFromServiceRecord (const uint8 *attr_data, uint16 attr_data_size)
{
/*  Store peer's supported codec bitmask
 *  TODO: handle the (currently unsupported) case where we have both AS and TWS  
 */
    uint32 codecs = 0;

    UNUSED(attr_data);
    UNUSED(attr_data_size);
    
#ifdef PEER_AS
    if (SdpParseGetArbitrary((uint8)attr_data_size, attr_data, UUID_SUPPORTED_CODECS_SHAREME, &codecs))
    {
        PEER_DEBUG(("Remote ShareMe codecs 0x%04lX\n", codecs));
    }
#endif
    
#ifdef PEER_TWS
    if (SdpParseGetArbitrary((uint8)attr_data_size, attr_data, UUID_SUPPORTED_CODECS_TWS, &codecs))
    {
        PEER_DEBUG(("Remote TWS codecs 0x%04lX\n", codecs));
    }
#endif

    peerSetRemoteSupportedCodecs((uint16)codecs);
}


/*************************************************************************
NAME    
    peerGetRemoteVersionFromServiceRecord
    
DESCRIPTION
    Extracts the Peer Device version number from the supplied SDP record.
    
RETURNS
    The Peer Device support version number of a connected Peer, or 0 if not
    present in SDP record
    
**************************************************************************/
uint16 peerGetRemoteVersionFromServiceRecord( const uint8 *attr_data, uint16 attr_data_size )
{
    uint16 version;
    uint32 long_version = 0;
    
    PEER_DEBUG(( "peerGetRemoteVersion" ));
    
    /* Extract the legacy Peer version number, which is the only Peer version number advertised by ADK3.x based devices */ 
    SdpParseGetArbitrary( (uint8)attr_data_size, attr_data, UUID_LEGACY_VERSION, &long_version );
    PEER_DEBUG(( "   legacy version = %lX\n", long_version ));
    version = ( uint16 )long_version;
    
    /* Extract current Peer version of remote device.  This will not exist for ADK3.x based devices */ 
    if( SdpParseGetArbitrary( (uint8)attr_data_size, attr_data, UUID_CURRENT_VERSION, &long_version ) )
    {
        PEER_DEBUG(( "   current version = %lX\n", long_version ));
        version = ( version > ( uint16 )long_version ) ? version : ( uint16 )long_version;
    }
    
    PEER_DEBUG(( "   version = %X\n", version ));
    return version;
}


/*************************************************************************
NAME    
    peerGetRemoteSupportedFeaturesFromServiceRecord
    
DESCRIPTION
    Extracts the Peer Device supported features from the supplied SDP record.
    
RETURNS
    The Peer Device supported features of a connected Peer, or 0 if not
    present in SDP record
    
**************************************************************************/
remote_features peerGetRemoteSupportedFeaturesFromServiceRecord (const uint8 *attr_data, uint16 attr_data_size)
{
    uint32 supported_features;
    
    PEER_DEBUG(("peerGetRemoteSupportedFeatures"));
    
    if ( SdpParseGetArbitrary((uint8)attr_data_size, attr_data, UUID_SUPPORTED_FEATURES, &supported_features) )
    {
        PEER_DEBUG(("   features = %lX\n",supported_features));
        return (remote_features)supported_features;
    }
    
    PEER_DEBUG(("   features not found\n"));
    return remote_features_none;
}


/*************************************************************************
NAME    
    peerDetermineCompatibleRemoteFeatures
    
DESCRIPTION
    Identifies the set of compatible features based on locally supported peer features 
    and the remote features obtained from a Peer during a Peer Device SDP record search 
    request.
    The compatible features are a set of mutually matching features i.e. if the local device
    supports a tws source role then the compatible feature would be the Peer supporting a tws
    sink role.

RETURNS
    The compatible set of features of the connected Peer
    
**************************************************************************/
remote_features peerDetermineCompatibleRemoteFeatures (remote_features supported_features, uint16 version)
{
    remote_features compatible_features = remote_features_shareme_a2dp_sink & PEER_DEVICE_COMPATIBLE_FEATURES;   /* Minimally, we can expect a device to be a standard A2DP sink */
    
    PEER_DEBUG(("GetCompatibleRemotePeerFeatures\n"));
    
    if ( supported_features && version )
    {
        PEER_DEBUG(("   features = %X   version = %X\n",supported_features,version));
        
        if (peerIsCompatibleDevice(version))
        {   /* Same peer version, so we can expect to support all A2DP and AVRCP features advertised */
            PEER_DEBUG(("   version matched\n"));
            compatible_features = supported_features & PEER_DEVICE_COMPATIBLE_FEATURES;
        }
        else
        {   /* ShareMe uses standard A2DP operation, thus ShareMe can be supported in one or more roles */
            compatible_features = supported_features & (remote_features_shareme_a2dp_source | remote_features_shareme_a2dp_sink) & PEER_DEVICE_COMPATIBLE_FEATURES;
        }
    }
    PEER_DEBUG(("   compatible features = %X\n",compatible_features));
    
    return compatible_features;
}
    

/*************************************************************************
NAME    
    peerIsCompatibleDevice
    
DESCRIPTION
    Uses the Peer version number to determine if another Peer device is compatible

RETURNS
    TRUE if device is deemed compatible, FALSE otherwise
    
**************************************************************************/
bool peerIsCompatibleDevice( uint16 version )
{
    PEER_DEBUG(( "peerIsCompatibleDevice  remote ver=0x%X local_ver=0x%X min_ver=0x%X\n", version, peerGetPeerVersion(), peerGetMinimumSupportedPeerVersion() ));
	
    if(peerCheckForRenderingModeCompatablity() && peerCheckForVersionCompatablity(version))
    {
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
NAME    
    peerCheckForRenderingModeCompatablity
    
DESCRIPTION
    checks for rendering compatablity

RETURNS
    TRUE if device is rendering compatible, FALSE otherwise
    
**************************************************************************/
static bool peerCheckForRenderingModeCompatablity(void)
{
    if(AudioConfigGetRenderingMode() == peerGetRemoteRenderingMode())
    {
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
NAME    
    peerCheckForVersionCompatablity
    
DESCRIPTION
    checks for version compatablity

RETURNS
    TRUE if device is version compatible, FALSE otherwise
    
**************************************************************************/
static bool peerCheckForVersionCompatablity(uint16 version)
{
    if( version < peerGetMinimumSupportedPeerVersion() )
    {
        return FALSE;
    }
    return TRUE;
}

static void peerUpdateWriteableGlobalData(void)
{
    sink_peer_writeable_config_def_t *write_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID, (const void **)&write_data))
    {
        PEER_GDATA.device_trim_master = write_data->device_trim_master;   /* TWS master device trim gain */
        PEER_GDATA.device_trim_slave = write_data->device_trim_slave;    /* TWS slave device trim gain */
        configManagerReleaseConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID);
    }
}

/*************************************************************************
DESCRIPTION
    Sets peer mode in audio_config 
    
**************************************************************************/
static void peerUpdatePeerMode(void)
{
#ifdef PEER_AS
    AudioConfigSetPeerMode(peer_mode_shareme);
#endif
#ifdef PEER_TWS
    AudioConfigSetPeerMode(peer_mode_tws);
#endif
}

/*************************************************************************
NAME    
    peerInit
    
DESCRIPTION
    Register Peer Device SDP record and initialize peer data structure and states.

RETURNS
    None
    
**************************************************************************/
void peerInit( void )
{
    sink_peer_readonly_config_def_t *read_data = NULL;
 
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
       /*Note: Update Global Data from read only config data.
         This is needed as these data are updated in runtime without writing it back to the config*/
        PEER_GDATA.PeerUseDeviceId = read_data->PeerUseDeviceId;
        PEER_GDATA.PeerSource = 0;/*Not a config parameter*/
        PEER_GDATA.audioSourcesAvailableToPeer = read_data->audioSourcesAvailableToPeer;
        PEER_GDATA.tws_qual_enable_peer_open = read_data->tws_qual_enable_peer_open;
        PEER_GDATA.UnlockPeerStateMachine = read_data->UnlockPeerStateMachine;

        peerUpdatePeerMode();
        RegisterPeerDeviceServiceRecord(read_data);
        peerInitPeerStatus();

        BdaddrSetZero(&PEER_GDATA.remote_peer_ag_bd_addr);

        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
   /*Note: Update Global Data from writeable config data.
    This is needed as these data are updated in runtime without writing it back to the config*/
    peerUpdateWriteableGlobalData();
}

#ifdef PEER_TWS
/*************************************************************************
NAME    
    peerUpdateTWSMuteState
    
DESCRIPTION
    Responsible for updating the mute state according to the state
    of the peer state machine. Should mute when device is a TWS master
    during relay setup or tear down, or when devices are in idle state
    with no audio source.
    Has no effect on ShareMe operation.

RETURNS
    None
    
**************************************************************************/
void peerUpdateTWSMuteState(void)
{  
    uint16 peer_idx ;

    /* Link-loss, relay or single peer device, no mute */
    if(!peerIsRelayAvailable())
    {
        if(isPeerStateStable() || !isRemotePeerAbleToRelay() ||
          (a2dpGetPeerIndex(&peer_idx) && (A2dpDeviceIsInLinkloss(getA2dpLinkDataDeviceId(peer_idx))))) /* Peer considered as available but in linkloss */
        {
            AudioSetInputAudioMute(FALSE);
        }
    }
    else if(isPeerStateStableAndStreaming()) 
    {/* Peer is streaming, unmute if muted */
        AudioSetInputAudioMute(FALSE);
    }
    else
    {/* Peer is not currently streaming, mute if not muted. */
        uint16 peer_id;
        /* Check if there is a peer index, and if so check it supports TWS.
           If both are true, Mute Input Audio.*/
        if(a2dpGetPeerIndex(&peer_id) && peerIsTWSPeer(peer_id))
        {
            AudioSetInputAudioMute(TRUE);
        }
    }      
}
#endif /*PEER_TWS*/

/*************************************************************************
NAME    
    updateAudioGating
    
DESCRIPTION
    Updates gating used for "multimedia" (non-sco) types of audio based on
    call status of any connected Peer and audio source being relayed

RETURNS
    None
    
**************************************************************************/
void updateAudioGating (void)
{
    uint16 peer_id;
    
    /* Initially assume that no audio needs gating */
    audioUngateAudio(audio_gate_all);
    
    if (PEER_STATES_GDATA.current_state != RELAY_STATE_STREAMING)
    {   /* Ensure we only relay audio data when in the Streaming state */
        audioGateAudio(audio_gate_relay);
    }
    
    if ( a2dpGetPeerIndex(&peer_id) )
    {   /* Have a connected Peer */
        if (peerIsSingleDeviceOperationEnabled() && 
            (getA2dpPeerRemoteFeatures(peer_id) & remote_features_tws_a2dp_sink) &&
            (getA2dpPeerStatus(peer_id, REMOTE_PEER) & PEER_STATUS_IN_CALL))
        {   /* Peer has an active call and we are in single device mode */
            audioGateAudio(audio_gate_multimedia);
            return;
        }
    }
}

/*************************************************************************
NAME    
    issuePeerStatusChange
    
DESCRIPTION
    Issues a relay availability status update to the specified Peer

RETURNS
    TRUE if successful, FALSE otherwise
    
**************************************************************************/
static void issuePeerStatusChange (uint16 peer_id, PeerStatusChange peer_status_change)
{
#ifdef ENABLE_AVRCP            
    uint16 avrcp_id;
    
    /* does the device support AVRCP and is AVRCP currently connected to this device? */
    for_all_avrcp(avrcp_id)
    {    
        /* ensure media is streaming and the avrcp channel is that requested to be paused */
        if (sinkAvrcpIsInitialised() && SinkA2dpIsInitialised() && sinkAvrcpIsConnected(avrcp_id) && 
            (BdaddrIsSame(getA2dpLinkBdAddr(peer_id), sinkAvrcpGetLinkBdAddr(avrcp_id))))
        {
            /* Inform remote peer of change to local devices status */
            sinkAvrcpVendorUniquePassthroughRequest( avrcp_id, AVRCP_PEER_CMD_PEER_STATUS_CHANGE, sizeof(peer_status_change), (const uint8 *)&peer_status_change );
        }
    }
#endif
}

/*************************************************************************
NAME    
    checkPeerFeatures
    
DESCRIPTION
    Checks the remote features bitmask of the current Peer device for support 
    of one or more of the supplied features.

RETURNS
    TRUE if one or more features supported, FALSE otherwise
    
**************************************************************************/
static bool checkPeerFeatures (remote_features features)
{
    uint16 peer_id;
    
    if (a2dpGetPeerIndex(&peer_id))
    {
        if (getA2dpPeerRemoteFeatures(peer_id) & features)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

/*************************************************************************
NAME    
    setRelayAvailability
    
DESCRIPTION
    Updates the relay availability for the specified peer status

RETURNS
    None
    
**************************************************************************/
static void setRelayAvailability (PeerStatus *peer_status, PeerStatusChange peer_status_change)
{
    PEER_DEBUG(("setRelayAvailability\n"));
    
    if ((peer_status_change & PEER_CALL_STATUS_CHANGE) == PEER_STATUS_CHANGE_CALL_INACTIVE)
    {
        PEER_DEBUG(("   call inactive\n"));
        *peer_status &= ~PEER_STATUS_IN_CALL;
    }
    
    if ((peer_status_change & PEER_CALL_STATUS_CHANGE) == PEER_STATUS_CHANGE_CALL_ACTIVE)
    {
        PEER_DEBUG(("   call active\n"));
        *peer_status |= PEER_STATUS_IN_CALL;
    }
    
    if ((peer_status_change & PEER_RELAY_STATUS_CHANGE) == PEER_STATUS_CHANGE_RELAY_AVAILABLE)
    {
        PEER_DEBUG(("   relay available\n"));
        *peer_status &= ~PEER_STATUS_DONT_RELAY;
    }
    
    if ((peer_status_change & PEER_RELAY_STATUS_CHANGE) == PEER_STATUS_CHANGE_RELAY_UNAVAILABLE)
    {
        PEER_DEBUG(("   relay unavailable\n"));
        *peer_status |= PEER_STATUS_DONT_RELAY;
    }
    
    if ((peer_status_change & PEER_RELAY_OWNERSHIP_CHANGE) == PEER_STATUS_CHANGE_RELAY_FREED)
    {
        PEER_DEBUG(("   relay free'd\n"));
        *peer_status &= ~PEER_STATUS_OWNS_RELAY;
    }
    
    if ((peer_status_change & PEER_RELAY_OWNERSHIP_CHANGE) == PEER_STATUS_CHANGE_RELAY_CLAIMED)
    {
        PEER_DEBUG(("   relay claimed\n"));
        *peer_status |= PEER_STATUS_OWNS_RELAY;
    }
}

/*************************************************************************
NAME    
    setRemoteRelayAvailability
    
DESCRIPTION
    Helper function used to update remote relay availability

RETURNS
    None
    
**************************************************************************/
static void setRemoteRelayAvailability (uint16 peer_id, PeerStatusChange peer_status_change)
{
    PeerStatus remote_peer_status = getA2dpPeerStatus(peer_id, REMOTE_PEER);
    
    PEER_DEBUG(("setRemoteRelayAvailability\n"));
    setRelayAvailability( &remote_peer_status, peer_status_change );
    setA2dpPeerStatus(peer_id, REMOTE_PEER, remote_peer_status);

    updateAudioGating();
    audioUpdateAudioRouting();
}

/*************************************************************************
NAME    
    setLocalRelayAvailability
    
DESCRIPTION
    Helper function to update local relay availability and issue status change to Peer

RETURNS
    None
    
**************************************************************************/
static void setLocalRelayAvailability (uint16 peer_id, PeerStatusChange peer_status_change)
{
    PeerStatus local_peer_status = getA2dpPeerStatus(peer_id, LOCAL_PEER);
    PEER_DEBUG(("setLocalRelayAvailability\n"));
    setRelayAvailability( &local_peer_status, peer_status_change);
    setA2dpPeerStatus(peer_id, LOCAL_PEER, local_peer_status);
    
    issuePeerStatusChange( peer_id, peer_status_change );
}


/*************************************************************************
NAME    
    peerClaimRelay
    
DESCRIPTION
    Helper function to update local relay availability(claim or free) and issue status change to Peer

RETURNS
    None
    
**************************************************************************/
void peerClaimRelay(bool claim)
{
    uint16 peer_id;
    
    if (a2dpGetPeerIndex(&peer_id))
    {
        if(claim)
        {        
            if(peerIsRelayAvailable() && !(getA2dpPeerStatus(peer_id, LOCAL_PEER) & PEER_STATUS_OWNS_RELAY))
            {
                PEER_DEBUG(("Relay claimed\n"));
                setLocalRelayAvailability( peer_id, PEER_STATUS_CHANGE_RELAY_CLAIMED);
            }
        }
        else
        {
            if(getA2dpPeerStatus(peer_id, LOCAL_PEER) & PEER_STATUS_OWNS_RELAY)
            {
                PEER_DEBUG(("Relay freed\n"));
                setLocalRelayAvailability( peer_id, PEER_STATUS_CHANGE_RELAY_FREED);
            }
        }
    }
}

/*************************************************************************
NAME    
    requestPeerRoleSwitch
    
DESCRIPTION
    Issues a request to role switch the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
static bool requestPeerRoleSwitch (uint16 peer_id)
{
    if (getA2dpLinkRole(peer_id) == LR_CURRENT_ROLE_MASTER)
    {
        return FALSE;
    }
    else
    {
        Sink signalling_sink = A2dpSignallingGetSink(getA2dpLinkDataDeviceId(peer_id));
        
        PEER_DEBUG(("   requesting role check before issuing start...\n"));
        
        setA2dpLinkRole(peer_id, LR_CHECK_ROLE_PENDING_START_REQ);
        linkPolicyGetRole(&signalling_sink);
        
        return TRUE;
    }
}

/*************************************************************************
NAME    
    setPeerStreaming
    
DESCRIPTION
    Issues a request to start the relay channel to the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
static bool setPeerStreaming (void)
{
    uint16 peer_id;
    
    if (a2dpGetPeerIndex(&peer_id))
    {
        PEER_DEBUG(("issuePeerStartRequest peer=%u  local_peer_status=0x%X  remote_peer_status=0x%X\n",peer_id,getA2dpPeerStatus(peer_id, LOCAL_PEER),getA2dpPeerStatus(peer_id,REMOTE_PEER)));
        
        if (peerIsRelayAvailable())
        {
#ifdef PEER_TWS
            if (peerIsThisDeviceTwsMaster() && PeerImplementsTwsForcedDownmix())
            {
                PeerTwsAudioRouting master_routing;
                PeerTwsAudioRouting slave_routing;
                
                peerGetStartRoutingModes(&master_routing, &slave_routing);
                
                if ((getTWSRoutingMode(tws_master) != master_routing) ||
                    (getTWSRoutingMode(tws_slave) != slave_routing))
                {
                     peerSetPeerAudioRouting(master_routing, slave_routing);
                }
            }
#endif
            if (requestPeerRoleSwitch(peer_id))
            {
                return TRUE;
            }
            
            if (a2dpIssuePeerStartRequest())
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}


/*************************************************************************
NAME    
    issueSourceStartResponse
    
DESCRIPTION
    Peer state machine helper function
    
    Issues a start response to the current source

RETURNS
    TRUE if successful, FALSE otherwise
    
**************************************************************************/
static bool issueSourceStartResponse (RelaySource current_source)
{
    uint16 av_id;
    
    PEER_DEBUG(("issueSourceStartResponse av=%u\n",current_source));
    
    switch (current_source)
    {
    case RELAY_SOURCE_NULL:
        break;

    case RELAY_SOURCE_ANALOGUE:
        /* Don't actually need to send a start response to an analogue source */
        return TRUE;

    case RELAY_SOURCE_USB:
        /* Don't actually need to send a start response to a USB source */
        return TRUE;

    case RELAY_SOURCE_A2DP:
        if (a2dpGetSourceIndex(&av_id))
        {
            a2dp_stream_state a2dp_state = A2dpMediaGetState(getA2dpLinkDataDeviceId(av_id), getA2dpLinkDataStreamId(av_id));
            
            if (a2dp_state == a2dp_stream_streaming)
            {   /* Already streaming */
                return TRUE;
            }
            else if (a2dp_state == a2dp_stream_starting)
            {
                /* Cancel the peer streaming indication timeout */
                MessageCancelAll(&theSink.task,EventSysPeerStreamTimeout);

                /* Starting, so attempt to send a response */
                return A2dpMediaStartResponse(getA2dpLinkDataDeviceId(av_id), getA2dpLinkDataStreamId(av_id), TRUE);
            }
            /* else in wrong state, so fall through and return FALSE */
        }
        break;
    }
    
    return FALSE;
}

static a2dp_link_role peerGetLinkRole(void)
{
    uint16 source_id;
    a2dp_link_role role = LR_UNKNOWN_ROLE;
    if(a2dpGetPeerIndex(&source_id))
    {
        role = getA2dpLinkRole(source_id);
    }
    return role;
}

bool peerIsLinkMaster(void)
{
    return (peerGetLinkRole() == LR_CURRENT_ROLE_MASTER);
}

static bool peerIsLinkRoleKnown(void)
{
    return peerGetLinkRole() != LR_UNKNOWN_ROLE;
}

static bool peerIsSlaveDelaySupportedByConnectedPeer(void)
{
    uint16 source_id;
    return (a2dpGetPeerIndex(&source_id)
            && (getA2dpLinkDataPeerVersion(source_id) >= PEER_DEVICE_MIN_SUPPORTED_SLAVE_DELAY_COMPATIBILITY_VERSION));
}

static bool peerIsAllowedToRelaySource(void)
{ 
    return ((peerIsLinkMaster() && peerIsSlaveDelaySupportedByConnectedPeer())
            || peerAllowedToEstablishConnection() || (peerIsLinkMaster() && peerTwsQualificationEnabled()));
}

bool peerIsModeTws(void)
{
    return audio_plugin_output_mode_tws == peerGetForwardingMode();
}

/*************************************************************************
NAME    
    determineRequiredState
    
DESCRIPTION
    Peer state machine helper function.
    
    Examines any currently connected sources to determine which source needs to be relayed.

RETURNS
    TRUE if a change to the required state occurs, FALSE otherwise
    
**************************************************************************/
static bool determineRequiredState (void)
{
    RelayState last_state = PEER_STATES_GDATA.required_state;
    RelaySource last_source = PEER_STATES_GDATA.required_source;
    
    /* Initially assume no source is connected */
    PEER_STATES_GDATA.required_state = RELAY_STATE_IDLE;
    PEER_STATES_GDATA.required_source = RELAY_SOURCE_NULL;
    
    if(peerIsAllowedToRelaySource())
    {
        uint16 source_id;
        RelaySource initial_source = peerTwsSource();
        
        if (((initial_source == RELAY_SOURCE_ANALOGUE) || (initial_source == RELAY_SOURCE_NULL)) && analogAudioConnected())
        {   /* If an analogue source is connected, it is considered to be streaming */
            PEER_STATES_GDATA.required_state = RELAY_STATE_STREAMING;
            PEER_STATES_GDATA.required_source = RELAY_SOURCE_ANALOGUE;
        }

        if (((initial_source == RELAY_SOURCE_USB) || (initial_source == RELAY_SOURCE_NULL)) && (usbAudioIsAttached() && sinkUsbIsSpeakerActive() && (!sinkUsbAudioIsSuspended())))
        {   /* Streaming USB source has higher priority than streaming analogue source */
            PEER_STATES_GDATA.required_state = RELAY_STATE_STREAMING;
            PEER_STATES_GDATA.required_source = RELAY_SOURCE_USB;
        }
    
        /* Check for A2DP Source */
        if (((initial_source == RELAY_SOURCE_A2DP) || (initial_source == RELAY_SOURCE_NULL)) && a2dpGetSourceIndex(&source_id))
        {
            a2dp_stream_state a2dp_state = A2dpMediaGetState(getA2dpLinkDataDeviceId(source_id), getA2dpLinkDataStreamId(source_id));
            
            if ((!sinkA2dpIsA2dpLinkSuspended(source_id)) && ((a2dp_state == a2dp_stream_starting) || (a2dp_state == a2dp_stream_streaming)))
            {   /* A streaming (or heading towards it) A2DP source has the highest priority */
                PEER_STATES_GDATA.required_state = RELAY_STATE_STREAMING;
                PEER_STATES_GDATA.required_source = RELAY_SOURCE_A2DP;
            }
            else if ((a2dp_state == a2dp_stream_open) || (a2dp_state == a2dp_stream_suspending) || (a2dp_state == a2dp_stream_streaming))
            {   /* A2DP media stream is open (or heading back to it).  Check for streaming state as well as AVRCP play status can lead AVDTP stream state changes */
                uint16 peer_id;

                if (a2dpGetPeerIndex(&peer_id) && !(getA2dpPeerRemoteFeatures(peer_id) & remote_features_peer_source))
                {   /* We have a Peer that can't be a source itself, thus it is ok to leave a media channel open when suspended as */
                    /* it will never want to use the relay channel. (Only when idle will the relay channel be marked as available) */
                    if (PEER_STATES_GDATA.required_state != RELAY_STATE_STREAMING)
                    {   /* Don't allow an open A2DP stream to have higher precedence that an streaming Analogue/USB stream */ 
                        PEER_STATES_GDATA.required_state = RELAY_STATE_OPEN;
                        PEER_STATES_GDATA.required_source = RELAY_SOURCE_A2DP;
                    }
                }
            }
        }
    }
    
    PEER_SM_DEBUG(("PEER: Required[%s:%s]\n",av_source_str[PEER_STATES_GDATA.required_source],relay_state_str[PEER_STATES_GDATA.required_state]));
    
    if ((last_state != PEER_STATES_GDATA.required_state) || (last_source != PEER_STATES_GDATA.required_source))
    {   /* Required state has changed */
        return TRUE;
    }
    else
    {   /* No change to required state */
        return FALSE;
    }
}

/*************************************************************************
NAME    
    updateTargetRelayState
    
DESCRIPTION
    Peer state machine helper function to manage updating of the target state.
    
    The Peer state machine is designed to manage the media (relay) channel to another Peer device.  Generally, the Peer driving the
    state machine is the one attempting to relay audio.
    However, for adherence to the GAVDP specification, an A2DP source must allow a sink to open and start a media channel.  We cope
    with this by allowing a Peer to change the current_state but keep the current_source and target_source as RELAY_SOURCE_NULL.  The 
    target_state is allowed to track the current_state in this scenario, so the state machine does not attempt to drive the relay 
    channel from this side.

    The required_state/source indicates what is currently streaming and what the relay channel ultimately needs to be forwarding.
    
    The required_state/source can change rapidly as a user pauses/resumes a single source or swaps between different sources.
    This is tempered by the target_state/source which attempts to track the required_state/source as closely as possible but is
    only updated when the current state reaches that last set target state.  
    
    However, changing the required_source will cause the state machine to return the relay state to idle before progressing again.
    This ensures the relay media channel is closed and then re-opened with a SEP suitable for forwarding audio data from the 
    required_source.
    
    The final stage of the state machine is to only action changes to the current state once one of the non-transitional (stable) 
    states has been reached (idle, open, streaming and routed).  This, again, prevents rapid changes to the required_state from 
    unduly upsetting operation.
    
RETURNS
    None
    
**************************************************************************/
static void updateTargetRelayState (void)
{
    if ( peerIsRelayAvailable() )
    {
        if (PEER_STATES_GDATA.current_state == PEER_STATES_GDATA.target_state)
        {   /* Reached the target state, so update as appropriate */
            if ((PEER_STATES_GDATA.target_source != PEER_STATES_GDATA.required_source) && (PEER_STATES_GDATA.target_state != RELAY_STATE_IDLE))
            {   /* Source has changed, go through idle state to get to it */
                PEER_STATES_GDATA.target_state = RELAY_STATE_IDLE;
            }
            else
            {   /* Update to the required state */
                PEER_STATES_GDATA.target_source = PEER_STATES_GDATA.required_source;
                PEER_STATES_GDATA.target_state = PEER_STATES_GDATA.required_state;
            }
        }
        else
        {   /* Current state differs to target state */
            if ((PEER_STATES_GDATA.current_source == RELAY_SOURCE_NULL) && (PEER_STATES_GDATA.target_source == RELAY_SOURCE_NULL))
            {   /* No source being managed.  Thus the other Peer is driving the state machine */
                /* Update target state to keep track of what other Peer is doing and also prevent this device from driving the state machine too */
                if (checkPeerFeatures(remote_features_peer_source) || (PEER_STATES_GDATA.required_source == RELAY_SOURCE_NULL))
                {   /* Only prevent local Peer state machine from driving state if other Peer can act as a source or we don't require use of relay channel */
                    /* Prevents sink only Peers (such as PTS) from holding a media channel open if we need to use it */
                    PEER_STATES_GDATA.target_state = PEER_STATES_GDATA.current_state;
                }
                
                if (PEER_STATES_GDATA.current_state == RELAY_STATE_IDLE)
                {   /* Other peer has now finished using the relay channel.  Update to the required state */
                    PEER_STATES_GDATA.target_source = PEER_STATES_GDATA.required_source;
                    PEER_STATES_GDATA.target_state = PEER_STATES_GDATA.required_state;
                }
            }
            /* else allow relay state to reach target state before updating target state again */
        }
    }
    else
    {
        if ((PEER_STATES_GDATA.current_source == RELAY_SOURCE_NULL) && (PEER_STATES_GDATA.target_source == RELAY_SOURCE_NULL))
        {   /* No source being managed.  Thus the other Peer is driving the state machine */
            /* Update target state to keep track of what other Peer is doing and also prevent this device from driving the state machine too */
            PEER_STATES_GDATA.target_state = PEER_STATES_GDATA.current_state;
        }
        else
        {   /* Either no Peer connected or not in a state to accept a media stream.  Revert target state to idle */
            PEER_STATES_GDATA.target_source = RELAY_SOURCE_NULL;
            PEER_STATES_GDATA.target_state = RELAY_STATE_IDLE;
        }
    }

    PEER_SM_DEBUG(("PEER: Target[%s:%s]\n",av_source_str[PEER_STATES_GDATA.target_source],relay_state_str[PEER_STATES_GDATA.target_state]));
}


/*************************************************************************
NAME    
    updateCurrentState
    
DESCRIPTION
    Peer state machine helper function to help manage changes to the current state

RETURNS
    TRUE if successful, FALSE otherwise
    
**************************************************************************/
void updateCurrentState (RelayState relay_state)
{
    PEER_STATES_GDATA.current_state = relay_state;
    
    if (PEER_STATES_GDATA.current_state == RELAY_STATE_IDLE)
    {   /* Reset current source once idle state reached */
        PEER_STATES_GDATA.current_source = RELAY_SOURCE_NULL;
    }
    
    /* Update target state, if necessary */
    updateTargetRelayState();
    
    if (PEER_STATES_GDATA.current_source == RELAY_SOURCE_NULL)
    {   /* Update current source if we have reached idle and target has changed */
        PEER_STATES_GDATA.current_source = PEER_STATES_GDATA.target_source;
    }
    
    PEER_SM_DEBUG(("PEER: Current[%s:%s]\n",av_source_str[PEER_STATES_GDATA.current_source],relay_state_str[PEER_STATES_GDATA.current_state]));
}

static void peerScheduleUnlockStateMachine(void)
{
    PEER_DEBUG(("PEER: schedule unlock\n"));
    MessageSendLater((TaskData*)&peer_internal_message_task, PEER_INTERNAL_MSG_STATE_MACHINE_UNLOCK, NULL, PEER_UNLOCK_DELAY );
}

#define peerUnlockStateMachine()   {PEER_DEBUG(("PEER: unlock\n")); peerSetUnlockPeerStateMachine(1);}
#define peerLockStateMachine()     {PEER_DEBUG(("PEER: lock\n")); peerSetUnlockPeerStateMachine(0);}

/*************************************************************************
NAME    
    peerSendBatteryLevel

DESCRIPTION
    Sends Battery level to currently connected Peer

RETURNS
    None

**************************************************************************/
#ifdef ENABLE_PEER_BATTERY_LEVEL
static void peerSendBatteryLevel (void)
{
    uint16 peerA2dpId;
    uint16 peerAvrcpIndex;

    if(!a2dpGetPeerIndex(&peerA2dpId))
    {
        return;
    }

    for_all_avrcp(peerAvrcpIndex)
    {
        if(sinkAvrcpIsInitialised() && BdaddrIsSame(getA2dpLinkBdAddr(peerA2dpId), sinkAvrcpGetLinkBdAddr(peerAvrcpIndex)))
        {
            uint8 battery_levels[2];
            uint16 batterylevel = powerManagerBatteryLevelAsPercentage();
            battery_levels[0] = (uint8)(batterylevel >> 8);
            battery_levels[1] = (uint8)batterylevel & 0xff;
            sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex , AVRCP_PEER_CMD_UPDATE_BATTERY_LEVEL,
                                                    sizeof(battery_levels), (const uint8 *)battery_levels);
            break;
        }
    }
}
#else
#define peerSendBatteryLevel() ((void)0)
#endif

/*************************************************************************
NAME
    peerScheduleSendBatteryLevel

DESCRIPTION
    Schedule sending battery level message

RETURNS
    None

**************************************************************************/
#ifdef ENABLE_PEER_BATTERY_LEVEL
static void peerScheduleSendBatteryLevel(uint16 delay)
{
    MessageSendLater((TaskData*)&peer_internal_message_task, PEER_INTERNAL_MSG_SEND_BATTERY_LEVEL , 0 , delay);
}
#else
#define peerScheduleSendBatteryLevel(delay)
#endif

/*************************************************************************
NAME
    peerStopSendingBatteryLevel

DESCRIPTION
    Stop sending internal send battery level messages

RETURNS
    None

**************************************************************************/
#ifdef ENABLE_PEER_BATTERY_LEVEL
static void peerStopSendingBatteryLevel(void)
{
    PEER_GDATA.peer_battery_level = BATTERY_LEVEL_INVALID;
    MessageCancelAll((TaskData*)&peer_internal_message_task, PEER_INTERNAL_MSG_SEND_BATTERY_LEVEL ) ;
}
#else
#define peerStopSendingBatteryLevel()
#endif

/*************************************************************************
NAME
    peerInternalMessageHandler

DESCRIPTION
    Handle peer internal messages

RETURNS
    None

**************************************************************************/
static void peerInternalMessageHandler  ( Task task, MessageId id, Message message )
{
    UNUSED(task);
    UNUSED(message);

    switch(id)
    {
        case PEER_INTERNAL_MSG_SEND_BATTERY_LEVEL:
        {
            peerSendBatteryLevel();
            peerScheduleSendBatteryLevel(BATTERY_LEVEL_SENDING_DELAY);
        }
        break;
        
        case PEER_INTERNAL_MSG_STATE_MACHINE_UNLOCK:
            PEER_DEBUG(("PEER: unlock msg recieved\n"));
            if(peerIsLinkRoleKnown())
            {
                peerUnlockStateMachine();
                PEER_UPDATE_REQUIRED_RELAY_STATE("UNLOCK PEER");
            }
            else
            {
                peerScheduleUnlockStateMachine();
            }
            break;

        case PEER_INTERNAL_MSG_DELAY_SOURCE_STREAM:
            /* Relay any AV Source stream to a connected Peer */
            PEER_DEBUG(("PEER: received PEER_INTERNAL_MSG_DELAY_SOURCE_STREAM\n"));
            peerRelaySourceStream();
            break;

        default:
            PEER_DEBUG(("PEER: Unhandled msg[%x]\n", id));
            break;
    }
}

/*************************************************************************
NAME
    peerAdvanceRelayState
    
DESCRIPTION
    Updates the Peer state machine current state based on the supplied event.
    This function should be called when an event occurs that would cause a change to
    the actual Peer state machine status.
    
    This forms one of the two functions that should be used to drive the Peer state machine.

RETURNS
    None
    
**************************************************************************/
void peerAdvanceRelayState (RelayEvent relay_event)
{
    PEER_SM_DEBUG(("----\nPEER: Relay[%s]\n",relay_event_str[relay_event]));
    
    /* For PTS there needs to be State machine change to trigger 
     * reconfiguring of Caps (audio SRC's Caps) on getting stream suspend indication.
     * And for some PTS TC's, PTS expects SRC to open the discover/set configuration.
     * So, this wrapper function is called to handle such situation */
    if(!peerQualificationAdvanceRelayState(relay_event))
    {
        switch(relay_event)
        {
            case RELAY_EVENT_DISCONNECTED:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_DISCONNECTED\n"));
                updateCurrentState( RELAY_STATE_IDLE );
                peerStopSendingBatteryLevel();
                peerLockStateMachine();
                determineRequiredState();
                break;
        
            case RELAY_EVENT_CONNECTED:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_CONNECTED\n"));
                updateCurrentState( RELAY_STATE_IDLE );
                peerScheduleSendBatteryLevel(FIRST_BATTERY_LEVEL_SENDING_DELAY);

                if ( (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state) && a2dpIssuePeerOpenRequest() )
                {   /* Open relay stream */
                    /* Opened relay stream */
                    PEER_DEBUG(("PEER: Issue PEER OPEN REQ\n"));
                    updateCurrentState( RELAY_STATE_OPENING );
                }
                else
                {
                    peerSetLowPowerLinkPolicyInConnectedState();
                }
                peerScheduleUnlockStateMachine();
                break;
        
            case RELAY_EVENT_CLOSED:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_CLOSED\n"));
                updateCurrentState( RELAY_STATE_IDLE );
                peerStopSendingBatteryLevel();
                if (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state)
                {   /* Open relay stream */
                    if (a2dpIssuePeerOpenRequest())
                    {
                        PEER_SM_DEBUG(("PEER: Issue[%s]\n","PEER OPEN REQ"));
                        updateCurrentState( RELAY_STATE_OPENING );
                    }
                }
                break;
        
            case RELAY_EVENT_OPEN:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_OPEN\n"));
                updateCurrentState( RELAY_STATE_IDLE );
                if (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state)
                {   /* Open relay stream */
                    if (a2dpIssuePeerOpenRequest())
                    {
                            PEER_DEBUG(("PEER: Issue PEER OPEN REQ\n"));
                        updateCurrentState( RELAY_STATE_OPENING );
                    }
                }
                break;
        
            case RELAY_EVENT_OPENING:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_OPENING\n"));
                updateCurrentState( RELAY_STATE_OPENING );
                /* Peer will wait for open to complete, so do nothing further */
                break;
        
            case RELAY_EVENT_NOT_OPENED:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_NOT_OPENED\n"));
                updateCurrentState( RELAY_STATE_IDLE );
                if (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state)
                {   /* Open relay stream */
                    if (a2dpIssuePeerOpenRequest())
                    {
                            PEER_DEBUG(("PEER: Issue PEER OPEN REQ\n"));
                        updateCurrentState( RELAY_STATE_OPENING );
                    }
                }
                break;
        
            case RELAY_EVENT_OPENED:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_OPENED\n"));
                updateCurrentState( RELAY_STATE_OPEN );
                if (PEER_STATES_GDATA.target_state < PEER_STATES_GDATA.current_state)
                {   /* Close relay stream */
                    if (a2dpIssuePeerCloseRequest())
                    {
                            PEER_DEBUG(("PEER: Issue PEER CLOSE REQ\n"));
                        updateCurrentState( RELAY_STATE_CLOSING );
                    }
                }
                else if (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state)
                {   /* Start relay stream */
                    if (setPeerStreaming())
                    {
                            PEER_DEBUG(("PEER: Issue PEER START REQ\n"));
                        updateCurrentState( RELAY_STATE_STARTING );
                    }
                }
                break;

            case RELAY_EVENT_CLOSE:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_CLOSE\n"));
                updateCurrentState( RELAY_STATE_OPEN );
                if (PEER_STATES_GDATA.target_state < PEER_STATES_GDATA.current_state)
                {   /* Close relay stream */
                    if (a2dpIssuePeerCloseRequest())
                    {
                            PEER_DEBUG(("PEER: Issue PEER CLOSE REQ\n"));
                        updateCurrentState( RELAY_STATE_CLOSING );
                    }
                }
                break;
    
            case RELAY_EVENT_SUSPENDED:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_SUSPENDED\n"));
                updateCurrentState( RELAY_STATE_OPEN );
                if (PEER_STATES_GDATA.target_state < PEER_STATES_GDATA.current_state)
                {   /* Close relay stream */
                        if(a2dpIssuePeerCloseRequest())
                    {
                            PEER_DEBUG(("PEER: Issue PEER CLOSE REQ\n"));
                        updateCurrentState( RELAY_STATE_CLOSING );
                    }
                }
                else if (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state)
                {   /* Start relay stream */
                    if (setPeerStreaming())
                    {
                            PEER_DEBUG(("PEER: Issue PEER START REQ\n"));
                        updateCurrentState( RELAY_STATE_STARTING );
                    }
                }
                break;
        
            case RELAY_EVENT_START:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_START\n"));
                updateCurrentState( RELAY_STATE_OPEN );
                if (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state)
                {   /* Start relay stream */
                    if (setPeerStreaming())
                    {
                            PEER_DEBUG(("PEER: Issue PEER START REQ\n"));
                        updateCurrentState( RELAY_STATE_STARTING );
                    }
                }
                break;

            case RELAY_EVENT_STARTING:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_STARTING\n"));
                updateCurrentState( RELAY_STATE_STARTING );
                /* Peer will wait for start to complete, so do nothing further */
                break;
        
            case RELAY_EVENT_NOT_STARTED:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_NOT_STARTED\n"));
                updateCurrentState( RELAY_STATE_OPEN );
                if (PEER_STATES_GDATA.target_state < PEER_STATES_GDATA.current_state)
                {   /* Close relay stream */
                    if (a2dpIssuePeerCloseRequest())
                    {
                            PEER_DEBUG(("PEER: Issue PEER CLOSE REQ\n"));
                        updateCurrentState( RELAY_STATE_CLOSING );
                    }
                        PEER_SM_DEBUG(("PEER: Issue[%s]\n","PEER CLOSE REQ"));
                }
                else if (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state)
                {   /* Start relay stream */
                    if (setPeerStreaming())
                    {
                            PEER_DEBUG(("PEER: Issue PEER START REQ\n"));
                        updateCurrentState( RELAY_STATE_STARTING );
                    }
                }
                break;
        
            case RELAY_EVENT_STARTED:
                {
                    bool started;

                    UNUSED(started);  /* only used for debug */

                    PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_STARTED\n"));

                    /* Issue start response to the AV source regardless of target_state */
                    started = issueSourceStartResponse(PEER_STATES_GDATA.current_source);
                    PEER_DEBUG(("PEER: Issue SOURCE START RESP: %u\n", started));

                    updateCurrentState( RELAY_STATE_STREAMING );
                    if (PEER_STATES_GDATA.target_state < PEER_STATES_GDATA.current_state)
                    {   /* Suspend relay stream */
                        if (a2dpIssuePeerSuspendRequest())
                        {
                            PEER_DEBUG(("PEER: Issue PEER SUSPEND REQ\n"));
                            updateCurrentState( RELAY_STATE_SUSPENDING );
                        }
                        PEER_SM_DEBUG(("PEER: Issue[%s]\n","PEER SUSPEND REQ"));
                    }
                }
                break;
        
            case RELAY_EVENT_SUSPEND:
                PEER_DEBUG(("peerAdvanceRelayState : RELAY_EVENT_SUSPEND\n"));
                updateCurrentState( RELAY_STATE_STREAMING );
                if (PEER_STATES_GDATA.target_state < PEER_STATES_GDATA.current_state)
                {   /* Suspend relay stream */
                    if (a2dpIssuePeerSuspendRequest())
                    {
                            PEER_DEBUG(("PEER: Issue PEER SUSPEND REQ\n"));
                        updateCurrentState( RELAY_STATE_SUSPENDING );
                    }
                }
                break;
        }
    }
    
    /* Update mute state if necessary */
    peerUpdateTWSMuteState();
    updateAudioGating();
    audioUpdateAudioRouting();
}
        

/*************************************************************************
NAME    
    kickPeerStateMachine
    
DESCRIPTION
    Will automatically attempt to advance the Relay channel if it is in a 'steady' state (i.e. not expecting an external event)

RETURNS
    None
    
**************************************************************************/
static void kickPeerStateMachine (void)
{
    switch (PEER_STATES_GDATA.current_state)
    {
    case RELAY_STATE_IDLE:
        if (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state)
        {   /* Open relay stream */
            peerAdvanceRelayState(RELAY_EVENT_OPEN);
        }
        break;
    case RELAY_STATE_OPEN:
        if (PEER_STATES_GDATA.target_state < PEER_STATES_GDATA.current_state)
        {   /* Close relay stream */
            peerAdvanceRelayState(RELAY_EVENT_CLOSE);
        }
        if (PEER_STATES_GDATA.target_state > PEER_STATES_GDATA.current_state)
        {   /* Start relay stream */
            peerAdvanceRelayState(RELAY_EVENT_START);
        }
        break;
    case RELAY_STATE_STREAMING:
        if (PEER_STATES_GDATA.target_state < PEER_STATES_GDATA.current_state)
        {   /* Suspend relay stream */
            peerAdvanceRelayState(RELAY_EVENT_SUSPEND);
        }
        break;
    default:
        /* Do nothing as we are waiting for an action to complete */
        break;
    }
}


/*************************************************************************
NAME    
    peerUpdateRequiredRelayState
    
DESCRIPTION
    Used to update the Peer state machine required state.  This should be called when any event occurs
    that would cause a change to the required streaming state or source.
    
    This forms one of the two functions that should be used to drive the Peer state machine.

RETURNS
    None
    
**************************************************************************/
void peerUpdateRequiredRelayState (void)
{
    determineRequiredState();
    updateTargetRelayState();
    kickPeerStateMachine();
}

/*************************************************************************
NAME    
    peerGetPeerSink
    
DESCRIPTION
    Obtains sink to relay channel

RETURNS
    Handle to relay channel, NULL otherwise
    
**************************************************************************/
Sink peerGetPeerSink (void)
{
    if (PEER_STATES_GDATA.current_state >= RELAY_STATE_OPEN)
    {
        uint16 peer_id;
        
        if (a2dpGetPeerIndex(&peer_id))
        {
            return A2dpMediaGetSink(getA2dpLinkDataDeviceId(peer_id), getA2dpLinkDataStreamId(peer_id));
        }
    }
    
    return (Sink)NULL;
}


/*************************************************************************
NAME    
    peerGetSourceSink
    
DESCRIPTION
    Obtains sink of the current source

RETURNS
    Handle to sink if there is a current streaming source, NULL otherwise
    
**************************************************************************/
Sink peerGetSourceSink (void)
{
    Sink sink = (Sink)NULL;

    switch (PEER_STATES_GDATA.current_source)
    {
    case RELAY_SOURCE_NULL:
        break;
        
    case RELAY_SOURCE_ANALOGUE:
        sink = analogGetAudioSink();
        break;
        
    case RELAY_SOURCE_USB:
        sink = usbGetAudioSink();
        break;
        
    case RELAY_SOURCE_A2DP:
        sink = a2dpGetSourceSink();
        break;
    }
    
    return sink;
}


/*************************************************************************
NAME    
    peerIsRelayAvailable
    
DESCRIPTION
    Determines if relay channel is available for use

RETURNS
    TRUE if relay available, FALSE otherwise
    
**************************************************************************/
bool peerIsRelayAvailable (void)
{
    uint16 peer_id;
    
    if (a2dpGetPeerIndex(&peer_id))
    {
        if ( !(getA2dpPeerStatus(peer_id, REMOTE_PEER) & (PEER_STATUS_IN_CALL | PEER_STATUS_DONT_RELAY | PEER_STATUS_OWNS_RELAY)) &&   /* Has remote peer requested that relay not be used? */
             !(getA2dpPeerStatus(peer_id, LOCAL_PEER) & (PEER_STATUS_IN_CALL | PEER_STATUS_DONT_RELAY)) &&    /* Does local device not want relay to be used? */
              ((getA2dpLinkDataSeId(peer_id) == 0) || (getA2dpLinkDataSeId(peer_id) & SOURCE_SEID_MASK)) &&   /* Either no SEP or a source SEP has been configured */
              (getA2dpPeerRemoteFeatures(peer_id) & remote_features_peer_sink) )                              /* Peer supports a sink role */
        {
            PEER_SM_DEBUG(("PEER: RelayAvailable[TRUE]\n"));
            return TRUE;
        }
    }
    
    PEER_SM_DEBUG(("PEER: RelayAvailable[FALSE]\n"));
    return FALSE;
}

/*************************************************************************
NAME    
    peerCheckSource
    
DESCRIPTION
    Determines if the current peer source is the same as the new_source.

RETURNS
    TRUE if the current peer source is the same as new_source or if it is RELAY_SOURCE_NULL, FALSE otherwise
    
**************************************************************************/
bool peerCheckSource(RelaySource new_source)
{
    if((peerTwsSource() != RELAY_SOURCE_NULL) && (peerTwsSource() != new_source))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*************************************************************************
NAME    
    peerUpdateLocalStatusChange
    
DESCRIPTION
    Updates the local relay availability and issues a status change to a connected Peer

RETURNS
    None
    
**************************************************************************/
void peerUpdateLocalStatusChange (PeerStatusChange peer_status_change)
{
    uint16 peer_id;
        
    PEER_DEBUG(("peerUpdateLocalStatusChange status=0x%X\n",peer_status_change));
    
    if (a2dpGetPeerIndex(&peer_id))
    {
        setLocalRelayAvailability( peer_id, peer_status_change);
        PEER_UPDATE_REQUIRED_RELAY_STATE("LOCAL STATUS CHANGED");
        peerUpdateTWSMuteState(); /* update mute state, if required */
    }
}

/*************************************************************************
NAME    
    peerUpdateLocalStatusChangeNoKick
    
DESCRIPTION
    Updates the local relay availability but doesn't kick the state machine
    which would cause additional actions to be taken

RETURNS
    None
    
**************************************************************************/
void peerUpdateLocalStatusChangeNoKick(PeerStatusChange peer_status_change)
{
    uint16 peer_id;
        
    PEER_DEBUG(("peerUpdateLocalStatusChangeNoKick status=0x%X\n",peer_status_change));
    
    if (a2dpGetPeerIndex(&peer_id))
    {
        setLocalRelayAvailability( peer_id, peer_status_change);
        
        updateTargetRelayState();
    }
}

/*************************************************************************
NAME    
    peerHandleStatusChangeCmd
    
DESCRIPTION
    Handles a relay availability status change from a Peer

RETURNS
    None
    
**************************************************************************/
void peerHandleStatusChangeCmd (PeerStatusChange peer_status_change)
{
    uint16 peer_id;
        
    PEER_DEBUG(("peerHandleStatusChangeCmd status=0x%X\n",peer_status_change));
    
    if (a2dpGetPeerIndex(&peer_id))
    {
        a2dp_index_t index;
        PeerStatus local_peer_status = getA2dpPeerStatus(peer_id, LOCAL_PEER);

        setRemoteRelayAvailability( peer_id, peer_status_change);
        /* If not in single device mode, we need to flag the link as available locally,
         * but not propagate to the remote device */
        if(!peerIsSingleDeviceOperationEnabled() && findCurrentA2dpSource(&index)
            && (index!=peer_id) && (peer_status_change==PEER_STATUS_CHANGE_RELAY_AVAILABLE))
        {
            setRelayAvailability( &local_peer_status, peer_status_change );
            setA2dpPeerStatus(peer_id, LOCAL_PEER, local_peer_status);
        }
        PEER_UPDATE_REQUIRED_RELAY_STATE("REMOTE STATUS CHANGED");
        peerUpdateTWSMuteState(); /* update mute state, if required */
    }
}


/*************************************************************************
NAME    
    peerHandleAudioRoutingCmd
    
DESCRIPTION
    Handles an audio routing notification from a Peer

RETURNS
    None
    
**************************************************************************/
void peerHandleAudioRoutingCmd (PeerTwsAudioRouting master_routing_mode, PeerTwsAudioRouting slave_routing_mode)
{
    uint16 peer_id;

    if (a2dpGetPeerIndex(&peer_id))
    {
        PeerFixedAudioRouting fixed_routing_mode = peerGetFixedRoutingMode();

        /* If the device was configured for a fixed audio routing, just use that instead. */
        if (fixed_routing_mode == PEER_FIXED_ROUTING_LEFT)
        {
            master_routing_mode = PEER_TWS_ROUTING_LEFT;
            slave_routing_mode = PEER_TWS_ROUTING_RIGHT;
        }
        else if (fixed_routing_mode == PEER_FIXED_ROUTING_RIGHT)
        {
            master_routing_mode = PEER_TWS_ROUTING_RIGHT;
            slave_routing_mode = PEER_TWS_ROUTING_LEFT;
        }
    
        PEER_DEBUG(("peerHandleAudioRoutingCmd master=%u slave=%u\n",
                    master_routing_mode,
                    slave_routing_mode));
        
        setTWSRoutingMode(tws_master, master_routing_mode);
        setTWSRoutingMode(tws_slave, slave_routing_mode);

        deviceManagerUpdateAttributes(getA2dpLinkBdAddr(peer_id), sink_tws, 0, 0);

        /* update the TWS audio routing indications */
        peerSendAudioRoutingInformationToUser();

        AudioSetTwsChannelModes(sinkAudioGetRoutedAudioTask(), master_routing_mode, slave_routing_mode);
    }
}


/*************************************************************************
NAME    
    peerHandleVolumeLowBattCmd
    
DESCRIPTION
    Handle a volume change notification from a Peer

RETURNS
    None
    
**************************************************************************/
void peerHandleVolumeLowBattCmd (uint8 volume_limit)
{
    uint16 peer_id;
        
    PEER_DEBUG(("peerHandleVolumeLowBattCmd  VolLimit=%u\n",volume_limit));
    if (a2dpGetPeerIndex(&peer_id))
    {        
        bool limitVolume = (volume_limit > 0);
        /* apply max volume limit on this device to match the remote peer */
        powerManagerSetPeerSlaveLowBat(limitVolume);
        /* apply or remove volume limit if applicable */
        powerManagerLowGasGaugeAction(EventInvalid);
    }
}

/*************************************************************************
NAME    
    peerHandleAncEvent

DESCRIPTION
    Handle an ANC change notification from a Peer

RETURNS
    None

**************************************************************************/
void peerHandleAncEvent (uint8 ancReceivedEvent)
{
	PeerAncEventType ancEvent = (PeerAncEventType)ancReceivedEvent;

    PEER_DEBUG(("peerHandleAncEvent  ancEvent=%u\n",ancReceivedEvent));

    switch(ancEvent)
    {
	    case ANC_ENABLE:
	    	sinkAncEnable();
	    	break;
          
	    case ANC_DISABLE:
	    	sinkAncDisable();
	    	break;
          
          case ANC_SET_MODE1:
              sinkAncSetMode(anc_mode_1);
              break;
           
           case ANC_SET_MODE2:
              sinkAncSetMode(anc_mode_2);
              break;
    
          case ANC_SET_MODE3:
              sinkAncSetMode(anc_mode_3);
              break;
           
           case ANC_SET_MODE4:
              sinkAncSetMode(anc_mode_4);
              break;
    
          case ANC_SET_MODE5:
              sinkAncSetMode(anc_mode_5);
              break;
           
           case ANC_SET_MODE6:
              sinkAncSetMode(anc_mode_6);
              break;
    
          case ANC_SET_MODE7:
              sinkAncSetMode(anc_mode_7);
              break;
           
           case ANC_SET_MODE8:
              sinkAncSetMode(anc_mode_8);
              break;
    
          case ANC_SET_MODE9:
              sinkAncSetMode(anc_mode_9);
              break;
           
           case ANC_SET_MODE10:
              sinkAncSetMode(anc_mode_10);
              break;
    
           default:
	    	break;
    }
}

/*************************************************************************
NAME
    peerUpdateMasterTrimVolume
    
DESCRIPTION
    Handle a trim volume change notification from a Peer

RETURNS
    None
    
**************************************************************************/

void peerUpdateTrimVolume(PeerTrimVolChangeCmd cmd)
{

    switch(cmd)
    {
        case PEER_TRIM_VOL_MASTER_UP:
            MessageSend(&theSink.task , EventUsrMasterDeviceTrimVolumeUp , 0);
            break;

        case PEER_TRIM_VOL_MASTER_DOWN:
            MessageSend(&theSink.task , EventUsrMasterDeviceTrimVolumeDown , 0);
            break;

        case PEER_TRIM_VOL_SLAVE_UP:
            MessageSend(&theSink.task , EventUsrSlaveDeviceTrimVolumeUp , 0);
            break;

        case PEER_TRIM_VOL_SLAVE_DOWN:
            MessageSend(&theSink.task , EventUsrSlaveDeviceTrimVolumeDown , 0);
            break;
            
        default:
            break;
        }
}


/*************************************************************************
NAME    
    peerUpdateMasterTrimVolume
    
DESCRIPTION
   Sends the appropriate trim volume change command to the master peer device.

RETURNS
    None
    
**************************************************************************/
#ifdef PEER_TWS
static void sendDeviceTrimUpdateToTwsMaster(volume_direction dir, tws_device_type tws_device)
{
    /* Audio routed from a Peer device which supports TWS as a Source.  This takes priority over ShareMe and thus means we are in a TWS session */
    PeerTrimVolChangeCmd cmd = PEER_TRIM_VOL_NO_CHANGE;
    PEER_DEBUG(("sendDeviceTrimUpdateToTwsMaster %x %x\n", dir, tws_device));
    if (tws_device == tws_master)
    {
        switch (dir)
        {
        case increase_volume:
            cmd = PEER_TRIM_VOL_MASTER_UP;
            break;

        case decrease_volume:
            cmd = PEER_TRIM_VOL_MASTER_DOWN;
            break;

        case same_volume:
        default:
            cmd = PEER_TRIM_VOL_NO_CHANGE;
            break;
        }                            
    }
    else if (tws_device == tws_slave)
    {
        switch (dir)
        {
        case increase_volume:
            cmd = PEER_TRIM_VOL_SLAVE_UP;
            break;
        
        case decrease_volume:
            cmd = PEER_TRIM_VOL_SLAVE_DOWN;
            break;
        
        case same_volume:
        default:
            cmd = PEER_TRIM_VOL_NO_CHANGE;
            break;
        }                            
    }
    if(cmd != PEER_TRIM_VOL_NO_CHANGE) 
    {
        uint16 avrcp_index;
        
        if ( avrcpGetPeerIndex(&avrcp_index) )
        {
            sinkAvrcpVendorUniquePassthroughRequest(avrcp_index , AVRCP_PEER_CMD_UPDATE_TRIM_VOLUME,
                                                    AVRCP_PAYLOAD_PEER_CMD_UPDATE_TRIM_VOLUME, (const uint8 *)&cmd);
        }
    }
}
#endif

/*************************************************************************
NAME    
    peerHandleRemoteAGConnected
    
DESCRIPTION
    Checks if the sink device is already connected to an AG with the same bd address as the one connected to the peer,
    if yes, then disconnect the local AG connected if the bd addr of the sink device is lower than that of peer.

RETURNS
    None
    
**************************************************************************/
void peerHandleRemoteAgConnected(void)
{    
    if(peerGetRemotePeerAudioConnectionStatus() & A2DP_AUDIO)
    {
        uint16 avA2dpId;
        uint16 peerA2dpId;
        /* check if the bd address of the local AG is the same as the AG connected to the peer */
        if(a2dpGetPeerIndex (&peerA2dpId) && a2dpGetSourceIndex(&avA2dpId) && 
            BdaddrIsSame(peerGetRemotePeerAgBdAddress(), getA2dpLinkBdAddr(avA2dpId)))
        {
            sinkAvrcpDisconnect(getA2dpLinkBdAddr(avA2dpId));
            A2dpSignallingDisconnectRequest(getA2dpLinkDataDeviceId(avA2dpId));
            sinkDisconnectAllSlc();
        }
    }
}

/*************************************************************************
NAME    
    compareBdAddr
    
DESCRIPTION
    Compares the first and the second bdaddr.
RETURNS
    TRUE if the first is greater than second, otherwise return FALSE.
    
**************************************************************************/
bool peerCompareBdAddr(const bdaddr* first , const bdaddr* second)
{
    if( first->nap == second->nap)
    {
        if(first->uap == second->uap)
        {
            if(first->lap == second->lap)
            {
                return FALSE;
            }
            else
            {
                if(first->lap > second->lap)
                {
                    return TRUE;
                }
            }
        }
        else
        {
            if(first->uap > second->uap)
            {
                return TRUE;
            }
        }
    }
    else
    {
        if(first->nap > second->nap)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static bool populateAudioEnhancementsParams(uint8 * audio_enhancement_params)
{
    if(audio_enhancement_params)
    {
        sinkMusicProcessingWriteToMsgPayload(audio_enhancement_params);
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
NAME    
    peerSendAudioEnhancements
    
DESCRIPTION
    Sends audio enhancement setting to currently connected Peer

RETURNS
    None
    
**************************************************************************/
void peerSendAudioEnhancements(void)
{
    if(peerIsThisDeviceTwsMaster())
    {
        uint16 peerAvrcpIndex;
        if(avrcpGetPeerIndex(&peerAvrcpIndex))
        {
            uint8 audio_enhancements_array[MUSIC_PROCESSING_PAYLOAD_SIZE];
            if(populateAudioEnhancementsParams(audio_enhancements_array))
            {
                sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex , AVRCP_PEER_CMD_UPDATE_AUDIO_ENHANCEMENT_SETTINGS,
                                                    sizeof(audio_enhancements_array), audio_enhancements_array);
            }
        }
    }
}


/*************************************************************************
NAME    
    peerSendEqSettings
    
DESCRIPTION
    Sends DSP EQ setting to currently connected Peer

RETURNS
    None
    
**************************************************************************/
void peerSendUserEqSettings(void)
{
    if(peerIsThisDeviceTwsMaster())
    {
        uint16 peerAvrcpIndex;

        if(avrcpGetPeerIndex(&peerAvrcpIndex))
        {
            uint8 user_eq_params_array[USER_EQ_PARAMS_SIZE];
            if(sinkAudioPeerGetPEQ(user_eq_params_array))
            {
                sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex , AVRCP_PEER_CMD_UPDATE_USER_EQ_SETTINGS,
                            sizeof(user_eq_params_array), user_eq_params_array);
            }
        }
    }
}


/*************************************************************************
NAME    
    peerRequestUserEqSetings
    
DESCRIPTION
    Request current DSP EQ setting from the connected Peer (Master)

RETURNS
    None
    
**************************************************************************/
void peerRequestUserEqSetings(void)
{
    uint16 peerA2dpId;
    a2dp_index_t index;
    
    if(a2dpGetPeerIndex(&peerA2dpId))
    {    
        /* Request the current user eq settings from the peer if this device is a TWS Slave*/
        if(findCurrentA2dpSource(&index) && (getA2dpPeerRemoteDevice(index) == remote_device_peer))
        {
            uint16 peerAvrcpIndex;
            for_all_avrcp(peerAvrcpIndex)
            {
                if((sinkAvrcpIsInitialised() && BdaddrIsSame(getA2dpLinkBdAddr(peerA2dpId), sinkAvrcpGetLinkBdAddr(peerAvrcpIndex))))
                {
                    sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex , AVRCP_PEER_CMD_REQUEST_USER_EQ_SETTINGS, 0 , NULL);
                }
            }
        }
    }
}

/*************************************************************************
NAME    
    peerConnectPeer
    
DESCRIPTION
    Attempts to connect a TWS Peer, if not already in a Peer session

RETURNS
    TRUE if a connection is attempted, FALSE otherwise
    
**************************************************************************/
bool peerConnectPeer (void)
{
    if (!a2dpGetPeerIndex(NULL))
    {
        typed_bdaddr  peer_addr;
        sink_attributes peer_attributes;    
        uint16 list_idx;
        uint16 list_size = ConnectionTrustedDeviceListSize();

        for (list_idx = 0; list_idx < list_size; list_idx++)
        {
            /* attempt to obtain the device attributes for the current ListID required */
            if (deviceManagerGetIndexedAttributes(list_idx, &peer_attributes, &peer_addr))
            {
                if ((peer_attributes.peer_device == remote_device_peer) && 
                    (peer_attributes.peer_features & (remote_features_tws_a2dp_sink | remote_features_tws_a2dp_source)))
                {
                    /* Only attempt to connect a TWS Peer */
                    slcConnectDevice(&peer_addr.addr, peer_attributes.profiles);
                    return TRUE;
                }
            }
        }
    }
    
    return FALSE;
}

/*************************************************************************
NAME    
    peerObtainPairingMode
    
DESCRIPTION
    Obtains the pairing mode used for the currently connected Peer device

RETURNS
    None
    
**************************************************************************/
PeerPairingMode peerObtainPairingMode (uint16 peer_id)
{
    PeerPairingMode pairing_mode = PEER_PAIRING_MODE_TEMPORARY;   /* By default, devices use temporary pairing */
    
    if (peerIsTWSPeer(peer_id))
    {   /* If remote Peer supports TWS then use the configured TWS pairing mode */
        pairing_mode = peerTwsPairingMode();
        PEER_DEBUG(("PEER: TWS Pairing Mode = %u\n",pairing_mode));
    }
    else 
    {
        if (getA2dpPeerRemoteFeatures(peer_id) & (remote_features_shareme_a2dp_sink | remote_features_shareme_a2dp_source))
        {   /* If remote Peer supports ShareMe and not TWS then use the configured ShareMe pairing mode */
            pairing_mode = peerShareMePairingMode();
            PEER_DEBUG(("PEER: ShareMe Pairing Mode = %u\n",pairing_mode));
        }
    }
    
    return pairing_mode;
}


/*************************************************************************
NAME    
    peerIsTWSPeer
    
DESCRIPTION
   Determines whether given Remote peer_id has a TWS Features or not (i.e is Audio Share)

RETURNS
    None
    
**************************************************************************/
bool peerIsTWSPeer(uint16 peer_id)
{
    return (getA2dpPeerRemoteFeatures(peer_id) & (remote_features_tws_a2dp_sink | remote_features_tws_a2dp_source)) != 0;
}

/*************************************************************************
NAME    
    peerUpdatePairing
    
DESCRIPTION
    Ensures permanent pairing data is updated

RETURNS
    None
    
**************************************************************************/
void peerUpdatePairing (uint16 peer_id, const sink_attributes *peer_attributes)
{
    switch (peerObtainPairingMode(peer_id))
    {
    case PEER_PAIRING_MODE_TEMPORARY:
    case PEER_PAIRING_MODE_NORMAL:
        break;
    case PEER_PAIRING_MODE_PERMANENT:
        /* Permanent pairing is enabled for Peer devices */
        PEER_DEBUG(("PEER: Add/update permanent pairing for Peer %u\n", peer_id));
        AuthUpdatePermanentPairing(getA2dpLinkBdAddr(peer_id), peer_attributes);
        break;
    }
}

/*************************************************************************
NAME    
    peerIsThisDeviceTwsMaster
    
DESCRIPTION
    Is this device a streaming TWS Master

RETURNS
    None
    
**************************************************************************/
#ifdef PEER_TWS
bool peerIsThisDeviceTwsMaster(void)
{
    uint16 peerA2dpId=0;
    
    if(a2dpGetPeerIndex(&peerA2dpId))
    {   
        /* Are we a streaming TWS master? */
        if((getA2dpLinkDataSeId(peerA2dpId) & (SOURCE_SEID_MASK | TWS_SEID_MASK)) == (SOURCE_SEID_MASK | TWS_SEID_MASK))
        {   
            /* A streaming TWS Master */
            return TRUE;
        }
    }
    
    return FALSE;
}
#endif

/*************************************************************************
NAME
    isThisDeviceTwsSlave

DESCRIPTION
    Is this device a streaming TWS slave

RETURNS
    None

**************************************************************************/
#ifdef PEER_TWS
bool peerIsThisDeviceTwsSlave(void)
{
    bool is_routed_tws = FALSE;
    if(sinkA2dpAudioPrimaryOrSecondarySinkMatch(sinkAudioGetRoutedAudioSink()))
    {
        uint16 a2dp_index;

        if(getA2dpIndexFromSink(sinkAudioGetRoutedAudioSink(), &a2dp_index) && a2dpIsIndexPeer(a2dp_index)
                && (getA2dpPeerRemoteFeatures(a2dp_index) & remote_features_tws_a2dp_source))
        {
            is_routed_tws = TRUE;
        }
    }
    return is_routed_tws;
}
#endif


/*************************************************************************
NAME    
    peerIsPeerStreaming
    
DESCRIPTION
    does this device currently have a Peer streaming connection?

RETURNS
    TRUE or FALSE
    
**************************************************************************/
bool peerIsPeerStreaming(void)
{
    return(PEER_STATES_GDATA.current_state == RELAY_STATE_STREAMING);
}

/*************************************************************************
NAME    
    peerGetLocalSupportedCodecs

DESCRIPTION
    Returns the list of optional codecs supported by the local peer device

RETURNS
    TRUE if the list is present othewise FALSE
    
**************************************************************************/
bool peerGetLocalSupportedCodecs(uint16 *codecs)
{
    if(SinkA2dpIsInitialised())
    {
        if(codecs)
        {
            *codecs = getA2dpPeerOptionalCodecs(LOCAL_PEER);
        }
        
        return TRUE;
    }  
    
    return FALSE;
}


/*************************************************************************
NAME    
    peerGetRemoteSupportedCodecs
    
DESCRIPTION
    Returns the list of optional codecs supported by the remote peer device

RETURNS
    TRUE if the list is present othewise FALSE

    
**************************************************************************/
bool peerGetRemoteSupportedCodecs(uint16 *codecs)
{
    if(SinkA2dpIsInitialised())
    {
        if(codecs)
        {
            *codecs = getA2dpPeerOptionalCodecs(REMOTE_PEER);  
        }
        
        return TRUE;
    }  
    
    return FALSE;
}



/*************************************************************************
NAME    
    peerSetLocalPeerSupportedCodecs
    
DESCRIPTION
    Stores the list of optional codecs supported by the local peer device

RETURNS
    None
    
**************************************************************************/
void peerSetLocalSupportedCodecs(uint16 codecs)
{
    if(SinkA2dpIsInitialised())
    {
        setA2dpPeerOptionalCodecs(LOCAL_PEER, codecs);
    }
}

/*************************************************************************
NAME    
    peerSetRemoteSupportedCodecs
    
DESCRIPTION
    Stores the list of optional codecs supported by the remote peer device

RETURNS
    None
    
**************************************************************************/
void peerSetRemoteSupportedCodecs(uint16 codecs)
{    
    PEER_DEBUG(("PEER: peerSetRemoteSupportedCodecs 0x%x\n", codecs));
    
    if(SinkA2dpIsInitialised())
    {
        setA2dpPeerOptionalCodecs(REMOTE_PEER, codecs);
    }
}

/****************************************************************************
NAME    
    peerLinkReservedCanDeviceConnect
    
DESCRIPTION
    Determine whether the given device can be connected based on whether we have reserved a link 
    for a peer device to connect.

RETURNS
    TRUE or FALSE
*/ 
bool peerLinkReservedCanDeviceConnect(const bdaddr *bd_addr)
{
    sink_attributes attributes;
    
    if(!peerGetLinkReserved())
    {
        /* We have not reserved a link for peer device to connect so just return TRUE*/
        return TRUE;
    }
    
    /* is there a connected peer device? */
    if(deviceManagerNumConnectedPeerDevs())
    {
        /* peer device already connected, allow passed in device to connect */
        return TRUE;
    }
    /* no current connected peer device */
    else
    {
       
        /* check passed in device, if a peer device then allow it to connect */
        if((deviceManagerGetAttributes(&attributes, bd_addr))&&(attributes.peer_device == remote_device_peer))
        {
            /* passed device is peer, allow to connect */
            return TRUE;
        }
        /* passed in device is not peer, its an AG, check if there is a free connection slot or
           the device already has a profile connected and this is the request for additional profiles in which
           case allow the connection to continue */
        else if(deviceManagerNumOtherConnectedDevs(bd_addr) == 0)
        {
            /* there are no connected AG's so allow to connect or this device already has other profiles connected */
            return TRUE;
        }
        /* passed in device is not a peer and an AG is already connected, refuse more AG connections */
        else
            return FALSE;                           
    }
}


/*************************************************************************
NAME    
    peerUpdateLocalStatusOnCallEnd
    
DESCRIPTION
    Responsible for updating the relay state when a call ends. The function
    call handles the scenarios when the device is in Single Device Mode (SDM)
    or Non Single Device Mode.

RETURNS
    None
    
**************************************************************************/
void peerUpdateLocalStatusOnCallEnd(void)
{
    a2dp_index_t index;
        
    /* When the call has ended/inactive/idle: Check if in NON single device mode AND whether it is connected to an AG */
    if( !peerIsSingleDeviceOperationEnabled() && findCurrentStreamingA2dpSource(&index) && !a2dpIsIndexPeer(index) )
    {
        /* mark the relay as unavailable */
        peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_CALL_INACTIVE | PEER_STATUS_CHANGE_RELAY_UNAVAILABLE);
    }
    else
    {
        /* in single device mode operation, mark the relay as available */
        peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_CALL_INACTIVE);
    }
}


/*************************************************************************
NAME
    peerUpdateBatteryLevel

DESCRIPTION
    This function is used by sink_avrcp to update the peer battery level when
    a AVRCP_PEER_CMD_UPDATE_BATTERY_LEVEL message is received from a connected
    peer.

RETURNS
    TRUE on success

**************************************************************************/
#ifdef ENABLE_PEER_BATTERY_LEVEL
bool peerUpdateBatteryLevel(uint16 battery_level)
{
    PEER_DEBUG(("peerUpdateBatteryLevel %d\n", battery_level));
    PEER_GDATA.peer_battery_level = battery_level;
    return TRUE;
}
#endif

/*************************************************************************
NAME
    peerGetBatteryLevel

DESCRIPTION
    This function can be used to retrive the cached battery level of connected
    peer device.

RETURNS
    Battery level in percentage
    BATTERY_LEVEL_INVALID on error

**************************************************************************/
#ifdef ENABLE_PEER_BATTERY_LEVEL
uint8 peerGetBatteryLevel(void)
{
    return PEER_GDATA.peer_battery_level;
}
#endif

/****************************************************************************
NAME    
    getPeerDSPBufferingRequired
    
DESCRIPTION
    function to deteremine whether TWS backwards compatibility mode within the 
    DSP is required, TWS version 3.1.0 uses an internal delay of 350ms to allow
    the use of larger buffers, this is incompatible with earlier versions of TWS
    which only use a 250ms internal delay, therefore to maintain backwards
    compatibility with older versions of TWS it is necessary to send a configuration
    message to the DSP to select the internal delay, STD or backwards compatibility
    required.
    
RETURNS
    peer_buffer_level
*/
peer_buffer_level getPeerDSPBufferingRequired(uint16 remote_peer_version)
{
 
    /* check for older version of a peer device supporting lower buffering levels */                   
    if((remote_peer_version) && (remote_peer_version < PEER_DEVICE_MIN_SUPPORTED_BUFFER_COMPATIBILITY_VERSION))
    {
        /* reported as an older version of TWS, therefore backwards comaptiblity
           is required */
        return PEER_BUFFER_250MS ;
    }
    /* check for newer version of a peer device supporting higher buffering levels */                   
    else if(remote_peer_version)
    {
        /* peer device whose version is 3.1.0 or higher supporting higher buffering rates */
        return PEER_BUFFER_350MS;
    }
    else
    {
        /* no backwards compatiblity is required since this is not a peer device */
        return PEER_BUFFER_NON_PEER_DEVICE;
    }
}

/*************************************************************************
NAME    
    peerAvrcpUpdateActiveConnection
    
DESCRIPTION
    Updates the active AVRCP connection based on what is currently connected.
   
RETURNS
    TRUE if the supplied AVRCP connection index is that of Peer device

**************************************************************************/
bool peerAvrcpUpdateActiveConnection(uint8 active_avrcp)
{
    if(sinkAvrcpIsAvrcpIndexPeer(active_avrcp, NULL))
    { 
        /* If there is a peer device connected then set it as being an active_avrcp 
        device only if TWS Single device operation is enabled and has A2DP or USB connected */ 
        if(peerIsSingleDeviceOperationEnabled() && (peerGetRemotePeerAudioConnectionStatus() & (A2DP_AUDIO | USB_AUDIO)))
        {
            sinkAvrcpSetActiveConnection(active_avrcp);
        }
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME    
    peerIsA2dpAudioConnected
    
DESCRIPTION
    Sends an EventSysSetActiveAvrcpConnection, lUpdateMessage.

RETURNS
    TRUE if have found a peer device with A2DP audio connected, else FALSE

**************************************************************************/
bool peerIsA2dpAudioConnected(uint8 Index)
{
    if(!a2dpIsIndexPeer(Index) || (peerGetRemotePeerAudioConnectionStatus() & (A2DP_AUDIO | USB_AUDIO)))
    {
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
NAME
    peerUpdateRelaySource

DESCRIPTION
    Function to match the source selected with the relevant Peer source
    attribute which is essential to be updated along with the Peer
    state machine.

INPUTS
    Audio Source selection.

RETURNS
    None

*/
void peerUpdateRelaySource(uint16 nextSource)
{
    switch((audio_sources)nextSource)
    {
        case audio_source_FM:
        case audio_source_ANALOG:
        case audio_source_SPDIF:
        case audio_source_I2S:
            peerSetPeerSource(RELAY_SOURCE_ANALOGUE);
            updateAudioGating();
            PEER_UPDATE_REQUIRED_RELAY_STATE("SELECT ANALOGUE");
            break;

        case audio_source_USB:
            peerSetPeerSource(RELAY_SOURCE_USB);
            updateAudioGating();
            PEER_UPDATE_REQUIRED_RELAY_STATE("SELECT USB");
            break;

        case audio_source_a2dp_1:
        case audio_source_a2dp_2:
            peerSetPeerSource(RELAY_SOURCE_A2DP);
            updateAudioGating();
            PEER_UPDATE_REQUIRED_RELAY_STATE("SELECT A2DP");
            break;

        case audio_source_none:
            peerSetPeerSource(RELAY_SOURCE_NULL);
            PEER_UPDATE_REQUIRED_RELAY_STATE("NO SOURCE SELECTED");
            break;

        case audio_source_ba:
            /* BA audio source is not supported in PEER mode, Ignore the BA source*/
            break;

        default:
            Panic();
            break;
    }
}

/*************************************************************************
NAME
    peerIsDeviceSlaveAndStreaming

DESCRIPTION
    Function call which confirms if the device is in Slave role and a Peer
    is connected and currently streaming.

INPUTS
    None

RETURNS
    TRUE if device is not a Master and peer is connected and streaming,
    else FALSE.

*/
bool peerIsDeviceSlaveAndStreaming(void)
{
    uint16 peerA2dpId;

    return ( (!peerIsThisDeviceTwsMaster()) && (a2dpGetPeerIndex(&peerA2dpId)) ) ? TRUE : FALSE;
}

/****************************************************************************
NAME
    peerRelayAvailableSources

DESCRIPTION
    Helper function to eliminate the routing of disabled audio sources from
    streaming to Peer in TWS mode.

RETURNS
    TRUE if source is enabled for Peer streaming, FALSE otherwise.

*/
static bool peerRelayAvailableSources(audio_sources routed_source)
{
    switch(routed_source)
    {
        case audio_source_none:
        case audio_source_end_of_list:
            return FALSE;

        case audio_source_FM:
            return (IS_FM_SOURCE_AVAILABLE_TO_PEER);

        case audio_source_ANALOG:
            return (IS_ANALOGUE_SOURCE_AVAILABLE_TO_PEER);

        case audio_source_SPDIF:
            return (IS_SPDIF_SOURCE_AVAILABLE_TO_PEER);

        case audio_source_I2S:
            return (IS_I2S_SOURCE_AVAILABLE_TO_PEER);

        case audio_source_USB:
            return (IS_USB_SOURCE_AVAILABLE_TO_PEER);

        case audio_source_a2dp_1:
            return (IS_A2DP1_SOURCE_AVAILABLE_TO_PEER);

        case audio_source_a2dp_2:
            return (IS_A2DP2_SOURCE_AVAILABLE_TO_PEER);

        case audio_source_ba:
            return (IS_BA_SOURCE_AVAILABLE_TO_PEER);
    }

    return FALSE;
}

/****************************************************************************
NAME
    peerGetForwardingMode

DESCRIPTION
    When qualifying TWS we must send outgoing packets with RTP headers and not
    TWS headers. Select the correct mode depending on whether qualification has
    been enabled
*/
static audio_plugin_output_mode_t peerGetForwardingMode(void)
{
#ifdef PEER_TWS
    if(peerTwsQualificationEnabled())
        return audio_plugin_output_mode_share_me;
    
    return audio_plugin_output_mode_tws;
#else
    return audio_plugin_output_mode_share_me;
#endif
}


/****************************************************************************
NAME    
    peerAudioUpdateTargetLatency
    
DESCRIPTION
    Update target latency when the use case changes (non-TWS -> TWS or vice versa)
    
RETURNS
*/
#ifdef UPDATE_TARGET_LATENCY_ON_TWS_CHANGE
static void peerAudioUpdateTargetLatency(void)
{
    uint16 peer_id;

    if (a2dpGetPeerIndex(&peer_id))
    {
        a2dp_codec_settings *source_codec_settings = A2dpCodecGetSettings(getA2dpLinkDataDeviceId(peer_id), getA2dpLinkDataStreamId(peer_id));
        A2dpPluginConnectParams *audioConnectParams = getAudioPluginConnectParams();
             
        if (source_codec_settings != NULL)
        {             
            if (audioConnectParams->latency.target != audioPriorLatency(source_codec_settings->seid) / LATENCY_TARGET_MULTIPLIER)
            {
                audioConnectParams->latency.target = audioPriorLatency(source_codec_settings->seid) / LATENCY_TARGET_MULTIPLIER;
                PEER_DEBUG(("AUD: target latency: %d codec settings seid %d \n", 
                            (uint16)audioConnectParams->latency.target * LATENCY_TARGET_MULTIPLIER, source_codec_settings->seid));
                AudioUpdateLatencySettings();
            }    
            free(source_codec_settings);
        }
    }
}
#else
#define peerAudioUpdateTargetLatency() ((void)0)
#endif /* UPDATE_TARGET_LATENCY_ON_TWS_CHANGE */

/****************************************************************************
NAME
    peerRelaySourceStream

DESCRIPTION
    Checks for a Peer relay (source) stream and asks DSP to relay audio from any active AV Source

RETURNS
    true if audio relayed, false otherwise
*/
bool peerRelaySourceStream (void)
{
    bool audio_relaying = FALSE;
    
    if(peerRelayAvailableSources(sinkAudioGetRoutedAudioSource()))
    {
        if (!isAudioGated(audio_gate_relay) && (peerCurrentRelaySource() != RELAY_SOURCE_NULL) && (peerCurrentRelayState()  >= RELAY_STATE_STARTING))
        {   /* We have an active relay stream, thus we must be wanting to forward audio data to a Peer */
            Sink av_sink;
            Sink peer_sink;

            /* Obtain Source and Peer sinks while doing some sanity checking */
            if (((peer_sink = peerGetPeerSink())!=NULL) && ((av_sink = peerGetSourceSink())!=NULL) && (sinkAudioGetRoutedAudioSink() == av_sink))
            {   /* Outbound forwarding stream will only be connected if main inbound stream is routed */
                uint16 peer_idx;

                if (a2dpGetPeerIndex(&peer_idx))
                {
                    if(!IsAudioRelaying())
                    {
                        a2dp_codec_settings *codec_settings = A2dpCodecGetSettings(getA2dpLinkDataDeviceId(peer_idx), getA2dpLinkDataStreamId(peer_idx));

                        if (codec_settings)
                        {
                            bool content_protection = (codec_settings->codecData.content_protection != avdtp_no_protection);
                            Task input_plugin = getRelayPlugin(codec_settings->seid);
                            Task output_plugin = AudioPluginOutputGetTwsPluginTask(peerGetForwardingMode());
                            if (peerTwsQualificationEnabled())
                            {
                                AudioConfigSetPeerMtu(codec_settings->codecData.packet_size);
                            }
                            peerAudioUpdateTargetLatency();
                            PEER_DEBUG(("PEER: Start forwarding sink 0x%X seid=0x%X\n", (unsigned)peer_sink, codec_settings->seid )) ;
                            AudioStartForwarding(input_plugin, peer_sink, content_protection, getPeerDSPRequiredBufferingLevel(peer_idx), output_plugin);
                            free(codec_settings);
                        }
                    }

                    audio_relaying = TRUE;
                }
            }
        }
    }
    if(sinkAudioIsAudioRouted() && !audio_relaying)
    {
        PEER_DEBUG(("PEER: Stop forwarding\n")) ;
        peerAudioUpdateTargetLatency();
        AudioStopForwarding();
    }

    return audio_relaying;
}

/****************************************************************************
NAME
    getRelayPlugin

DESCRIPTION
    Return relay plugin task. Method of getting task depends which type of audio plugins is in use.

RETURNS
    Relay plugin task
*/
static Task getRelayPlugin(uint8 seid)
{
    Task relay_plugin;

    if(AudioPluginMusicVariantsCodecDeterminesTwsEncoding())
    {
        relay_plugin = getA2dpPlugin(seid);
    }
    else
    {
        relay_plugin = sinkAudioGetRoutedAudioTask();
    }

    return relay_plugin;
}

bool peerIsRemotePeerInCall(void)
{
    uint16 peer_id;
    if(a2dpGetPeerIndex(&peer_id) && (getA2dpPeerStatus(peer_id, REMOTE_PEER) & PEER_STATUS_IN_CALL))
    {
        return TRUE;
    }
    return FALSE;
}

#ifdef PEER_TWS

void peerPopulatePluginConnectData(AudioPluginFeatures * features, uint16 sample_rate)
{
    A2dpPluginConnectParams *audio_connect_params = getAudioPluginConnectParams();
    features->audio_input_routing = AUDIO_ROUTE_INTERNAL_AND_RELAY;

    switch (sample_rate)
    {
        case 16000:
            audio_connect_params->bitpool = 53;
            audio_connect_params->format = 0x3F;    /* 16Khz, 16 blocks, Joint Stereo, SNR, 8 subbands */
            break;
        case 32000:
            audio_connect_params->bitpool = 53;
            audio_connect_params->format = 0x7F;    /* 32Khz, 16 blocks, Joint Stereo, SNR, 8 subbands */
            break;
        case 44100:
            audio_connect_params->bitpool = 53;
            audio_connect_params->format = 0xBF;    /* 44.1Khz, 16 blocks, Joint Stereo, SNR, 8 subbands */
            break;
        case 48000:
            audio_connect_params->bitpool = 51;
            audio_connect_params->format = 0xFF;    /* 48Khz, 16 blocks, Joint Stereo, SNR, 8 subbands */
            break;
        default:
            /* Unsupported rate */
            break;
    }

#ifdef SBC_ENCODER_CONFORMANCE
    {
        audio_connect_params->format = SinkSbcGetAudioConnectFormat();
        
        /*Use max bitpool as configured bitpool from Source*/
        audio_connect_params->bitpool = SinkSbcGetMaxBitpool();
    }
#endif

    audio_connect_params->peer_is_available = peerCanThisTwsDeviceRelayRequestedAudio();
}

static void peerSendTimeStampedPeerEvent(const uint8 * event_data, uint16 length)
{
    uint16 peerAvrcpIndex;

    if(avrcpGetPeerIndex (&peerAvrcpIndex) )
    {
        sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex , AVRCP_PEER_CMD_TIME_STAMPED_COMMAND,
                                                        length, event_data);
    }
}

/* Peer synchronised events transmit */

static bool areSynchronisedEventsSupportedByConnectedPeer(void)
{
    uint16 source_id;
    return (a2dpGetPeerIndex(&source_id)
                && (getA2dpLinkDataPeerVersion(source_id) >= PEER_DEVICE_SYNCHRONISED_EVENTS_MINIMUM_SUPPORTED_VERSION));
}

static bool areSychronisedEventsSupported(void)
{
    if(twsSynchronisedControlIsEnabled())
    {
        return areSynchronisedEventsSupportedByConnectedPeer();
    }
    return FALSE;
}

static void scheduleSynchronisedEvent(synchronised_peer_event_t peer_event,
                    uint8 * peer_event_payload, unsigned peer_event_payload_length)
{
    uint8 total_length = (TIMESTAMP_LENGTH + SYNCHRONISED_PEER_EVENT_ID_LENGTH + peer_event_payload_length);
    uint8 * timestamped_event = mallocPanic(total_length);
    tws_timestamp_t timestamp = twsSynchronisedControlGetFutureTimestamp(SYNCHRONISED_COMMAND_LATENCY_IN_MILLISECONDS);

    memcpy(timestamped_event, (uint8 *)&timestamp, TIMESTAMP_LENGTH);
    timestamped_event[TIMESTAMP_LENGTH] = peer_event;
    memcpy(&timestamped_event[TIMESTAMP_LENGTH + SYNCHRONISED_PEER_EVENT_ID_LENGTH], peer_event_payload, peer_event_payload_length);

    peerSendTimeStampedPeerEvent((const uint8 *)timestamped_event, total_length);
    free(timestamped_event);
}

/* Peer volume event */

static void scheduleSynchronisedPeerVolumeEvent(void)
{
    uint8 volume_payload[SYNCHRONISED_PEER_EVENT_UPDATE_VOLUME_PAYLOAD_SIZE];
    scaled_volume_percentage_t volume = sinkVolumeGetCurrentVolumeAsScaledPercentage();

    PEER_DEBUG(("PEER: Send sync volume %d\n", volume));
    
    volume_payload[SCALED_VOLUME_PERCENTAGE_LO_OFFSET] = LOBYTE(volume);
    volume_payload[SCALED_VOLUME_PERCENTAGE_HI_OFFSET] = HIBYTE(volume);

    scheduleSynchronisedEvent(SYNCHRONISED_PEER_EVENT_UPDATE_VOLUME,
                                            volume_payload, sizeof(volume_payload));
                                            
    MessageCancelFirst(&theSink.task, EventSysPeerSelfGeneratedUpdateMainVolume);
    MessageSendLater(&theSink.task, EventSysPeerSelfGeneratedUpdateMainVolume, NULL, SYNCHRONISED_COMMAND_LATENCY_IN_MILLISECONDS);
}

static MessageId processReceivedSynchronisedPeerVolumeEvent(uint8 * payload)
{
    scaled_volume_percentage_t volume = MAKEWORD(payload[SCALED_VOLUME_PERCENTAGE_LO_OFFSET],
            payload[SCALED_VOLUME_PERCENTAGE_HI_OFFSET]);
    
    PEER_DEBUG(("PEER: Apply sync volume %d\n", volume));
    
    return sinkVolumeSetCurrentVolumeAsScaledPercentage(volume);
}

/* Peer trim event */

static void scheduleSynchronisedPeerTrimEvent(void)
{
    uint8 peer_trim_payload[SYNCHRONISED_PEER_EVENT_UPDATE_TRIM_PAYLOAD_SIZE];
    PEER_DEBUG(("scheduleSynchronisedPeerTrimEvent %x\n", peerGetTwsSlaveTrim()));
    peer_trim_payload[PEER_TRIM_LOW_OFFSET] = LOBYTE(peerGetTwsSlaveTrim());
    peer_trim_payload[PEER_TRIM_HIGH_OFFSET] = HIBYTE(peerGetTwsSlaveTrim());

    scheduleSynchronisedEvent(SYNCHRONISED_PEER_EVENT_UPDATE_TRIM,
            peer_trim_payload, sizeof(peer_trim_payload));

    MessageCancelFirst(&theSink.task, EventSysPeerSelfGeneratedUpdateMainVolume);
    MessageSendLater(&theSink.task, EventSysPeerSelfGeneratedUpdateMainVolume, NULL, SYNCHRONISED_COMMAND_LATENCY_IN_MILLISECONDS);
}

static MessageId processReceivedSynchronisedPeerTrimEvent(uint8 * payload)
{
    int16 peer_trim = MAKEWORD(payload[PEER_TRIM_LOW_OFFSET],
            payload[PEER_TRIM_HIGH_OFFSET]);
    PEER_DEBUG(("processReceivedSynchronisedPeerTrimEvent %x\n", peer_trim));
    setPeerTwsSlaveTrim(peer_trim);

    return EventSysPeerRemoteGeneratedUpdateMainVolume;
}

/* Peer mute event */

static bool getMuteStateFromUsrEvent(MessageId id)
{
    return ((id == EventUsrMainOutMuteOn) ? TRUE : FALSE);
}

static MessageId getPeerGeneratedMessageIdFromMuteEvent(MessageId mute_event)
{
    MessageId id = EventInvalid;
    if(mute_event == EventUsrMainOutMuteOn)
    {
        id = EventSysPeerGeneratedMainMuteOn;
    }
    else if(mute_event == EventUsrMainOutMuteOff)
    {
        id = EventSysPeerGeneratedMainMuteOff;
    }
    return id;
}

static void scheduleSynchronisedPeerMuteEvent(MessageId id)
{
    uint8 mute_payload[SYNCHRONISED_PEER_EVENT_MUTE_PAYLOAD_SIZE];
    mute_payload[0] = (uint8)getMuteStateFromUsrEvent(id);

    scheduleSynchronisedEvent(SYNCHRONISED_PEER_EVENT_MUTE,
                                            mute_payload, sizeof(mute_payload));
    MessageSendLater(&theSink.task, getPeerGeneratedMessageIdFromMuteEvent(id), NULL, SYNCHRONISED_COMMAND_LATENCY_IN_MILLISECONDS);
}

static void scheduleUnsynchronisedPeerMuteEvent(MessageId id)
{
    peerUpdatePeerMute(getMuteStateFromUsrEvent(id));
    MessageSend(&theSink.task, getPeerGeneratedMessageIdFromMuteEvent(id), NULL);
}

static MessageId processReceivedSynchronisedPeerMuteEvent(uint8 * payload)
{
    return (payload[0] ? EventSysPeerGeneratedMainMuteOn : EventSysPeerGeneratedMainMuteOff);
}


/* Peer power event */

static bool getPowerStateFromUsrEvent(MessageId id)
{
    return ((id == EventUsrPowerOff) ? FALSE : TRUE);
}

static MessageId getPeerGeneratedMessageIdFromPowerEvent(MessageId power_event)
{
    return ((power_event == EventUsrPowerOff) ? EventSysPeerGeneratedPowerOff : EventInvalid);
}

static void scheduleSynchronisedPeerPowerEvent(MessageId id)
{
    uint8 power_payload[SYNCHRONISED_PEER_EVENT_POWER_PAYLOAD_SIZE];
    power_payload[0] = (uint8)getPowerStateFromUsrEvent(id);

    scheduleSynchronisedEvent(SYNCHRONISED_PEER_EVENT_POWER,
                            power_payload, sizeof(power_payload));
    MessageSendLater(&theSink.task, getPeerGeneratedMessageIdFromPowerEvent(id), NULL, SYNCHRONISED_COMMAND_LATENCY_IN_MILLISECONDS);
}

static void scheduleUnsynchronisedPeerPowerEvent(MessageId id)
{
    if(id == EventUsrPowerOff)
    {
        sinkAvrcpPeerPowerOff();
    }
}

static MessageId processReceivedSynchronisedPeerPowerEvent(uint8 * payload)
{
    return (payload[0] ? EventInvalid : EventSysPeerGeneratedPowerOff);
}


/* Peer update audio enhancements event */

static MessageId getPeerGeneratedMessageIdFromAudioEnhancementEvent(MessageId audio_enhancement_event)
{
    return ((audio_enhancement_event == EventSysUpdateAudioEnhancements) ?
                        EventSysPeerGeneratedUpdateAudioEnhancements : EventInvalid);
}

static void scheduleSynchronisedPeerUpdateAudioEnhancementsEvent(MessageId id)
{
    uint8 audio_enhancements_params_array[SYNCHRONISED_PEER_EVENT_UPDATE_AUDO_ENHANCEMENTS_PAYLOAD_SIZE];

    if(populateAudioEnhancementsParams(audio_enhancements_params_array))
    {
        scheduleSynchronisedEvent(SYNCHRONISED_PEER_EVENT_UPDATE_AUDIO_ENHANCEMENTS,
                audio_enhancements_params_array, sizeof(audio_enhancements_params_array));
        MessageSendLater(&theSink.task, getPeerGeneratedMessageIdFromAudioEnhancementEvent(id), NULL,
                                        SYNCHRONISED_COMMAND_LATENCY_IN_MILLISECONDS);
    }
}

static void scheduleUnsynchronisedPeerUpdateAudioEnhancementsEvent(MessageId id)
{
    peerSendAudioEnhancements();
    MessageSend(&theSink.task, getPeerGeneratedMessageIdFromAudioEnhancementEvent(id), NULL);
}

static MessageId processReceivedSynchronisedPeerUpdateAudioEnhancementsEvent(uint8 * payload)
{
    sinkMusicProcessingStoreFromMsgPayload(payload);
    return EventSysPeerGeneratedUpdateAudioEnhancements;
}


/* Peer update EQ event */

static MessageId getPeerGeneratedMessageIdFromUpdateEqEvent(MessageId update_eq_event)
{
    return ((update_eq_event == EventSysUpdateUserEq) ?
            EventSysPeerGeneratedUpdateUserEq : EventInvalid);
}

static void scheduleSynchronisedPeerUpdateEqEvent(MessageId id)
{
    uint8 user_eq_params_array[SYNCHRONISED_PEER_EVENT_UPDATE_EQ_PAYLOAD_SIZE];

    if(sinkAudioPeerGetPEQ(user_eq_params_array))
    {
        scheduleSynchronisedEvent(SYNCHRONISED_PEER_EVENT_UPDATE_EQ,
                            user_eq_params_array, sizeof(user_eq_params_array));
        MessageSendLater(&theSink.task, getPeerGeneratedMessageIdFromUpdateEqEvent(id), NULL,
                                        SYNCHRONISED_COMMAND_LATENCY_IN_MILLISECONDS);
    }
}

static void scheduleUnsynchronisedPeerUpdateEqEvent(MessageId id)
{
    peerSendUserEqSettings();
    MessageSend(&theSink.task, getPeerGeneratedMessageIdFromUpdateEqEvent(id), NULL);
}

static MessageId processReceivedSynchronisedPeerUpdateEqEvent(uint8 * payload)
{
    sinkAudioPeerUpdatePEQ(payload);
    return EventSysPeerGeneratedUpdateUserEq;
}

/* Peer update ANC event */
static PeerAncEventType getAncEventPayload(MessageId anc_event)
{
	PeerAncEventType peerAncEvent;

    switch(anc_event)
    {
        case EventUsrAncOn:
        	peerAncEvent = ANC_ENABLE;
        	break;

        case EventUsrAncOff:
        	peerAncEvent = ANC_DISABLE;
        	break;

        case EventUsrAncMode1:
            peerAncEvent = ANC_SET_MODE1;
            break;

        case EventUsrAncMode2:
            peerAncEvent = ANC_SET_MODE2;
            break;

        case EventUsrAncMode3:
            peerAncEvent = ANC_SET_MODE3;
            break;

        case EventUsrAncMode4:
            peerAncEvent = ANC_SET_MODE4;
            break;

        case EventUsrAncMode5:
            peerAncEvent = ANC_SET_MODE5;
            break;

        case EventUsrAncMode6:
            peerAncEvent = ANC_SET_MODE6;
            break;

        case EventUsrAncMode7:
            peerAncEvent = ANC_SET_MODE7;
            break;

        case EventUsrAncMode8:
            peerAncEvent = ANC_SET_MODE8;
            break;

        case EventUsrAncMode9:
            peerAncEvent = ANC_SET_MODE9;
            break;

        case EventUsrAncMode10:
            peerAncEvent = ANC_SET_MODE10;
            break;

        default:
        	peerAncEvent = ANC_DISABLE;
        	break;
    }

    return peerAncEvent;
}

static MessageId getPeerGeneratedMessageIdFromAncEvent(MessageId anc_event)
{
	MessageId PeerGeneratedEvent;

    switch(anc_event)
    {
        case EventUsrAncOn:
        	PeerGeneratedEvent = EventSysPeerGeneratedAncOn;
        	break;

        case EventUsrAncOff:
        	PeerGeneratedEvent = EventSysPeerGeneratedAncOff;
        	break;

        case EventUsrAncMode1:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode1;
            break;

        case EventUsrAncMode2:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode2;
            break;

        case EventUsrAncMode3:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode3;
            break;

        case EventUsrAncMode4:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode4;
            break;

        case EventUsrAncMode5:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode5;
            break;

        case EventUsrAncMode6:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode6;
            break;

        case EventUsrAncMode7:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode7;
            break;

        case EventUsrAncMode8:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode8;
            break;

        case EventUsrAncMode9:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode9;
            break;

        case EventUsrAncMode10:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode10;
            break;

        default:
        	PeerGeneratedEvent = EventInvalid;
        	break;
    }

    return PeerGeneratedEvent;
}

static void peerSendUnsynchronisedAncEvent(MessageId id)
{
	PeerAncEventType ancEvent;

	ancEvent = getAncEventPayload(id);

    {
        uint16 peerAvrcpIndex;

        if(avrcpGetPeerIndex(&peerAvrcpIndex))
        {
            sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex,
                                                    AVRCP_PEER_CMD_UPDATE_ANC_EVENT,
                                                    AVRCP_PAYLOAD_PEER_CMD_UPDATE_ANC_EVENT,
													(const uint8 *)&ancEvent );
        }
    }
}

static void peerSendSynchronisedAncEvent(MessageId id)
{
    uint8 anc_payload[SYNCHRONISED_PEER_EVENT_ANC_PAYLOAD_SIZE];

    anc_payload[0] = (uint8)getAncEventPayload(id);

    scheduleSynchronisedEvent(SYNCHRONISED_PEER_EVENT_UPDATE_ANC_EVENT,
	    	                  anc_payload, sizeof(anc_payload));
}

void peerSendAncState(void)
{
	MessageId ancEvent;

	ancEvent = ((sinkAncIsEnabled()) ? EventUsrAncOn : EventUsrAncOff);

	if(areSychronisedEventsSupported())
    {
		peerSendSynchronisedAncEvent(ancEvent);
    }
    else
    {
        peerSendUnsynchronisedAncEvent(ancEvent);
    }
}

void peerSendAncMode(void)
{
    MessageId ancEvent = sinkAncGetUsrEventFromAncMode(sinkAncGetMode());

    if(areSychronisedEventsSupported())
    {
		peerSendSynchronisedAncEvent(ancEvent);
    }
    else
    {
    	peerSendUnsynchronisedAncEvent(ancEvent);
    }
}

static void scheduleSynchronisedPeerUpdateAncEvent(MessageId id)
{
	peerSendSynchronisedAncEvent(id);
    MessageSendLater(&theSink.task, getPeerGeneratedMessageIdFromAncEvent(id), NULL,
                                        SYNCHRONISED_COMMAND_LATENCY_IN_MILLISECONDS);
}

static void scheduleUnsynchronisedPeerUpdateAncEvent(MessageId id)
{
	peerSendUnsynchronisedAncEvent(id);
	MessageSend(&theSink.task, getPeerGeneratedMessageIdFromAncEvent(id), NULL);
}

static MessageId getNextAncEvent(MessageId id)
{
	if (id == EventUsrAncToggleOnOff)
	{
	    id = sinkAncGetNextState();
	}

	if (id == EventUsrAncNextMode)
	{
        id = sinkAncGetUsrEventFromAncMode(sinkAncGetNextMode(sinkAncGetMode()));
	}

	return id;
}

static MessageId processReceivedSynchronisedPeerAncEvent(uint8 * payload)
{
	MessageId PeerGeneratedEvent;

    switch(payload[0])
    {
        case ANC_ENABLE:
        	PeerGeneratedEvent = EventSysPeerGeneratedAncOn;
        	break;
        case ANC_DISABLE:
        	PeerGeneratedEvent = EventSysPeerGeneratedAncOff;
        	break;
            
        case ANC_SET_MODE1:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode1;
            break;

        case ANC_SET_MODE2:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode2;
            break;

        case ANC_SET_MODE3:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode3;
            break;

        case ANC_SET_MODE4:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode4;
            break;

        case ANC_SET_MODE5:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode5;
            break;

        case ANC_SET_MODE6:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode6;
            break;

        case ANC_SET_MODE7:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode7;
            break;

        case ANC_SET_MODE8:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode8;
            break;

        case ANC_SET_MODE9:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode9;
            break;

        case ANC_SET_MODE10:
            PeerGeneratedEvent = EventSysPeerGeneratedAncMode10;
            break;
        default:
        	PeerGeneratedEvent = EventInvalid;
        	break;
	}

    return PeerGeneratedEvent;
}

/* Peer synchronised events receive */

static MessageId processReceivedSynchronisedPeerEvent(synchronised_peer_event_t peer_event, uint8 * payload)
{
    MessageId id = EventInvalid;
    switch(peer_event)
    {
        case SYNCHRONISED_PEER_EVENT_MUTE:
            id = processReceivedSynchronisedPeerMuteEvent(payload);
            break;
        case SYNCHRONISED_PEER_EVENT_POWER:
            id = processReceivedSynchronisedPeerPowerEvent(payload);
            break;
        case SYNCHRONISED_PEER_EVENT_UPDATE_EQ:
            id = processReceivedSynchronisedPeerUpdateEqEvent(payload);
            break;
        case SYNCHRONISED_PEER_EVENT_UPDATE_AUDIO_ENHANCEMENTS:
            id = processReceivedSynchronisedPeerUpdateAudioEnhancementsEvent(payload);
            break;
        case SYNCHRONISED_PEER_EVENT_UPDATE_VOLUME:
            id = processReceivedSynchronisedPeerVolumeEvent(payload);
            break;
        case SYNCHRONISED_PEER_EVENT_UPDATE_TRIM:
            id = processReceivedSynchronisedPeerTrimEvent(payload);
            break;
        case SYNCHRONISED_PEER_EVENT_UPDATE_ANC_EVENT:
        	id = processReceivedSynchronisedPeerAncEvent(payload);
    }
    return id;
}

void peerReceiveSychronisedEvent(uint8 * event_data)
{
    tws_timestamp_t timestamp;
    synchronised_peer_event_t peer_event = event_data[TIMESTAMP_LENGTH];
    MessageId id = processReceivedSynchronisedPeerEvent(peer_event, &event_data[TIMESTAMP_LENGTH +1]);
    memcpy((uint8 *)&timestamp, event_data, TIMESTAMP_LENGTH);
    PEER_DEBUG(("peerReceiveSychronisedEvent %x %ld\n", id, twsSynchronisedControlConvertTimeStampToMilliseconds(timestamp)));
    MessageSendLater(&theSink.task, id, NULL, twsSynchronisedControlConvertTimeStampToMilliseconds(timestamp));
}

/* TWS trim events */

static void setAndSendDeviceTrimUpdateToSlave(const volume_direction direction, const tws_device_type tws_device)
{
    PEER_DEBUG(("setAndSendDeviceTrimUpdateToSlave %x %x\n", direction, tws_device));
    peerUpdateTWSDeviceTrim(direction, tws_device);
    if(peerTrimsAreHandledThroughSynchronisedEvents())
    {
        scheduleSynchronisedPeerTrimEvent();
    }
    else
    {
        PEER_DEBUG(("setAndSendDeviceTrimUpdateToSlave update\n"));
        sinkVolumeModifyAndUpdateRoutedAudioMainVolume(same_volume);
    }
}

static void handleTwsTrimEventIfAudioRouted(const volume_direction direction, const tws_device_type tws_device)
{
    PEER_DEBUG(("handleTwsTrimEventIfAudioRouted %x %x\n", direction, tws_device));
    if(sinkAudioIsAudioRouted())
    {
        if(peerIsThisDeviceTwsSlave() && avrcpAvrcpIsEnabled())
        {
            sendDeviceTrimUpdateToTwsMaster(direction, tws_device);
        }
        else
        {
            setAndSendDeviceTrimUpdateToSlave(direction, tws_device);
        }
    }
}

static void processTwsTrimEvent(MessageId id)
{
    volume_direction trim_direction = increase_volume;
    tws_device_type tws_device = tws_master;

    if(id == EventUsrSlaveDeviceTrimVolumeUp || id == EventUsrSlaveDeviceTrimVolumeDown)
    {
        tws_device = tws_slave;
}

    if(id == EventUsrMasterDeviceTrimVolumeDown || id == EventUsrSlaveDeviceTrimVolumeDown)
    {
        trim_direction = decrease_volume;
    }
    PEER_DEBUG(("processTwsTrimEvent %x\n", id));
    handleTwsTrimEventIfAudioRouted(trim_direction, tws_device);
}

/* Peer specific Usr/Sys event handler */

static bool isLocalPeerPoweringOff(void)
{
    uint16 a2dp_peer_index;
    if(a2dpGetPeerIndex(&a2dp_peer_index))
    {
        if(getA2dpPeerStatus(a2dp_peer_index, LOCAL_PEER) & PEER_STATUS_POWER_OFF)
        {
            return TRUE;
        }
    }
    return FALSE;
}

bool peerProcessEvent(MessageId id)
{
    bool event_complete = FALSE;
    uint16 avrcp_peer_index;
    PEER_DEBUG(("peerProcessEvent %x\n", id));
    if(peerIsSingleDeviceOperationEnabled() && avrcpGetPeerIndex(&avrcp_peer_index))
    {
        switch(id)
        {
            case EventUsrMainOutMuteOn:
            case EventUsrMainOutMuteOff:
                if(areSychronisedEventsSupported())
                {
                    scheduleSynchronisedPeerMuteEvent(id);
                }
                else
                {
                    scheduleUnsynchronisedPeerMuteEvent(id);
                }
                event_complete = TRUE;
                break;

            case EventSysUpdateMainVolume:
                if(areSychronisedEventsSupported())
                {
                    scheduleSynchronisedPeerVolumeEvent();
                    event_complete = TRUE;
                }
                break;

            case EventUsrPowerOff:
                if (peerGetSingleDeviceMode() == PEER_SINGLE_DEVICE_MODE_FULL)
                {
                    uint16 a2dp_peer_index;
                    if(a2dpGetPeerIndex(&a2dp_peer_index))
                    {
                        if(!isLocalPeerPoweringOff())
                        {
                            PeerStatus localPeerStatus = getA2dpPeerStatus(a2dp_peer_index, LOCAL_PEER);
                            localPeerStatus |= PEER_STATUS_POWER_OFF;
                            setA2dpPeerStatus(a2dp_peer_index, LOCAL_PEER, localPeerStatus);
                            if(areSychronisedEventsSupported())
                            {
                                scheduleSynchronisedPeerPowerEvent(id);
                            }
                            else
                            {
                                scheduleUnsynchronisedPeerPowerEvent(id);
                            }
                            event_complete = TRUE;
                        }
                    }
                }
                break;

            case EventSysUpdateUserEq:
                if(areSychronisedEventsSupported())
                {
                    scheduleSynchronisedPeerUpdateEqEvent(id);
                }
                else
                {
                    scheduleUnsynchronisedPeerUpdateEqEvent(id);
                }
                event_complete = TRUE;
                break;

            case EventSysUpdateAudioEnhancements:
                if(areSychronisedEventsSupported())
                {
                    scheduleSynchronisedPeerUpdateAudioEnhancementsEvent(id);
                }
                else
                {
                    scheduleUnsynchronisedPeerUpdateAudioEnhancementsEvent(id);
                }
                event_complete = TRUE;
                break;

            case EventUsrSelectNextUserEQBank:
                if(peerSlaveRequestEqNextBank())
                {
                    event_complete = TRUE;
                }
                break;

            case EventUsrUserEqOnOffToggle:
                if(peerSlaveRequestEqEnableChange(EQ_TOGGLE))
                {
                    event_complete = TRUE;
                }
                break;

            case EventUsrUserEqOn:
                if(peerSlaveRequestEqEnableChange(EQ_ON))
                {
                    event_complete = TRUE;
                }
                break;

            case EventUsrUserEqOff:
                if(peerSlaveRequestEqEnableChange(EQ_OFF))
                {
                    event_complete = TRUE;
                }
                break;

            case EventUsrMasterDeviceTrimVolumeUp:
            case EventUsrMasterDeviceTrimVolumeDown:
            case EventUsrSlaveDeviceTrimVolumeUp:
            case EventUsrSlaveDeviceTrimVolumeDown:
                processTwsTrimEvent(id);
                event_complete = TRUE;
                break;

            case EventUsrChangeAudioRouting:
                event_complete = peerChangeAudioRouting();
                break;
                
            case EventUsrDetermineTwsForcedDownmixMode:
                event_complete = PeerDetermineTwsForcedDownmixMode();
                break;

            case EventUsrAncOn:
            case EventUsrAncOff:
            case EventUsrAncMode1:
            case EventUsrAncMode2:
            case EventUsrAncMode3:
            case EventUsrAncMode4:
            case EventUsrAncMode5:
            case EventUsrAncMode6:
            case EventUsrAncMode7:
            case EventUsrAncMode8:
            case EventUsrAncMode9:
            case EventUsrAncMode10:
            case EventUsrAncToggleOnOff:
            case EventUsrAncNextMode:
            	id = getNextAncEvent(id);
            	if(areSychronisedEventsSupported())
            	{
            		scheduleSynchronisedPeerUpdateAncEvent(id);
            	}
            	else
            	{
                    scheduleUnsynchronisedPeerUpdateAncEvent(id);
            	}
                event_complete = TRUE;
                break;
        }
    }
    return event_complete;
}

MessageId peerConvertPeerGeneratedEventToUsrEvent(MessageId original_id)
{
    MessageId converted_id = original_id;
    switch(original_id)
    {
        case EventSysPeerGeneratedMainMuteOn:
            converted_id = EventUsrMainOutMuteOn;
            break;
        case EventSysPeerGeneratedMainMuteOff:
            converted_id = EventUsrMainOutMuteOff;
            break;
        case EventSysPeerGeneratedPowerOff:
            converted_id = EventUsrPowerOff;
            break;
        case EventSysPeerGeneratedUpdateUserEq:
            converted_id = EventSysUpdateUserEq;
            break;
        case EventSysPeerGeneratedUpdateAudioEnhancements:
            converted_id = EventSysUpdateAudioEnhancements;
            break;
        default:
            break;
    }
    return converted_id;
}


/* Peer AVRCP connect/disconnect */

void peerAvrcpConnect(void)
{
    uint16 peer_avrcp_index;
    if(avrcpGetPeerIndex(&peer_avrcp_index))
    {
        twsSynchronisedControlEnable(AvrcpGetSink(sinkAvrcpGetProfileInstance(peer_avrcp_index)));
    }
    else
    {
        Panic();
    }
}

void peerAvrcpDisconnect(void)
{
    twsSynchronisedControlDisable();
}

#endif

/*************************************************************************
NAME    
    peerPurgeTemporaryPairing
    
DESCRIPTION
    If in Temporary Pairing mode, remove any peer devices from the PDL
    
RETURNS
    None

**************************************************************************/
void peerPurgeTemporaryPairing(void)
{
    if (peerTwsPairingMode() == PEER_PAIRING_MODE_TEMPORARY)
    {
        uint8 index = 0;
        sink_attributes attributes;
        typed_bdaddr dev_addr;
        
        while (deviceManagerGetIndexedAttributes(index, &attributes, &dev_addr))
        {
            if (attributes.peer_device == remote_device_peer)
            {
                PEER_DEBUG(("PEER: purge temp %04x %02x %06lx\n",
                            dev_addr.addr.nap,
                            dev_addr.addr.uap,
                            dev_addr.addr.lap));
                
                deviceManagerRemoveDevice(&dev_addr.addr);
            /*  Later entries move up the list so don't increment index */
            }
            else
            {
                ++index;
            }
        }
    }
}

/*************************************************************************
NAME    
    peerSendAudioRoutingInformationToUser
    
DESCRIPTION
    Indicate the audio channel being used in this PEER device, i.e. LEFT or RIGHT

RETURNS
    None
    
**************************************************************************/
void peerSendAudioRoutingInformationToUser(void)
{
    /* ensure a2dp configuration is valid */
    if(SinkA2dpIsInitialised())
    {
        uint16 peerA2dpId;

        /* check whether peer is connected/available */
        if(a2dpGetPeerIndex(&peerA2dpId))
        {       
            /* determine if this device is a LEFT or RIGHT channel and generate a system event
               that can be used to drive LED indications for channel selection */
            /* confirm whether this device is master of the peer connection */
            if(LR_CURRENT_ROLE_MASTER == getA2dpLinkRole(peerA2dpId))
            {
                /* this device is using the master routing configuration */
                switch(getTWSRoutingMode(tws_master))
                {
                    /* fall-through for all the recognised options */
                    case PEER_TWS_ROUTING_LEFT:
                    case PEER_TWS_ROUTING_RIGHT:
                    case PEER_TWS_ROUTING_STEREO:
                    case PEER_TWS_ROUTING_DMIX:
                        MessageSend(&theSink.task, (EventSysPeerAudioRoutingStereo + getTWSRoutingMode(tws_master)), NULL);
                        break;
                    default:
                        /* unrecognised routing mode */
                        break;
                }
            }
            /* or device is a slave peer */
            else
            {
                /* this device is using the slave peer routing configuration */
                /* due to the fact that the master/slave routing modes become swapped over avrcp message from master,
                   due to supporting non single device modes, it is necessary to swap left and right routing information 
                   to show correct slave device routing info */
                switch(getTWSRoutingMode(tws_slave))
                {
                    case PEER_TWS_ROUTING_LEFT:
                        MessageSend(&theSink.task, EventSysPeerAudioRoutingRight, NULL);
                        break;

                    case PEER_TWS_ROUTING_RIGHT:
                        MessageSend(&theSink.task, EventSysPeerAudioRoutingLeft, NULL);
                        break;

                    case PEER_TWS_ROUTING_STEREO:
                    case PEER_TWS_ROUTING_DMIX:
                        /* either stereo or downmix */
                        MessageSend(&theSink.task, (EventSysPeerAudioRoutingStereo + getTWSRoutingMode(tws_slave)), NULL);
                        break;

                    default:
                        /* unrecognised routing mode */
                        break;
                }
            }
        }
    }
}

/*************************************************************************
NAME   
    peerUpdateAGConectionState
    
DESCRIPTION
    update the state of the AG connection of the remote peer to allow LED
    indications of separate peer and AG connection states 

RETURNS
    None
    
**************************************************************************/
void peerUpdateAGConectionState(bool ag_connected)
{
    MessageSend(&theSink.task, ag_connected ? EventSysPeerAgSourceConnected : EventSysPeerAllAgSourcesDisconnected, NULL);
}

/*************************************************************************
NAME  
    PeerIsShareMeSourceEnabled
    
DESCRIPTION
    Check status of Share Me source feature bit.

RETURNS
    TRUE if ShareMe source is enabled, FALSE otherwise

**************************************************************************/
bool PeerIsShareMeSourceEnabled(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->ShareMeSource;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return enabled;
}

/*************************************************************************
NAME    
    PeerIsTwsSourceEnabled
    
DESCRIPTION
    Check status of TWS source feature bit.

RETURNS
    TRUE if TWS source is enabled, FALSE otherwise

**************************************************************************/
bool PeerIsTwsSourceEnabled(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->TwsSource;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return enabled;
}

/*************************************************************************
NAME    
    PeerIsTwsSinkEnabled
    
DESCRIPTION
    Check status of TWS sink feature bit.

RETURNS
    TRUE if TWS sink is enabled, FALSE otherwise

**************************************************************************/
bool PeerIsTwsSinkEnabled(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->TwsSink;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return enabled;
}

/*************************************************************************
NAME    
    peerUseLiacForPairing
    
DESCRIPTION
   Determines if the peer pairing should use LIAC instead of the GIAC.

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerUseLiacForPairing(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->PeerUseLiac;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return enabled;
}

/*************************************************************************
NAME    
    peerUseDeviceIdRecord
    
DESCRIPTION
   Checks if peer devices will use the Device Id record to filter suitable devices.

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerUseDeviceIdRecord(void)
{
    return (PEER_GDATA.PeerUseDeviceId)?(TRUE):(FALSE);
}

/*************************************************************************
NAME    
    PeerGetConnectionPio
    
DESCRIPTION
    Returns the output PIO configured to indicate the peer connection state

RETURNS
    PIO number

**************************************************************************/
uint8 PeerGetConnectionPio(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    uint8 pio = PIN_INVALID;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        pio = read_data->PeerConnectionPio;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    
    return pio;
}
/*************************************************************************
NAME    
    PeerUseCsrUuid
    
DESCRIPTION
   Returns the status of the feature bit for using QTIL UUID.

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
static bool PeerUseCsrUuid(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->PeerUseQtilUuid;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return enabled;
}

/*************************************************************************
NAME    
    peerShareMePairingMode
    
DESCRIPTION
   Returns the configured pairing mode for ShareMe peers

RETURNS
    Peer pairing mode for share me peers

**************************************************************************/
static PeerPairingMode peerShareMePairingMode(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    PeerPairingMode pairing_mode = PEER_PAIRING_MODE_TEMPORARY;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        pairing_mode = read_data->ShareMePairingMode;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return pairing_mode;
}

/*************************************************************************
NAME    
    peerTwsPairingMode
    
DESCRIPTION
   Returns the configured pairing mode for TWS peers

RETURNS
    Peer pairing mode for TWS peers

**************************************************************************/
static PeerPairingMode peerTwsPairingMode(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    PeerPairingMode pairing_mode = PEER_PAIRING_MODE_TEMPORARY;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        pairing_mode = read_data->TwsPairingMode;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return pairing_mode;
}

/*************************************************************************
NAME    
    peerTwsSource
    
DESCRIPTION
   Returns the current relay source for TWS

RETURNS
    Relay cource for TWS

**************************************************************************/
static RelaySource peerTwsSource(void)
{
    return (PEER_GDATA.PeerSource);
}

/*************************************************************************
NAME    
    peerGetSingleDeviceMode
    
DESCRIPTION
    Returns AVRCP forwarding mode in TWS Single Device operation 

RETURNS
    PEER_SINGLE_DEVICE_MODE_DISABLED       Single Device Mode is not enabled
    PEER_SINGLE_DEVICE_MODE_FULL           Single Device Mode, Power Off forwarded
    PEER_SINGLE_DEVICE_MODE_NO_POWER_OFF   Single Device Mode, Power Off not forwarded

**************************************************************************/
PeerSingleDeviceMode peerGetSingleDeviceMode(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    PeerSingleDeviceMode mode = PEER_SINGLE_DEVICE_MODE_DISABLED;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        mode = read_data->TwsSingleDeviceOperation;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    
    return mode;
}

/*************************************************************************
NAME    
    peerIsSingleDeviceOperationEnabled
    
DESCRIPTION
    Determines whether devices in TWS session behave as a single unit for AVRCP operation 

RETURNS
	TRUE if have single device operation enabled, else FALSE

**************************************************************************/
bool peerIsSingleDeviceOperationEnabled(void)
{
    return peerGetSingleDeviceMode() != PEER_SINGLE_DEVICE_MODE_DISABLED;
}

/*************************************************************************
NAME    
    peerShareMeControlEnabled
    
DESCRIPTION
    Determines if AVRCP commands received from a ShareMe Peer are forwarded to an AV Source 

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerShareMeControlEnabled(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->ShareMePeerControlsSource;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return enabled;
}

/*************************************************************************
NAME    
    peerLinkRecoveryWhileStreamingEnabled
    
DESCRIPTION
    Determines if Master Peer device will attempt to page a Slave Peer during linkloss while streaming .

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerLinkRecoveryWhileStreamingEnabled(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->PeerLinkRecoveryWhileStreaming;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return enabled;    
}

/*************************************************************************
NAME    
    PeerReconnectAgOnPeerConnectionEnabled
    
DESCRIPTION
    Determines if BT source is to be disconnected and re-connected upon peer device connection.

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
bool PeerReconnectAgOnPeerConnectionEnabled(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->ReconnectAgOnPeerConnection;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return enabled;    
}

/*************************************************************************
NAME    
    peerGetPermittedRoutingModes
    
DESCRIPTION
    Returns the Permitted routing configurations on EventUsrChangeAudioRouting 

RETURNS
	Permitted routing configurations

**************************************************************************/
uint8 peerGetPermittedRoutingModes(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    uint8 routing_modes = 0;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        routing_modes = read_data->PeerPermittedRouting;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return routing_modes;
}

/*************************************************************************
NAME
    peerGetFixedRoutingMode

DESCRIPTION
    Returns the TWS Fixed Routing configuration

RETURNS
    Fixed routing configuration

**************************************************************************/
PeerFixedAudioRouting peerGetFixedRoutingMode(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    PeerFixedAudioRouting fixed_routing_mode = PEER_FIXED_ROUTING_NONE;

    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        fixed_routing_mode = read_data->FixedRoutingMode;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    
    return fixed_routing_mode;
}

/*************************************************************************
NAME    
    peerAllowedToEstablishConnection
    
DESCRIPTION
    Determines if the peer is allowed to establish connection

RETURNS
	TRUE if peer is allowed, FALSE otherwise

**************************************************************************/
static bool peerAllowedToEstablishConnection(void)
{
    return (PEER_GDATA.UnlockPeerStateMachine)?(TRUE):(FALSE);
}

/*************************************************************************
NAME    
    peerOpenRequestForQualificationEnabled
    
DESCRIPTION
    Returns the status of Flag which indicates App to trigger opening of Peer media channel 

RETURNS
	TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerOpenRequestForQualificationEnabled(void)
{
    return (PEER_GDATA.tws_qual_enable_peer_open)?(TRUE):(FALSE);
}

/*************************************************************************
NAME    
    peerTwsQualificationEnabled
    
DESCRIPTION
    Returns the status of TWS qualification feature bit

RETURNS
	TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerTwsQualificationEnabled(void)
{
    sink_peer_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->TwsQualificationEnable;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }
    return enabled;
}

/*************************************************************************
NAME
    sinkDataGetSourceSteamingDelay

DESCRIPTION
    Interfaces for getting source streaming delay value

PARAMS
    void

RETURNS
    void
*/
uint16 sinkDataGetSourceSteamingDelay(void)
{
    uint16 delay = 0;
    sink_peer_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINK_PEER_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        delay = read_configdata->TwsQualificationSourceStreamDelay;
        configManagerReleaseConfig(SINK_PEER_READONLY_CONFIG_BLK_ID);
    }

    return delay;
}

/****************************************************************************
NAME
    peerGetLinkReserved

DESCRIPTION
    Returns the status of Peer link reservation flag.

RETURNS
    TRUE if peer link is reserved, FALSE otherwise

**************************************************************************/
bool peerGetLinkReserved(void)
{
    sink_peer_writeable_config_def_t *writeable_data=NULL;
    bool reserved = FALSE;
    
    if (configManagerGetReadOnlyConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
        reserved = writeable_data->PeerLinkReserved;
        configManagerReleaseConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID);
    }
    return reserved;
}

/****************************************************************************
NAME
    peerSetLinkReserved

DESCRIPTION
    Turns on or off the feature to reserve a link for the peer device to connect.
    
**************************************************************************/
void peerSetLinkReserved(bool reserved)
{
    sink_peer_writeable_config_def_t *writeable_data=NULL;
        
    if (configManagerGetWriteableConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID, (void **)&writeable_data, 0))
    {
        writeable_data->PeerLinkReserved = reserved;
        configManagerUpdateWriteableConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID);
    }
    
    if(reserved)
    {   
        /* If the user has requested to reserve a link for the peer and both links are connected then the 
                 connection to non-gaia device will be dropped otherwise both the links are dropped*/
        deviceManagerDisconnectNonGaiaDevices();
    }
}

/****************************************************************************
NAME
    peerCurrentRelayState

DESCRIPTION
    Returns the Current Relay state.

RETURNS
    None

**************************************************************************/
RelayState peerCurrentRelayState(void)
{
    return PEER_STATES_GDATA.current_state;
}

/****************************************************************************
NAME
    peerCurrentRelaySource

DESCRIPTION
    Returns the Current Relay source.

RETURNS
    None

**************************************************************************/
RelaySource peerCurrentRelaySource(void)
{
    return PEER_STATES_GDATA.current_source;
}

/****************************************************************************
NAME
    peerTargetRelayState

DESCRIPTION
    Returns the Target Relay state.

RETURNS
    None

**************************************************************************/
RelayState peerTargetRelayState(void)
{
    return PEER_STATES_GDATA.target_state;
}

/****************************************************************************
NAME
    peerGetRemotePeerAgBdAddress

DESCRIPTION
    Returns the Remote Peer AG BD Address.

RETURNS
    bdaddr*

**************************************************************************/
bdaddr* peerGetRemotePeerAgBdAddress(void)
{
    return &PEER_GDATA.remote_peer_ag_bd_addr;
}

void peerSetRemotePeerAgBdAddress(audio_src_conn_state_t * new_state)
{
    PEER_GDATA.remote_peer_ag_bd_addr.lap = ((uint32)new_state->bd_addr.lap[2] << 16) | ((uint16)new_state->bd_addr.lap[1] << 8) | new_state->bd_addr.lap[0];
    PEER_GDATA.remote_peer_ag_bd_addr.uap = new_state->bd_addr.uap;   
    PEER_GDATA.remote_peer_ag_bd_addr.nap = (new_state->bd_addr.nap[1] << 8) | new_state->bd_addr.nap[0] ;
}

/****************************************************************************
NAME
    peerGetRemotePeerAudioConnectionStatus

DESCRIPTION
    Get the Remote Peer Audio Connection Status

RETURNS
    uint8

**************************************************************************/
uint8 peerGetRemotePeerAudioConnectionStatus(void)
{
    return PEER_GDATA.remote_peer_audio_conn_status;
}

/****************************************************************************
NAME
    peerSetRemotePeerAudioConnectionStatus

DESCRIPTION
    Set the Remote Peer Audio Connection Status

RETURNS
    void

**************************************************************************/
void peerSetRemotePeerAudioConnectionStatus(uint8 status)
{
    PEER_GDATA.remote_peer_audio_conn_status = status;
}

/****************************************************************************
NAME
    peerUpdateTWSDeviceTrim

DESCRIPTION
    Modifies the TWS device trims for master and slave to be used for volume updates

RETURNS
    None

**************************************************************************/
#ifdef PEER_TWS
static void peerUpdateTWSDeviceTrim(const volume_direction direction , const tws_device_type tws_device)
{
    int16 device_trim;
    sink_peer_writeable_config_def_t *writeable_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
        PEER_DEBUG(("PEER: sinkPeerUpdateTWSDeviceTrim [%i][%i][%i][%i][%i]\n", writeable_data->device_trim_master,
                                                                writeable_data->device_trim_slave, writeable_data->device_trim_change,
                                                                writeable_data->device_trim_min, writeable_data->device_trim_max));

        /*   device_trim_master and device_trim_slave are actually global data which has to be read at initialization
             and updated during run time but never makes it to the PS store, Can only be updated using GAIA Set cmd
             hence for in phase 2 we are operating on Global data.
        */       
        if (tws_device == tws_master)
        {
            device_trim = PEER_GDATA.device_trim_master;
        }
        else
        {
            device_trim = PEER_GDATA.device_trim_slave;
        }

        if (direction == increase_volume)
        {   /* Increase and clamp to upper limit */
            device_trim += writeable_data->device_trim_change;

            if (device_trim >= writeable_data->device_trim_max)
            {
                device_trim = writeable_data->device_trim_max;
                MessageSend ( &theSink.task , EventSysTrimVolumeMax , 0 );
            }

            PEER_DEBUG(("PEER: inc device trim [%i]\n", device_trim));

            if(tws_device == tws_master) 
                PEER_GDATA.device_trim_master = device_trim; /* peer_global_data.device_trim_master = device_trim */
            else
                PEER_GDATA.device_trim_slave = device_trim; /* peer_global_data.device_trim_slave = device_trim */
        }
        else if(direction == decrease_volume)
        {   /* Decrease and clamp to lower limit */
            device_trim -= writeable_data->device_trim_change;

            if (device_trim <= writeable_data->device_trim_min)
            {
                device_trim = writeable_data->device_trim_min;
                MessageSend ( &theSink.task , EventSysTrimVolumeMin , 0 );
            }

            PEER_DEBUG(("PEER: dec device trim [%i]\n", device_trim));

            if(tws_device == tws_master) 
                PEER_GDATA.device_trim_master = device_trim; /* peer_global_data.device_trim_master = device_trim */
            else
                PEER_GDATA.device_trim_slave = device_trim; /* peer_global_data.device_trim_slave = device_trim */
        }
        configManagerReleaseConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID);
    }
}
#endif
/****************************************************************************
NAME
    peerGetTwsSlaveTrim

DESCRIPTION
    Returns  the TWS device trims for slave to be used for volume updates

RETURNS
    TWS slave trim

**************************************************************************/
#ifdef PEER_TWS
int16 peerGetTwsSlaveTrim(void)
{
    return PEER_GDATA.device_trim_slave;
}
#endif

#ifdef PEER_TWS
static void setPeerTwsSlaveTrim(int16 slave_trim)
{
    PEER_GDATA.device_trim_slave = slave_trim;
}
#endif

/****************************************************************************
NAME
    peerGetTwsMasterTrim

DESCRIPTION
    Returns  the TWS device trims for master to be used for volume updates

RETURNS
    TWS master trim

**************************************************************************/
#ifdef PEER_TWS
int16 peerGetTwsMasterTrim(void)
{
    return  PEER_GDATA.device_trim_master;
}
#endif

#ifdef PEER_TWS
bool peerGetTWSDeviceTrimConfig(peer_tws_trim_config_t *trim_config)
{
    sink_peer_writeable_config_def_t *config_data;

    if (configManagerGetReadOnlyConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID, (const void **)&config_data))
    {
        trim_config->device_trim_master = PEER_GDATA.device_trim_master;
        trim_config->device_trim_slave = PEER_GDATA.device_trim_slave;
        trim_config->device_trim_change = config_data->device_trim_change;
        trim_config->device_trim_min = config_data->device_trim_min;
        trim_config->device_trim_max = config_data->device_trim_max;
        configManagerReleaseConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}
#endif

#ifdef PEER_TWS
bool peerSetTWSDeviceTrimConfig(const peer_tws_trim_config_t *trim_config)
{
    sink_peer_writeable_config_def_t *config_data;

    if (configManagerGetWriteableConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID, (void **)&config_data, 0))
    {
        config_data->device_trim_master = PEER_GDATA.device_trim_master = trim_config->device_trim_master;
        config_data->device_trim_slave = PEER_GDATA.device_trim_slave = trim_config->device_trim_slave;
        config_data->device_trim_change = trim_config->device_trim_change;
        config_data->device_trim_min = trim_config->device_trim_min;
        config_data->device_trim_max = trim_config->device_trim_max;
        configManagerUpdateWriteableConfig(SINK_PEER_WRITEABLE_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}
#endif

/*************************************************************************
NAME
    peerNotifyWiredSourceConnected

DESCRIPTION
    Notifies audio source connection to the remote Peer

RETURNS
    None

**************************************************************************/
void peerNotifyWiredSourceConnected (AudioSrc audio_source)
{
    sinkAvrcpUpdatePeerWiredSourceConnected(audio_source);
    peerClaimRelay(TRUE);

    if(audio_source == ANALOG_AUDIO)
    {
      PEER_UPDATE_REQUIRED_RELAY_STATE("ANALOG AUDIO CONNECTED");
    }
    else if(audio_source == USB_AUDIO)
    {
        PEER_UPDATE_REQUIRED_RELAY_STATE("USB SPEAKER ACTIVE");
    }
}

/*************************************************************************
NAME
    peerNotifyWiredSourceDisconnected

DESCRIPTION
    Notifies audio source disconnection to the remote Peer

RETURNS
    None

**************************************************************************/
void peerNotifyWiredSourceDisconnected (AudioSrc audio_source)
{
    sinkAvrcpUpdatePeerSourceDisconnected(audio_source);
    peerClaimRelay(FALSE);

    if(audio_source == ANALOG_AUDIO)
    {
      PEER_UPDATE_REQUIRED_RELAY_STATE("ANALOG AUDIO DISCONNECTED");
    }
    else if(audio_source == USB_AUDIO)
    {
        PEER_UPDATE_REQUIRED_RELAY_STATE("USB SPEAKER INACTIVE");
    }
}

/*************************************************************************
NAME    
    peerHandleVolumeLimitCmd
    
DESCRIPTION
    Handles the volume changes in response to the peers low battery command

RETURNS
    None
    
**************************************************************************/
void peerHandleVolumeLimitCmd (peer_volume_max_min max_min)
{
    uint16 peer_id;
        
    PEER_DEBUG(("peerHandleVolumeLimitCmd  Limit = %s\n", max_min==VOLUME_MAX?"MAX":"MIN"));
    
    if (a2dpGetPeerIndex(&peer_id))
    {
        if(max_min)
        {
            MessageSend(&theSink.task , EventSysVolumeMax , 0);
        }
        else
        {
            MessageSend(&theSink.task , EventSysVolumeMin , 0);
        }
    }
}

#ifdef PEER_TWS
bool peerIsA2dpIndexTwsSource(uint8 a2dpIndex)
{
    return (getA2dpPeerRemoteDevice(a2dpIndex) == remote_device_peer) &&
            (getA2dpPeerRemoteFeatures(a2dpIndex) & remote_features_tws_a2dp_source);
}
#endif

#ifdef PEER_TWS
bool peerSlaveVolumeIsHandledViaMaster(void)
{
    if(!areSychronisedEventsSupported())
    {
        return TRUE;
    }
    return FALSE;
}
#endif

#ifdef PEER_TWS
void peerSignalVolumeUpdateToMaster(const volume_direction direction)
{
     if(direction == increase_volume)
     {
         sinkAvrcpVolumeUp();
     }
     else if(direction == decrease_volume)
     {
         sinkAvrcpVolumeDown();
     }
}
#endif

/*************************************************************************
NAME
    peerUpdatePeerMute

DESCRIPTION
    Updates mute status on a remote Peer

**************************************************************************/
#ifdef PEER_TWS
void peerUpdatePeerMute(bool mute_enable)
{

    uint16 peerAvrcpIndex;

    /* If a peer is connected, then send the avrcp vendor dependant command to the peer device updating the mute status */
    if(avrcpGetPeerIndex (&peerAvrcpIndex) )
    {
        uint8 payload = mute_enable;

        /* Inform remote peer about mute toggle */
        sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex,
                                                AVRCP_PEER_CMD_UPDATE_MUTE,
                                                AVRCP_PAYLOAD_PEER_CMD_UPDATE_MUTE,
                                                &payload );
    }
}
#endif

/*************************************************************************
NAME
    peerUpdateLedIndication

DESCRIPTION
    Informs the remote peer of a change in LED indication state.

**************************************************************************/
#ifdef PEER_TWS
void peerUpdateLedIndication(bool led_enable)
{
    uint16 peerAvrcpIndex;

    if(avrcpGetPeerIndex (&peerAvrcpIndex) )
    {
        uint8 payload = led_enable;

        sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex,
                                                AVRCP_PEER_CMD_UPDATE_LED_INDICATION_ON_OFF,
                                                AVRCP_PAYLOAD_PEER_CMD_UPDATE_LED_INDICATION_ON_OFF,
                                                &payload );
    }
}
#endif

#ifdef PEER_TWS
static void swapMasterSlaveRoutingModes(uint8 * routing_modes,
        PeerTwsAudioRouting master_routing, PeerTwsAudioRouting slave_routing)
{
    routing_modes[ROUTING_MODE_MASTER_INDEX] = (uint8)slave_routing;
    routing_modes[ROUTING_MODE_SLAVE_INDEX] = (uint8)master_routing;
}
#endif

/*************************************************************************
NAME
    peerSetPeerAudioRouting

DESCRIPTION
    Informs current Peer of the required routing modes and updates DSP

RETURNS
    TRUE if successful, FALSE otherwise

**************************************************************************/
#ifdef PEER_TWS
void peerSetPeerAudioRouting (PeerTwsAudioRouting master_routing_mode, PeerTwsAudioRouting slave_routing_mode)
{
    uint16 peer_id;

    if(a2dpGetPeerIndex(&peer_id))
    {
#ifdef ENABLE_AVRCP
        uint16 peerAvrcpIndex;
        if(avrcpGetPeerIndex(&peerAvrcpIndex))
        {
            uint8 routing_modes[2];

            swapMasterSlaveRoutingModes(routing_modes, master_routing_mode, slave_routing_mode);

            sinkAvrcpVendorUniquePassthroughRequest( peerAvrcpIndex, AVRCP_PEER_CMD_TWS_AUDIO_ROUTING,
                                sizeof(routing_modes), (const uint8 *)routing_modes );
        }
#endif
        peerHandleAudioRoutingCmd( master_routing_mode, slave_routing_mode );
    }
}
#endif

/*************************************************************************
NAME
    peerSlaveRequestEqEnableChange

DESCRIPTION
    EQ Enable change requested from Peer slave device, processed on master
    device

**************************************************************************/
#ifdef PEER_TWS
static bool peerSlaveRequestEqEnableChange(PeerEqEnableRelayType action)
{
    uint16 peerAvrcpIndex;

    /* If a peer is connected, it is a slave device and is receiving a peer stream
       then send the avrcp vendor dependant command to the master peer device
       to action the EQ change request */
    if((avrcpGetPeerIndex(&peerAvrcpIndex)) && (!peerIsThisDeviceTwsMaster()) && (peerIsPeerStreaming()))
    {
        /* Inform remote peer about EQ toggle request */
        sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex,
                                                AVRCP_PEER_CMD_SLAVE_EQ_ENABLE_REQUEST,
                                                AVRCP_PAYLOAD_PEER_CMD_SLAVE_EQ_ENABLE_REQUEST,
                                                (const uint8 *)&action );

        /* command has been relayed to master peer device, do not process locally */
        return TRUE;
    }

    /* no peer slave streaming connection present */
    return FALSE;
}
#endif
/*************************************************************************
NAME
    peerSlaveRequestEqNextBank

DESCRIPTION
    next EQ bank requested from Peer slave device, processed on master device
    and EQ settings passed back to slave to keep peer device correctly aligned

**************************************************************************/
#ifdef PEER_TWS
static bool peerSlaveRequestEqNextBank(void)
{

    uint16 peerAvrcpIndex;

    /* If a peer is connected, it is a slave device and is receiving a peer stream
       then send the avrcp vendor dependant command to the master peer device
       to action the next EQ bank request */
    if((avrcpGetPeerIndex(&peerAvrcpIndex)) && (!peerIsThisDeviceTwsMaster()) && (peerIsPeerStreaming()))
    {
        /* Inform remote peer about EQ next bank request */
        sinkAvrcpVendorUniquePassthroughRequest(peerAvrcpIndex,
                                                AVRCP_PEER_CMD_SLAVE_EQ_NEXT_BANK_REQUEST,
                                                AVRCP_PAYLOAD_PEER_CMD_SLAVE_EQ_NEXT_BANK_REQUEST,
                                                NULL );

        /* command has been relayed to master peer device, do not process locally */
        return TRUE;
    }

    /* no peer slave streaming connection present */
    return FALSE;
}
#endif

#ifdef PEER_TWS
void peerUpdateEqEnable(uint8 enable)
{
    MessageId enable_event_id = EventUsrUserEqOff;
    switch(enable)
    {
        case EQ_TOGGLE:
            enable_event_id = EventUsrUserEqOnOffToggle;
            break;
        case EQ_ON:
            enable_event_id = EventUsrUserEqOn;
            break;
        default:
            break;
    }
    MessageSend(&theSink.task, enable_event_id, 0);
}
#endif

#ifdef PEER_TWS
static bool peerTrimsAreHandledThroughSynchronisedEvents(void)
{
    if(areSychronisedEventsSupported())
    {
        return TRUE;
    }
    return FALSE;
}
#endif

#ifdef PEER_TWS

int16 peerGetTwsTrimForLocalDevice(void)
{
    int16 tws_trim = 0;
    if(peerTrimsAreHandledThroughSynchronisedEvents())
    {
        if(peerIsThisDeviceTwsMaster())
        {
            tws_trim = peerGetTwsMasterTrim();
        }
        else if(peerIsThisDeviceTwsSlave())
        {
            tws_trim = peerGetTwsSlaveTrim();
        }
    }
    return tws_trim;
}
#endif
#ifdef PEER_TWS

bool peerHandlesVolumeThroughSyncCommands(void)
{
    uint16 avrcp_peer_index;
    
    if(peerIsSingleDeviceOperationEnabled() && avrcpGetPeerIndex(&avrcp_peer_index))
    {
        if(areSychronisedEventsSupported())
            return TRUE;
    }
    
    return FALSE;
}
void peerSyncVolumeIfMaster(void)
{
    if(peerHandlesVolumeThroughSyncCommands() && peerIsThisDeviceTwsMaster())
    {
        scheduleSynchronisedPeerVolumeEvent();
    }
}
#endif
#ifdef PEER_TWS
bool peerDoesRemotePeerSupportAudioRoutingConfiguration(void)
{
    uint16 peer_id;
    bool audio_routing_config_support = FALSE;

    if(a2dpGetPeerIndex(&peer_id))
    {
        audio_routing_config_support = (getA2dpLinkDataPeerVersion(peer_id) >= PEER_DEVICE_MIN_SUPPORTED_AUDIO_ROUTING_COMPATIBILITY_VERSION);
    }
    return audio_routing_config_support;
}
#endif

static bool isLatestTwsVersionSupported(void)
{
#ifdef PEER_TWS
    return (AudioPluginMusicVariantsCodecDeterminesTwsEncoding() == FALSE);
#else
    return FALSE;
#endif
}

static uint16 peerGetPeerVersion(void)
{
    uint16 peer_version = PEER_DEVICE_VERSION;
    if(!isLatestTwsVersionSupported())
    {
        peer_version = PEER_DEVICE_LEGACY_VERSION;
    }
    return peer_version;
}

static uint16 peerGetMinimumSupportedPeerVersion(void)
{
    return (peerGetPeerVersion() & BCD_MAJOR_VERSION);
}

#ifdef PEER_TWS
bool peerCanThisTwsDeviceRelayRequestedAudio(void)
{
    return (peerIsModeTws()
            && (peerIsLinkMaster() && peerRelayAvailableSources(sinkAudioGetRequestedAudioSource())));
}
#endif

/****************************************************************************
NAME
    peerDisconnectLowestPriorityAG

DESCRIPTION
    Disconnect the device considered to be least active, this involves checking for
    streaming A2DP and priority in the PDL such that a connection slot can be free'd
    for purposes such as connecting to a PEER device.

RETURNS
    TRUE if AG scheduled to be disconnected, FALSE if error occured such as PEER link
    already connected 

*/
bool peerDisconnectLowestPriorityAG(void)
{
    uint8 a2dp_index; 
    bdaddr  dev_addr;

    memset(&dev_addr,0,sizeof(bdaddr));

    /* ensure no peer devices are already connected and there are two AG's connected */
    if((deviceManagerNumConnectedPeerDevs() == 0)&&(deviceManagerNumConnectedDevs() == MAX_A2DP_CONNECTIONS)
        &&(FALSE == sinkIsAGDisconnecting()))
    {
        /* determine if either device is streaming A2DP and disconnect the one that isn't */
        if(sinkAudioIsAudioRouted())
        {
            /* an audio source is being streamed, check if this is A2DP */
            for_all_a2dp(a2dp_index)
            {
                /* is a2dp connected? */
                if(getA2dpStatusFlag(CONNECTED, a2dp_index))
                {
                    /* check whether the a2dp connection is present and streaming data and that the audio is routed */
                    if(sinkAudioGetRoutedAudioSink() == A2dpMediaGetSink(getA2dpLinkDataDeviceId(a2dp_index), getA2dpLinkDataStreamId(a2dp_index)))
                    {
                        /* this A2DP connection is streaming A2DP and is the highest priority, discconnect the other one */
                        if(a2dp_index == a2dp_secondary)
                        {
                            /* disconnect primary device */
                            if(TRUE == deviceManagerGetProfileAddr(conn_hfp_pri |conn_a2dp_pri , &dev_addr))
                            {
                                deviceManagerDisconnectAG(&dev_addr);
                                /* disconnect initiated */
                            }                            
                            return TRUE;
                        }
                        else
                        {
                            /* disconnect secondary device */
                            break;
                        }
                    }
                }
            }
        }

        /* if no A2DP audio being routed, disconnect lower priority secondary device */
        if(TRUE == deviceManagerGetProfileAddr(conn_hfp_sec |conn_a2dp_sec , &dev_addr))
        {
            deviceManagerDisconnectAG(&dev_addr);
            /* disconnect initiated */
        }
        return TRUE;
    }
    else if(TRUE == sinkIsAGDisconnecting())
    {
        return TRUE;
    }
    /* peer device already connected or only 1 AG connected, no need to disconnect anything */
    return FALSE;
}

/***************************************************************************
NAME
    peerDelaySourceStream

DESCRIPTION
    Utility function for delay the source media streaming.

PARAMS
    void

RETURNS
    void
*/
#ifdef PEER_TWS
void peerDelaySourceStream(void)
{
    PEER_DEBUG(("PEER: peerDelaySourceStream\n"));
    MessageSendLater((TaskData*)&peer_internal_message_task, 
        PEER_INTERNAL_MSG_DELAY_SOURCE_STREAM, NULL, sinkDataGetSourceSteamingDelay());
}
#endif   /* PEER_TWS */

/***************************************************************************
NAME
    peerGetPeerSampleRateForWiredInput

DESCRIPTION
    Get the Peer sample Rate.

PARAMS
    wired_input_sample_rate Wired input sample rate

RETURNS
    Peer sample rate
*/
WIRED_SAMPLING_RATE_T peerGetPeerSampleRateForWiredInput(WIRED_SAMPLING_RATE_T wired_input_sample_rate)
{
    UNUSED(wired_input_sample_rate);
    return WIRED_RATE_48000;
} 

/***************************************************************************
NAME
    peerUpdateMonoSupportInfo

DESCRIPTION
    Upadtes the mono support information based on ps key configuration.

PARAMS
    Base address of service record entry used for storing the supported features of tws. 

RETURNS
    -
*/
static void peerUpdateMonoSupportInfo(uint8 *supported_features)
{
    if(AudioConfigGetRenderingMode()==single_channel_rendering)
    {        
        *(supported_features) |= remote_features_peer_mono_channel_supported ;
        PEER_DEBUG(("PEER : Mono support info got updated at master side\n"));
    }
}

/********************************************************************************
NAME
    peerCheckRemoteRenderingMode

DESCRIPTION
    Determine if mono channel is supported in the peer device

PARAMS
    Remote feature list

RETURNS
    single channel rendering or multi channel rendering depending upon the supported features of the peer device
*/
PeerRemoteRenderingMode peerCheckRemoteRenderingMode(remote_features features)
{
    if(features & remote_features_peer_mono_channel_supported )
    {
        PEER_DEBUG(("PEER : Mono Channel is supported in the peer device\n"));
        return remote_single_channel_rendering;
    }
    PEER_DEBUG(("PEER : Mono Channel is not supported by peer device\n"));
    return remote_multi_channel_rendering;
}

void peerUpdateRelayStateOnSuspend(uint16 Id)
{
    if (getA2dpPeerRemoteDevice(Id) == remote_device_nonpeer)
    {   /* AV Source suspended it's media channel, update the required state */
        PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE SUSPENDED");
    }
    else if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
    {   /* Peer suspended it's media channel, look to see if local device has a streaming AV source */
        peerAdvanceRelayState(RELAY_EVENT_SUSPENDED);
    }
}


#endif  /* ENABLE_PEER */


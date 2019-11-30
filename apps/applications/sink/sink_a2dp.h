/*
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file    sink_a2dp.h
@brief   Interface to the a2dp profile initialisation functions.
*/

#ifndef _SINK_A2DP_INIT_H_
#define _SINK_A2DP_INIT_H_

#include <a2dp.h>
#include <audio_plugin_music_params.h>
#include "sink_peer.h"


/* Local stream end point codec IDs */
#define SOURCE_SEID_MASK        0x20        /*!< @brief Combined with a SEP codec id to produce an id for source codecs */
#define TWS_SEID_MASK           0x10        /*!< @brief Combined with a SEP codec id to produce an id for TWS codecs */
#define BASE_SEID_MASK          0x0F        /*!< @brief Mask used to determine base codec SEID, masking away any SOURCE or TWS bits */
#define INVALID_SEID            0x00        /*!< @brief A seid is a 6 bit non-zero value */
#define SBC_SEID                0x01        /*!< @brief Local Stream End Point ID for SBC codec */
#define MP3_SEID                0x02        /*!< @brief Local Stream End Point ID for MP3 codec */
#define AAC_SEID                0x03        /*!< @brief Local Stream End Point ID for AAC codec */
#define APTX_SEID               0x05        /*!< @brief Local Stream End Point ID for aptX codec */
#define FASTSTREAM_SEID         0x04        /*!< @brief Local Stream End Point ID for FastStream codec */
#define APTX_SPRINT_SEID        0x06        /*!< @brief Local Stream End Point ID for aptX Sprint codec */
#define APTXHD_SEID             0x07        /*!< @brief Local Stream End Point ID for aptX HD codec */
#define APTX_LL_SEID            0x08        /*!< @brief Local Stream End Point ID for aptX LL codec */
#define APTX_ADAPTIVE_SEID      0x09        /*!< @brief Local Stream End Point ID for aptX Adaptive codec */

/* The bits used to enable codec support for A2DP, as read from CONFIG_CODEC_ENABLED */
#define SBC_CODEC_BIT           0xFF        /*!< @brief SBC is mandatory and always enabled */
#define MP3_CODEC_BIT           0           /*!< @brief Bit used to enable MP3 codec in PSKEY */
#define AAC_CODEC_BIT           1           /*!< @brief Bit used to enable AAC codec in PSKEY */
#define FASTSTREAM_CODEC_BIT    2           /*!< @brief Bit used to enable FastStream codec in PSKEY */
#define APTX_CODEC_BIT          3           /*!< @brief Bit used to enable aptX codec in PSKEY */
#define APTX_SPRINT_CODEC_BIT   4           /*!< @brief Bit used to enable aptX Sprint/LL codec in PSKEY */
#define APTXHD_CODEC_BIT        5           /*!< @brief Bit used to enable aptX-HD codec in PSKEY */
#define APTX_AD_CODEC_BIT       6           /*!< @brief Bit used to enable aptX Adaptive codec in PSKEY */

#define KALIMBA_RESOURCE_ID     1           /*!< @brief Resource ID for Kalimba */

/* Bitmasks used to indicate what application feature is supported by a codec */
#define STD_SNK 0x01
#define STD_SRC 0x02
#define TWS_SNK 0x04
#define TWS_SRC 0x08

#define SBC_BITPOOL_MIN                  2
#define SBC_BITPOOL_MAX                250
#define SBC_BITPOOL_MEDIUM_QUALITY      35
#define SBC_BITPOOL_HIGH_QUALITY        53

/* Time out before considering that the peer device is taking too much time to respond to a start streaming indication */
#define PEER_STREAMING_TIMEOUT D_SEC(1.5) /* 2 Sec allowed before responding to the Streaming Source */

#ifdef INCLUDE_A2DP_EXTRA_CODECS
#ifndef INCLUDE_APTX
#define INCLUDE_APTX
#endif
#endif /* INCLUDE_A2DP_EXTRA_CODECS */


typedef enum
{
    a2dp_primary = 0x00,
    a2dp_secondary = 0x01,
    a2dp_invalid = 0xff
} a2dp_index_t;

#define INVALID_ID 0xff
#define for_all_a2dp(idx)      for((idx) = 0; (idx) < MAX_A2DP_CONNECTIONS; (idx)++)

typedef enum
{
    a2dp_not_suspended,
    a2dp_local_suspended,
    a2dp_remote_suspended
} a2dp_suspend_state;


#ifdef ENABLE_AVRCP
typedef enum
{
    avrcp_support_unknown,
    avrcp_support_second_attempt,
    avrcp_support_unsupported,
    avrcp_support_supported
} avrcpSupport;
#endif

typedef struct
{
    a2dp_index_t a2dp_index;
} EVENT_STREAM_ESTABLISH_T;

typedef enum
{
    LR_UNKNOWN_ROLE,
    LR_CURRENT_ROLE_SLAVE,
    LR_CURRENT_ROLE_MASTER,
    LR_CHECK_ROLE_PENDING_START_REQ
} a2dp_link_role;

typedef enum
{
    REMOTE_AG_CONNECTION,
    CONNECTED,
    MEDIA_RECONNECT,
    PLAYING,
    QUAL_DISABLE_STREAM_RESUME
}a2dp_status_flag;

typedef enum
{
    LOCAL_PEER,
    REMOTE_PEER,
    UNKNOWN_PEER
}peer_type;

/****************************************************************************
  FUNCTIONS
*/

/*************************************************************************
NAME
    SinkA2dpIsInitialised

DESCRIPTION
    Test if the sink_a2dp module data has been initialised.

RETURNS
    TRUE if InitA2dp has been called; FALSE otherwise.

**************************************************************************/
bool SinkA2dpIsInitialised(void);

/*************************************************************************
NAME
    a2dpGetPeerIndex

DESCRIPTION
    Attempts to obtain the index into a2dp_link_data structure for a currently
    connected Peer device.

RETURNS
    TRUE if successful, FALSE otherwise

**************************************************************************/
#ifdef ENABLE_PEER
bool a2dpGetPeerIndex (uint16* Index);
#else
#define a2dpGetPeerIndex(x) (UNUSED(x), FALSE)
#endif
/*************************************************************************
NAME
    a2dpIsIndexPeer

DESCRIPTION
    Determines whether the provided a2dp index is of a peer device or not.

RETURNS
    TRUE if the index is that of a peer device, false otherwise.

**************************************************************************/
#ifdef ENABLE_PEER
bool a2dpIsIndexPeer(uint16 index);
#else
#define a2dpIsIndexPeer(index) (FALSE)
#endif

/*************************************************************************
NAME
    a2dpGetSourceIndex

    Attempts to obtain the index into a2dp_link_data structure for a currently
    connected A2dp Source device.

RETURNS
    TRUE if successful, FALSE otherwise

**************************************************************************/
bool a2dpGetSourceIndex (uint16* Index);

/*************************************************************************
NAME
    a2dpGetSourceSink

DESCRIPTION
    Attempts to obtain the media sink for a currently connected A2dp Source.

RETURNS
    Handle to media sink if present, NULL otherwise

**************************************************************************/
Sink a2dpGetSourceSink (void);

/*************************************************************************
NAME
    sinkA2dpHandlePeerAvrcpConnectCfm

DESCRIPTION
    Configure initial relay availability when a Peer connects

RETURNS
    None

**************************************************************************/
void sinkA2dpHandlePeerAvrcpConnectCfm (uint16 peer_id, bool successful);

/****************************************************************************
NAME
    sinkA2dpSetLinkRole

DESCRIPTION
    Updates stored BT role for specified device

RETURNS
    None

**************************************************************************/
void sinkA2dpSetLinkRole (Sink sink, hci_role role);


/*************************************************************************
NAME
    a2dpSetSuspendState

DESCRIPTION
    Sets the suspend state for the specified device

RETURNS
    None

**************************************************************************/
void a2dpSetSuspendState (uint16 id, a2dp_suspend_state state);

/*************************************************************************
NAME
    a2dpPauseNonRoutedSource

DESCRIPTION
    Check whether the a2dp connection is present and streaming data and that the audio is routed,
    if thats true then pause/stop the incoming stream corresponding to the input deviceId.

RETURNS
    None

**************************************************************************/
#ifdef ENABLE_AVRCP
    void a2dpPauseNonRoutedSource(uint16 id);
#endif

/*************************************************************************
NAME
    avrcpStop

DESCRIPTION
    This function requests for playback to stop via avrcp,

RETURNS
    None

**************************************************************************/
#ifdef ENABLE_AVRCP
    void a2dpAvrcpStop(const uint16 avrcpIndex);
#endif

/*************************************************************************
NAME
    a2dpIssuePeerOpenRequest

DESCRIPTION
    Issues a request to opens a media stream to a currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerOpenRequest (void);

/*************************************************************************
NAME
    a2dpIssuePeerCloseRequest

DESCRIPTION
    Issues a request to close the relay channel to the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerCloseRequest (void);

/*************************************************************************
NAME
    a2dpIssuePeerStartRequest

DESCRIPTION
    Issues a request to start the relay channel to the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerStartRequest (void);

/*************************************************************************
NAME
    a2dpIssuePeerSuspendRequest

DESCRIPTIONDESCRIPTION
    Issues a request to suspend the relay channel to the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerSuspendRequest (void);

/*************************************************************************
NAME
    a2dpIssuePeerStartResponse

DESCRIPTION
    Issues a start response to a Peer based on availability of the relay channel

RETURNS
    TRUE if response sent, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerStartResponse (void);

/****************************************************************************
NAME
 a2dpCheckDeviceTrimVol

DESCRIPTION
 check whether any a2dp connections are present and if these are currently active
 and routing audio to the device, if that is the case adjust the trim volume up or down
 as appropriate

RETURNS
 bool   TRUE if volume adjusted, FALSE if no streams found

***************************************************************************/
bool a2dpCheckDeviceTrimVol (volume_direction dir, tws_device_type tws_device);

/*************************************************************************
NAME
    a2dpSetPlayingState

DESCRIPTION
    Logs the current AVRCP play status for the specified A2DP connection and
    updates the Suspend State for the Media channel so that it reflects the true
    media playing status.

RETURNS
    None

**************************************************************************/
void a2dpSetPlayingState (uint16 id, bool playing);

/*************************************************************************
NAME
    InitA2dp

DESCRIPTION
    This function initialises the A2DP library.
*/
void InitA2dp(void);

/*************************************************************************
NAME
    getA2dpIndex

DESCRIPTION
    This function tries to find a device id match in the array of a2dp links
    to that device id passed in

RETURNS
    match status of true or false
**************************************************************************/
bool getA2dpIndex(uint8 DeviceId, uint16 * Index);

/*************************************************************************
NAME
    getA2dpIndexFromSink

DESCRIPTION
    This function tries to find the a2dp device associated with the supplied
    sink.  The supplied sink can be either a signalling or media channel.

RETURNS
    match status of true or false
**************************************************************************/
bool getA2dpIndexFromSink(Sink sink, uint16 * Index);

/*************************************************************************
NAME
    InitSeidConnectPriority

DESCRIPTION
    Retrieves a list of the preferred Stream End Points to connect with.
*/
uint16 InitSeidConnectPriority(uint8 seid, uint8 *seid_list);


/*************************************************************************
NAME
    getA2dpStreamData

DESCRIPTION
    Function to retreive media sink and state for a given A2DP source

RETURNS
    void
**************************************************************************/
void getA2dpStreamData(a2dp_index_t index, Sink* sink, a2dp_stream_state* state);


/*************************************************************************
NAME
    getA2dpStreamRole

DESCRIPTION
    Function to retreive the role (source/sink) for a given A2DP source

RETURNS
    void
**************************************************************************/
void getA2dpStreamRole(a2dp_index_t index, a2dp_role_type* role);


/*************************************************************************
NAME
    getA2dpPlugin

DESCRIPTION
    Retrieves the audio plugin for the requested SEP.
*/
Task getA2dpPlugin(uint8 seid);

/****************************************************************************
NAME
    sinkA2dpInitComplete

DESCRIPTION
    Headset A2DP initialisation has completed, check for success.

RETURNS
    void
**************************************************************************/
void sinkA2dpInitComplete(const A2DP_INIT_CFM_T *msg);

/****************************************************************************
NAME
    issueA2dpSignallingConnectResponse

DESCRIPTION
    Issue response to a signalling channel connect request, following discovery of the
    remote device type.

RETURNS
    void
**************************************************************************/
void issueA2dpSignallingConnectResponse(const bdaddr *bd_addr, bool accept);

/*************************************************************************
NAME
    handleA2DPSignallingConnectInd

DESCRIPTION
    handle a signalling channel connect indication

RETURNS

**************************************************************************/
void handleA2DPSignallingConnectInd(uint8 DeviceId, bdaddr SrcAddr);

/*************************************************************************
NAME
    handleA2DPSignallingConnected

DESCRIPTION
    handle a confirm of a signalling channel connected message

RETURNS

**************************************************************************/
void handleA2DPSignallingConnected(a2dp_status_code status, uint8 DeviceId, bdaddr SrcAddr, bool locally_initiated);

/*************************************************************************
NAME
    connectA2dpStream

DESCRIPTION
    Issues a request to the A2DP library to establish a media stream to a
    remote device.  The request can be delayed by a certain amount of time
    if required.

RETURNS

**************************************************************************/
void connectA2dpStream (a2dp_index_t index, uint16 delay);

/*************************************************************************
NAME
    handleA2DPOpenInd

DESCRIPTION
    handle an indication of an media channel open request, decide whether
    to accept or reject it

RETURNS

**************************************************************************/
void handleA2DPOpenInd(uint8 DeviceId, uint8 seid);

/*************************************************************************
NAME
    handleA2DPOpenCfm

DESCRIPTION
    handle a successfull confirm of a media channel open

RETURNS

**************************************************************************/
void handleA2DPOpenCfm(uint8 DeviceId, uint8 StreamId, uint8 seid, a2dp_status_code status);

/*************************************************************************
NAME
    handleA2DPSignallingDisconnected

DESCRIPTION
    handle the disconnection of the signalling channel
RETURNS

**************************************************************************/
void handleA2DPSignallingDisconnected(uint8 DeviceId, a2dp_status_code status,  bdaddr SrcAddr);

/*************************************************************************
NAME
    handleA2DPSignallingLinkloss

DESCRIPTION
    handle the indication of a link loss
RETURNS

**************************************************************************/
void handleA2DPSignallingLinkloss(uint8 DeviceId);


/*************************************************************************
NAME
    handleA2DPLinklossReconnectCancel

DESCRIPTION
    handle the indication of a link loss reconnection cancel
RETURNS

**************************************************************************/
void handleA2DPLinklossReconnectCancel(uint8 DeviceId);

/*************************************************************************
NAME
    handleA2DPStartInd

DESCRIPTION
    handle the indication of media start ind
RETURNS

**************************************************************************/
void handleA2DPStartInd(uint8 DeviceId, uint8 StreamId);

/*************************************************************************
NAME
    handleA2DPStartStreaming

DESCRIPTION
    handle the indication of media start cfm
RETURNS

**************************************************************************/
void handleA2DPStartStreaming(uint8 DeviceId, uint8 StreamId, a2dp_status_code status);

/*************************************************************************
NAME
    handleA2DPStoreClockMismatchRate

DESCRIPTION
    handle storing the clock mismatch rate for the active stream
RETURNS

**************************************************************************/
void handleA2DPStoreClockMismatchRate(uint16 clockMismatchRate);

/*************************************************************************
NAME
    SuspendA2dpStream

DESCRIPTION
    called when it is necessary to suspend an a2dp media stream due to
    having to process a call from a different AG
RETURNS

**************************************************************************/
void SuspendA2dpStream(a2dp_index_t index);

/*************************************************************************
NAME
    sinkA2dpIsA2dpLinkSuspended

DESCRIPTION
    Helper to indicate whether A2DP is suspended

RETURNS
    TRUE if A2DP suspended, otherwise FALSE
**************************************************************************/
bool sinkA2dpIsA2dpLinkSuspended(a2dp_index_t index);

/*************************************************************************
NAME
    sinkA2dpGetA2dpLinkSuspendState

DESCRIPTION
    Helper to get the current suspend state of the given A2DP link

RETURNS
    The A2DP suspend state of the given link
**************************************************************************/
a2dp_suspend_state sinkA2dpGetA2dpLinkSuspendState(a2dp_index_t index);


/*************************************************************************
NAME
    ResumeA2dpStream

DESCRIPTION
    Called to resume a suspended A2DP stream
RETURNS

**************************************************************************/
bool ResumeA2dpStream(a2dp_index_t index);

/*************************************************************************
NAME
    handleA2DPSyncDelayInd

DESCRIPTION
    Handle request from A2DP library for a Sink device to supply an initial
    Synchronisation Delay (audio latency) report.

RETURNS

**************************************************************************/
void handleA2DPSyncDelayInd (uint8 device_id, uint8 seid);

/*************************************************************************
NAME
    handleA2DPLatencyReport

DESCRIPTION
    Handle notification from an audio plugin raised due to the DSP providing
    a measured audio latency value.

RETURNS

**************************************************************************/
void handleA2DPLatencyReport (bool estimated, uint16 latency, Sink sink);

/*************************************************************************
NAME
    handleA2DPMessage

DESCRIPTION
    A2DP message Handler, this function handles all messages returned
    from the A2DP library and calls the relevant functions if required

RETURNS

**************************************************************************/
void handleA2DPMessage( Task task, MessageId id, Message message );

/*************************************************************************
NAME
    getA2dpIndexFromBdaddr

DESCRIPTION
    Attempts to find a A2DP link data index based on the supplied bdaddr.

RETURNS
    TRUE if successful, FALSE otherwise

**************************************************************************/
bool getA2dpIndexFromBdaddr (const bdaddr *bd_addr, uint16 *index);

/*************************************************************************
NAME
    disconnectAllA2dpAVRCP

DESCRIPTION
    disconnect any a2dp and avrcp connections

RETURNS

**************************************************************************/
void disconnectAllA2dpAvrcp (bool disconnect_peer);

/*************************************************************************
NAME
    disconnectA2dpAvrcpFromDevice

DESCRIPTION
    disconnect A2DP and AVRCP connections with the device provided.

RETURNS

**************************************************************************/
void disconnectA2dpAvrcpFromDevice(const bdaddr *bdaddr_non_gaia_device);


/*************************************************************************
NAME
    a2dpGetNextAvBdAddress

DESCRIPTION
    Returns the BD address of the other connected non-peer device, if any.

RETURNS
    TRUE if successful, FALSE otherwise
**************************************************************************/
bool a2dpGetNextAvBdAddress(const bdaddr *bd_addr , bdaddr *next_bdaddr );

/*************************************************************************
NAME
    disconnectAllA2dpPeerDevices

DESCRIPTION
    disconnect any a2dp connections to any peer devices

RETURNS
    TRUE is any peer devices disconnected, FALSE otherwise

**************************************************************************/
bool disconnectAllA2dpPeerDevices (void);

/*************************************************************************
 NAME
    handleA2DPUserEqBankUpdate

DESCRIPTION
    Handle notification from an audio plugin for DSP ready for data message in order to update user defined EQ bank
    when GAIA set EQ parameter commands are processed

RETURNS
**************************************************************************/
void handleA2DPUserEqBankUpdate(void);

/*************************************************************************
NAME
    findCurrentA2dpSource

DESCRIPTION
    Attempts to obtain index of a connected A2dp Source that has established
    a media channel

RETURNS
    TRUE if found, FALSE otherwise

**************************************************************************/
bool findCurrentA2dpSource (a2dp_index_t* index);

/*************************************************************************
NAME
    findCurrentStreamingA2dpSource

DESCRIPTION
    Attempts to obtain index of a connected A2dp Source that has established
    a media channel and is streaming

RETURNS
    TRUE if found, FALSE otherwise

**************************************************************************/
bool findCurrentStreamingA2dpSource (a2dp_index_t* index);

/*************************************************************************
NAME
    controlA2DPPeer

DESCRIPTION
    Issues AVDTP Start/Suspend command to current Peer device

RETURNS
    FALSE if no appropriate Peer device found
    TRUE if a command was issued
**************************************************************************/

bool controlA2DPPeer (uint16 event);

#ifdef ENABLE_SUBWOOFER
/*************************************************************************
NAME
    suspendWhenSubwooferStreamingLowLatency

DESCRIPTION
    funnction to determine if a2dp stream should be suspended due to the use
    of the low latency subwoofer link. Streaming a2dp media will adversely affect
    the quality of the sub low latency link due to bluetooth link bandwidth
    limitations

RETURNS
    none

**************************************************************************/
void suspendWhenSubwooferStreamingLowLatency(uint16 Id);
#endif

/*************************************************************************
NAME
    a2dpAudioSinkMatch

DESCRIPTION
    helper function to enable the client to determine if the specified link
    priority (a2dp_link) is the currently routed audio (sink).

RETURNS
    TRUE if the a2dp_link is the current routed audio.
    otherwise FALSE is returned.

**************************************************************************/
bool a2dpAudioSinkMatch(a2dp_index_t a2dp_link, Sink sink);


bool a2dpA2dpAudioIsRouted(void);

a2dp_index_t a2dpGetRoutedInstanceIndex(void);

/*************************************************************************
NAME
     a2dpSuspendNonRoutedStream

DESCRIPTION
    Special-case of sending an A2DP suspend to an AG when in TWS extended
    mode (an AG connected to each peer).
    The intention is to force the AG to suspend its A2DP stream so that it
    does not intefere with the A2DP stream from the peer device.

    It is a workaround for a known bluetooth bandwidth issue when
    handling > 1 A2DP stream in the 'streaming' state.

RETURNS
    none

**************************************************************************/
#ifdef ENABLE_PEER
void  a2dpSuspendNonRoutedStream(a2dp_index_t index);
#else
#define a2dpSuspendNonRoutedStream(index) ((void)0)
#endif

/*************************************************************************
NAME
    HandlePeerRemoveAuthDevice

DESCRIPTION
    Delete the link key, if there is no peer device connected,
    or another peer device connected

RETURNS
    none

**************************************************************************/
#ifdef ENABLE_PEER
void HandlePeerRemoveAuthDevice(const bdaddr* message);
#endif

#ifdef ENABLE_PEER
/*************************************************************************
NAME
    a2dpStartPauseSuspendTimeout

DESCRIPTION
    Start a timeout after which we will ask the remote device to suspend
    its a2dp media channel.

    We don't send the suspend immediately because not all AG's behave
    well in that situation. Instead we delay it to give the AG time
    to send its own suspend.

RETURNS
    none

**************************************************************************/
void a2dpStartPauseSuspendTimeout(a2dp_index_t index);

/*************************************************************************
NAME
    a2dpCancelPauseSuspendTimeout

DESCRIPTION
    Cancel any outstanding suspend timeout for the given a2dp device.

RETURNS
    none

**************************************************************************/
void a2dpCancelPauseSuspendTimeout(a2dp_index_t index);

#endif

/*************************************************************************
NAME
    sinkA2dpGetA2dpVolumeFromBdaddr

DESCRIPTION
    Retrieve the A2DP volume for the connection to the device with the address specified.

RETURNS
    Returns TRUE if the volume was retrieved, FALSE otherwise.
    The actual volume is returned in the a2dp_volume variable.

**************************************************************************/
bool sinkA2dpGetA2dpVolumeFromBdaddr(const bdaddr *bd_addr, uint16 * const a2dp_volume);

/*************************************************************************
NAME
    sinkA2dpSetA2dpVolumeAtIndex

DESCRIPTION
    Sets the A2DP volume for the connection at the specified index.

**************************************************************************/
void sinkA2dpSetA2dpVolumeAtIndex(uint8 a2dp_index, uint16 a2dp_volume);

/*************************************************************************
NAME
    sinkA2dpSetA2dpAuxVolumeAtIndex

DESCRIPTION
    Sets the A2DP Aux volume for the connection at the specified index.

**************************************************************************/
void sinkA2dpSetA2dpAuxVolumeAtIndex(uint8 a2dp_index, uint16 a2dp_volume);

/*************************************************************************
NAME
    sinkA2dpGetA2dpVolumeInfoAtIndex

DESCRIPTION
    Gets the volume_info structure of the connection at the specified index.

**************************************************************************/
volume_info * sinkA2dpGetA2dpVolumeInfoAtIndex(uint8 a2dp_index);

/*************************************************************************
NAME
    sinkA2dpSetA2dpVolumeInfoAtIndex

DESCRIPTION
    Sets the volume_info structure of the connection at the specified index.

**************************************************************************/
void sinkA2dpSetA2dpVolumeInfoAtIndex(uint8 a2dp_index, volume_info volume);

/*************************************************************************
NAME
    sinkA2dpAudioPrimaryOrSecondarySinkMatch

DESCRIPTION
    Checks the status of the two A2DP streams.

RETURNS
    TRUE if one of them is connected, otherwise FALSE.

**************************************************************************/
bool sinkA2dpAudioPrimaryOrSecondarySinkMatch(Sink sink);

/*************************************************************************
NAME
    sinkA2dpGetStreamState

DESCRIPTION
    Get the stream state for the connected A2DP stream

RETURNS
    a2dp stream state

**************************************************************************/
a2dp_stream_state sinkA2dpGetStreamState(a2dp_index_t index);


/*************************************************************************
NAME
    sinkA2dpGetRoleType

DESCRIPTION
    Get the a2dp media role type

RETURNS
    a2dp role type

**************************************************************************/
a2dp_role_type sinkA2dpGetRoleType(a2dp_index_t index);

/*************************************************************************
NAME
    sinkA2dpGetAudioSink

DESCRIPTION
    Returns the sink at the requested priority index.

**************************************************************************/
Sink sinkA2dpGetAudioSink(a2dp_index_t index);


/*************************************************************************
NAME
    SinkA2dpGetMainVolume

DESCRIPTION
    Get the A2DP Main volume.

RETURNS
    int16

**************************************************************************/
int16 SinkA2dpGetMainVolume(uint8 index);

/*************************************************************************
NAME
    sinkA2dpGetDeviceClass

DESCRIPTION
    Get the A2DP Device Class.

RETURNS
    uint8

**************************************************************************/
uint8 sinkA2dpGetDeviceClass(void);

#ifdef ENABLE_PEER
/*************************************************************************
NAME
    getA2dpPeerLinkLossReconnect

DESCRIPTION
    Returns the status of peer link loss reconnect flag.

RETURNS
    TRUE if peer link loss reconnect is enabled, FALSE otherwise

**************************************************************************/
bool getA2dpPeerLinkLossReconnect(void);

/*************************************************************************
NAME
    getA2dpPeerLinkLossReconnectPtr

DESCRIPTION
    Returns a pointer to a uint16 holding the peer link loss reconnect
    flag, for use with MessageSendConditionally()

RETURNS
    Pointer to the status flag.

**************************************************************************/
uint16 *getA2dpPeerLinkLossReconnectPtr(void);

/*************************************************************************
NAME
    setA2dpPeerLinkLossReconnect

DESCRIPTION
    Sets the status of peer link loss reconnect flag.

RETURNS
    None

**************************************************************************/
void setA2dpPeerLinkLossReconnect(bool enable);

/*************************************************************************
NAME
    getPeerDSPRequiredBufferingLevel

DESCRIPTION
    Returns the A2DP peer DSP required buffering level.

RETURNS
    peer buffer level

**************************************************************************/
peer_buffer_level getPeerDSPRequiredBufferingLevel(uint16 index);

/*************************************************************************
NAME
    setPeerDSPRequiredBufferingLevel

DESCRIPTION
    Sets the A2DP peer DSP required buffering level.

RETURNS
    None

**************************************************************************/
void setPeerDSPRequiredBufferingLevel(uint16 index, peer_buffer_level buffer_level);

#endif

/*************************************************************************
NAME
    getA2dpStatusFlag

DESCRIPTION
    Returns the status of indicated A2DP status flag for the specified index.

RETURNS
    Flag status

**************************************************************************/
bool getA2dpStatusFlag(a2dp_status_flag flag, uint16 index);

/*************************************************************************
NAME
    setA2dpStatusFlag

DESCRIPTION
    Sets the status of indicated A2DP status flag for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpStatusFlag(a2dp_status_flag flag, uint16 index, bool value);

/*************************************************************************
NAME
    getA2dpLinkLossReconnectTimeout

DESCRIPTION
    Returns the A2DP link Loss reconnection timeout.

RETURNS
    A2DP link loss reconnection timeout

**************************************************************************/
uint16 getA2dpLinkLossReconnectTimeout(void);

/*************************************************************************
NAME
    setA2dpLinkLossReconnectTimeout

DESCRIPTION
    Set the A2DP link Loss reconnection timeout.

RETURNS
    bool

**************************************************************************/
bool setA2dpLinkLossReconnectTimeout(uint16 timeout);

/*************************************************************************
NAME
    sinkA2dpGetDefaultVolumeInPercentage

DESCRIPTION
    Returns default A2DP volume level

RETURNS
    Default A2DP volume level in percentage.

**************************************************************************/
uint8 sinkA2dpGetDefaultVolumeInPercentage(void);

/*************************************************************************
NAME
    sinkA2dpSetDefaultVolumeInPercentage

DESCRIPTION
    Updates default A2DP volume level in percentage.

RETURNS
    TRUE if new volume was set correctly, FALSE otherwise.

**************************************************************************/
bool sinkA2dpSetDefaultVolumeInPercentage(uint8 volume);

/*************************************************************************
NAME
    sinkA2dpGetDefaultVolumeInSteps

DESCRIPTION
    Returns default A2DP volume level in steps.

RETURNS
    int16 Default A2DP volume level in steps.

**************************************************************************/
int16 sinkA2dpGetDefaultVolumeInSteps(void);

/*************************************************************************
NAME
    sinkA2dpGetDefaultAuxVolumeInSteps

DESCRIPTION
    Returns default A2DP aux volume level in steps.

RETURNS
    int16 Default A2DP aux volume level in steps.

**************************************************************************/
int16 sinkA2dpGetDefaultAuxVolumeInSteps(void);

/*************************************************************************
NAME
    sinkA2dpGetOptionalCodecsEnabledFlag

DESCRIPTION
    Get the status of A2DP Optional Codecs enabled.

RETURNS
    uint8: Enabled Optional Codecs

**************************************************************************/
uint8 sinkA2dpGetOptionalCodecsEnabledFlag(void);

/*************************************************************************
NAME
    sinkA2dpEnabled

DESCRIPTION
    Check the status of A2DP enable streaming feature bit.

RETURNS
    TRUE if A2DP streaming is enabled, FALSE otherwise

**************************************************************************/
bool sinkA2dpEnabled(void);

/*************************************************************************
NAME
    sinkA2dpOpenMediaOnConnection

DESCRIPTION
    Check the status of A2DP Media Open upon connection feature bit.

RETURNS
    TRUE if A2DP Media can open on connection, FALSE otherwise

**************************************************************************/
bool sinkA2dpOpenMediaOnConnection(void);

/*************************************************************************
NAME
    getA2dpPeerRemoteFeatures

DESCRIPTION
    Returns the remote A2DP peer supported features for the specified index.

RETURNS
    remote_features

**************************************************************************/
remote_features getA2dpPeerRemoteFeatures(uint16 index);

/*************************************************************************
NAME
    setA2dpPeerRemoteFeatures

DESCRIPTION
    Updates the remote A2DP peer supported features for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpPeerRemoteFeatures(uint16 index, remote_features peer_features);

/*************************************************************************
NAME
    getA2dpPeerRemoteDevice

DESCRIPTION
    Returns the remote A2DP peer device for the specified index.

RETURNS
    remote_features

**************************************************************************/
remote_device getA2dpPeerRemoteDevice(uint16 index);

/*************************************************************************
NAME
    setA2dpPeerRemoteDevice

DESCRIPTION
    Updates the remote A2DP peer device for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpPeerRemoteDevice(uint16 index, remote_device peer_device);

/*************************************************************************
NAME
    getA2dpPeerStatus

DESCRIPTION
    Returns the local or remote peer status for the specified index.

RETURNS
    peer status

**************************************************************************/
PeerStatus getA2dpPeerStatus(uint16 index, peer_type peer);

/*************************************************************************
NAME
    setA2dpPeerStatus

DESCRIPTION
    Updates the local or remote peer status for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpPeerStatus(uint16 index, peer_type peer, PeerStatus peer_status);

/*************************************************************************
NAME
    getA2dpLinkRole

DESCRIPTION
    Returns the A2DP link Role for the specified index.

RETURNS
    A2DP link role

**************************************************************************/
a2dp_link_role getA2dpLinkRole(uint16 index);

/*************************************************************************
NAME
    setA2dpLinkRole

DESCRIPTION
    Updates the A2DP link role for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpLinkRole(uint16 index, a2dp_link_role role);

/*************************************************************************
NAME
    getA2dpLinkBdAddr

DESCRIPTION
    Returns the address of A2DP Remote Device bd address for the specified index.

RETURNS
    Remote device bd addr

**************************************************************************/
bdaddr *getA2dpLinkBdAddr(uint16 index);

/*************************************************************************
NAME
    setA2dpLinkBdAddr

DESCRIPTION
    Updates the A2DP Remote Device bd address for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpLinkBdAddr(uint16 index, bdaddr a2dpBdAddr);

/*************************************************************************
NAME
    getA2dpLinkBdAddrForIndex

DESCRIPTION
    Returns the A2DP Remote Device bd address for the specified index.

RETURNS
    Remote device bd addr

**************************************************************************/
bdaddr getA2dpLinkBdAddrForIndex(uint16 index);


/*************************************************************************
NAME
    getA2dpPdlListId

DESCRIPTION
    Returns the A2DP Remote Device list ID in PDL for the specified index.

RETURNS
    Remote device bd addr

**************************************************************************/
uint8 getA2dpPdlListId(uint16 index);

/*************************************************************************
NAME
    setA2dpPdlListId

DESCRIPTION
    Updates the remote device A2DP List ID in PDL for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpPdlListId(uint16 index, uint8 list_id);

/*************************************************************************
NAME
    getA2dpPeerOptionalCodecs

DESCRIPTION
    Returns the A2DP peer optional codecs

RETURNS
    peer optional codecs

**************************************************************************/
uint8 getA2dpPeerOptionalCodecs(peer_type peer);

/*************************************************************************
NAME
    setA2dpPeerOptionalCodecs

DESCRIPTION
    Updates the A2DP peer optional codecs

RETURNS
    None

**************************************************************************/
void setA2dpPeerOptionalCodecs(peer_type peer, uint8 codecs);

/*************************************************************************
NAME
    getA2dpReconfigServiceCaps

DESCRIPTION
    Returns the Remote Reconfig service capabilities used for PTS qualification

RETURNS
    Remote reconfig caps

**************************************************************************/
uint8* getA2dpReconfigServiceCaps(void);

/*************************************************************************
NAME
    setA2dpReconfigServiceCaps

DESCRIPTION
    Stores the Remote Reconfig service capabilities used for PTS qualification

RETURNS
    None

**************************************************************************/
void setA2dpReconfigServiceCaps(uint8 *service_caps);

/*************************************************************************
NAME
    getAudioPluginConnectParams

DESCRIPTION
    Returns the A2DP audio plugin Connect parameters

RETURNS
    A2DP audio Plugin connect parameters

**************************************************************************/
A2dpPluginConnectParams* getAudioPluginConnectParams(void);

/*************************************************************************
NAME
    getAudioPluginModeParams

DESCRIPTION
    Returns the A2DP audio plugin Mode parameters

RETURNS
    A2DP audio Plugin Mode parameters

**************************************************************************/
A2dpPluginModeParams *getAudioPluginModeParams(void);

/*************************************************************************
NAME
    getMusicProcessingMode

DESCRIPTION
    Returns the Audio plugin Music Mode processing

RETURNS
    Audio Plugin Music mode processing

**************************************************************************/
A2DP_MUSIC_PROCESSING_T getMusicProcessingMode(void);

/*************************************************************************
NAME
    setMusicProcessingMode

DESCRIPTION
    Returns the Audio plugin Music Mode processing

RETURNS
    Audio Plugin Music mode processing

**************************************************************************/
void setMusicProcessingMode(A2DP_MUSIC_PROCESSING_T music_processing);

/*************************************************************************
NAME
    getMusicProcessingBypassFlags

DESCRIPTION
    Returns the music processing bypass flags

RETURNS
    Music processing bypass flags

**************************************************************************/
uint16 getMusicProcessingBypassFlags(void);

/*************************************************************************
NAME
    setMusicProcessingBypassFlags

DESCRIPTION
    Updates the music processing bypass flags

RETURNS
    None

**************************************************************************/
void setMusicProcessingBypassFlags(uint16 bypass_flags);

/*************************************************************************
NAME
    getA2dpLinkDataPeerVersion

DESCRIPTION
    Returns the Peer version

RETURNS
   Peer version

**************************************************************************/
uint16 getA2dpLinkDataPeerVersion(uint16 index);

/*************************************************************************
NAME
    setA2dpLinkDataPeerVersion

DESCRIPTION
    Updates Peer Version

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataPeerVersion(uint16 index,uint16 version);

/*************************************************************************
NAME
    getA2dpLinkDataStreamId

DESCRIPTION
    Returns the Steam Id

RETURNS
    Stream ID

**************************************************************************/
uint16 getA2dpLinkDataStreamId(uint16 index);


/*************************************************************************
NAME
    setA2dpLinkDataStreamId

DESCRIPTION
    Updates Stream ID

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataStreamId(uint16 index,uint16 streamind);

/*************************************************************************
NAME
    getA2dpLinkDataDeviceId

DESCRIPTION
    Returns the device ID

RETURNS
    Device ID

**************************************************************************/
uint16 getA2dpLinkDataDeviceId(uint16 index);

/*************************************************************************
NAME
    setA2dpLinkDataDeviceId

DESCRIPTION
    Updates Device ID

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataDeviceId(uint16 index,uint16 deviceid);

/*************************************************************************
NAME
    getA2dpLinkDataSeId

DESCRIPTION
    Returns the SEID

RETURNS
    SEID

**************************************************************************/
uint16 getA2dpLinkDataSeId(uint16 index);

/*************************************************************************
NAME
    setA2dpLinkDataSeId

DESCRIPTION
    Updates Seid

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataSeId(uint16 index,uint16 seid);


/*************************************************************************
NAME
    getA2dpLinkDataLatency

DESCRIPTION
    Returns Data Latency

RETURNS
    Data latency

**************************************************************************/
uint16 getA2dpLinkDataLatency(uint16 index);


/*************************************************************************
NAME
    setA2dpLinkDataLatency

DESCRIPTION
    Updates Data latency

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataLatency(uint16 index,uint16 latency);


/*************************************************************************
NAME
    getA2dpLinkDataClockMismatchRate

DESCRIPTION
    Returns Clock Mismatch rate

RETURNS
    Clock mismatch rate

**************************************************************************/
uint16 getA2dpLinkDataClockMismatchRate(uint16 index) ;

/*************************************************************************
NAME
    setA2dpLinkDataClockMismatchRate

DESCRIPTION
    Updates Clock Mismatch Rate

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataClockMismatchRate(uint16 index,uint16 clkrate);


/*************************************************************************
NAME
    getTWSRoutingMode

DESCRIPTION
    Returns the Peer TWS audio routing mode

RETURNS
    peer TWS audio routing mode

**************************************************************************/
PeerTwsAudioRouting getTWSRoutingMode(tws_device_type peer_mode);

/*************************************************************************
NAME
    setTWSRoutingMode

DESCRIPTION
    Updates the Peer TWS audio routing mode

RETURNS
    None

**************************************************************************/
void setTWSRoutingMode(tws_device_type peer_mode, PeerTwsAudioRouting routing_mode);

/*************************************************************************
NAME
    setExternalMic

DESCRIPTION
    Updates the External mic setting

RETURNS
    None

**************************************************************************/
void setExternalMic(uint8 external_mic);

/*************************************************************************
NAME
    updateMicMuteState

DESCRIPTION
    Updates the External mic mute state for back channel enabled streaming (such as APTX-ll)

RETURNS
    None

**************************************************************************/
void updateMicMuteState(uint8 state);

/****************************************************************************
NAME
    sinkA2dpSetA2dpDataFromSessionData

DESCRIPTION
    Sets A2DP related data using stored session data values.

RETURNS
    None

**************************************************************************/
void sinkA2dpSetA2dpDataFromSessionData(void);

/****************************************************************************
NAME
    sinkA2dpSetSessionDataFromA2dpData

DESCRIPTION
    Sets A2DP related session data using current data values.

RETURNS
    None

**************************************************************************/
void sinkA2dpSetSessionDataFromA2dpData(void);

#ifdef PEER_SCATTERNET_DEBUG   /* Scatternet debugging only */
/*************************************************************************
NAME
    sinkA2dpIsAgRoleToBeInverted

DESCRIPTION
    Returns the status of Ag Role invert flag.

RETURNS
    TRUE: If set, False otherwise

**************************************************************************/
bool sinkA2dpIsAgRoleToBeInverted(uint16 index);

#endif /* PEER_SCATTERNET_DEBUG*/

#ifdef ENABLE_AVRCP
/*************************************************************************
NAME
    getAgAvrcpSupportStatus

DESCRIPTION
    Returns the status of AVRCP support on AG

RETURNS
    avrcp support status

**************************************************************************/
avrcpSupport getAgAvrcpSupportStatus(uint16 index);

/*************************************************************************
NAME
    setAgAvrcpSupportStatus

DESCRIPTION
    Updates the AVRCP support status of the AG

RETURNS
    None

**************************************************************************/
void setAgAvrcpSupportStatus(uint16 index, avrcpSupport support);
#endif

uint16 sinkA2dpGetAudioEnhancements(void);

void sinkA2dpUpdateAudioEnhancements(uint8 * audio_enhancements);

/*************************************************************************
NAME
    sinkA2dpIsA2dpSinkPlaying

DESCRIPTION
    Returns whether the a2dp sink at the given index is considered
    to be in a 'playing' state

RETURNS
    TRUE if playing, else FALSE

**************************************************************************/
bool sinkA2dpIsA2dpSinkPlaying(a2dp_index_t a2dp_index);

/****************************************************************************
NAME
    sinkA2dpIndicateCodecExit

DESCRIPTION
    Sends an event to indicate which A2DP codec has exit

RETURNS
    void
*/

void sinkA2dpIndicateCodecExit(uint8 seid);

/*************************************************************************
NAME
    sinkA2dpIsA2dpSinkRoutable

DESCRIPTION
    returns whether the a2dp sink at the given index is considered
    to be in a state ready for audio routing

RETURNS
    TRUE if routable, else FALSE

**************************************************************************/
bool sinkA2dpIsA2dpSinkRoutable(a2dp_index_t a2dp_index);

/*************************************************************************
NAME
    sinkA2dpSetSpeakerPeqBypass

DESCRIPTION
    Returns whether Speaker PEQ bypass is enabled or not

RETURNS
    TRUE if Speaker PEQ bypass is enabled, else FALSE
**************************************************************************/
bool sinkA2dpGetSpeakerPeqBypass(void);

/****************************************************************************
NAME    
    sinkA2dpSignallingConnectRequest
    
DESCRIPTION
    Issue an A2DP Signalling Connect request and increment paging count
    
PARAMETERS
    addr    The Bluetooth device address of the remote device.
    
RETURNS
    void
*/
void sinkA2dpSignallingConnectRequest(const bdaddr *addr);

/*************************************************************************
DESCRIPTION
    Call sinkA2dpRenegotiateCodecsIfNotSupported() for A2DP sources which are
    connected. Sources doesn't need be streaming.
**************************************************************************/
void sinkA2dpRenegotiateCodecsForAllSources(void);

/*************************************************************************
DESCRIPTION
    If source corresponding to a2dp_source_index is using
    unsupported codec then media channel is closed to trigger
    reconnection and to enable renegotiation of codecs.

PARAMETERS
    a2dp_source_index A2DP source index

RETURNS
    TRUE if disconnection was triggered.
**************************************************************************/
bool sinkA2dpRenegotiateCodecsIfNotSupported(a2dp_index_t a2dp_source_index);

/*************************************************************************
DESCRIPTION
    Checks if source corresponding to a2dp_source_index is using
    unsupported sep (codec).

PARAMETERS
    a2dp_source_index A2DP source index

RETURNS
    TRUE if source has unsupported sep.
**************************************************************************/
bool sinkA2dpSourceHasUnsupportedCodec(a2dp_index_t a2dp_source_index);

/*************************************************************************
DESCRIPTION
    Restricts A2DP codecs to codecs listed in supported_codecs parameter.
    Codecs which which are not in supported_codecs are disabled.

**************************************************************************/
void sinkA2dpRestrictSupportedCodecs(uint16 supported_codecs);

/*************************************************************************
DESCRIPTION
    Returns codec bit mask for given seid.

RETURNS
    Mask with codec bit set or 0 if there is no seid.

**************************************************************************/
uint16 sinkA2dpGetCodecBitMaskFromSeid(uint16 seid);

/*************************************************************************
DESCRIPTION
    Enable all the supported codecs.

RETURNS
    None.

**************************************************************************/
void sinkA2dpEnableAllSupportedCodecs(void);

/*************************************************************************
DESCRIPTION
    Check if A2DP streaming is allowed:
        Involves checking Upgrade in progress

RETURNS
    TRUE if A2DP streaming is allowed, FALSE if not allowed

**************************************************************************/
bool sinkA2dpIsStreamingAllowed(void);

/*************************************************************************
DESCRIPTION
    Suspends all A2DP media streams including peer

RETURNS
    None.

**************************************************************************/

void sinkA2dpSuspendAll(void);

/*************************************************************************
DESCRIPTION
    Suspends all A2DP madia streams including peer ensuring that the A2DP
    stream is paused on resume

RETURNS
    None.

**************************************************************************/

void sinkA2dpSuspendAllNoAutomaticResume(void);


/*************************************************************************
DESCRIPTION
    Get the suspend state for a given connection

PARAMS
    The index of the A2DP connection

RETURNS
    The suspend state

**************************************************************************/
a2dp_suspend_state SinkA2dpGetSuspendState(uint16 index);

/*************************************************************************
DESCRIPTION
    Used to resume peer session

RETURNS
    None
**************************************************************************/
#ifdef ENABLE_PEER
void sinkA2dpResumePeerSession(void);
#else
#define sinkA2dpResumePeerSession()
#endif

#endif /* _SINK_A2DP_INIT_H_ */



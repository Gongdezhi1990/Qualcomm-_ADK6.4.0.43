/*
Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file    sink_peer.h
@ingroup sink_app
@brief   Interface to the Peer device functionality. 
*/

#ifndef _SINK_PEER_H_
#define _SINK_PEER_H_

#include <bdaddr.h>
#include <pio_common.h>

#include "audio_plugin_if.h"
#include "sink_avrcp.h"
#include "sink_inquiry.h"
#include "sink_volume.h"
#include "sink_wired.h"

/* Declare structure. Header file dependencies preclude including */
struct sink_attributes_str;


/* SDP Attribute UUIDs */
#define UUID_SERVICE_CLASS_ID_LIST (0x0001)
#define UUID_SUPPORTED_CODECS_SHAREME (0xFF01)
#define UUID_SUPPORTED_CODECS_TWS (0xFF02)
#define UUID_SUPPORTED_FEATURES (0x0311)
#define UUID_LEGACY_VERSION (0x0200)
#define UUID_CURRENT_VERSION (0x0201)

#define PEER_DEVICE_VERSION                 0x400   /* Major.Minor.Patch */
#define PEER_DEVICE_LEGACY_VERSION          0x330   /* Major.Minor.Patch */


typedef enum
{
    PEER_PAIRING_MODE_TEMPORARY,
    PEER_PAIRING_MODE_NORMAL,
    PEER_PAIRING_MODE_PERMANENT
} PeerPairingMode;

typedef enum
{
    RELAY_SOURCE_NULL,      /* There is no source active that is providing audio to be relayed */
    RELAY_SOURCE_ANALOGUE,  /* Wired analogue is acting as the source for relayed audio data */
    RELAY_SOURCE_USB,       /* USB is acting as the source for relayed audio data */
    RELAY_SOURCE_A2DP       /* Bluetooth AV Source, i.e. an AG, is acting as the source for relayed audio data */
} RelaySource;

typedef enum
{
    RELAY_STATE_IDLE,
    RELAY_STATE_CLOSING,
    RELAY_STATE_OPENING,
    RELAY_STATE_OPEN,
    RELAY_STATE_SUSPENDING,
    RELAY_STATE_STARTING,
    RELAY_STATE_STREAMING
} RelayState;

typedef enum
{
    RELAY_EVENT_DISCONNECTED,
    RELAY_EVENT_CONNECTED,
    RELAY_EVENT_CLOSED,
    RELAY_EVENT_OPEN,
    RELAY_EVENT_OPENING,
    RELAY_EVENT_NOT_OPENED,
    RELAY_EVENT_OPENED,
    RELAY_EVENT_CLOSE,
    RELAY_EVENT_SUSPENDED,
    RELAY_EVENT_START,
    RELAY_EVENT_STARTING,
    RELAY_EVENT_NOT_STARTED,
    RELAY_EVENT_STARTED,
    RELAY_EVENT_SUSPEND
} RelayEvent;

typedef enum
{
    PEER_STATUS_CHANGE_CALL_INACTIVE     = 0x01,
    PEER_STATUS_CHANGE_CALL_ACTIVE       = 0x02,
    PEER_CALL_STATUS_CHANGE              = 0x03,
    PEER_STATUS_CHANGE_RELAY_UNAVAILABLE = 0x04,
    PEER_STATUS_CHANGE_RELAY_AVAILABLE   = 0x08,
    PEER_RELAY_STATUS_CHANGE             = 0x0C,
    PEER_STATUS_CHANGE_RELAY_CLAIMED     = 0x10,
    PEER_STATUS_CHANGE_RELAY_FREED       = 0x20,
    PEER_RELAY_OWNERSHIP_CHANGE          = 0x30
} PeerStatusChange;

typedef enum
{
    PEER_STATUS_IN_CALL    = 0x01,
    PEER_STATUS_DONT_RELAY = 0x02,
    PEER_STATUS_OWNS_RELAY = 0x04,
    PEER_STATUS_POWER_OFF = 0x08
} PeerStatus;

typedef enum
{
    PEER_TWS_ROUTING_STEREO,
    PEER_TWS_ROUTING_LEFT,
    PEER_TWS_ROUTING_RIGHT,
    PEER_TWS_ROUTING_DMIX
} PeerTwsAudioRouting;

typedef enum
{
    PEER_FIXED_ROUTING_NONE,
    PEER_FIXED_ROUTING_LEFT,
    PEER_FIXED_ROUTING_RIGHT
} PeerFixedAudioRouting;

typedef enum
{
    PEER_TRIM_VOL_NO_CHANGE,
    PEER_TRIM_VOL_MASTER_UP ,
    PEER_TRIM_VOL_MASTER_DOWN,
    PEER_TRIM_VOL_SLAVE_UP,
    PEER_TRIM_VOL_SLAVE_DOWN
}PeerTrimVolChangeCmd;

typedef enum
{
    PEER_VOLUME_MIN = 0,
    PEER_VOLUME_MAX
}peer_volume_max_min;

typedef enum
{
    PEER_SINGLE_DEVICE_MODE_DISABLED,
    PEER_SINGLE_DEVICE_MODE_FULL,
    PEER_SINGLE_DEVICE_MODE_NO_POWER_OFF
} PeerSingleDeviceMode;

typedef enum
{
    PEER_TWS_FORCED_DOWNMIX_MODE_DISABLED,
    PEER_TWS_FORCED_DOWNMIX_MODE_OFF,
    PEER_TWS_FORCED_DOWNMIX_MODE_ON
} PeerTwsForcedDownmixMode;

typedef enum
{
    remote_single_channel_rendering,
    remote_multi_channel_rendering
}PeerRemoteRenderingMode;

#define PEER_UNLOCK_DELAY 2000

#ifdef DEBUG_PEER_SM
#define PEER_UPDATE_REQUIRED_RELAY_STATE(str)  {DEBUG(("----\nPEER: Update[%s]\n",str)); peerUpdateRequiredRelayState();}
#else
#define PEER_UPDATE_REQUIRED_RELAY_STATE(str)  {peerUpdateRequiredRelayState();}
#endif

/*************************************************************************
NAME    
    peerRequestServiceRecord
    
DESCRIPTION
    Issues a request to obtain the attributes of a Peer Device SDP record from
    the specified device

RETURNS
    TRUE if a search requested, FALSE otherwise
    
**************************************************************************/
bool RequestRemotePeerServiceRecord (Task task, const bdaddr *bd_addr);

/*************************************************************************
NAME    
    peerSetRemoteSupportedCodecsFromServiceRecord
    
DESCRIPTION
    Extracts and stores the list of optional codecs supported by the remote peer device
    from the supplied Peer Device SDP record

RETURNS
    None
**************************************************************************/
void peerSetRemoteSupportedCodecsFromServiceRecord (const uint8 *attr_data, uint16 attr_data_size);

/*************************************************************************
NAME    
    peerGetRemoteVersion
    
DESCRIPTION
    Extracts the Peer Device version number from the supplied SDP record.
    
RETURNS
    The Peer Device support version number of a connected Peer, or 0 if not
    present in SDP record
    
**************************************************************************/
uint16 peerGetRemoteVersionFromServiceRecord (const uint8 *attr_data, uint16 attr_data_size);

/*************************************************************************
NAME    
    peerGetRemoteSupportedFeatures
    
DESCRIPTION
    Extracts the Peer Device supported features from the supplied SDP record.
    
RETURNS
    The Peer Device supported features of a connected Peer, or 0 if not
    present in SDP record
    
**************************************************************************/
remote_features peerGetRemoteSupportedFeaturesFromServiceRecord (const uint8 *attr_data, uint16 attr_data_size);

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
remote_features peerDetermineCompatibleRemoteFeatures (remote_features supported_features, uint16 version);

/*************************************************************************
NAME    
    peerIsCompatibleDevice
    
DESCRIPTION
    Uses the Peer version number and rendering mode info to determine if another Peer device is compatible

RETURNS
    TRUE if device is deemed compatible, FALSE otherwise
    	
**************************************************************************/
bool peerIsCompatibleDevice (uint16 version);

/*************************************************************************
NAME    
    peerSetRemoteRenderingMode
    
DESCRIPTION
    sets the remote device renedering mode - Single channel / remote channel

RETURNS
    -
**************************************************************************/
void peerSetRemoteRenderingMode(PeerRemoteRenderingMode rendering_mode);

/*************************************************************************
NAME
    peerCheckRemoteRenderingMode

DESCRIPTION
    Determine if mono channel is supported in the peer device
RETURNS
    True if mono device support mono channel,FALSE otherwise
*************************************************************************/
 
PeerRemoteRenderingMode peerCheckRemoteRenderingMode(remote_features features);

/*************************************************************************
NAME    
    peerInit
    
DESCRIPTION
    Register Peer Device SDP record and initialize peer data structure and states.

RETURNS
    None
    
**************************************************************************/
void peerInit( void );

/****************************************************************************
NAME    
    peerCredentialsRequest
    
DESCRIPTION
    Request Peer device credentials (device id and/or peer device SDP record)
    from the specified device
    
RETURNS
    TRUE if credentials were requested, FALSE otherwise
*/
bool peerCredentialsRequest (bdaddr *device_addr);

/*************************************************************************
NAME    
    peerGetPeerSink
    
DESCRIPTION
    Obtains sink to relay channel

RETURNS
    Handle to relay channel, NULL otherwise
    
**************************************************************************/
Sink peerGetPeerSink (void);

/*************************************************************************
NAME    
    peerGetSourceSink
    
DESCRIPTION
    Obtains sink of the current source

RETURNS
    Handle to sink if there is a current streaming source, NULL otherwise
    
**************************************************************************/
Sink peerGetSourceSink (void);

/*************************************************************************
NAME    
    updateAudioGating
    
DESCRIPTION
    Updates gating used for "multimedia" (non-sco) types of audio based on
    call status of any connected Peer and audio source being relayed

RETURNS
    None
    
**************************************************************************/
void updateAudioGating (void);

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
void peerAdvanceRelayState (RelayEvent relay_event);

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
void peerUpdateRequiredRelayState (void);

/*************************************************************************
NAME    
    peerIsRelayAvailable
    
DESCRIPTION
    Determines if relay channel is available for use

RETURNS
    TRUE if relay available, FALSE otherwise
    
**************************************************************************/
bool peerIsRelayAvailable (void);

/*************************************************************************
NAME    
    peerCheckSource
    
DESCRIPTION
    Determines if the provided source can be relayed

RETURNS
    TRUE if relay available, FALSE otherwise
    
**************************************************************************/

bool peerCheckSource(RelaySource new_source);

/*************************************************************************
NAME    
    peerUpdateLocalStatusChange
    
DESCRIPTION
    Updates the local relay availability and issues a status change to a connected Peer

RETURNS
    None
    
**************************************************************************/
void peerUpdateLocalStatusChange (PeerStatusChange peer_status_change);

/*************************************************************************
NAME    
    peerUpdateLocalStatusChangeNoKick
    
DESCRIPTION
    Updates the local relay availability but doesn't kick the state machine
    which would cause additional actions to be taken

RETURNS
    None
    
**************************************************************************/
void peerUpdateLocalStatusChangeNoKick(PeerStatusChange peer_status_change);

/*************************************************************************
NAME    
    peerHandleStatusChangeCmd
    
DESCRIPTION
    Handles a relay availability status change from a Peer

RETURNS
    None
    
**************************************************************************/
void peerHandleStatusChangeCmd (PeerStatusChange peer_status_change);

/*************************************************************************
NAME    
    peerHandleAudioRoutingCmd
    
DESCRIPTION
    Handles an audio routing notification from a Peer

RETURNS
    None
    
**************************************************************************/
void peerHandleAudioRoutingCmd (PeerTwsAudioRouting master_routing_mode, PeerTwsAudioRouting slave_routing_mode);

/*************************************************************************
NAME    
    peerHandleVolumeLowBattCmd
    
DESCRIPTION
    Handles the volume changes in response to the peers low battery command

RETURNS
    None
    
**************************************************************************/
void peerHandleVolumeLowBattCmd (uint8 volume);

/*************************************************************************
NAME    
    peerHandleAncEvent

DESCRIPTION
    Handle an ANC event change notification from a Peer

RETURNS
    None

**************************************************************************/
void peerHandleAncEvent (uint8 ancReceivedEvent);

/*************************************************************************
NAME    
    peerSendAncState

DESCRIPTION
    Send ANC state on TWS connection

RETURNS
    None

**************************************************************************/
#ifdef PEER_TWS
void peerSendAncState (void);
#else
#define peerSendAncState() ((void)(0))
#endif

/*************************************************************************
NAME
    peerSendAncMode

DESCRIPTION
    Send ANC mode on TWS connection

RETURNS
    None

**************************************************************************/
#ifdef PEER_TWS
void peerSendAncMode (void);
#else
#define peerSendAncMode() ((void)(0))
#endif
/*************************************************************************
NAME
    peerUpdateMasterTrimVolume
    
DESCRIPTION
    Handle a trim volume change notification from a Peer : TWS slave

RETURNS
    None
    
**************************************************************************/
void peerUpdateTrimVolume(PeerTrimVolChangeCmd cmd);

/*************************************************************************
NAME    
    peerSendAudioEnhancements
    
DESCRIPTION
    Sends audio enhancement setting to currently connected Peer

RETURNS
    None
    
**************************************************************************/
void peerSendAudioEnhancements(void);

/*************************************************************************
NAME    
    peerSendEqSettings
    
DESCRIPTION
    Sends DSP EQ setting to currently connected Peer

RETURNS
    None
    
**************************************************************************/
void peerSendUserEqSettings(void);

/*************************************************************************
NAME    
    peerRequestUserEqSetings
    
DESCRIPTION
    Request current DSP EQ setting from the connected Peer (Master)

RETURNS
    None
    
**************************************************************************/
void peerRequestUserEqSetings(void);

/*************************************************************************
NAME    
    peerConnectPeer
    
DESCRIPTION
    Attempts to connect a TWS Peer, if not already in a Peer session

RETURNS
    TRUE if a connection is attempted, FALSE otherwise
    
**************************************************************************/
bool peerConnectPeer (void);

/*************************************************************************
NAME    
    peerObtainPairingMode
    
DESCRIPTION
    Obtains the pairing mode used for the currently connected Peer device

RETURNS
    None
    
**************************************************************************/
PeerPairingMode peerObtainPairingMode (uint16 peer_id);

/*************************************************************************
NAME    
    peerUpdatePairing
    
DESCRIPTION
    Ensures permanent pairing data is updated

RETURNS
    None
    
**************************************************************************/
void peerUpdatePairing (uint16 peer_id, const struct sink_attributes_str *peer_attributes);

/*************************************************************************
NAME    
    peerHandleRemoteAGConnected
    
DESCRIPTION
    Checks if the sink device is already connected to an AG with the same bd address as the one connected to the peer,
    if yes, then disconnect the local AG connected if the bd addr of the sink device is lower than that of peer.

RETURNS
    None
    
**************************************************************************/
void peerHandleRemoteAgConnected (void);

/*************************************************************************
NAME    
    peerCompareBdAddr
    
DESCRIPTION
    Compares the first and the second bdaddr.
RETURNS
    TRUE if the first bd addr is greater than second, otherwise returns FALSE.
    
**************************************************************************/
bool peerCompareBdAddr(const bdaddr* first , const bdaddr* second);


/*************************************************************************
NAME    
    peerIsThisDeviceTwsMaster
    
DESCRIPTION
    Is the device a TWS Master
RETURNS
    TRUE if the device is a streaming TWS Master, otherwise returns FALSE.
    
**************************************************************************/
#if defined ENABLE_PEER && defined(PEER_TWS)
bool peerIsThisDeviceTwsMaster(void);
#else
#define peerIsThisDeviceTwsMaster() (FALSE)
#endif

/*************************************************************************
NAME    
    peerIsTWSPeer
    
DESCRIPTION
     Determines whether given Remote peer_id has a TWS Features or not (i.e is Audio Share)
RETURNS
    TRUE indicates TWS, FALSE indicates ShareMe
    
**************************************************************************/
bool peerIsTWSPeer(uint16 peer_id);



/*************************************************************************
NAME    
    peerIsThisDeviceTwsSlave

DESCRIPTION
    Is the device a TWS Slave
RETURNS
    TRUE if the device is a streaming TWS Slave, otherwise returns FALSE.

**************************************************************************/
#if defined ENABLE_PEER && defined(PEER_TWS)
bool peerIsThisDeviceTwsSlave(void);
#else
#define peerIsThisDeviceTwsSlave() (FALSE)
#endif

/*************************************************************************
NAME
    peerIsPeerStreaming
    
DESCRIPTION
    does this device currently have a Peer streaming connection?

RETURNS
    TRUE or FALSE
    
**************************************************************************/
#if defined ENABLE_PEER
bool peerIsPeerStreaming(void);
#else
#define peerIsPeerStreaming() (FALSE)
#endif

/*************************************************************************
NAME
    peerIsLinkMaster

DESCRIPTION
    Check if this device is the link master of a peer connection.
    Regardless of any peer states.

RETURNS
    TRUE if this device is the link master

**************************************************************************/
#ifdef ENABLE_PEER
bool peerIsLinkMaster(void);
#else
#define peerIsLinkMaster() (FALSE)
#endif

/*************************************************************************
NAME
    peerIsLinkMaster

DESCRIPTION
    Check if peer mode is TWS or ShareMe.

RETURNS
    TRUE if peer mode is TWS.

**************************************************************************/
#ifdef ENABLE_PEER
bool peerIsModeTws(void);
#else
#define peerIsModeTws() (FALSE)
#endif

/*************************************************************************
NAME    
    peerClaimRelay
    
DESCRIPTION
    Helper function to update local relay availability(claim or free) and issue status change to Peer

RETURNS
    None
    
**************************************************************************/

void peerClaimRelay(bool claim);

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
bool peerUpdateBatteryLevel(uint16 battery_level);
#else
#define peerUpdateBatteryLevel(x) (FALSE)
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
uint8 peerGetBatteryLevel(void);
#else
#define peerGetBatteryLevel() (BATTERY_LEVEL_INVALID)
#endif


/*************************************************************************
NAME
    updateCurrentState

DESCRIPTION
    Peer state machine helper function to help manage changes to the current state.

RETURNS
    None

**************************************************************************/
void updateCurrentState (RelayState relay_state);

/*************************************************************************
NAME    
    peerGetLocalSupportedCodecs
    
DESCRIPTION
    Returns the list of codec supported by the local peer device

RETURNS
    TRUE if the list is present othewise FALSE
    
**************************************************************************/
bool peerGetLocalSupportedCodecs(uint16 *codecs);

/*************************************************************************
NAME    
    peerGetRemoteSupportedCodecs
    
DESCRIPTION
    Returns the list of codec supported by the remote peer device

RETURNS
    TRUE if the list is present othewise FALSE

    
**************************************************************************/
bool peerGetRemoteSupportedCodecs(uint16 *codecs);

/*************************************************************************
NAME    
    peerSetLocalPeerSupportedCodecs
    
DESCRIPTION
    Stores the list of codec supported by the local peer device

RETURNS
    None
    
**************************************************************************/
void peerSetLocalSupportedCodecs(uint16 codecs);

/*************************************************************************
NAME    
    peerSetRemoteSupportedCodecs
    
DESCRIPTION
    Stores the list of codec supported by the remote peer device

RETURNS
    None
    
**************************************************************************/
void peerSetRemoteSupportedCodecs(uint16 codecs);

/****************************************************************************
NAME    
    peerLinkReservedCanDeviceConnect
    
DESCRIPTION
    Determine whether the given device can be connected based on whether we have reserved a link 
    for a peer device to connect.

RETURNS
    TRUE or FALSE
*/ 
bool peerLinkReservedCanDeviceConnect(const bdaddr *bd_addr);

/****************************************************************************
NAME    
    peerSetLinkReserved
    
DESCRIPTION
    Turns on or off the feature to reserve a link for the peer device to connect.

RETURNS
    void
*/
void peerSetLinkReserved(bool reserved);

/*************************************************************************
NAME    
    peerUpdateTWSMuteState
    
DESCRIPTION
    Responsible for updating the mute state according to the state
    of the peer state machine. Should mute when device is a TWS master
    during relay setup or tear down, or when devices are in idle state
    with no audio source.

RETURNS
    None
    
**************************************************************************/
#ifdef PEER_TWS
void peerUpdateTWSMuteState(void);
#else
#define peerUpdateTWSMuteState() ((void)0)
#endif

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
void peerUpdateLocalStatusOnCallEnd(void);

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
	peer_buffer_level enum 250 or 350ms 
*/
peer_buffer_level getPeerDSPBufferingRequired(uint16 remote_peer_version);

/*************************************************************************
NAME    
    peerAvrcpUpdateActiveConnection
    
DESCRIPTION
    Updates the active AVRCP connection based on what is currently connected.

RETURNS
	TRUE if the supplied AVRCP connection index is that of Peer device, else FALSE

**************************************************************************/
#ifdef ENABLE_PEER
bool peerAvrcpUpdateActiveConnection(uint8 active_avrcp);
#else
#define peerAvrcpUpdateActiveConnection(x) (FALSE)
#endif

/*************************************************************************
NAME    
    peerIsA2dpAudioConnected
    
DESCRIPTION
    Determines whether or not there is a peer with A2DP audio connected

RETURNS
	TRUE if have found a peer device with A2DP audio connected, else FALSE

**************************************************************************/
#ifdef ENABLE_PEER
bool peerIsA2dpAudioConnected(uint8 Index);
#else
#define peerIsA2dpAudioConnected(x) (TRUE)
#endif


/*************************************************************************
NAME
    peerUpdateRelaySource

DESCRIPTION
    Function to match the source selected with the relevant Peer source
    attribute which is essential to be updated along with the Peer
    state machine.

PARAMETERS
    nextSource  Audio Source selection.

RETURNS
	None

*/
#ifdef ENABLE_PEER
void peerUpdateRelaySource(uint16 nextSource);
#else
#define peerUpdateRelaySource(nextSource)
#endif /* ENABLE_PEER */

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
#ifdef ENABLE_PEER
bool peerIsDeviceSlaveAndStreaming(void);
#else
#define peerIsDeviceSlaveAndStreaming() (FALSE)
#endif

/*************************************************************************
NAME    
    PeerIsShareMeSourceEnabled
    
DESCRIPTION
    Check status of Share Me source feature bit.

RETURNS
    TRUE if ShareMe source is enabled, FALSE otherwise

**************************************************************************/
bool PeerIsShareMeSourceEnabled(void);

/*************************************************************************
NAME    
    PeerIsTwsSourceEnabled
    
DESCRIPTION
    Check status of TWS source feature bit.

RETURNS
    TRUE if TWS source is enabled, FALSE otherwise

**************************************************************************/
bool PeerIsTwsSourceEnabled(void);

/*************************************************************************
NAME    
    PeerIsTwsSinkEnabled
    
DESCRIPTION
    Check status of TWS sink feature bit.

RETURNS
    TRUE if TWS sink is enabled, FALSE otherwise

**************************************************************************/
bool PeerIsTwsSinkEnabled(void);

/*************************************************************************
NAME    
    peerUseDeviceIdRecord
    
DESCRIPTION
   Checks if peer devices will use the Device Id record to filter suitable devices.

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerUseDeviceIdRecord(void);

/*************************************************************************
NAME    
    PeerGetConnectionPio
    
DESCRIPTION
   Returns the output PIO configured to indicate the peer connection state

RETURNS
    PIO number

**************************************************************************/
#ifdef ENABLE_PEER
uint8 PeerGetConnectionPio(void);
#else
#define PeerGetConnectionPio() (PIN_INVALID)
#endif

/*************************************************************************
NAME    
    PeerDetermineTwsForcedDownmixMode
    
DESCRIPTION
    Select and apply appropriate Forced Downmix mode based on configured PIO
    
RETURNS
    TRUE if a Forced Downmix mode is in effect (On or Off) after the
    function call, FALSE if the Forced Downmix feature is not in use.
    
**************************************************************************/
#ifdef PEER_TWS
bool PeerDetermineTwsForcedDownmixMode(void);
#else
#define PeerDetermineTwsForcedDownmixMode() (FALSE)
#endif

/*************************************************************************
NAME    
    peerGetTwsForcedDownmixMode
    
DESCRIPTION
    Gets the TWS Forced Downmix mode based on the configuration and
    state of the controlling PIO
    
RETURNS
    Forced Downmix routing mode.
    
**************************************************************************/
#ifdef PEER_TWS
PeerTwsForcedDownmixMode peerGetTwsForcedDownmixMode(void);
#else
#define peerGetTwsForcedDownmixMode() (PEER_TWS_FORCED_DOWNMIX_MODE_DISABLED)
#endif

/*************************************************************************
NAME    
    PeerImplementsTwsForcedDownmix
    
DESCRIPTION
    Determines if the device implements TWS Forced Downmix routing mode.
    
RETURNS
    TRUE if Forced Downmix is implemented, FALSE otherwise
    
**************************************************************************/
#ifdef PEER_TWS
bool PeerImplementsTwsForcedDownmix(void);
#else
#define PeerImplementsTwsForcedDownmix() (FALSE)
#endif

/*************************************************************************
NAME    
    peerUseLiacForPairing
    
DESCRIPTION
   Determines if the peer pairing should use LIAC instead of the GIAC.

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
#ifdef ENABLE_PEER
bool peerUseLiacForPairing(void);
#else
#define peerUseLiacForPairing() (FALSE)
#endif
/*************************************************************************
NAME    
    peerShareMeControlEnabled
    
DESCRIPTION
    Determines if AVRCP commands received from a ShareMe Peer are forwarded to an AV Source 

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerShareMeControlEnabled(void);

/*************************************************************************
NAME    
    PeerReconnectAgOnPeerConnectionEnabled
    
DESCRIPTION
    Determines if BT source is to be disconnected and re-connected upon peer device connection.

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
bool PeerReconnectAgOnPeerConnectionEnabled(void);

/*************************************************************************
NAME    
    peerLinkRecoveryWhileStreamingEnabled
    
DESCRIPTION
    Determines if Master Peer device will attempt to page a Slave Peer during linkloss while streaming .

RETURNS
    TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerLinkRecoveryWhileStreamingEnabled(void);

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
PeerSingleDeviceMode peerGetSingleDeviceMode(void);

/*************************************************************************
NAME    
    peerIsSingleDeviceOperationEnabled
    
DESCRIPTION
    Determines whether devices in TWS session behave as a single unit for AVRCP operation 

RETURNS
	TRUE if have single device operation enabled, else FALSE

**************************************************************************/
bool peerIsSingleDeviceOperationEnabled(void);

/*************************************************************************
NAME    
    peerGetPermittedRoutingModes
    
DESCRIPTION
    Returns the Permitted routing configurations on EventUsrChangeAudioRouting 

RETURNS
	Permitted routing configurations

**************************************************************************/
uint8 peerGetPermittedRoutingModes(void);

/*************************************************************************
NAME    
    peerTwsQualificationEnabled
    
DESCRIPTION
    Returns the status of TWS qualification feature bit

RETURNS
	TRUE if enabled, FALSE otherwise

**************************************************************************/
#ifdef ENABLE_PEER
bool peerTwsQualificationEnabled(void);
#else
#define peerTwsQualificationEnabled() (FALSE)
#endif

/*************************************************************************
NAME    
    sinkDataGetSourceSteamingDelay

DESCRIPTION
    Interfaces for getting source streaming delay value

PARAMS
    void

RETURNS
    void
**************************************************************************/
#ifdef ENABLE_PEER
uint16 sinkDataGetSourceSteamingDelay(void);
#else
#define sinkDataGetSourceSteamingDelay()    (0)
#endif

/*************************************************************************
NAME
    peerGetFixedRoutingMode

DESCRIPTION
    Returns the TWS Fixed Routing configuration

RETURNS
    Fixed routing configuration

**************************************************************************/
PeerFixedAudioRouting peerGetFixedRoutingMode(void);

/*************************************************************************
NAME    
    peerOpenRequestForQualificationEnabled
    
DESCRIPTION
    Returns the status of Flag which indicates App to trigger opening of Peer media channel 

RETURNS
	TRUE if enabled, FALSE otherwise

**************************************************************************/
bool peerOpenRequestForQualificationEnabled(void);

/****************************************************************************
NAME
    peerGetLinkReserved

DESCRIPTION
    Returns the status of Peer link reservation flag.

RETURNS
    TRUE if peer link is reserved, FALSE otherwise

**************************************************************************/
bool peerGetLinkReserved(void);

/****************************************************************************
NAME
    peerCurrentRelayState

DESCRIPTION
    Returns the Current Relay state.

RETURNS
    None

**************************************************************************/
RelayState peerCurrentRelayState(void);

/****************************************************************************
NAME
    peerCurrentRelaySource

DESCRIPTION
    Returns the Current Relay source.

RETURNS
    None

**************************************************************************/
RelaySource peerCurrentRelaySource(void);

/****************************************************************************
NAME
    peerTargetRelayState

DESCRIPTION
    Returns the Target Relay state.

RETURNS
    None

**************************************************************************/
RelayState peerTargetRelayState(void);

/****************************************************************************
NAME
    peerGetTwsSlaveTrim

DESCRIPTION
    Returns  the TWS device trims for slave to be used for volume updates

RETURNS
    TWS slave trim

**************************************************************************/
#if defined ENABLE_PEER && defined PEER_TWS
int16 peerGetTwsSlaveTrim(void);
#else
#define peerGetTwsSlaveTrim()   (0)
#endif

/****************************************************************************
NAME
    peerGetTwsMasterTrim

DESCRIPTION
    Returns  the TWS device trims for master to be used for volume updates

RETURNS
    TWS master trim

**************************************************************************/
#if defined ENABLE_PEER && defined PEER_TWS
int16 peerGetTwsMasterTrim(void);
#else
#define peerGetTwsMasterTrim()  (0)
#endif

/****************************************************************************
NAME
    peerGetRemotePeerAgBdAddress

DESCRIPTION
    Returns the Remote Peer AG BD Address.

RETURNS
    bdaddr*

**************************************************************************/
bdaddr* peerGetRemotePeerAgBdAddress(void);

/****************************************************************************
NAME
    peerSetRemotePeerAudioConnectionStatus

DESCRIPTION
    Set the Remote Peer Audio Connection Status

RETURNS
    void

**************************************************************************/
void peerSetRemotePeerAudioConnectionStatus(uint8 status);

/****************************************************************************
NAME
    peerGetRemotePeerAudioConnectionStatus

DESCRIPTION
    Get the Remote Peer Audio Connection Status

RETURNS
    uint8

**************************************************************************/
uint8 peerGetRemotePeerAudioConnectionStatus(void);

/*************************************************************************
NAME    
    peerSetTwsQualEnablePeerOpen
    
DESCRIPTION
    Set the Peer Tws Qual Enable Peer Open bit

RETURNS
    None
    
**************************************************************************/
void peerSetTwsQualEnablePeerOpen(uint8 value);

/*************************************************************************
NAME    
    peerSetUnlockPeerStateMachine
    
DESCRIPTION
    Set the Peer Unlock Peer State Machine bit

RETURNS
    None
    
**************************************************************************/
void peerSetUnlockPeerStateMachine(uint8 value);

/*************************************************************************
NAME    
    peerSetPeerSource
    
DESCRIPTION
    Set the Peer source

RETURNS
    None
    
**************************************************************************/
void peerSetPeerSource(uint8 new_source);

/*************************************************************************
NAME    
    peerSetPeerUseDeviceId
    
DESCRIPTION
    Set the Peer Use Device Id

RETURNS
    None
    
**************************************************************************/
void peerSetPeerUseDeviceId(uint8 value);

/*************************************************************************
NAME    
    peerPauseIfStreaming
    
DESCRIPTION
    Handle the User Event EventUsrPeerSessionConnDisc

RETURNS
    None
    
**************************************************************************/
void peerPauseIfStreaming(void);

#if defined ENABLE_PEER && defined PEER_TWS

/*************************************************************************
NAME    
    getPeerRequiredStates
    
DESCRIPTION
    Function to get required peer state.

RETURNS
    RelayState
    
**************************************************************************/
RelayState getPeerRequiredStates(void);
#endif

#if defined ENABLE_PEER
/*************************************************************************
NAME    
    peerSetRemotePeerAgBdAddress
    
DESCRIPTION
    Function to to set Remote AG Bd address.

RETURNS
    None
**************************************************************************/
void peerSetRemotePeerAgBdAddress(audio_src_conn_state_t * new_state);
#endif


/****************************************************************************
NAME
    peerRelaySourceStream

DESCRIPTION
    Checks for a Peer relay (source) stream and asks DSP to relay audio from any active AV Source

RETURNS
    true if audio relayed, false otherwise
*/
#ifdef ENABLE_PEER
bool peerRelaySourceStream (void);
#else
#define peerRelaySourceStream(x) (void)(FALSE)
#endif

#ifdef ENABLE_PEER
bool peerIsRemotePeerInCall(void);
#else
#define peerIsRemotePeerInCall() (FALSE)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerPopulatePluginConnectData(AudioPluginFeatures * features, uint16 sample_rate);
#else
#define peerPopulatePluginConnectData(x,y)    ((void)0)
#endif

#ifdef ENABLE_PEER
/*************************************************************************
NAME    
    peerPurgeTemporaryPairing
    
DESCRIPTION
    If in Temporary Pairing mode, remove any peer devices from the PDL

RETURNS
    None
    
**************************************************************************/
void peerPurgeTemporaryPairing(void);
#endif

#ifdef ENABLE_PEER
/*************************************************************************
NAME    
    peerSendAudioRoutingInformationToUser
    
DESCRIPTION
    Indicate the audio channel being used in this PEER device, i.e. LEFT or RIGHT

RETURNS
    None
    
**************************************************************************/
void peerSendAudioRoutingInformationToUser(void);
#endif

/*************************************************************************
NAME    
    peerUpdateAGConectionState
    
DESCRIPTION
    update the state of the AG connection and send info to the remote peer 
	to allow LED indications of separate peer and AG connection states 

RETURNS
    None
    
**************************************************************************/
void peerUpdateAGConectionState(bool ag_connected);

typedef struct
{
    int16 device_trim_master;
    int16 device_trim_slave;
    int16 device_trim_change;
    int16 device_trim_min;
    int16 device_trim_max;
} peer_tws_trim_config_t;

/****************************************************************************
NAME
    peerGetTWSDeviceTrimConfig

DESCRIPTION
    Get the current TWS trim config values.

RETURNS
    TRUE if values read successfully, FALSE otherwise.
*/
#if defined ENABLE_PEER && defined PEER_TWS
bool peerGetTWSDeviceTrimConfig(peer_tws_trim_config_t *trim_config);
#else
#define peerGetTWSDeviceTrimConfig(trim_config) (FALSE)
#endif

/****************************************************************************
NAME
    peerSetTWSDeviceTrimConfig

DESCRIPTION
    Set the current TWS trim config values.

RETURNS
    TRUE if values set successfully, FALSE otherwise.
*/
#if defined ENABLE_PEER && defined PEER_TWS
bool peerSetTWSDeviceTrimConfig(const peer_tws_trim_config_t *trim_config);
#else
#define peerSetTWSDeviceTrimConfig(trim_config) (UNUSED(trim_config),FALSE)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerReceiveSychronisedEvent(uint8 * event_data);
#else
#define peerReceiveSychronisedEvent(x)      ((void)0)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
bool peerProcessEvent(MessageId id);
#else
#define peerProcessEvent(x)     (FALSE)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerAvrcpConnect(void);
#else
#define peerAvrcpConnect()      ((void)0)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerAvrcpDisconnect(void);
#else
#define peerAvrcpDisconnect()   ((void)0)
#endif

/*************************************************************************
NAME
    peerNotifyWiredSourceConnected

DESCRIPTION
    Notifies audio source connection to the remote Peer

RETURNS
    None

**************************************************************************/
#ifdef ENABLE_PEER
void peerNotifyWiredSourceConnected (AudioSrc audio_source);
#else
#define peerNotifyWiredSourceConnected(x) ((void)0)
#endif

/*************************************************************************
NAME
    peerNotifyWiredSourceDisconnected

DESCRIPTION
    Notifies audio source disconnection to the remote Peer

RETURNS
    None

**************************************************************************/
#ifdef ENABLE_PEER
void peerNotifyWiredSourceDisconnected (AudioSrc audio_source);
#else
#define peerNotifyWiredSourceDisconnected(x) ((void)0)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
MessageId peerConvertPeerGeneratedEventToUsrEvent(MessageId id);
#else
#define peerConvertPeerGeneratedEventToUsrEvent(id)     (id)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
bool peerIsA2dpIndexTwsSource(uint8 a2dpIndex);
#else
#define peerIsA2dpIndexTwsSource(x)     (FALSE)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
bool peerSlaveVolumeIsHandledViaMaster(void);
#else
#define peerSlaveVolumeIsHandledViaMaster(x)    (FALSE)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerSignalVolumeUpdateToMaster(const volume_direction direction);
#else
#define peerSignalVolumeUpdateToMaster(x)    ((void)0)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerUpdatePeerMute(bool OnOrOff);
#else
#define peerUpdatePeerMute(x)   ((void)0)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerUpdateLedIndication(bool led_enable);
#else
#define peerUpdateLedIndication(x)     ((void)0)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerSetPeerAudioRouting(PeerTwsAudioRouting master_routing_mode, PeerTwsAudioRouting slave_routing_mode);
#else
#define peerSetPeerAudioRouting(x,y)       ((void)0)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerUpdateEqEnable(uint8 enable);
#else
#define peerUpdateEqEnable(x)       ((void)0)
#endif

/*************************************************************************
NAME    
    peerHandleVolumeLimitCmd
    
DESCRIPTION
    Handle a volume limit notification from a Peer

RETURNS
    None
    
**************************************************************************/
void peerHandleVolumeLimitCmd (peer_volume_max_min max_min);


#if defined ENABLE_PEER && defined PEER_TWS
int16 peerGetTwsTrimForLocalDevice(void);
#else
#define peerGetTwsTrimForLocalDevice()  (0)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
bool peerHandlesVolumeThroughSyncCommands(void);
#else
#define peerHandlesVolumeThroughSyncCommands()  (FALSE)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
void peerSyncVolumeIfMaster(void);
#else
#define peerSyncVolumeIfMaster()  ((void)0)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
bool peerDoesRemotePeerSupportAudioRoutingConfiguration(void);
#else
#define peerDoesRemotePeerSupportAudioRoutingConfiguration()    (FALSE)
#endif

#if defined ENABLE_PEER && defined PEER_TWS
bool peerCanThisTwsDeviceRelayRequestedAudio(void);
#else
#define peerCanThisTwsDeviceRelayRequestedAudio()    (FALSE)
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
#if defined ENABLE_PEER
bool peerDisconnectLowestPriorityAG(void);
#else
#define peerDisconnectLowestPriorityAG()        (FALSE)     /* Feature not enabled. */
#endif

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
#if defined ENABLE_PEER && defined PEER_TWS
void peerDelaySourceStream(void);
#else
#define peerDelaySourceStream() ((void) 0)
#endif

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
#if defined ENABLE_PEER
WIRED_SAMPLING_RATE_T peerGetPeerSampleRateForWiredInput(WIRED_SAMPLING_RATE_T wired_input_sample_rate);
#else
#define peerGetPeerSampleRateForWiredInput(wired_input_sample_rate) (wired_input_sample_rate)
#endif

/***************************************************************************
NAME
    peerUpdateRelayStateOnSuspend

DESCRIPTION
    Update the peer relay state following an A2DP Suspend

PARAMS
    Id of connection

*/
#if defined ENABLE_PEER
void peerUpdateRelayStateOnSuspend(uint16 Id);
#else
#define peerUpdateRelayStateOnSuspend(Id) ((void)(0))
#endif
#endif /* _SINK_PEER_H_ */

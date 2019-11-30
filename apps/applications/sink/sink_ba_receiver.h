/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ba_receiver.h

DESCRIPTION
    Interface File to manage receiver of Broadcast Audio
    
NOTES

*/


#ifndef _SINK_BA_RECEIVER_H_
#define _SINK_BA_RECEIVER_H_

#include "sink_ba.h"
#include "sink_ba_common.h"

#include <gatt_server.h>
#include <gatt_gap_server.h>
#include <gatt_manager.h>

/* Message Base for receiver application . */
#define RECEIVER_MESSAGE_BASE 0x0000
/* Message definitions for internal use within the receiver application. */

/*! Delay to write volume changes to PS. */
#define WRITE_RECEIVER_PSKEY_VOLUME_DELAY    D_SEC(5)

/* Step size for Receiver Volume */
#define RECEIVER_VOLUME_STEP_SIZE (4)
/* Maximum Receiver volume */
#define RECEIVER_MAX_VOLUME (RECEIVER_VOLUME_STEP_SIZE * BA_MAX_STEPS)

/***************************************************************************
NAME
    sinkReceiverInit
 
DESCRIPTION
    This function could be used to initialise broadcast audio receiver.
    It will initialise receiver data structures and internally initialise
    broadcaster lib with broadcaster role as broadcast_role_receiver
 
PARAMS
    Task Task which required to register with Broadcast Lib.
 
RETURNS
    void 
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverInit(Task task);
#else
#define sinkReceiverInit(task) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverHandleBroadcastMessage
 
DESCRIPTION
    Handler for Broadcaster Lib Messages
 
PARAMS
    Task Task which registred with Broadcast Lib.
    id Message ID
    message Messages Strcuture
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverHandleBroadcastMessage(Task task, MessageId id, Message message);
#else
#define sinkReceiverHandleBroadcastMessage(task, id, msg) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverHandleBcmdMessage
 
DESCRIPTION
    Handler for Broacast Cmd Lib Messages
 
PARAMS
    Task Task which registerd with Broadcast Cmd Lib.
    id Message ID
    message Messages Strcuture
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverHandleBcmdMessage(Task task, MessageId id, Message message);
#else
#define sinkReceiverHandleBcmdMessage(task, id, msg) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverStartAssociation
 
DESCRIPTION
    Start the sinkReceiver side Association
 
PARAMS
    void
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverStartAssociation(void);
#else
#define sinkReceiverStartAssociation() ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverPowerOff
 
DESCRIPTION
    Handles the Power Off for Receiver
 
PARAMS
    task Task to which power off complete message need to send
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverPowerOff(Task task);
#else
#define sinkReceiverPowerOff(task) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverWriteVolumePsKey
 
DESCRIPTION
    Stores the Receiver volume PS Store
 
PARAMS
    void
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverWriteVolumePsKey(void);
#else
#define sinkReceiverWriteVolumePsKey() ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverScanForVariantIv
 
DESCRIPTION
    This interface scans for Broadcaster Variant Iv
 
PARAMS
    Task task
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverScanForVariantIv(Task task);
#else
#define sinkReceiverScanForVariantIv(task) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverHandleDMBleMessage
 
DESCRIPTION
    Handler for Receiver Connection Library BLE Messages
 
PARAMS
    ind non-connectable BA advert indication
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverHandleIVAdvert(const CL_DM_BLE_ADVERTISING_REPORT_IND_T* ind);
#else
#define sinkReceiverHandleIVAdvert(ind) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverGetScmInstance
 
DESCRIPTION
    This interface can be used to get broadcast system-wide volume

PARAMS
    void
 
RETURNS
    int16 broadcast system-wide volume
*/
#ifdef ENABLE_BROADCAST_AUDIO
int16 sinkReceiverGetGlobalVolume(void);
#else
#define sinkReceiverGetGlobalVolume() 0
#endif

/***************************************************************************
NAME
    sinkReceiverGetScmInstance
 
DESCRIPTION
    This interface can be used to get the Receiver actual volume

PARAMS
    void
 
RETURNS
    uint16 Actual Receiver volume
*/
#ifdef ENABLE_BROADCAST_AUDIO
uint16 sinkReceiverGetActualVolume(void);
#else
#define sinkReceiverGetActualVolume() 0
#endif

/***************************************************************************
NAME
    sinkReceiverGetLocalVolume
 
DESCRIPTION
    This interface can be used to get Receiver Local Volume Offset

PARAMS
    int16 local volume offset
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
int16 sinkReceiverGetLocalVolume(void);
#else
#define sinkReceiverGetLocalVolume() 0
#endif

/***************************************************************************
NAME
    sinkReceiverSetGlobalVolume
 
DESCRIPTION
    This interface can be used to set broadcast system-wide volume

PARAMS
    int16 volume
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverSetGlobalVolume(int16 volume);
#else
#define sinkReceiverSetGlobalVolume(volume) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverSetLocalVolume
 
DESCRIPTION
    This interface can be used to set Receiver Local Volume Offset

PARAMS
    int16 local volume offset
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverSetLocalVolume(int16 volume);
#else
#define sinkReceiverSetLocalVolume(volume) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverSetActualVolume
 
DESCRIPTION
    This interface can be used to set the Receiver actual volume

PARAMS
    uint16 volume
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverSetActualVolume(uint16 volume);
#else
#define sinkReceiverSetActualVolume(volume) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverResetVariantIv
 
DESCRIPTION
    This interface can be used to reset Receiver Variant IV

PARAMS
    None
 
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverResetVariantIv(void);
#else
#define sinkReceiverResetVariantIv() ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverIsAssociated
 
DESCRIPTION
    This interface can be used to check whether Receiver is already associated

PARAMS
    void
 
RETURNS
    bool TRUE if Receiver is already associated else FALSE
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkReceiverIsAssociated(void);
#else
#define sinkReceiverIsAssociated() (FALSE)
#endif

/***************************************************************************
NAME
    sinkReceiverHandlePowerOn
 
DESCRIPTION
    This interface is used to perform activities after Receiver is Powered
    ON. Starts the Broadcast Receiver

PARAMS
    Task receiver_task
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverHandlePowerOn(Task receiver_task);
#else
#define sinkReceiverHandlePowerOn(task) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverHandleBroadcasterConnectCfm
 
DESCRIPTION
    Interface to allow receiver association for the connected broadcaster
 
PARAMS
    cfm Gatt Managers connection response of broadcaster
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverHandleBroadcasterConnectCfm(GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM_T* cfm);
#else
#define sinkReceiverHandleBroadcasterConnectCfm(cfm) ((void)(0))
#endif

/***************************************************************************
NAME
    receiverGattHandleGattManDisconnectInd
 
DESCRIPTION
    Utility function to Handle indication about attempt to disconnect GATT connection to broadcaster
    
PARAMS
    ind GATT_MANAGER_DISCONNECT_IND_T Message
    
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverHandleBroadcasterDisconnectInd(GATT_MANAGER_DISCONNECT_IND_T* ind);
#else
#define sinkReceiverHandleBroadcasterDisconnectInd(ind) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverIsBroadcasterCid
 
DESCRIPTION
    Utility function to check if the provided CID is that of broadcaster link
 
PARAMS
    cid GATT connection ID of the broadcaster
RETURNS
    TRUE if broadcaster's CID else FALSE
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkReceiverIsBroadcasterCid(uint16 cid);
#else
#define sinkReceiverIsBroadcasterCid(cid) (FALSE)
#endif

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
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverHandleAssocRecvCfm(broadcast_assoc_data* assoc_data);
#else
#define sinkReceiverHandleAssocRecvCfm(assoc_data) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverDeInit
 
DESCRIPTION
    This function could be used to De-initialise broadcast audio receiver.
    It will De-initialise receiver data structures and internally De-initialise
    broadcaster lib

PARAMS
    void
 
RETURNS
    void 
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverDeInit(void);
#else
#define sinkReceiverDeInit() ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverUpStreamMsgHandler
 
DESCRIPTION
    This function is used to Handle BA Receiver Plugin up stream messages

PARAMS
    Task task
    MessageId id
    Message message

RETURNS
    void 
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverUpStreamMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkReceiverUpStreamMsgHandler(task, id, message) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverIsStreaming

DESCRIPTION
    This interface can be used to check whether Receiver is streaming Broadcast Audio

PARAMS
    void

RETURNS
    bool TRUE if Receiver is streaming Broadcast Audio else FALSE
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkReceiverIsStreaming(void);
#else
#define sinkReceiverIsStreaming() (FALSE)
#endif


/***************************************************************************
NAME
    sinkReceiverIsRoutable
 
DESCRIPTION
    Check if the broadcast receiver would be a valid audio input source to route.

RETURNS
    bool TRUE if broadcast audio can be routed as an input, FALSE otherwise.
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkReceiverIsRoutable(void);
#else
#define sinkReceiverIsRoutable() (FALSE)
#endif

/***************************************************************************
NAME
    baAudioSinkMatch
 
DESCRIPTION
    Check if the given sink matches the sink for received broadcast audio.

RETURNS
    bool TRUE if sink matches the broadcast receive sink, FALSE otherwise.
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool baAudioSinkMatch(Sink sink);
#else
#define baAudioSinkMatch(sink) (FALSE)
#endif

/***************************************************************************
NAME
    baAudioIsRouted
 
DESCRIPTION
    Check if the broadcast receiver audio input source is currently routed.

RETURNS
    bool TRUE if broadcast audio is currently routed, FALSE otherwise.
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool baAudioIsRouted(void);
#else
#define baAudioIsRouted() (FALSE)
#endif

/***************************************************************************
NAME
    baPopulateConnectParameters
 
DESCRIPTION
    Populate the given audio connect parameters with the values for
    the broadcast audio input audio plugin.

RETURNS
    bool TRUE if connect_params was populated successfully, FALSE otherwise.
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool baPopulateConnectParameters(audio_connect_parameters *connect_parameters);
#else
#define baPopulateConnectParameters(params) (FALSE)
#endif

/***************************************************************************
NAME
    baAudioPostDisconnectConfiguration
 
DESCRIPTION
    Perform any steps that are needed after broadcast audio input has been 
    disconnected.

    This is called immediately after broadcast audio input has been un-routed.
    The reason for un-routing is not given but common reasons are:
        - A higher priority audio source was routed.
        - Broadcasting was stopped.

RETURNS
    bool TRUE if connect_params was populated successfully, FALSE otherwise.
*/
#ifdef ENABLE_BROADCAST_AUDIO
void baAudioPostDisconnectConfiguration(void);
#else
#define baAudioPostDisconnectConfiguration() ((void)0)
#endif

/***************************************************************************
NAME
    sinkReceiverStartReceiver

DESCRIPTION
    Start listening for incoming broadcast data packets.
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverStartReceiver(void);
#else
#define sinkReceiverStartReceiver() ((void)0)
#endif

/***************************************************************************
NAME
    sinkReceiverStopReceiver

DESCRIPTION
    Stop listening for incoming broadcast data packets.
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverStopReceiver(void);
#else
#define sinkReceiverStopReceiver() ((void)0)
#endif

/***************************************************************************
NAME
    sinkReceiverDestroyReceiver

DESCRIPTION
    Destroy the broadcast receiver instance and related resources.

    Note: In the kymera audio framework this must only be called after the scm
          instance has been disabled and the broadcast packetiser has been
          destroyed. Otherwise there will be a race between disabling scm and
          destroying scm.
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverDestroyReceiver(void);
#else
#define sinkReceiverDestroyReceiver() ((void)0)
#endif

/***************************************************************************
NAME
    sinkReceiverDeleteBroadcasterInfo

DESCRIPTION
    Delete the security information about the broadcaster.
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverDeleteBroadcasterInfo(void);
#else
#define sinkReceiverDeleteBroadcasterInfo() ((void)0)
#endif

/***************************************************************************
NAME
    sinkReceiverInternalStartReceiver

DESCRIPTION
    This function is used start broadcast receiver on internal start receiver request
    If variant is is not set, this fucntion reposts the internal start receiver request on paased
    task
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverInternalStartReceiver(Task task);
#else
#define sinkReceiverInternalStartReceiver(task) ((void)(0))
#endif

/***************************************************************************
NAME
    sinkReceiverInternalDisconnectAudio
 
DESCRIPTION
    Utility function to Disconnect BA Receiver Audio

    Set the BA receiver source TO NULL and then call the audio routing
    framewrok to update the routing. It will disconnect the BS receiver
    plugin if it is no longer needed.

PARAMS
    void

RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkReceiverInternalDisconnectAudio(void);
#else
#define sinkReceiverInternalDisconnectAudio() ((void)0)
#endif

#endif /* _SINK_BA_RECEIVER_H_ */

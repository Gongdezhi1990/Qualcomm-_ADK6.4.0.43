/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ba.h

DESCRIPTION
    Interface File to manage Broadcast Audio Application.This file provide
    a common interface to manage both Broadcaster and Receiver.
    
NOTES

*/

#ifndef _SINK_BA_MANAGER_H_
#define _SINK_BA_MANAGER_H_

#include "sink_events.h"
#include "sink_audio_routing.h"
#include <message.h>
#include <csrtypes.h>
#include <broadcast_context.h>


/*! 
    @brief sink_ba_app_mode_t

    This type defines Broadcast Audio Application Mode.
    sink_ba_appmode_normal is the mode where broadcat audio is not available
    sink_ba_appmode_broadcaster is the mode where BA Broadcaster is Active
    sink_ba_appmode_receiver is the mode where BA Receiver is Active
*/
typedef enum _sink_ba_app_mode_t
{
    sink_ba_appmode_normal,          /*! BA app mode is normal, No Broadcast Audio available */
    sink_ba_appmode_broadcaster,  /*! BA app mode is BA Broadcaster */
    sink_ba_appmode_receiver       /*! BA app mode is BA Receiver */
}sink_ba_app_mode_t;


/* Macro to check if broacast audio is active and is in receiver mode */
#define BA_RECEIVER_MODE_ACTIVE (sink_ba_appmode_receiver == sinkBroadcastAudioGetMode())
/* Macro to check if broacast audio is active and is in receiver mode */
#define BA_BROADCASTER_MODE_ACTIVE (sink_ba_appmode_broadcaster == sinkBroadcastAudioGetMode())

/*!
    @brief sinkBroadcastAudioGetAppTask(). This utility is used
    to return the broadcast audio app task.
 
    @param void 
 
    @return Task BA app task
*/ 
#ifdef ENABLE_BROADCAST_AUDIO
Task sinkBroadcastAudioGetAppTask(void);
#else
#define sinkBroadcastAudioGetAppTask() ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioInitialise(). This interface could be used
    to Initialise Broadcast Audio. BroadCast Audio need to be initialised
    before using any other interfaces except sinkBroadcastAudioConfigure(),
    else unexpected behaviour could occur.
 
    @param void 
 
    @return void
*/ 
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioInitialise(void);
#else
#define sinkBroadcastAudioInitialise() ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioIsActive(). This interface could be used
    to get information on whether the broadcast audio mode is active or not
 
    @param void 
 
    @return bool TRUE if BA is enabled and there is a broadcast mode set,
                 else FALSE
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkBroadcastAudioIsActive(void);
#else
#define sinkBroadcastAudioIsActive() (FALSE)
#endif

/*!
    @brief sinkBroadcastAudioConfigure(). This interface could be used
    to configure broadcast Audio based on last stored configuration in PS store. 
    Broadcast Audio configure Must be invoked before using sinkBroadcastAudioInitialise() API.
 
    @param Void
 
    @return void
*/ 
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioConfigure(void);
#else
#define sinkBroadcastAudioConfigure() ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioSetVolume(). This interface could be used
    to set broadcast Audio Volume
 
    @param volume volume level
    @param volume_in_db Volume in db
 
    @return void
    NOTE: This interface is used to change the volume of broadcaster
*/ 
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioSetVolume(int16 volume, int16 volume_in_db);
#else
#define sinkBroadcastAudioSetVolume(volume, volume_in_db) ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioPowerOff(). This interface is used
    to power off the Broadcast Audio System
 
    @param void 
 
    @return bool TRUE power off is being processed else false
*/  
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioPowerOff(void);
#else
#define sinkBroadcastAudioPowerOff() ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioHandleUserPowerOn(). This interface is used
    to handle user power on in Broadcast Audio mode
 
    @param void 
 
    @return none
*/  
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioHandleUserPowerOn(void);
#else
#define sinkBroadcastAudioHandleUserPowerOn() ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioVolumeUp(). This interface is used
    to increase the Broadcast Audio Volume
 
    @param void 
 
    @return void
*/ 
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioVolumeUp(void);
#else
#define sinkBroadcastAudioVolumeUp() ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioVolumeDown(). This interface is used
    to decrease the Broadcast Audio Volume
 
    @param void 
 
    @return void
*/ 
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioVolumeDown(void);
#else
#define sinkBroadcastAudioVolumeDown() ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioStartAssociation(). This interface could be used
    to invoke association action on broadcast audio
 
    @param id Message ID
    @param message DM BLE messages
 
    @return void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioStartAssociation(void);
#else
#define sinkBroadcastAudioStartAssociation() ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioSetMasterRole(). This interface could be used
    to set the a2dp link role of broadcaster to master. Broadcast Audio need to be initialised
    before using any other interfaces except sinkBroadcastAudioConfigure(),
    else unexpected behavior could occur.
 
    @param device_id a2dp device identifier 
 
    @return void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioSetMasterRole( uint16 device_id);
#else
#define sinkBroadcastAudioSetMasterRole(device_id) ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioChangeMode(). This interface could be used
    to change Modes as per defined in sink_ba_app_mode_t.
    Example :From Normal mode to broadcaster, Broadcaster to Receiver or Receiver to Normal

    @param mode Mode as defined in sink_ba_app_mode_t
    sink_ba_app_mode_t Mode can be either of Normal sink app mode, BA Broadcaster mode
    or BA Receiver mode

    @return bool TRUE if Mode change is success else FALSE
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkBroadcastAudioChangeMode(sink_ba_app_mode_t mode);
#else
#define sinkBroadcastAudioChangeMode(mode) (FALSE)
#endif

/*!
    @brief sinkBroadcastAudioGetHandler(). This interface is used to get the Broadcast Audio
    handler based on the role.
    
    @param void
 
    @return void
*/
#ifdef ENABLE_BROADCAST_AUDIO
Task sinkBroadcastAudioGetHandler(void);
#else
#define sinkBroadcastAudioGetHandler() ((void)(0))
#endif

/*!
    @brief sinkBroadcastAudioPowerOn(). This interface could be used
    to perform BA role specific actions after Power ON.
    @param void
 
    @return void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioPowerOn(void);
#else
#define sinkBroadcastAudioPowerOn() ((void)(0))
#endif

/*!
    @brief Returns mask of optional codecs supported in BA mode.
 
    @return Mask of optional codecs.
*/
#ifdef ENABLE_BROADCAST_AUDIO
uint8 sinkBroadcastAudioGetOptionalCodecsMask(void);
#else
#define sinkBroadcastAudioGetOptionalCodecsMask() (0)
#endif

/*!
    @brief sinkBroadcastAudioGetVolume(). This interface is used
    to get BA volume for the corresponding audio source
    @param audio_source the audio source to get volume for
 
    @return int16 BA volume for the audio source
*/
#ifdef ENABLE_BROADCAST_AUDIO
int16 sinkBroadcastAudioGetVolume(audio_sources audio_source);
#else
#define sinkBroadcastAudioGetVolume(audio_source) (0 * (audio_source))
#endif

/*!
    @brief useBroadcastPlugin(). This interface is used
    to identify if BA specific plugin is to be used or not.
    @param void
 
    @return bool TRUE is BA plugin is used FALSE otherwise
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool useBroadcastPlugin( void );
#else
#define useBroadcastPlugin() (FALSE)
#endif

/*!
    @brief sinkBroadcastAudioGetMode(). This interface could be used
    to get current App Mode (Sink App, Broadcaster or Receiver)
 
    @param void 
 
    @return sink_ba_app_mode_t The BA App Mode defined as per sink_ba_app_mode_t type
*/  
#ifdef ENABLE_BROADCAST_AUDIO
sink_ba_app_mode_t sinkBroadcastAudioGetMode(void);
#else
#define sinkBroadcastAudioGetMode() (sink_ba_appmode_normal)
#endif

/*!
    @brief sinkBroadcastAudioSetMode(). This interface could be used
    to set current App Mode (Sink App, Broadcaster or Receiver)
 
    @param sink_ba_app_mode_t The BA App Mode defined as per sink_ba_app_mode_t type 
 
    @return void
*/  
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioSetMode(sink_ba_app_mode_t mode);
#else
#define sinkBroadcastAudioSetMode(mode) ((void) (0))
#endif

/*!
    @brief sinkBroadcastAudioHandleInitCfm(). This interface is used
    to handle the initialization confirmation of BA library
    @param void
 
    @return None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioHandleInitCfm(void);
#else
#define sinkBroadcastAudioHandleInitCfm() ((void)(0))
#endif

/*!
    @brief sinkBroadcastAudioHandleDeInitCfm(). This interface is used
    to handle the de-initialization confirmation of BA library
    @param mode indicates from which mode de-initialization was complete
 
    @return None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioHandleDeInitCfm(sink_ba_app_mode_t mode);
#else
#define sinkBroadcastAudioHandleDeInitCfm(mode) ((void)(0))
#endif

/***************************************************************************
NAME
    SinkBroadcasAudioGetA2dpLatency
 
DESCRIPTION
    This function is used to retrieve the a2dp audio latency for delay reporting when 
    broadcast audio is active
 
PARAMS
    void
 
RETURNS
    uint16 a2dp latency to use.
*/
#ifdef ENABLE_BROADCAST_AUDIO
uint16 SinkBroadcasAudioGetA2dpLatency(void);
#else
#define SinkBroadcasAudioGetA2dpLatency() (0)
#endif

/*!
    @brief sinkBroadcastAudioStopIVAdvertsAndScan(). This interface is used
    to stop advertising IV packets
    @param None
 
    @return None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioStopIVAdvertsAndScan(void);
#else
#define sinkBroadcastAudioStopIVAdvertsAndScan() ((void)(0))
#endif


/***************************************************************************
NAME
    SinkBroadcastAudioHandlePluginUpStreamMessage
 
DESCRIPTION
    This function is used to Handle BA Plugin up stream messages
 
PARAMS
    Task task
    MessageId id
    Message message
 
RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void SinkBroadcastAudioHandlePluginUpStreamMessage(Task task, MessageId id, Message message);
#else
#define SinkBroadcastAudioHandlePluginUpStreamMessage(task, id, message) ((void)(0))
#endif

/*!
    @brief Checks if a codec currently used by A2DP source
    is supported in broadcaster mode.
    @param a2dp_source_index A2DP source index

    @return TRUE if codec is supported.
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkBroadcastIsA2dpCodecSupported(uint16 a2dp_source_index);
#else
#define sinkBroadcastIsA2dpCodecSupported(a2dp_source_index) (TRUE)
#endif

/*!
    @brief Checks if broadcaster is ready for streaming.

    @return TRUE if it is ready.
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkBroadcastIsReadyForRouting(void);
#else
#define sinkBroadcastIsReadyForRouting() (TRUE)
#endif

/***************************************************************************
NAME
    baAudioPostRoutingAudioConfiguration

DESCRIPTION
    Perform any steps that are needed after a new audio source has been
    routed.

    For example, if no other audio is routed we may need to
    re-start the broadcast receiver (except if the device is powering off).

RETURNS
    bool TRUE if connect_params was populated successfully, FALSE otherwise.
*/
#ifdef ENABLE_BROADCAST_AUDIO
void baAudioPostRoutingAudioConfiguration(void);
#else
#define baAudioPostRoutingAudioConfiguration() ((void)0)
#endif

/*!
    @brief Loads broadcast volume conversion table.
    Only pointer to static configuration is loaded
    to prevent loading whole table into memory.
    To keep pointer valid configuration is not released
    until sinkBroadcastReleaseVolumeTable() is called.
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAcquireVolumeTable(void);
#else
#define sinkBroadcastAcquireVolumeTable() ((void)0)
#endif

/*!
    @brief Releases broadcast volume conversion table.
    See sinkBroadcastAcquireVolumeTable().
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastReleaseVolumeTable(void);
#else
#define sinkBroadcastReleaseVolumeTable() ((void)0)
#endif

/***************************************************************************
NAME
    sinkBroadcastAudioGetAssociationTimeOut
 
DESCRIPTION
    This function is used to get the BA association time out
    Reads the configuration for the timer value. If config read
    fails then returns the default value of 30 seconds.

PARAMS
    void
 
RETURNS
    uint16 BA association time out in seconds
*/
#ifdef ENABLE_BROADCAST_AUDIO
uint16 sinkBroadcastAudioGetAssociationTimeOut(void);
#else
#define sinkBroadcastAudioGetAssociationTimeOut() (0)
#endif

/***************************************************************************
NAME
    sinkBroadcastAudioIsModeChangeInProgress
 
DESCRIPTION
    This function is used to check whether BA mode change is in progress

PARAMS
    void
 
RETURNS
    bool TRUE if BA mode change is in progress else FALSE
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool sinkBroadcastAudioIsModeChangeInProgress(void);
#else
#define sinkBroadcastAudioIsModeChangeInProgress() (FALSE)
#endif

/***************************************************************************
DESCRIPTION
    Set requested erasure coding scheme.

PARAMS
    Erasure coding scheme

RETURNS
    none
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioSetEcScheme(erasure_coding_scheme_t ec_scheme);
#else
#define sinkBroadcastAudioSetEcScheme(ec_scheme) ((void)0)
#endif

/***************************************************************************
DESCRIPTION
    Get requested erasure coding scheme.

PARAMS
    void

RETURNS
    Erasure coding scheme
*/
#ifdef ENABLE_BROADCAST_AUDIO
erasure_coding_scheme_t sinkBroadcastAudioGetEcScheme(void);
#else
#define sinkBroadcastAudioGetEcScheme() (0)
#endif

/***************************************************************************
DESCRIPTION
    Function to store Broadcast Audio Config Items (BA Mode)

PARAMS
    void

RETURNS
    void
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcastAudioStoreConfigItem(void);
#else
#define sinkBroadcastAudioStoreConfigItem() ((void)0)
#endif

#ifdef ENABLE_BROADCAST_AUDIO
void broadcastPacketiserInitialisedCfm (bool status);
#else
#define broadcastPacketiserInitialisedCfm(status) ((void)0)
#endif


#ifdef ENABLE_BROADCAST_AUDIO
void baBroadcastlibStoppedCfm(void);
#else
#define baBroadcastlibStoppedCfm() ((void)0)
#endif

#endif /*_SINK_BA_MANAGER_H_*/


/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef SINK_VOLUME_H
#define SINK_VOLUME_H

#include <audio_plugin_if.h>
#include <audio_data_types.h>
#include <hfp.h>

#include "sink_events.h"


#define VOL_NUM_VOL_SETTINGS     (16)


typedef enum  
{
    increase_volume,
    decrease_volume,
    same_volume
}volume_direction;

typedef enum
{
    tws_none,
    tws_master,
    tws_slave
} tws_device_type;

typedef struct
{
    int16 main_volume;
    int16 aux_volume;
}volume_info;

typedef struct
{
    unsigned inc_vol:4;     /* The volume level to go to when volume up is pressed */
    unsigned dec_vol:4;     /* The volume level to go to when volume down is pressed */
    unsigned tone:8;        /* The tone associated with a given volume level */
    unsigned vol_gain:8;    /* The DAC gain to use for the given volume level */
    unsigned unused:8;
} volume_mapping_t;

#define MAX_A2DP_CONNECTIONS    (2)

#define MICROPHONE_MUTE_ON      (0)
#define MICROPHONE_MUTE_OFF     (10)

#define VOLUME_A2DP_MIN_LEVEL   (0)

#define VOLUME_HFP_MAX_LEVEL    (15)
#define VOLUME_HFP_MIN_LEVEL    (0)

#define VOLUME_NUM_VOICE_STEPS  (16)

#define VOLUME_STEP_MIN         (0)

/* Wait period between USB Audio event creation */
#define VOLUME_USB_EVENT_WAIT (1500)

/****************************************************************************
NAME
 volumeInit

DESCRIPTION
 malloc memory for storing of current volume levels for a2dp, usb, wired and fm
 interfaces

RETURNS
 none

*/
void volumeInit(void);

/****************************************************************************
NAME 
 VolumeCheckA2dpMute

DESCRIPTION
 check whether any a2dp connections are at minimum volume and mutes them properly if they are

RETURNS
 bool   Returns true if stream muted
    
*/
bool VolumeCheckA2dpMute(void);


/****************************************************************************
NAME 
 VolumeSetHeadsetVolume

DESCRIPTION
 sets the internal speaker gain to the level corresponding to the phone volume level.
 sends the volume to the AG if the updateAG flag is TRUE.
    
RETURNS
 void
    
*/
void VolumeSetHeadsetVolume( uint16 pNewVolume , bool pPlayTone, hfp_link_priority priority, const bool updateAG );


/****************************************************************************
NAME 
 VolumeSendAndSetHeadsetVolume

DESCRIPTION
    sets the vol to the level corresponding to the phone volume level
    In addition - send a response to the AG indicating new volume level

RETURNS
 void
    
*/
void VolumeSendAndSetHeadsetVolume( uint16 pNewVolume , bool pPlayTone , hfp_link_priority priority );

/****************************************************************************
NAME 
 VolumeIfAudioRoutedModifyAndUpdateTWSDeviceTrim

DESCRIPTION
 check for an active A2DP, Wired or USB streaming currently routing audio to the device and adjust the trim volume up or down
 as appropriate

RETURNS
 bool   TRUE if volume adjusted, FALSE if no streams found
    
*/
bool VolumeIfAudioRoutedModifyAndUpdateTWSDeviceTrim(volume_direction dir, tws_device_type tws_device);

/****************************************************************************
NAME 
 VolumeModifyAndUpdateTWSDeviceTrim

DESCRIPTION
 Updates the trim volume changes locally in the device.

RETURNS
void
    
*/
void VolumeModifyAndUpdateTWSDeviceTrim(volume_direction dir, tws_device_type tws_device);

/****************************************************************************
NAME 
 VolumeHandleSpeakerGainInd

DESCRIPTION
 Handle speaker gain change indication from the AG

RETURNS
 void
    
*/
void VolumeHandleSpeakerGainInd(const HFP_VOLUME_SYNC_SPEAKER_GAIN_IND_T* ind);

/******************************************************************************
NAME
    VolumeApplySoftMuteStates

DESCRIPTION
    Send the stored soft mute state to the audio library, overriding the output
    mute states if necessary.

RETURNS
    void

*/
void VolumeApplySoftMuteStates(void);

/******************************************************************************
NAME 
    VolumeAllOutputsMuted

DESCRIPTION
    Checks if both main and aux outputs are in a muted state.

RETURNS
    bool
    
*/
bool VolumeAllOutputsMuted(void);

/******************************************************************************
NAME
    VolumeUpdateMuteState

DESCRIPTION
    Update the stored mute state for a specific mute group

RETURNS
    void

*/
void VolumeUpdateMuteState(AUDIO_MUTE_GROUP_T mute_group, AUDIO_MUTE_STATE_T state);

/******************************************************************************
NAME 
    VolumeUpdateMuteStatusAllOutputs

DESCRIPTION
    Manual override to temporarily force muting of all outputs or restore them
    to their previous mute states. This is so that other modules do not need to
    be aware of the distinction between multi-channel output groups ('main' and
    'aux'). They can simply mute everything as they always have done, but using
    the new soft mute interface rather than through mode setting. The stored
    multi-channel mute states can then be automatically restored later.

PARAMETERS
    enable_mute    If TRUE, mute all outputs, else restore previous states.

RETURNS
    void

*/
void VolumeUpdateMuteStatusAllOutputs(bool enable_mute);

/******************************************************************************
NAME 
    VolumeUpdateMuteStatusMicrophone

DESCRIPTION
    Mute or unmute the microphone. Convenience function which would have the
    same effect as calling VolumeUpdateMuteState(audio_mute_group_mic, state);
    the only difference being that it takes a bool parameter. Complementary
    function to VolumeUpdateMuteStatusAllOutputs(), for older code that used to
    use AUDIO_PLUGIN_SET_MODE_MSG to control the mute state of the speaker/mic.

PARAMETERS
    enable_mute    If TRUE, mute microphone, else unmute it.

RETURNS
    void

*/
void VolumeUpdateMuteStatusMicrophone(bool enable_mute);

/******************************************************************************
NAME 
    VolumeSetMicrophoneMute

DESCRIPTION
    Update the stored mute state for the HFP microphone and send it to the DSP.
    Will also mute all outputs if this feature is enabled.

RETURNS
    void

*/
void VolumeSetMicrophoneMute(AUDIO_MUTE_STATE_T state);


/******************************************************************************
NAME 
    sinkVolumeResetMaxAndMinVolumeFlag

DESCRIPTION
    Reset the vol max and vol min falgs to FALSE.

RETURNS
    void

*/
void sinkVolumeResetMaxAndMinVolumeFlag(void);

/******************************************************************************
NAME    
    sinkVolumeSetSessionVolButtonInverted
    
DESCRIPTION
     Interface to set VolButtonInverted flag in session data.For writing session data
    
RETURNS
    void
*/
void sinkVolumeSetSessionVolButtonInverted(void);

/******************************************************************************
NAME    
    sinkVolumeGetSessionVolButtonInverted
    
DESCRIPTION
     Interface to get VolButtonInverted flag from session data. For reading session data
    
RETURNS
    TRUE if set,else FALSE 
*/
bool sinkVolumeGetSessionVolButtonInverted(void);

/****************************************************************************
NAME    
    sinkVolumeSetVolButtonInverted
    
DESCRIPTION
     Set the volume button inverted flag
    
RETURNS
    void
*/
void  sinkVolumeSetVolButtonInverted(bool enable);

/****************************************************************************
NAME    
    sinkVolumeIsVolButtonInverted
    
DESCRIPTION
      Get the volume button inverted flag
    
RETURNS
    void
*/
bool sinkVolumeIsVolButtonInverted(void);

/****************************************************************************
DESCRIPTION
    Set mute or unmute (mic gain of MICROPHONE_MUTE_ON - 0 is mute, all other
    gain settings unmute) of HFP mic.
*/
void VolumeSetHfpMicrophoneGain(hfp_link_priority priority, uint8 mic_gain);


/****************************************************************************
DESCRIPTION
    Set mute or unmute remotely from AG if SyncMic feature bit is enabled
    (mic gain of MICROPHONE_MUTE_ON - 0 is mute, all other gain settings unmute).
*/
void VolumeSetHfpMicrophoneGainCheckMute(hfp_link_priority priority, uint8 mic_gain);


/****************************************************************************
DESCRIPTION
    Sends the current HFP microphone volume to the AG on connection.
*/
void VolumeSendHfpMicrophoneGain(hfp_link_priority priority, uint8 mic_gain);


/****************************************************************************
DESCRIPTION
    Determine whether the mute reminder tone should be played in the device, e.g. #
    if AG1 is in mute state but AG2 is not muted and is the active AG then the mute reminder
    tone will not be played, when AG1 becomes the active AG it will be heard.
*/
bool VolumePlayMuteToneQuery(void);


/****************************************************************************
    Functions to increment/decrement or resend stored group volumes to the
    current audio plugin
*/
void sinkVolumeModifyAndUpdateRoutedAudioMainVolume(const volume_direction direction);
void sinkVolumeModifyAndUpdateRoutedA2DPAudioMainVolume(const volume_direction direction);
void sinkVolumeUpdateRoutedAudioMainAndAuxVolume(void);

/*************************************************************************
NAME    
    storeCurrentSinkVolume
    
DESCRIPTION
    Stores the current volume level of the sink which is streaming audio (HFP or A2DP)

RETURNS

*/
void storeCurrentSinkVolume( void );

/****************************************************************************
    Send new main group volume level to currently loaded plugin
    - updates display, sub and uses previous volume to update dsp
    operating mode
*/
void VolumeSetNewMainVolume(const volume_info * const volumes, const int16 previousVolume);

/*****************************************
    Set the mute status and volume levels.
*/
void VolumeSetupInitialMutesAndVolumes(const volume_info * const volumes);

/****************************************************************************
    Set the initial mute states for audio inputs and outputs
*/
void VolumeSetInitialMuteState(void);

/****************************************************************************
    Processes user and system events specific to volume
*/
bool sinkVolumeProcessEventVolume(const MessageId volume_event);

/****************************************************************************
    Queries whether the given event requires a reset of the source and
    volume data save timer
*/
bool sinkVolumeEventRequiresSessionDataToBeStored(const MessageId volume_event);

/****************************************************************************
    Returns a modified event based on the volume orientation setting
*/
MessageId sinkVolumeModifyEventAccordingToVolumeOrientation(const MessageId volume_event);

/****************************************************************************
    Resets the timer that triggers a saving of the session data
*/
void sinkVolumeResetVolumeAndSourceSaveTimeout(void);
/****************************************************************************
    Get the volume Mapping for CVC volume
*/
uint8 sinkVolumeGetVolumeMappingforCVC(uint16 index);

/******************************************************************************
    Helper function to get volume GroupConfig data based on index  
*/
void sinkVolumeGetGroupConfig(audio_output_group_t group, volume_group_config_t*);

/******************************************************************************
    Helper function to set volume GroupConfig data based on index
*/
bool sinkVolumeSetGroupConfig(audio_output_group_t group, const volume_group_config_t * group_config);

/****************************************************************************
    Get the Group Config Steps as per the group index
*/
int16 sinkVolumeGetGroupConfigSteps(uint16 group_index);

#define sinkVolumeGetNumberOfVolumeSteps(group) (sinkVolumeGetGroupConfigSteps(group))
#define sinkVolumeGetMaxVolumeStep(group) ((sinkVolumeGetNumberOfVolumeSteps(group) - 1))

/****************************************************************************
    Get the volume mapping config for the given volume level.
*/
bool sinkVolumeGetVolumeMappingForLevel(uint16 level, volume_mapping_t *mapping);

/****************************************************************************
    Set the volume mapping config for the given volume level.
*/
bool sinkVolumeSetVolumeMappingForLevel(uint16 level, const volume_mapping_t *mapping);

/******************************************************************************
NAME
    sinkvolumeGetVolumeStoreTimeout

DESCRIPTION
    Helper function to get the volume store timeout 

RETURNS
    uint16 : Timer value.
*/
uint16 sinkvolumeGetVolumeStoreTimeout(void);

/******************************************************************************
NAME
    sinkvolumeSetVolumeStoreTimeout

DESCRIPTION
    Helper function to set the volume store timeout 

PARAMS
    timeout  Timer value.
*/
bool sinkvolumeSetVolumeStoreTimeout(uint16 timeout);

/******************************************************************************
NAME
    sinkVolumeGetCurrentVolumeAsScaledPercentage

DESCRIPTION
    Returns the current volume expressed as a percentage of maximum volume

RETURNS
    A volume expressed as a scaled percentage of maximum volume
*/
scaled_volume_percentage_t sinkVolumeGetCurrentVolumeAsScaledPercentage(void);

/******************************************************************************
NAME
    sinkVolumeSetCurrentVolumeAsScaledPercentage

DESCRIPTION
    Sets the current volume as a scaled percentage of maximum volume unless
    the device is streaming from USB, in which case the volume change is
    converted into either a volume-up or a volume-down button press so that
    the USB audio source can determine the new volume setting.

PARAMS
    volume in percent
    
RETURNS
    Event ID for subsequent processing: button press event if streaming from USVB,
    update main volume otherwise.
*/
MessageId sinkVolumeSetCurrentVolumeAsScaledPercentage(scaled_volume_percentage_t volume);

/******************************************************************************
NAME
    sinkVolumeSetGroupVolumeInInputVolume

DESCRIPTION
    Sets a group volume within a given volume_info

PARAMS
    volume - the volume_info to modify
    group - the group to modify
    volume_step - the volume step value to set
*/
void sinkVolumeSetGroupVolumeInInputVolume(volume_info * const volume, const audio_output_group_t group, int16 volume_step);

/******************************************************************************
NAME
    sinkVolumeGetVolInDb

DESCRIPTION
    Gets the Db eqivalent of the steps provided

PARAMS
    group_vol - volume for which db is required
    group - the group to check against
*/
int16 sinkVolumeGetVolInDb(int16 group_vol, audio_output_group_t group);



/******************************************************************************
NAME
    sinkVolumeGetChargeDeficitMaxVolumeLimit

DESCRIPTION
    Gets the value being used for the charge deficit limitation for max volume.

PARAMS
    None
*/
unsigned sinkVolumeGetChargeDeficitMaxVolumeLimit(void);

/******************************************************************************
NAME
    sinkVolumeGetChargeDeficitMaxVolumeLimit

DESCRIPTION
    Sets the value to be used for the charge deficit limitation for max volume.

PARAMS
    unsigned
*/
void sinkVolumeSetChargeDeficitMaxVolumeLimit(unsigned newLimit);


/******************************************************************************
NAME
    modifyAndUpdateRoutedAudioVolume

DESCRIPTION
    Updates the Routed Audio Volume for the current routed audio source.

PARAMS
    unsigned
*/
void modifyAndUpdateRoutedAudioVolume(volume_direction direction, audio_output_group_t group, const bool unmuteOnChange);

/******************************************************************************
NAME
    volumeGetMuteState

DESCRIPTION
    Get the stored mute state of a mute group

RETURNS
    AUDIO_MUTE_STATE_T of the mute group

*/
AUDIO_MUTE_STATE_T volumeGetMuteState(AUDIO_MUTE_GROUP_T mute_group);

#endif


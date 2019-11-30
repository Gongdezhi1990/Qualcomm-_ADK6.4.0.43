/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    module responsible for Volume control 
*/
#include "sink_main_task.h"
#include "sink_statemanager.h"
#include "sink_volume.h"
#include "sink_tones.h"
#include "sink_pio.h"
#include "sink_slc.h"
#include "sink_hfp_data.h"
#include "sink_audio.h"
#include "sink_wired.h"
#include "sink_display.h"
#include "gain_utils.h"
#include "sink_callmanager.h"
#include "sink_configmanager.h"
#include "sink_avrcp.h"
#include "sink_ba_receiver.h"
#include "sink_debug.h"
#include "sink_usb.h"

#include "sink_volume_config_def.h"
#include "sink_a2dp.h"
#include "sink_events.h"

/* Include config store and definition headers */
#include "config_definition.h"
#include <config_store.h>

#include "sink_peer.h"

#include "sink_fm.h"

#include "sink_swat.h"

#include "sink_accessory.h"

#include <audio.h>
#include <sink.h>
#include <stddef.h>
#include <string.h>
#include <message.h>

#ifdef DEBUG_VOLUME
#define VOL_DEBUG(x) DEBUG(x)
#else
#define VOL_DEBUG(x) 
#endif

#define ARRAY_COUNT(array) (sizeof((array)) / sizeof((array)[0]))

#define VOLUME_NUM_VOLUME_MAPPINGS (ARRAY_COUNT(((sink_volume_writeable_config_def_t *)0)->gVolMaps))

/* Referance to Gloabal data for volume module  */
typedef struct __sink_volume_globaldata_t
{
    vol_mod_gain_curve_t volgroup_config[audio_output_group_all];
    unsigned mute_all_outputs:1;
    unsigned mute_states:3;
    unsigned vol_at_max:1;
    unsigned vol_at_min:1;  
    unsigned gVolButtonsInverted:1;
    unsigned _spare_:9;
    unsigned cdl_max_volume; /* maximum volume for charging deficit limitation */
}sink_volume_globaldata_t;

/* Global data strcuture element for sink volume */
static sink_volume_globaldata_t gSinkVolume;
#define GSINK_VOLUME_DATA  gSinkVolume


static void getVolumeInfoFromAudioSource(const audio_sources source, volume_info * volume);

#if defined (APTX_LL_BACK_CHANNEL) || defined(INCLUDE_FASTSTREAM)
static void VolumeSetA2dpMicrophoneGain(AUDIO_MUTE_STATE_T state);
#endif /* defined (APTX_LL_BACK_CHANNEL) || defined(INCLUDE_FASTSTREAM)*/

/****************************************************************************
 Helper to decide if sync request from AG affects our gain settings 

 */
static bool volumeSyncAffectsGain(hfp_link_priority priority)
{
    Sink sink;
    /* Match active sink against HFP sink */
    if(HfpLinkGetAudioSink(priority, &sink))
        if( sinkAudioIsVoiceRouted() && sinkAudioGetRoutedVoiceSink() == sink )
            return TRUE;

    return FALSE;
}

/******************************************************************************
NAME
    volumeGetMuteState

DESCRIPTION
    Get the stored mute state of a mute group

RETURNS
    AUDIO_MUTE_STATE_T of the mute group

*/
AUDIO_MUTE_STATE_T volumeGetMuteState(AUDIO_MUTE_GROUP_T mute_group)
{
    if (GSINK_VOLUME_DATA.mute_states & AUDIO_MUTE_MASK(mute_group))
        return AUDIO_MUTE_ENABLE;
    else
        return AUDIO_MUTE_DISABLE;
}

/******************************************************************************
NAME
    sinkvolumeGetVolumeStoreTimeout

DESCRIPTION
    Helper function to get the volume store timeout 

RETURNS
    uint16 : Timer value.
*/
uint16 sinkvolumeGetVolumeStoreTimeout(void)
{
    uint16 time_out = 0;
    sink_volume_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        time_out = read_configdata->StoreCurrentSinkVolSrcTimeout_s;
        configManagerReleaseConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID);
    }
    return time_out;
}

bool sinkvolumeSetVolumeStoreTimeout(uint16 timeout)
{
    sink_volume_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->StoreCurrentSinkVolSrcTimeout_s = timeout;
        configManagerUpdateWriteableConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/******************************************************************************
NAME
    sinkvolumeIsMuteSpeakerAndMic

DESCRIPTION
    Helper function to check speaker and mic mute is enabled or not

RETURNS
    bool : TRUE/FALSE.
*/
static bool sinkvolumeIsMuteSpeakerAndMic(void)
{
    bool muteSpeakerandMic = FALSE;
    sink_volume_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
       muteSpeakerandMic = read_configdata->MuteSpeakerAndMic;
       configManagerReleaseConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID);
    }
    return muteSpeakerandMic;
}

/******************************************************************************
NAME 
    sinkVolumeGetVolumeMappingforVolIncrease

DESCRIPTION
    Helper function to get volume mapping for increasing volume

RETURNS
    uint8 volume mapping

*/
static uint8 sinkVolumeGetVolumeMappingforVolIncrease(uint16 index)
{
    uint8 volIncrease= 0;   
    sink_volume_writeable_config_def_t *writeable_data = NULL;

    /* read the existing configuration */
    if (configManagerGetReadOnlyConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
       volIncrease = writeable_data->gVolMaps[index].IncVol;
       configManagerReleaseConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);
    }
    VOL_DEBUG(("SinkVolume:sinkVolumeGetVolumeMappingforVolIncrease() level %d\n", volIncrease));
    return volIncrease;
}

/******************************************************************************
NAME 
    sinkVolumeGetVolumeMappingforVolDecrease

DESCRIPTION
    Helper function to get volume mapping for decreasing volume

RETURNS
    uint8 volume mapping

*/
static uint8 sinkVolumeGetVolumeMappingforVolDecrease(uint16 index)
{
    uint8 volDecrease = 0;   
    sink_volume_writeable_config_def_t *writeable_data = NULL;

    /* read the existing configuration */
    if (configManagerGetReadOnlyConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
       volDecrease = writeable_data->gVolMaps[index].DecVol;
       configManagerReleaseConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);
    }
    VOL_DEBUG(("SinkVolume:sinkVolumeGetVolumeMappingforVolDecrease() level %d\n", volDecrease));
    return volDecrease;
}

/******************************************************************************
NAME 
    sinkVolumeGetVolumeMappingforTone

DESCRIPTION
    Helper function to get volume mapping for tone volume

RETURNS
    uint8 volume mapping

*/
static uint8 sinkVolumeGetVolumeMappingforTone(uint16 index)
{
    uint8 toneVol = 0;   
    sink_volume_writeable_config_def_t *writeable_data = NULL;

    VOL_DEBUG(("SinkVolume:sinkVolumeGetVolumeMappingforTone()\n"));
    /* read the existing configuration */
    if (configManagerGetReadOnlyConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
       toneVol = writeable_data->gVolMaps[index].Tone;
       configManagerReleaseConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);
    }
    return toneVol;
}

/******************************************************************************
NAME 
    sinkVolumeUpdateGlobalData

DESCRIPTION
    Helper function to update global data

RETURNS
    none

*/
static void sinkVolumeUpdateGlobalData(void)
{      
    sink_volume_writeable_config_def_t *writeable_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
       memcpy(&GSINK_VOLUME_DATA.volgroup_config, &writeable_data->volgroup_config, sizeof(vol_mod_gain_curve_t)*audio_output_group_all);
       configManagerReleaseConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME
 volumeInit

DESCRIPTION
 malloc memory for storing of current volume levels for a2dp, usb, wired and fm
 interfaces

RETURNS
 none

*/
void volumeInit(void)
{
    /* Update Global data from Config */
    sinkVolumeUpdateGlobalData();
    GSINK_VOLUME_DATA.cdl_max_volume = 0;   /* 0 represents disabled */

    AudioSetVolume((int16)sinkVolumeGetVolumeMappingforCVC(sinkHfpDataGetDefaultVolume()), 0);
}

/******************************************************************************
NAME 
    sinkVolumeGetVolumeMappingforCVC

DESCRIPTION
    Function to get the volume mapping for CVC

RETURNS
    uint8 volume mapping

*/
uint8 sinkVolumeGetVolumeMappingforCVC(uint16 index)
{
    uint8 cvcVol = 0;   
    sink_volume_writeable_config_def_t *writeable_data = NULL;

    VOL_DEBUG(("SinkVolume:sinkVolumeGetVolumeMappingforCVC()\n"));

    /* read the existing configuration */
    if (configManagerGetReadOnlyConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
       cvcVol = writeable_data->gVolMaps[index].VolGain;
       configManagerReleaseConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);
    }
    return cvcVol;
}

/******************************************************************************
NAME 
    sinkVolumeResetMaxAndMinVolumeFlag

DESCRIPTION
     Reset the vol max and vol min flags to FALSE.

RETURNS
    void

*/
void sinkVolumeResetMaxAndMinVolumeFlag(void)
{
    GSINK_VOLUME_DATA.vol_at_min = FALSE;
    GSINK_VOLUME_DATA.vol_at_max = FALSE;
}

/******************************************************************************
NAME 
    sinkVolumeGetGroupConfig

DESCRIPTION
    Helper function to get volume GroupConfig data based on index.
    The volume curve data is returned by reference and has the type defined in
    the gain_utils library. I.e. it does not expose the Volume Module's config
    type.

RETURNS
   void
   
*/
void sinkVolumeGetGroupConfig(audio_output_group_t group, volume_group_config_t* gain_utils_vol)
{
    vol_mod_gain_curve_t* vol_conf =  &GSINK_VOLUME_DATA.volgroup_config[group];

    if (gain_utils_vol!=NULL && vol_conf!=NULL)
    {
        gain_utils_vol->no_of_steps = vol_conf->no_of_steps;
        gain_utils_vol->dB_min = vol_conf->dB_min;
        gain_utils_vol->volume_knee_value_1 = vol_conf->volume_knee_value_1;
        gain_utils_vol->dB_knee_value_1 = vol_conf->dB_knee_value_1;
        gain_utils_vol->volume_knee_value_2 = vol_conf->volume_knee_value_2;
        gain_utils_vol->dB_knee_value_2 = vol_conf->dB_knee_value_2;
        gain_utils_vol->dB_max = vol_conf->dB_max;
    }
    else
    {
        Panic();
    }
}

bool sinkVolumeSetGroupConfig(audio_output_group_t group, const volume_group_config_t *group_config)
{
    sink_volume_writeable_config_def_t *config_data = NULL;

    if (group >= audio_output_group_all)
        return FALSE;

    if (configManagerGetWriteableConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (void **)&config_data, 0))
    {
        config_data->volgroup_config[group].no_of_steps = group_config->no_of_steps;
        config_data->volgroup_config[group].volume_knee_value_1 = group_config->volume_knee_value_1;
        config_data->volgroup_config[group].volume_knee_value_2 = group_config->volume_knee_value_2;
        config_data->volgroup_config[group].dB_knee_value_1 = group_config->dB_knee_value_1;
        config_data->volgroup_config[group].dB_knee_value_2 = group_config->dB_knee_value_2;
        config_data->volgroup_config[group].dB_max = group_config->dB_max;
        config_data->volgroup_config[group].dB_min = group_config->dB_min;
    
        configManagerUpdateWriteableConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);

        /* Update the values in GSINK_VOLUME_DATA to the new values as well. */
        GSINK_VOLUME_DATA.volgroup_config[group] = config_data->volgroup_config[group];

        return TRUE;
    }

    return FALSE;
}

/******************************************************************************
NAME 
    sinkVolumeGetGroupConfigSteps

DESCRIPTION
    Get the group volume config Steps.

RETURNS
    int16 number of steps

*/
int16 sinkVolumeGetGroupConfigSteps(uint16 group_index)
{
    int16 num_steps = 0;

    VOL_DEBUG(("SinkVolume:sinkVolumeGetGroupConfigSteps()\n"));

    num_steps = GSINK_VOLUME_DATA.volgroup_config[group_index].no_of_steps;
       
    return num_steps;
}

/**********************************************************************
NAME 
    sinkVolumeSetSessionVolButtonInverted

DESCRIPTION
    Interfaces for setting session data for VolButtonInverted flag

RETURNS
    void
    
*/
void sinkVolumeSetSessionVolButtonInverted(void)
{
    sink_volume_writeable_config_def_t *writeable_data = NULL;
    VOL_DEBUG(("SinkVolume:SinkVolumeSetSessionVolButtonInverted()\n"));

    if (configManagerGetWriteableConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (void **)&writeable_data, 0))
    {
       writeable_data->gVolButtonsInverted = GSINK_VOLUME_DATA.gVolButtonsInverted;
       configManagerUpdateWriteableConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);
    }
}

/**********************************************************************
NAME 
    sinkVolumeGetSessionVolButtonInverted

DESCRIPTION
    Interfaces for getting session data for VolButtonInverted flag

RETURNS
    void
    
*/
bool sinkVolumeGetSessionVolButtonInverted(void)
{
    sink_volume_writeable_config_def_t *writeable_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
       GSINK_VOLUME_DATA.gVolButtonsInverted = writeable_data->gVolButtonsInverted;
       configManagerReleaseConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);
    }

    VOL_DEBUG(("SinkVolume: vol_buttons_inverted = %d\n",GSINK_VOLUME_DATA.gVolButtonsInverted)); 
    return (GSINK_VOLUME_DATA.gVolButtonsInverted)?TRUE:FALSE;
}

/****************************************************************************
NAME    
    sinkVolumeSetVolButtonInverted
    
DESCRIPTION
     Set the volume button inverted flag
    
RETURNS
    void
*/
void  sinkVolumeSetVolButtonInverted(bool enable)
{
    GSINK_VOLUME_DATA.gVolButtonsInverted = enable;
}

/****************************************************************************
NAME    
    sinkVolumeIsVolButtonInverted
    
DESCRIPTION
    Get the volume button inverted flag
    
RETURNS
    bool
*/
bool sinkVolumeIsVolButtonInverted(void)
{
    return (GSINK_VOLUME_DATA.gVolButtonsInverted) ? TRUE : FALSE;
}

/******************************************************************************
NAME
    volumeSetMuteState

DESCRIPTION
    Set the stored mute state of a mute group

RETURNS
    void

*/
static void volumeSetMuteState(AUDIO_MUTE_GROUP_T mute_group, AUDIO_MUTE_STATE_T state)
{
    /* First un-set the bit corresponding to mute_group in the mute_states bit-field) */
    GSINK_VOLUME_DATA.mute_states &= ~AUDIO_MUTE_MASK(mute_group);
    /* New state can now be OR'd over the top of the masked bit */
    GSINK_VOLUME_DATA.mute_states |= (state << mute_group);
}

/******************************************************************************
NAME
    volumeGetMuteAllStatus

DESCRIPTION
    Get the status of the 'mute all outputs' override flag.

RETURNS
    TRUE if mute enabled, FALSE otherwise.

*/
static bool volumeGetMuteAllStatus(void)
{
    return GSINK_VOLUME_DATA.mute_all_outputs;
}

/******************************************************************************
NAME
    volumeSetMuteAllStatus

DESCRIPTION
    Set the 'mute all outputs' override flag.

PARAMETERS
    enable_mute    Status to set the flag to.

RETURNS
    void

*/
static void volumeSetMuteAllStatus(bool enable_mute)
{
    GSINK_VOLUME_DATA.mute_all_outputs = enable_mute;
}

/******************************************************************************
NAME
    VolumeApplySoftMuteStates

DESCRIPTION
    Send the stored soft mute state to the audio library, overriding the output
    mute states if necessary.

RETURNS
    void

*/
void VolumeApplySoftMuteStates(void)
{
    AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T message;

    message.mute_states = GSINK_VOLUME_DATA.mute_states;

    if (volumeGetMuteAllStatus())
    {
        message.mute_states |= (AUDIO_MUTE_ENABLE << audio_mute_group_main);
        message.mute_states |= (AUDIO_MUTE_ENABLE << audio_mute_group_aux);
    }

    AudioSetSoftMute(&message);
}

/******************************************************************************
NAME
    VolumeAllOutputsMuted

DESCRIPTION
    Checks if both main and aux outputs are in a muted state.

RETURNS
    bool

*/
bool VolumeAllOutputsMuted(void)
{
    if (volumeGetMuteAllStatus())
        return TRUE;

    return volumeGetMuteState(audio_mute_group_main) &&
           volumeGetMuteState(audio_mute_group_aux);
}

/******************************************************************************
NAME
    VolumeUpdateMuteState

DESCRIPTION
    Update the stored mute state for a specific mute group

RETURNS
    void

*/
void VolumeUpdateMuteState(AUDIO_MUTE_GROUP_T mute_group, AUDIO_MUTE_STATE_T state)
{
    if (volumeGetMuteState(mute_group) == state)
        return;

    if (mute_group == audio_mute_group_mic)
    {
        VolumeSetMicrophoneMute(state);
    }
    else
    {
        volumeSetMuteState(mute_group, state);

        if (sinkAudioIsAudioRouted() || sinkAudioIsVoiceRouted() || sinkReceiverIsStreaming())
        {
            VolumeApplySoftMuteStates();
        }
    }

}

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
void VolumeUpdateMuteStatusAllOutputs(bool enable_mute)
{
    if (volumeGetMuteAllStatus() == enable_mute)
        return;

    volumeSetMuteAllStatus(enable_mute);

    VolumeApplySoftMuteStates();
}

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
void VolumeUpdateMuteStatusMicrophone(bool enable_mute)
{
    AUDIO_MUTE_STATE_T state = enable_mute? AUDIO_MUTE_ENABLE : AUDIO_MUTE_DISABLE;

    if (volumeGetMuteState(audio_mute_group_mic) == state)
        return;

    VolumeSetMicrophoneMute(state);
}

/****************************************************************************
NAME
    VolumeToggleMute

DESCRIPTION
    Toggles the mute state of a specific mute group.

RETURNS
    void

*/
static void VolumeToggleMute(const AUDIO_MUTE_GROUP_T mute_group)
{
    sinkEvents_t event;

    /* If MUTE enabled for a group, generate the corresponding 'MuteOff' event.
       Otherwise, generate 'MuteOn' event, to toggle its mute status bit. */
    switch (mute_group)
    {
        case audio_mute_group_main:
        {
            if (volumeGetMuteState(mute_group))
                event = EventUsrMainOutMuteOff;
            else
                event = EventUsrMainOutMuteOn;
        }
        break;

        case audio_mute_group_aux:
        {
            if (volumeGetMuteState(mute_group))
                event = EventUsrAuxOutMuteOff;
            else
                event = EventUsrAuxOutMuteOn;
        }
        break;

        case audio_mute_group_mic:
        {
            if (volumeGetMuteState(mute_group))
                event = EventUsrMicrophoneMuteOff;
            else
                event = EventUsrMicrophoneMuteOn;
        }
        break;

        default:
        return;
    }

    MessageSend(&theSink.task, event, 0);
}

/******************************************************************************
NAME
    VolumeSetMicrophoneMute

DESCRIPTION
    Update the stored mute state for the HFP microphone and send it to the DSP.
    Will also mute all outputs if this feature is enabled.

RETURNS
    void

*/
void VolumeSetMicrophoneMute(AUDIO_MUTE_STATE_T state)
{
    if ((sinkAudioGetRoutedVoiceSink() == sinkCallManagerGetHfpSink(hfp_primary_link)) || (sinkAudioGetRoutedVoiceSink() == sinkCallManagerGetHfpSink(hfp_secondary_link)))
    {
        /* Determine which AG is to be muted or unmuted, start with the AG that is currently routing audio */
        hfp_link_priority priority = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());

        /* Send unmute message if suitable AG found */
        if (priority)
        {
            VOL_DEBUG(("VOL: Mute T [%c]\n", (sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority) )? 'F' : 'T') ));

            /* HFP audio currently being routed, update mute state of appropriate AG */
            VolumeSetHfpMicrophoneGain(priority, (state? MICROPHONE_MUTE_ON : MICROPHONE_MUTE_OFF));
        }
    }
    else if(usbIsCurrentVoiceSinkUsb())
    {
        volumeSetMuteState(audio_mute_group_mic, state);
        VolumeApplySoftMuteStates();
    }
#if defined(APTX_LL_BACK_CHANNEL) || defined(INCLUDE_FASTSTREAM)
    else if(sinkA2dpAudioPrimaryOrSecondarySinkMatch(sinkAudioGetRoutedAudioSink()))
    {
        /* A2DP currently routed, update mute state of the low latency back channel */
        VolumeSetA2dpMicrophoneGain(state);
    }
#endif /* defined(APTX_LL_BACK_CHANNEL) || defined(INCLUDE_FASTSTREAM)*/
}


#if defined (APTX_LL_BACK_CHANNEL) || defined(INCLUDE_FASTSTREAM)
/****************************************************************************
DESCRIPTION
    Set mute or unmute of A2DP low latency back channel mic.
*/
static void VolumeSetA2dpMicrophoneGain(AUDIO_MUTE_STATE_T state)
{
    VOL_DEBUG(("VOL: Set A2DP Mic Mute [%s]\n", state? "ON" : "OFF"));


    /* Update the mute state for the music streaming with back channel support dsp apps */
    updateMicMuteState(state);
    
    volumeSetMuteState(audio_mute_group_mic, state);
    
    /* Check whether feature to mute microphone and speaker is set */
    if (sinkvolumeIsMuteSpeakerAndMic())
    {
        if (state == AUDIO_MUTE_ENABLE)
            volumeSetMuteAllStatus(TRUE);
        else
            volumeSetMuteAllStatus(FALSE);
    }
    
    /* Apply mute/unmute the via the audio plugin */
    VolumeApplySoftMuteStates();
}
#endif /* defined (APTX_LL_BACK_CHANNEL) || defined(INCLUDE_FASTSTREAM)*/


/****************************************************************************
DESCRIPTION
    Set mute or unmute (mic gain of MICROPHONE_MUTE_ON - 0 is mute, all other
    gain settings unmute) of HFP mic.
*/
void VolumeSetHfpMicrophoneGain(hfp_link_priority priority, uint8 mic_gain)
{
    VOL_DEBUG(("VOL: Set Mic Gain [%d] %d\n", priority, mic_gain));


    /* determine the priority to control if not passed in */
    if(!priority)
    {
        /* try to get the currently routed audio */
        priority = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());

        /* is there a routed audio that can be muted or unmuted ? */
        if(!priority)
        {
            /* no routed audio on device, check for active call on either AG */
            hfp_call_state CallState = hfp_call_state_idle;
            HfpLinkGetCallState(PROFILE_INDEX(hfp_primary_link), &CallState);
            /* any calls on AG1? */
            if(CallState != hfp_call_state_idle)
                priority = hfp_primary_link;
            /* no calls on AG1, check AG2 */
            else
            {
                /* call state of AG2 */
                HfpLinkGetCallState(PROFILE_INDEX(hfp_secondary_link), &CallState);
                /* any calls on AG2? */
                if(CallState != hfp_call_state_idle)
                    priority = hfp_secondary_link;
            }
        }
    }

    VOL_DEBUG(("VOL: Set Mic Gain [%d] priority is: %d\n",  mic_gain, priority));

    /* ensure there is a valid AG to mute */
    if(priority)
    {
        /* If vol has been set to 0 then mute */
        if(mic_gain == MICROPHONE_MUTE_ON)
        {
            VOL_DEBUG(("VOL: Mute On\n"));
            /* Update settings */
            sinkHfpDataSetAudioGMuted(TRUE,PROFILE_INDEX(priority));
            volumeSetMuteState(audio_mute_group_mic, AUDIO_MUTE_ENABLE);
            /* Check whether feature to mute microphone and speaker is set */
            if (sinkvolumeIsMuteSpeakerAndMic())
                volumeSetMuteAllStatus(TRUE);
            sinkCancelAndIfDelaySendLater(EventSysMuteReminder, D_SEC(sinkHfpDataGetMuteRemindTime()));
        }
        /* mute off */
        else
        {
            VOL_DEBUG(("VOL: Mute Off on priority %d\n",priority));
            /* Update settings */
            sinkHfpDataSetAudioGMuted(FALSE,PROFILE_INDEX(priority));
            volumeSetMuteState(audio_mute_group_mic, AUDIO_MUTE_DISABLE);
            /* Check whether feature to mute microphone and speaker was set */
            if (sinkvolumeIsMuteSpeakerAndMic())
                volumeSetMuteAllStatus(FALSE);
            /* Cancel reminder tones if no other muted AG */
            if((!sinkHfpDataGetAudioGMuted(PROFILE_INDEX(hfp_primary_link) ))&&
               (!sinkHfpDataGetAudioGMuted(PROFILE_INDEX(hfp_secondary_link) )))
            {
                MessageCancelAll( &theSink.task , EventSysMuteReminder ) ;
                VOL_DEBUG(("VOL: Mute Off - no mute - stop reminder tones\n"));
            }
        }

        /* mute/unmute the audio via the audio plguin */
        VolumeApplySoftMuteStates();
    }
}

/****************************************************************************
DESCRIPTION
    Set mute or unmute remotely from AG if SyncMic feature bit is enabled
    (mic gain of MICROPHONE_MUTE_ON - 0 is mute, all other gain settings unmute).
*/
void VolumeSetHfpMicrophoneGainCheckMute(hfp_link_priority priority, uint8 mic_gain)
{

    VOL_DEBUG(("VOL: Set Remote Mic Gain [%d] priority is: %d\n",  mic_gain, priority));

    /* ensure profile passed in is valid */
    if(priority)
    {
        /* if currently muted and volume is non zero then send unmute event */
        if((mic_gain != MICROPHONE_MUTE_ON) && (sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority) )))
        {
            /* does this mute state change affect the currently routed audio? */
            if(volumeSyncAffectsGain(priority))
            {
                /* it does so indicate unmute has taken place */
                MessageSend(&theSink.task, EventUsrMicrophoneMuteOff, 0);
            }
            /* unmute state change occurred on AG that isn't currently having its audio
               routed, if other AG is not muted then cancel the mute reminder timer */
            else if(!sinkHfpDataGetAudioGMuted(OTHER_PROFILE(priority)))
            {
                /* prevent mute reminder timer from continuing as no valid mute cases any more */
                MessageCancelAll( &theSink.task , EventSysMuteReminder ) ;
                VOL_DEBUG(("VOL: Check Mute Off - no mute - stop reminder tones\n"));
            }
            /* update the profile's mute bit */
            sinkHfpDataSetAudioGMuted(FALSE,PROFILE_INDEX(priority));
        }
        /* if not muted and request to go into mute state */
        else if((mic_gain == MICROPHONE_MUTE_ON) && (!sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority) )))
        {
            /* is this AG currently having its audio routed?*/
            if(volumeSyncAffectsGain(priority))
            {
                /* audio from this AG routed so process full mute indication */
                MessageSend(&theSink.task, EventUsrMicrophoneMuteOn, 0);
            }
            /* update the profile's mute bit */
            sinkHfpDataSetAudioGMuted(TRUE,PROFILE_INDEX(priority));
        }
    }
}

/****************************************************************************
DESCRIPTION
    Sends the current HFP microphone volume to the AG on connection.
*/
void VolumeSendHfpMicrophoneGain(hfp_link_priority priority, uint8 mic_gain)
{
    /* ensure link is valid before applying mute */
    if(priority != hfp_invalid_link)
    {
        VOL_DEBUG(("VOL: Sending Mic Gain [%d] %d\n", priority, mic_gain));
        /* Set profile's mute bit */
        sinkHfpDataSetAudioGMuted(mic_gain ? FALSE : TRUE, PROFILE_INDEX(priority));
        /* Send mic volume to AG */
        HfpVolumeSyncMicrophoneGainRequest(priority , &mic_gain);
    }
}


/****************************************************************************
DESCRIPTION
    Determine whether the mute reminder tone should be played in the device, e.g. #
    if AG1 is in mute state but AG2 is not muted and is the active AG then the mute reminder
    tone will not be played, when AG1 becomes the active AG it will be heard.
*/
bool VolumePlayMuteToneQuery(void)
{
    hfp_link_priority priority;

    /* determine which AG is currently the active AG */
    priority = audioGetLinkPriority(TRUE);


    /* check whether AG that is currently routing its audio to the device speaker is in mute state,
       if it is not then prevent the mute reminder tone from being heard. */
    if((priority) && (sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority) ) == FALSE))
    {
        VOL_DEBUG(("VOL: PlayMuteTone FALSE on %d mute is %d \n",priority,sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority) )));
        return FALSE;
    }
    /* all other use cases the mute reminder tone will be heard */
    else
    {
        VOL_DEBUG(("VOL: PlayMuteTone TRUE on %d mute is %d\n",priority,sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority) )));
        return TRUE;
    }
}

/****************************************************************************
    VolumeSetxxVolumeIndB functions, populate the AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T
    message and call the function to send group volume message to the plugins
*****************************************************************************/
static void volumeSetMessageMainVolumeInDb(AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T* msg, int16 vol)
{
    msg->main.master = vol;
    msg->main.tone = TonesGetToneVolumeInDb(audio_output_group_main);

    /* Always supply device trims when setting main volume */
    msg->main.device_trim_master = peerGetTwsMasterTrim();
    msg->main.device_trim_slave = peerGetTwsSlaveTrim();
}

static void volumeSetMessageAuxVolumeInDb(AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T* msg, int16 vol)
{
    msg->aux.master = vol;
    msg->aux.tone = TonesGetToneVolumeInDb(audio_output_group_aux);
}

static void VolumeSetMainAndAuxVolumeIndB(const int16 volume_main, const int16 volume_aux)
{
    /* Populate all fields in group volume message */
    AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T group_vol;
    group_vol.group = audio_output_group_all;
    volumeSetMessageMainVolumeInDb(&group_vol, volume_main);
    volumeSetMessageAuxVolumeInDb(&group_vol, volume_aux);

    AudioSetGroupVolume(&group_vol);
}

static void VolumeSetMainVolumeIndB(const int16 volume_main)
{
    /* Populate fields for main volume */
    AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T group_vol;
    group_vol.group = audio_output_group_main;
    volumeSetMessageMainVolumeInDb(&group_vol, volume_main);

    AudioSetGroupVolume(&group_vol);
}

static void VolumeSetAuxVolumeIndB(const int16 volume_aux)
{
    /* Populate fields for aux volume */
    AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T group_vol;
    group_vol.group = audio_output_group_aux;
    volumeSetMessageAuxVolumeInDb(&group_vol, volume_aux);

    AudioSetGroupVolume(&group_vol);
}


/****************************************************************************
    Volume related helper functions
*****************************************************************************/
static int16 getTrimForGroupInDb(const audio_output_group_t group)
{
    int16 trim_in_db = 0;
    if(group == audio_output_group_main)
    {
        trim_in_db = peerGetTwsTrimForLocalDevice();
    }
    return trim_in_db;
}

static int16 getBoundedVolumeIndB(int16 volume, const volume_group_config_t * volume_config)
{
    int16 bounded_volume = volume;

    bounded_volume = ((bounded_volume > GainIn60thdB(volume_config->dB_max))
                            ? GainIn60thdB(volume_config->dB_max) : bounded_volume);
    bounded_volume = ((bounded_volume < GainIn60thdB(volume_config->dB_min))
                            ? GainIn60thdB(volume_config->dB_min) : bounded_volume);

    return bounded_volume;
}

static int16 VolumeGetVolumeIndB(const int16 volume, const audio_output_group_t group)
{
    int16 volume_in_dB = 0;
    volume_group_config_t gain_utils_vol;

    sinkVolumeGetGroupConfig(group, &gain_utils_vol);

    if(volume <= VOLUME_A2DP_MIN_LEVEL)
    {
        volume_in_dB = DIGITAL_VOLUME_MUTE;
    }
    else
    {
        volume_in_dB = VolumeConvertStepsToDB(volume, &gain_utils_vol) + getTrimForGroupInDb(group);

        VOL_DEBUG(("VolumeGetVolumeIndB %x %x\n", volume_in_dB, getTrimForGroupInDb(group)));

        volume_in_dB = getBoundedVolumeIndB(volume_in_dB, &gain_utils_vol);
    }

    return volume_in_dB;
}

static void VolumeSetVolumeIndB(const int16 volume, const audio_output_group_t group)
{
    VOL_DEBUG(("VolumeSetVolumeIndB %x %x\n", volume, group));
    if(group == audio_output_group_all)
    {
        VolumeSetMainAndAuxVolumeIndB(volume, volume);
    }
    if(group == audio_output_group_main)
    {
        VolumeSetMainVolumeIndB(volume);
    }
    else
    {
        VolumeSetAuxVolumeIndB(volume);
    }
}

static int16 VolumeGetGroupVolumeFromInputVolumes(const volume_info * const volume, const audio_output_group_t group)
{
    if(group == audio_output_group_aux)
    {
        return volume->aux_volume;
    }
    return volume->main_volume;
}

void sinkVolumeSetGroupVolumeInInputVolume(volume_info * const volume, const audio_output_group_t group, int16 volume_step)
{
    if(group == audio_output_group_aux)
    {
        volume->aux_volume = volume_step;
    }
    else
    {
        volume->main_volume = volume_step;
    }
}

int16 sinkVolumeGetVolInDb(int16 group_vol, audio_output_group_t group)
{
    return VolumeGetVolumeIndB(group_vol, group);
}

static void VolumeSetDSPOperatingMode(const volume_info * const volumes, const audio_output_group_t group, const int16 previousVolume)
{
    if(VolumeGetGroupVolumeFromInputVolumes(volumes, group) != previousVolume)
    {
        /* Check whether volume has gone from 0 (mute) to a valid level, or vice versa, as if so the DSP needs to enter/exit standby mode. */
        if (previousVolume == VOLUME_A2DP_MIN_LEVEL &&
                (volumes->aux_volume == VOLUME_A2DP_MIN_LEVEL || volumes->main_volume == VOLUME_A2DP_MIN_LEVEL))
        {
            /* Set the operating mode to FULL_PROCESSING */
            AudioSetMode(AUDIO_MODE_CONNECTED, getAudioPluginModeParams());
        }
        else if ((previousVolume != VOLUME_A2DP_MIN_LEVEL) &&
                (volumes->main_volume == VOLUME_A2DP_MIN_LEVEL && volumes->aux_volume == VOLUME_A2DP_MIN_LEVEL))
        {
            /* Set the operating mode to STANDBY */
            AudioSetMode(AUDIO_MODE_STANDBY, getAudioPluginModeParams());
        }
    }
}

static void updateVolume(const volume_info * const volumes, const audio_output_group_t group)
{
    int16 group_volume = VolumeGetGroupVolumeFromInputVolumes(volumes, group);
    VOL_DEBUG(("updateVolume %x\n", VolumeGetVolumeIndB(group_volume, group)));

    VolumeSetVolumeIndB((VolumeGetVolumeIndB(group_volume, group)), group);
    if(group == audio_output_group_main)
    {
        displayUpdateVolume((VOLUME_NUM_VOICE_STEPS * group_volume)/sinkVolumeGetNumberOfVolumeSteps(group));
#ifdef ENABLE_SUBWOOFER
        updateSwatVolume(group_volume);
#endif
    }

}

static void VolumeSetNewVolume(const volume_info * const volumes, const int16 previousVolume,
        const audio_output_group_t group, const volume_direction direction)
{
    if(group == audio_output_group_main)
    {
        if(peerIsDeviceSlaveAndStreaming() && direction == same_volume)
        {
            /* Send a volume update message which won't be propagated to TWS peer
               only when this device is a slave and volume change wasn't triggered by
               pressing a button. */
            MessageSend(&theSink.task, EventSysUpdateMainVolumeLocallyOnly, 0);
        }
        else
        {
            MessageSend(&theSink.task, EventSysUpdateMainVolume, 0);
        }
    }
    else
    {
        updateVolume(volumes, group);
    }

    VolumeSetDSPOperatingMode(volumes, group, previousVolume);
}

static void VolumeIncrementGroupVolumeAndCheckLimit(volume_info * const volume, const audio_output_group_t group)
{
    int16 *groupVolume = &volume->main_volume;
    int16 upperVolumeLimit;

    if(group == audio_output_group_aux)
    {
        groupVolume = &volume->aux_volume;
    }

    (*groupVolume)++;

    if(GSINK_VOLUME_DATA.cdl_max_volume != 0)
        upperVolumeLimit = GSINK_VOLUME_DATA.cdl_max_volume;
    else
        upperVolumeLimit = sinkVolumeGetMaxVolumeStep(group);

    if(*groupVolume >= upperVolumeLimit)
    {
        *groupVolume = upperVolumeLimit;
        MessageSend(&theSink.task, EventSysVolumeMax, 0);
    }
}

static void VolumeDecrementGroupVolumeAndCheckLimit(volume_info * const volume, const audio_output_group_t group)
{
    int16 *groupVolume = &volume->main_volume;

    if(group == audio_output_group_aux)
    {
        groupVolume = &volume->aux_volume;
    }

    (*groupVolume)--;

    if(*groupVolume <= VOLUME_A2DP_MIN_LEVEL)
    {
        *groupVolume = VOLUME_A2DP_MIN_LEVEL;
        MessageSend(&theSink.task, EventSysVolumeMin, 0);
    }
}

static void VolumeSameButCheckLimit(volume_info * const volume, const audio_output_group_t group)
{
    int16 *groupVolume = &volume->main_volume;
    int16 upperVolumeLimit;

    if(group == audio_output_group_aux)
    {
        groupVolume = &volume->aux_volume;
    }

    if(GSINK_VOLUME_DATA.cdl_max_volume != 0)
        upperVolumeLimit = GSINK_VOLUME_DATA.cdl_max_volume;
    else
        upperVolumeLimit = sinkVolumeGetMaxVolumeStep(group);

    if(*groupVolume >= upperVolumeLimit)
    {
        *groupVolume = upperVolumeLimit;
        MessageSend(&theSink.task, EventSysVolumeMax, 0);
    }
}



static void VolumeUnmuteOnVolumeChange(const audio_output_group_t group)
{
    sink_volume_readonly_config_def_t *read_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
       if (read_data->VolumeChangeCausesUnMute)
       {
           if((group == audio_output_group_all || group == audio_output_group_main) &&
                volumeGetMuteState(audio_mute_group_main) == AUDIO_MUTE_ENABLE)
           {
               MessageSend(&theSink.task, EventUsrMainOutMuteOff, 0);
           }
           if((group == audio_output_group_all || group == audio_output_group_aux) &&
                volumeGetMuteState(audio_mute_group_aux) == AUDIO_MUTE_ENABLE)
           {
               MessageSend(&theSink.task, EventUsrAuxOutMuteOff, 0);
           }
       }
       configManagerReleaseConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID);
    }   
}

/****************************************************************************
NAME 
 VolumeCheckA2dpMute

DESCRIPTION
 check whether any a2dp connections are at minimum volume and mutes them properly if they are

RETURNS
 bool   Returns true if stream muted
    
*/
bool VolumeCheckA2dpMute(void)
{
    uint8 index;
 
    /* check both possible instances of a2dp connection */
    for(index = a2dp_primary; index < (a2dp_secondary+1); index++)
    {
        /* is a2dp connected? */
        if(getA2dpStatusFlag(CONNECTED, index))
        {
            /* check whether the a2dp connection is present and streaming data and that the audio is routed */
            if(sinkAudioIsAudioRouted() &&
               (sinkAudioGetRoutedAudioSink() == A2dpMediaGetSink(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index))))
            {
                /* get current volume for this profile */
                uint16 currentVol = SinkA2dpGetMainVolume(index);
                                       
                if( currentVol == VOLUME_A2DP_MIN_LEVEL)
                {
                    /* mute adjusted for a A2DP media stream */
                    VOL_DEBUG(("VOL: A2dp re-mute\n"));
                    return TRUE;
                }
            }
        }
    }
    /* no routed a2dp media streams found */
    return FALSE;
}

/****************************************************************************
DESCRIPTION
 sets the internal speaker gain to the level corresponding to the phone volume level
 sends the updated volume to the AG if updateAG flag is TRUE.
 
*/
void VolumeSetHeadsetVolume( uint16 pNewVolume , bool pPlayTone, hfp_link_priority priority, const bool updateAG)
{      
    bool lPlayTone = FALSE ;
    sink_volume_readonly_config_def_t *read_data = NULL;
    bool lVolumeChangeCausesUnMute= FALSE;
    bool lAdjustVolumeWhilstMuted= FALSE;
    
    uint16 upperVolumeLimit = VOLUME_HFP_MAX_LEVEL;
    bool limitationApplied = FALSE;
    
    bool set_gain = volumeSyncAffectsGain(priority);          
    

    if(GSINK_VOLUME_DATA.cdl_max_volume != 0)
    {
        upperVolumeLimit = GSINK_VOLUME_DATA.cdl_max_volume;
    }     
    
    if (configManagerGetReadOnlyConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        lVolumeChangeCausesUnMute = read_data->VolumeChangeCausesUnMute; 
        lAdjustVolumeWhilstMuted = read_data->AdjustVolumeWhilstMuted;
        configManagerReleaseConfig(SINK_VOLUME_READONLY_CONFIG_BLK_ID);
    }

    
    if(pNewVolume >= upperVolumeLimit)
	{
    	pNewVolume = upperVolumeLimit;
		limitationApplied = TRUE;
	}

    /*if hfp is attached - then send the vol change, but only if the device is not
      muted or the feature to allow volume changes whilst muted is enabled or the feature to unmute on
      volume changes is enabled */
    if ( ((stateManagerIsConnected() && (!sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority) ))) ||
          (lVolumeChangeCausesUnMute)||(lAdjustVolumeWhilstMuted)) && (updateAG) )
    {
        HfpVolumeSyncSpeakerGainRequest(priority, (uint8 *)&pNewVolume);
        VOL_DEBUG(("VOL: SEND and %x",(unsigned int) priority));
    }

    VOL_DEBUG(("SetVol [%x] [%d][%d][%d]\n " ,pNewVolume, sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority) ), lVolumeChangeCausesUnMute , lAdjustVolumeWhilstMuted));

	/* only adjust the volume if this AG is not muted or the device has the feature to allow volume change
       whilst muted or the feature to unmute on volume changes */
    if ( (!sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority) ) ) || ( lVolumeChangeCausesUnMute ) || 
         (lAdjustVolumeWhilstMuted))
    {
        /* set the local volume only, check whether unMute on volume change feature is enabled*/
        if( sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority))  && lVolumeChangeCausesUnMute)
		{
            VolumeSetHfpMicrophoneGainCheckMute(priority, MICROPHONE_MUTE_OFF);
        }
        
        /* the tone needs to be played so set flag */
        lPlayTone = TRUE ;     
        
        /* set new volume */
        sinkHfpdataSetAudioSMVolumeLevel(pNewVolume,PROFILE_INDEX(priority));
        
        if(sinkAudioIsVoiceRouted())
        {
            DEBUG(("DISP: VolumeSetHeadsetVolume\n"));
            /* update the display */
            displayUpdateVolume((VOLUME_NUM_VOICE_STEPS * pNewVolume)/sinkVolumeGetNumberOfVolumeSteps(audio_output_group_main));

            /* determine whether this volume change affects the audio currently being routed to the speaker #
               and update volume level via audio plugin if this is the case */
            if(set_gain)
            {
                AudioSetVolume ( (int16)sinkVolumeGetVolumeMappingforCVC(pNewVolume) , (int16)TonesGetToneVolume()) ;
            }
        }
    }
    
    /* ensure no limitation is enforced and valid tone (non zero) to be played */
    if( !limitationApplied && pPlayTone && lPlayTone && sinkVolumeGetVolumeMappingforTone(pNewVolume))
    {   /*only attempt to play the tone if it has not yet been played*/
        VOL_DEBUG(("VOL: VolTone[%x]\n" , (int)sinkVolumeGetVolumeMappingforTone(pNewVolume))) ;
        TonesPlayTone(sinkVolumeGetVolumeMappingforTone(pNewVolume) ,sinkTonesCanQueueVolumeTones());
    }  
    
    /* Send notification for max/min volume */
    if(pNewVolume >= upperVolumeLimit)
    {
        MessageSend ( &theSink.task , EventSysVolumeMax , 0 );
    }
    else if (pNewVolume == VOLUME_HFP_MIN_LEVEL)
    {
        MessageSend ( &theSink.task , EventSysVolumeMin , 0 );
    }
}

/*************************************************************************
NAME    
    storeCurrentSinkVolume
    
DESCRIPTION
    Stores the current volume level of the sink which is streaming audio (HFP or A2DP) 
    or in HFP connected state.
RETURNS

*/
void storeCurrentSinkVolume( void )
{
    tp_bdaddr SrcAddr = {{TYPED_BDADDR_INVALID},TRANSPORT_NONE};

    audio_route_available route = sinkAudioRouteAvailable();

    switch( route )
    {
        case audio_route_a2dp_primary:
            VOL_DEBUG(("VOL : update a2dp_primary attributes\n")) ;
            SinkGetBdAddr(sinkAudioGetRoutedAudioSink(), &SrcAddr);
            deviceManagerUpdateAttributes(&SrcAddr.taddr.addr, sink_a2dp, hfp_invalid_link, a2dp_primary);  
            break;
        case audio_route_a2dp_secondary:
            VOL_DEBUG(("VOL : update a2dp_secondary attributes\n")) ;
            SinkGetBdAddr(sinkAudioGetRoutedAudioSink(), &SrcAddr);
            deviceManagerUpdateAttributes(&SrcAddr.taddr.addr, sink_a2dp, hfp_invalid_link, a2dp_secondary);
            break;
        case audio_route_hfp_primary:
            /* check primary hfp    */
            VOL_DEBUG(("VOL : update hfp_primary attributes\n")) ;
            SinkGetBdAddr(sinkAudioGetRoutedVoiceSink(), &SrcAddr);
            deviceManagerUpdateAttributes(&SrcAddr.taddr.addr, sink_hfp, hfp_primary_link, 0);  
            break;
        case audio_route_hfp_secondary:
            VOL_DEBUG(("VOL : update hfp_secondary attributes\n")) ;
            SinkGetBdAddr(sinkAudioGetRoutedVoiceSink(), &SrcAddr);
            deviceManagerUpdateAttributes(&SrcAddr.taddr.addr, sink_hfp, hfp_secondary_link, 0);  
            break;
        case audio_route_none:
            {
                if((hfp_connected == sinkHfpDataGetProfileStatusConnected(PROFILE_INDEX(hfp_primary_link))) && HfpLinkGetBdaddr(hfp_primary_link, &SrcAddr.taddr.addr) ) 
                {
                    VOL_DEBUG(("VOL : update hfp primary attributes in connected state \n")) ;
                    deviceManagerUpdateAttributes(&SrcAddr.taddr.addr, sink_hfp, hfp_primary_link, 0);
                }
            }
            break;
        case audio_route_usb:
        case audio_route_analog:
        case audio_route_spdif:
        case audio_route_i2s:
        case audio_route_fm:
        default:
            VOL_DEBUG(("VOL: No Attributes to store for route :[%d]\n",route));
            break;
    }
}


/****************************************************************************
DESCRIPTION
    Checks the priority and sets the vol to the level corresponding to the phone volume level


*/
void VolumeSendAndSetHeadsetVolume( uint16 pNewVolume , bool pPlayTone, hfp_link_priority priority ) 
{
    /* ensure profile is connected before changing volume */
    if (priority)
    {
        /* make the corresponding local volume changes and update the AG with the volume changes*/
        VolumeSetHeadsetVolume(pNewVolume, pPlayTone, priority, TRUE);
    }
}


/****************************************************************************
    HFP Audio Volume increment/decrement functions
*****************************************************************************/
static bool ifHfpConnectedModifyAndUpdateVolume(const volume_direction direction, const audio_output_group_t group)
{

    if(deviceManagerNumConnectedDevs())
    {
        if(group == audio_output_group_main)
        {

            /* Get the link to change volume on */
            hfp_link_priority priority = audioGetLinkPriority(TRUE);
            /* Get current volume for this profile */
            uint16 volume = sinkHfpDataGetAudioSMVolumeLevel(PROFILE_INDEX(priority));

            
            if(direction == increase_volume)
            {
                VOL_DEBUG(("VOL: VolUp[%d][%d] to AG%d\n",volume, 
                         sinkVolumeGetVolumeMappingforVolIncrease(volume),
                         priority));

                volume = sinkVolumeGetVolumeMappingforVolIncrease(volume);
            }
            else if(direction == decrease_volume)
            {
                VOL_DEBUG(("VOL: VolDown[%d][%d] to AG%d\n",volume, 
                        sinkVolumeGetVolumeMappingforVolDecrease(volume), priority));
                volume = sinkVolumeGetVolumeMappingforVolDecrease(volume);
            }
            else
            {/* must be same_volume, check if charging deficit limitation applies */
               VOL_DEBUG(("VOL: Vol Kept The Same at index %d\n",volume ));
            }

            /* send, set and store new volume level */
            VolumeSendAndSetHeadsetVolume(volume, TRUE, priority);

        }
        return TRUE;
    }
    return FALSE;
}

static bool ifHfpAudioRoutedModifyAndUpdateVolume(const volume_direction direction, const audio_output_group_t group)
{
    if(HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink()))
    {
        return ifHfpConnectedModifyAndUpdateVolume(direction, group);
    }
    return FALSE;
}


/****************************************************************************
    Generic VolumeModifyAndUpdateVolume function called from plugin specific
    VolumeXXModifyAndUpdateVolume functions
    unmuteOnChange is used to flag whether the volume change was due to manual/human change of volume.


*****************************************************************************/
static void modifyAndUpdateVolume(const volume_direction direction, const audio_output_group_t group, volume_info * const volumeToModify, const bool unmuteOnChange)
{
    int16 previousVolume = VolumeGetGroupVolumeFromInputVolumes(volumeToModify, group);

    if(direction == increase_volume)
    {
        VolumeIncrementGroupVolumeAndCheckLimit(volumeToModify, group);
    }
    else if(direction == decrease_volume)
    {
        VolumeDecrementGroupVolumeAndCheckLimit(volumeToModify, group);
    }
    else
    { /* must be same_volume, check if charging deficit limitation applies */
        VolumeSameButCheckLimit(volumeToModify, group);
    }

    VOL_DEBUG(("VOL: Music main [%d] aux [%d] \n", volumeToModify->main_volume, volumeToModify->aux_volume));    
    
    VolumeSetNewVolume(volumeToModify, previousVolume, group, direction);
    if(unmuteOnChange)
    {
    	VolumeUnmuteOnVolumeChange(group);
    }
}

/****************************************************************************
    A2DP Audio related increment/decrement volume functions
*****************************************************************************/
static bool ifA2dpAudioRoutedModifyAndUpdateVolume(const volume_direction direction, const audio_output_group_t group, const bool unmuteOnChange)
{
    if(a2dpA2dpAudioIsRouted())
    {
        a2dp_index_t index = a2dpGetRoutedInstanceIndex();

        if(peerIsA2dpIndexTwsSource(index) && peerSlaveVolumeIsHandledViaMaster()
                                    && group == audio_output_group_main)
        {
            peerSignalVolumeUpdateToMaster(direction);
            if(unmuteOnChange)
            {
            	VolumeUnmuteOnVolumeChange(group);
            }
        }
        else
        {
            modifyAndUpdateVolume(direction, group, sinkA2dpGetA2dpVolumeInfoAtIndex(index), unmuteOnChange);

            if(group == audio_output_group_main)
            {
                sinkAvrcpUpdateVolume(VolumeGetGroupVolumeFromInputVolumes(sinkA2dpGetA2dpVolumeInfoAtIndex(index), group));
            }
        }

        return TRUE;
    }
    else if(sinkReceiverIsStreaming())
    {
        /* TODO Once BA Receiver will be added in the list of Routable Audio Source,
        this code should be removed. For time being use volume at Index 0 of A2DP Volumes */
        modifyAndUpdateVolume(direction, group, sinkA2dpGetA2dpVolumeInfoAtIndex(0), unmuteOnChange);
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
    Wired Audio related increment/decrement volume functions
*****************************************************************************/
static bool ifWiredAudioRoutedModifyAndUpdateVolume(const volume_direction direction, const audio_output_group_t group, const bool unmuteOnChange)
{
    volume_info analog_volume;
    volume_info spdif_volume;
    volume_info i2s_volume;
    bool volume_updated = FALSE;

    if(wiredAnalogAudioIsRouted())
    {    
        SinkWiredGetAnalogVolume(&analog_volume);
        modifyAndUpdateVolume(direction, group, &analog_volume, unmuteOnChange);
        SinkWiredSetAnalogVolume(analog_volume);
        volume_updated = TRUE;
    }
    if(wiredSpdifAudioIsRouted())
    {
        SinkWiredGetSpdifVolume(&spdif_volume);
        modifyAndUpdateVolume(direction, group, &spdif_volume, unmuteOnChange);
        SinkWiredSetSpdifVolume(spdif_volume);
        volume_updated = TRUE;
    }
    if(wiredI2SAudioIsRouted())
    {
        SinkWiredGetI2sVolume(&i2s_volume);
        modifyAndUpdateVolume(direction, group, &i2s_volume, unmuteOnChange);
        SinkWiredSetI2sVolume(i2s_volume);
        volume_updated = TRUE;
    }

    return volume_updated;
}

/****************************************************************************
    FM related increment/decrement volume functions
*****************************************************************************/
static bool ifFmIsOnModifyAndUpdateVolume(const volume_direction direction, const audio_output_group_t group, const bool unmuteOnChange)
{  
    if(sinkFmIsFmRxOn())
    {
        volume_info fm_volume_info = {0};

        sinkFmGetFmVolume(&fm_volume_info);
        modifyAndUpdateVolume(direction, group, &fm_volume_info, unmuteOnChange);
        sinkFmSetFmVolume(fm_volume_info);
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
    USB Audio related increment/decrement volume functions
*****************************************************************************/
static void modifyAndUpdateUsbVolume(const volume_direction direction, const audio_output_group_t group, bool unmuteOnChange)
{
    volume_info usb_volume;
    sinkUsbGetUsbVolume(&usb_volume);
 
    if(group == audio_output_group_aux)
    {
        modifyAndUpdateVolume(direction, group, &usb_volume, unmuteOnChange);
    }
    else
    {
        if(direction == increase_volume)
        {
            sinkUsbVolumeIncrement();
        }
        else if(direction == decrease_volume)
        {
            sinkUsbVolumeDecrement();
        }
    }
    sinkUsbSetUsbVolume(usb_volume); /*update the volume*/
}

static bool ifCurrentSinkIsUsbModifyAndUpdateVolume(const volume_direction direction, const audio_output_group_t group, const bool unmuteOnChange)
{
    if(usbIsCurrentAudioSinkUsb())
    {
        modifyAndUpdateUsbVolume(direction, group, unmuteOnChange);
        return TRUE;
    }
    return FALSE;
}

static bool ifUsbSinkExistsModifyAndUpdateVolume(const volume_direction direction, const audio_output_group_t group, const bool unmuteOnChange)
{
    if(usbIfUsbSinkExists())
    {
        modifyAndUpdateUsbVolume(direction, group, unmuteOnChange);
        return TRUE;
    }
    return FALSE;
}

void modifyAndUpdateRoutedAudioVolume(volume_direction direction, audio_output_group_t group, const bool unmuteOnChange)
{
    /* Try A2DP first */
    if(!sinkAccessoryVolumeRoute(direction) && ifA2dpAudioRoutedModifyAndUpdateVolume(direction, group, unmuteOnChange))
        return;

    /* Try HFP if has audio */
    if(ifHfpAudioRoutedModifyAndUpdateVolume(direction, group))
        return;

    /* Try USB if has audio */
    if(ifCurrentSinkIsUsbModifyAndUpdateVolume(direction, group, unmuteOnChange))
        return;

    /* Try wired if has audio */
    if(ifWiredAudioRoutedModifyAndUpdateVolume(direction, group, unmuteOnChange))
        return;

    /*Try FM RX*/
    if(ifFmIsOnModifyAndUpdateVolume(direction, group, unmuteOnChange))
        return;

    /* Try USB */
    if(ifUsbSinkExistsModifyAndUpdateVolume(direction, group, unmuteOnChange))
        return;

    /* Try HFP without any audio connections */
    ifHfpConnectedModifyAndUpdateVolume(direction, group);
}

/****************************************************************************
NAME 
 VolumeHandleSpeakerGainInd

DESCRIPTION
 Handle speaker gain change indication from the AG

RETURNS
 void
    
*/
void VolumeHandleSpeakerGainInd(const HFP_VOLUME_SYNC_SPEAKER_GAIN_IND_T* ind)
{
    VolumeSetHeadsetVolume ( ind->volume_gain , sinkTonesIsPlayLocalVolumeTone() , ind->priority, FALSE);
    sinkVolumeResetVolumeAndSourceSaveTimeout();
}

/****************************************************************************
    Functions to increment/decrement or resend stored group volumes to
    the current audio plugin
*/
void sinkVolumeModifyAndUpdateRoutedAudioMainVolume(const volume_direction dir)
{
    if(increase_volume == dir)
        GSINK_VOLUME_DATA.vol_at_min = FALSE;
    else if(decrease_volume == dir)
        GSINK_VOLUME_DATA.vol_at_max = FALSE;
    modifyAndUpdateRoutedAudioVolume(dir, audio_output_group_main, TRUE);
}

static void modifyAndUpdateRoutedAudioAuxVolume(const volume_direction dir)
{
    modifyAndUpdateRoutedAudioVolume(dir, audio_output_group_aux, TRUE);
}

void sinkVolumeModifyAndUpdateRoutedA2DPAudioMainVolume(const volume_direction direction)
{
    ifA2dpAudioRoutedModifyAndUpdateVolume(direction, audio_output_group_main, TRUE);
}

void sinkVolumeUpdateRoutedAudioMainAndAuxVolume(void)
{
    modifyAndUpdateRoutedAudioVolume(same_volume, audio_output_group_main, TRUE);
    modifyAndUpdateRoutedAudioVolume(same_volume, audio_output_group_aux, TRUE);
}

/****************************************************************************
    Send new main group volume level to currently loaded plugin
    - updates display, sub and uses previous volume to update dsp
    operating mode
*/
void VolumeSetNewMainVolume(const volume_info * const volumes, const int16 previousVolume)
{
    VolumeSetNewVolume(volumes, previousVolume, audio_output_group_main, same_volume);
}

/*****************************************
    Set the mute status and volume levels.
*/
void VolumeSetupInitialMutesAndVolumes(const volume_info * const volumes)
{
    int16 main_volume_in_dB = 0;
    int16 aux_volume_in_dB = 0;
    volume_group_config_t gain_utils_vol;

    sinkVolumeGetGroupConfig(audio_output_group_main, &gain_utils_vol);
    main_volume_in_dB = VolumeConvertStepsToDB(volumes->main_volume, &gain_utils_vol);

    sinkVolumeGetGroupConfig(audio_output_group_aux, &gain_utils_vol);
    aux_volume_in_dB = VolumeConvertStepsToDB(volumes->aux_volume, &gain_utils_vol);

    /* Although volume levels must be set for every source,
       sink must also check the mute status for aux and main groups.*/
    if(volumeGetMuteState(audio_mute_group_main))
    {
        volumeSetMuteState(audio_mute_group_main, AUDIO_MUTE_ENABLE);
        VolumeApplySoftMuteStates();
    }

    if(volumeGetMuteState(audio_mute_group_aux))
    {
        volumeSetMuteState(audio_mute_group_aux, AUDIO_MUTE_ENABLE);
        VolumeApplySoftMuteStates();
    }

    VolumeSetMainAndAuxVolumeIndB(main_volume_in_dB, aux_volume_in_dB);
}

#define PIO_AUX_OUT_DETECT sinkAudioGetAuxOutDetectPio()

/* JACK_DET LOW = Disconnected, HIGH = Connected */
#define pio_aux_out_connected() (PioGetPio(PIO_AUX_OUT_DETECT))

/****************************************************************************
    Set the initial mute states for audio inputs and outputs
*/
void VolumeSetInitialMuteState(void)
{
    /* Default is to un-mute all outputs so main and aux play concurrently */
    GSINK_VOLUME_DATA.mute_states = AUDIO_MUTE_DISABLE_ALL;
    
    /* If the aux_out_detect is configured then we apply privacy rules */
    if(PIO_AUX_OUT_DETECT != NO_PIO)
    {
        /* Mute main if aux is connected, otherwise mute aux */
        if(pio_aux_out_connected())
            GSINK_VOLUME_DATA.mute_states |= AUDIO_MUTE_MASK(audio_mute_group_main);
        else
            GSINK_VOLUME_DATA.mute_states |= AUDIO_MUTE_MASK(audio_mute_group_aux);
    }
    
    GSINK_VOLUME_DATA.mute_all_outputs = FALSE;
}

static void sinkVolumeResetSessionDataToBeStoredTimer(const MessageId volume_event)
{
    switch(volume_event)
    {
        case EventUsrMainOutVolumeUp:
        case EventUsrMainOutVolumeDown:
        case EventUsrAuxOutVolumeUp:
        case EventUsrAuxOutVolumeDown:
            sinkVolumeResetVolumeAndSourceSaveTimeout();
            break;
        default:
            break;
    }
}

static bool isAbsoluteVolumeToBeIndicated(void)
{
    return a2dpA2dpAudioIsRouted();
}

static void updateMainVolumeForCurrentSource(void)
{
    volume_info volume;
    /* TODO */
    if(sinkReceiverIsStreaming())
    {
        getVolumeInfoFromAudioSource(audio_source_a2dp_1, &volume);
    }
    else
    {
        getVolumeInfoFromAudioSource(sinkAudioGetRoutedAudioSource(), &volume);
    }
    updateVolume(&volume, audio_output_group_main);
}

static void syncVolumeWithA2dpSource(void)
{
    if(isAbsoluteVolumeToBeIndicated())
    {
        volume_info volume;
        getVolumeInfoFromAudioSource(sinkAudioGetRoutedAudioSource(), &volume);
        sinkAvrcpUpdateVolume(VolumeGetGroupVolumeFromInputVolumes(&volume, audio_output_group_main));
    }
}

bool sinkVolumeProcessEventVolume(const MessageId volume_event)
{
    bool indicate_event = TRUE;

    if(peerProcessEvent(volume_event))
    {
        indicate_event = FALSE;
        VOL_DEBUG(("Event handled by peer"));
    }
    else
    {
        switch(volume_event)
        {
            case EventUsrMicrophoneMuteToggle:
                VOL_DEBUG(("EventUsrMicrophoneMuteToggle")) ;
                VolumeToggleMute(audio_mute_group_mic);
                break ;
            case EventUsrMicrophoneMuteOn :
                VOL_DEBUG(("EventUsrMicrophoneMuteOn")) ;
                VolumeUpdateMuteState(audio_mute_group_mic, AUDIO_MUTE_ENABLE);
                break ;
            case EventUsrMicrophoneMuteOff:
                VOL_DEBUG(("EventUsrMicrophoneMuteOff\n")) ;
                VolumeUpdateMuteState(audio_mute_group_mic, AUDIO_MUTE_DISABLE);
                break;

            case EventUsrVolumeOrientationToggle:
                sinkVolumeSetVolButtonInverted(sinkVolumeIsVolButtonInverted()^1);
                VOL_DEBUG(("HS: Toggle Volume Orientation[%d]\n", sinkVolumeIsVolButtonInverted())) ;
                break ;
            case EventUsrVolumeOrientationNormal:
                sinkVolumeSetVolButtonInverted(FALSE);
                VOL_DEBUG(("HS: VOL ORIENT NORMAL [%d]\n", sinkVolumeIsVolButtonInverted())) ;
                break;
            case EventUsrVolumeOrientationInvert:
                sinkVolumeSetVolButtonInverted(TRUE);
                VOL_DEBUG(("HS: VOL ORIENT INVERT[%d]\n", sinkVolumeIsVolButtonInverted())) ;
                break;

            case EventSysVolumeMax:
                VOL_DEBUG(("HS : EventSysVolumeMax\n")) ;
                GSINK_VOLUME_DATA.vol_at_max = TRUE;
#if defined(ENABLE_PEER) && defined(PEER_TWS)
                if(peerIsLinkMaster())
                {
                    sinkAvrcpUpdateTWSPeerVolumeLimitReached(VOLUME_MAX);
                }
#endif
                break;
            case EventSysVolumeMin:
                VOL_DEBUG(("HS : EventSysVolumeMin\n")) ;
                GSINK_VOLUME_DATA.vol_at_min = TRUE;
#if defined(ENABLE_PEER) && defined(PEER_TWS)
                if(peerIsLinkMaster())
                {
                    sinkAvrcpUpdateTWSPeerVolumeLimitReached(VOLUME_MIN);
                }
#endif
                break;

            case EventUsrMainOutVolumeUp:
                VOL_DEBUG(("EventUsrMainOutVolumeUp\n"));
                if(GSINK_VOLUME_DATA.vol_at_max)
                {
                    indicate_event = FALSE;
                }
                sinkVolumeModifyAndUpdateRoutedAudioMainVolume(increase_volume);
                break;
            case EventUsrMainOutVolumeDown:
                VOL_DEBUG(("EventUsrMainOutVolumeDown\n"));
                if(GSINK_VOLUME_DATA.vol_at_min)
                {
                    indicate_event = FALSE;
                }
                sinkVolumeModifyAndUpdateRoutedAudioMainVolume(decrease_volume);
                break;
            case EventUsrAuxOutVolumeUp:
                VOL_DEBUG(( "HS : Aux Out Vol Up\n" ));
                modifyAndUpdateRoutedAudioAuxVolume(increase_volume);
                break;
            case EventUsrAuxOutVolumeDown:
                VOL_DEBUG(( "HS : Aux Out Vol Down\n" ));
                modifyAndUpdateRoutedAudioAuxVolume(decrease_volume);
                break;

            case EventUsrMainOutMuteOn:
            case EventSysPeerGeneratedMainMuteOn:
                VOL_DEBUG(( "HS : Main Vol Mute On\n" ));
                VolumeUpdateMuteState(audio_mute_group_main, AUDIO_MUTE_ENABLE);
                break;
            case EventUsrMainOutMuteOff:
            case EventSysPeerGeneratedMainMuteOff:
                VOL_DEBUG(( "HS : Main Vol Mute Off\n" ));
                VolumeUpdateMuteState(audio_mute_group_main, AUDIO_MUTE_DISABLE);
                break;
            case EventUsrMainOutMuteToggle:
                VOL_DEBUG(( "HS : Main Vol Mute Toggle\n" ));
                VolumeToggleMute(audio_mute_group_main);
                break;
            case EventUsrAuxOutMuteOn:
                VOL_DEBUG(( "HS : Aux Out Vol Mute On\n" ));
                VolumeUpdateMuteState(audio_mute_group_aux, AUDIO_MUTE_ENABLE);
                break;
            case EventUsrAuxOutMuteOff:
                VOL_DEBUG(( "HS : Aux Out Vol Mute Off\n" ));
                VolumeUpdateMuteState(audio_mute_group_aux, AUDIO_MUTE_DISABLE);
                break;
            case EventUsrAuxOutMuteToggle:
                VOL_DEBUG(( "HS : Aux Out Vol Mute Toggle\n" ));
                VolumeToggleMute(audio_mute_group_aux);
                break;
            case EventSysVolumeAndSourceChangeTimer:
                VOL_DEBUG(("EventSysVolumeAndSourceChangeTimer\n"));
                /* Check if the timer is enabled.*/
                if(sinkvolumeGetVolumeStoreTimeout())
                {
                    /* Store the Volume information of the currently streaming(either A2Dp or HFP) sink device into the PS Store. */
                    storeCurrentSinkVolume();
                }
                break;
            case EventSysUpdateMainVolume:
            case EventSysUpdateMainVolumeLocallyOnly:
            case EventSysPeerSelfGeneratedUpdateMainVolume:
                VOL_DEBUG(("EventSysUpdateMainVolume or EventSysUpdateMainVolumeLocallyOnly or EventSysPeerSelfGeneratedUpdateMainVolume\n"))
                updateMainVolumeForCurrentSource();
                break;

            case EventSysPeerRemoteGeneratedUpdateMainVolume:
                VOL_DEBUG(("EventSysPeerRemoteGeneratedUpdateMainVolume\n"));
                updateMainVolumeForCurrentSource();
                syncVolumeWithA2dpSource();
                break;
            }
        }

    sinkVolumeResetSessionDataToBeStoredTimer(volume_event);

    return indicate_event;
}

MessageId sinkVolumeModifyEventAccordingToVolumeOrientation(const MessageId volume_event)
{
    MessageId newMessageId = volume_event;

    if(sinkVolumeIsVolButtonInverted())
    {
        if(volume_event == EventUsrMainOutVolumeUp)
        {
            newMessageId = EventUsrMainOutVolumeDown;
        }
        else if(volume_event == EventUsrMainOutVolumeDown)
        {
            newMessageId = EventUsrMainOutVolumeUp;
        }
    }
    return newMessageId;
}

void sinkVolumeResetVolumeAndSourceSaveTimeout(void)
{
     uint16 time_out= sinkvolumeGetVolumeStoreTimeout();
    sinkCancelAndIfDelaySendLater(EventSysVolumeAndSourceChangeTimer, D_SEC(time_out));
}

bool sinkVolumeGetVolumeMappingForLevel(uint16 level, volume_mapping_t *mapping)
{
    sink_volume_writeable_config_def_t *config_data = NULL;
    
    if (level >= VOLUME_NUM_VOLUME_MAPPINGS)
        return FALSE;

    if (configManagerGetReadOnlyConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (const void **)&config_data))
    {
        mapping->inc_vol = config_data->gVolMaps[level].IncVol;
        mapping->dec_vol = config_data->gVolMaps[level].DecVol;
        mapping->tone = config_data->gVolMaps[level].Tone;
        mapping->vol_gain = config_data->gVolMaps[level].VolGain;

        configManagerReleaseConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

bool sinkVolumeSetVolumeMappingForLevel(uint16 level, const volume_mapping_t *mapping)
{
    sink_volume_writeable_config_def_t *config_data = NULL;
    
    if (level >= VOLUME_NUM_VOLUME_MAPPINGS)
        return FALSE;

    if (configManagerGetWriteableConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID, (void **)&config_data, 0))
    {
        config_data->gVolMaps[level].IncVol = mapping->inc_vol;
        config_data->gVolMaps[level].DecVol = mapping->dec_vol;
        config_data->gVolMaps[level].Tone = mapping->tone;
        config_data->gVolMaps[level].VolGain = mapping->vol_gain;

        configManagerUpdateWriteableConfig(SINK_VOLUME_WRITEABLE_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

static void getVolumeInfoFromAudioSource(const audio_sources source, volume_info * volume)
{
    switch(source)
    {
        case audio_source_FM:
            sinkFmGetFmVolume(volume);
            break;
        case audio_source_ANALOG:
            SinkWiredGetAnalogVolume(volume);
            break;
        case audio_source_I2S:
            SinkWiredGetI2sVolume(volume);
            break;
        case audio_source_SPDIF:
            SinkWiredGetSpdifVolume(volume);
            break;
        case audio_source_USB:
            sinkUsbGetUsbVolume(volume);
            break;
        case audio_source_a2dp_1:
            *volume = *sinkA2dpGetA2dpVolumeInfoAtIndex(a2dp_primary);
            break;
        case audio_source_a2dp_2:
            *volume = *sinkA2dpGetA2dpVolumeInfoAtIndex(a2dp_secondary);
            break;
        case audio_source_none:
        default:
            memset(volume, 0, sizeof(volume_info));
            break;
    }
}

static void setVolumeInfoForAudioSource(const audio_sources source, volume_info * volume)
{
    switch(source)
    {
        case audio_source_FM:
            sinkFmSetFmVolume(*volume);
            break;
        case audio_source_ANALOG:
        case audio_source_I2S:
            SinkWiredSetAnalogVolume(*volume);
            break;
        case audio_source_SPDIF:
            SinkWiredSetSpdifVolume(*volume);
            break;
        case audio_source_USB:
            sinkUsbSetUsbVolume(*volume);
            break;
        case audio_source_a2dp_1:
            sinkA2dpSetA2dpVolumeInfoAtIndex(a2dp_primary, *volume);
            break;
        case audio_source_a2dp_2:
            sinkA2dpSetA2dpVolumeInfoAtIndex(a2dp_secondary, *volume);
            break;
        case audio_source_none:
        default:
            break;
    }
}

static int16 getVolumeFromAudioSource(audio_sources source, audio_output_group_t audio_output_group)
{
    volume_info volume;
    getVolumeInfoFromAudioSource(source, &volume);
    return VolumeGetGroupVolumeFromInputVolumes(&volume, audio_output_group);
}

static void setVolumeForAudioSource(audio_sources source, audio_output_group_t audio_output_group, int16 volume_step)
{
    volume_info volume;
    getVolumeInfoFromAudioSource(source, &volume);
    sinkVolumeSetGroupVolumeInInputVolume(&volume, audio_output_group, volume_step);
    setVolumeInfoForAudioSource(source, &volume);
}

static scaled_volume_percentage_t getVolumeStepAsScaledPercentage(int16 volume_step, audio_output_group_t audio_output_group)
{
    volume_group_config_t volume_config;
    sinkVolumeGetGroupConfig(audio_output_group, &volume_config);
    return GainUtilsConvertVolumeStepToScaledPercentage(volume_step, &volume_config);
}

static int16 getPercentageAsVolumeStep(scaled_volume_percentage_t volume, audio_output_group_t audio_output_group)
{
    volume_group_config_t volume_config;
    sinkVolumeGetGroupConfig(audio_output_group, &volume_config);
    return GainUtilsConvertScaledVolumePercentageToStep(volume, &volume_config);
}

scaled_volume_percentage_t sinkVolumeGetCurrentVolumeAsScaledPercentage(void)
{
    int16 volume_step = getVolumeFromAudioSource(sinkAudioGetRoutedAudioSource(), audio_output_group_main);
    return getVolumeStepAsScaledPercentage(volume_step, audio_output_group_main);
}

static MessageId convertVolumeChangeIntoKeyPressEvent(int16 new_volume)
{
    MessageId output_event = EventInvalid;
    scaled_volume_percentage_t current_volume_percent;
    int16 current_volume;

    current_volume_percent = sinkVolumeGetCurrentVolumeAsScaledPercentage();
    current_volume = getPercentageAsVolumeStep(current_volume_percent, audio_output_group_main);

    if (current_volume > new_volume)
        output_event = EventUsrMainOutVolumeDown;
    else if (current_volume < new_volume)
        output_event = EventUsrMainOutVolumeUp;

    return output_event;
}

MessageId sinkVolumeSetCurrentVolumeAsScaledPercentage(scaled_volume_percentage_t volume)
{
    MessageId output_event;
    int16 volume_step = getPercentageAsVolumeStep(volume, audio_output_group_main);

    if (sinkAudioGetRoutedAudioSource() == audio_source_USB)
    {
        output_event = convertVolumeChangeIntoKeyPressEvent(volume_step);
    }
    else
    {
        setVolumeForAudioSource(sinkAudioGetRoutedAudioSource(),
                                audio_output_group_main,
                                volume_step);
        output_event = EventSysPeerRemoteGeneratedUpdateMainVolume;
    }

    return output_event;
}

unsigned sinkVolumeGetChargeDeficitMaxVolumeLimit(void)
{
    return GSINK_VOLUME_DATA.cdl_max_volume;
}


void sinkVolumeSetChargeDeficitMaxVolumeLimit(unsigned newLimit)
{
    GSINK_VOLUME_DATA.cdl_max_volume = newLimit;
}

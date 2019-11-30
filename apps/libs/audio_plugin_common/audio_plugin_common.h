/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_plugin_common.h
    
DESCRIPTION
    Header file for the audio plugin common library.
*/

/*!
\defgroup audio_plugin_common audio_plugin_common
\ingroup vm_libs

\brief  Header file for the audio plugin common library.
    
\section audio_plugin_common_intro INTRODUCTION
        This defines functionality common to all audio plugins.
*/

/** @{ */

#ifndef _AUDIO_PLUGIN_COMMON_H_
#define _AUDIO_PLUGIN_COMMON_H_

#include "audio_plugin_if.h"

/****************************************************************************
DESCRIPTION
	Data Description of Mic Gain Levels
*/
typedef struct tag_mic_type
{
   unsigned preamp_enable:1; /* high bit */
   unsigned unused:6;
   unsigned digital_gain:4;
   unsigned analogue_gain:5;   /* low bits */
} T_mic_gain;

/****************************************************************************
DESCRIPTION
	Data Description of a couple of common mic gain levels
*/
extern const T_mic_gain MIC_MUTE;         /* -45db, -24db, preamp=off */
extern const T_mic_gain MIC_DEFAULT_GAIN; /* +3db for digital and analog, preamp=in */

/****************************************************************************
DESCRIPTION
    Get hardware instance from mic parameters
*/
audio_instance AudioPluginGetMicInstance(const audio_mic_params params);

/****************************************************************************
DESCRIPTION
    Get hardware instance from analogue input parameters
*/
audio_instance AudioPluginGetAnalogueInputInstance(const analogue_input_params analogue_in);

/****************************************************************************
DESCRIPTION
    Get mic source
*/
Source AudioPluginGetMicSource(const audio_mic_params mic_params, audio_channel channel);

/****************************************************************************
DESCRIPTION
    Get analogue input source
*/
Source AudioPluginGetAnalogueInputSource(const analogue_input_params analogue_input, audio_channel channel);

/****************************************************************************
DESCRIPTION
    Configure Mic channel
*/
void AudioPluginSetMicRate(Source mic_source, bool digital, uint32 adc_rate);

/****************************************************************************
DESCRIPTION
    Set mic gain
*/
void AudioPluginSetMicGain(Source mic_source, bool digital, uint16 gain, bool preamp);

/****************************************************************************
DESCRIPTION
    Configure the state of the microphone bias drive
*/
void AudioPluginSetMicBiasDrive(const audio_mic_params params, bool set);

/****************************************************************************
DESCRIPTION
    Set mic bias or digital mic PIO to default state (off)
*/
void AudioPluginInitMicBiasDrive(const audio_mic_params audio_mic);


/****************************************************************************
DESCRIPTION
    Apply mic configuration and set mic PIO
*/
Source AudioPluginMicSetup(audio_channel channel, const audio_mic_params audio_mic, uint32 rate);


/****************************************************************************
DESCRIPTION
    Apply analogue input configuration and set line in PIO
*/
Source AudioPluginAnalogueInputSetup(audio_channel channel, const analogue_input_params params, uint32 rate);

/****************************************************************************
DESCRIPTION
    Apply shutdown to a mic previously setup 
*/
void AudioPluginMicShutdown(audio_channel channel, const audio_mic_params * params, bool close_mic);

/****************************************************************************
DESCRIPTION
    Configure to use rate adjustment capability
*/
void AudioPluginUseStandaloneRateAdjustment(Source source, uint32 op);

#endif /* _AUDIO_PLUGIN_COMMON_H_ */

/** @} */

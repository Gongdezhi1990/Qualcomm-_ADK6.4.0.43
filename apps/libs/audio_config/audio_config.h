/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_config.h
DESCRIPTION
    Library to implement a general purpose configuration facility.
    Provide getter and setter functionality to configuration data.
*/


#include <source.h>
#include <audio_data_types.h>

#ifndef _AUDIO_CONFIG_H_
#define _AUDIO_CONFIG_H_

#include <audio_plugin_music_params.h>
#include <audio_plugin_if.h>
#include <audio_sbc_encoder_params.h>

#include <micbias.h>

typedef enum
{
    audio_stream_music,
    audio_stream_voice
} audio_stream_t;

typedef enum
{
    audio_quality_low_power,
    audio_quality_high
} audio_quality_t;

typedef enum
{
    peer_mode_none,
    peer_mode_tws,
    peer_mode_shareme
} peer_mode_t;

typedef enum
{
    single_channel_rendering,
    multi_channel_rendering
} rendering_mode_t;

typedef enum
{
    full_band_compander,
    two_band_compander,
    three_band_compander
} compander_band_selection_t;

/****************************************************************************
DESCRIPTION
    Function store the music enhancements state
*/
void AudioConfigSetMusicEnhancements(unsigned music_mode_enhancements);

/****************************************************************************
DESCRIPTION
    Function return the music enhancements state
*/
unsigned AudioConfigGetMusicEnhancements(void);

/****************************************************************************
DESCRIPTION
    Function store the music processing mode state
*/
void AudioConfigSetMusicProcessingMode(A2DP_MUSIC_PROCESSING_T music_processing_mode);

/****************************************************************************
DESCRIPTION
    Function return the music processing mode state
*/
A2DP_MUSIC_PROCESSING_T AudioConfigGetMusicProcessingMode(void);

/****************************************************************************
DESCRIPTION
    Set if tones are played at fixed volume
*/
void AudioConfigSetTonesAreFixedVolume(bool fixed_tone_volume);

/****************************************************************************
DESCRIPTION
    Check if tones are played at fixed volume
*/
bool AudioConfigGetTonesAreFixedVolume(void);

/****************************************************************************
DESCRIPTION
    Set tone volume
*/
void AudioConfigSetToneVolume(int tone_volume);

/****************************************************************************
DESCRIPTION
    Get tone volume
*/
int AudioConfigGetToneVolume(void);

/****************************************************************************
DESCRIPTION
    Set master volume
*/
void AudioConfigSetMasterVolume(int master_volume);

/****************************************************************************
DESCRIPTION
    Get master volume
*/
int AudioConfigGetMasterVolume(void);

/****************************************************************************
DESCRIPTION
    Get current tone volume for the current fixed tone volume status
*/
int16 AudioConfigGetToneVolumeToUse(void);


/****************************************************************************
DESCRIPTION
    Set a user eq configuration parameter
*/
bool AudioConfigSetUserEqParameter(const audio_plugin_user_eq_param_t* param);

/****************************************************************************
DESCRIPTION
    Get a user eq configuration parameter
*/
audio_plugin_user_eq_param_t* AudioConfigGetUserEqParameter(const unsigned index);

/****************************************************************************
DESCRIPTION
    Get the number of stored user eq parameters
*/
unsigned AudioConfigGetNumberOfEqParams(void);

/****************************************************************************
DESCRIPTION
    Clear stored user eq parameters
*/
void AudioConfigClearUserEqParams(void);

/****************************************************************************
DESCRIPTION
    Set App Task
*/
void AudioConfigSetAppTask(Task app_task);

/****************************************************************************
DESCRIPTION
    Get App Task
*/
Task AudioConfigGetAppTask(void);

/****************************************************************************
DESCRIPTION
    Get output sample rate.
*/
uint32 AudioConfigGetOutputSampleRate(void);

/****************************************************************************
DESCRIPTION
    Set wired audio output sample rate.
*/
void AudioConfigSetWiredAudioOutputSampleRate(uint32 sample_rate);

/****************************************************************************
DESCRIPTION
    Get wired audio output sample rate.
*/
uint32 AudioConfigGetWiredAudioOutputSampleRate(void);

/****************************************************************************
DESCRIPTION
    Set number of concurrent inputs supported
*/
void AudioConfigSetMaximumConcurrentInputs(unsigned number_of_inputs);

/****************************************************************************
DESCRIPTION
    Get number of concurrent inputs supported
*/
unsigned AudioConfigGetMaximumConcurrentInputs(void);

/****************************************************************************
DESCRIPTION
    Set the Speaker PEQ Bypass
*/
void AudioConfigSetSpeakerPeqBypass(bool speaker_peq_bypass);

/****************************************************************************
DESCRIPTION
    Get the Speaker PEQ Bypass
*/
bool AudioConfigGetSpeakerPeqBypass(void);


/****************************************************************************
DESCRIPTION
    Set the PEER mode
*/
void AudioConfigSetPeerMode(peer_mode_t mode);

/****************************************************************************
DESCRIPTION
    Get the PEER mode
*/
peer_mode_t AudioConfigGetPeerMode(void);

/****************************************************************************
DESCRIPTION
    Set the tws master channel mode
*/
void AudioConfigSetTwsChannelModeLocal(AUDIO_MUSIC_CHANNEL_MODE_T music_channel_mode);

/****************************************************************************
DESCRIPTION
    Get the tws master channel mode
*/
AUDIO_MUSIC_CHANNEL_MODE_T AudioConfigGetTwsChannelModeLocal(void);

/****************************************************************************
DESCRIPTION
    Set the tws slave channel mode
*/
void AudioConfigSetTwsChannelModeRemote(AUDIO_MUSIC_CHANNEL_MODE_T channel_mode);

/****************************************************************************
DESCRIPTION
    Get the tws slave channel mode
*/
AUDIO_MUSIC_CHANNEL_MODE_T AudioConfigGetTwsChannelModeRemote(void);

/****************************************************************************
DESCRIPTION
    Set the selected peq configuration
*/
void AudioConfigSetPeqConfig(peq_config_t peq_config_selected);

/****************************************************************************
DESCRIPTION
    Get the selected peq configuration
*/
peq_config_t AudioConfigGetPeqConfig(void);

/****************************************************************************
DESCRIPTION
    Set the master mute state
*/
void AudioConfigSetMasterMuteState(AUDIO_MUTE_STATE_T mute_state);

/****************************************************************************
DESCRIPTION
    Get the master mute state
*/
AUDIO_MUTE_STATE_T AudioConfigGetMasterMuteState(void);

/****************************************************************************
DESCRIPTION
    Set the DAC output re-sampling rate
*/
void AudioConfigSetDacOutputResamplingRate(uint32 resampling_rate);

/****************************************************************************
DESCRIPTION
    Get the DAC output re-sampling rate
*/
uint32 AudioConfigGetDacOutputResamplingRate(void);

/****************************************************************************
DESCRIPTION
    Set the raw gain to be applied at the DAC outputs
*/
void AudioConfigSetRawDacGain(audio_output_t channel, uint32 raw_gain);

/****************************************************************************
DESCRIPTION
    Get the raw gain to be applied at the DAC outputs
*/
uint32 AudioConfigGetRawDacGain(audio_output_t channel);

/****************************************************************************
DESCRIPTION
    Get the quality setting
*/
audio_quality_t AudioConfigGetQuality(audio_stream_t stream);

/****************************************************************************
DESCRIPTION
    Check if the music audio stream is in low power mode
*/
bool AudioConfigIsMusicLowPower(void);

/****************************************************************************
DESCRIPTION
    Set the quality setting
*/
void AudioConfigSetQuality(audio_stream_t stream, audio_quality_t quality);

/****************************************************************************
DESCRIPTION
    Get the A2DP TTP latency settings
*/
ttp_latency_t AudioConfigGetA2DPTtpLatency(void);

/****************************************************************************
DESCRIPTION
    Get the TWS TTP latency settings
*/
ttp_latency_t AudioConfigGetTWSTtpLatency(void);

/****************************************************************************
DESCRIPTION
    Get the Wired TTP latency settings
*/
ttp_latency_t AudioConfigGetWiredTtpLatency(void);

/****************************************************************************
DESCRIPTION
    Set the A2DP TTP latency settings
*/
void AudioConfigSetA2DPTtpLatency(uint16 min, uint16 target, uint16 max);

/****************************************************************************
DESCRIPTION
    Set the TWS TTP latency settings
*/
void AudioConfigSetTWSTtpLatency(uint16 min, uint16 target, uint16 max);

/****************************************************************************
DESCRIPTION
    Set the Wired TTP latency settings
*/
void AudioConfigSetWiredTtpLatency(uint16 min, uint16 target, uint16 max);

/****************************************************************************
DESCRIPTION
    Set the Maximum Packet size
    NOTE: For AptX this value should always be larger than RTP_PACKET_LIMITED_LENGTH
*/
void AudioConfigSetPeerMtu(uint16 max_packet_size);

/****************************************************************************
DESCRIPTION
    Get the Maximum Packet size
*/
uint16 AudioConfigGetPeerMtu(void);

/****************************************************************************
DESCRIPTION
    Set the mic Params for voice assistant
*/

void AudioConfigSetVaMicParams(audio_mic_params mic_params);

/****************************************************************************
DESCRIPTION
    Get the mic params settings for Voice Assistant
*/
audio_mic_params AudioConfigGetVaMicParams(void);

/****************************************************************************
DESCRIPTION
    set the sbc encoder params settings for Voice Assistant
*/
void AudioConfigSetSbcEncoderParams(sbc_encoder_params_t sbc_encoder_params);

/****************************************************************************
DESCRIPTION
    Get the sbc encoder params settings for Voice Assistant
*/
sbc_encoder_params_t AudioConfigGetSbcEncoderParams(void);

/****************************************************************************
DESCRIPTION
    set the msbc encoder bitpool value for Voice Assistant
*/
void AudioConfigSetMsbcEncoderBitpool(uint16 bitpool_value);

/****************************************************************************
DESCRIPTION
    Get the msbc encoder bitpool value for Voice Assistant
*/
uint16 AudioConfigGetMsbcEncoderBitpool(void);

/****************************************************************************
DESCRIPTION
    Set the standalone rate adjustment value
    (FALSE - default rate adjustment at source end point; TRUE - standalone rate adjustment)
*/
void AudioConfigSetStandaloneRateAdjustment(bool value);

/****************************************************************************
DESCRIPTION
    Get the standalone rate adjustment value
*/
bool AudioConfigGetStandaloneRateAdjustment(void);

/****************************************************************************
DESCRIPTION
    Set the music rendering mode
*/
void AudioConfigSetRenderingMode(rendering_mode_t rendering_mode);

/****************************************************************************
DESCRIPTION
    Get the music rendering mode
*/
rendering_mode_t AudioConfigGetRenderingMode(void);

/****************************************************************************
DESCRIPTION
    Set the audio instance for I2S input
*/
void AudioConfigSetI2sAudioInstance(audio_instance i2s_audio_instance);

/****************************************************************************
DESCRIPTION
    Get the audio instance for I2S input
*/
audio_instance AudioConfigGetI2sAudioInstance(void);

/****************************************************************************
DESCRIPTION
    Gets the maximum input sample rate
*/
uint32 AudioConfigGetMaxSampleRate(void);

/****************************************************************************
DESCRIPTION
    Set the maximum input sample rate
*/
void AudioConfigSetMaxSampleRate(uint32 max_input_rate);

/****************************************************************************
DESCRIPTION
    Set the audio instance for SPDIF input
*/
void AudioConfigSetSpdifAudioInstance(audio_instance spdif_audio_instance);

/****************************************************************************
DESCRIPTION
    Get the audio instance for SPDIF input
*/
audio_instance AudioConfigGetSpdifAudioInstance(void);

/****************************************************************************
DESCRIPTION
    Set the microphone bias voltage
*/
void AudioConfigSetMicrophoneBiasVoltage(mic_bias_id, unsigned);

/****************************************************************************
DESCRIPTION
    Get the microphone bias voltage
*/
unsigned AudioConfigGetMicrophoneBiasVoltage(mic_bias_id);

/****************************************************************************
DESCRIPTION
    Tell the Audio hardware module to always use AEC ref operator in audio_hardware
*/
void AudioConfigSetAlwaysUseAecRef(bool use_aec);

/****************************************************************************
DESCRIPTION
    Find out if Audio hardware module will always add AEC ref operator in audio_hardware
*/
bool AudioConfigGetAlwaysUseAecRef(void);

/****************************************************************************
DESCRIPTION
    Set the A2DP letency buffer location.
    TRUE will set the SRAM as buffer location else internal memory.
*/
void AudioConfigSetUseSramForA2dp(bool value);

/****************************************************************************
DESCRIPTION
    Get the current A2DP letency buffer location.
*/
bool AudioConfigGetUseSramForA2dp(void);

/****************************************************************************
    Get the type of codec to be used when capturing VA audio
*/
void AudioConfigSetVaCaptureCodec(va_capture_codec_t capure_codec);

/****************************************************************************
DESCRIPTION
    Set the type of codec to be used when capturing VA audio
*/
va_capture_codec_t AudioConfigGetVaCaptureCodec(void);

/****************************************************************************
DESCRIPTION
    Enable/Disable VA voice trigger set from the application
*/
void AudioConfigSetVoiceTrigger(bool enable);

/****************************************************************************
DESCRIPTION
    Find out if the VA voice trigger is enabled 
*/
bool AudioConfigIsVoiceTriggerEnabled( void );

/****************************************************************************
    Get compander band that used in audio_mixer music processing chain.
*/
compander_band_selection_t AudioConfigGetMusicProcessingCompanderBand(void);

/****************************************************************************
DESCRIPTION
    Set compander band to be used in audio_mixer music processing chain.
*/
void AudioConfigSetMusicProcessingCompanderBand(compander_band_selection_t band);

#ifdef HOSTED_TEST_ENVIRONMENT
/* Reset any static variables in audio_config.c. This is only intended for unit tests. */
void AudioConfigTestReset(void);
#endif

#endif /* _AUDIO_CONFIG_H_ */

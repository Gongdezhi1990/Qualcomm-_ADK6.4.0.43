/*******************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_configure.c

DESCRIPTION
    Functions required to configure ANC Sinks/Sources.
*/

#include "anc_configure.h"
#include "anc.h"
#include "anc_data.h"
#include "anc_debug.h"
#include "anc_gain.h"
#include "anc_configure_coefficients.h"
#include <audio_anc.h>
#include <operators.h>
#include <source.h>
#include <sink.h>
#include <audio_config.h>
#include <gain_utils.h>
#include <audio_processor.h>

#define ANC_ENABLE_MASK            0x08
#define IGNORE_SAMPLE_RATE        0x00

#define ACCMD_ANC_CONTROL_DMIC_X2_A_SEL_MASK           0x0004
#define ACCMD_ANC_CONTROL_DMIC_X2_B_SEL_MASK           0x0008
#define ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT  16


static void enableAudioFramework(void)
{
    OperatorsFrameworkEnable();
    AudioProcessorAddUseCase(audio_ucid_not_defined);
}

static void disableAudioFramework(void)
{
    OperatorsFrameworkDisable();
}

static uint32 getRawGain(anc_path_enable mic_path)
{
    uint32 gain = (ancConfigDataGetHardwareGainForMicPath(mic_path));
    return ((gainUtilsCalculateRawAdcGainAnalogueComponent(gain) << 16) | RAW_GAIN_DIGITAL_COMPONENT_0_GAIN);
}

static void configureMicGain(anc_path_enable mic_path)
{
    audio_mic_params * mic_params = ancConfigDataGetMicForMicPath(mic_path);

    if(mic_params->is_digital == FALSE)
    {
        Source mic_source = AudioPluginGetMicSource(*mic_params, mic_params->channel);
        ANC_ASSERT(SourceConfigure(mic_source, STREAM_CODEC_RAW_INPUT_GAIN, getRawGain(mic_path)));
    }
}

static void configureMicGains(void)
{
    anc_mic_params_t* mic_params = ancDataGetMicParams();

    if(mic_params->enabled_mics & feed_forward_left)
    {
        configureMicGain(feed_forward_left);
    }

    if(mic_params->enabled_mics & feed_forward_right)
    {
        configureMicGain(feed_forward_right);
    }

    if(mic_params->enabled_mics & feed_back_left)
    {
        configureMicGain(feed_back_left);
    }

    if(mic_params->enabled_mics & feed_back_right)
    {
        configureMicGain(feed_back_right);
    }
}

static void associateInstance(audio_anc_instance instance, Source ffa_source, Source ffb_source)
{
    if(ffa_source)
    {
        ANC_ASSERT(SourceConfigure(ffa_source, STREAM_ANC_INSTANCE, instance));
    }
    if(ffb_source)
    {
        ANC_ASSERT(SourceConfigure(ffb_source, STREAM_ANC_INSTANCE, instance));
    }
}

static void associateInputPaths(Source ffa_source, Source ffb_source)
{
    if(ffa_source)
    {
       ANC_ASSERT(SourceConfigure(ffa_source, STREAM_ANC_INPUT, AUDIO_ANC_PATH_ID_FFA));
    }
    
    if(ffb_source)
    {
       ANC_ASSERT(SourceConfigure(ffb_source, STREAM_ANC_INPUT, AUDIO_ANC_PATH_ID_FFB)); 
    }
}

static void configureControl(audio_anc_instance instance, Source ffa_source, Source ffb_source)
{
    anc_instance_config_t * config = getInstanceConfig(instance);

    if(ffa_source)
    {
       uint32 ffa_dmic_x2_mask = (ACCMD_ANC_CONTROL_DMIC_X2_A_SEL_MASK << ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT);  
                
       if(config->feed_forward_a.upconvertor_config.dmic_x2_ff == 1)
       {
          ffa_dmic_x2_mask |= ACCMD_ANC_CONTROL_DMIC_X2_A_SEL_MASK;
       }
       ANC_ASSERT(SourceConfigure(ffa_source, STREAM_ANC_CONTROL, ffa_dmic_x2_mask));
    }
    
    if(ffb_source)
    {
       uint32 ffb_dmic_x2_mask = (ACCMD_ANC_CONTROL_DMIC_X2_B_SEL_MASK << ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT);
       
       if(config->feed_forward_b.upconvertor_config.dmic_x2_ff == 1)
       {
          ffb_dmic_x2_mask |= ACCMD_ANC_CONTROL_DMIC_X2_B_SEL_MASK;
       }
       ANC_ASSERT(SourceConfigure(ffb_source, STREAM_ANC_CONTROL, ffb_dmic_x2_mask));
    }
}

static void configureAncInstance(audio_anc_instance instance, anc_path_enable ffa_path, anc_path_enable ffb_path)
{
    audio_mic_params * mic_params = ancConfigDataGetMicForMicPath(ffa_path);
    Source ffa_source = AudioPluginMicSetup(mic_params->channel, *mic_params, IGNORE_SAMPLE_RATE);
    Source ffb_source = NULL;

    if(ffb_path)
    {
       mic_params = ancConfigDataGetMicForMicPath(ffb_path);
       ffb_source = AudioPluginMicSetup(mic_params->channel, *mic_params, IGNORE_SAMPLE_RATE);
    }

    associateInstance(instance, ffa_source, ffb_source);
    associateInputPaths(ffa_source, ffb_source);
    configureControl(instance, ffa_source, ffb_source);
}

static void configureFeedForwardModeMics(void)
{
    anc_mic_params_t* mic_params = ancDataGetMicParams();

    if(mic_params->enabled_mics & feed_forward_left)
    {
        configureAncInstance(AUDIO_ANC_INSTANCE_0, feed_forward_left, all_disabled);
    }

    if(mic_params->enabled_mics & feed_forward_right)
    {
        configureAncInstance(AUDIO_ANC_INSTANCE_1, feed_forward_right, all_disabled);
    }
}

static void configureFeedBackModeMics(void)
{
    anc_mic_params_t* mic_params = ancDataGetMicParams();

    if(mic_params->enabled_mics & feed_back_left)
    {
        configureAncInstance(AUDIO_ANC_INSTANCE_0, feed_back_left, all_disabled);
    }

    if(mic_params->enabled_mics & feed_back_right)
    {
        configureAncInstance(AUDIO_ANC_INSTANCE_1, feed_back_right, all_disabled);
    }
}

static void configureHybridModeMics(void)
{
    anc_mic_params_t* mic_params = ancDataGetMicParams();
    if(mic_params->enabled_mics & feed_forward_left)
    {
        configureAncInstance(AUDIO_ANC_INSTANCE_0, feed_back_left, feed_forward_left);
    }
    if(mic_params->enabled_mics & feed_forward_right)
    {
        configureAncInstance(AUDIO_ANC_INSTANCE_1, feed_back_right, feed_forward_right);
    }
}

/******************************************************************************/
static void configureMics(void)
{
    anc_mic_params_t* mic_params = ancDataGetMicParams();

    switch(mic_params->enabled_mics)
    {
        case hybrid_mode:
        case hybrid_mode_left_only:
        case hybrid_mode_right_only:
            configureHybridModeMics();
            break;

        case feed_back_mode:
        case feed_back_mode_left_only:
        case feed_back_mode_right_only:
            configureFeedBackModeMics();
            break;

        case feed_forward_mode:
        case feed_forward_mode_left_only:
        case feed_forward_mode_right_only:
            configureFeedForwardModeMics();
            break;

        default:
            ANC_PANIC();
            break;
    }

    configureMicGains();
}

static void deconfigureMicSource(anc_path_enable mic_path)
{
    audio_mic_params * mic_params = ancConfigDataGetMicForMicPath(mic_path);    
    Source mic_source = AudioPluginGetMicSource(*mic_params, mic_params->channel);
    ANC_ASSERT(SourceConfigure(mic_source, STREAM_ANC_INPUT, AUDIO_ANC_PATH_ID_NONE));
    ANC_ASSERT(SourceConfigure(mic_source, STREAM_ANC_INSTANCE, AUDIO_ANC_INSTANCE_NONE));
    AudioPluginMicShutdown(mic_params->channel, mic_params, FALSE);
}

/******************************************************************************/
static void deconfigureMics(void)
{
    anc_mic_params_t* mic_params = ancDataGetMicParams();

    if(mic_params->enabled_mics & feed_forward_left)
    {
        deconfigureMicSource(feed_forward_left);
    }

    if(mic_params->enabled_mics & feed_forward_right)
    {
        deconfigureMicSource(feed_forward_right);
    }

    if(mic_params->enabled_mics & feed_back_left)
    {
        deconfigureMicSource(feed_back_left);
    }

    if(mic_params->enabled_mics & feed_back_right)
    {
        deconfigureMicSource(feed_back_right);
    }
}

static void configureDacChannel(audio_channel channel, audio_anc_instance instance)
{
    Sink dac_channel = StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, channel);
    ANC_ASSERT(SinkConfigure(dac_channel, STREAM_ANC_INSTANCE, instance));
}

/******************************************************************************/
static void configureDacs(void)
{
    if(ancDataIsLeftChannelConfigurable())
    {
        configureDacChannel(AUDIO_CHANNEL_A, AUDIO_ANC_INSTANCE_0);
        AudioConfigSetRawDacGain(audio_output_primary_left, ancDataGetConfigData()->hardware_gains.dac_output_left);
    }
    if(ancDataIsRightChannelConfigurable())
    {
        configureDacChannel(AUDIO_CHANNEL_B, AUDIO_ANC_INSTANCE_1);
        AudioConfigSetRawDacGain(audio_output_primary_right, ancDataGetConfigData()->hardware_gains.dac_output_right);
    }
}

/******************************************************************************/
static void deconfigureDacs(void)
{
    if(ancDataIsLeftChannelConfigurable())
    {
        configureDacChannel(AUDIO_CHANNEL_A, AUDIO_ANC_INSTANCE_NONE);
    }
    if(ancDataIsRightChannelConfigurable())
    {
        configureDacChannel(AUDIO_CHANNEL_B, AUDIO_ANC_INSTANCE_NONE);
    }
}

/******************************************************************************/
static void ancStreamEnable(bool enable)
{
    uint16 enable_instance_0 = ((enable && ancDataIsLeftChannelConfigurable()) ? ancDataGetCurrentModeConfig()->instance[ANC_INSTANCE_0_INDEX].enable_mask : 0);
    uint16 enable_instance_1 = ((enable && ancDataIsRightChannelConfigurable()) ? ancDataGetCurrentModeConfig()->instance[ANC_INSTANCE_1_INDEX].enable_mask : 0);

    ANC_ASSERT(AudioAncStreamEnable(enable_instance_0, enable_instance_1));
}

static void enableAnc(void)
{
    enableAudioFramework();
    configureMics();
    configureDacs();
    ancConfigureCoefficientsAndGains();
    ancStreamEnable(TRUE);
}

static void disableAnc(void)
{
    ancStreamEnable(FALSE);
    deconfigureMics();
    deconfigureDacs();
    disableAudioFramework();
}

void ancConfigureFinetuneGains(void)
{
    /* Not required */
}

void ancConfigureSidetoneGains(void)
{
    /* Not required */
}

bool ancConfigure(bool enable)
{
    enable ? enableAnc() : disableAnc();
    return TRUE;
}

bool ancConfigureAfterModeChange(void)
{
    ancConfigureMutePathGains();
    ancConfigureCoefficientsAndGains();
    return TRUE;
}

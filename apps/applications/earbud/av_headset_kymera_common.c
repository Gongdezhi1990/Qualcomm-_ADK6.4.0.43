/*!
\copyright  Copyright (c) 2017-2018  Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Kymera common code
*/

#include <audio_clock.h>
#include <audio_power.h>
#include <pio_common.h>

#include "av_headset_kymera_private.h"

#include "chains/chain_output_volume.h"

/*! Convert a channel ID to a bit mask */
#define CHANNEL_TO_MASK(channel) ((uint32)1 << channel)

/*!@{ \name Port numbers for the Source Sync operator */
#define KYMERA_SOURCE_SYNC_INPUT_PORT (0)
#define KYMERA_SOURCE_SYNC_OUTPUT_PORT (0)
/*!@} */

/* Configuration of source sync groups and routes */
static const source_sync_sink_group_t sink_groups[] =
{
    {
        .meta_data_required = TRUE,
        .rate_match = FALSE,
        .channel_mask = CHANNEL_TO_MASK(KYMERA_SOURCE_SYNC_INPUT_PORT)
    }
};

static const source_sync_source_group_t source_groups[] =
{
    {
        .meta_data_required = TRUE,
        .ttp_required = TRUE,
        .channel_mask = CHANNEL_TO_MASK(KYMERA_SOURCE_SYNC_OUTPUT_PORT)
    }
};

static source_sync_route_t routes[] =
{
    {
        .input_terminal = KYMERA_SOURCE_SYNC_INPUT_PORT,
        .output_terminal = KYMERA_SOURCE_SYNC_OUTPUT_PORT,
        .transition_samples = 0,
        .sample_rate = 0, /* Overridden later */
        .gain = 0
    }
};

int32 volTo60thDbGain(uint16 volume)
{
    int32 gain = -90;
    if (volume)
    {
        int32 minv = appConfigMinVolumedB();
        int32 maxv = appConfigMaxVolumedB();
        gain = volume * (maxv - minv);
        gain /= VOLUME_MAX;
        gain += minv;
    }
    return gain * KYMERA_DB_SCALE;
}

void appKymeraSetState(appKymeraState state)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOGF("appKymeraSetState, state %u -> %u", theKymera->state, state);
    theKymera->state = state;

    /* Set busy lock if not in idle or tone state */
    theKymera->busy_lock = (state != KYMERA_STATE_IDLE) && (state != KYMERA_STATE_TONE_PLAYING);
}

void appKymeraConfigureDspPowerMode(bool tone_playing)
{
#if (defined(__QCC302X_APPS__) || defined(__QCC512X_APPS__) || defined(__QCC3400_APP__) || defined(__QCC514X_APPS__))
    kymeraTaskData *theKymera = appGetKymera();

    DEBUG_LOGF("appKymeraConfigureDspPowerMode, tone %u, state %u, a2dp seid %u", tone_playing, appKymeraGetState(), theKymera->a2dp_seid);
    
    /* Assume we are switching to the low power slow clock unless one of the
     * special cases below applies */
    audio_dsp_clock_configuration cconfig =
    {
        .active_mode = AUDIO_DSP_SLOW_CLOCK,
        .low_power_mode = AUDIO_DSP_CLOCK_NO_CHANGE,
        .trigger_mode = AUDIO_DSP_CLOCK_NO_CHANGE
    };
    
    audio_dsp_clock kclocks;
    audio_power_save_mode mode = AUDIO_POWER_SAVE_MODE_3;

    switch (appKymeraGetState())
    {
        case KYMERA_STATE_A2DP_STARTING_A:
        case KYMERA_STATE_A2DP_STARTING_B:
        case KYMERA_STATE_A2DP_STARTING_C:
        case KYMERA_STATE_A2DP_STREAMING:
        case KYMERA_STATE_A2DP_STREAMING_WITH_FORWARDING:
        {
            if (tone_playing)
            {
                /* Always jump up to normal clock for tones - for most codecs there is
                 * not enough MIPs when running on a slow clock to also play a tone */
                cconfig.active_mode = AUDIO_DSP_BASE_CLOCK;
                mode = AUDIO_POWER_SAVE_MODE_1;                
            }
            else
            {
                /* Either setting up for the first time or returning from a tone, in
                 * either case return to the default clock rate for the codec in use */
                switch (theKymera->a2dp_seid)
                {
                    case AV_SEID_APTX_SNK:
                    case AV_SEID_APTX_ADAPTIVE_SNK:
                    case AV_SEID_APTX_ADAPTIVE_TWS_SNK:
                    {
                        /* Not enough MIPs to run aptX master (TWS standard) or
                         * aptX adaptive (TWS standard and TWS+) on slow clock */
                        cconfig.active_mode = AUDIO_DSP_BASE_CLOCK;
                        mode = AUDIO_POWER_SAVE_MODE_1;
                    }
                    break;
        
                    default:
                        break;
                }
            }
        }
        break;
        
        case KYMERA_STATE_SCO_ACTIVE:
        case KYMERA_STATE_SCO_ACTIVE_WITH_FORWARDING:
        case KYMERA_STATE_SCO_SLAVE_ACTIVE:
        {
            if (theKymera->sco_info)
            {
                switch (theKymera->sco_info->mode)
                {
                    case SCO_NB:
                    case SCO_WB:
                    {
                        /* Always jump up to normal clock (80Mhz) for NB or WB CVC */
                        cconfig.active_mode = AUDIO_DSP_BASE_CLOCK;
                        mode = AUDIO_POWER_SAVE_MODE_1;
                    }
                    break;

                    case SCO_SWB:
                    case SCO_UWB:
                    {
                        /* Always jump up to turbo clock (120Mhz) for SWB or UWB CVC */
                        cconfig.active_mode = AUDIO_DSP_TURBO_CLOCK;
                        mode = AUDIO_POWER_SAVE_MODE_1;
                    }
                    break;

                    default:
                        break;
                }
            }
        }
        break;

        case KYMERA_STATE_ANC_TUNING:
        {
            /* Always jump up to turbo clock (120Mhz) for ANC tuning */
            cconfig.active_mode = AUDIO_DSP_TURBO_CLOCK;
            mode = AUDIO_POWER_SAVE_MODE_1;
        }
        break;        
        
        case KYMERA_STATE_IDLE:
        case KYMERA_STATE_TONE_PLAYING:
        {
            /* All other states default to slow */
        }
        break;        
    }

#ifdef AUDIO_IN_SQIF
    /* Make clock faster when running from SQIF */
    cconfig.active_mode += 1;
#endif
    
    PanicFalse(AudioDspClockConfigure(&cconfig));
    PanicFalse(AudioPowerSaveModeSet(mode));

    PanicFalse(AudioDspGetClock(&kclocks));
    mode = AudioPowerSaveModeGet();
    DEBUG_LOGF("appKymeraConfigureDspPowerMode, kymera clocks %d %d %d, mode %d", kclocks.active_mode, kclocks.low_power_mode, kclocks.trigger_mode, mode);
#elif defined(__CSRA68100_APP__)
    /* No DSP clock control on CSRA68100 */
    UNUSED(tone_playing);
#else
    #error DSP clock not handled for this chip
#endif
}

void appKymeraExternalAmpSetup(void)
{
    if (appConfigExternalAmpControlRequired())
    {
        kymeraTaskData *theKymera = appGetKymera();
        int pio_mask = (1 << (appConfigExternalAmpControlPio() % 32));
        int pio_bank = appConfigExternalAmpControlPio() / 32;

        /* Reset usage count */
        theKymera->dac_amp_usage = 0;

        /* map in PIO */
        PioSetMapPins32Bank(pio_bank, pio_mask, pio_mask);
        /* set as output */
        PioSetDir32Bank(pio_bank, pio_mask, pio_mask);
        /* start disabled */
        PioSet32Bank(pio_bank, pio_mask,
                     appConfigExternalAmpControlDisableMask());
    }
}

void appKymeraExternalAmpControl(bool enable)
{
    kymeraTaskData *theKymera = appGetKymera();

    if (appConfigExternalAmpControlRequired())
    {        
        theKymera->dac_amp_usage += enable ? 1 : - 1;

        /* Drive PIO high if enabling AMP and usage has gone from 0 to 1,
         * Drive PIO low if disabling AMP and usage has gone from 1 to 0 */
        if ((enable && theKymera->dac_amp_usage == 1) ||
            (!enable && theKymera->dac_amp_usage == 0))
        {
            int pio_mask = (1 << (appConfigExternalAmpControlPio() % 32));
            int pio_bank = appConfigExternalAmpControlPio() / 32;

            PioSet32Bank(pio_bank, pio_mask,
                         enable ? appConfigExternalAmpControlEnableMask() :
                                  appConfigExternalAmpControlDisableMask());
        }
    }

    if(enable)
    {
        /* If we're enabling the amp then also call OperatorFrameworkEnable() so that the audio S/S will
           remain on even if the audio chain is destroyed, this allows us to control the timing of when the audio S/S
           and DACs are powered off to mitigate audio pops and clicks.*/

        /* Cancel any pending audio s/s disable message since we're enabling.  If message was cancelled no need
           to call OperatorFrameworkEnable() as audio S/S is still powered on from previous time */
        if(MessageCancelFirst(&theKymera->task, KYMERA_INTERNAL_AUDIO_SS_DISABLE))
        {
            DEBUG_LOG("appKymeraExternalAmpControl, there is already a client for the audio SS");
        }
        else
        {
            DEBUG_LOG("appKymeraExternalAmpControl, adding a client to the audio SS");
            OperatorFrameworkEnable(MAIN_PROCESSOR_ON);
        }
    }
    else
    {
        /* If we're disabling the amp then send a timed message that will turn off the audio s/s later rather than 
           immediately */
        DEBUG_LOG("appKymeraExternalAmpControl, sending later KYMERA_INTERNAL_AUDIO_SS_DISABLE");
        MessageSendLater(&theKymera->task, KYMERA_INTERNAL_AUDIO_SS_DISABLE, NULL, appConfigDacDisconnectionDelayMs());
    }
}

void appKymeraConfigureSpcMode(Operator op, bool is_consumer)
{
    uint16 msg[2];
    msg[0] = OPMSG_SPC_ID_TRANSITION; /* MSG ID to set SPC mode transition */
    msg[1] = is_consumer;
    PanicFalse(OperatorMessage(op, msg, 2, NULL, 0));
}

void appKymeraConfigureSpcDataFormat(Operator op, audio_data_format format)
{
#if defined(HAVE_STR_ROM_2_0_1) || defined(HAVE_STRPLUS_ROM)
    if (format == ADF_GENERIC_ENCODED)
        format = ADF_GENERIC_ENCODED_32BIT;
#endif                
    uint16 msg[2];
    msg[0] = OPMSG_OP_TERMINAL_DATA_FORMAT; /* MSG ID to set SPC data type */
    msg[1] = format;
    PanicFalse(OperatorMessage(op, msg, 2, NULL, 0));
}

void appKymeraConfigureScoSpcDataFormat(Operator op, audio_data_format format)
{
    uint16 msg[2];
    msg[0] = OPMSG_OP_TERMINAL_DATA_FORMAT; /* MSG ID to set SPC data type */
    msg[1] = format;
    PanicFalse(OperatorMessage(op, msg, 2, NULL, 0));
}

static void appKymeraConfigureSourceSync(Operator op, uint32 rate, unsigned kick_period_us)
{
    /* Override sample rate in routes config */
    routes[0].sample_rate = rate;

    /* Send operator configuration messages */
    OperatorsStandardSetSampleRate(op, rate);
    OperatorsSourceSyncSetSinkGroups(op, DIMENSION_AND_ADDR_OF(sink_groups));
    OperatorsSourceSyncSetSourceGroups(op, DIMENSION_AND_ADDR_OF(source_groups));
    OperatorsSourceSyncSetRoutes(op, DIMENSION_AND_ADDR_OF(routes));

    /* Output buffer needs to be able to hold at least SS_MAX_PERIOD worth
     * of audio (default = 2 * Kp), but be less than SS_MAX_LATENCY (5 * Kp).
     * The recommendation is 2 Kp more than SS_MAX_PERIOD, so 4 * Kp. */
    OperatorsStandardSetBufferSizeWithFormat(op, US_TO_BUFFER_SIZE_MONO_PCM(4 * kick_period_us, rate), operator_data_format_pcm);
}

void appKymeraSetMainVolume(kymera_chain_handle_t chain, uint16 volume)
{
    Operator volop;

    if (GET_OP_FROM_CHAIN(volop, chain, OPR_VOLUME_CONTROL))
    {
        OperatorsVolumeSetMainGain(volop, volTo60thDbGain(volume));
    }
}

void appKymeraSetVolume(kymera_chain_handle_t chain, uint16 volume)
{
    Operator volop;

    if (GET_OP_FROM_CHAIN(volop, chain, OPR_VOLUME_CONTROL))
    {
        OperatorsVolumeSetMainAndAuxGain(volop, volTo60thDbGain(volume));
    }
}

void appKymeraConfigureOutputChainOperators(kymera_chain_handle_t chain,
                                            uint32 sample_rate, unsigned kick_period,
                                            unsigned buffer_size, uint8 volume)
{
    Operator sync_op;
    Operator volume_op;

    /* Configure operators */
    if (GET_OP_FROM_CHAIN(sync_op, chain, OPR_SOURCE_SYNC))
    {
        /* SourceSync is optional in chains. */
        appKymeraConfigureSourceSync(sync_op, sample_rate, kick_period);
    }

    volume_op = ChainGetOperatorByRole(chain, OPR_VOLUME_CONTROL);
    OperatorsStandardSetSampleRate(volume_op, sample_rate);

    appKymeraSetVolume(chain, volume);

    if (buffer_size)
    {
        Operator op = ChainGetOperatorByRole(chain, OPR_LATENCY_BUFFER);
        OperatorsStandardSetBufferSizeWithFormat(op, buffer_size, operator_data_format_pcm);
    }
}

void appKymeraCreateOutputChain(uint32 rate, unsigned kick_period,
                                unsigned buffer_size, uint8 volume)
{
    kymeraTaskData *theKymera = appGetKymera();
    Sink dac;
    kymera_chain_handle_t chain;

    /* Create chain */
    chain = ChainCreate(&chain_output_volume_config);
    theKymera->chainu.output_vol_handle = chain;
    appKymeraConfigureOutputChainOperators(chain, rate, kick_period, buffer_size, volume);

    /* Configure the DAC channel */
    dac = StreamAudioSink(appConfigLeftAudioHardware(), appConfigLeftAudioInstance(), appConfigLeftAudioChannel());
    PanicFalse(SinkConfigure(dac, STREAM_CODEC_OUTPUT_RATE, rate));
    PanicFalse(SinkConfigure(dac, STREAM_RM_ENABLE_DEFERRED_KICK, 0));

    /* Connect chain output to the DAC */
    ChainConnect(chain);
    PanicFalse(ChainConnectOutput(chain, dac, EPR_SOURCE_MIXER_OUT));
}

/*! \brief Set the UCID for a single operator */
static bool appKymeraSetOperatorUcid(kymera_chain_handle_t chain, chain_operator_role_t role, kymera_operator_ucid_t ucid)
{
    Operator op;
    if (GET_OP_FROM_CHAIN(op, chain, role))
    {
        OperatorsStandardSetUCID(op, ucid);
        return TRUE;
    }
    return FALSE;
}

void appKymeraSetOperatorUcids(bool is_sco, appKymeraScoMode mode)
{
    /* Operators that have UCID set either reside in sco_handle or output_vol_handle
       which are both in the chainu union. */
    kymera_chain_handle_t chain = appGetKymera()->chainu.output_vol_handle;

    if (is_sco)
    {
        /* All SCO chains have AEC */
        const uint8 aec_ucid[] = { 0, UCID_AEC_NB, UCID_AEC_WB, UCID_AEC_SWB, UCID_AEC_UWB };
        PanicFalse(appKymeraSetOperatorUcid(chain, OPR_SCO_AEC, aec_ucid[mode]));

        /* SCO/MIC forwarding RX chains do not have CVC send or receive */
        appKymeraSetOperatorUcid(chain, OPR_CVC_SEND, UCID_CVC_SEND);
        appKymeraSetOperatorUcid(chain, OPR_CVC_RECEIVE, UCID_CVC_RECEIVE);
    }

    /* Operators common to SCO/A2DP chains */
    PanicFalse(appKymeraSetOperatorUcid(chain, OPR_VOLUME_CONTROL, UCID_VOLUME_CONTROL));
    PanicFalse(appKymeraSetOperatorUcid(chain, OPR_SOURCE_SYNC, UCID_SOURCE_SYNC));
}


void appKymeraMicInit(void)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOG("appKymeraMicInit");

    theKymera->mic_params[0].bias_config = appConfigMic0Bias();
    theKymera->mic_params[0].pre_amp = appConfigMic0PreAmp();
    theKymera->mic_params[0].pio = appConfigMic0Pio();
    theKymera->mic_params[0].gain = appConfigMic0Gain();
    theKymera->mic_params[0].is_digital = appConfigMic0IsDigital();
    theKymera->mic_params[0].instance = appConfigMic0AudioInstance();
    theKymera->mic_params[0].channel = appConfigMic0AudioChannel();

    theKymera->mic_params[1].bias_config = appConfigMic1Bias();
    theKymera->mic_params[1].pre_amp = appConfigMic1PreAmp();
    theKymera->mic_params[1].pio = appConfigMic1Pio();
    theKymera->mic_params[1].gain = appConfigMic1Gain();
    theKymera->mic_params[1].is_digital = appConfigMic1IsDigital();
    theKymera->mic_params[1].instance = appConfigMic1AudioInstance();
    theKymera->mic_params[1].channel = appConfigMic1AudioChannel();

    theKymera->mic_params[2].bias_config = appConfigMic2Bias();
    theKymera->mic_params[2].pre_amp = appConfigMic2PreAmp();
    theKymera->mic_params[2].pio = appConfigMic2Pio();
    theKymera->mic_params[2].gain = appConfigMic2Gain();
    theKymera->mic_params[2].is_digital = appConfigMic2IsDigital();
    theKymera->mic_params[2].instance = appConfigMic2AudioInstance();
    theKymera->mic_params[2].channel = appConfigMic2AudioChannel();

    theKymera->mic_params[3].bias_config = appConfigMic3Bias();
    theKymera->mic_params[3].pre_amp = appConfigMic3PreAmp();
    theKymera->mic_params[3].pio = appConfigMic3Pio();
    theKymera->mic_params[3].gain = appConfigMic3Gain();
    theKymera->mic_params[3].is_digital = appConfigMic3IsDigital();
    theKymera->mic_params[3].instance = appConfigMic3AudioInstance();
    theKymera->mic_params[3].channel = appConfigMic3AudioChannel();

    /* Ensure PIOs are mapped correctly */
    uint8 mic;
    pio_common_allbits mask;
    PioCommonBitsInit(&mask);
    for (mic = 0; mic < 4; mic++)
    {
        if (theKymera->mic_params[mic].bias_config == BIAS_CONFIG_PIO)
            PioCommonBitsSetBit(&mask, theKymera->mic_params[mic].pio);
    }
    PioCommonSetMap(&mask, &mask);
}

void appKymeraMicSetup(uint8 mic_1a, Source *p_mic_src_1a, uint8 mic_1b, Source *p_mic_src_1b, uint16 rate)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOG("appKymeraMicSetup");

    PanicFalse(mic_1a != NO_MIC);
    PanicFalse(p_mic_src_1b || (mic_1b == NO_MIC));
    Source mic_src_0 = AudioPluginMicSetup(theKymera->mic_params[mic_1a].channel, theKymera->mic_params[mic_1a], rate);
    Source mic_src_1 = (mic_1b != NO_MIC) ? AudioPluginMicSetup(theKymera->mic_params[mic_1b].channel, theKymera->mic_params[mic_1b], rate) : 0;
    if (mic_src_0 && mic_src_1)
        SourceSynchronise(mic_src_0, mic_src_1);
    *p_mic_src_1a = mic_src_0;
    *p_mic_src_1b = mic_src_1;

    /* Default to 0 gain */
    if (mic_src_0 && !theKymera->mic_params[mic_1a].is_digital)
        PanicFalse(SourceConfigure(mic_src_0, STREAM_CODEC_RAW_INPUT_GAIN, 0x8020));
    if (mic_src_1 && !theKymera->mic_params[mic_1b].is_digital)
        PanicFalse(SourceConfigure(mic_src_1, STREAM_CODEC_RAW_INPUT_GAIN, 0x8020));
}

void appKymeraMicCleanup(uint8 mic_1a, uint8 mic1b)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOG("appKymeraMicCleanup");

    PanicFalse(mic_1a != NO_MIC);
    Source mic_src_0 = AudioPluginGetMicSource(theKymera->mic_params[mic_1a], theKymera->mic_params[mic_1a].channel);
    Source mic_src_1 = (mic1b != NO_MIC) ? AudioPluginGetMicSource(theKymera->mic_params[mic1b], theKymera->mic_params[mic1b].channel) : 0;
    SourceClose(mic_src_0);
    if (mic_src_1)
        SourceClose(mic_src_1);

    /* Disable microphone bias if ANC is not enabled */
    if (!appKymeraAncIsEnabled())
    {
        DEBUG_LOG("appKymeraMicCleanup, disable MIC bias for SCO MIC 1");
        AudioPluginSetMicBiasDrive(theKymera->mic_params[appConfigScoMic1()], FALSE);
        if (appConfigScoMic2() != NO_MIC)
        {
            DEBUG_LOG("appKymeraMicCleanup, disable MIC bias for SCO MIC 2");
            AudioPluginSetMicBiasDrive(theKymera->mic_params[appConfigScoMic2()], FALSE);
        }
    }
}


unsigned AudioConfigGetMicrophoneBiasVoltage(mic_bias_id id)
{
    unsigned bias = 0;
    if (id == MIC_BIAS_0)
    {
        if (appConfigMic0Bias() == BIAS_CONFIG_MIC_BIAS_0)
            bias =  appConfigMic0BiasVoltage();
        else if (appConfigMic1Bias() == BIAS_CONFIG_MIC_BIAS_0)
            bias = appConfigMic1BiasVoltage();
        else
            Panic();
    }
    else if (id == MIC_BIAS_1)
    {
        if (appConfigMic0Bias() == BIAS_CONFIG_MIC_BIAS_1)
            bias = appConfigMic0BiasVoltage();
        else if (appConfigMic1Bias() == BIAS_CONFIG_MIC_BIAS_1)
            bias = appConfigMic1BiasVoltage();
        else
            Panic();
    }
    else
        Panic();

    DEBUG_LOGF("AudioConfigGetMicrophoneBiasVoltage, id %u, bias %u", id, bias);
    return bias;
}

void AudioConfigSetRawDacGain(audio_output_t channel, uint32 raw_gain)
{
    DEBUG_LOGF("AudioConfigSetRawDacGain, channel %u, gain %u", channel, raw_gain);
    if (channel == audio_output_primary_left)
    {
        Sink sink = StreamAudioSink(appConfigLeftAudioHardware(), appConfigLeftAudioInstance(), appConfigLeftAudioChannel());
        PanicFalse(SinkConfigure(sink, STREAM_CODEC_RAW_OUTPUT_GAIN, raw_gain));
    }
    else
        Panic();
}

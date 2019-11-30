/*!
\copyright  Copyright (c) 2017-2018  Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Kymera tones / prompts
*/

#include "av_headset_kymera_private.h"

#include "chains/chain_tone_gen.h"
#include "chains/chain_tone_gen_no_iir.h"
#include "chains/chain_prompt_decoder.h"
#include "chains/chain_prompt_decoder_no_iir.h"
#include "chains/chain_prompt_pcm.h"

/*! \brief Setup the prompt audio source.
    \param source The prompt audio source.
*/
static void appKymeraPromptSourceSetup(Source source)
{
    kymeraTaskData *theKymera = appGetKymera();
    MessageStreamTaskFromSource(source, &theKymera->task);
}

/*! \brief Close the prompt audio source.
    \param source The prompt audio source.
*/
static void appKymeraPromptSourceClose(Source source)
{
    if (source)
    {
        MessageStreamTaskFromSource(source, NULL);
        StreamDisconnect(source, NULL);
        SourceClose(source);
    }
}

/*! \brief Create the tone / prompt audio chain.
    \param msg Message containing the create parameters.
*/
static Source appKymeraCreateTonePromptChain(const KYMERA_INTERNAL_TONE_PROMPT_PLAY_T *msg)
{
    kymeraTaskData *theKymera = appGetKymera();
    kymera_chain_handle_t chain = NULL;
    const chain_config_t *config = NULL;
    const bool has_resampler = (theKymera->output_rate != msg->rate);
    const bool is_tone = (msg->tone != NULL);
    const bool is_prompt = (msg->prompt != FILE_NONE);

    /* Can play tone or prompt, not both */
    PanicFalse(is_tone != is_prompt);

    if (is_tone)
    {
        config = has_resampler ? &chain_tone_gen_config : &chain_tone_gen_no_iir_config;
    }
    else
    {
        if (msg->prompt_format == PROMPT_FORMAT_SBC)
        {
            config = has_resampler ? &chain_prompt_decoder_config : &chain_prompt_decoder_no_iir_config;
        }
        else if (msg->prompt_format == PROMPT_FORMAT_PCM)
        {
            /* If PCM at the correct rate, no chain required at all. */
            config = has_resampler ? &chain_prompt_pcm_config : NULL;
        }
    }

    if (config)
    {
        Operator op;
        chain = ChainCreate(config);
        if (has_resampler)
        {
            /* Configure resampler */
            op = ChainGetOperatorByRole(chain, OPR_TONE_PROMPT_RESAMPLER);
            OperatorsResamplerSetConversionRate(op, msg->rate, theKymera->output_rate);
        }

        if (is_tone)
        {
            /* Configure ringtone generator */
            op = ChainGetOperatorByRole(chain, OPR_TONE_GEN);
            OperatorsStandardSetSampleRate(op, msg->rate);
            OperatorsConfigureToneGenerator(op, msg->tone, &theKymera->task);
        }
        ChainConnect(chain);
        theKymera->chain_tone_handle = chain;
    }

    if (is_prompt)
    {
        /* Configure prompt file source */
        theKymera->prompt_source = PanicNull(StreamFileSource(msg->prompt));
        appKymeraPromptSourceSetup(theKymera->prompt_source);
        if (chain)
        {
            PanicFalse(ChainConnectInput(chain, theKymera->prompt_source, EPR_PROMPT_IN));
        }
        else
        {
            /* No chain (prompt is PCM at the correct sample rate) so the source
            is just the file */
            return theKymera->prompt_source;
        }
    }
    return ChainGetOutput(chain, EPR_TONE_PROMPT_CHAIN_OUT);
}

void appKymeraHandleInternalTonePromptPlay(const KYMERA_INTERNAL_TONE_PROMPT_PLAY_T *msg)
{
    kymeraTaskData *theKymera = appGetKymera();
    Operator op;

    DEBUG_LOGF("appKymeraHandleInternalTonePromptPlay, prompt %x, tone %p, int %u, lock 0x%x, mask 0x%x",
                msg->prompt, msg->tone, msg->interruptible, msg->client_lock, msg->client_lock_mask);

    /* If there is a tone still playing at this point, it must be an interruptable tone, so cut it off */
    appKymeraTonePromptStop();

    switch (appKymeraGetState())
    {
        case KYMERA_STATE_IDLE:
            /* Need to set up audio output chain to play tone from scratch */
            appKymeraCreateOutputChain(msg->rate, KICK_PERIOD_TONES, 0, 0);
            appKymeraExternalAmpControl(TRUE);
            ChainStart(theKymera->chainu.output_vol_handle);
            op = ChainGetOperatorByRole(theKymera->chainu.output_vol_handle, OPR_VOLUME_CONTROL);
            OperatorsVolumeMute(op, FALSE);

            /* Update state variables */
            appKymeraSetState(KYMERA_STATE_TONE_PLAYING);
            theKymera->output_rate = msg->rate;

        // fall-through
        case KYMERA_STATE_SCO_ACTIVE:
        case KYMERA_STATE_SCO_ACTIVE_WITH_FORWARDING:
        case KYMERA_STATE_SCO_SLAVE_ACTIVE:
        case KYMERA_STATE_A2DP_STREAMING:
        case KYMERA_STATE_A2DP_STREAMING_WITH_FORWARDING:
        {
            kymera_chain_handle_t output_chain = theKymera->chainu.output_vol_handle;
            Source aux_source = appKymeraCreateTonePromptChain(msg);
            int16 volume_db = (msg->tone != NULL) ? (KYMERA_DB_SCALE * APP_UI_TONE_VOLUME) :
                                                    (KYMERA_DB_SCALE * APP_UI_PROMPT_VOLUME);

            /* Connect tone/prompt chain to output */
            ChainConnectInput(output_chain, aux_source, EPR_VOLUME_AUX);
            /* Set tone/prompt volume */
            op = ChainGetOperatorByRole(theKymera->chainu.output_vol_handle, OPR_VOLUME_CONTROL);
            OperatorsVolumeSetAuxGain(op, volume_db);

            /* May need to exit low power mode to play tone simultaneously */
            appKymeraConfigureDspPowerMode(TRUE);

            /* Start tone */
            if (theKymera->chain_tone_handle)
            {
                ChainStart(theKymera->chain_tone_handle);
            }
        }
        break;

        case KYMERA_STATE_TONE_PLAYING:
        default:
            /* Unknown state / not supported */
            DEBUG_LOGF("appKymeraHandleInternalTonePromptPlay, unsupported state %u", appKymeraGetState());
            Panic();
            break;
    }
    if (!msg->interruptible)
    {
        appKymeraSetToneLock(theKymera);
    }
    theKymera->tone_client_lock = msg->client_lock;
    theKymera->tone_client_lock_mask = msg->client_lock_mask;
}

void appKymeraTonePromptStop(void)
{
    kymeraTaskData *theKymera = appGetKymera();

    /* Exit if there isn't a tone or prompt playing */
    if (!theKymera->chain_tone_handle && !theKymera->prompt_source)
        return;

    DEBUG_LOGF("appKymeraTonePromptStop, state %u", appKymeraGetState());

    switch (appKymeraGetState())
    {
        case KYMERA_STATE_SCO_ACTIVE:
        case KYMERA_STATE_SCO_SLAVE_ACTIVE:
        case KYMERA_STATE_SCO_ACTIVE_WITH_FORWARDING:
        case KYMERA_STATE_A2DP_STREAMING:
        case KYMERA_STATE_A2DP_STREAMING_WITH_FORWARDING:
        case KYMERA_STATE_TONE_PLAYING:
        {
            Operator op = ChainGetOperatorByRole(theKymera->chainu.output_vol_handle, OPR_VOLUME_CONTROL);
            uint16 volume = volTo60thDbGain(0);
            OperatorsVolumeSetAuxGain(op, volume);

            if (theKymera->prompt_source)
            {
                appKymeraPromptSourceClose(theKymera->prompt_source);
                theKymera->prompt_source = NULL;
            }

            if (theKymera->chain_tone_handle)
            {
                ChainStop(theKymera->chain_tone_handle);
                ChainDestroy(theKymera->chain_tone_handle);
                theKymera->chain_tone_handle = NULL;
            }

            if (appKymeraGetState() != KYMERA_STATE_TONE_PLAYING)
            {
                /* Return to low power mode (if applicable) */
                appKymeraConfigureDspPowerMode(FALSE);
            }
            else
            {
                OperatorsVolumeSetMainGain(op, volume);
                OperatorsVolumeMute(op, TRUE);

                /* Disable external amplifier if required */
                appKymeraExternalAmpControl(FALSE);
                ChainStop(theKymera->chainu.output_vol_handle);
                ChainDestroy(theKymera->chainu.output_vol_handle);
                theKymera->chainu.output_vol_handle = NULL;

                /* Move back to idle state */
                appKymeraSetState(KYMERA_STATE_IDLE);
                theKymera->output_rate = 0;
            }
        }
        break;

        case KYMERA_STATE_IDLE:
        default:
            /* Unknown state / not supported */
            DEBUG_LOGF("appKymeraTonePromptStop, unsupported state %u", appKymeraGetState());
            Panic();
            break;
    }

    appKymeraClearToneLock(theKymera);

    PanicZero(theKymera->tone_count);
    theKymera->tone_count--;

    /* Tone now stopped, clear the client's lock */
    if (theKymera->tone_client_lock)
    {
        *theKymera->tone_client_lock &= ~theKymera->tone_client_lock_mask;
        theKymera->tone_client_lock = 0;
        theKymera->tone_client_lock_mask = 0;
    }
}

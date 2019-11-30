/*!
\copyright  Copyright (c) 2017-2018  Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Kymera SCO
*/

#include <vmal.h>
#include <packetiser_helper.h>

#include "av_headset_kymera_private.h"

#define AEC_RX_BUFFER_SIZE_MS 15

/*! AEC REF message ID */
#define OPMSG_AEC_REFERENCE_ID_SAME_INPUT_OUTPUT_CLK_SOURCE 0x0008

/*! \brief Message AECREF operator that the back-end of the operator are coming
    from same clock source. This is for optimisation purpose and it's recommended
    to be enabled for use cases where speaker input and microphone output are
    synchronised (e.g. SCO and USB voice use cases). Note: Send/Resend this message
    when all microphone input/output and REFERENCE output are disconnected.
    \param aec_op The AEC Reference operator.
*/
static void appKymeraSetAecSameIOClockSource(Operator aec_op)
{
    uint16 msg[2];
    msg[0] = OPMSG_AEC_REFERENCE_ID_SAME_INPUT_OUTPUT_CLK_SOURCE;
    msg[1] = 1;
    PanicFalse(OperatorMessage(aec_op, msg, 2, NULL, 0));
}

static void appKymeraScoForwardingSetSwitchedPassthrough(switched_passthrough_states state)
{
    kymera_chain_handle_t sco_chain = appKymeraGetScoChain();

    Operator spc_op = (Operator)PanicZero(ChainGetOperatorByRole(sco_chain,
                                                                 OPR_SWITCHED_PASSTHROUGH_CONSUMER));
    appKymeraConfigureSpcMode(spc_op, state);
}

static void appKymeraMicForwardingSetSwitchedPassthrough(switched_passthrough_states state)
{
    kymera_chain_handle_t sco_chain = appKymeraGetScoChain();
    Operator spc_op = (Operator)PanicZero(ChainGetOperatorByRole(sco_chain,
                                                                 OPR_MICFWD_RECV_SPC));
    appKymeraConfigureSpcMode(spc_op, state);
}

void appKymeraScoForwardingPause(bool pause)
{
    switch (appKymeraGetState())
    {
        case KYMERA_STATE_SCO_ACTIVE_WITH_FORWARDING:
        case KYMERA_STATE_SCO_SLAVE_ACTIVE:
        {
            kymeraTaskData *theKymera = appGetKymera();
            if (theKymera->sco_info->mic_fwd)
            {
                DEBUG_LOGF("appKymeraScoForwardingPause, pause %u", pause);
                appKymeraScoForwardingSetSwitchedPassthrough(pause ? CONSUMER_MODE : PASSTHROUGH_MODE);
            }
            else
            {
                DEBUG_LOGF("appKymeraScoForwardingPause, MIC forwarding not enabled");                
            }
        }        
        break;
        
        default:
        {
            DEBUG_LOGF("appKymeraScoForwardingPause, pause %u, incorrect state %u",
                       pause, appKymeraGetState());
        }
        break;        
    }
}

void appKymeraHandleInternalScoForwardingStartTx(Sink forwarding_sink)
{
    UNUSED(forwarding_sink);
    kymeraTaskData *theKymera = appGetKymera();

    /* Ignore request if not in SCO active state, or chain doesn't support SCO forwarding */
    if ((appKymeraGetState() != KYMERA_STATE_SCO_ACTIVE) ||
        !theKymera->sco_info->sco_fwd)
    {
        DEBUG_LOGF("appKymeraHandleInternalScoStartForwardingTx, failed, state %d, sco_fwd %u",
                   appKymeraGetState(), theKymera->sco_info->sco_fwd);
        return;
    }
    
    kymera_chain_handle_t sco_chain = appKymeraGetScoChain();   
    Source scofwd_ep_src = PanicNull(ChainGetOutput(sco_chain, EPR_SCOFWD_TX_OTA));
        
    DEBUG_LOGF("appKymeraHandleInternalScoStartForwardingTx, sink %p, source %p, state %d",
               forwarding_sink, scofwd_ep_src, appKymeraGetState());

    PanicNotZero(theKymera->lock);

    /* Tell SCO forwarding what the source of SCO frames is and enable the
     * passthrough to give it the SCO frames. */
    appScoFwdInitScoPacketising(scofwd_ep_src);
    appKymeraScoForwardingSetSwitchedPassthrough(PASSTHROUGH_MODE);

    /* Setup microphone forwarding if chain support it */
    if (theKymera->sco_info->mic_fwd)
    {
        /* Tell SCO forwarding what the forwarded microphone data sink is */
        Sink micfwd_ep_snk = PanicNull(ChainGetInput(sco_chain, EPR_MICFWD_RX_OTA));
        appScoFwdNotifyIncomingMicSink(micfwd_ep_snk);

        /* Setup the SPC to use the currently selected microphone
         * Will inform the peer to enable/disable MIC fowrading once the
         * peer MIC path has been setup.
        */
        appKymeraSelectSpcSwitchInput(appKymeraGetMicSwitch(), theKymera->mic);

        /* Put the microphone receive switched passthrough consumer into passthrough mode */
        appKymeraMicForwardingSetSwitchedPassthrough(PASSTHROUGH_MODE);
    }

    /* All done, so update kymera state */
    appKymeraSetState(KYMERA_STATE_SCO_ACTIVE_WITH_FORWARDING);
}



bool appKymeraHandleInternalScoForwardingStopTx(void)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOGF("appKymeraHandleInternalScoStopForwardingTx, state %u", appKymeraGetState());

    /* Can't stop forwarding if it hasn't been started */
    if (appKymeraGetState() != KYMERA_STATE_SCO_ACTIVE_WITH_FORWARDING)
        return FALSE;

    if (theKymera->sco_info->sco_fwd)
    {   
        /* Put switched passthrough consumer into consume mode, so that no SCO frames are sent*/
        appKymeraScoForwardingSetSwitchedPassthrough(CONSUMER_MODE);
    
        /* Put the microphone receive switched passthrough consumer into consume mode */
        if (theKymera->sco_info->mic_fwd)
            appKymeraMicForwardingSetSwitchedPassthrough(CONSUMER_MODE);
    
        /* All done, so update kymera state and return indicating success */
        appKymeraSetState(KYMERA_STATE_SCO_ACTIVE);
    }

    /* Clean up local SCO forwarding master forwarding state */
    appScoFwdClearForwarding();
    if (appConfigMicForwardingEnabled())
        appScoFwdClearForwardingReceive();

    return TRUE;
}

void appKymeraHandleInternalScoSlaveStart(Source link_src, const appKymeraScoChainInfo *info,
                                          uint16 volume)
{
    kymeraTaskData *theKymera = appGetKymera();

    DEBUG_LOGF("appKymeraHandleInternalScoSlaveStart, source 0x%x, rate %u, state %u",
               link_src, info->rate, appKymeraGetState());

    PanicNotZero(theKymera->lock);

    if (appKymeraGetState() == KYMERA_STATE_TONE_PLAYING)
    {
        /* If there is a tone still playing at this point,
         * it must be an interruptible tone, so cut it off */
        appKymeraTonePromptStop();
    }
    
    /* Can't start voice chain if we're not idle */
    PanicFalse(appKymeraGetState() == KYMERA_STATE_IDLE);

    /* SCO chain must be destroyed if we get here */
    PanicNotNull(appKymeraGetScoChain());
    
    /* Move to SCO active state now, what ever happens we end up in this state
      (even if it's temporary) */
    appKymeraSetState(KYMERA_STATE_SCO_SLAVE_ACTIVE);

    /* Create appropriate SCO chain */
    appKymeraScoCreateChain(info);
    kymera_chain_handle_t sco_chain = PanicNull(appKymeraGetScoChain());

    /* Get microphone sources */
    Source mic_src_1a;
    Source mic_src_1b;
    appKymeraMicSetup(appConfigScoMic1(), &mic_src_1a, appConfigScoMic2(), &mic_src_1b, theKymera->sco_info->rate);

    /* Get speaker sink */
    Sink spk_snk = StreamAudioSink(appConfigLeftAudioHardware(), appConfigLeftAudioInstance(), appConfigLeftAudioChannel());
    SinkConfigure(spk_snk, STREAM_CODEC_OUTPUT_RATE, theKymera->sco_info->rate);

    /* Get sources and sinks for chain endpoints */
    Source spk_ep_src  = ChainGetOutput(sco_chain, EPR_SCO_SPEAKER);
    Source sco_ep_src  = ChainGetOutput(sco_chain, EPR_MICFWD_TX_OTA);
    Sink sco_ep_snk    = ChainGetInput(sco_chain, EPR_SCOFWD_RX_OTA);
    Sink mic_1a_ep_snk = ChainGetInput(sco_chain, EPR_SCO_MIC1);
    Sink mic_1b_ep_snk = (appConfigScoMic2() != NO_MIC) ? ChainGetInput(sco_chain, EPR_SCO_MIC2) : 0;

    appScoFwdNotifyIncomingSink(sco_ep_snk);
    if (theKymera->sco_info->mic_fwd)
        appScoFwdInitMicPacketising(sco_ep_src);

    /* Set AEC REF sample rate */
    Operator aec_op = ChainGetOperatorByRole(sco_chain, OPR_SCO_AEC);
    OperatorsAecSetSampleRate(aec_op, theKymera->sco_info->rate, theKymera->sco_info->rate);
    OperatorsStandardSetTimeToPlayLatency(aec_op, SFWD_TTP_DELAY_US);
    appKymeraSetAecSameIOClockSource(aec_op);

    /* Set AEC REF input terminal buffer size */
    appKymeraSetTerminalBufferSize(aec_op, theKymera->sco_info->rate, AEC_RX_BUFFER_SIZE_MS, 1 << 0, 0);

    /* Set async WBS decoder bitpool and buffer size */
    Operator awbs_op = ChainGetOperatorByRole(sco_chain, OPR_SCOFWD_RECV);
    OperatorsAwbsSetBitpoolValue(awbs_op, SFWD_MSBC_BITPOOL, TRUE);
    OperatorsStandardSetBufferSizeWithFormat(awbs_op, SFWD_RECV_CHAIN_BUFFER_SIZE, operator_data_format_pcm);

    Operator spc_op = ChainGetOperatorByRole(sco_chain, OPR_SWITCHED_PASSTHROUGH_CONSUMER);
    if (spc_op)
        appKymeraConfigureScoSpcDataFormat(spc_op, ADF_GENERIC_ENCODED);
    
    /* Set async WBS encoder bitpool if microphone forwarding is enabled */
    if (theKymera->sco_info->mic_fwd)
    {
        Operator micwbs_op = ChainGetOperatorByRole(sco_chain, OPR_MICFWD_SEND);
        OperatorsAwbsSetBitpoolValue(micwbs_op, SFWD_MSBC_BITPOOL, FALSE);
    }

    /*! \todo Before updating from Products, this was not muting */
    appKymeraConfigureOutputChainOperators(sco_chain, theKymera->sco_info->rate, KICK_PERIOD_VOICE, 0, 0);
    appKymeraSetOperatorUcids(TRUE, SCO_WB);

    /* Connect microphones to chain microphone endpoints */
    StreamConnect(mic_src_1a, mic_1a_ep_snk);
    if (appConfigScoMic2() != NO_MIC)
        StreamConnect(mic_src_1b, mic_1b_ep_snk);

    /* Connect chain speaker endpoint to speaker */
    StreamConnect(spk_ep_src, spk_snk);

    /* Connect chain */
    ChainConnect(sco_chain);

    /* Enable external amplifier if required */
    appKymeraExternalAmpControl(TRUE);

    /* Start chain */
    if (ChainStartAttempt(sco_chain))
    {
        theKymera->output_rate = theKymera->sco_info->rate;
        appKymeraHandleInternalScoSetVolume(volume);

        if (theKymera->sco_info->mic_fwd)
        {
            /* Default to not forwarding the MIC. The primary will tell us when to start/stop using
               SFWD_OTA_MSG_MICFWD_START and SFWD_OTA_MSG_MICFWD_STOP mesages */

            DEBUG_LOG("appKymeraHandleInternalScoSlaveStart, not forwarding MIC data");
            appKymeraScoForwardingSetSwitchedPassthrough(CONSUMER_MODE);
        }        
    }
    else
    {
        DEBUG_LOG("appKymeraHandleInternalScoSlaveStart, could not start chain");
        appKymeraHandleInternalScoSlaveStop();
    }
}

void appKymeraHandleInternalScoSlaveStop(void)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOGF("appKymeraHandleInternalScoSlaveStop, state %u", appKymeraGetState());

    PanicNotZero(theKymera->lock);

    /* Should be in SCO slave active state */
    PanicFalse(appKymeraGetState() == KYMERA_STATE_SCO_SLAVE_ACTIVE);

    /* Get current SCO chain */
    kymera_chain_handle_t sco_chain = PanicNull(appKymeraGetScoChain());

    Sink mic_1a_ep_snk = ChainGetInput(sco_chain, EPR_SCO_MIC1);
    Sink mic_1b_ep_snk = (appConfigScoMic2() != NO_MIC) ? ChainGetInput(sco_chain, EPR_SCO_MIC2) : 0;
    Source spk_ep_src  = ChainGetOutput(sco_chain, EPR_SCO_SPEAKER);

    /* A tone still playing at this point must be interruptable */
    appKymeraTonePromptStop();

    /* Stop chains */
    ChainStop(sco_chain);

    /* Disconnect microphones from chain microphone endpoints */
    StreamDisconnect(NULL, mic_1a_ep_snk);
    if (appConfigScoMic2() != NO_MIC)
        StreamDisconnect(NULL, mic_1b_ep_snk);

    /* Disconnect chain speaker endpoint to speaker */
    StreamDisconnect(spk_ep_src, NULL);

    /* Close microphone sources */
    appKymeraMicCleanup(appConfigScoMic1(), appConfigScoMic2());

    /* Destroy chains */
    ChainDestroy(sco_chain);
    theKymera->chainu.sco_handle = sco_chain = NULL;

    /* Disable external amplifier if required */
    if (!appKymeraAncIsEnabled())
        appKymeraExternalAmpControl(FALSE);

    /* Update state variables */
    appKymeraSetState(KYMERA_STATE_IDLE);
    theKymera->output_rate = 0;
}



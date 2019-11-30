/*!
\copyright  Copyright (c) 2017-2018  Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Kymera ANC code
*/

#include <audio_clock.h>
#include <audio_power.h>
#include <vmal.h>
#include <file.h>
#include <cap_id_prim.h>
#include <opmsg_prim.h>

#include "av_headset_kymera_private.h"


#define ANC_TUNING_SINK_USB_LEFT      0 /*can be any other backend device. PCM used in this tuning graph*/
#define ANC_TUNING_SINK_USB_RIGHT     1
#define ANC_TUNING_SINK_FBMON_LEFT    2 /*reserve slots for FBMON tap. Always connected.*/
#define ANC_TUNING_SINK_FBMON_RIGHT   3
#define ANC_TUNING_SINK_MIC1_LEFT     4 /* must be connected to internal ADC. Analog or digital */
#define ANC_TUNING_SINK_MIC1_RIGHT    5
#define ANC_TUNING_SINK_MIC2_LEFT     6
#define ANC_TUNING_SINK_MIC2_RIGHT    7

#define ANC_TUNING_SOURCE_USB_LEFT    0 /*can be any other backend device. USB used in the tuning graph*/
#define ANC_TUNING_SOURCE_USB_RIGHT   1
#define ANC_TUNING_SOURCE_DAC_LEFT    2 /* must be connected to internal DAC */
#define ANC_TUNING_SOURCE_DAC_RIGHT   3


/*! Macro for creating messages */
#define MAKE_KYMERA_MESSAGE(TYPE) \
    TYPE##_T *message = PanicUnlessNew(TYPE##_T);

void appKymeraAncInit(void)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOG("appKymeraAncInit");

    theKymera->anc_state = KYMERA_ANC_UNINITIALISED;
    if (appConfigAncPathEnable() != all_disabled)
    {
        memset(&theKymera->anc_mic_params, 0, sizeof(anc_mic_params_t));
        theKymera->anc_mic_params.mic_gain_step_size = appConfigAncMicGainStepSize();

        if (appConfigAncPathEnable() & feed_forward_left)
        {
            PanicFalse(appConfigAncFeedForwardMic() != NO_MIC);
            theKymera->anc_mic_params.enabled_mics |= feed_forward_left;
            theKymera->anc_mic_params.feed_forward_left = theKymera->mic_params[appConfigAncFeedForwardMic()];
        }
        if (appConfigAncPathEnable() & feed_back_left)
        {
            PanicFalse(appConfigAncFeedBackMic() != NO_MIC);
            theKymera->anc_mic_params.enabled_mics |= feed_back_left;
            theKymera->anc_mic_params.feed_back_left = theKymera->mic_params[appConfigAncFeedBackMic()];
        }

        if (AncInit(&theKymera->anc_mic_params, appConfigAncMode(), appConfigAncSidetoneGain()))
            theKymera->anc_state = KYMERA_ANC_OFF;
    }
}

void appKymeraAncEnable(void)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOGF("appKymeraAncEnable, state %u", theKymera->anc_state);

    if (theKymera->anc_state == KYMERA_ANC_OFF)
    {
        if (AncEnable(TRUE))
        {
            theKymera->anc_state = KYMERA_ANC_ON;
            appKymeraExternalAmpControl(TRUE);
        }
    }
}

void appKymeraAncDisable(void)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOGF("appKymeraAncDisable, state %u", theKymera->anc_state);

    if (theKymera->anc_state == KYMERA_ANC_ON)
    {
        if (AncEnable(FALSE))
        {
            theKymera->anc_state = KYMERA_ANC_OFF;
            appKymeraExternalAmpControl(FALSE);
        }
    }
}

bool appKymeraAncIsEnabled(void)
{
    kymeraTaskData *theKymera = appGetKymera();
    return (theKymera->anc_state == KYMERA_ANC_ON);
}

void appKymeraAncTuningStart(uint16 usb_rate)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOG("appKymeraAncTuningStart");
    MAKE_KYMERA_MESSAGE(KYMERA_INTERNAL_ANC_TUNING_START);
    message->usb_rate = usb_rate;
    MessageSendConditionally(&theKymera->task, KYMERA_INTERNAL_ANC_TUNING_START, message, &theKymera->busy_lock);
}


void appKymeraAncTuningStop(void)
{
    kymeraTaskData *theKymera = appGetKymera();
    DEBUG_LOG("appKymeraAncTuningStop");
    MessageSend(&theKymera->task, KYMERA_INTERNAL_ANC_TUNING_STOP, NULL);
}

static void appKymeraAncGetMics(uint8 *mic0, uint8 *mic1)
{
    *mic0 = NO_MIC;
    *mic1 = NO_MIC;
    
    if (appConfigAncPathEnable() & feed_forward_left)
    {
        *mic0 = appConfigAncFeedForwardMic();
        if (appConfigAncPathEnable() & feed_back_left)
            *mic1 = appConfigAncFeedBackMic();
    }
    else
    {
        if (appConfigAncPathEnable() & feed_back_left)
            *mic0 = appConfigAncFeedBackMic();
    }
}

void appKymeraAncTuningCreateChain(uint16 usb_rate)
{
    kymeraTaskData *theKymera = appGetKymera();
    theKymera->usb_rate = usb_rate;

    const char anc_tuning_edkcs[] = "download_anc_tuning.edkcs";
    DEBUG_LOG("appKymeraAncTuningCreateChain, rate %u", usb_rate);

    /* Only 48KHz supported */
    PanicFalse(usb_rate == 48000);

    /* Turn on audio subsystem */
    OperatorFrameworkEnable(1);

    /* Move to ANC tuning state, this prevents A2DP and HFP from using kymera */
    appKymeraSetState(KYMERA_STATE_ANC_TUNING);

    /* Create ANC tuning operator */
    FILE_INDEX index = FileFind(FILE_ROOT, anc_tuning_edkcs, strlen(anc_tuning_edkcs));
    PanicFalse(index != FILE_NONE);
    theKymera->anc_tuning_bundle_id = PanicZero (OperatorBundleLoad(index, 0)); /* 0 is processor ID */
    theKymera->anc_tuning = (Operator)PanicFalse(VmalOperatorCreate(CAP_ID_DOWNLOAD_ANC_TUNING));

    /* Create the operators for USB Rx & Tx audio */
    uint16 usb_config[] =
    {
        OPMSG_USB_AUDIO_ID_SET_CONNECTION_CONFIG,
        0,              // data_format
        usb_rate / 25,  // sample_rate
        2,              // number_of_channels
        16,             // subframe_size
        16,             // subframe_resolution
    };

/* On Aura 2.1 usb rx, tx capabilities are downloaded */
#ifdef DOWNLOAD_USB_AUDIO
    const char usb_audio_edkcs[] = "download_usb_audio.edkcs";
    index = FileFind(FILE_ROOT, usb_audio_edkcs, strlen(usb_audio_edkcs));
    PanicFalse(index != FILE_NONE);
    theKymera->usb_audio_bundle_id = PanicZero (OperatorBundleLoad(index, 0)); /* 0 is processor ID */
#endif

    theKymera->usb_rx = (Operator)PanicFalse(VmalOperatorCreate(EB_CAP_ID_USB_AUDIO_RX));
    PanicFalse(VmalOperatorMessage(theKymera->usb_rx,
                                   usb_config, SIZEOF_OPERATOR_MESSAGE(usb_config),
                                   NULL, 0));

    theKymera->usb_tx = (Operator)PanicFalse(VmalOperatorCreate(EB_CAP_ID_USB_AUDIO_TX));
    PanicFalse(VmalOperatorMessage(theKymera->usb_tx,
                                   usb_config, SIZEOF_OPERATOR_MESSAGE(usb_config),
                                   NULL, 0));

    /* Get the DAC output sinks */
    Sink DAC_L = (Sink)PanicFalse(StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, AUDIO_CHANNEL_A));
    PanicFalse(SinkConfigure(DAC_L, STREAM_CODEC_OUTPUT_RATE, usb_rate));

    /* Get the ANC microphone sources */
    Source mic_in0, mic_in1;
    uint8 mic0, mic1;
    appKymeraAncGetMics(&mic0, &mic1);
    appKymeraMicSetup(mic0, &mic_in0, mic1, &mic_in1, usb_rate);

    /* Get the ANC tuning monitor microphone sources */
    Source fb_mon0;
    appKymeraMicSetup(appConfigAncTuningMonitorMic(), &fb_mon0, NO_MIC, NULL, usb_rate);
    PanicFalse(SourceSynchronise(mic_in0, fb_mon0));


    uint16 anc_tuning_frontend_config[3] =
    {
        OPMSG_ANC_TUNING_ID_FRONTEND_CONFIG,        // ID
        0,                                          // 0 = mono, 1 = stereo
        mic_in1 ? 1 : 0                             // 0 = 1-mic, 1 = 2-mic
    };
    PanicFalse(VmalOperatorMessage(theKymera->anc_tuning,
                                   &anc_tuning_frontend_config, SIZEOF_OPERATOR_MESSAGE(anc_tuning_frontend_config),
                                   NULL, 0));

    /* Connect microphone */
    PanicFalse(StreamConnect(mic_in0,
                             StreamSinkFromOperatorTerminal(theKymera->anc_tuning, ANC_TUNING_SINK_MIC1_LEFT)));
    if (mic_in1)
        PanicFalse(StreamConnect(mic_in1,
                                 StreamSinkFromOperatorTerminal(theKymera->anc_tuning, ANC_TUNING_SINK_MIC2_LEFT)));

    /* Connect FBMON microphone */
    PanicFalse(StreamConnect(fb_mon0,
                             StreamSinkFromOperatorTerminal(theKymera->anc_tuning, ANC_TUNING_SINK_FBMON_LEFT)));

    /* Connect speaker */
    PanicFalse(StreamConnect(StreamSourceFromOperatorTerminal(theKymera->anc_tuning, ANC_TUNING_SOURCE_DAC_LEFT),
                             DAC_L));

    /* Connect backend (USB) out */
    PanicFalse(StreamConnect(StreamSourceFromOperatorTerminal(theKymera->anc_tuning, ANC_TUNING_SOURCE_USB_LEFT),
                             StreamSinkFromOperatorTerminal(theKymera->usb_tx, 0)));
    PanicFalse(StreamConnect(StreamSourceFromOperatorTerminal(theKymera->anc_tuning, ANC_TUNING_SOURCE_USB_RIGHT),
                             StreamSinkFromOperatorTerminal(theKymera->usb_tx, 1)));

    /* Connect backend (USB) in */
    PanicFalse(StreamConnect(StreamSourceFromOperatorTerminal(theKymera->usb_rx, 0),
                             StreamSinkFromOperatorTerminal(theKymera->anc_tuning, ANC_TUNING_SINK_USB_LEFT)));
    PanicFalse(StreamConnect(StreamSourceFromOperatorTerminal(theKymera->usb_rx, 1),
                             StreamSinkFromOperatorTerminal(theKymera->anc_tuning, ANC_TUNING_SINK_USB_RIGHT)));

    /* Connect USB ISO in endpoint to USB Rx operator */
    PanicFalse(StreamConnect(StreamUsbEndPointSource(end_point_iso_in),
                             StreamSinkFromOperatorTerminal(theKymera->usb_rx, 0)));

    /* Connect USB Tx operator to USB ISO out endpoint */
    PanicFalse(StreamConnect(StreamSourceFromOperatorTerminal(theKymera->usb_tx, 0),
                             StreamUsbEndPointSink(end_point_iso_out)));

    /* Finally start the operators */
    Operator op_list[] = {theKymera->usb_rx, theKymera->anc_tuning, theKymera->usb_tx};
    PanicFalse(OperatorStartMultiple(3, op_list, NULL));

    /* Ensure audio amp is on */
    appKymeraExternalAmpControl(TRUE);

    /* Set kymera lock to prevent anything else using kymera */
    theKymera->lock = TRUE;
}

void appKymeraAncTuningDestroyChain(void)
{
    kymeraTaskData *theKymera = appGetKymera();

    /* Turn audio amp is off */
    appKymeraExternalAmpControl(FALSE);

    /* Stop the operators */
    Operator op_list[] = {theKymera->usb_rx, theKymera->anc_tuning, theKymera->usb_tx};
    PanicFalse(OperatorStopMultiple(3, op_list, NULL));

    /* Disconnect USB ISO in endpoint */
    StreamDisconnect(StreamUsbEndPointSource(end_point_iso_in), 0);

    /* Disconnect USB ISO out endpoint */
    StreamDisconnect(0, StreamUsbEndPointSink(end_point_iso_out));

    /* Get the DAC output sinks */
    Sink DAC_L = (Sink)PanicFalse(StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, AUDIO_CHANNEL_A));

    /* Get the ANC microphone sources */
    Source mic_in0, mic_in1;
    uint8 mic0, mic1;
    appKymeraAncGetMics(&mic0, &mic1);
    appKymeraMicSetup(mic0, &mic_in0, mic1, &mic_in1, theKymera->usb_rate);
    
    /* Get the ANC tuning monitor microphone sources */
    Source fb_mon0;
    appKymeraMicSetup(appConfigAncTuningMonitorMic(), &fb_mon0, NO_MIC, NULL, theKymera->usb_rate);

    /* Connect microphone */
    StreamDisconnect(mic_in0, 0);
    if (mic_in1)
        StreamDisconnect(mic_in1, 0);

    /* Connect FBMON microphone */
    StreamConnect(fb_mon0, 0);

    /* Disconnect speaker */
    StreamDisconnect(0, DAC_L);

    /* Distroy operators */
    OperatorsDestroy(op_list, 3);

    /* Unload bundle */
    PanicFalse(OperatorBundleUnload(theKymera->anc_tuning_bundle_id));
    #ifdef DOWNLOAD_USB_AUDIO
    PanicFalse(OperatorBundleUnload(theKymera->usb_audio_bundle_id));
    #endif

    /* Clear kymera lock and go back to idle state to allow other uses of kymera */
    theKymera->lock = FALSE;
    appKymeraSetState(KYMERA_STATE_IDLE);

    /* Turn off audio subsystem */
    OperatorFrameworkEnable(0);
}

void appKymeraAncEnableSynchronizeWithPeer(void)
{
    DEBUG_LOG("appKymeraAncEnableSynchronizeWithPeer");
	
    kymeraTaskData *theKymera = appGetKymera();
    peerSyncTaskData* ps = appGetPeerSync();
    ps->peer_anc_enabled = TRUE;

    appPeerSyncSend(FALSE);

    MessageSendLater(&theKymera->task, KYMERA_INTERNAL_ANC_ON,NULL,ANC_PEER_SYNC_TIMEOUT_DELAY);
}


void appKymeraAncDisableSynchronizeWithPeer(void)
{
    DEBUG_LOG("appKymeraAncDisableSynchronizeWithPeer");

    /* Getting and setting initial peer ANC state */
    kymeraTaskData *theKymera = appGetKymera();
    peerSyncTaskData* ps = appGetPeerSync();
    ps->peer_anc_enabled = FALSE;

    appPeerSyncSend(FALSE);

    MessageSendLater(&theKymera->task, KYMERA_INTERNAL_ANC_OFF,NULL,ANC_PEER_SYNC_TIMEOUT_DELAY);
}

void appKymeraAncDelayedEnable(uint16 anc_delay)
{
    DEBUG_LOG("appKymeraAncDelayedEnable");

    kymeraTaskData *theKymera = appGetKymera();

    MessageCancelAll(&theKymera->task,KYMERA_INTERNAL_ANC_ON);
    MessageSendLater(&theKymera->task, KYMERA_INTERNAL_ANC_ON,NULL,anc_delay);
}

void appKymeraAncDelayedDisable(uint16 anc_delay)
{
    DEBUG_LOG("appKymeraAncDelayedDisable");

    kymeraTaskData *theKymera = appGetKymera();

    MessageCancelAll(&theKymera->task,KYMERA_INTERNAL_ANC_OFF);
    MessageSendLater(&theKymera->task, KYMERA_INTERNAL_ANC_OFF,NULL,anc_delay);
}

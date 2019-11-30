/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_control.c

DESCRIPTION
    Code to control voice assistant plugin.
*/



#include <operators.h>
#include <audio_plugin_common.h>
#include <audio_config.h>
#include <vmtypes.h>
#include <print.h>
#include <audio_plugin_if.h>
#include <audio.h>
#include <chain.h>
#include <gain_utils.h>
#include <rtime.h>
#include <audio_hardware.h>
#include <audio_ports.h>
#include <operators.h>
#include <audio_plugin_ucid.h>

#include "audio_voice_assistant_control.h"
#include "audio_voice_assistant_capture_chain.h"
#include "audio_voice_assistant_private.h"
#include "audio_plugin_voice_assistant_variants.h"
#include "audio_voice_assistant_common_chain.h"
#include "audio_voice_assistant_trigger_chain.h"
#include "audio_voice_assistant_handlers.h"

#ifdef VOICE_ASSISTANT_LOW_POWER
#include <vmal.h>
#include <audio/audio_if.h>
#include <audio_clock.h>
#endif

#define MIC_SAMPLE_RATE 16000

#define MIC_PATH_0 0
#define MIC_PATH_1 1
#define MIC_PATH_2 2


#define CVC_MIC_0_INPUT audioVaGetCommonChainMicInput(MIC_PATH_0)
#define CVC_MIC_1_INPUT audioVaGetCommonChainMicInput(MIC_PATH_1)
#define CVC_MIC_2_INPUT audioVaGetCommonChainMicInput(MIC_PATH_2)
#define CVC_REFERENCE_INPUT audioVaGetCommonChainCvcRefInput()


#define AEC_MIC_0_INPUT AudioPortsGetAecMicInput(MIC_PATH_0)
#define AEC_MIC_1_INPUT AudioPortsGetAecMicInput(MIC_PATH_1)
#define AEC_MIC_2_INPUT AudioPortsGetAecMicInput(MIC_PATH_2)

#define AEC_MIC_0_OUTPUT AudioPortsGetAecOutputTerminal(MIC_PATH_0)
#define AEC_MIC_1_OUTPUT AudioPortsGetAecOutputTerminal(MIC_PATH_1)
#define AEC_MIC_2_OUTPUT AudioPortsGetAecOutputTerminal(MIC_PATH_2)
#define AEC_REFERENCE_OUTPUT AudioPortsGetAecReference()

static Operator graph_manager;
const TaskData vaQvaTask = {audioVaQvaMessageHandler};
#define SYSTEM_KEYS_MSG_OP_CLIENT_PROMOTION 0xFF

/****************************************************************************
DESCRIPTION
    Function Sends Data Source to registered application task
*/
static void indicateDataSource(Task app_task, Source src)
{
    PRINT(("VA_Plugin: Indicate Data Source to registred task \n"));

    MAKE_AUDIO_MESSAGE( AUDIO_VA_INDICATE_DATA_SOURCE, message) ;
    message->plugin = (Task)&voice_assistant_plugin ;
    message->data_src = src;
    MessageSend(app_task, AUDIO_VA_INDICATE_DATA_SOURCE, message);
}

/****************************************************************************
DESCRIPTION
    Connect output of capture chain to sink
*/
static void connectCaptureOutput(Task app_task)
{
    Source capture_output = audioVaGetCaptureChainOutput();

    PanicFalse(SourceMapInit(capture_output, STREAM_TIMESTAMPED, AUDIO_FRAME_VA_DATA_LENGTH));

    indicateDataSource(app_task, capture_output);
}

/****************************************************************************
DESCRIPTION
    Disconnect output of capture chain to sink
*/
static void disconnectCaptureOutput(void)
{
    Source capture_output = audioVaGetCaptureChainOutput();

    SourceUnmap(capture_output);

    StreamDisconnect(capture_output, 0);
}

/****************************************************************************
DESCRIPTION
    Get Digital or Analog mic channel for VA microphone identified by the mic_path
*/
static audio_channel getMicChannel(uint8 mic_path, bool is_digital)
{
    audio_channel channel = AUDIO_CHANNEL_A;

    switch(mic_path)
    {
        case MIC_PATH_0:
            channel = is_digital ? AUDIO_CHANNEL_SLOT_0 : AUDIO_CHANNEL_A;
            break;

        case MIC_PATH_1:
            channel = is_digital ? AUDIO_CHANNEL_SLOT_1 : AUDIO_CHANNEL_B;
            break;

        case MIC_PATH_2:
            channel = AUDIO_CHANNEL_SLOT_2;
            break;

        default:
            break;
    }

    return channel;
}

/****************************************************************************
DESCRIPTION
    Get VA microphone source identified by the mic_path which already configured
*/
static Source getVaMicSource(uint8 mic_path, audio_mic_params mic_param)
{
    audio_channel channel = getMicChannel(mic_path, mic_param.is_digital);

    return AudioPluginGetMicSource(mic_param, channel);
}

/****************************************************************************
DESCRIPTION
    Get VA microphone source identified by the mic_path
*/
static void configureVaMic(uint8 mic_path, audio_mic_params mic_param)
{
    audio_channel channel = getMicChannel(mic_path, mic_param.is_digital);

    AudioPluginMicSetup(channel, mic_param, MIC_SAMPLE_RATE);
}

/****************************************************************************
DESCRIPTION
    Connect mic paths to AEC ref
*/
static void connecMicsToAEC(voice_mic_params_t *mic_params)
{
    Source primary_mic = NULL;
    Source secondary_mic_1 = NULL;
    Source secondary_mic_2 = NULL;

    primary_mic = getVaMicSource(MIC_PATH_0, mic_params->mic_a);
    configureVaMic(MIC_PATH_0, mic_params->mic_a);

    if(CVC_MIC_1_INPUT)
    {
        /* When using 2 mic or FFV */
        secondary_mic_1 = getVaMicSource(MIC_PATH_1, mic_params->mic_b);
        configureVaMic(MIC_PATH_1, mic_params->mic_b);
        SourceSynchronise(primary_mic, secondary_mic_1);
    }

    if(CVC_MIC_2_INPUT)
    {
        /* When using FFV */
        secondary_mic_2 = getVaMicSource(MIC_PATH_2, mic_params->mic_c);
        configureVaMic(MIC_PATH_2, mic_params->mic_c);
        SourceSynchronise(primary_mic, secondary_mic_2);
    }
    
    PanicNull(StreamConnect(primary_mic, AEC_MIC_0_INPUT));

    if(CVC_MIC_1_INPUT)
    {
        PanicNull(StreamConnect(secondary_mic_1, AEC_MIC_1_INPUT));
    }

    if(CVC_MIC_2_INPUT)
    {
        PanicNull(StreamConnect(secondary_mic_2, AEC_MIC_2_INPUT));
    }
}

/****************************************************************************
DESCRIPTION
    Connect AEC Outputs to CVC mic interfaces
*/
static void connectAEC(void)
{
    PanicNull(StreamConnect(AEC_MIC_0_OUTPUT, CVC_MIC_0_INPUT));

    if(CVC_MIC_1_INPUT)
    {
        /* When using 2 mic or FFV */
        PanicNull(StreamConnect(AEC_MIC_1_OUTPUT, CVC_MIC_1_INPUT));
    }

    if(CVC_MIC_2_INPUT)
    {
        /* When using FFV */
        PanicNull(StreamConnect(AEC_MIC_2_OUTPUT, CVC_MIC_2_INPUT));
    }
    
    PanicNull(StreamConnect(AEC_REFERENCE_OUTPUT, CVC_REFERENCE_INPUT));
}

/****************************************************************************
DESCRIPTION
    Disconnect mics paths to AEC ref
*/
static void disconnectAEC(voice_mic_params_t *mic_params)
{
    StreamDisconnect(NULL, AEC_MIC_0_INPUT);
    StreamDisconnect(NULL, AEC_MIC_1_INPUT);
    StreamDisconnect(NULL, AEC_MIC_2_INPUT);

    AudioPluginMicShutdown(AUDIO_CHANNEL_A, &mic_params->mic_a, TRUE);

    if(CVC_MIC_1_INPUT)
    {
        /* When using 2 mic or FFV */
        StreamDisconnect(AEC_MIC_1_OUTPUT, NULL);
        AudioPluginMicShutdown(AUDIO_CHANNEL_B, &mic_params->mic_b, TRUE);
    }

    if(CVC_MIC_2_INPUT)
    {
        /* When using FFV */
        StreamDisconnect(AEC_MIC_2_OUTPUT, NULL);
        AudioPluginMicShutdown(AUDIO_CHANNEL_SLOT_2, &mic_params->mic_c, TRUE);
    }
    StreamDisconnect(AEC_REFERENCE_OUTPUT, NULL);

    StreamDisconnect(AEC_MIC_0_OUTPUT, NULL);

}

/****************************************************************************
DESCRIPTION
    Function to Create common chain with CVC and AEC ref
*/
static void createCommonChain(va_plugin_context_t *va_context,bool cvc_omni_mode)
{
    audioVaCreateCommonChain(audioVaGetContext()->cvc,cvc_omni_mode);
    AudioHardwareSetMicUse(audio_hw_voice_assistant);
    AudioHardwareConnectInput(MIC_SAMPLE_RATE);
    connectAEC();
    connecMicsToAEC(va_context->mic_params);
}

/****************************************************************************
DESCRIPTION
    Function to stop common chain 
*/
static void destroyCommonChain(va_plugin_context_t *va_context)
{
    /*TODO: Need to check any other chains are alive or not, 
     * If then, need to gracefully destroy them  before destroying common chain
     */
    disconnectAEC(va_context->mic_params);   
    audioVaDestroyCommonChain();
    AudioHardwareDisconnectInput();
}

static void connectCommonChainAndCaptureChain(void)
{
    /* Connect spc output to encoder input*/
    PanicNull(StreamConnect(audioVaGetCommonChainCaptureOutput(0), audioVaGetCaptureChainInput(0)));

    if(audioVaIsFFVEnabled())
    {
        PanicNull(StreamConnect(audioVaGetCommonChainCaptureOutput(1), audioVaGetCaptureChainInput(1)));
        PanicNull(StreamConnect(audioVaGetCommonChainCaptureOutput(2), audioVaGetCaptureChainInput(2)));
        PanicNull(StreamConnect(audioVaGetCommonChainCaptureOutput(3), audioVaGetCaptureChainInput(3)));
    }
}

static void disconnectCommonChainCaptureOutputs(void)
{
    StreamDisconnect(audioVaGetCommonChainCaptureOutput(0), NULL);

    if(audioVaIsFFVEnabled())
    {
        StreamDisconnect(audioVaGetCommonChainCaptureOutput(1), NULL);
        StreamDisconnect(audioVaGetCommonChainCaptureOutput(2), NULL);
        StreamDisconnect(audioVaGetCommonChainCaptureOutput(3), NULL);
    }
}

static void startCaptureAfterVoiceTrigger(va_plugin_context_t *va_context, uint32 start_timestamp)
{
    if(va_context)
    {        
        audioVaCreateCaptureChain();

        connectCommonChainAndCaptureChain();

        connectCaptureOutput(va_context->app_task);
        audioVaStartCaptureChain();
        
        /* If trigger_detected flag is reset only if startTrigger is called, after a successful trigger detection.
            In other words, if this flag is set then trigger chain is not active (as its already triggered) */
        audioVaConfigCommonChainForStartCapture(start_timestamp, (va_context->trigger_detected ? FALSE: TRUE));
    }
}

static void stopCaptureForVoiceTrigger(va_plugin_context_t *va_context)
{
    UNUSED(va_context);

    audioVaStopCaptureChain();
    
    audioVaConfigCommonChainForStopCapture();
    disconnectCommonChainCaptureOutputs();  

    disconnectCaptureOutput();
    audioVaDestroyCaptureChain();

    audioVaStopCommonChain();

    PRINT(("VA_Plugin: Destroy VA audio chain completed\n"));
}

/****************************************************************************
DESCRIPTION
    Function to Create VA operator graph/chain for voice capture
*/
static void startCaptureAfterButtonTrigger(va_plugin_context_t *va_context)
{
    if(va_context)
    {
        PRINT(("VA_Plugin: Create VA audio chain \n"));

        OperatorsFrameworkEnable();
        OperatorsFrameworkSetKickPeriod(DEFAULT_KICK_PERIOD);
        audioVaCreateCaptureChain();
        
        /* Check voice trigger supported, if not, 
         * then we need enable common chain here 
         */
        if((audioVaIsFFVEnabled()&& audioVaIsCommonChainCreated()))
                createCommonChain(va_context,va_context->cvc_omni_mode);
        else
                createCommonChain(va_context,FALSE);

        /* Common Chain Conifguration is done, connect the chain */
        audioVaConnectCommonChain();

        connectCommonChainAndCaptureChain();

        connectCaptureOutput(va_context->app_task);
        audioVaStartCaptureChain();
        
        audioVaConfigCommonChainForStartCapture(0, FALSE);
        audioVaStartCommonChain();

        PRINT(("VA_Plugin: Va operator chain creation completed \n"));

    }
}

/****************************************************************************
DESCRIPTION
    Function to Stop/Destroy VA graph/chain for voice capture
*/
static void stopCaptureForButtonTrigger(va_plugin_context_t *va_context)
{
    PRINT(("VA_Plugin: Destroy VA audio chain \n"));
    UNUSED(va_context);

    audioVaStopCaptureChain();
    audioVaStopCommonChain();
    
    
    audioVaConfigCommonChainForStopCapture();
    disconnectCommonChainCaptureOutputs();  

    disconnectCaptureOutput();
    audioVaDestroyCaptureChain();

    destroyCommonChain(va_context);

    OperatorsFrameworkDisable();

    PRINT(("VA_Plugin: Destroy VA audio chain completed\n"));
}

static void connectCommonChainAndTriggerChain(void)
{
    /* Connect splitter buffer output(s) to qva input(s)*/
    PanicNull(StreamConnect(audioVaGetCommonChainTriggerOutput(0), audioVaGetTriggerChainInput(0)));

    if(audioVaIsFFVEnabled())
    {
        PanicNull(StreamConnect(audioVaGetCommonChainTriggerOutput(1), audioVaGetTriggerChainInput(1)));
        PanicNull(StreamConnect(audioVaGetCommonChainTriggerOutput(2), audioVaGetTriggerChainInput(2)));
        PanicNull(StreamConnect(audioVaGetCommonChainTriggerOutput(3), audioVaGetTriggerChainInput(3)));
    }
}

/****************************************************************************
DESCRIPTION
    Function to create graph manager standalone operator  
*/
static void createGraphManagerOperator(void)
{
    if(!graph_manager)
    {
        graph_manager = CustomOperatorCreate(capability_id_graph_manager, OPERATOR_PROCESSOR_ID_0, operator_priority_lowest, NULL);
        MessageOperatorTask(graph_manager, (Task)&vaQvaTask);
    }
}

/****************************************************************************
DESCRIPTION
    Function to promote/de-promote GM to handle VAD QVA & CVC
*/
static void promoteGraphManager(bool promote)
{
    Operator op_list[4];
    Operator delegate_op = (promote ? graph_manager : 0);
    
    op_list[0] = audioVaGetCommonChainOperator(va_common_vad_role);
    op_list[1] = audioVaGetTriggerChainQvaOperator();
    op_list[2] = audioVaGetCommonChainOperator(va_common_cvc_role);
    op_list[3] = audioVaGetCommonChainOperator(va_common_splitter_role);
    
    PanicFalse(OperatorDelegateMultiple(delegate_op, 4, op_list));
}

/****************************************************************************
DESCRIPTION
    Function Destroy graph manager operator
*/
static void destroyGraphManagerOperator(void)
{
    if(graph_manager)
    {
        CustomOperatorDestroy(&graph_manager,1);
        graph_manager = 0;
        PRINT(("VA_Plugin: destroyGraphManagerOperator(): Graph Manager destroyed \n")); 
    }
}


#ifdef VOICE_ASSISTANT_LOW_POWER
/****************************************************************************
DESCRIPTION
    Function to preserve the operators and source in GM
*/
static void preserveReleaseOperators(voice_mic_params_t *mic_params, bool preserve)
{
    Source src_array[3];
    Operator op_array[1];
    uint8 mic_cnt = 0;
    Source primary_mic = NULL;
    Source secondary_mic_1 = NULL;
    Source secondary_mic_2 = NULL;

    op_array[0] = AudioHardwareGetOperator();

    primary_mic = getVaMicSource(MIC_PATH_0, mic_params->mic_a);
    src_array[mic_cnt++] = primary_mic;
    if(CVC_MIC_1_INPUT)
    {
        /* When using 2 mic or FFV */
        secondary_mic_1 = getVaMicSource(MIC_PATH_1, mic_params->mic_b);
        src_array[mic_cnt++] = secondary_mic_1;
    }
    if(CVC_MIC_2_INPUT)
    {
        /* When using FFV */
        secondary_mic_2 = getVaMicSource(MIC_PATH_2, mic_params->mic_c);
        src_array[mic_cnt++] = secondary_mic_2;
    }
    /* preserve the operators and src in GM. No need to preserve the sink */
    if(preserve)
        PanicFalse(OperatorFrameworkPreserve(1, op_array, mic_cnt, src_array, 0, NULL));
    else
        PanicFalse(OperatorFrameworkRelease(1, op_array, mic_cnt, src_array, 0, NULL));
}

static void setDSPClockForLowPower(void)
{
    audio_dsp_clock_configuration myconfig;
    
    myconfig.active_mode=AUDIO_DSP_CLOCK_NO_CHANGE;
    myconfig.trigger_mode=AUDIO_DSP_CLOCK_NO_CHANGE;
    myconfig.low_power_mode=AUDIO_DSP_SLOW_CLOCK;
    AudioDspClockConfigure(&myconfig);
}

/****************************************************************************
DESCRIPTION
    Function to enter LP mode
*/
static void enterLowPowerMode(va_plugin_context_t *va_context)
{
    /* Enable notification on GM */
    PanicFalse(OperatorFrameworkTriggerNotificationStart(3, graph_manager));
    preserveReleaseOperators(va_context->mic_params, TRUE);
    setDSPClockForLowPower();
    /* This will negate OperatorFrameworkEnable() called by trigger chain creation */
    OperatorsFrameworkDisable();
    /* This will negate OperatorFrameworkEnable() called by common chain creation */
    OperatorsFrameworkDisable();
    /* This will negate OperatorFrameworkEnable() called by AEC chain creation */
    OperatorsFrameworkDisable();
}

/****************************************************************************
DESCRIPTION
    Function to exit LP mode
*/
static void exitLowPowerMode(va_plugin_context_t *va_context)
{
    /* This will counter OperatorsFrameworkDisable() called for common chain */
    OperatorsFrameworkEnable();
    /* This will counter OperatorsFrameworkDisable() called for AEC chain */
    OperatorsFrameworkEnable();
    /* This will counter OperatorsFrameworkDisable() called for trigger chain */
    OperatorsFrameworkEnable();
    OperatorsFrameworkSetKickPeriod(DEFAULT_KICK_PERIOD);
    
    PanicFalse(OperatorFrameworkTriggerNotificationStop());
    /* release the operators and sources */
    preserveReleaseOperators(va_context->mic_params, FALSE);
}
#endif /* #ifndef VOICE_ASSISTANT_LOW_POWER */
/****************************************************************************
DESCRIPTION
    Function to Create VA operator graph/chain for voice trigger
*/
static void startTrigger(va_plugin_context_t *va_context,FILE_INDEX trigger_file )
{
    PRINT(("VA_Plugin: VaCreateVoiceTriggerChain() Create and Start Trigger Chain \n"));
    if((FILE_NONE != trigger_file)&&(va_context))
    {
        if(audioVaCreateTriggerChain(trigger_file))
            connectCommonChainAndTriggerChain();
        
        OperatorsFrameworkSetKickPeriod(DEFAULT_KICK_PERIOD);
        createGraphManagerOperator();

        audioVaConfigCommonChainForStartTrigger();

        audioVaStartTriggerChain();
        audioVaStartCommonChain();

        promoteGraphManager(TRUE);

#ifdef VOICE_ASSISTANT_LOW_POWER
        enterLowPowerMode(va_context);
#endif /* VOICE_ASSISTANT_LOW_POWER */
     }
}

/****************************************************************************
DESCRIPTION
    Function to Stop/Destroy VA graph/chain for voice trigger
*/
static void stopTrigger(va_plugin_context_t *va_context)
{
    PRINT(("VA_Plugin: Stop VA trigger chain \n"));

    if(va_context)
    {
        audioVaStopTriggerChain();
        PRINT(("VA_Plugin: Destroy VA audio triger chain completed\n"));
    }
}

static void disconnectCommonChainAndTriggerChain(void)
{
    /* Disconnect splitter buffer output(s) from qva input(s)*/
    StreamDisconnect(audioVaGetCommonChainTriggerOutput(0), audioVaGetTriggerChainInput(0));
	
    if(audioVaIsFFVEnabled())
    {
        StreamDisconnect(audioVaGetCommonChainTriggerOutput(1), audioVaGetTriggerChainInput(1));
        StreamDisconnect(audioVaGetCommonChainTriggerOutput(2), audioVaGetTriggerChainInput(2));
        StreamDisconnect(audioVaGetCommonChainTriggerOutput(3), audioVaGetTriggerChainInput(3));
    }
}

/****************************************************************************
DESCRIPTION
    Function to Puase the trigger until there is a stop trigger
*/
static void pauseTrigger(va_plugin_context_t *va_context)
{
    UNUSED(va_context);
    
#ifdef VOICE_ASSISTANT_LOW_POWER
   exitLowPowerMode(va_context);
#endif /* VOICE_ASSISTANT_LOW_POWER */
    promoteGraphManager(FALSE);
    audioVaConfigCommonChainForStopTrigger(); 
}

/****************************************************************************
DESCRIPTION
    Starts voice capture. Will also instantiate common chain in voice capture
    only senario
*/
void audioVaStartCapture(va_plugin_context_t *va_context,bool use_timestamp,uint32 start_timestamp)
{
    UNUSED(use_timestamp);
    if(va_context->support_voice_trigger)
        startCaptureAfterVoiceTrigger(va_context, start_timestamp);
    else
        startCaptureAfterButtonTrigger(va_context);
}

/******************************************************************************
DESCRIPTION
    Stop voice capture
*/
void audioVaStopCapture(va_plugin_context_t *va_context)
{
    if(va_context->support_voice_trigger)
        stopCaptureForVoiceTrigger(va_context);
    else
        stopCaptureForButtonTrigger(va_context);
}

/******************************************************************************
DESCRIPTION
    Start Trigger detection
*/
void audioVaStartTrigger(va_plugin_context_t *va_context, AUDIO_PLUGIN_START_TRIGGER_DETECT_MSG_T *msg)
{
    startTrigger(va_context, msg->trigger_phrase_data_file);
    va_context->trigger_detected = FALSE;
}

/******************************************************************************
DESCRIPTION
    Stop Trigger detection
*/
void audioVaStopTrigger(va_plugin_context_t *va_context)
{
    if(!va_context->trigger_detected)
        pauseTrigger(va_context);
    stopTrigger(va_context);
}

/******************************************************************************
DESCRIPTION
    Pause Trigger detection until stop trigger
*/
void audioVaPauseTrigger(va_plugin_context_t *va_context)
{
    pauseTrigger(va_context);
    va_context->trigger_detected = TRUE;
}

/******************************************************************************
DESCRIPTION
    Connect Audio and Common Chain
*/
void audioVaStart(va_plugin_context_t *va_context)
{
    /* Enable the buffer/cvc chain */
   createCommonChain(va_context,FALSE);
    /* Common Chain Conifguration is done, connect the chain */
    audioVaConnectCommonChain();
}

/******************************************************************************
DESCRIPTION
    Disconnect Audio and Common Chain
*/
void audioVaStop(va_plugin_context_t *va_context)
{
    /* To Do: Need to have a re-look for low power mode */
    destroyGraphManagerOperator();
    disconnectCommonChainAndTriggerChain();
    audioVaDestroyTriggerChain();

    audioVaStopCommonChain();
    destroyCommonChain(va_context);
}


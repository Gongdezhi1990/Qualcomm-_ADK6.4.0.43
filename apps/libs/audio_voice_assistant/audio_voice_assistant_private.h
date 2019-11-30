/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_private.h

DESCRIPTION
    Voice Assistant Private header file contains defines and default values
*/

#include <stdlib.h>
#include <vmtypes.h>
#include <print.h>
#include <stdio.h>
#include <chain.h>
#include <audio_plugin_voice_variants.h>
#include <audio_plugin_if.h>

#ifndef AUDIO_VOICE_ASSISTANT_PRIVATE_H_
#define AUDIO_VOICE_ASSISTANT_PRIVATE_H_

/* Voice Assistant Context structure */
typedef struct
{
    Task               app_task;
    voice_mic_params_t *mic_params;
    bool               support_voice_trigger;
    plugin_variant_t     cvc;
    bool                  cvc_omni_mode;
    bool 		     trigger_detected;
}va_plugin_context_t;

typedef struct
{
    uint16 trigger_confidence;
    /* Start Timestamp of trigger phrase,
     * upper half then lower half. */
    uint16 MSB_start_ttp;
    uint16 LSB_start_ttp;
    /* End Timestamp of trigger phrase,
     * upper half then lower half. */
    uint16 MSB_end_ttp;
    uint16 LSB_end_ttp;
}channel_trigger_details_t;


/* trigger detect msg from QVA */
typedef struct
{
    uint16 number_of_channels;
    channel_trigger_details_t ch0;
    channel_trigger_details_t ch1;
    channel_trigger_details_t ch2;
    channel_trigger_details_t ch3;
}va_trigger_message_t;

/*-------------------  Defines -------------------*/

#define AUDIO_FRAME_VA_DATA_LENGTH 9
#define VA_AUDIO_SAMPLE_RATE       16000
#define VA_CHAIN_INPUT_BUFFER_SIZE 1204

#define INITIAL_GAIN_SCALED_DB     GainIn60thdB(-30)
#define VA_LATENCY_IN_MS           (20)
#define VA_LATENCY_IN_US           (VA_LATENCY_IN_MS * US_PER_MS)


/****************************************************************************
DESCRIPTION
    Function to Create Voice Assistant Plugin Context
*/
void audioVaCreateContext(Task app_task, voice_mic_params_t *mic, bool support_voice_trigger,
                          plugin_variant_t cvc, bool omni_mode);

/****************************************************************************
DESCRIPTION
    Function to returns Voice Assistant Plugin Context
*/
va_plugin_context_t * audioVaGetContext(void);

/****************************************************************************
DESCRIPTION
    Function to Clean up Voice Assistant Plugin Context
*/
void audioVaCleanUpContext(void);

/****************************************************************************
DESCRIPTION
    Check context is enabled for Far Filed Voice or not.
*/
bool audioVaIsFFVEnabled(void);


#endif /* AUDIO_VOICE_ASSISTANT_PRIVATE_H_ */


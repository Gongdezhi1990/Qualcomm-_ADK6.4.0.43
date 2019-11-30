/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_capture_chain.h

DESCRIPTION
    Handles creation and control of the capture chain of the
    VA graph
*/

#ifndef AUDIO_VOICE_ASSISTANT_CAPTURE_CHAIN_H_
#define AUDIO_VOICE_ASSISTANT_CAPTURE_CHAIN_H_

#include <chain.h>

typedef enum
{
    va_common_switched_passthrough_role,
    va_capture_encoder_role,
} va_capture_operator_role_t;

typedef enum
{
    va_capture_input_base = 0,
    va_capture_input_0    = 0,
    va_capture_input_1    = 1,
    va_capture_input_2    = 2,
    va_capture_input_3    = 3,
    va_capture_output     = 4
}va_capture_path_role_t;

/****************************************************************************
DESCRIPTION
    Creates and connects the capture chain
*/
void audioVaCreateCaptureChain(void);

/****************************************************************************
DESCRIPTION
    Starts the capture chain
*/
void audioVaStartCaptureChain(void);

/****************************************************************************
DESCRIPTION
    Get input to the capture chain
*/
Sink audioVaGetCaptureChainInput(uint16 input_num);

/****************************************************************************
DESCRIPTION
    Get output to capture chain
*/
Source audioVaGetCaptureChainOutput(void);

/****************************************************************************
DESCRIPTION
    Stop the capture chain
*/
void audioVaStopCaptureChain(void);

/****************************************************************************
DESCRIPTION
    Destroy the capture chain
*/
void audioVaDestroyCaptureChain(void);

/****************************************************************************
DESCRIPTION
    Select capture chain Switched passthrough input
    Range: 0 to 3
*/
void audioVaSelectCaptureChainSPCInput(spc_select_passthrough_input_t input);

#endif /* AUDIO_VOICE_ASSISTANT_CAPTURE_CHAIN_H_ */

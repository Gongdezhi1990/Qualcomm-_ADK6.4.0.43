/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant_trigger_chain.h

DESCRIPTION
    Voice assistant plugin implementation to handle triggere phase enable/disable
    and QVA Indication. This file manages Splitter->QVA chain of VA
*/

#ifndef AUDIO_VOICE_ASSISTANT_TRIGGER_CHAIN_H_
#define AUDIO_VOICE_ASSISTANT_TRIGGER_CHAIN_H_

#include <message.h>
#include <audio_plugin_if.h>

#include "audio_voice_assistant_private.h"


typedef enum
{
    va_qva_role
} va_trigger_operator_role_t;

typedef enum
{
    va_trigger_path_base = 0,
    va_trigger1_path     = 0,
    va_trigger2_path     = 1,
    va_trigger3_path     = 2,
    va_trigger4_path     = 3
} va_trigger_path_role_t;


/****************************************************************************
DESCRIPTION
    Creates and connects the trigger chain
*/
bool audioVaCreateTriggerChain(FILE_INDEX trigger_file);

/****************************************************************************
DESCRIPTION
    Destroy the trigger chain
*/
void audioVaDestroyTriggerChain(void);

/****************************************************************************
DESCRIPTION
    Starts the trigger chain and Enable notification from QVA 
*/
void audioVaStartTriggerChain(void);

/****************************************************************************
DESCRIPTION
    Stops the trigger chain
*/
void audioVaStopTriggerChain(void);

/****************************************************************************
DESCRIPTION
    Get the input/sink of trigger chain based on trigger input number
    (0-VA_TRIGGER1/ 1-VA_TRIGGER2/ 2-VA_TRIGGER3/ 3-VA_TRIGGER4)
*/
Sink audioVaGetTriggerChainInput(uint16 input_num);

/****************************************************************************
DESCRIPTION
    Get QVA operator
*/
Operator audioVaGetTriggerChainQvaOperator(void);

#endif /* AUDIO_VOICE_ASSISTANT_TRIGGER_CHAIN_H_ */


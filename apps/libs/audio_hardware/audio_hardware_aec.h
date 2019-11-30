/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_hardware_aec.h

DESCRIPTION
       Declarations for AEC chain management
*/

#ifndef _AUDIO_HARDWARE_AEC_H_
#define _AUDIO_HARDWARE_AEC_H_

#include "audio_hardware.h"
#include <chain.h>


/****************************************************************************
NAME
    hardwareAecConfigureMicSampleRate
    
DESCRIPTION
    Configure AEC sample rates and update UCID to match    
*/
void hardwareAecConfigureMicSampleRate(uint32 mic_sample_rate);

/****************************************************************************
NAME
    hardwareIsAecRequired

DESCRIPTION
    Check if the AEC block is required - This block is required for HFP and concurrent audio
*/
bool hardwareIsAecRequired(connection_type_t connection_type);

/****************************************************************************
NAME
    hardwareAecCreateAndConnectInput

DESCRIPTION
    Creates and starts the AEC chain if necessary, connects the AEC input path
*/
void hardwareAecCreateAndConnectInput(const uint32 sample_rate);


/****************************************************************************
NAME
    hardwareAecCreateAndConnectOutput

DESCRIPTION
    Creates and starts the AEC chain if necessary, connects the AEC output path
*/
void hardwareAecCreateAndConnectOutput(const audio_hardware_connect_t* hw_connect_data);

/****************************************************************************
NAME
    hardwareAecDisconnectInput

DESCRIPTION
    Disconnect the AEC input path and destroy the chain if necessary
*/
void hardwareAecDisconnectInput(void);

/****************************************************************************
NAME
    hardwareAecDisconnectOutput

DESCRIPTION
    Disconnect the AEC output path and destroy the chain if necessary
*/
void hardwareAecDisconnectOutput(void);

/****************************************************************************
NAME
    hardwareAecMuteMicOutput

DESCRIPTION
    Mutes microphone output
*/
void hardwareAecMuteMicOutput(bool enable);

/****************************************************************************
NAME
    hardwareAecGetOperator

DESCRIPTION
    Returns the AEC operator
*/
Operator hardwareAecGetOperator(void);

/****************************************************************************
NAME
    hardwareAecSetMicUse

DESCRIPTION
    Sets the user of Microphone
*/
void hardwareAecSetMicUse(audio_hw_mic_use_t mic_use);

#endif /* _AUDIO_HARDWARE_AEC_H_ */

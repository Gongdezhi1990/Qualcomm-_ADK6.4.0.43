/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_hardware_output.h
DESCRIPTION
    Defines the interconnections between the various audio hardware chains.
NOTES
*/

#ifndef _AUDIO_HARDWARE_OUTPUT_H_
#define _AUDIO_HARDWARE_OUTPUT_H_

#include <chain.h>
#include <audio_output.h>
#include "audio_hardware.h"

#define MAP_AUDIO_OUTPUT(src, output_type)\
        if (src)\
        {\
            AudioOutputAddSource(src, output_type);\
        }
        
#define AUDIO_CONNECT_STREAMS(src, sink)\
        if (src && sink)\
        {\
            StreamConnect(src, sink);\
        }
    

/*******************************************************************************
NAME
    hardwareMapAecToAudioOutput
DESCRIPTION
    Connect the AEC chain to the audio_output library.
*/
bool hardwareMapAecToAudioOutput(kymera_chain_handle_t chain);

/*******************************************************************************
NAME
    hardwareMapSourcesToAudioOutput
DESCRIPTION
    Map input sources to audio output depending on the output type
*/
void hardwareMapSourcesToAudioOutput(const audio_hardware_connect_t* hw_connect_data);

/*******************************************************************************
NAME
    hardwareConnectAecInputs
DESCRIPTION
    Connect the audio hardware inputs and AEC sources
*/
void hardwareConnectAecInputs(const audio_hardware_connect_t* hw_connect_data, kymera_chain_handle_t dest_chain);

/****************************************************************************
NAME
    hardwareConnectToAudioOutput
DESCRIPTION
    Helper function to connect to audio output
*/
void hardwareConnectToAudioOutput(uint32 output_sample_rate);


/****************************************************************************
NAME
    hardwareConnectDirectToOutput
DESCRIPTION
    Helper function to map and connect to direct audio output
    If no AEC, connect mixer chain directly to audio output
*/
void hardwareConnectDirectToOutput(const audio_hardware_connect_t* hw_connect_data);


#endif /* _AUDIO_HARDWARE_OUTPUT_H_ */

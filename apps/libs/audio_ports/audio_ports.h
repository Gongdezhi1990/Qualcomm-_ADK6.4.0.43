/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_ports.h
 
DESCRIPTION
    Library to implement an external interface to audio library ports.
    It provides getters and setters for various external ports as required by different
    parts of the audio sub-system

*/

#ifndef _AUDIO_PORTS_H_
#define _AUDIO_PORTS_H_

#include <source.h>

#define MAX_NUMBER_OF_MICS 4

/****************************************************************************
DESCRIPTION
    Function to set AEC Reference Terminal
*/
void AudioPortsSetAecReference(Source source);

/****************************************************************************
DESCRIPTION
    Function to get AEC Reference Terminal
*/
Source AudioPortsGetAecReference(void);

/****************************************************************************
DESCRIPTION
        Function to set a specific AEC Output terminal
*/
void AudioPortsSetAecOutputTerminal(Source source, unsigned terminal_number);

/****************************************************************************
DESCRIPTION
        Function to get a specific AEC Output terminal
*/
Source AudioPortsGetAecOutputTerminal(unsigned terminal_number);

/****************************************************************************
DESCRIPTION
        Function to set a specific AEC Mic input terminal
*/
void AudioPortsSetAecMicInput(Sink sink, unsigned terminal_number);

/****************************************************************************
DESCRIPTION
        Function to get a specific AEC Mic input terminal
*/
Sink AudioPortsGetAecMicInput(unsigned terminal_number);

/****************************************************************************
DESCRIPTION
        Function to clear all locally registered audio ports
*/
void AudioPortsClearAudioPorts(void);


/****************************************************************************
DESCRIPTION
        Function to set output terminal of Switched Passthrough module. 
        Note: Also caller can reset the output to NULL after the use with this same API. 
*/
void AudioPortsSetSwitchedPassthruOutput(Source source);

/****************************************************************************
DESCRIPTION
        Function to get the output terminal of Switched Passthrough module
*/
Source AudioPortsGetSwitchedPassthruOutput(void);


#endif /* _AUDIO_PORTS_H_ */

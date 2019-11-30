/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_ports.c
DESCRIPTION
    Library to implement an external interface to audio library ports.
    It provides getters and setters for various external ports as required by different
    parts of the audio sub-system
NOTES
*/

#include <string.h>

#include <source.h>
#include <panic.h>

#include "audio_ports.h"

typedef struct
{
    Source aec_reference;
    Source aec_output_terminal[MAX_NUMBER_OF_MICS];
    Sink aec_mic_terminal[MAX_NUMBER_OF_MICS];
} audio_ports_t;

static audio_ports_t audio_ports = {0};


typedef struct
{
    Source switched_passthru;
} audio_port_switched_passthru_t;

static audio_port_switched_passthru_t audio_switched_passthru_port = {0};


void AudioPortsSetAecReference(Source source)
{
    audio_ports.aec_reference = source;
}

Source AudioPortsGetAecReference(void)
{
    return audio_ports.aec_reference;
}

void AudioPortsSetAecOutputTerminal(Source source, unsigned terminal_number)
{
    if (terminal_number < MAX_NUMBER_OF_MICS)
        audio_ports.aec_output_terminal[terminal_number] = source;
}

Source AudioPortsGetAecOutputTerminal(unsigned terminal_number)
{
    Source source = (Source) NULL;
    if (terminal_number < MAX_NUMBER_OF_MICS)
        source = audio_ports.aec_output_terminal[terminal_number];

    return source;
}

void AudioPortsSetAecMicInput(Sink sink, unsigned terminal_number)
{
    if (terminal_number < MAX_NUMBER_OF_MICS)
        audio_ports.aec_mic_terminal[terminal_number] = sink;
}

Sink AudioPortsGetAecMicInput(unsigned terminal_number)
{
    Sink sink = (Sink) NULL;
    if (terminal_number < MAX_NUMBER_OF_MICS)
        sink = audio_ports.aec_mic_terminal[terminal_number];

    return sink;
}

void AudioPortsClearAudioPorts(void)
{
    unsigned i;

    audio_ports.aec_reference = (Source)NULL;

    for (i = 0; i < MAX_NUMBER_OF_MICS; i++)
    {
        audio_ports.aec_output_terminal[i] = (Source)NULL;
        audio_ports.aec_mic_terminal[i] = (Sink)NULL;
    }
}

void AudioPortsSetSwitchedPassthruOutput(Source source)
{
    audio_switched_passthru_port.switched_passthru  = source;
}

Source AudioPortsGetSwitchedPassthruOutput(void)
{
    return audio_switched_passthru_port.switched_passthru;
}



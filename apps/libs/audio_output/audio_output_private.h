/******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_output_private.h
 
DESCRIPTION
    Private data/functions for audio_output lib
*/

#ifndef __AUDIO_OUTPUT_PRIVATE_H__
#define __AUDIO_OUTPUT_PRIVATE_H__

#include "audio_output.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Convenience Function Macros */
#define forEachOutput(audio_out) for(audio_out=0; audio_out<audio_output_max; audio_out++)

/*============================================================================*
 *  Private Data Types
 *============================================================================*/

/*!
    @brief Audio Output State Variables
    
    Used internally by the plugin to maintain a record of its current state.
*/
typedef struct __audio_output_state
{
    unsigned connected:1;           /*! Whether connection has take place */
    unsigned i2s_devices_active:1;  /*! Whether any I2S devices are active */
    audio_output_params_t params;   /*! Latest params passed on connection */
    Source* sources;                /*! Store sources prior to connection */
} audio_output_state_t;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
 
extern const audio_output_config_t* config;

/******************************************************************************
NAME:
    audioOutputGetSink

DESCRIPTION:
    Gets the sink for a specified output channel, if a mapping exists.

PARAMETERS:
    audio_out     Output to get the sink for.
    
RETURNS:
    The sink, or NULL if there was an error.
*/
Sink audioOutputGetSink(audio_output_t audio_out);

/******************************************************************************
NAME:
    audioOutputPcmInitHardware

DESCRIPTION:
    Performs internal hardware initialisation for PCM, such as configuring the
    required PIOs, based on the outputs enabled in the audio output
    configuration. Note that external I2S devices e.g. I2S amplifiers are
    initialised separately, on a per connection basis.

PARAMETERS:
    config      Audio output configuration data.

RETURNS:
    Whether the hardware was initialised. TRUE = success, FALSE = error.
*/
bool audioOutputPcmInitHardware(const audio_output_config_t* config);

/******************************************************************************
NAME:
    audioOutputCheckUseCase

DESCRIPTION:
    Performs use-case actions on output to a digital device becoming active or inactive.

PARAMETERS:
    active      Whether the output to the digital device becoming active (TRUE) or inactive
    type        The type of digital device becoming active (TRUE) or inactive

RETURNS:
    Nothing.
*/
void audioOutputCheckUseCase(bool active, audio_output_hardware_type_t type);

#endif /*__AUDIO_OUTPUT_PRIVATE_H__*/

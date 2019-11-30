/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    header file which defines all of the tones which can be used in the
    sink device application
*/

#ifndef SINK_TONES_H
#define SINK_TONES_H

#include <audio_output.h>
#include "sink_events.h"

/****************************************************************************
NAME
   sinkTonesInit

DESCRIPTION
   Function to initialize sink tones.

RETURNS
   TRUE or FALSE

*/
bool sinkTonesInit(void);
/****************************************************************************
NAME
   sinkTonesCanQueueVolumeTones

DESCRIPTION
   Function to get volume tones.

RETURNS
   TRUE or FALSE

*/
bool sinkTonesCanQueueVolumeTones(void);
/****************************************************************************
NAME
   sinkTonesCanQueueEventTones

DESCRIPTION
   Function to get event tones.

RETURNS
   TRUE or FALSE

*/
bool sinkTonesCanQueueEventTones(void);

/****************************************************************************
NAME
   sinkTonesIsPlayLocalVolumeTone

DESCRIPTION
   Function to get PlayLocalVolumeTone
RETURNS
   TRUE or FALSE

*/
bool sinkTonesIsPlayLocalVolumeTone(void);
/****************************************************************************
NAME
   sinkTonesGetFixedToneVolumeLevel

DESCRIPTION
   Function to get fixed tone volume level.

RETURNS
   uint16
*/
uint16 sinkTonesGetFixedToneVolumeLevel(void);

/****************************************************************************
DESCRIPTION
    Store tone volume settings for use by audio plugins
*/
void TonesInitSettings(bool fixed_volume, unsigned fixed_volume_level);

/****************************************************************************
NAME
   sinkTonesSetFixedToneVolumeLevel

DESCRIPTION
   Function to set fixed tone volume level.

PARAMS
    volume_level Fixed volume level to set for tones.

RETURNS
   TRUE if new fixed volume was set ok, FALSE otherwise.
*/
bool sinkTonesSetFixedToneVolumeLevel(uint16 volume_level);

/****************************************************************************
NAME
   sinkTonesIsPlayTonesAtFixedVolume

DESCRIPTION
   Function to get PlayTonesAtFixedVolume
RETURNS
   TRUE or FALSE

*/
bool sinkTonesIsPlayTonesAtFixedVolume(void);


/****************************************************************************
NAME
    TonesGetToneVolume

DESCRIPTION
    Works out the correct volume to play tones or Audio Prompts at

RETURNS
    void
*/
uint16 TonesGetToneVolume(void);

/****************************************************************************
NAME
    TonesPlayTone

DESCRIPTION
    Playsback the tone given by the heasdsetTone_t index

RETURNS
    void
*/
void TonesPlayTone ( uint16 pTone , bool pCanQueue ) ;

/****************************************************************************
NAME
    ToneTerminate

DESCRIPTION
    function to terminate a ring tone prematurely.

RETURNS

*/
void ToneTerminate ( void ) ;

/****************************************************************************
NAME
    TonesGetToneVolumeDb

DESCRIPTION
    Works out the correct volume to play tones or Audio Prompts at in dB/60

RETURNS
    Volume in dB/60
*/
int16 TonesGetToneVolumeInDb(audio_output_group_t group);


/****************************************************************************
NAME    
    IsToneConfiguredForEvent
    
DESCRIPTION
    Checks if audio tone is configured for the given sink event.
 
PARAMETERS
    sink_event    - incoming sink event

RETURNS
    TRUE if audio tone is configured for the sink event
    FALSE if audio tone is not configured for the sink event
*/
bool IsToneConfiguredForEvent(sinkEvents_t sink_event);

/****************************************************************************
NAME    
    AudioTonePlayEvent
    
DESCRIPTION
    Plyas audio tone attached to the incoming sink event.
 
PARAMETERS
    sink_event           - incoming sink event
    event_can_be_queued  - Flag which indicate if this event
    					   is to be played imidiately or not

RETURNS
    void
*/
void AudioTonePlayEvent(sinkEvents_t sink_event, bool event_can_be_queued);

typedef struct
{
	sinkEvents_t event; /* Event that triggers the tone. */
    uint8 tone;         /* Index of pre-defined tone to play. */
} tone_config_type;

/****************************************************************************
NAME
    SinkTonesGetToneConfig

DESCRIPTION
    Get the pre-defined tone played for a given sink event.

PARAMS
    event        Event to get the tone config for.
    tone_config  Pointer to where tone_config_type will be placed.

RETURNS
    bool: TRUE if success else FALSE
*/
bool SinkTonesGetToneConfig(uint16 event, tone_config_type *tone_config);

/******************************************************************************
NAME
    SinkTonesSetToneConfig

DESCRIPTION
    Add or update the tone config for an event.

PARAMS
    tone_config  Tone config data for an event.

RETURNS
    bool TRUE if success else FALSE
*/
bool SinkTonesSetToneConfig(const tone_config_type *tone_config);

/********************************************************************************
NAME
    SinkTonesGetUserTone

DESCRIPTION
    Get a pointer to the user-defined tone at the given index.

PARAMS
    id [in] The id of the user-defined tone to get.
    tone [out] The pointer to the user-defined tone, if it is found.
    length [out] The number of elements in the user-defined tone array.

RETURNS
    bool TRUE if the tone was found, FALSE otherwise.
*/
bool SinkTonesGetUserTone(uint16 id, const ringtone_note **tone, uint16 *length);

/******************************************************************************
NAME
    SinkTonesSetUserTone

DESCRIPTION
    Set or update a user-defined tone at the given index.

PARAMS
    id [in] The id of the user-defined tone to set.
    tone [in] Pointer to the user-defined tone data array.
    length [in] The number of elements in the user-defined tone array.

RETURNS
    bool TRUE if the tone was set ok, FALSE otherwise.
*/
bool SinkTonesSetUserTone(uint16 id, const ringtone_note *tone, uint16 length);

/******************************************************************************
NAME
    SinkTonesUserToneNumMax

DESCRIPTION
    Get the maximum number of user-defined tones supported by this module.

PARAMS
    None.

RETURNS
    uint16 The maximum number of user-defined tones.
*/
uint16 SinkTonesUserToneNumMax(void);

#endif


/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_instance.h

DESCRIPTION
    Audio instance library
*/

#ifndef AUDIO_INSTANCE_H_
#define AUDIO_INSTANCE_H_

#include <sink.h>
#include <message.h>
#include <audio_plugin_if.h>

/*!
    @brief audio_instance_t opaque data structure
*/
typedef struct audio_instance_tag * audio_instance_t;

/*!
    @brief Creates an audio instance using the given task and sink

    @param task The Task to be associated with the returned instance
    @param sink The Sink to be associated with the returned instance
    @param sink_type The AUDIO_SINK_T type to be associated with the returned instance


    @return audio_instance_t the created audio instance.
*/
audio_instance_t AudioInstanceCreate(Task task, Sink sink, AUDIO_SINK_T sink_type);

/*!
    @brief Destroys an audio instance

    @param audio_instance_t the audio instance to be destroyed
*/
void AudioInstanceDestroy(audio_instance_t instance);

/*!
    @brief Returns the Sink associated with a given audio instance

    @param audio_instance_t an audio instance

    @return Sink associated with the audio instance
*/
Sink AudioInstanceGetSink(audio_instance_t instance);

/*!
    @brief Returns the AUDIO_SINK_T associated with a given audio instance

    @param audio_instance_t an audio instance

    @return AUDIO_SINK_T associated with the audio instance
*/
AUDIO_SINK_T AudioInstanceGetSinkType(audio_instance_t instance);

/*!
    @brief Returns the Task associated with a given audio instance

    @param audio_instance_t an audio instance

    @return Task associated with the audio instance
*/
Task AudioInstanceGetPluginTask(audio_instance_t instance);

/*!
    @brief Returns the audio_instance associated with a given task

    @param task belonging to an audio instance

    @return audio instance containing task
*/
audio_instance_t AudioInstanceGetInstanceFromTask(Task task);


#ifdef HOSTED_TEST_ENVIRONMENT
/*!
    @brief Resets the audio instance library, for unit test use only
*/
void AudioInstanceTestReset(void);
#endif

#endif

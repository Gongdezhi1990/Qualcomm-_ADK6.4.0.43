/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_latency.h

DESCRIPTION
    Definition of latencies used to set buffer sizes in audio_mixer
*/

#ifndef _AUDIO_MIXER_LATENCY_H_
#define _AUDIO_MIXER_LATENCY_H_

/* Latency in ms we send to source sync for different use cases */
#define MUSIC_LATENCY_MILLISECONDS      (16)
#define VOICE_LATENCY_MILLISECONDS      (14)
#define CONCURRENT_LATENCY_MILLISECONDS (MAX(MUSIC_LATENCY_MILLISECONDS, VOICE_LATENCY_MILLISECONDS))

/* Buffering in concurrent / high quality music graphs is split between source
   sync and pre-processing operators. */
#define CONCURRENT_SOURCE_SYNC_BUFFER_MILLISECONDS      (12)
#define CONCURRENT_PRE_PROCESSING_BUFFER_MILLISECONDS   (CONCURRENT_LATENCY_MILLISECONDS - CONCURRENT_SOURCE_SYNC_BUFFER_MILLISECONDS)

#endif /* _AUDIO_MIXER_LATENCY_H_ */

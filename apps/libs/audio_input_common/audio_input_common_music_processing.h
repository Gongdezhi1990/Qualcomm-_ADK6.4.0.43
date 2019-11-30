/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_common_music_processing.h

DESCRIPTION
	Music processing related handlers
*/

#ifndef AUDIO_INPUT_COMMON_MUSIC_PROCESSING_H_
#define AUDIO_INPUT_COMMON_MUSIC_PROCESSING_H_

/****************************************************************************
DESCRIPTION
    Set music processing default configuration.
*/
void audioInputCommonSetMusicProcessing(audio_input_context_t *ctx, const A2DP_MUSIC_PROCESSING_T music_processing_mode, const uint16 music_mode_enhancements);

#endif /* AUDIO_INPUT_COMMON_MUSIC_PROCESSING_H_ */

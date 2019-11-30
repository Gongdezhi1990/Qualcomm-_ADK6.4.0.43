/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_common_states.h

DESCRIPTION
    State machine API
*/

#ifndef AUDIO_INPUT_COMMON_STATES_H_
#define AUDIO_INPUT_COMMON_STATES_H_

void AudioInputCommonHandleEvent(Task task, audio_input_event_t event, Message payload, audio_input_context_t* ctx);

#endif /* AUDIO_INPUT_COMMON_STATES_H_ */

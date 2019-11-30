/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_common_states.c

DESCRIPTION
    State machine
*/

#include "audio_voice_common_states.h"

#include "audio_voice_common.h"

void AudioVoiceCommonSetStateTable(const audio_voice_state_table_t* table, audio_voice_context_t* ctx)
{
    ctx->state = audio_voice_not_loaded;
    ctx->state_table = table;
}


void AudioVoiceCommonHandleEvent(Task task, audio_voice_event_t event, Message payload, audio_voice_context_t* ctx)
{
    audio_voice_handler_t handler;
    const audio_voice_state_table_t* table = ctx->state_table;
    PanicFalse(table != NULL);

    handler = table->handlers[ctx->state][event].handler;
    PanicFalse(handler != NULL);

    handler(task, payload, ctx);
    ctx->state = table->handlers[ctx->state][event].next_state;
}

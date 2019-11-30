/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_context.c

DESCRIPTION
    Manages the context information of the audio plugin
*/

#include "audio_input_a2dp_context.h"
#include <stdlib.h>


static audio_input_context_t *ctx = NULL;

void audioInputA2dpCreateContextData(void)
{
    PanicNotNull(ctx);
    ctx = PanicNull(calloc(1, sizeof(*ctx)));
}

void audioInputA2dpDeleteContextData(void)
{
    free(ctx);
    ctx = NULL;
}

audio_input_context_t * audioInputA2dpGetContextData(void)
{
    return ctx;
}

kymera_chain_handle_t audioInputA2dpGetChainHandle(void)
{
    if (ctx)
        return ctx->chain;
    else
        return NULL;
}

/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_common_states.c

DESCRIPTION
    State machine
*/

#include "audio_input_common.h"
#include "audio_input_common_states.h"

static void audioInputCommonHandleIdle(audio_input_event_t event, audio_input_context_t* ctx)
{
    switch(event)
    {
        case audio_input_connect_req:
            ctx->state = audio_input_connecting;
            break;
        
        default:
            break;
    }
}

static void audioInputCommonHandleConnecting(audio_input_event_t event, audio_input_context_t* ctx)
{
    switch(event)
    {
        case audio_input_connect_complete:
            ctx->state = audio_input_connected;
            break;
        
        case audio_input_error:
            ctx->state = audio_input_error_state;
            break;
            
        default:
            break;
    }
}

static void audioInputCommonHandleConnected(audio_input_event_t event, audio_input_context_t* ctx)
{
    switch(event)
    {
        case audio_input_disconnect_req:
            ctx->state = audio_input_disconnecting;
            break;
        
        case audio_input_forward_req:
            ctx->state = audio_input_forwarding_setup;
            break;
        
        default:
            break;
    }
}

static void audioInputCommonHandleDisconnecting(audio_input_event_t event, audio_input_context_t* ctx)
{
    switch(event)
    {
        case audio_input_disconnect_complete:
            ctx->state = audio_input_idle;
            break;
        
        case audio_input_forward_destroyed:
            ctx->state = audio_input_idle;
            break;

        default:
            break;
    }
}

static void audioInputCommonHandleForwardingSetup(audio_input_event_t event, audio_input_context_t* ctx)
{
    switch(event)
    {
        case audio_input_forward_started:
            ctx->state = audio_input_forwarding;
            break;
        
        case audio_input_error:
            ctx->state = audio_input_error_state;
            break;
        
        default:
            break;
    }
}

static void audioInputCommonHandleForwarding(audio_input_event_t event, audio_input_context_t* ctx)
{
    switch(event)
    {
        case audio_input_forward_stop_req:
            ctx->state = audio_input_forwarding_tear_down;
            break;

        case audio_input_disconnect_req:
            ctx->state = audio_input_forwarding_disconnect;
            break;
        
        default:
            break;
    }
}

static void audioInputCommonHandleForwardingTearDown(audio_input_event_t event, audio_input_context_t* ctx)
{
    UNUSED(ctx);
    switch(event)
    {
        case audio_input_connect_complete:
            ctx->state = audio_input_connected;
            break;
            
        default:
            break;
    }
}

static void audioInputCommonHandleForwardingDisconnect(audio_input_event_t event, audio_input_context_t* ctx)
{
    UNUSED(ctx);
    switch(event)
    {
        case audio_input_forward_destroyed:
            ctx->state = audio_input_idle;
            break;
            
        default:
            break;
    }
}

static void audioInputCommonHandleErrorState(audio_input_event_t event, audio_input_context_t* ctx)
{
    UNUSED(ctx);
    switch(event)
    {
        case audio_input_disconnect_req:
            ctx->state = audio_input_idle;
            break;
            
        default:
            break;
    }
}

static void audioInputCommonHandleStateTransition(audio_input_event_t event, audio_input_context_t* ctx)
{
    switch(ctx->state)
    {
        case audio_input_idle:
            audioInputCommonHandleIdle(event, ctx);
        break;
        
        case audio_input_connecting:
            audioInputCommonHandleConnecting(event, ctx);
        break;
        
        case audio_input_connected:
            audioInputCommonHandleConnected(event, ctx);
        break;
        
        case audio_input_disconnecting:
            audioInputCommonHandleDisconnecting(event, ctx);
        break;
        
        case audio_input_forwarding_setup:
            audioInputCommonHandleForwardingSetup(event, ctx);
        break;
        
        case audio_input_forwarding:
            audioInputCommonHandleForwarding(event, ctx);
        break;
        
        case audio_input_forwarding_tear_down:
            audioInputCommonHandleForwardingTearDown(event, ctx);
        break;
        
        case audio_input_forwarding_disconnect:
            audioInputCommonHandleForwardingDisconnect(event, ctx);
        break;
        
        case audio_input_error_state:
            audioInputCommonHandleErrorState(event, ctx);
        break;
        
        default:
        break;
    }
}

void AudioInputCommonSetStateTable(const audio_input_state_table_t* table, audio_input_context_t* ctx)
{
    ctx->state_table = table;
}

void AudioInputCommonHandleEvent(Task task, audio_input_event_t event, Message payload, audio_input_context_t* ctx)
{
    audio_input_handler_t handler;
    const audio_input_state_table_t* table = ctx->state_table;
    PanicFalse(table != NULL);
    
    handler = table->handlers[ctx->state][event];
    PanicFalse(handler != NULL);
    
    handler(task, payload, ctx);
    audioInputCommonHandleStateTransition(event, ctx);
}

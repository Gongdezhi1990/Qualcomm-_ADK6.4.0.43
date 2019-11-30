/******************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_broadcast_chain_connect.h

DESCRIPTION
    Definition of the operator chain for the broadcast audio receiver plugin.
*/


/*!
@file    audio_input_broadcast_chain_connect.h
@brief   Definition of the operator chain for the broadcast audio receiver
         plugin.
*/


#ifndef AUDIO_INPUT_BROADCAST_CHAIN_CONFIG_H_
#define AUDIO_INPUT_BROADCAST_CHAIN_CONFIG_H_

#include <chain.h>

enum
{
    input_buffer_role,
    decoder_role,
    pre_processing_role
};

typedef enum
{
    encoded_audio_input,
    decoded_audio_output_left,
    decoded_audio_output_right
} chain_endpoint_role_t;

kymera_chain_handle_t audioInputBroadcastCreateChain(void);

#endif /* AUDIO_INPUT_BROADCAST_CHAIN_CONFIG_H_ */

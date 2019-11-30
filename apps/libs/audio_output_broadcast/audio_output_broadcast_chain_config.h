/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_output_broadcast_chain_config.h
 
DESCRIPTION
    Kymera chain configuration for audio_output_broadcast.
*/

#ifndef AUDIO_OUTPUT_BROADCAST_CHAIN_CONFIG_H_
#define AUDIO_OUTPUT_BROADCAST_CHAIN_CONFIG_H_

#include <chain.h>

enum
{
    pre_processing_role,
    compander_role,
    peq_role,
    vse_role,
    dbe_role,
    cancel_headroom_role,
    encoder_role,
    splitter_role,
    decoder_role,
    broadcast_buffer_role,
    local_buffer_role
};

typedef enum
{
    left_decoded_audio_input,
    right_decoded_audio_input,
    split_encoded_audio,
    broadcast_encoded_audio_output,
    decode_encoded_audio,
    left_decoded_audio_output,
    right_decoded_audio_ouput
} chain_path_role_t;

#define SPLITTER_BUFFER_SIZE (6000)
#define BROADCAST_BUFFER_SIZE (1024)
#define LOCAL_BUFFER_SIZE (1024)
#define PRE_PROCESSING_BUFFER_SIZE (1600)

kymera_chain_handle_t audioOutputBroadcastCreateChain(void);

#endif /* AUDIO_OUTPUT_BROADCAST_CHAIN_CONFIG_H_ */

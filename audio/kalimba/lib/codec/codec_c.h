/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  codec_c.h
 * \ingroup codec
 *
 * CODEC library C header file. <br>    
 *
 */

#ifndef CODEC_C_H
#define CODEC_C_H

#include "buffer/buffer.h"
#include "codec_constants.h"


/** Structure used by stereo decoders for processing */
typedef struct
{
    /** The encoded input buffer */
    tCbuffer *in_buffer;
    /** The decoded left channel buffer (Terminal 0) */
    tCbuffer *out_left_buffer;
    /** The decoded right channel buffer (Terminal 1) */
    tCbuffer *out_right_buffer;
    /** The current mode of the decoder */
    unsigned mode;
    /* The number of samples output by the decoder */
    unsigned num_output_samples;
    /* The total number of octets consumed by the decoder
     * Note: Not all decoders update this field currently,
     *       should only be used by the decoders that
     *       implement this.
     */
    unsigned num_input_octets_consumed;    
    /** A pointer to the decoder specific structure */
    void *decoder_data_object;
}DECODER;

/** Structure used by stereo encoders for processing */
typedef struct
{
    /** The encoded output buffer */
    tCbuffer *out_buffer;
    /** The input left channel buffer (Terminal 0) */
    tCbuffer *in_left_buffer;
    /** The input right channel buffer (Terminal 1) */
    tCbuffer *in_right_buffer;
    /** The current mode of the encoder */
    unsigned mode;
    /** A pointer to the decoder specific structure */
    void *encoder_data_object;
}ENCODER;

#endif /* CODEC_C_H */


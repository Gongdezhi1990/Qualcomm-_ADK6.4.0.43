/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
*    \file a2dp_common_decode_struct.h
*
*    Common A2DP decoder structures and definitions.
*/

#ifndef A2DP_COMMON_DECODE_STRUCT_H
#define A2DP_COMMON_DECODE_STRUCT_H

/****************************************************************************
Include Files
*/
#include "a2dp_header_struct.h"
#include "audio_fadeout/audio_fadeout.h"
#include "codec/codec_c.h"

/****************************************************************************
Public Type Declarations
*/

/** Common attributes from sbc and aptX decoder operator structures */
typedef struct
{
    /** A DECODER structure used by the codec library */
    DECODER codec;

    tCbuffer *op_out_left;

    tCbuffer *op_out_right;

    /** A2DP header structure. unused when no A2DP header is present */
    A2DP_HEADER_PARAMS *a2dp_header;

    /** fadeout parameters for left channel */
    FADEOUT_PARAMS left_fadeout;

    /** fadeout_parameters for right channel */
    FADEOUT_PARAMS right_fadeout;

    /** pointer to codec decode's $codec_type.frame_decode function this is not C callble */
    void (*decode_frame)(void);

    /** pointer to codec decode's $codec_type.silence_decoder function. Not currently used */
    void (*silence)(void);

    /** Location of the playing state flag signalled by the connected source endpoint */
    bool *play;

#ifndef TIMED_PLAYBACK_MODE
    /** The software rate adjust cbop structure */
    void *sra;
#endif

#ifdef INSTALL_METADATA
   /** The output buffer with metadata to transport to. (output can be multiple streams) */
   tCbuffer *metadata_op_buffer;
#endif /* INSTALL_METADATA */

} A2DP_DECODER_PARAMS;

/****************************************************************************
Public Constant Declarations
*/
/** Minimum input buffer size */
#define A2DP_DECODE_INPUT_BUFFER_SIZE    768
/** Minimum output buffer size */
#define A2DP_DECODE_OUTPUT_BUFFER_SIZE   256

/** Terminal numbers */
#define LEFT_OUT_TERMINAL_ID            0
#define RIGHT_OUT_TERMINAL_ID           1
#define INPUT_TERMINAL_ID               (0 | TERMINAL_SINK_MASK)

#endif /* A2DP_COMMON_DECODE_STRUCT_H */

// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

// Header file for C stubs of "sbcdec" library
// Comments show the syntax to call the routine

#if !defined(CODEC_LIBRARY_C_STUBS_H)
#define CODEC_LIBRARY_C_STUBS_H


/* PUBLIC TYPES DEFINITIONS *************************************************/

typedef struct
{
    struct Cbuffer *in_buffer;
    struct Cbuffer *out_left_buffer;
    struct Cbuffer *out_right_buffer;
    int mode;
    int num_output_samples;
} tCodecDecoder;



typedef struct
{
    struct Cbuffer *out_buffer;
    struct Cbuffer *in_left_buffer;
    struct Cbuffer *in_right_buffer;
    int mode;
} tCodecEncoder;



/* PUBLIC FUNCTION PROTOTYPES ***********************************************/

// Nothing implemented currently

#endif // SBCDEC_LIBRARY_C_STUBS_H

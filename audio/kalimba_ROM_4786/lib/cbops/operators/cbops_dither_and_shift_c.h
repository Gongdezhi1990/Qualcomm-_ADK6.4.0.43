/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_dither_and_shift_c.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_DITHER_AND_SHIFT_C_H_
#define _CBOPS_DITHER_AND_SHIFT_C_H_

/****************************************************************************
Public Type Declarations
*/
/** No dither just shift*/
#define DITHER_TYPE_NONE 0
/** add triangular pdf dither before shifting */
#define DITHER_TYPE_TRIANGULAR 1
/** add dither and shape quantization noise, then shift */
#define DITHER_TYPE_SHAPED 2

/** Structure of the dither and shift cbop operator specific data */
typedef struct
{
    /** The index into the buffer to start the copy from */
    unsigned input_start_index;
    /** The index into the channel 1 buffer to start a copy to */
    unsigned output_start_index;
    /** The amount to left shift by (-ve gives a right shift) */
    int shift_amount;
    /** The type of dither to use */
    unsigned dither_type;
    /** Historical index used for channel 1 dither */
    unsigned dither_filter_hist;
    /** Indicates whether the compressor is used */
    bool enable_compressor;
}cbops_dither;

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_dither_table[];

#endif /* _CBOPS_DITHER_AND_SHIFT_C_H_ */

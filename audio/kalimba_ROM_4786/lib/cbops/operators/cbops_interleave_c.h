/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_interleave_c.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_INTERLEAVE_C_H_
#define _CBOPS_INTERLEAVE_C_H_

/****************************************************************************
Public Type Declarations
*/
/** Structure of the interleave cbop operator specific data */
typedef struct
{
    /** The number of channels to interleave */
    unsigned num_channels;
}cbops_interleave_op;

typedef enum cbops_interleave_mode {
    CBOPS_INTERLEAVE = 0,
    CBOPS_DEINTERLEAVE = 1
} cbops_interleave_mode;

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_interleave_table[];
extern unsigned cbops_deinterleave_table[];


#endif /* _CBOPS_INTERLEAVE_C_H_ */

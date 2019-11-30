/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_shift_c.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_SHIFT_C_H_
#define _CBOPS_SHIFT_C_H_

/****************************************************************************
Public Type Declarations
*/

/** Structure of the multi-channel shift cbop operator-specific data */
typedef struct cbops_shift{
    /** The number of bits to left shift by (signed hence -ve = right shift */
    int shift_amount;
} cbops_shift;

/****************************************************************************
Public Variable Definitions
*/
/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_shift_table[];

#endif /* _CBOPS_SHIFT_C_H_ */

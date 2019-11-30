/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_insert_op.h
 *
 * \ingroup cbops
 *
 * mix two channels.   
 *   output = [mix_value*input1 + (1.0-mix_value)*input2]*2^shift
 *
 *  Mix operation may be inplace for either input
 *  Input1 may not be a port
 *  Input2 may be a buffer or a port
 *  Output may be a buffer or a port
 */

#ifndef CBOPS_MIXER_H
#define CBOPS_MIXER_H

/****************************************************************************
Public Type Declarations
*/

typedef struct mixer_op{
    int   shift;            /**< pow2 scale applied to output  */
    unsigned mix_value;     /**< fractional mix ratio  */
}cbops_mixer_op;

/****************************************************************************
Public Variable Definitions
*/

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_mixer_table[];


#endif // CBOPS_MIXER_H


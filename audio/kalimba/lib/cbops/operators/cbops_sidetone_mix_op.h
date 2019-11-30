/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_sidetone_mix_op.h
 *
 * \ingroup cbops
 *
 * mix sidetone with the output.   Provide latency control
 * and synchronization between signals
 */

#ifndef CBOPS_SIDETONE_MIX_OP_H
#define CBOPS_SIDETONE_MIX_OP_H
/****************************************************************************
Include Files
*/

#include "buffer/cbuffer_c.h"
  
/****************************************************************************
Public Type Declarations
*/

typedef struct st_mix{
    /* Channel-independent params, technically speaking... as this cbop only
     * ever does a single channel and just "fits" into the multi-channel model
     */
    unsigned    st_in_idx;          /**< pointer to sidetone buffer */
    unsigned    max_samples;        /**< maximum samples mixed per iteraction  */
    unsigned    num_inserts;        /**< Total number of inserted samples.  Status  */
    unsigned    num_drops;          /**< Total number of dropped samples.  Status  */
}cbops_sidetone_mix_op;

/****************************************************************************
Public Variable Definitions
*/

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_sidetone_mix_table[];


#endif /* CBOPS_SIDETONE_MIX_OP_H */

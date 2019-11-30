/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_shift_op.c
 * \ingroup cbops 
 *
 * This file contains functions for the shift copy cbops operator
 */

/****************************************************************************
Include Files
 */
#include "pmalloc/pl_malloc.h"
#include "cbops_c.h"

/****************************************************************************
Public Function Definitions
*/

/*
 * create_shift_op
 * Due to its particulars, it may only have one param for number of creation-time channels. Other cbops
 * may have two different numbers.
 */
cbops_op* create_shift_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx, int shift_amt)
{
    // cbop param struct size (header plus cbop-specific parameters)
    cbops_op *op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_shift, nr_channels, nr_channels));

    if(op != NULL)
    {
        cbops_shift *params;
        /* Setup Operator func table*/
        op->function_vector = cbops_shift_table;

        /* Setup cbop param struct header info */
        params = (cbops_shift*)cbops_populate_param_hdr(op, nr_channels, nr_channels, input_idx, output_idx);

        /* Setup cbop-specific parameters - well, just one in this case. Just exemplifies using the
         * generic macro, but in this cbop's case we could get to the param location "directly"
         */
        params->shift_amount = shift_amt;
    }

    return(op);
}



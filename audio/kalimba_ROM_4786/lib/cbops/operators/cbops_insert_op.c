/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_insert_op.c
 * \ingroup cbops
 *
 * This file contains functions for the HW rate monitoring cbops operator
 */

/****************************************************************************
Include Files
 */
#include "pmalloc/pl_malloc.h"
#include "cbops_c.h"
#include "cbops_insert_op.h"

/****************************************************************************
Public Function Definitions
*/

/*
 * create_insert_op
 */
cbops_op* create_insert_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx, unsigned threshold)
{
    // cbop param struct size (header plus cbop-specific parameters)
    cbops_op *op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_insert_op, nr_channels, nr_channels));

    if(op)
    {
        cbops_insert_op *params;
        
        /* Setup Operator func table */
        op->function_vector    = cbops_insert_table;

        /* Setup cbop param struct header info */
        params = (cbops_insert_op*)cbops_populate_param_hdr(op, nr_channels, nr_channels, input_idx, output_idx);

        /* Setup cbop-specific parameters - well, just one in this case. Just exemplifies using the
         * generic macro, but in this cbop's case we could get to the param location "directly"
         */
        params->threshold = threshold;
    }

    return(op);
}


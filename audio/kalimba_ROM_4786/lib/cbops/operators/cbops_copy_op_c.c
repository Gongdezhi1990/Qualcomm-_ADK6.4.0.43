/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_copy_op_c.c
 * \ingroup cbops
 *
 * This file contains functions for the DC offset removeal cbops operator
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
 * create_copy_op
 * Due to its particulars, it may only have one param for number of creation-time channels. Other cbops
 * may have two different numbers.
 */
cbops_op* create_copy_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx)
{
    /* This cbop has no op-specific params at all */
    cbops_op* op = (cbops_op *)xpmalloc(sizeof(cbops_op) + CBOPS_PARAM_HDR_SIZE(nr_channels, nr_channels));

    if(NULL != op)
    {
        /* Setup Operator func table */
        op->function_vector = cbops_copy_table;

        /* Setup cbop param struct header info */
        cbops_populate_param_hdr(op, nr_channels, nr_channels, input_idx, output_idx);
    }

    return(op);
}



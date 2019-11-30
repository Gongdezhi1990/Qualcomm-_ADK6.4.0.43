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
#include "cbops_sidetone_mix_op.h"

/****************************************************************************
Public Function Definitions
*/

/*
 * create_sidetone_mix_op
 * It only works on a single input and output channel.
 */
cbops_op* create_sidetone_mix_op(unsigned input_idx, unsigned output_idx,unsigned st_in_idx, unsigned threshold)
{
    cbops_op *op;

    op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_sidetone_mix_op, 1, 1));
    if(op)
    {
        cbops_sidetone_mix_op  *params;
        
        op->function_vector    = cbops_sidetone_mix_table;

        /* Setup cbop param struct header info */
        params = (cbops_sidetone_mix_op*)cbops_populate_param_hdr(op, 1, 1, &input_idx, &output_idx);

        /* Set up the parameters - there are no channel-specific params as such, as it only works on a
         * single channel.
         */

        params->st_in_idx = st_in_idx;
        params->max_samples = threshold;
    }

    return(op);
}



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
#include "aec_reference_cap_c.h"
#include "cbops_aec_ref_spkr_op.h"

/****************************************************************************
Public Function Definitions
*/

/*
 * create_insert_op
 */
cbops_op* create_aec_ref_spkr_op(unsigned nr_inputs,unsigned* input_idx,unsigned in_threshold,unsigned nr_outputs,unsigned* output_idx,unsigned out_threshold)
{
    // cbop param struct size (header plus cbop-specific parameters)
    cbops_op *op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_aec_ref_op, nr_inputs, nr_outputs));

    if(op)
    {
        cbops_aec_ref_op *params;
        
        /* Setup Operator func table */
        op->function_vector    = cbops_aec_ref_spkr_table;

        /* Setup cbop param struct header info */
        params = (cbops_aec_ref_op*)cbops_populate_param_hdr(op, nr_inputs, nr_outputs, input_idx, output_idx);

        /* Setup cbop-specific parameters - well, just one in this case. Just exemplifies using the
         * generic macro, but in this cbop's case we could get to the param location "directly"
         */
        params->in_threshold  = in_threshold;
        params->max_advance   = out_threshold;
        params->buffer_adj = 3;
    }

    return(op);
}


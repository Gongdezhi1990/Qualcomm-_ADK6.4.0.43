/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_underrun_comp_op.c
 * \ingroup cbops
 *
 * This file contains functions for the underrun compensating cbops operator
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
 * create_underrun_comp_op
 * The table of indexes contain nr_channels entries.
 */
cbops_op* create_underrun_comp_op(unsigned nr_ins, unsigned nr_outs,
                                        unsigned* input_idx, unsigned* output_idx,
                                        int* rm_diff_ptr, unsigned *block_size_ptr,
                                        unsigned *total_inserts_ptr, unsigned *data_block_size_ptr,
                                        unsigned *delta_samples_ptr, unsigned *insertion_vals_ptr,
                                        bool *sync_start_ptr)
{
    // cbop param struct size (header plus cbop-specific parameters)
    cbops_op *op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_underrun_comp_op, nr_ins, nr_outs));

    if(op != NULL)
    {
        cbops_underrun_comp_op *params;
        
        /* Setup Operator func table*/
        op->function_vector = cbops_underrun_comp_table;

        /* Setup cbop param struct header info */
        /* Get to the alg parameters and init some of them with given values - all the rest are zero already */
        params = (cbops_underrun_comp_op*)cbops_populate_param_hdr(op, nr_ins, nr_outs, input_idx, output_idx);

        /* Params used to communicate with others in the chain and/or with the owner entity */
        params->block_size_ptr = block_size_ptr;
        params->rm_diff_ptr = rm_diff_ptr;
        params->total_inserts_ptr = total_inserts_ptr;
        params->insertion_vals_ptr = insertion_vals_ptr;
        params->data_block_size_ptr = data_block_size_ptr;
        params->delta_samples_ptr = delta_samples_ptr;
        params->sync_start_ptr = sync_start_ptr;
    }

    return(op);
}

/* Get cbop values that are used for inter-component communication (cbop<->cbop, cbop<->endpoint) */
bool get_underrun_comp_op_vals(cbops_op *op, int **rm_diff_ptr, unsigned **block_size_ptr,
                               unsigned **total_inserts_ptr, unsigned **data_block_size_ptr,
                               unsigned **delta_samples_ptr, unsigned **insertion_vals_ptr,
                               bool **sync_start_ptr)
{
    if(op == NULL)
    {
        return FALSE;
    }

    /* Get to the alg parameters */
    cbops_underrun_comp_op *params = CBOPS_PARAM_PTR(op, cbops_underrun_comp_op);

    *rm_diff_ptr = params->rm_diff_ptr;
    *total_inserts_ptr = params->total_inserts_ptr;
    *data_block_size_ptr = params->data_block_size_ptr;
    *delta_samples_ptr = params->delta_samples_ptr;
    *insertion_vals_ptr = params->insertion_vals_ptr;
    *block_size_ptr = params->block_size_ptr;
    *sync_start_ptr = params->sync_start_ptr;

    return TRUE;
}


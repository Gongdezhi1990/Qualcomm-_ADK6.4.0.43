/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_discard_op.c
 * \ingroup cbops
 *
 * This file contains functions for the discard cbops operator
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
 * create_discard_op
 * The table of indexes contain nr_channels entries.
 */
cbops_op* create_discard_op(unsigned nr_ins, unsigned* input_idx,
                                  unsigned *block_size_ptr, unsigned rm_headroom,
                                  unsigned *total_inserts_ptr, unsigned *data_block_size_ptr,
                                  bool *sync_start_ptr)
{
    // cbop param struct size (header plus cbop-specific parameters)
    cbops_op *op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_discard_op, nr_ins, 0));

    if(op != NULL)
    {
        cbops_discard_op *params;
        
        /* Setup Operator func table*/
        op->function_vector = cbops_discard_table;

        /* Setup cbop param struct header info */
        /* Get to the alg parameters and init some of them with given values - all the rest are zero already */
        params = (cbops_discard_op*)cbops_populate_param_hdr(op, nr_ins, 0, input_idx, NULL);

        params->block_size_ptr = block_size_ptr;
        params->rm_headroom = rm_headroom;

        /* Params used to communicate with others in the chain and/or with the owner entity */
        params->total_inserts_ptr = total_inserts_ptr;
        params->data_block_size_ptr = data_block_size_ptr;
        params->sync_start_ptr = sync_start_ptr;
    }

    return(op);
}

/* Get cbop values that are used for inter-component communication (cbop<->cbop, cbop<->endpoint) */
bool get_discard_op_vals(cbops_op *op,
                         unsigned **block_size_ptr, unsigned *rm_headroom,
                         unsigned **total_inserts_ptr, unsigned **data_block_size_ptr,
                         bool **sync_start_ptr)
{
    if(op == NULL)
    {
        return FALSE;
    }

    /* Get to the alg parameters */
    cbops_discard_op *params = CBOPS_PARAM_PTR(op, cbops_discard_op);

    *block_size_ptr = params->block_size_ptr;
    *rm_headroom = params->rm_headroom;
    *total_inserts_ptr = params->total_inserts_ptr;
    *data_block_size_ptr = params->data_block_size_ptr;
    *sync_start_ptr = params->sync_start_ptr;

    return TRUE;
}

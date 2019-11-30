/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_mixer_op.c
 * \ingroup cbops
 *
 * This file contains functions for the two channel mixer cbops operator
 */

/****************************************************************************
Include Files
 */
#include "pmalloc/pl_malloc.h"
#include "cbops_c.h"
#include "cbops_mixer_op.h"

/****************************************************************************
Public Function Definitions
*/


/*
 * create_mixer_op
 * It fits into multichannel model as a 2-in , 1-out cbop (number of channels is pre-defined
 * in this case. Further generalisation is possible, as/when needed. to N-to-1 downmixer.
 */
cbops_op* create_mixer_op(unsigned input1_idx, unsigned input2_idx, unsigned output_idx,
                                unsigned shift, unsigned mix_ratio)
{
    cbops_op *op;

    op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_mixer_op, 2, 1));

    if(op)
    {
        cbops_param_hdr *params_hdr;
        cbops_mixer_op  *params;
        
        op->next_operator_addr = CBOPS_NO_MORE_OPERATORS_PTR;
		op->prev_operator_addr = CBOPS_NO_MORE_OPERATORS_PTR;
        op->function_vector    = cbops_mixer_table;

        /* As channels are hard-wired, just cheaply fill in header - no need to use the
         * full-blown generalised function.
         */
        params_hdr = (cbops_param_hdr*)CBOPS_OPERATOR_DATA_PTR(op);

        params_hdr->operator_data_ptr = (void*)&params_hdr->index_table[3];
        
        params_hdr->nr_inputs = 2;
        params_hdr->nr_outputs = 1;
        params_hdr->index_table[0] = input1_idx;
        params_hdr->index_table[1] = input2_idx;
        params_hdr->index_table[2] = output_idx;

        /* Alg-specific channel-independent params */
        params = (cbops_mixer_op*)params_hdr->operator_data_ptr;
        params->shift = shift;
        params->mix_value = mix_ratio;
    }

    return(op);
}


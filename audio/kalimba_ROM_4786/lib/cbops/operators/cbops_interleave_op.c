/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_interleave_op.c
 * \ingroup cbops
 *
 */

#include "pmalloc/pl_malloc.h"
#include "cbops_c.h"
#include "cbops_interleave_c.h"

cbops_op* create_interleave_op(cbops_interleave_mode mode, unsigned num_channels)
{
    cbops_op *op;
    void *function_vector;
    if (mode==CBOPS_INTERLEAVE)
    {
        op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_interleave_op, num_channels, 1));
        function_vector = cbops_interleave_table;
    }
    else
    {
        op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_interleave_op, 1, num_channels));
        function_vector = cbops_deinterleave_table;
    }

    if(op)
    {
        cbops_interleave_op  *params = (cbops_interleave_op*)(&op->parameter_area_start);
        op->next_operator_addr = CBOPS_NO_MORE_OPERATORS_PTR;
        op->function_vector = function_vector;
        params->num_channels = num_channels;
    }
    return(op); 
}

void destroy_interleave_op(cbops_op *op)
{    
    pfree(op);
}

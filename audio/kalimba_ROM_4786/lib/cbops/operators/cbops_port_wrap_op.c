/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops cbops_port_wrap_op.c
 * \ingroup cbops
 *
 * This file contains functions for the port wrap protection cbops operator
 */

/****************************************************************************
Include Files
 */
#include "pmalloc/pl_malloc.h"
#include "cbops_c.h"
#include "cbops_port_wrap_op.h"

/****************************************************************************
Public Function Definitions
*/

/*
 * create_port_wrap_op
 * Due to its particulars, it does not have channels basically nor indexes (nr_ins / outs = 0),
 * it just receives some buffer pointers. Latter are not necessarily buffers in the framework
 * object buffer info table, so can't make that assumption and treat them as actual channels.
 */
cbops_op* create_port_wrap_op(unsigned nr_out_bufs, unsigned *out_bufs, unsigned threshold)
{
    cbops_op *op;

    if(nr_out_bufs < 1)
    {
        return NULL;
    }

    /* Allocate - NOTE: zero input channels basically, it "just" works on some output buffers so can be modelled 
     * with nr_ins = 0. One buffer ptr is accounted for already in the cbop param struct.
     */
    op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_port_wrap_op, nr_out_bufs, 0) );

    if(op)
    {
        cbops_port_wrap_op *params;
        
        /* Setup Operator func table */
        op->function_vector    = cbops_port_wrap_table;

        /* Setup cbop param struct header info */
        params = (cbops_port_wrap_op*)cbops_populate_param_hdr(op, nr_out_bufs, 0, out_bufs, NULL);

        /* Channel-independent parameters, followed by channel-specific ones (NOTE: no input channel params) */
        params->buffer_adj = 3;
        params->max_advance = threshold;
    }
    return(op);
}


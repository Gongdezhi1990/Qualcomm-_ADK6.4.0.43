/****************************************************************************
 * Copyright (c) 2018Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_rate_monitor_op_extra.c
 * \ingroup cbops
 *
 * This file contains extra functions for the rate monitoring cbops operator
 */

/****************************************************************************
Include Files
 */
#include "pmalloc/pl_malloc.h"
#include "cbops_c.h"
#include "cbops/operators/cbops_rate_monitor_op.h"
#include "platform/pl_fractional.h"

/****************************************************************************
Public Function Definitions
*/

/*
 * create_rate_monitor_operator
 */
#define RATE_MATCH_OP_NUM_INPUTS    1

cbops_op* create_rate_monitor_operator(unsigned clk_per_sec,unsigned idx)
{
    // cbop param struct size (header plus cbop-specific parameters)
    cbops_op *op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_rate_monitor_op, RATE_MATCH_OP_NUM_INPUTS, 0));

    if(op)
    {
        cbops_rate_monitor_op *params;
        
        /* Setup Operator func table */
        op->function_vector    = cbops_rate_monitor_table;

        /* Setup cbop param struct header info */
        params  = (cbops_rate_monitor_op*)cbops_populate_param_hdr(op, RATE_MATCH_OP_NUM_INPUTS, 0, &idx, NULL);

        params->period_per_second = clk_per_sec;
        params->average_io_rate = FRACTIONAL(0.5);
    }

    return(op);
}

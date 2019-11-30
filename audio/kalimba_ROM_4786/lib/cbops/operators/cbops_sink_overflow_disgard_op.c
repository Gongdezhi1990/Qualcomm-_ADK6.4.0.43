/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_sink_overflow_disgard_op.c
 * \ingroup cbops 
 *
 * This file contains functions for overflow of sink operator
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
 * create_sink_overflow_disgard_op
 * The table of indexes contain nr_channels entries.
 */
cbops_op* create_sink_overflow_disgard_op(unsigned nr_channels,unsigned* output_idx,unsigned min_space)
{
    // cbop param struct size (header plus cbop-specific parameters)
    cbops_op *op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_sink_overflow_disgard_op, nr_channels, 0));

    if(op != NULL)
    {
        cbops_sink_overflow_disgard_op *params;
        
        /* Setup Operator func table*/
        op->function_vector = cbops_sink_overflow_disgard_table;

        /* Setup cbop param struct header info */
        /* Get to the alg parameters and init some of them with given values - all the rest are zero already */
        params = (cbops_sink_overflow_disgard_op*)cbops_populate_param_hdr(op, nr_channels, 0, output_idx, NULL);

        /* Params used to communicate with others in the chain and/or with the owner entity */
        params->min_space = min_space;
    }

    return(op);
}






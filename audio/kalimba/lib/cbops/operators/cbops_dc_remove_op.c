/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_dc_remove_op.c
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
 * create_dc_remove_op
 * Due to its particulars, it may only have one param for number of creation-time channels. Other cbops
 * may have two different numbers.
 */
cbops_op* create_dc_remove_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx)
{
    unsigned channel;

    /* Allocate for struct for all channels, one channel's parameter is included in the size of cbops_dc_remove */
    cbops_op* op = (cbops_op *)xpmalloc(sizeof_cbops_op(cbops_dc_remove, nr_channels, nr_channels) +
                                                    (nr_channels - 1)*sizeof(int));

    if(op != NULL)
    {
        cbops_dc_remove *params;
        /* Setup Operator func table */
        op->function_vector = cbops_dc_remove_table;

        /* Setup cbop param struct header info */
        params = (cbops_dc_remove*)cbops_populate_param_hdr(op, nr_channels, nr_channels, input_idx, output_idx);

        /* This cbop has no channel-independent params, so at the start of cbop-specific
         * param block, we have the channel-specific params.
         */

        /* Fill channel-specific history buffer information. The history buffers were contiguously
         * allocated after the cbop paramer struct, thus they follow each other after the 
         * channel-specific parameters.
         */
        for(channel=0; channel < nr_channels; channel++)
        {
            params->dc_estimate[channel] = 0;
        }
    }

    return(op);
}



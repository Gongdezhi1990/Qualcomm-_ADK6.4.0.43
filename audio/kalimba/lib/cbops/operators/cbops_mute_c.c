/****************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_mute_op.c
 * \ingroup cbops
 *
 * This file contains functions for the cbops mute operator
 */

/****************************************************************************
Include Files
 */
#include "pmalloc/pl_malloc.h"
#include "cbops_c.h"
/****************************************************************************
Public Function Definitions
*/

/**
 * create a cbops mute operator (multi-channel)
 *
 * \param nr_channels   Number of channels at creation time.
 * \param io_idx     Pointer to input/output channel indexes.
 * \return pointer to operator
 */
cbops_op* create_mute_op(unsigned nr_channels, unsigned* io_idx)
{
    /* Allocate cbops_op struct, mute op only has input channels */
    cbops_op* op = (cbops_op *)xpmalloc(sizeof_cbops_op(cbops_mute, nr_channels, 0));
    if(op != NULL)
    {
        /* Setup Operator func table */
        op->function_vector = cbops_mute_table;

        /* Setup cbop param struct header info */
        cbops_populate_param_hdr(op, nr_channels, 0, io_idx, NULL);

        /* reset to unmute state */
        cbops_mute_reset(op);
    }
    return(op);
}

/**
 * configure cbops_mute for muting/unmuting
 *
 * \param op Pointer to cbops_mute operator structure
 * \param enable Any non-zero value will mute
 */
void cbops_mute_enable(cbops_op *op, bool enable)
{
    cbops_mute *params = CBOPS_PARAM_PTR(op, cbops_mute);
    params->mute_enable = enable;
}

/**
 * reset cbops_mute (to unmute state)
 *
 * \param op Pointer to cbops_mute operator structure
 */
void cbops_mute_reset(cbops_op *op)
{
    cbops_mute *params = CBOPS_PARAM_PTR(op, cbops_mute);
    params->mute_enable = FALSE;
    params->mute_state = FALSE;
}


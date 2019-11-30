/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_discard_op.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_DISCARD_OP_H_
#define _CBOPS_DISCARD_OP_H_

#include "types.h"

/****************************************************************************
Public Type Declarations
*/

/** Structure of the multi-channel discard cbop operator-specific data */
typedef struct discard
{
    /** Discard amount across channels */
    unsigned discard_amount;

    /* RM headroom amount */
    unsigned rm_headroom;

    /* Parameters used to communicate between this cbop, the discard cbop
     * and the owner entity.
     */

    /** Block size used by the owner entity (sink endpoint);
     *  this equates to data amount for the kick period
     */
    unsigned *block_size_ptr;

    /** Pointer to estimated data block size for arriving lumps of data */
    unsigned *data_block_size_ptr;

    /** Pointer to cumulative count of insertions that happened since start */
    unsigned *total_inserts_ptr;

    /** Pointer to "data started" flag which is updated by others */
    bool *sync_start_ptr;

} cbops_discard_op;

/****************************************************************************
Public Variable Definitions
*/
/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_discard_table[];

#endif /* _CBOPS_DISCARD_OP_H_ */

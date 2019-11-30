/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_underrun_comp.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_UNDERRUN_COMP_OP_H_
#define _CBOPS_UNDERRUN_COMP_OP_H_

#include "types.h"

/****************************************************************************
Public Type Declarations
*/

/** Structure of the multi-channel underrun compensating cbop operator-specific data */
typedef struct underrun_comp
{
    /** Number of samples to insert across channels */
    unsigned max_insert;

    /** amount_to_use calculated based on rm_diff and other params (internal) */
    unsigned amount;

    /** Previous amount "seen" at inputs (internal) */
    unsigned prev_amount;

    /** A prev_amount tweak that occurs due to changes in RM (internal) */
    int prev_amount_tweak;

    /** Number of consecutive stalls, after which we revert to
     * "not yet started" state (internal)
     */
    unsigned nr_consecutive_stalls;

    /** Sum and count of data block estimations (internal) */
    unsigned sum_data_blks;
    unsigned data_blk_updates;

    /** Postponed start flag (internal) */
    bool start_next_kick;

    /* Parameters used to communicate between this cbop, the discard cbop
     * and the owner entity.
     */

    /** Block size used by the owener entity (sink endpoint);
     *  this equates to data amount for the kick period
     */
    unsigned* block_size_ptr;

    /** Pointer to RM diff value, utilised also at endpoint level */
    int* rm_diff_ptr;

    /** Pointer to estimated data block size for arriving lumps of data */
    unsigned *data_block_size_ptr;

    /** Pointer to cumulative count of insertions that happened since start */
    unsigned *total_inserts_ptr;

    /* Delta samples as calculated by RM functions at endpoint level */
    unsigned *delta_samples_ptr;

    /** Pointer to vector of values of the last copied samples on each channel.
     *  If not known, set those values to zero for just silence insertions.
     */
    unsigned* insertion_vals_ptr;

    /** In-sync started state flag (when multiple channels).
     * In single channel case, it is the data started state for that channel.
     *  true if data has started to arrive after init or long stall
     */
    bool *sync_start_ptr;
#ifdef TODO_MITIGATE_CBOPS_DRIFT_IN_NON_TTP_USE_CASE
    /* minimum amount of space available across all output channels */
    unsigned min_space_available;
#endif

} cbops_underrun_comp_op;

/****************************************************************************
Public Variable Definitions
*/
/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_underrun_comp_table[];

#endif /* _CBOPS_UNDERRUN_COMP_OP_H_ */

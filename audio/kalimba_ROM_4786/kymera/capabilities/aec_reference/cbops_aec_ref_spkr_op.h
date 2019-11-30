/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_insert_op.h
 *
 * \ingroup cbops
 *
 * insert samples in stream if there is insufficient input
 * to produce the desired output. It acts on all its channels in tandem,
 * so amounts, when instantiated as multi-channel operator, match across
 * channels - this makes it act in "sync". If sync is not desired, then
 * instantiate the operators as single-channel instances and let them act
 * completely independently of each other.
 */


#ifndef CBOPS_AECREF_SPKR_H
#define CBOPS_AECREF_SPKR_H

typedef struct aec_ref_op
{
    unsigned in_threshold;   /**< threshold on input for insertion - one for all channels */
    unsigned max_advance;    /**< threshold on input for insertion - one for all channels */
    unsigned buffer_adj;     /**< Adjustment to account for abiguity in reading amount of data in buffer  */

    unsigned amount_data;    /**< internal working variable - data amount for all channels */

    unsigned wrap_count;     /**< Total number of insertions at output*/
    unsigned num_inserts;    /**< total number of samples inserted at input */
}cbops_aec_ref_op;

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_aec_ref_spkr_table[];

#endif /* CBOPS_AECREF_SPKR_H */

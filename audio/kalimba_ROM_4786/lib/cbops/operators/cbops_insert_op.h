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


#ifndef CBOPS_INSERT_H
#define CBOPS_INSERT_H

typedef struct insert_op
{
    unsigned threshold;     /**< threshold on input for insertion - one for all channels */
    unsigned amount_data;   /**< internal working variable - data amount for all channels */
    unsigned num_inserts;   /**< total number of samples inserted.  status - same for all channels */
}cbops_insert_op;

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_insert_table[];

#endif /* CBOPS_INSERT_H */

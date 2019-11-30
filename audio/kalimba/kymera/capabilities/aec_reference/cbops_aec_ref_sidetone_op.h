/****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_aec_ref_sidetone_op.h
 *
 * \ingroup cbops
 * In place mixing of sidetone into speaker
 */

#ifndef CBOPS_AECREF_SIDETONE_OP_H
#define CBOPS_AECREF_SIDETONE_OP_H
#include "cbuffer_c.h"
#define MAX_SIDETONE_SPEAKERS 2
typedef struct aec_ref_sidetone_op
{
    tCbuffer **spkr_bufs;           /* speaker buffers */
    tCbuffer *mic_buf;              /* mic buffer */
    unsigned nr_spkrs;
    unsigned spkr_mixing_offsets[MAX_SIDETONE_SPEAKERS];
    unsigned mic_idx;               /* first mic buffer index */
    unsigned safety_threshold_low;  /* Minimum difference between HW offset and mixing offset */
    unsigned safety_threshold_high; /* Minimum difference between HW offset and mixing offset */
    unsigned safety_set_threshold;  /* This is used for resetting mixing offset if goes out of range */
    unsigned spkr_threshold;        /* number of samples expected to be consumed by spkr HW each time */
    unsigned sidetone_discarded_samples; /* debug variable: total discarded samples */
    unsigned sidetone_inserted_samples;  /* debug variable: total inserted silence samples */
    unsigned last_sidetone_sample;       /* last sidetone samples for repeating */
    unsigned sidetone_aligned_samples;   /* debug variable: total samples moved to align mixing offset
                                          * with HW offset*/
    unsigned last_spkr_addr;             /* last speaker read address, used for calculating number of
                                          * new samples */
    unsigned amount_to_mix;
}cbops_aec_ref_sidetone_op;

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_aec_ref_sidetone_table[];
#endif /* CBOPS_AECREF_SIDETONE_OP_H */

/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_iir_resamplerv2_op.h
 *
 * \ingroup cbops
 *
 * IIR resampler cbops operator
 */

#ifndef CBOPS_IIR_RESAMPLER_H
#define CBOPS_IIR_RESAMPLER_H

#include "iir_resamplerv2_common.h"


/* The cbops parameters data structure, this is preceded by the cbops multichannel
 * common header (see cbops_param_hdr definition in common cbops.h).
 */
typedef struct iir_resampler_op
{
    /* Channel-independent parameters */
    iir_resamplerv2_common    common;

    /* Channel-specific parameters and working data blocks.
     * NOTE: below fields get allocated dynamically
     * one or more times deoending on number of channels!!
     * The best we can do in C is to signal with [1] but dynamic allocation
     * in multichannel generic case will end up with the following actual
     * structure:
     *
     * [common params]
     * pointer to [working] for channel 1...N
     * ...
     * [workingN] for channel N
     *
     * [channel1 params]
     * ...
     * [channelN params]
     *
     * Note that same number of input and outputs are in use in the case of this cbop.
     */
    unsigned*    working;
    iir_resamplerv2_channel   channel[];

}cbops_iir_resampler_op;

/****************************************************************************
Public Variable Definitions
*/
/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_iir_resampler_table[];


#endif /* CBOPS_IIR_RESAMPLER_H */




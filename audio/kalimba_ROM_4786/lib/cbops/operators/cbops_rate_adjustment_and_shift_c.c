/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_rate_adjustment_and_shift_c.c
 * \ingroup cbops
 *
 * This file contains the functions that facilitate dynamic instatiation of the
 * rate adjust and shift cbop operator.
 */

/****************************************************************************
Include Files
*/
#include "mem_utils/shared_memory_ids.h"
#include "pmalloc/pl_malloc_preference.h"
#include "pmalloc/pl_malloc.h"
#include "cbops_c.h"
#include "platform/pl_fractional.h"
#include "platform/pl_assert.h"
#include "stream/stream.h"
#include "patch/patch.h"

/****************************************************************************
Public Function Definitions
*/
/*
 * cbops_rateadjust_passthrough_mode
 */
void cbops_rateadjust_passthrough_mode(cbops_op *op, bool enable)
{
     cbops_rate_adjustment_and_shift *params = CBOPS_PARAM_PTR(op, cbops_rate_adjustment_and_shift);

     params->passthrough_mode = enable;
}


/*
 * create_sw_rate_adj_op
 */
cbops_op* create_sw_rate_adj_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx,
                                      unsigned quality, unsigned *rate_val_addr, int shift_amt)
{
    cbops_op *op;
    unsigned       hist_len, channel;
    int            *lp_coeffs;

    patch_fn_shared(cbops_lib);

    /* Get Filter Quality - TODO: This doesn't really work at the moment, as underneath
     * there has only been a build-time switching of the filters & filtering. It needs
     * significant changes inside historic sra_resample asm function.
     */
    switch(quality)
    {
#ifdef SRA_NORMAL_QUALITY_COEFFS
        case SW_RA_NORMAL_QUALITY_COEFFS:
            hist_len = SRA_COEFFS_NORMAL_SIZE;
            break;
#endif
#ifdef SRA_HIGH_QUALITY_COEFFS
        case SW_RA_HIGH_QUALITY_COEFFS:
            hist_len = SRA_COEFFS_HIGH_SIZE;
            break;
#endif
#ifdef SRA_VERY_HIGH_QUALITY_COEFFS
        case SW_RA_VERY_HIGH_QUALITY_COEFFS:
            hist_len = SRA_COEFFS_VERY_HIGH_SIZE;
            break;
#endif
        default:
            return NULL;
    }

    /* Get Shared Coefficients */
    lp_coeffs = sra_get_coeffs(quality);

    if(lp_coeffs == NULL)
    {
        return NULL;
    }

    /* Allocate cbop struct - it so happens that for this cbop, input and output channel number is the same
     * The history buffer needs to be in DM1 as the coefficients are in DM2 */
    op = (cbops_op*)xzppmalloc(sizeof_cbops_op(cbops_rate_adjustment_and_shift, nr_channels, nr_channels) +
                                    nr_channels*(sizeof(cbops_rate_adjustment_and_shift_chn_params) +
                                    hist_len*sizeof(unsigned)), MALLOC_PREFERENCE_DM1);

    if(op == NULL)
    {
        /* Release Shared Coefficients */
        sra_release_coeffs(lp_coeffs);
    }
    else
    {
        cbops_rate_adjustment_and_shift *params;
        
        /* Memory was zero initialised so only set non-zero parameters*/

        /* Setup Operator func table */
        op->function_vector = cbops_rate_adjust_table;

        /* Setup cbop param struct header info */
        params = (cbops_rate_adjustment_and_shift*)cbops_populate_param_hdr(op, nr_channels, nr_channels, input_idx, output_idx);

        /* Setup channel-independent (common) cbop parameters. */

        params->sra.filter_coeffs_size  = hist_len;
        params->sra.filter_coeffs = lp_coeffs;
        params->sra.shift_amount = shift_amt;
        params->sra.sra_target_rate_addr = rate_val_addr;

        /* the rf should be initialised to -1.0 (asm) */
        params->sra.rf = FRACTIONAL(-1.0);

        /* Setup channel-specific parameters */
        cbops_rate_adjustment_and_shift_chn_params *chan_params = &(params->channel_params[0]);

        /* Fill channel-specific history buffer information. The history buffers were contiguously
         * allocated after the cbop paramer struct, thus they follow each other after the
         * channel-specific parameters.
         */
        for(channel=0; channel < nr_channels; channel++)
        {
            chan_params->hist_buf = chan_params->hist_buf_start =
                                    ((unsigned*)&(params->channel_params[0])) +
                                    nr_channels*sizeof(cbops_rate_adjustment_and_shift_chn_params)/sizeof(unsigned) +
                                    channel*hist_len;

            /* Move to next channel's block of params */
            chan_params++;
        }
    }

    return(op);
}


/*
 * destroy_sw_rate_adj_op
 */
void destroy_sw_rate_adj_op(cbops_op *op)
{
     cbops_rate_adjustment_and_shift *params = CBOPS_PARAM_PTR(op, cbops_rate_adjustment_and_shift);

     /* Free channel-independent parameter allocation */
     if(params->sra.filter_coeffs)
     {
        sra_release_coeffs(params->sra.filter_coeffs);
     }

     /* Free the cbop structure */
     pfree(op);
}

void cbops_sra_reset(cbops_op *op, unsigned nr_channels)
{
    unsigned channel = 0, hist_len;
    cbops_rate_adjustment_and_shift *params = CBOPS_PARAM_PTR(op, cbops_rate_adjustment_and_shift);
    cbops_rate_adjustment_and_shift_chn_params *chan_params;

    PL_ASSERT(params != NULL);
    hist_len = params->sra.filter_coeffs_size;

    chan_params = &(params->channel_params[0]);
    PL_ASSERT(chan_params != NULL);

    params->sra.rf = FRACTIONAL(-1.0);
    params->sra.prev_short_samples = 0;

    for(channel = 0; channel < nr_channels; channel++)
    {
        chan_params->hist_buf = chan_params->hist_buf_start =
                                ((unsigned*)&(params->channel_params[0])) +
                                nr_channels*sizeof(cbops_rate_adjustment_and_shift_chn_params)/sizeof(unsigned) +
                                channel*hist_len;

        /* Clear the history buffer. */
        memset(chan_params->hist_buf_start, 0, hist_len << LOG2_ADDR_PER_WORD);

        /* Move to next channel's block of params */
        chan_params++;
    }
}

/*
 * cbops_sra_set_rate_adjust
 */
void cbops_sra_set_rate_adjust(cbops_op *op, unsigned nr_channels, unsigned rate)
{
    cbops_rate_adjustment_and_shift *params = CBOPS_PARAM_PTR(op, cbops_rate_adjustment_and_shift);
    PL_ASSERT(params != NULL);

    /* Bypass the current rate calculation by setting the target rate pointer to NULL*/
    params->sra.sra_target_rate_addr = NULL;
    /* Fractional */
    params->sra.sra_current_rate = rate;
}

/*
 * cbops_sra_get_current_rate_adjust
 */
unsigned cbops_sra_get_current_rate_adjust(const cbops_op *op)
{
    cbops_rate_adjustment_and_shift *params = CBOPS_PARAM_PTR(op, cbops_rate_adjustment_and_shift);
    PL_ASSERT(params != NULL);
    /* Fractional */
    return params->sra.sra_current_rate;
}

/*
 * cbops_sra_get_phase
 */
int cbops_sra_get_phase(const cbops_op *op)
{
    if (op == NULL)
    {
        return 0;
    }

    const cbops_rate_adjustment_and_shift *params =
            CBOPS_PARAM_PTR(op, cbops_rate_adjustment_and_shift);

    /*
     * RF is updated at the end of a processing run, so after the cbops
     * returns, this RF is valid for the first sample of the next run.
     * The phase of the next output sample is determined by RF.
     * Prev_short_samples indicates whether in the next run the first input
     * sample will be skipped, i.e. no output sample between last sample
     * of previous block and first sample of next block, i.e. the next
     * output sample whose phase is determined by RF will lie between
     * first and second input samples of the next block. I.e.
     *
     * - if prev_short_samples == 0
     *   - if rf is -1, the next output sample will be generated from the last
     *     input sample of the previous block;
     *   - if rf is 0, the next output sample will be generated from the first
     *     input sample of the next block;
     *   - and proportionally in between.
     * - if prev_short_samples == 1
     *   - like above, shifted by one input sample to the right
     *
     * ^
     * |
     * |                        3 i 4
     * |                      *
     * +------------------2-*---------------------->
     * |               *
     * |           * 1
     * |         i
     *       ---->                <-----
     *  last block                next block
     *  i - input sample
     *  1 - next output sample for RF near -0.8
     *  2 - next output sample for RF near -0.5
     *  3 - next output sample for RF near -0.1
     *  4 - next output sample for RF > 0 (prev_short_samples==1)
     *
     * So the result y can be -1 < y < dmax,
     * where d is the max sample period deviation.
     */
    int value = (int)params->sra.rf;
    if (params->sra.prev_short_samples != 0)
    {
        value += FRACTIONAL(1.0);
    }
    return value;
}

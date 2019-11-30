/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_iir_resamplerv2_op.c
 * \ingroup cbops
 *
 * This file contains functions for IIR resampler cbops operator
 */

/****************************************************************************
Include Files
 */
#include "pmalloc/pl_malloc.h"
#include "cbops_c.h"
#include "cbops_iir_resamplerv2_op.h"

/****************************************************************************
Public Function Definitions
*/

/*
 * create_iir_resamplerv2_op
 */

cbops_op* create_iir_resamplerv2_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx,
                                          unsigned in_rate, unsigned out_rate,
                                          unsigned inter_stage_size, unsigned *inter_stage,
                                          int shift, unsigned dbl_precision, unsigned low_mips)
{
    cbops_op *op;
    void *lpconfig;
    unsigned hist_size=0;

    /* Allocate Filter */
    lpconfig = iir_resamplerv2_allocate_config_by_rate(in_rate,out_rate,low_mips);
    if(lpconfig)
    {
        hist_size = iir_resamplerv2_get_buffer_sizes(lpconfig,dbl_precision);
    }
    /* Filter should be allocated unless configured in passthrough. */
    else if (in_rate != out_rate)
    {
        /* Couldn't allocate memory for the filter. Fail cbop creation. */
        return NULL;
    }

    /* Allocate operator for N channels (N x channel params and N x history buffers */
    op = (cbops_op*)xzppmalloc(sizeof_cbops_op(cbops_iir_resampler_op, nr_channels, nr_channels) +
                              nr_channels*(sizeof(iir_resamplerv2_channel) + hist_size*sizeof(unsigned)), MALLOC_PREFERENCE_DM1);

    if(op)
    {
        /* Get to the start of the parameters, these begin after the index table and nr in/out channel header fields */
        cbops_iir_resampler_op *params;

        op->function_vector    = cbops_iir_resampler_table;

        /* Setup cbop param struct header info */
        params = (cbops_iir_resampler_op*)cbops_populate_param_hdr(op, nr_channels, nr_channels, input_idx, output_idx);

        params->common.intermediate_size  = inter_stage_size;
        params->common.intermediate_ptr   = inter_stage;

        params->common.dbl_precission = dbl_precision;
        /* Hook in ptr to working area (history buffers). These were allocated in one lump together with the
         * cbop struct not to splinter allocs and free separately. History buffs start after the channel params.
         */
        params->working = (unsigned*) (params->channel + nr_channels);

        /* Input shifted to 16-bit for processing head-room */
        if(shift>=0)
        {
            params->common.input_scale  =  shift-8;
            params->common.output_scale =  8;
            /* Output shift to 24-bit */
        }
        else
        {
            params->common.input_scale  = -8;
            params->common.output_scale =  8-shift;
            /* Output shift to 16-bit */
        }
        iir_resamplerv2_set_config(&params->common,lpconfig);
    }
    else if(lpconfig)
    {
        /* Release Filter */
        iir_resamplerv2_release_config(lpconfig);
    }
    return(op);
}

/*
 * destroy_iir_resamplerv2_op
 */
void destroy_iir_resamplerv2_op(cbops_op *op)
{
    /* Get to params that start after the "header" of nr ins/outs and index table */
    cbops_iir_resampler_op *params = CBOPS_PARAM_PTR(op, cbops_iir_resampler_op);

    void *lpconfig = params->common.filter;
    iir_resamplerv2_set_config(&params->common,NULL);
    iir_resamplerv2_release_config(lpconfig);
    pfree(op);
}




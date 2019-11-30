/****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  cbops_aec_ref_sidetone_op.c
 * \ingroup cbops
 *
 * This file contains functions for the in place side tone mixing in SW
 */

/****************************************************************************
Include Files
 */
#include "aec_reference_cap_c.h"
#include "cbops_aec_ref_sidetone_op.h"
#include "patch/patch.h"

/****************************************************************************
Public Function Definitions
*/

#ifdef AEC_REFERENCE_CAN_PERFORM_INPLACE_SIDETONE_MIXING
/**
 * create_aec_ref_sidetone_op
 * \brief creates override cbops operator for standalone sidetone mixing
 *
 * \param mic_buf MMU mic buffer to read microphone samples from
 * \param mic_idx cbops buffer index for mic buffer
 * \param spkr_bufs array of speaker buffers for mixing sidetone into (up to two)
 * \param nr_spkrs number of speakers (1 or 2)
 * \param sidetone_idx pointer to cbops buffer index for sidetone buffer
 * \spkr_threshold max expected mixing samples
 * \safety_threshold min number of samples between HW read point and sidetone mixing point
 */

cbops_op* create_aec_ref_sidetone_op(tCbuffer *mic_buf,
                                     unsigned mic_idx,
                                     tCbuffer **spkr_bufs,
                                     unsigned nr_spkrs,
                                     unsigned *sidetone_idx,
                                     unsigned spkr_threshold,
                                     unsigned safety_threshold)

{
    cbops_op *op = NULL;

    patch_fn_shared(aec_reference);

    /* sidetone can be mixed to 1 or 2 speakers only */
    if(nr_spkrs > 2)
    {
        return op;
    }

    // cbop param struct size (header plus cbop-specific parameters)
    op = (cbops_op*)xzpmalloc(sizeof_cbops_op(cbops_aec_ref_sidetone_op, 1, nr_spkrs));
    if(op)
    {
        cbops_aec_ref_sidetone_op *params;

        /* Setup Operator func table */
        op->function_vector    = cbops_aec_ref_sidetone_table;

        /* Setup cbop param struct header info */
        params = (cbops_aec_ref_sidetone_op*)cbops_populate_param_hdr(op,
                                                                      1,
                                                                      0,
                                                                      sidetone_idx,
                                                                      NULL);

        /* Setup cbop-specific parameters */
        params->spkr_threshold  = spkr_threshold;
        params->safety_threshold_low = safety_threshold;
        params->safety_threshold_high = safety_threshold*2;
        params->safety_set_threshold = (params->safety_threshold_low + params->safety_threshold_high)/2;
        params->mic_buf = mic_buf;
        params->spkr_bufs = spkr_bufs;
        params->mic_idx = mic_idx;
        params->nr_spkrs = nr_spkrs;

        /* initialise the operator */
        aec_ref_sidetone_initialise(op);
    }

    return(op);
}
#endif /* AEC_REFERENCE_CAN_PERFORM_INPLACE_SIDETONE_MIXING */

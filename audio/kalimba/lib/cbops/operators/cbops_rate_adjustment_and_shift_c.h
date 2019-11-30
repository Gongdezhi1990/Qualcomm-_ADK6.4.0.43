/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_rate_adjustment_and_shift_c.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_RATE_ADJUSTMENT_AND_SHIFT_C_H_
#define _CBOPS_RATE_ADJUSTMENT_AND_SHIFT_C_H_
/****************************************************************************
Include Files
*/

#include "types.h"
#include "audio_proc/sra_c.h"

/****************************************************************************
Public Constant Declarations
*/

/* Definitions for SRA filter coefficients table
 * Currently we can only build with one of these defined
 * Ideally the filter would be configurable at run time
 * See B-183792
 */
#ifdef SRA_VERY_HIGH_QUALITY_COEFFS
#define CBOPS_RATEADJUST_COEFFS SW_RA_VERY_HIGH_QUALITY_COEFFS
#endif
#ifdef SRA_HIGH_QUALITY_COEFFS
#define CBOPS_RATEADJUST_COEFFS SW_RA_HIGH_QUALITY_COEFFS
#endif
#ifdef SRA_NORMAL_QUALITY_COEFFS
#define CBOPS_RATEADJUST_COEFFS SW_RA_NORMAL_QUALITY_COEFFS
#endif
/****************************************************************************
Public Type Declarations
*/

/** Channel-specific parameter block, there is one for each channel */
typedef sra_hist_params cbops_rate_adjustment_and_shift_chn_params;

/** Structure of the rate_adjustment and shift multi-channel cbop parameters */
typedef struct cbops_rate_adjustment_and_shift{
    /** The channel independent working data used by the sra algorithm */
    sra_params sra;

    bool passthrough_mode;

    /** Start of channel-specific parameters */
    cbops_rate_adjustment_and_shift_chn_params channel_params[];
}cbops_rate_adjustment_and_shift;

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_rate_adjust_table[];

#endif /* _CBOPS_RATE_ADJUSTMENT_AND_SHIFT_C_H_ */


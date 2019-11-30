// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************


#ifndef CBOPS_RATE_ADJUSTMENT_AND_SHIFT_HEADER_INCLUDED
#define CBOPS_RATE_ADJUSTMENT_AND_SHIFT_HEADER_INCLUDED

#include "audio_proc/sra.h"
#include "cbops_rate_adjustment_and_shift_c_asm_defs.h"

   .CONST   $cbops.rate_adjustment_and_shift.SRA_FIELD							  $cbops_rate_adjustment_and_shift_c.cbops_rate_adjustment_and_shift_struct.SRA_FIELD;
   .CONST   $cbops.rate_adjustment_and_shift.SHIFT_AMOUNT_FIELD                   $cbops.rate_adjustment_and_shift.SRA_FIELD + $cbops_rate_adjustment_and_shift_c.sra_params_struct.SHIFT_AMOUNT_FIELD; // shift amount
   .CONST   $cbops.rate_adjustment_and_shift.FILTER_COEFFS_FIELD                  $cbops.rate_adjustment_and_shift.SRA_FIELD + $cbops_rate_adjustment_and_shift_c.sra_params_struct.FILTER_COEFFS_FIELD; // filter coeffs address
   .CONST   $cbops.rate_adjustment_and_shift.SRA_TARGET_RATE_ADDR_FIELD           $cbops.rate_adjustment_and_shift.SRA_FIELD + $cbops_rate_adjustment_and_shift_c.sra_params_struct.SRA_TARGET_RATE_ADDR_FIELD;         // target rate (address)
   .CONST   $cbops.rate_adjustment_and_shift.FILTER_COEFFS_SIZE_FIELD             $cbops.rate_adjustment_and_shift.SRA_FIELD + $cbops_rate_adjustment_and_shift_c.sra_params_struct.FILTER_COEFFS_SIZE_FIELD;
   .CONST   $cbops.rate_adjustment_and_shift.SRA_CURRENT_RATE_FIELD               $cbops.rate_adjustment_and_shift.SRA_FIELD + $cbops_rate_adjustment_and_shift_c.sra_params_struct.SRA_CURRENT_RATE_FIELD;      // current rate
   .CONST   $cbops.rate_adjustment_and_shift.RF                                   $cbops.rate_adjustment_and_shift.SRA_FIELD + $cbops_rate_adjustment_and_shift_c.sra_params_struct.RF_FIELD;        // internal state
   .CONST   $cbops.rate_adjustment_and_shift.PREV_SHORT_SAMPLES_FIELD             $cbops.rate_adjustment_and_shift.SRA_FIELD + $cbops_rate_adjustment_and_shift_c.sra_params_struct.PREV_SHORT_SAMPLES_FIELD;
   .CONST   $cbops.rate_adjustment_and_shift.PASSTHROUGH_MODE_FIELD               $cbops_rate_adjustment_and_shift_c.cbops_rate_adjustment_and_shift_struct.PASSTHROUGH_MODE_FIELD;      // Passthrough
   .CONST   $cbops.rate_adjustment_and_shift.COMMON_PARAM_STRUC_SIZE              $cbops_rate_adjustment_and_shift_c.cbops_rate_adjustment_and_shift_struct.STRUC_SIZE;

   // channel-specific parameters moved to separate block - one per each channel
   .CONST   $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_FIELD                  $cbops_rate_adjustment_and_shift_c.sra_hist_params_struct.HIST_BUF_FIELD; // history circular buffer
   .CONST   $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_START_FIELD            $cbops_rate_adjustment_and_shift_c.sra_hist_params_struct.HIST_BUF_START_FIELD; // history cirular buffer start address
   .CONST   $cbops.rate_adjustment_and_shift.CHAN_STRUC_SIZE                      $cbops_rate_adjustment_and_shift_c.sra_hist_params_struct.STRUC_SIZE;

   .CONST $cbops.rate_adjustment_and_shift.SRA_UPRATE      $sra.SRA_UPRATE;

#ifndef KYMERA
   // define one of options (better quality, bigger computation)
   //#define SRA_NORMAL_QUALITY_COEFFS
   #define SRA_HIGH_QUALITY_COEFFS
   //#define SRA_VERY_HIGH_QUALITY_COEFFS
#endif

   #ifdef SRA_VERY_HIGH_QUALITY_COEFFS
      .CONST $cbops.rate_adjustment_and_shift.SRA_COEFFS_SIZE 18;
   #endif

   #ifdef SRA_HIGH_QUALITY_COEFFS
      .CONST $cbops.rate_adjustment_and_shift.SRA_COEFFS_SIZE 12;
   #endif

   #ifdef SRA_NORMAL_QUALITY_COEFFS
      .CONST $cbops.rate_adjustment_and_shift.SRA_COEFFS_SIZE 8;
   #endif

  .CONST $sra.MOVING_STEP (0.0015*(1.0/1000.0)/10.0); // 0.0015: interrupt period, this means it would take 8 seconds for 1hz change for a 1khz tone

   #ifdef $sra_ENABLE_DITHER_FUNCTIONS
   // TODO: cannot declare constant as below.  For now replace $sra.scratch_buffer with $M.cbops.av_copy.left_silence_buffer
   // COME BACK AND ADDRESS THIS LATER!
   #endif

#endif // CBOPS_RATE_ADJUSTMENT_AND_SHIFT_HEADER_INCLUDED

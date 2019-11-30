// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef CBOPS_WARP_AND_SHIFT_HEADER_INCLUDED
#define CBOPS_WARP_AND_SHIFT_HEADER_INCLUDED

   .CONST   $cbops.warp_and_shift.INPUT_START_INDEX_FIELD             0;
   .CONST   $cbops.warp_and_shift.OUTPUT_START_INDEX_FIELD            1;
   .CONST   $cbops.warp_and_shift.SHIFT_AMOUNT_FIELD                  2;
   .CONST   $cbops.warp_and_shift.FILT_COEFS_ADDR_FIELD               3;
   .CONST   $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD                4;
#ifdef BASE_REGISTER_MODE
   .CONST   $cbops.warp_and_shift.DATA_TAPS_START_FIELD               $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD + 1;
   .CONST   $cbops.warp_and_shift.WARP_TARGET_ADDR_FIELD              $cbops.warp_and_shift.DATA_TAPS_START_FIELD + 1;
#else
   .CONST   $cbops.warp_and_shift.WARP_TARGET_ADDR_FIELD              $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD + 1;
#endif
   .CONST   $cbops.warp_and_shift.WARP_MAX_RAMP_FIELD                 $cbops.warp_and_shift.WARP_TARGET_ADDR_FIELD + 1;
   .CONST   $cbops.warp_and_shift.CURRENT_WARP_FIELD                  $cbops.warp_and_shift.WARP_MAX_RAMP_FIELD + 1;
   .CONST   $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD             $cbops.warp_and_shift.CURRENT_WARP_FIELD + 1;
   .CONST   $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD        $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD + 1;
   .CONST   $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD              $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD + 1;
   .CONST   $cbops.warp_and_shift.PREVIOUS_STATE_FIELD                $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD + 1;
   .CONST   $cbops.warp_and_shift.STRUC_SIZE                          $cbops.warp_and_shift.PREVIOUS_STATE_FIELD + 1;

   .CONST   $cbops.warp_and_shift.filt_coefs.L_FIELD                  0;
   .CONST   $cbops.warp_and_shift.filt_coefs.R_FIELD                  1;
   .CONST   $cbops.warp_and_shift.filt_coefs.INV_R_FIELD              2;
   .CONST   $cbops.warp_and_shift.filt_coefs.COEFS_FIELD              3;

#endif // CBOPS_WARP_AND_SHIFT_HEADER_INCLUDED

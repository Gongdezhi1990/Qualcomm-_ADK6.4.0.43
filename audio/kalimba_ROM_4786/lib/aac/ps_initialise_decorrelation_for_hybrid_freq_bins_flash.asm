// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_PS_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.ps_initialise_decorrelation_for_hybrid_freq_bins_flash
//
// DESCRIPTION:
//    -
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - toupdate
//
// *****************************************************************************
.MODULE $M.aacdec.ps_initialise_decorrelation_for_hybrid_freq_bins_flash;
   .CODESEGMENT AACDEC_INITIALISE_PS_DECORRELATION_FOR_HYBRID_FREQ_BINS_FLASH_PM;
   .DATASEGMENT DM;

   $aacdec.ps_initialise_decorrelation_for_hybrid_freq_bins_flash:

   // push rLink onto stack
   push rLink;

   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_TEMP_R8] = r8;


   r0 = &$aacdec.ps_decorrelation;
   call $mem.ext_window_access_as_ram;
   // initialise pointers for Hybrid section
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FLASH_TABLES_DM_ADDRESS] = r0;


   // initialise hybrid decorrelation pointers and parametrs
   r0 = M[r9 + $aac.mem.PS_X_hybrid_real_address + (0*ADDR_PER_WORD)];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_REAL_BASE_ADDR] = r0;
   r0 = M[r9 + $aac.mem.PS_X_hybrid_imag_address + (0*ADDR_PER_WORD)];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_IMAG_BASE_ADDR] = r0;

   r0 = M[r9 + $aac.mem.PS_X_hybrid_real_address + (1*ADDR_PER_WORD)];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_DK_OUT_REAL_BASE_ADDR] = r0;
   r0 = M[r9 + $aac.mem.PS_X_hybrid_imag_address + (1*ADDR_PER_WORD)];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_DK_OUT_IMAG_BASE_ADDR] = r0;

   r0 = 1;
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NUM_FREQ_BINS_PER_SAMPLE] = r0;

   r0 = M[r9 + $aac.mem.PS_hybrid_allpass_feedback_buffer_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_ADDRESS] = r0;

   r0 = $aacdec.PS_NUM_ALLPASS_LINKS * 4 * ($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS - 2);
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_SIZE] = r0;

   r0 = ($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS - 2);
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NUMBER_HYBRID_OR_QMF_ALLPASS_FREQS] = r0;

   r0 = r9 + $aac.mem.PS_prev_frame_last_two_hybrid_samples_real;
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_LAST_TWO_SAMPLES_BUFFER_REAL_ADDR] = r0;

   r0 = r9 + $aac.mem.PS_prev_frame_last_two_hybrid_samples_imag;
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_LAST_TWO_SAMPLES_BUFFER_IMAG_ADDR] = r0;

   M1 = 0;
   r1 = $aacdec.PS_NUM_HYBRID_SUB_SUBBANDS - 2;
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SUBBAND_LOOP_BOUND] = r1;

   // load hybrid gDecaySlope_filter[m=0:2]
   r0 = 0.65143905753106;
   M[r9 + $aac.mem.TMP + $aacdec.PS_G_DECAY_SLOPE_FILTER_A + (0*ADDR_PER_WORD)] = r0;
   r0 = 0.56471812200776;
   M[r9 + $aac.mem.TMP + $aacdec.PS_G_DECAY_SLOPE_FILTER_A + (1*ADDR_PER_WORD)] = r0;
   r0 = 0.48954165955695;
   M[r9 + $aac.mem.TMP + $aacdec.PS_G_DECAY_SLOPE_FILTER_A + (2*ADDR_PER_WORD)] = r0;


   r1 = M[r9 + $aac.mem.PS_hybrid_allpass_feedback_buffer_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ZERO_ADDR] = r1;
   r0 = r1 + (3 * ($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS - 2) * ADDR_PER_WORD);
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ONE_ADDR] = r0;
   r0 = r1 + ((3 + 4) * ($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS - 2) * ADDR_PER_WORD);
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_TWO_ADDR] = r0;

   r0 = 1;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

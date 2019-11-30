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
//    $aacdec.ps_initialise_decorrelation_for_qmf_freq_bins_flash
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
.MODULE $M.aacdec.ps_initialise_decorrelation_for_qmf_freq_bins_flash;
   .CODESEGMENT AACDEC_INITIALISE_PS_DECORRELATION_FOR_QMF_FREQ_BINS_FLASH_PM;
   .DATASEGMENT DM;

   $aacdec.ps_initialise_decorrelation_for_qmf_freq_bins_flash:

   // initialise pointers and parameters for QMF decorrelation

   r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FLASH_TABLES_DM_ADDRESS];
   r0 = r0 + ($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS * (2 + ($aacdec.PS_NUM_ALLPASS_LINKS * 2))*ADDR_PER_WORD);

   I0 = r0 + ($aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS*ADDR_PER_WORD);
   I4 = r0 + (($aacdec.X_SBR_WIDTH + $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS)*ADDR_PER_WORD);
   r1 = r0 + (($aacdec.X_SBR_WIDTH * 2)*ADDR_PER_WORD);
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_G_DECAY_SLOPE_FILTER_TABLE_BASE_ADDR] = r1;
   I2 = r0 + ((($aacdec.X_SBR_WIDTH * 2) + (($aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS - $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS + 1) * $aacdec.PS_NUM_ALLPASS_LINKS)
                                        + ($aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS * $aacdec.PS_NUM_ALLPASS_LINKS))*ADDR_PER_WORD);
   I6 = I2 + ($aacdec.X_SBR_WIDTH * $aacdec.PS_NUM_ALLPASS_LINKS*ADDR_PER_WORD);

   r0 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_REAL_BASE_ADDR] = r0;
   r0 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_IMAG_BASE_ADDR] = r0;
   r0 = $aacdec.X_SBR_WIDTH;
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NUM_FREQ_BINS_PER_SAMPLE] = r0;

   r0 = M[r9 + $aac.mem.SBR_X_sbr_other_real_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_DK_OUT_REAL_BASE_ADDR] = r0;
   r0 = M[r9 + $aac.mem.SBR_X_sbr_other_imag_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_DK_OUT_IMAG_BASE_ADDR] = r0;

   r0 = M[r9 + $aac.mem.PS_qmf_allpass_feedback_buffer_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_ADDRESS] = r0;

   r0 = ($aacdec.PS_NUM_ALLPASS_LINKS*(4*(($aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS - $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS) + 1)));
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_SIZE] = r0;

   r0 = (($aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS - $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS) + 1);
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NUMBER_HYBRID_OR_QMF_ALLPASS_FREQS] = r0;

   r0 = r9 + $aac.mem.PS_prev_frame_last_two_qmf_samples_real;
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_LAST_TWO_SAMPLES_BUFFER_REAL_ADDR] = r0;

   r0 = r9 + $aac.mem.PS_prev_frame_last_two_qmf_samples_imag;
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_LAST_TWO_SAMPLES_BUFFER_IMAG_ADDR] = r0;

   r1 = M[r9 + $aac.mem.PS_qmf_allpass_feedback_buffer_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ZERO_ADDR] = r1;
   r0 = r1 + (3 * (($aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS - $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS) + 1)*ADDR_PER_WORD);
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ONE_ADDR] = r0;
   r0 = r1 + ((3 + 4) * (($aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS - $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS) + 1)*ADDR_PER_WORD);
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_TWO_ADDR] = r0;
   rts;

.ENDMODULE;

#endif

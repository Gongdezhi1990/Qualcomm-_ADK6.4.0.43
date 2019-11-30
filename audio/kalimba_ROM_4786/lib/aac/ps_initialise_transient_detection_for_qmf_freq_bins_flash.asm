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
//    $aacdec.ps_initialise_transient_detection_for_qmf_freq_bins_flash
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
.MODULE $M.aacdec.ps_initialise_transient_detection_for_qmf_freq_bins_flash;
   .CODESEGMENT AACDEC_INITIALISE_PS_TRANSIENT_DETECTION_FOR_QMF_FREQ_BINS_FLASH_PM;
   .DATASEGMENT DM;

   $aacdec.ps_initialise_transient_detection_for_qmf_freq_bins_flash:

   // initialise pointers and parameters for QMF Transient-Detection

   r0 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_REAL_BASE_ADDR] = r0;
   r0 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_IMAG_BASE_ADDR] = r0;
   r0 = $aacdec.X_SBR_WIDTH;
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NUM_FREQ_BINS_PER_SAMPLE] = r0;
   r0 = 1*ADDR_PER_WORD;
   M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_INTER_SUBBAND_STRIDE] = r0;

   r0 = M[r9 + $aac.mem.PS_map_freq_bands_to_20_par_bands_x_num_samples_per_frame_table_ptr];
   I1 = r0 + ($aacdec.PS_NUM_HYBRID_FREQ_BANDS_WHEN_20_PAR_BANDS*ADDR_PER_WORD);
   r0 = M[r9 + $aac.mem.PS_frequency_border_table_20_par_bands_ptr];
   I5 = r0 + ($aacdec.PS_NUM_HYBRID_FREQ_BANDS_WHEN_20_PAR_BANDS*ADDR_PER_WORD);

   M1 = $aacdec.PS_NUM_HYBRID_FREQ_BANDS_WHEN_20_PAR_BANDS;
   r0 = $aacdec.PS_NUM_FREQ_BANDS_WHEN_20_PAR_BANDS;
   M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_SUBBAND_LOOP_BOUND] = r0;

   rts;

.ENDMODULE;

#endif

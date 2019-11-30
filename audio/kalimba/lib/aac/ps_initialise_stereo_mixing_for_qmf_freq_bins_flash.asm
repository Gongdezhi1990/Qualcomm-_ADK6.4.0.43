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
//    $aacdec.ps_initialise_stereo_mixing_for_qmf_freq_bins_flash
//
// DESCRIPTION:
//    -
//
// *****************************************************************************
.MODULE $M.aacdec.ps_initialise_stereo_mixing_for_qmf_freq_bins_flash;
   .CODESEGMENT AACDEC_INITIALISE_PS_STEREO_MIXING_FOR_QMF_FREQ_BINS_FLASH_PM;
   .DATASEGMENT DM;

   $aacdec.ps_initialise_stereo_mixing_for_qmf_freq_bins_flash:

   // initialise pointers and parameters for QMF Stereo Mixing
   r0 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SK_REAL_BASE_ADDR] = r0;
   r0 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SK_IMAG_BASE_ADDR] = r0;
   r0 = M[r9 + $aac.mem.SBR_X_sbr_other_real_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DK_REAL_BASE_ADDR] = r0;
   r0 = M[r9 + $aac.mem.SBR_X_sbr_other_imag_ptr];
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DK_IMAG_BASE_ADDR] = r0;

   r0 = $aacdec.X_SBR_WIDTH*ADDR_PER_WORD;
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_INTER_SAMPLE_STRIDE] = r0;
   r0 = 1*ADDR_PER_WORD;
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_INTER_SUBBAND_STRIDE] = r0;

   M1 = $aacdec.PS_NUM_HYBRID_FREQ_BANDS_WHEN_20_PAR_BANDS;
   r0 = $aacdec.PS_NUM_FREQ_BANDS_WHEN_20_PAR_BANDS;    // extend frequency_bands loop bound to process remaining QMF subbands
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_FREQ_BAND_LOOP_BOUND] = r0;
   rts;

.ENDMODULE;

#endif

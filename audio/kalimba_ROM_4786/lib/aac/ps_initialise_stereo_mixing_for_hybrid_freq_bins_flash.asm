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
//    $aacdec.ps_initialise_stereo_mixing_for_hybrid_freq_bins_flash
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
.MODULE $M.aacdec.ps_initialise_stereo_mixing_for_hybrid_freq_bins_flash;
   .CODESEGMENT AACDEC_INITIALISE_PS_STEREO_MIXING_FOR_HYBRID_FREQ_BINS_FLASH_PM;
   .DATASEGMENT DM;

   $aacdec.ps_initialise_stereo_mixing_for_hybrid_freq_bins_flash:

   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_TEMP_R8] = r8;

   // initialise pointers and parameters for hybrid Stereo Mixing

   // if(PS_IID_MODE < 3)
   r0 = M[r9 + $aac.mem.PS_info_ptr];
   r0 = M[r0 + $aacdec.PS_IID_MODE];
   Null = 3 - r0;
   if LE jump fine_resolution_stereo_mixing;
      // coarse resolution
      r0 = M[r9 + $aac.mem.PS_iid_coarse_resolution_scale_factor_table_ptr];
      r0 = r0 + ($aacdec.PS_IID_NUM_QUANT_STEPS_COARSE_RES*ADDR_PER_WORD);
      jump end_if_coarse_or_fine_stereo_mixing;
   // else
   fine_resolution_stereo_mixing:
      // fine resolution
      r0 = M[r9 + $aac.mem.PS_iid_fine_resolution_scale_factor_table_ptr];
      r0 = r0 + ($aacdec.PS_IID_NUM_QUANT_STEPS_FINE_RES*ADDR_PER_WORD);
   end_if_coarse_or_fine_stereo_mixing:

   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_IID_ZERO_SCALE_FACTOR_TABLE_POINTER] = r0;

   // initialise base address pointers for hybrid section
   r0 = M[r9 + $aac.mem.PS_X_hybrid_real_address + (0*ADDR_PER_WORD)];
   r1 = M[r9 + $aac.mem.PS_X_hybrid_imag_address + (0*ADDR_PER_WORD)];
   r2 = M[r9 + $aac.mem.PS_X_hybrid_real_address + (1*ADDR_PER_WORD)];
   r3 = M[r9 + $aac.mem.PS_X_hybrid_imag_address + (1*ADDR_PER_WORD)];
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SK_REAL_BASE_ADDR] = r0;
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SK_IMAG_BASE_ADDR] = r1;
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DK_REAL_BASE_ADDR] = r2;
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DK_IMAG_BASE_ADDR] = r3;

   r0 = 1*ADDR_PER_WORD;
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_INTER_SAMPLE_STRIDE] = r0;
   r0 = $aacdec.PS_NUM_SAMPLES_PER_FRAME*ADDR_PER_WORD;
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_INTER_SUBBAND_STRIDE] = r0;

   M1 = 0;
   r0 = $aacdec.PS_NUM_HYBRID_FREQ_BANDS_WHEN_20_PAR_BANDS;    // extend frequency_bands loop bound to process remaining QMF subbands
   M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_FREQ_BAND_LOOP_BOUND] = r0;

   I3 = r9 + $aac.mem.PS_h12_previous_envelope;
   I6 = r9 + $aac.mem.PS_h21_previous_envelope;
   I7 = r9 + $aac.mem.PS_h22_previous_envelope;
   r0 = 1;
   rts;

.ENDMODULE;

#endif

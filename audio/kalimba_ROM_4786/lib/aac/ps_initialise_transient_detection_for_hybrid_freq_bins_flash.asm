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
//    $aacdec.ps_initialise_transient_detection_for_hybrid_freq_bins_flash
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
.MODULE $M.aacdec.ps_initialise_transient_detection_for_hybrid_freq_bins_flash;
   .CODESEGMENT AACDEC_INITIALISE_PS_TRANSIENT_DETECTION_FOR_HYBRID_FREQ_BINS_FLASH_PM;
   .DATASEGMENT DM;

   $aacdec.ps_initialise_transient_detection_for_hybrid_freq_bins_flash:

   // push rLink onto stack
   push rLink;

   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_TEMP_R8] = r8;

   // allocate temporary memory for Gain_transient_ratio[n=0:31][parameter_band=0:19] and double(P[n=0:31][parameter_band=0:19])
   r0 = (2 * $aacdec.PS_NUM_SAMPLES_PER_FRAME * $aacdec.PS_NUM_PAR_BANDS_IN_BASELINE_DECORRELATION);
   call $aacdec.frame_mem_pool_allocate;

   M[r9 + $aac.mem.TMP + $aacdec.PS_INPUT_POWER_MATRIX_BASE_ADDR] = r1;

   r10 = (2 * $aacdec.PS_NUM_SAMPLES_PER_FRAME);
   I0 = r1;
   r0 = 0;
   do clear_input_power_array_loop_msb;
      M[I0, MK1] = r0;
   clear_input_power_array_loop_msb:

   r10 = (2 * $aacdec.PS_NUM_SAMPLES_PER_FRAME);
   I0 = r1 + ($aacdec.PS_NUM_SAMPLES_PER_FRAME * $aacdec.PS_NUM_PAR_BANDS_IN_BASELINE_DECORRELATION * ADDR_PER_WORD);
   do clear_input_power_array_loop_lsb;
      M[I0, MK1] = r0;
   clear_input_power_array_loop_lsb:

   // use tmp_mem_pool[ [2048:2048+511]; sbr_temp_2[0:639-512] ] as temporary memory
   // for PS_GAIN_TRANSIENT_RATIO[n=0:31][parameter_band=0:19]
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r0 + (2048*ADDR_PER_WORD);
   M[r9 + $aac.mem.TMP + $aacdec.PS_GAIN_TRANSIENT_RATIO_ADDR] = r0;
   // I7 -> PS_GAIN_TRANSIENT_RATIO[n=0:31][parameter_band=0]
   I7 = r0;

   I0 = r9 + $aac.mem.PS_power_peak_decay_nrg_prev;
   I3 = r9 + $aac.mem.PS_power_smoothed_peak_decay_diff_nrg_prev;
   I2 = r9 + $aac.mem.PS_smoothed_input_power_prev;

   // initialise pointers and parameters for hybrid Transient Detection
   r0 = M[r9 + $aac.mem.PS_X_hybrid_real_address + (0*ADDR_PER_WORD)];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_REAL_BASE_ADDR] = r0;
   r0 = M[r9 + $aac.mem.PS_X_hybrid_imag_address + (0*ADDR_PER_WORD)];
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_IMAG_BASE_ADDR] = r0;
   r0 = 1;
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NUM_FREQ_BINS_PER_SAMPLE] = r0;
   r0 = $aacdec.PS_NUM_SAMPLES_PER_FRAME * ADDR_PER_WORD;
   M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_INTER_SUBBAND_STRIDE] = r0;

   r0 = M[r9 + $aac.mem.PS_map_freq_bands_to_20_par_bands_x_num_samples_per_frame_table_ptr];
   I1 = r0;
   r0 = M[r9 + $aac.mem.PS_frequency_border_table_20_par_bands_ptr];
   I5 = r0;

   r0 = $aacdec.PS_NUM_HYBRID_FREQ_BANDS_WHEN_20_PAR_BANDS;
   M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_SUBBAND_LOOP_BOUND] = r0;

   M1 = 0;
   M2 = 1*ADDR_PER_WORD;

   r0 = 1;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

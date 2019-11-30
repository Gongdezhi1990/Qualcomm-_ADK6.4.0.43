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
//    $aacdec.ps_transient_detection
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
.MODULE $M.aacdec.ps_transient_detection;
   .CODESEGMENT AACDEC_PS_TRANSIENT_DETECTION_PM;
   .DATASEGMENT DM;

   $aacdec.ps_transient_detection:

   // push rLink onto stack
   push rLink;

   // first outer-loop iteration calculates Transient Attenuation Factors across the frame
   // for Parameter Bands representing Hybrid Spectrum Sub-subbands
   // and the second outer-loop iteration does the same for Parameter Bands representing the
   // non-hybrid QMF subbands

   call $aacdec.ps_initialise_transient_detection_for_hybrid_freq_bins_flash;

   calculate_transient_attenuation_factors_outer_loop:

      M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_HYBRID_MODE] = r0;

      // for freq_band={0:9(hybrid_freq_bands) || 10:21(remaining_qmf_freq_bands)}
      calculate_input_power_freq_bands_loop:


         //r0 = M[I1, MK1];//r0 = M[I6, MK1];     // parameter_band = map_freq_band_to_20_par_bands_table[freq_band]
         r1 = M[I5, MK1];  // start_k = frequency_border_table_20_par_bands[freq_band]

         // n = 0
         M0 = 0;

         r2 = M[I5,0];
         // if(freq_band < 10)   r2 = end_k = start_k + 1  (hybrid freq_bands are 1 frequency wide)
         //    else r2 = end_k = frequency_border_table_20_par_bands[freq_band+1]

         r3 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_HYBRID_MODE];
         if NZ r2 = r1 + r3, r0 = M[I1,M2];  // r0 = parameter_band = map_freq_band_to_20_par_bands_table[freq_band]

         // store &(P[n=0][parameter_band]) in tmp[]
         r6 = M[r9 + $aac.mem.TMP + $aacdec.PS_INPUT_POWER_MATRIX_BASE_ADDR];
         Words2Addr(r0);
         r6 = r6 + r0;;
         M[r9 + $aac.mem.TMP + $aacdec.PS_INPUT_POWER_MATRIX_ROW_ADDRESS] = r6;

         // skip Transient Detection for Hybrid sub-subband 4,5 as these are combined with 2 and 3
         // and zeroed in the Hybrid Analysis Filterbank
         Null = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_HYBRID_MODE];
         if Z jump transient_detection_x_array_start_subband_assigned;
            Null = r1 - 4;
            if LT jump transient_detection_x_array_start_subband_assigned;
               r1 = r1 - 2;   // start_k = start_k - 2
               r2 = r2 - 2;   // end_k = end_k - 2
         transient_detection_x_array_start_subband_assigned:

         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_INTER_SUBBAND_STRIDE]; // units are addrs

         r0 = r0 * r1 (int);
         r3 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_REAL_BASE_ADDR];
         r3 = r3 + r0;
         r6 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_IMAG_BASE_ADDR];
         r6 = r6 + r0;

         I6 = r3; // I6 -> real({X_SBR[ch=0][k=start_k][l=SBR_tHFAdj+0] | X_ps_hybrid[ch=0][k=start_k][n=0]})
         I4 = r6; // I4 -> imag({X_SBR[ch=0][k=start_k][l=SBR_tHFAdj+0] | X_ps_hybrid[ch=0][k=start_k][n=0]})

         M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_START_FREQ] = r1;
         M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_END_FREQ] = r2;
         r0 = r2 - r1;
         M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_WIDTH] = r0;


         // for n=0:PS_NUM_SAMPLES_PER_FRAME-1,

         calculate_input_power_time_samples_loop:

            // parameter bands 0,1 represent hybrid sub-subbands bands 0,3 and 1,2 respectively - therefore the
            // calculation of P[n=0:31][p_band=0,1] is spread over 2 frequency bands and must be saved in
            // Double Precision form inbetween
            rMAC = 0;
            Null = M1 - 1;
            if GT jump input_power_calculation_rmac_initialised;
               r6 = M[r9 + $aac.mem.TMP + $aacdec.PS_INPUT_POWER_MATRIX_ROW_ADDRESS];
               r0 = M0;
               Words2Addr(r0);
               r6 = r6 + r0;
               // rMAC(47:0) = [P[n][p_band]_MSW; P[n][p_band]_LSW]
               rMAC = M[r6];
               r0 = M[r6 + ($aacdec.PS_NUM_SAMPLES_PER_FRAME * $aacdec.PS_NUM_PAR_BANDS_IN_BASELINE_DECORRELATION*ADDR_PER_WORD)];
               rMAC0 = r0;
            input_power_calculation_rmac_initialised:

            // for k=start_k:end_k-1
            r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_INTER_SUBBAND_STRIDE]; // units are addrs
            r10 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_WIDTH];
            M3 = r0;

            do calculate_input_power_subbands_loop;
               // P[n][p_band] += S_k[n][k] ^ 2
               r3 = M[I6,M3];  // r3 = real(S_k[n][k])

               rMAC = rMAC + r3 * r3,
                r4 = M[I4,M3]; // r4 = imag(S_k[n][k])

               rMAC = rMAC + r4 * r4;
            calculate_input_power_subbands_loop:

            Null = M1 - 1;
            if GT jump end_if_store_input_power_calculation;
               M[r6] = rMAC;  // store P[n][p_band]_MSW
               r0 = rMAC0;
               // store P[n][p_band]_LSW
               M[r6 + ($aacdec.PS_NUM_SAMPLES_PER_FRAME * $aacdec.PS_NUM_PAR_BANDS_IN_BASELINE_DECORRELATION*ADDR_PER_WORD)] = r0;
               jump end_if_freq_band_gt_one_time_sample_loop;
            end_if_store_input_power_calculation:


            // if(freq_band > 1)

               // store P[n][parameter_band] in Single Precision floating-point form
               // r3,rMAC = Mantissa(P[n][parameter_band])
               // r6,r10 = Exponent(P[n][parameter_band])
               r0 = SIGNDET rMAC;
               r3 = DAWTH;
               r10 = r3 - r0;
               rMAC = rMAC LSHIFT r0;
               if Z r10 = 0, r4 = M[I0,M2];  // r4 = Mantissa(ps_power_peak_decay_nrg_prev[n-1][p_band])
               r3 = rMAC;
               r6 = r10;

               // ps_power_peak_decay_nrg[n][parameter_band] = max(ps_power_peak_decay_nrg[n-1][parameter_band] * PS_ALPHA_DECAY, ...
                                                                                                            // ... P[n][parameter_band])
               M3 = -ADDR_PER_WORD;
               // [r3 (Mantissa), r5 (Exponent)] = ps_power_peak_decay_nrg_prev[n-1][p_band] * PS_ALPHA_DECAY
               r4 = r4 * $aacdec.PS_ALPHA_DECAY (frac);
               r0 = SIGNDET r4,
                r5 = M[I0,M3];   // r5 = Exponent(ps_power_peak_decay_nrg_prev[n-1][p_band])
               r5 = r5 - r0,
                r1 = M[I2,M2];   // r1 = Mantissa(power_in_this_par_band[n-1][p_band])
               r4 = r4 LSHIFT r0;
               if Z r5 = 0, r2 = M[I2,M3];   // r2 = Exponent(power_in_this_par_band[n-1][p_band])

               if Z jump end_if_p_peak_decay_nrg_gt_input_power;
                  r0 = 0;
                  r7 = r10 - r5;
                  if Z r0 = rMAC - r4;
                  Null = r0 + r7;
                  if GT jump end_if_p_peak_decay_nrg_gt_input_power;
                     r3 = r4;
                     r6 = r5;
               end_if_p_peak_decay_nrg_gt_input_power:

               r5 = r6,
                M[I0,M2] = r3;   // Mantissa(ps_power_peak_decay_nrg[n][parameter_band])

               // apply smoothing filter to power
               // power_in_this_par_band[n][parameter_band] = power_in_this_par_band[n-1][parameter_band] + ...
                                                             // ... (P[n][parameter_band] - power_in_this_par_band[n-1][parameter_band])*PS_ALPHA_SMOOTH

               // r1 = Mantissa(power_in_this_par_band[n][parameter_band])
               // r2 = Exponent(power_in_this_par_band[n][parameter_band])

               r7 = rMAC;  // r7 = Mantissa(P[n][parameter_band])

               // equalise exponents of power_in_this_par_band[n-1][p_band] and P[n][parameter_band]
               r0 = r10 - r2,
                M[I0,M3] = r5;   // Exponent(ps_power_peak_decay_nrg[n][parameter_band])
               if LT r7 = r7 LSHIFT r0;
               r0 = r2 - r10;
               if LT r1 = r1 LSHIFT r0;
               r1 = r1 * (1.0 - $aacdec.PS_ALPHA_SMOOTH) (frac);
               r7 = r7 * $aacdec.PS_ALPHA_SMOOTH (frac);

               r1 = r1 + r7;
               // r4 = Exponent(power_in_this_par_band[n][parameter_band])
               Null = r2 - r10,
                r5 = M[I3,M2];   // r5 = Mantissa(ps_power_smoothed_peak_decay_diff_nrg_prev[n-1][p_band])
               if LT r2 = r10;

               r0 = SIGNDET r1;
               r2 = r2 - r0;
               r1 = r1 LSHIFT r0,
                r4 = M[I3,M3];   // r4 = Exponent(ps_power_smoothed_peak_decay_diff_nrg_prev[n-1][p_band])
               if Z r2 = 0,
                M[I2,M2] = r1;   // store Mantissa(power_in_this_par_band[n][parameter_band])


               // apply smoothing filter Hsmooth to ps_power_peak_decay_nrg[n][parameter_band]
               // ps_power_smoothed_peak_decay_diff_nrg[n][parameter_band] = Hsmooth * ...
                                                                     // ... (ps_power_peak_decay_nrg[n][parameter_band] - P[n][parameter_band])
               // Hsmooth(z) = a_smooth / (1 + (a_smooth -1).z^-1)

               // equalise exponents of these three
               // [r5,r4] = ps_power_smoothed_peak_decay_diff_nrg_prev[n-1][p_band]
               // [r3,r6] = ps_power_peak_decay_nrg[n][parameter_band]
               // [rMAC,r10] = P[n][p_band]

               // r1 = common exponent
               r1 = r4;
               Null = r1 - r6,
                M[I2,M3] = r2;   // store Exponent(power_in_this_par_band[n][parameter_band])
               if LT r1 = r6;

               // adjust all 3 mantissas to make this their exponent
               r0 = r6 - r1;
               r3 = r3 ASHIFT r0;
               r0 = r4 - r1;
               r5 = r5 ASHIFT r0;
               r0 = r10 - r1;
               rMAC = rMAC ASHIFT r0;

               rMAC = rMAC * -$aacdec.PS_ALPHA_SMOOTH;
               rMAC = rMAC + r3 * $aacdec.PS_ALPHA_SMOOTH;
               rMAC = rMAC + r5 * (1.0 - $aacdec.PS_ALPHA_SMOOTH);
               if Z r1 = 0,
                M[I3,M2] = rMAC; // Mantissa(ps_power_smoothed_peak_decay_diff_nrg[n][p_band])

               r0 = SIGNDET rMAC,
                M[I3,M3] = r1;   // Exponent(ps_power_smoothed_peak_decay_diff_nrg[n][p_band])

               // r5 = Mantissa(ps_power_smoothed_peak_decay_diff_nrg[n][p_band]) normailised to 24-bits
               // r8 = compensated Exponent(ps_power_smoothed_peak_decay_diff_nrg[n][p_band])
               r8 = r1 - r0;
               r5 = rMAC LSHIFT r0;
               if Z r8 = 0;


               // if(PS_TRANSIENT_IMPACT_FACTOR * ps_power_smoothed_peak_decay_diff_nrg[n][p_band] > power_in_this_par_band[n][p_band])
               //    G_transient_ratio[n][p_band] = power_in_this_par_band[n][p_band] / PS_TRANSIENT_IMPACT_FACTOR * ps_power_smoothed_peak_decay_diff_nrg[n][p_band]
               if Z jump no_transient_detected;

                  // rMAC = Mantissa(power_in_this_par_band[n][p_band]) normalised to 24-bits
                  // r4 = compensated Exponent(power_in_this_par_band[n][p_band])
                  rMAC = M[I2,M2];
                  rMAC = rMAC * $aacdec.PS_ONE_OVER_TRANSIENT_IMPACT_FACTOR (frac);
                  r0 = SIGNDET rMAC,
                   r4 = M[I2,M3];
                  r4 = r4 - r0;
                  rMAC = rMAC LSHIFT r0;
                  if Z r4 = 0;

                  r1 = 0;
                  r0 = r4 - r8;
                  if Z r1 = rMAC - r5;
                  Null = r0 + r1;
                  if GE jump no_transient_detected;

                     rMAC = rMAC LSHIFT -2;
                     Div = rMAC / r5;

                     r4 = r4 + 1;
                     M0 = M0 + 1;
                     r4 = r4 - r8;
                     r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_INTER_SUBBAND_STRIDE]; // units are addrs
                     r5 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NUM_FREQ_BINS_PER_SAMPLE];
                     r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_START_FREQ];
                     r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_END_FREQ];
                     r0 = r0 * r2 (int);
                     r3 = I6 - r0;

                     rMAC = DivResult;
                     rMAC = rMAC LSHIFT r4;

                     r4 = I4 - r0;
                     M[I7, MK1] = rMAC;   // store G_transient_ratio[n][p_band]

                     jump g_transient_calculation_divide_complete;

               no_transient_detected:
               // else G_transient_ratio[n][p_band] = 1.0
                  rMAC = 1.0;
                  // store G_transient_ratio[n][parameter_band]
                  M[I7, MK1] = rMAC;

            end_if_freq_band_gt_one_time_sample_loop:

            r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_START_FREQ];
            r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_FREQ_BAND_END_FREQ];
            M0 = M0 + 1;
            r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_INTER_SUBBAND_STRIDE]; // units are addrs
            r0 = r0 * r2 (int);
            r3 = I6 - r0;
            r4 = I4 - r0;

            r5 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NUM_FREQ_BINS_PER_SAMPLE];

            g_transient_calculation_divide_complete:
            Words2Addr(r5);

            Null = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_HYBRID_MODE];
            if NZ jump end_if_crossing_qmf_thfgen_envelope;
               Null = M0 - ($aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj);
               if NZ jump end_if_crossing_qmf_thfgen_envelope;
                  // r3 -> real(X_SBR[ch=0][k=0][l=SBR_tHFAdj+6]))
                  r3 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
                  r3 = r3 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
                  // r4 -> imag(X_SBR[ch=0][k=0][l=SBR_tHFAdj+6]))
                  r4 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
                  r4 = r4 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
                  r5 = 0;
            end_if_crossing_qmf_thfgen_envelope:

            r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_INTER_SUBBAND_STRIDE]; // units are addrs
            r3 = r3 + r5;
            r0 = r0 * r1 (int);
            r4 = r4 + r5;
            I6 = r3 + r0;  // I6 -> real(S_k[ch=0][n+1][k]))
            I4 = r4 + r0;  // I4 -> imag(S_k[ch=0][n+1][k]))

            Null = M0 - $aacdec.PS_NUM_SAMPLES_PER_FRAME;
         if LT jump calculate_input_power_time_samples_loop;

         // if(freq_band > 1)
         Null = M1 - 1;
         if LE jump end_if_freq_band_gt_one_freq_band_loop;
            // increment pointers by one to point to the next parameter_band
            I0 = I0 + (2*ADDR_PER_WORD);   // I0 -> ps_power_peak_decay_nrg_prev[p_band+1]
            I3 = I3 + (2*ADDR_PER_WORD);   // I3 -> ps_power_smoothed_peak_decay_diff_nrg_prev[p_band+1]
            I2 = I2 + (2*ADDR_PER_WORD);   // I2 -> ps_smoothed_input_power_prev[p_band+1]
         end_if_freq_band_gt_one_freq_band_loop:


         M1 = M1 + 1;
         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_SUBBAND_LOOP_BOUND];
         Null = r0 - M1;
      if GT jump calculate_input_power_freq_bands_loop;


      call $aacdec.ps_initialise_transient_detection_for_qmf_freq_bins_flash;


      r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_TRANSIENT_DETECTOR_HYBRID_MODE];
      r0 = r0 - 1;
   if POS jump calculate_transient_attenuation_factors_outer_loop;


   // free temporary memory allocated for double(P[n=0:31][parameter_band=0:19])
   r0 = (2 * $aacdec.PS_NUM_SAMPLES_PER_FRAME * $aacdec.PS_NUM_PAR_BANDS_IN_BASELINE_DECORRELATION);
   call $aacdec.frame_mem_pool_free;

   r8 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_TEMP_R8];

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

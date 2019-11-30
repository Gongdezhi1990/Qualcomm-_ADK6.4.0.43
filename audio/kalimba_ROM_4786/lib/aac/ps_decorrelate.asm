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
//    $aacdec.ps_decorrelate
//
// DESCRIPTION:
//    - Pass the left channel (S_k[n][k]) through the Decorrelator to form the right channel (D_k[n][k])
//    - Left channel S_k[n][k]:
//                Hybrid Section: ps_X_hybrid[ch=0][k=0:PS_NUM_HYBRID_SUB_SUBBANDS-1][n=0:PS_NUM_SAMPLES_PER_FRAME-1]
//                QMF Section:    X_SBR[ch=0][k=PS_NUM_HYBRID_SUBBANDS][l=SBR_tHFAdj+0:SBR_tHFAdj+PS_NUM_SAMPLES_PER_FRAME-1]
//    - left channel  : S_k[n=0:PS_NUM_SAMPLES_PER_FRAME-1][k=0:63+PS_NUM_HYBRID_SUB_SUBBANDS]
//    - right channel : D_k[n][k] = S_k[n][k] * H_k[z] * G_transient_ratio[n][parameter_band]
//
//    - All Hybrid Sub-subbands and QMF bands 3:22 are passed through the cascade of 3 Allpass filters
//    - QMF Subbands 23:34 are passed thorough the LongDelay (14 samples)
//    - QMF Subbands 35:63 are passed throught the ShortDelay (1 sample)
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
.MODULE $M.aacdec.ps_decorrelate;
   .CODESEGMENT AACDEC_PS_DECORRELATE_PM;
   .DATASEGMENT DM;

   $aacdec.ps_decorrelate:

   // push rLink onto stack
   push rLink;

   // First outer_loop iteration passes Hybrid sub-subband signals (derived from QMF subbands 0:2) through Decorrelator
   // Second outer_loop iteration passes non-hybrid QMF subbands 3:63 through Decorrelator

   call $aacdec.ps_initialise_decorrelation_for_hybrid_freq_bins_flash;

   filter_outer_loop:

      M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_HYBRID_MODE] = r0;

      // for freq_band={(0:9)||(10:21)}
      filter_freq_bands_loop:


         r0 = M1;
         Words2Addr(r0);
         r2 = M[r9 + $aac.mem.PS_frequency_border_table_20_par_bands_ptr];
         r2 = r2 + r0;
         r7 = M[r2];  // r7 = start_k = frequency_border_table_20_par_bands[freq_band]

         r1 = M[r2+(1*ADDR_PER_WORD)];// r1 = frequency_border_table_20_par_bands[freq_band+1]
         // if(mixing hybrid-spectrum)   end_k = start_k + 1  (as hybrid freq_bands are 1 frequency wide)
         //    else end_k = frequency_border_table_20_par_bands[freq_band+1]
         r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_HYBRID_MODE];
         if NZ r1 = r7 + r2;
         M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_END_SUBBAND_OF_FREQ_BAND] = r1;


         // for k=start_k:end_k-1
         filter_subbands_loop:

            // if(k <= PS_DECORRELATION_NUM_ALLPASS_BANDS)
            Null = r7 - $aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS;
            if GT jump subband_within_delay_band;

               // load last allpass_feedback_delay_buffer_indice[k][m=0:PS_NUM_ALLPASS_LINKS-1] values
               // from previous frame into tmp[] as starting values for this frame
               r1 = M[r9 + $aac.mem.PS_info_ptr];
               r0 = M[r1 + $aacdec.PS_ALLPASS_FEEDBACK_BUFFER_INDICES + (0*ADDR_PER_WORD)];
               M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_INDICES + (0*ADDR_PER_WORD)] = r0;
               M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_INDICES + (1*ADDR_PER_WORD)] = Null;
               r0 = M[r1 + $aacdec.PS_ALLPASS_FEEDBACK_BUFFER_INDICES + (1*ADDR_PER_WORD)];
               M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_INDICES + (2*ADDR_PER_WORD)] = r0;


               Null = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_HYBRID_MODE];
               if NZ jump initialise_filter_for_hybrid_qmf_subband;
                  // Initialisation for Allpass-chain filtering of next QMF subband

                  r2 = r7 - $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS;

                  r3 = r9 + $aac.mem.PS_prev_frame_last_two_qmf_samples_real;
                  r5 = r9 + $aac.mem.PS_prev_frame_last_two_qmf_samples_imag;

                  // [r4,r6] = S_k[k][n=-2]
                  call load_last_two_samples_of_prev_frame_and_phi_fract_allpass_values;

                  r10 = $aacdec.PS_NUM_ALLPASS_LINKS;
                  r0 = 0;
                  // r2 -> gDecaySlope[k][m=0]
                  r2 = r2 * ($aacdec.PS_NUM_ALLPASS_LINKS*ADDR_PER_WORD) (int);
                  r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_G_DECAY_SLOPE_FILTER_TABLE_BASE_ADDR];
                  r2 = r2 + r1;

                  // load gDecaySlope[k][m=0:2] from flash into tmp[]
                  // for m=0:PS_NUM_ALLPASS_LINKS-1,
                  push r3;
                  r3 = r9 + $aac.mem.TMP + $aacdec.PS_G_DECAY_SLOPE_FILTER_A;
                  do allpass_filter_g_decay_slope_loop;
                     r1 = M[r2 + r0];
                     r1 = r1 ASHIFT (DAWTH-16);
                     M[r3 + r0] = r1;
                     r0 = r0 + ADDR_PER_WORD;
                  allpass_filter_g_decay_slope_loop:
                  pop r3;

                  // r2 = k
                  r2 = r7;

                  jump end_if_subband_within_delay_or_allpass_band_initialisation;
               initialise_filter_for_hybrid_qmf_subband:
               // Initalisation for Allpass-chain filtering of next Hybrid sub-subband

                  // skip Decorrelation for Hybrid sub-subband 4,5 as these are combined with 2 and 3 in the Hybrid Analysis Filterbank
                  // r2 = k_adjusted
                  r2 = r7;
                  r0 = r7 - 4;
                  if LT jump hybrid_sub_subband_index_offset_applied;
                     r1 = r7 - 5;
                     Null = r0 * r1 (int);
                     // if(hybrid_sub_subband = {4|5}) don't pass this hybrid_sub_subband through the Decorrelator
                     if Z jump end_of_filter_subbands_loop_iteration;
                        r2 = r7 - 2;
                  hybrid_sub_subband_index_offset_applied:

                  r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FLASH_TABLES_DM_ADDRESS];
                  Words2Addr(r7);
                  r0 = r7 + r0;
                  I0 = r0;
                  I4 = r0 + ($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS*ADDR_PER_WORD);

                  // [I2,I6] -> phi_fract_hybrid_allpass[m=0][k]
                  r3 = r7 * $aacdec.PS_NUM_ALLPASS_LINKS (int);
                  Addr2Words(r7);
                  r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FLASH_TABLES_DM_ADDRESS];
                  r0 = r3 + r0;
                  I2 = r0 + (2 * $aacdec.PS_NUM_HYBRID_SUB_SUBBANDS*ADDR_PER_WORD);
                  I6 = I2 + ($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS * $aacdec.PS_NUM_ALLPASS_LINKS*ADDR_PER_WORD);

                  r3 = r9 + $aac.mem.PS_prev_frame_last_two_hybrid_samples_real;
                  r5 = r9 + $aac.mem.PS_prev_frame_last_two_hybrid_samples_imag;

                  // [r4,r6] = S_k[k][n=-2]
                  call load_last_two_samples_of_prev_frame_and_phi_fract_allpass_values;

                  // r2 = k_adjusted * PS_HYBRID_SPECTRUM_INTER_FREQ_STRIDE
                  r2 = r2 * $aacdec.PS_NUM_SAMPLES_PER_FRAME (int);

                  jump end_if_subband_within_delay_or_allpass_band_initialisation;
            subband_within_delay_band:

               // Initialisation of delay {z^-14 | z^-1} of next QMF subband within the {long | short} delay band
               Null = r7 - $aacdec.PS_DECORRELATION_SHORT_DELAY_BAND;
               if GE jump subband_within_short_delay_band;
                  // QMF subband is within the Long Delay band (z^-14)

                  // r8 = D_delay_indice = Long_D_Delay_indice
                  r0 = M[r9 + $aac.mem.PS_info_ptr];
                  r8 = M[r0 + $aacdec.PS_DECORRELATION_LONG_DELAY_BAND_BUFFER_INDICE];
                  Words2Addr(r8);

                  // [I2,I6] -> PS_LONG_DELAY_BAND_BUFFER[k][Long_D_Delay_indice]
                  r2 = r7 - ($aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS + 1);
                  r0 = r2 * ($aacdec.PS_DECORRELATION_LONG_DELAY_IN_SAMPLES*ADDR_PER_WORD) (int);
                  r0 = r0 + r8;
                  Addr2Words(r8);
                  r2 = M[r9 + $aac.mem.PS_long_delay_band_buffer_real_ptr];
                  I2 = r2 + r0;
                  r2 = M[r9 + $aac.mem.PS_long_delay_band_buffer_imag_ptr];
                  I6 = r2 + r0;
                  // r2 = k
                  r2 = r7;

                  r0 = $aacdec.PS_DECORRELATION_LONG_DELAY_IN_SAMPLES;
                  M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_DELAY_IN_SAMPLES] = r0;

                  jump end_if_subband_within_delay_or_allpass_band_initialisation;
               subband_within_short_delay_band:
                  // QMF subband is within the Short Delay band (z^-1)

                  // r8 = D_delay_indice = Short_D_Delay_indice
                  r8 = 0;

                  // [I2,I6] -> PS_SHORT_DELAY_BAND_BUFFER[k][Short_D_Delay_indice]
                  r2 = r7 - ($aacdec.PS_DECORRELATION_SHORT_DELAY_BAND);
                  r0 = r2 * ($aacdec.PS_DECORRELATION_SHORT_DELAY_IN_SAMPLES*ADDR_PER_WORD) (int);
                  I2 = r9 + $aac.mem.PS_short_delay_band_buffer_real;
                  I2 = I2 + r0;
                  I6 = r9 + $aac.mem.PS_short_delay_band_buffer_imag;
                  I6 = I6 + r0;
                  // r2 = k
                  r2 = r7;

                  r0 = $aacdec.PS_DECORRELATION_SHORT_DELAY_IN_SAMPLES;
                  M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_DELAY_IN_SAMPLES] = r0;

            end_if_subband_within_delay_or_allpass_band_initialisation:

            // base addresses have been set for either either the Hybrid or QMF spectrum
            Words2Addr(r2);
            r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_REAL_BASE_ADDR];
            I3 = r0 + r2; // I3 -> real(S_k[n=0][k])
            r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SK_IN_IMAG_BASE_ADDR];
            I7 = r0 + r2; // I7 -> imag(S_k[n=0][k])

            r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_DK_OUT_REAL_BASE_ADDR];
            I1 = r0 + r2; // I1 -> real(D_k[n=0][k])
            r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_DK_OUT_IMAG_BASE_ADDR];
            I5 = r0 + r2; // I5 -> imag(D_k[n=0][k])

            // save k
            M2 = r7;

            // for n=0:31,
            M3 = 0;

            filter_time_samples_loop:

               // if(k > PS_DECORRELATION_NUM_ALLPASS_BANDS)
               Null = M2 - $aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS;
               if LE jump subband_is_within_allpass_band;
                  // Hk[z] = z^-14; or z^-1 (in the short_delay_band)

                  r4 = M[I3,0],  // r4 = real(X_SBR[ch=0][k][l=SBR_tHFAdj+n])
                   r5 = M[I7,0]; // r5 = imag(X_SBR[ch=0][k][l=SBR_tHFAdj+n])

                  // allpass_output = PS_{LONG|SHORT}_DELAY_BAND_BUFFER[k][D_delay_indice]
                  r2 = M[I2,0],
                   r3 = M[I6,0];

                  // PS_{LONG|SHORT}_DELAY_BAND_BUFFER[k] = X_SBR[ch=0][k][l=SBR_tHFAdj+n]
                  M[I2, MK1] = r4,
                   M[I6, MK1] = r5;

                  r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_DELAY_IN_SAMPLES];
                  // D_delay_indice += 1, modulo(PS_{LONG|SHORT}_DELAY_NUM_SAMPLES)
                  r8 = r8 + 1;
                  Null = r8 - r0;
                  if GE r8 = 0;
                  if NZ jump end_if_wrap_delay_buffer_indice;
                     Words2Addr(r0);
                     I2 = I2 - r0;
                     I6 = I6 - r0;
                     Addr2Words(r0);
                  end_if_wrap_delay_buffer_indice:

                  jump end_if_subband_within_delay_or_allpass_band_filtering;
               subband_is_within_allpass_band:


               /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
               //                                       __________________________                                                    //
               //   Hk[m][z]                           |                          |  allpass_input_component                          //
               //             ------------------------>| * -a[m]*G_DecaySlope[sb] |----->----------------------¦                      //
               //             |                        |__________________________|                            ¦                      //
               //             |                    _________________________                                   ¦                      //
               //             |                   |                         |                                  ¦                      //
               // AP_input[m]-->[Z^-d(m)]--(+)--->| * Q_fractAllPass[sb][m] |-------------------------------->(+)------> AP_output[m] //
               //                           ¦     |_________________________| allpass_feedback_component            ¦                 //
               //                           ¦                                                                       ¦                 //
               //                           ¦            _________________________                                  ¦                 //
               //                           ¦           |                         |                                 ¦                 //
               //                           -----<------| * a[m]*G_DecaySlope[sb] |<------------[Z^-d(m)]<----------¦                 //
               //                                       |_________________________|                                                   //
               //                                                                                                                     //
               //                                                                                                                     //
               //                                           m = 0           1             2                                           //
               //                                                                                                                     //
               //                       AP_input    Sk[n-2].Qfract[sb]  AP_output[0]   AP_output[1]                                   //
               //                       delay[m]                                                                                      //
               //                       Fs < 32kHz            1             2             3                                           //
               // baseline PS Decoder : Fs >= 32kHz           3             4             5                                           //
               //                                                                                                                     //
               /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


                  // read phi_fract[k]   // pointers set to hybrid or qmf tables
                  r0 = M[I0,0],  // real(phi_fract[k])
                   r1 = M[I4,0];  // imag(phi_fract[k])

                  r0 = r0 ASHIFT (DAWTH-16);
                  r1 = r1 ASHIFT (DAWTH-16);

                  // r2 = real(filter_input[n-2] * phi_fract[k])
                  rMAC = r6 * r0;
                  rMAC = rMAC - r4 * r1;
                  r2 = rMAC;

                  // r3 = imag(filter_input[n-2] * phi_fract[k])
                  rMAC = r6 * r1;
                  rMAC = rMAC + r4 * r0;
                  r3 = rMAC;

                  // for m=0:PS_NUM_ALLPASS_LINKS-1,
                  r10 = $aacdec.PS_NUM_ALLPASS_LINKS;

                  M0 = I3;
                  I3 = r9 + $aac.mem.TMP + $aacdec.PS_G_DECAY_SLOPE_FILTER_A;


                  do allpass_filter_loop;

                     Words2Addr(r10);
                     r10 = r10 + r9;
                     r0 = M[r10 + ($aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_TWO_ADDR - ADDR_PER_WORD)];
                     r4 = M[r10 + ($aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_INDICES - ADDR_PER_WORD)];
                     r10 = r10 - r9;

                     Words2Addr(r4);
                     r0 = r0 + r4;
                     // allpass_feedback_delay_buffer_indice[k][m] += 1, (modulo 3+m)
                     r4 = r4 + ADDR_PER_WORD;
                     r5 = r4 + r10;

                     r10 = r10 + r9;

                     Null = r5 - (6*ADDR_PER_WORD);
                     if Z r4 = 0;
                     Addr2Words(r4);
                     M[r10 + ($aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_INDICES - ADDR_PER_WORD)] = r4;

                     // r1 = real(phi_fract_allpass[k][m])
                     r1 = M[r10 + ($aac.mem.TMP + $aacdec.PS_DECORRELATION_PHI_FRACT_ALLPASS_REAL_SUBBAND_VALUES - ADDR_PER_WORD)];
                     // r8 = imag(phi_fract_allpass[k][m])
                     r8 = M[r10 + ($aac.mem.TMP + $aacdec.PS_DECORRELATION_PHI_FRACT_ALLPASS_IMAG_SUBBAND_VALUES - ADDR_PER_WORD)];
                     r10 = r10 - r9;
                     Addr2Words(r10);

                     r5 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_SIZE];
                     Words2Addr(r5);
                     // r7 = ps_{hybrid|qmf}_allpass_feedback_buffer_real[k]
                     r7 = M[r0];
                     // r6 = ps_{hybrid|qmf}_allpass_feedback_buffer_imag[k]
                     r6 = M[r0 + r5];

                     // rMAC = real(allpass_feedback_component) = real(ps_{hybrid|qmf}_allpass_feedback_buffer[k] * phi_fract_allpass[k][m])
                     rMAC = r7 * r1,
                      r5 = M[I3, MK1];  // r5 = gDecaySlope_filter_a[k][m]
                     rMAC = rMAC - r6 * r8;
                     // rMAC = real(allpass_output) = real(allpass_feedback_component - inter_allpass_sample * gDecaySlope_filter_a[k][m])
                     rMAC = rMAC - r2 * r5;
                     r4 = rMAC * r5 (frac);
                     M[r0] = r4 + r2;  // real(allpass_output * gDecaySlope_filter_a[k][m] + inter_allpass_sample)

                     r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_SIZE];
                     Words2Addr(r2);
                     r0 = r0 + r2;

                     r2 = rMAC;

                     // rMAC = imag(allpass_feedback_component) = imag(ps_{hybrid|qmf}_allpass_feedback_buffer[k] * phi_fract_allpass[k][m])

                     rMAC = r7 * r8;
                     rMAC = rMAC + r6 * r1;
                     // rMAC = imag(allpass_output) = imag(allpass_feedback_component - inter_allpass_sample * gDecaySlope_filter_a[k][m])
                     rMAC = rMAC - r3 * r5;
                     r8 = rMAC * r5 (frac);

                     M[r0] = r8 + r3;  // imag(allpass_output * gDecaySlope_filter_a[k][m] + inter_allpass_sample)

                     r3 = rMAC;

                  allpass_filter_loop:


                  // restore I3
                  I3 = M0;

                  r0 = M[I3,0],  // r0 = real(S_k[(n+1)-1][k])
                   r1 = M[I7,0]; // r1 = imag(S_k[(n+1)-1][k])

                  // r6 = real(S_k[(n+1)-2][k])
                  r6 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_REAL];
                  // r4 = imag(S_k[(n+1)-2][k])
                  r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_IMAG];

                  M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_REAL] = r0;
                  M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_IMAG] = r1;


               end_if_subband_within_delay_or_allpass_band_filtering:


               // Apply Transient Attenuation

               // r0 = freq_band
               r0 = M1;
               Words2Addr(r0);
               r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_GAIN_TRANSIENT_RATIO_ADDR];
               // r0 = parameter_band
               r5 = M[r9 + $aac.mem.PS_map_freq_bands_to_20_par_bands_x_num_samples_per_frame_table_ptr];
               r0 = M[r5 + r0];

               // r0 = G_transient_ratio[n][parameter_band]
               r0 = r0 + M3;
               Words2Addr(r0);

               r5 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NUM_FREQ_BINS_PER_SAMPLE];
               Words2Addr(r5);
               r0 = M[r0 + r1];

                // r2 = real(D_k[n][k]) = real(S_k[n][k] * H_k[z]) * G_transient_ratio[n][parameter_band]
               r2 = r2 * r0 (frac);
               // r3 = imag(D_k[n][k]) = imag(S_k[n][k] * H_k[z]) * G_transient_ratio[n][parameter_band]
               r3 = r3 * r0 (frac);

               M0 = r5;
               I3 = I3 + r5,
                M[I1,M0] = r2;
               I7 = I7 + r5,
                M[I5,M0] = r3;


               // memory organisation of X_SBR[ch=1][][] is as follows

               // X_sbr_other_real = real(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+0:SBR_tHFAdj+5])
               // fmp_remains = real(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+6:SBR_tHFAdj+PS_NUM_SAMPLES_PER_FRAME-1])

               // X_sbr_other_imag = imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+0:SBR_tHFAdj+5])
               // overlap_add_right = imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+6:SBR_tHFAdj+14])
               // x_input_buffer_right = imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+15:SBR_tHFAdj+19])
               // fmp_remains[((PS_NUM_SAMPLES_PER_FRAME-6)*64):(26*64 + 12*64 - 1)] = ...
                                                      // imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+20:SBR_tHFAdj+PS_NUM_SAMPLES_PER_FRAME-1])
               r0 = M2;
               Words2Addr(r0);

               Null = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_HYBRID_MODE];
               if NZ jump end_if_crossing_xsbr_memory_partition;
                  Null = M3 - ($aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj - 1);
                  if NZ jump not_crossing_xsbr_thfgen_partition;
                     // I1 -> real(X_sbr[ch=1][k][l=SBR_tHFAdj+6]) = real(D_k[n=6][k])
                     r1 = M[r9 + $aac.mem.PS_fmp_remains_ptr];
                     I1 = r1 + r0;
                     // I5 -> imag(X_sbr[ch=1][k][l=SBR_tHFAdj+6]) = imag(D_k[n=6][k])
                     r1 = M[r9 + $aac.mem.OVERLAP_ADD_RIGHT_PTR];
                     I5 = r1 + r0;
                     jump end_if_crossing_xsbr_memory_partition;
                  not_crossing_xsbr_thfgen_partition:
                  Null = M3 - (($aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj) + ((($aacdec.CORE_AAC_AUDIO_FRAME_OUTPUT_SIZE_IN_WORDS / 2) + $aacdec.X_SBR_WIDTH) / $aacdec.X_SBR_WIDTH) - 1);
                  if NZ jump not_crossing_xsbr_thfgen_plus_16_partition;
                     // I5 -> imag(X_sbr[ch=1][k][l=SBR_tHFAdj+15]) = imag(D_k[n=15][k])
                     r1 = M[r9 + $aac.mem.SBR_x_input_buffer_right_ptr];
                     I5 = r1 + r0;
                     jump end_if_crossing_xsbr_memory_partition;
                  not_crossing_xsbr_thfgen_plus_16_partition:
                  Null = M3 - (($aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj) + ((($aacdec.CORE_AAC_AUDIO_FRAME_OUTPUT_SIZE_IN_WORDS / 2) + $aacdec.X_SBR_WIDTH) / $aacdec.X_SBR_WIDTH) //...
                                                                                          + ($aacdec.X_INPUT_BUFFER_LENGTH / $aacdec.X_SBR_WIDTH) - 1);
                  if NZ jump end_if_crossing_xsbr_memory_partition;
                     // I5 -> imag(X_sbr[ch=1][k][l=SBR_tHFAdj+20]) = imag(D_k[n=20][k])

                     r1 = M[r9 + $aac.mem.PS_fmp_remains_ptr];
                     r1 = r1 + r0;
                     I5 = r1 + ($aacdec.X_SBR_WIDTH * 26*ADDR_PER_WORD);
               end_if_crossing_xsbr_memory_partition:


               M3 = M3 + 1;
               Null = M3 - $aacdec.PS_NUM_SAMPLES_PER_FRAME;
            if LT jump filter_time_samples_loop;

            // restore k
            r7 = M2;

            // if(just processed a subband within the Allpass band)
            Null = r7 - $aacdec.PS_DECORRELATION_NUM_ALLPASS_BANDS;
            if GT jump end_of_filter_subbands_loop_iteration;

               // ps_prev_frame_last_two_samples[k][1] = {ps_X_hybrid¦¦X_SBR}[ch=0][k][n=31+SBR_tHFAdj]
               r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NON_HYBRID_QMF_CHANNEL_NUMBER];
               r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_LAST_TWO_SAMPLES_BUFFER_REAL_ADDR];
               r1 = r1 * (2*ADDR_PER_WORD) (int);
               r6 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_LAST_TWO_SAMPLES_BUFFER_IMAG_ADDR];
               r4 = r4 + r1;
               r6 = r6 + r1;
               r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_REAL];
               r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_IMAG];
               M[r4 + (1*ADDR_PER_WORD)] = r2;   // real(filter_input[k][n=-1]) in next frame
               M[r6 + (1*ADDR_PER_WORD)] = r1;   // imag(filter_input[k][n=-1]) in next frame

               // ps_prev_frame_last_two_samples[k][0] = {ps_X_hybrid¦¦X_SBR}[ch=0][k][n=30+SBR_tHFAdj]
               r0 = r5 * 2 (int);
               r3 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ZERO_ADDR];
               I3 = I3 - r0;
               I7 = I7 - r0;
               r0 = M[I3,0],
                r2 = M[I7,0];
               r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ONE_ADDR];
               M[r4] = r0;  // real(filter_input[k][n=-2]) in next frame
               M[r6] = r2;  // imag(filter_input[k][n=-2]) in next frame

               // increment pointers to {Hybrid¦¦QMF}_allpass_feedback_buffer to point to the next successive subband
               r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_TWO_ADDR];
               r3 = r3 + (3*ADDR_PER_WORD);
               r1 = r1 + (4*ADDR_PER_WORD);
               r2 = r2 + (5*ADDR_PER_WORD);
               M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ZERO_ADDR] = r3;
               M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_ONE_ADDR] = r1;
               M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_M_EQS_TWO_ADDR] = r2;

               // increment pointers to ps_phi_fract[] and phi_fract_allpass[][] to point to the next successive subband
               I0 = I0 + MK1;
               I4 = I4 + MK1;
            end_of_filter_subbands_loop_iteration:

            r7 = r7 + 1;
            r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_END_SUBBAND_OF_FREQ_BAND];
            Null = r7 - r0;
         if LT jump filter_subbands_loop;

         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SUBBAND_LOOP_BOUND];
         M1 = M1 + 1;
         Null = r0 - M1;
      if GT jump filter_freq_bands_loop;

      // extend frequency_bands loop bound to process remaining (non-hybrid) QMF subbands
      r0 = $aacdec.PS_NUM_FREQ_BANDS_WHEN_20_PAR_BANDS;
      M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_SUBBAND_LOOP_BOUND] = r0;

      call $aacdec.ps_initialise_decorrelation_for_qmf_freq_bins_flash;


      r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_HYBRID_MODE];
      r0 = r0 - 1;
   if POS jump filter_outer_loop;


   // save allpass_feedback_buffer and long_delay_band indices (for all subbands in the Allpass band and LongDelay band respectively)
   // for beginning of Allpass filtering in the next frame
   r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_INDICES + (0*ADDR_PER_WORD)];
   r2 = M[r9 + $aac.mem.PS_info_ptr];
   M[r2 + $aacdec.PS_ALLPASS_FEEDBACK_BUFFER_INDICES + (0*ADDR_PER_WORD)] = r0;
   r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_ALLPASS_FEEDBACK_BUFFER_INDICES + (2*ADDR_PER_WORD)];
   M[r2 + $aacdec.PS_ALLPASS_FEEDBACK_BUFFER_INDICES + (1*ADDR_PER_WORD)] = r0;

   r0 = M[r2 + $aacdec.PS_DECORRELATION_LONG_DELAY_BAND_BUFFER_INDICE];
   r0 = r0 + 4;
   r1 = $aacdec.PS_DECORRELATION_LONG_DELAY_IN_SAMPLES;
   r1 = r0 - r1;
   if GE r0 = r1;
   M[r2 + $aacdec.PS_DECORRELATION_LONG_DELAY_BAND_BUFFER_INDICE] = r0;


   r8 = M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_TEMP_R8];

   // pop rLink from stack
   jump $pop_rLink_and_rts;





// ********************************************************
//  DESCRIPTION:
//       common routine used for initialising frequency band
//       loop for both hybrid and qmf subbands
//
//  INPUTS:
//          - r2
//          - r3
//          - r5
//
//  OUTPUTS:
//          - r4, r6
//
// TRASHED REGISTERS:
//    - r0, r1, r4, r10
//
// ********************************************************


   load_last_two_samples_of_prev_frame_and_phi_fract_allpass_values:

   r0 = r2 * (2*ADDR_PER_WORD) (int);

   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_NON_HYBRID_QMF_CHANNEL_NUMBER] = r2;

   r6 = M[r3 + r0];     // real(filter_input[k][n=-2])
   r4 = M[r5 + r0];     // imag(filter_input[k][n=-2])
   r0 = r0 + (1*ADDR_PER_WORD);
   r10 = $aacdec.PS_NUM_ALLPASS_LINKS;
   r1 = M[r3 + r0];  // real(filter_input[k][n=-1])
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_REAL] = r1;
   r1 = M[r5 + r0];  // imag(filter_input[k][n=-1])
   M[r9 + $aac.mem.TMP + $aacdec.PS_DECORRELATION_FILTER_INPUT_N_MINUS_ONE_IMAG] = r1;

   // load phi_fract_{hybrid¦¦qmf}_allpass[m][k] values from Flash
   do copy_phi_fract_allpass_flash_values_loop;
      r0 = M[I2, MK1],
       r1 = M[I6, MK1];
      r0 = r0 ASHIFT (DAWTH-16);
      r1 = r1 ASHIFT (DAWTH-16);
      Words2Addr(r10);
      r10 = r10 + r9;
      M[r10 + ($aac.mem.TMP + $aacdec.PS_DECORRELATION_PHI_FRACT_ALLPASS_REAL_SUBBAND_VALUES - (1*ADDR_PER_WORD))] = r0;
      M[r10 + ($aac.mem.TMP + $aacdec.PS_DECORRELATION_PHI_FRACT_ALLPASS_IMAG_SUBBAND_VALUES - (1*ADDR_PER_WORD))] = r1;
      r10 = r10 - r9;
      Addr2Words(r10);
      nop;
   copy_phi_fract_allpass_flash_values_loop:
   rts;

.ENDMODULE;

#endif

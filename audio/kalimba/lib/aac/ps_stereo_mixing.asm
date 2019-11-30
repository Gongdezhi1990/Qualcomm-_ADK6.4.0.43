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
//    $aacdec.ps_stereo_mixing
//
// DESCRIPTION:
//    - Mix the left and right channels
//    - left channel  : S_k[n=0:PS_NUM_SAMPLES_PER_FRAME][k=0:63+PS_NUM_HYBRID_SUBSUBANDS]
//    - right channel : D_k[n=0:PS_NUM_SAMPLES_PER_FRAME][k=0:63+PS_NUM_HYBRID_SUBSUBANDS]
//
//    - according to inter-channel coherence (insert phase-delay before mixing) (PS_ICC[env=0:PS_NUM_ENV-1][parameter_band=0:19])
//    - and inter-channel difference (scaling before mixing) (PS_IDD[env=0:PS_NUM_ENV-1][parameter_band=0:19])
//    - quantisation levels aquired from the PS bitstream data
//
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
.MODULE $M.aacdec.ps_stereo_mixing;
   .CODESEGMENT AACDEC_PS_STEREO_MIXING_PM;
   .DATASEGMENT DM;

   $aacdec.ps_stereo_mixing:

   // push rLink onto stack
   push rLink;

   // First outer_loop iteration mixes Hybrid sub-subband signals (derived from QMF subbands 0:2) together
   // Second outer_loop iteration mixes non-hybrid QMF subbands 3:63 together
   call $aacdec.ps_initialise_stereo_mixing_for_hybrid_freq_bins_flash;

   mixing_outer_loop:


      M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_HYBRID_MODE] = r0;

      // for freq_band={(0:9)||(10:21)}
      mixing_freq_bands_loop:

         r0 = M1;
         Words2Addr(r0);
         r2 = M[r9 + $aac.mem.PS_frequency_border_table_20_par_bands_ptr];
         r2 = r2 + r0;

         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SK_REAL_BASE_ADDR];
         r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DK_REAL_BASE_ADDR];
         r3 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SK_IMAG_BASE_ADDR];
         r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DK_IMAG_BASE_ADDR];
         I1 = r0; // I1 -> {real(ps_X_hybrid[ch=0][k=0][n=0] || real(X_SBR[ch=0][k=0][l=SBR_tHFAdj+0])}
         I4 = r1; // I4 -> {real(ps_X_hybrid[ch=1][k=0][n=0] || real(X_SBR[ch=1][k=0][l=SBR_tHFAdj+0])}
         I5 = r3; // I5 -> {imag(ps_X_hybrid[ch=0][k=0][n=0] || imag(X_SBR[ch=0][k=0][l=SBR_tHFAdj+0])}
         I2 = r4; // I2 -> {imag(ps_X_hybrid[ch=1][k=0][n=0] || imag(X_SBR[ch=1][k=0][l=SBR_tHFAdj+0])}

         r0 = M[r2];  // r0 = start_k = frequency_border_table_20_par_bands[freq_band]
         M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_FREQ_BAND_START_FREQ] = r0;

         r1 = M[r2+(1*ADDR_PER_WORD)];// r1 = frequency_border_table_20_par_bands[freq_band+1]
         // if(mixing hybrid-spectrum)   end_k = start_k + 1  (as hybrid freq_bands are 1 frequency wide)
         //    else end_k = frequency_border_table_20_par_bands[freq_band+1]
         r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_HYBRID_MODE];
         if NZ r1 = r0 + r2;
         M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_END_SUBBAND_OF_FREQ_BAND] = r1;

         // adjusted when mixing hybrid spectrum so that storage is not allocated to the zeroed hybrid sub-subbands (4:5)
         M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_ADJUSTED_END_SUBBAND_OF_FREQ_BAND] = r1;

         // for envelope=0:max(PS_NUM_ENV-1,0),
         // r8 = envelope
         r8 = 0;

         mixing_time_envelope_loop:

            r0 = M1;
            Words2Addr(r0);
            r1 = M[r9 + $aac.mem.PS_map_freq_bands_to_20_par_bands_table_ptr];
            r0 = r1 + r0;
            // r1 = envelope * $aacdec.PS_MAX_NUM_PARAMETERS
            r1 = r8 * $aacdec.PS_MAX_NUM_PARAMETERS (int);
            // r0 = parameter_band
            r0 = M[r0];
            r0 = r0 + r1;
            Words2Addr(r0);

            r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_IID_ZERO_SCALE_FACTOR_TABLE_POINTER];
            // r1 = PS_IID_INDEX[envelope][paramter_band]
            r2 = M[r9 + $aac.mem.PS_info_ptr];
            r2 = r2 + r0;
            r1 = M[r2 + $aacdec.PS_IID_INDEX];
            Words2Addr(r1);

            // r10 = PS_ICC_INDEX[envelope][parameter_band]
            r10 = M[r2 + $aacdec.PS_ICC_INDEX];
            Words2Addr(r10);

            // r3 = c1 = sqrt(2.0 / (1.0 + 10^iid_quantisation_level[parameter_band][envelope])) / sqrt(2)
            r3 = M[r4 + r1];
            r2 = r4 - r1;

            // r5 = cos(alpha_angle[envelope][parameter_band])
            r0 = M[r9 + $aac.mem.PS_cos_alpha_table_ptr];
            r5 = M[r0 + r10];

            // r2 = c2 = sqrt(2.0 / (1.0 + 10^-iid_quantisation_level[parameter_band][envelope])) / sqrt(2)
            r2 = M[r2];

            // r0 = scale_factor_iid / sqrt(2) = c1 - c2
            r0 = r3 - r2;

            // r6 = sin(alpha_angle[envelope][parameter_band])
            r6 = M[r9 + $aac.mem.PS_sin_alpha_table_ptr];
            r6 = M[r6 + r10];
            // r4 = alpha_angle[envelope][parameter_band] / 2
            r4 = M[r9 + $aac.mem.PS_alpha_angle_table_ptr];
            r4 = M[r4 + r10];

            // r0 = beta_angle[envelope][parameter_band] = (scale_factor_iid * alpha_angle[envelope][parameter_band]) / 2
            r0 = r0 * r4 (frac);

            // r0 = beta / 2 (in radians)
            r0 = r0 * $aacdec.PS_STEREO_MIXING_ONE_OVER_PI (frac);

            // save c1 and c2
            M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SCALE_FACTOR_ONE] = r3;
            M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SCALE_FACTOR_TWO] = r2;

            // extract sign bit of beta
            r7 = r0 AND MININT;

            // r1 = sin(beta/2)
            call $math.sin;

            // r10 = cos(beta) = -((2.0*(sin(beta/2)^2)) - 1.0)
            r0 = r1 * r1 (frac);
            r0 = r0 ASHIFT 1;
            r10 = r0 - 1.0;

            // r1 = sin(|beta|) = sqrt(1.0 - (cos(beta)^2))
            r0 = r10 * r10 (frac);
            r0 = r0 - $aacdec.PS_ONE_SQUARED_24_BIT_FRACTIONAL;
            r0 = -r0;
            call $math.sqrt;

            // if((beta/2) < 0) r1 = -sin(|beta|) = sin(beta)
            Null = r7;
            if NZ r1 = -r1;

            // scaling done to ensure h## values do not exceed 24-bit fractional bounds
            r10 = r10 * -0.5 (frac);   // r10 = -0.5 * cos(beta)
            r1 = r1 * 0.5 (frac);      // r1 = 0.5 * sin(beta)

            // r0 = cos(alpha)*cos(beta)
            r0 = r5 * r10 (frac);
            // r2 = sin(alpha)*sin(beta)
            r2 = r6 * r1 (frac);
            // r3 = sin(alpha)*cos(beta)
            r3 = r6 * r10 (frac);
            // r4 = sin(beta)*cos(alpha)
            r4 = r5 * r1 (frac);

            r6 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SCALE_FACTOR_ONE];  // r6 = c1
            r7 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_SCALE_FACTOR_TWO];  // r7 = c2

            // r1 = h11 = (cos(alpha)*cos(beta) - sin(alpha)*sin(beta)) * c2
            r1 = r0 - r2;
            r1 = r1 * r7 (frac);

            // r0 = h12 = (cos(alpha)*cos(beta) + sin(alpha)*sin(beta)) * c1
            r0 = r0 + r2;
            r0 = r0 * r6 (frac);

            // r2 = h21 = (sin(alpha)*cos(beta) + sin(beta)*cos(alpha)) * c2
            r2 = r3 + r4;
            r2 = r2 * r7 (frac);

            // r3 = h22 = (sin(beta)*cos(alpha) - sin(alpha)*cos(beta)) * c1
            r3 = r4 - r3;
            r3 = r3 * r6 (frac);

            // r5 = envelope_length = max(PS_BORDER_POSITION[envelope+1] - PS_BORDER_POSITION[envelope], 1)
            Words2Addr(r8);
            r5 = M[r9 + $aac.mem.PS_info_ptr];
            r5 = r5 + r8;
            r4 = M[r5 + ($aacdec.PS_BORDER_POSITION)];
            r5 = M[r5 + ($aacdec.PS_BORDER_POSITION + (1*ADDR_PER_WORD))];
            r6 = ADDR_PER_WORD;
            r5 = r5 - r4;
            if LT r5 = r6;
            Words2Addr(r5);

            // r5 = (int)(1 / envelope_length)
            r10 = M[r9 + $aac.mem.SBR_one_over_x_ptr];
            r10 = r10 - (1*ADDR_PER_WORD);
            r5 = M[r10 + r5];

            r10 = M1;   // r10 = freq_band
            Words2Addr(r10);
            M3 = r4;    // M3 = PS_BORDER_POSITION[envelope]

            // rMAC = h11[env-1] = ps_h11_previous_envelope[freq_band][env]
            r4 = r9 + $aac.mem.PS_h11_previous_envelope;
            rMAC = M[r4 + r10];
            // ps_h11_previous_envelope[freq_band][env+1] = h11[env]
            M[r4 + r10] = r1;

            // delta_H11[env] = (h11[env] - h11[env-1]) * (1/envelope_length)
            r1 = r1 - rMAC;
            r1 = r1 * r5 (frac),
             r4 = M[I3,0];  // r4 = h12[env-1] = ps_h12_previous_envelope[freq_band][env]
            r1 = r1 ASHIFT 1;
            r1 = r1 * $aacdec.PS_STEREO_MIXING_SQTR_2_OVER_TWO (frac);
            M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DELTA_H11] = r1;

            // r1 = H11[n=PS_BORDER_POSITION[envelope]] = h11[env-1] = ps_h11_previous_envelope[freq_band][env]
            r1 = rMAC ASHIFT 1;  // compensate for earlier scaling done on h11[env-1]


            // delta_H12[env] = (h12[env] - h12[env-1]) * (1/envelope_length)
            r0 = r0 - r4,
             M[I3,0] = r0; // ps_h12_previous_envelope[freq_band][env+1] = h12[env]
            r0 = r0 * r5 (frac),
             rMAC = M[I6,0];  // rMAC = h21[env-1] = ps_h21_previous_envelope[freq_band][env]
            r0 = r0 ASHIFT 1;
            r0 = r0 * $aacdec.PS_STEREO_MIXING_SQTR_2_OVER_TWO (frac);
            M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DELTA_H12] = r0;

            // r0 = H12[n=PS_BORDER_POSITION[envelope]] = h12[env-1] = ps_h12_previuos_envelope[freq_band][env]
            r0 = r4 ASHIFT 1; // compensate for earlier scaling done on h12[env-1]


            // delta_H21[env] = (h21[env] - h21[env-1]) * (1/envelope_length)
            r2 = r2 - rMAC,
             M[I6,0] = r2; // ps_h21_previous_envelope[freq_band][env+1] = h21[env]
            r2 = r2 * r5 (frac),
             r4 = M[I7,0];  // r4 = h22[env-1] = ps_h22_previous_envelope[freq_band][env]
            r2 = r2 ASHIFT 1;
            r2 = r2 * $aacdec.PS_STEREO_MIXING_SQTR_2_OVER_TWO (frac);
            M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DELTA_H21] = r2;

            // r2 = H21[n=PS_BORDER_POSITION[envelope]] = h21[env-1] = ps_h21_previous_envelope[freq_band][env]
            r2 = rMAC ASHIFT 1;  // compensate for earlier scaling done on h21[env-1]


            // delta_H22[env] = (h22[env] - h22[env-1]) * (1/envelope_length)
            r3 = r3 - r4,
             M[I7,0] = r3; // ps_h22_previous_envelope[freq_band][env+1] = h22[env]
            r3 = r3 * r5 (frac);
            r3 = r3 ASHIFT 1;
            r3 = r3 * $aacdec.PS_STEREO_MIXING_SQTR_2_OVER_TWO (frac);
            M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DELTA_H22] = r3;

            // r3 = H22[n=PS_BORDER_POSITION[envelope]] = h22[env-1] = ps_h22_previous_envelope[freq_band][env]
            r3 = r4 ASHIFT 1; // compensate for earlier scaling done on h22[env-1]

            // store first time-sample of next envelope in tmp[]
            r5 = M[r9 + $aac.mem.PS_info_ptr];
            r5 = r5 + r8;
            r5 = M[r5 + ($aacdec.PS_BORDER_POSITION + (1*ADDR_PER_WORD))];
            M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_BORDER_END] = r5;

            rMAC = $aacdec.PS_STEREO_MIXING_SQTR_2_OVER_TWO;
            Addr2Words(r8);
            M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_ENVELOPE] = r8;
            // both delta_H## and H## are scaled by sqrt(2)/2 to ensure H##[n] does not exceed 24-bit fractional bounds
            r0 = r0 * rMAC (frac);  // H12[n=PS_BORDER_POSITION[envelope]] *= sqrt(2)/2
            r6 = r1 * rMAC (frac);  // H11[n=PS_BORDER_POSITION[envelope]] *= sqrt(2)/2
            r7 = r2 * rMAC (frac);  // H21[n=PS_BORDER_POSITION[envelope]] *= sqrt(2)/2
            r8 = r3 * rMAC (frac);  // H22[n=PS_BORDER_POSITION[envelope]] *= sqrt(2)/2

            r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_INTER_SUBBAND_STRIDE];
            M2 = r2;


            // for n=PS_BORDER_POSITION[envelope]:PS_BORDER_POSITION[envelope+1]-1,

            mixing_time_samples_loop:

               // memory organisation of X_SBR[ch=1][][] is as follows

               // X_sbr_other_real = real(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+0:SBR_tHFAdj+5])
               // fmp_remains = real(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+6:SBR_tHFAdj+PS_NUM_SAMPLES_PER_FRAME-1])

               // X_sbr_other_imag = imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+0:SBR_tHFAdj+5])
               // overlap_add_right = imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+6:SBR_tHFAdj+14])
               // x_input_buffer_right = imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+15:SBR_tHFAdj+19])
               // fmp_remains[((PS_NUM_SAMPLES_PER_FRAME-6)*64):(26*64 + 12*64 - 1)] = ...
                                                      // imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+20:SBR_tHFAdj+PS_NUM_SAMPLES_PER_FRAME-1])

               Null = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_HYBRID_MODE];
               if NZ jump end_if_crossing_xsbr_memory_partition;
                  Null = M3 - ($aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj);
                  if NZ jump not_crossing_xsbr_thfgen_partition;
                     // I4 -> real(X_sbr[ch=1][k=0][l=SBR_tHFGen]) = real(D_k[n=SBR_tHFGen-SBR_tHFAdj][k=0])
                     r5 = M[r9 + $aac.mem.PS_fmp_remains_ptr];
                     I4 = r5;
                     // I2 -> imag(X_sbr[ch=1][k=0][l=SBR_tHFGen]) = imag(D_k[n=SBR_tHFGen-SBR_tHFAdj][k=0])
                     r5 = M[r9 + $aac.mem.OVERLAP_ADD_RIGHT_PTR];
                     I2 = r5;
                     jump end_if_crossing_xsbr_memory_partition;
                  not_crossing_xsbr_thfgen_partition:
                  Null = M3 - (($aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj) + ((($aacdec.CORE_AAC_AUDIO_FRAME_OUTPUT_SIZE_IN_WORDS / 2) + $aacdec.X_SBR_WIDTH) / $aacdec.X_SBR_WIDTH));
                  if NZ jump not_crossing_xsbr_thfgen_plus_16_partition;
                     // I2 -> imag(X_sbr[ch=1][k=0][l=SBR_tHFGen+15]) = imag(D_k[n=SBR_tHFGen-SBR_tHFAdj+15][k=0])
                     r5 = M[r9 + $aac.mem.SBR_x_input_buffer_right_ptr];
                     I2 = r5;
                     jump end_if_crossing_xsbr_memory_partition;
                  not_crossing_xsbr_thfgen_plus_16_partition:
                  Null = M3 - (($aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj) + ((($aacdec.CORE_AAC_AUDIO_FRAME_OUTPUT_SIZE_IN_WORDS / 2) + $aacdec.X_SBR_WIDTH) / $aacdec.X_SBR_WIDTH) //...
                                                                                          + ($aacdec.X_INPUT_BUFFER_LENGTH / $aacdec.X_SBR_WIDTH));
                  if NZ jump end_if_crossing_xsbr_memory_partition;
                     // I2 -> imag(X_sbr[ch=1][k=0][l=SBR_tHFGen+15+5]) = imag(D_k[n=SBR_tHFGen-SBR_tHFAdj+15+5][k=0])
                     r5 = M[r9 + $aac.mem.PS_fmp_remains_ptr];
                     I2 = r5 + ($aacdec.X_SBR_WIDTH * 26 * ADDR_PER_WORD);
               end_if_crossing_xsbr_memory_partition:

               // r5 = start_k
               r5 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_FREQ_BAND_START_FREQ];

               // Hybrid sub-subbands (4:5) are skipped as they were zeroed during Hybrid Analysis
               // thus adjust the Hybrid sub-subband index used to calculate pointers to Hybrid spectrum
               // as no storage is assigned to these sub-subbands
               Null = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_HYBRID_MODE];
               if Z jump freq_band_start_freq_assigned;
                  Null = r5 - 6;
                  if LT jump freq_band_start_freq_assigned;
                     r5 = r5 - 2;
                     r4 = r5 + 1;
                     M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_ADJUSTED_END_SUBBAND_OF_FREQ_BAND] = r4;
               freq_band_start_freq_assigned:

               r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_INTER_SUBBAND_STRIDE];
               r4 = r4 * r5 (int);
               I1 = I1 + r4;  // {real(ps_X_hybrid[ch=0][k=start_k][n] || real(X_SBR[ch=0][k=start_k][l=SBR_tHFAdj+n])}
               I4 = I4 + r4;  // {imag(ps_X_hybrid[ch=0][k=start_k][n] || imag(X_SBR[ch=0][k=start_k][l=SBR_tHFAdj+n])}
               I2 = I2 + r4;  // {imag(ps_X_hybrid[ch=1][k=start_k][n] || imag(X_SBR[ch=1][k=start_k][l=SBR_tHFAdj+n])}
               I5 = I5 + r4;  // {real(ps_X_hybrid[ch=1][k=start_k][n] || real(X_SBR[ch=1][k=start_k][l=SBR_tHFAdj+n])}

               // H11[n] = H11[n-1] + delta_H11
               r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DELTA_H11];
               r6 = r6 + r4;
               // H12[n] = H12[n-1] + delta_H12
               r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DELTA_H12];
               r0 = r0 + r4;
               // H21[n] = H21[n-1] + delta_H21
               r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DELTA_H21];
               r7 = r7 + r4;
               // H22[n] = H22[n-1] + delta_H22
               r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_DELTA_H22];
               r8 = r8 + r4;

               r10 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_END_SUBBAND_OF_FREQ_BAND];
               r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_FREQ_BAND_START_FREQ];
               r10 = r10 - r4;

               r4 = M[I1,0],  // r4 = real(S_k[n][k])
                r5 = M[I4,0];  // r5 = real(D_k[n][k])

               // for k=start_k:end_k-1

               do mixing_subbands_loop;

                  // real(S_k[n][k]) = H11*real(S_k[n][k]) + H21*real(D_k[n][k])
                  rMAC = r6 * r4,
                   r3 = M[I5,0];  // r3 = imag(S_k[n][k])
                  rMAC = rMAC + r7 * r5,
                   r2 = M[I2,0];  // r2 = imag(D_k[n][k])

                  // real(D_k[n][k]) = H12*real(S_k[n][k]) + H22*real(D_k[n][k])
                  rMAC = r0 * r4,
                   M[I1,M2] = rMAC;
                  rMAC = rMAC + r8 * r5;

                  // imag(S_k[n][k]) = H11*imag(S_k[n][k]) + H21*imag(D_k[n][k])
                  rMAC = r6 * r3,
                   M[I4,M2] = rMAC;
                  rMAC = rMAC + r7 * r2,
                   r4 = M[I1,0];  // r4 = real(S_k[n+1][k])

                  // imag(D_k[n][k]) = H12*imag(S_k[n][k]) + H22*imag(D_k[n][k])
                  rMAC = r0 * r3,
                   M[I5,M2] = rMAC;
                  rMAC = rMAC + r8 * r2,
                   r5 = M[I4,0];  // r5 = real(D_k[n+1][k])

                  M[I2,M2] = rMAC;

               mixing_subbands_loop:

               // r4 = INTER_SUBBAND_STRIDE * end_k
               r4 = M2;
               r5 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_ADJUSTED_END_SUBBAND_OF_FREQ_BAND];
               r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_INTER_SAMPLE_STRIDE]; // measured in Addr units
               r4 = r4 * r5 (int);

               r4 = r1 - r4;
               I1 = I1 + r4;  // {real(ps_X_hybrid[ch=0][k=start_k][n+1] || real(X_SBR[ch=0][k=start_k][l=SBR_tHFAdj+n+1])}
               I4 = I4 + r4;  // {imag(ps_X_hybrid[ch=0][k=start_k][n+1] || imag(X_SBR[ch=0][k=start_k][l=SBR_tHFAdj+n+1])}
               I2 = I2 + r4;  // {imag(ps_X_hybrid[ch=1][k=start_k][n+1] || imag(X_SBR[ch=1][k=start_k][l=SBR_tHFAdj+n+1])}
               I5 = I5 + r4;  // {real(ps_X_hybrid[ch=1][k=start_k][n+1] || real(X_SBR[ch=1][k=start_k][l=SBR_tHFAdj+n+1])}

               r4 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_BORDER_END];
               M3 = M3 + 1;
               Null = M3 - r4;
            if LT jump mixing_time_samples_loop;

            r8 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_ENVELOPE];

            r8 = r8 + 1;
            r0 = M[r9 + $aac.mem.PS_info_ptr];
            r0 = M[r0 + $aacdec.PS_NUM_ENV];
            Null = r8 - r0;
         if LT jump mixing_time_envelope_loop;

         I3 = I3 + (1*ADDR_PER_WORD);   // I3 -> h12[freq_band+1][last_env_of_prev_frame] = ps_h12_previous_envelope[freq_band+1][env=0]
         I6 = I6 + (1*ADDR_PER_WORD);   // I6 -> h21[freq_band+1][last_env_of_prev_frame] = ps_h21_previous_envelope[freq_band+1][env=0]
         I7 = I7 + (1*ADDR_PER_WORD);   // I7 -> h22[freq_band+1][last_env_of_prev_frame] = ps_h22_previous_envelope[freq_band+1][env=0]

         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_FREQ_BAND_LOOP_BOUND];
         M1 = M1 + 1;
         Null = r0 - M1;
      if GT jump mixing_freq_bands_loop;

      call $aacdec.ps_initialise_stereo_mixing_for_qmf_freq_bins_flash;

      r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_HYBRID_MODE];
      r0 = r0 - 1;
   if POS jump mixing_outer_loop;

   r8 = M[r9 + $aac.mem.TMP + $aacdec.PS_STEREO_MIXING_TEMP_R8];

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

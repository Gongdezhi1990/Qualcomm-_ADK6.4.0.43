// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"
#include "fft.h"

#ifdef AACDEC_SBR_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.sbr_synthesis_filterbank_combined
//
// DESCRIPTION:
//    Synthesis filterbank. Converts frequency domain X_sbr into time domain
//    audio samples and writes them to the output buffer
//
//
// INPUTS:
//    - r7 = $aacdec.SBR_LEFT_CH or $aacdec.SBR_RIGHT_CH
//    - r8 = initial loop number
//    - $aacdec.in_synth_loops = final loop number
//    - $aacdec.tmp + 5 = Write address of output buffer
//    - $aacdec.tmp + 6 = Length of output buffer, if circular
//
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - all including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_synthesis_filterbank_combined;
   .CODESEGMENT AACDEC_SBR_SYNTHESIS_FILTERBANK_COMBINED_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_synthesis_filterbank_combined:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SBR_SYNTHESIS_FILTERBANK_COMBINED_ASM.SBR_SYNTHESIS_FILTERBANK_COMBINED.SBR_SYNTHESIS_FILTERBANK_COMBINED.PATCH_ID_1, r1)
#endif
   

   r0 = $aacdec.SBR_AUDIO_OUT_SCALE_AMOUNT;
   #ifdef AACDEC_PS_ADDITIONS
      r1 = $aacdec.SBR_AUDIO_OUT_SCALE_AMOUNT * 2;
      Null = M[r9 + $aac.mem.PS_present];
      if NZ r0 = r1;
   #endif
   M[r9 + $aac.mem.TMP + $aacdec.SBR_qmf_synthesis_filterbank_audio_out_scale_factor] = r0;


   M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = r7;

   // store constant specifying whether using downsampled mode in r6
   // Downsampled mode currently not supported
      r6 = $aacdec.SBR_NORMALSAMPLED;
      r1 = 32;
      M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.NUM_POINTS_FIELD] = r1;
      r1 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.REAL_ADDR_FIELD] = r1;
      jump selected_samplemode;

   downsampled_initial:
      r6 = $aacdec.SBR_DOWNSAMPLED;
      r1 = $aacdec.SBR_DOWNSAMPLED_N;
      M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.NUM_POINTS_FIELD] = r1;

   selected_samplemode:
   M[r9 + $aac.mem.TMP + (4*ADDR_PER_WORD)] = r6;
   r1 = M[r9 + $aac.mem.SBR_temp_2_ptr];
   M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.IMAG_ADDR_FIELD] = r1;



   // get current write address for v_buffer and store in temporary buffers
   Words2Addr(r7);
   r0 = r9 + r7;
   r0 = M[r0 + $aac.mem.SBR_v_cbuffer_struc_address];
   Addr2Words(r7);
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   M[r9 + $aac.mem.TMP + (12*ADDR_PER_WORD)] = r2;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = r0;
   r0 = -1;                                     // if in downsampled mode only use first half of v_buffer
   Null = r6 - $aacdec.SBR_NORMALSAMPLED;
   if NZ r1 = r1 ASHIFT r0;
   M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)] = r1;

   main_loop:

      // store main loop number in temporary buffer
      M[r9 + $aac.mem.TMP] = r8;

      r1 = r8 + ($aacdec.SBR_tHFAdj);
      r1 = r1 * ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD) (int);

      r0 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
      I0 = r0 + r1;
      r0 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
      I4 = r0 + r1;

      // construct the v_buffer in either normal or downsampled mode
      r0 = &$aacdec.sbr_synthesis_construct_v;
      r1 = &$aacdec.sbr_synthesis_downsampled_construct_v;
      Null = r6;
      if NZ r0 = r1;
      call r0;


      r7 = M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)];
      r6 = M[r9 + $aac.mem.TMP + (4*ADDR_PER_WORD)];
      r0 = M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)];
      r1 = M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)];


#ifdef AACDEC_ELD_ADDITIONS
      r2 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
      Null = r2 - $aacdec.ER_AAC_ELD;
      if Z jump non_eld_init_done;
#endif
         I5 = r0;
         L5 = r1;

         r1 = I4;
         M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = r1;
      non_eld_init_done:


#ifdef BASE_REGISTER_MODE
      r0 = M[r9 + $aac.mem.TMP + (11*ADDR_PER_WORD)];
      push r0; pop B5;
#endif


      // store constant specifying whether using downsampled mode in r6
      // also set N, K, step based on whether using downsampled mode
      // M0 = K
      // M1 = N + K
      // M2 = K * step
      Null = r6 - $aacdec.SBR_NORMALSAMPLED;
      if NZ jump downsampled;
         M0 = ($aacdec.SBR_K*ADDR_PER_WORD);
         M1 = M0 + ($aacdec.SBR_N*ADDR_PER_WORD);
         M2 = ($aacdec.SBR_K * $aacdec.SBR_STEP * ADDR_PER_WORD);
         r4 = ($aacdec.SBR_STEP*ADDR_PER_WORD);
         r0 = M[r9 + $aac.mem.SBR_qmf_filterbank_window_ptr];
         I1 = r0 + ($aacdec.SBR_STEP*ADDR_PER_WORD);
         I2 = r0 + (320*ADDR_PER_WORD) - ($aacdec.SBR_STEP*ADDR_PER_WORD);
         jump samplemode_selected;

      downsampled:
         M0 = ($aacdec.SBR_DOWNSAMPLED_K*ADDR_PER_WORD);
         M1 = M0 + ($aacdec.SBR_DOWNSAMPLED_N*ADDR_PER_WORD);
         M2 = ($aacdec.SBR_DOWNSAMPLED_K * $aacdec.SBR_DOWNSAMPLED_STEP * ADDR_PER_WORD);
         r4 = ($aacdec.SBR_DOWNSAMPLED_STEP*ADDR_PER_WORD);
         r0 = M[r9 + $aac.mem.SBR_qmf_filterbank_window_ptr];
         I1 = r0 + ($aacdec.SBR_DOWNSAMPLED_STEP);
         I2 = r0 + (320*ADDR_PER_WORD) - ($aacdec.SBR_DOWNSAMPLED_STEP*ADDR_PER_WORD);

      samplemode_selected:



#ifdef AACDEC_ELD_ADDITIONS
      r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
      Null = r0 - $aacdec.ER_AAC_ELD;
      if Z jump eld_version;
#endif


      // *** NON ELD MODE ***
      // ********************

      // Following code performs these operations
      //
      //  v_buffer[10N]
      //  ___________________________________________________________________________________________________
      // |___________|___________|___________|___________|___________|___________|___________|___________|___
      //  0    |     |K          |N          |N+K  |     |2N   |     |2N+K       |3N         |3N+K  |    |4N
      //       |           -------------------------           |                                    |
      //       |           |           -------------------------                                    |
      //       |           |           |           --------------------------------------------------
      //  ____\|/_________\|/_________\|/_________\|/_________
      // |___________|___________|___________|___________|____
      //  0          |K          |N          |N+K        |2N
      //  g_buffer[5N]
      //
      //
      // w(n) = g(n) * QMF_filterbank_window(step*n)
      //   for n = 0:5N-1
      //
      //
      // out(m) = w(m) + w(K+m) + w(2K+m) + w(3K+m) + ... + w(9K+m)
      //   for m = 0:K-1

      r8 = M[r9 + $aac.mem.TMP + $aacdec.SBR_qmf_synthesis_filterbank_audio_out_scale_factor];

      r0 = M[I4, MK1];   // dummy read                 I4 = beginning of v_buffer
      r0 = M[r9 + $aac.mem.SBR_qmf_filterbank_window_ptr];
      I3 = r0;                                      // I3 = beginning of QMF_filterbank_window (win)
      r0 = M[I4, M1],                   // r0 = v(m)
       r1 = M[I3, M2];                  // r1 = win(m)
      rMAC = r0 * r1,
       r0 = M[I4, M0],                  // r0 = v(m+N+K)
       r1 = M[I3, M2];                  // r1 = win((m+K)*step)
      r10 = r1;
      rMAC = rMAC + r0 * r1,
       r0 = M[I4, M1],                  // r0 = v(m+2N)
       r2 = M[I3, M2];                  // r1 = win((m+2K)*step)
      rMAC = rMAC + r0 * r2,
       r0 = M[I4, M0],                  // r0 = v(m+3N+K)
       r5 = M[I3, M2];                  // r1 = win((m+3K)*step)
      rMAC = rMAC + r0 * r5,
       r0 = M[I4, M1],                  // r0 = v(m+4N)
       r3 = M[I3, M2];                  // r1 = win((m+4K)*step)
      rMAC = rMAC + r0 * r3,
       r0 = M[I4, M0],                  // r0 = v(m+5N+K)
       r1 = M[I3, M2];                  // r1 = win((m+5K)*step)
      rMAC = rMAC + r0 * r1,
       r0 = M[I4, M1];                  // r0 = v(m+6N)
      rMAC = rMAC - r0 * r3,
       r0 = M[I4, M0];                  // r0 = v(m+7N+K)
      rMAC = rMAC + r0 * r5,
       r0 = M[I4, M1];                  // r0 = v(m+8N)
      rMAC = rMAC - r0 * r2,
       r0 = M[I4, M0];                  // r0 = v(m+9N+K)
      r3 = M[I4, MK1];                    // dummy read
      rMAC = rMAC + r0 * r10,
       r0 = M[I4, M1],                  // r0 = v(m)
       r1 = M[I1, M2];                  // r1 = win(m)
      //rMAC = rMAC * $aacdec.SBR_AUDIO_OUT_SCALE_AMOUNT (int) (sat);
      rMAC = rMAC * r8 (int) (sat);

      r10 = M0;
      Addr2Words(r10);
      r10 = r10 - 1;
      r2 = M2;
      r2 = r2 * 5 (int);
      r2 = r2 - r4,
       M[I5, MK1] = rMAC;                    // out = sum of 'w's
      M3 = -M2;

      do synthesis_loop;
         rMAC = r0 * r1,
          r0 = M[I4, M0],                  // r0 = v(m+N+K)
          r1 = M[I1, M2];                  // r1 = win((m+K)*step)
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M1],                  // r0 = v(m+2N)
          r1 = M[I1, M2];                  // r1 = win((m+2K)*step)
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],                  // r0 = v(m+3N+K)
          r1 = M[I1, M2];                  // r1 = win((m+3K)*step)
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M1],                  // r0 = v(m+4N)
          r1 = M[I1, M2];                  // r1 = win((m+4K)*step)
         I1 = I1 - r2;
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],                  // r0 = v(m+5N+K)
          r1 = M[I2, M3];                  // r1 = win((m+5K)*step)
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M1],                  // r0 = v(m+6N)
          r1 = M[I2, M3];                  // r1 = win((m+6K)*step)
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],                  // r0 = v(m+7N+K)
          r1 = M[I2, M3];                  // r1 = win((m+7K)*step)
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M1],                  // r0 = v(m+8N)
          r1 = M[I2, M3];                  // r1 = win((m+8K)*step)
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],                  // r0 = v(m+9N+K)
          r1 = M[I2, M3];                  // r1 = win((m+9K)*step)
         I2 = I2 + r2,                     // m = m + 1
          r3 = M[I4, MK1];                   // dummy read
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M1],                  // r0 = v(m)
          r1 = M[I1, M2];                  // r1 = win(m)
         //rMAC = rMAC * $aacdec.SBR_AUDIO_OUT_SCALE_AMOUNT (int) (sat);
         rMAC = rMAC * r8 (int) (sat);
         M[I5, MK1] = rMAC;                  // out = sum of 'w's
      synthesis_loop:


      r0 = I5;
      M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)] = r0;
      L5 = 0;

      L4 = 0;

      // loop again if not yet done SBR_numTimeSlotsRate passes
      r8 = M[r9 + $aac.mem.TMP];
      r8 = r8 + 1;
      r0 = M[r9 + $aac.mem.SBR_in_synth_loops];
      Null = r8 - r0;
   if LT jump main_loop;

   Words2Addr(r7);
   r0 = r9 + r7;
   r0 = M[r0 + $aac.mem.SBR_v_cbuffer_struc_address];
   Addr2Words(r7);
   r1 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
   call $cbuffer.set_write_address;


   // if its a mono stream and we have stereo buffers connected then copy
   // the left channel's output to the right channel
   Null = M[r9 + $aac.mem.CONVERT_MONO_TO_STEREO_FIELD];
   if Z jump dont_copy_to_right_channel;

      // move I5 pointer back 1024 (to start of the frame) before we do the copy
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         M0 = (-1024*ADDR_PER_WORD);
      #else
         M0 = (-2048*ADDR_PER_WORD);
      #endif
      r1 = M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)];
      L5 = r1;
#ifdef BASE_REGISTER_MODE
      r0 = M[r9 + $aac.mem.TMP + (11*ADDR_PER_WORD)];
      push r0; pop B5;
#endif

      r0 = M[I5,M0];

      // set I1/L1 to point to the right audio output buffer
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
      call $cbuffer.get_write_address_and_size_and_start_address;
      push r2;
      pop B1;
#else
      call $cbuffer.get_write_address_and_size;
#endif
      I1 = r0;
      L1 = r1;

      // do the copy
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         r10 = 1024;
      #else
         r10 = 2048;
      #endif

      do copy_to_right_loop;
         r0 = M[I5, MK1];
         M[I1, MK1] = r0;
      copy_to_right_loop:

      // store updated cbuffer pointer for the right channel
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
      r1 = I1;
      call $cbuffer.set_write_address;
      L1 = 0;
      L5 = 0;
   dont_copy_to_right_channel:

   // pop rLink from stack
   jump $pop_rLink_and_rts;


#ifdef AACDEC_ELD_ADDITIONS
   eld_version:
      // *** ELD MODE ***
      // ****************


      // *****************************************************************************
      // for( n = 0; n <= 4; n++)
      // {
      //    for( k = 0; k <= 63; k++)
      //    {
      //       g[128 * n + k] = v[256 * n + k]
      //       g[128 * n + 64 + k] = v[256 * n + 192 + k]
      //    }
      // }
      // for( n = 0; n <= 639; n++)
      // {
      //    w[n] = g[n] * c[n]
      // }
      // *****************************************************************************
      // I4 is the pointer to the v_buffer  // length = 1280
      L5 = L4;

#ifdef BASE_REGISTER_MODE
      push B4;    pop B5;
#endif

      r0 = M[r9 + $aac.mem.ELD_qmf_filterbank_window_synthesis_ptr];
      I0 = r0;
      r0 = M[r9 + $aac.mem.ELD_synthesis_g_w_buffer_ptr];
      I1 = r0;
      r6 = 5;
      M1 = MK1;
      M0 = -MK1;
      M3 = (192*ADDR_PER_WORD);
      windowing_main_loop:
         I2 = I1 + (64*ADDR_PER_WORD);                               // point to g(64)
         I6 = I0 + (64*ADDR_PER_WORD);                               // point to window_coeff(64)
         I5 = I4;
         r1 = M[I5,M3];                                              // dummy read to change I5 pointer to v(192)
         r10 = 64;
         r1 = M[I4,M1] , r0 = M[I0,M1];                              // preload v , preload window_coeff
         r3 = M[I5,M1];                                              // preload v(192)
         r4 = M[I6,M1];                                              // preload window_coeff(64)
         do synthesis_windowing_loop;
            r2 = r1 * r0(frac);                                      // calculate w(n) = g(n)*c(n)
            r5 = r3 * r4(frac);                                      // calculate w(n+) = g(n+)*c(n+)
            r0 = M[I0,M1] , r1 = M[I4,M1];                           // load window_coeff , load v
            r3 = M[I5,M1] , M[I1,M1] = r2;                           // load v(192+) , write w(n)
            M[I2,M1] = r5 , r4 = M[I6,M1];                           // write w(n+64) , load window_coeff(64+)
         synthesis_windowing_loop:

         I0 = I6 - MK1;                                              // coeff_buffer
         I1 = I2;                                                    // g_buffer
         I4 = I5 - MK1;                                              // v_buffer
         r6 = r6 - 1;
      if NZ jump windowing_main_loop;

      r0 = M[I4, -MK1];                                                 // dummy read to adjust I4 pointer
      r1 = I4;
      M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = r1;
      L4 = 0;

      // *****************************************************************************
      // for( k = 0; k <= 63; k++)
      // {
      //    temp = w[k]
      //    for( n = 1; n <= 9; n++)
      //    {
      //       temp = temp + w[64*n + k];
      //    }
      //    nextOutputAudioSample = temp
      // }
      // *****************************************************************************
      r0 = M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)];
      r1 = M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)];
      I5 = r0;                                // pointer to the output buffer
      L5 = r1;                                // length of the output buffer
#ifdef BASE_REGISTER_MODE
      r0 = M[r9 + $aac.mem.TMP + (11*ADDR_PER_WORD)];
      push r0; pop B5;
#endif

      r0 = M[r9 + $aac.mem.ELD_synthesis_g_w_buffer_ptr];
      I1 = r0;                                // buffer holding the windowing output (length:640)
      I2 = I1;
      r5 = 64;                                // total number of output values
      M0 = 0;
      M1 = (64*ADDR_PER_WORD);
      M2 = MK1;
      r8 = M[r9 + $aac.mem.TMP + $aacdec.SBR_qmf_synthesis_filterbank_audio_out_scale_factor];
      output_main_loop:
         I1 = I2 + M0;
         r10 = 10;
         rMAC = 0 , r1 = M[I1,M1];       // set rMAC = 0 , preload w
         do output_gen_loop;
            rMAC = rMAC + r1 , r1 = M[I1,M1];
         output_gen_loop:
         rMAC = rMAC * r8 (int) (sat);
         M0 = M0 + M2 , M[I5,M2] = rMAC;
         r5 = r5 - 1;
      if NZ jump output_main_loop;

      r0 = I5;
      M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)] = r0;
      L5 = 0;
      L4 = 0;
      // *****************************************************************************
      // loop again if not yet done SBR_numTimeSlotsRate passes
      // *****************************************************************************
      r8 = M[r9 + $aac.mem.TMP];
      r8 = r8 + 1;
      r0 = M[r9 + $aac.mem.SBR_in_synth_loops];
      Null = r8 - r0;
   if LT jump main_loop;


   Words2Addr(r7);
   r0 = r9 + r7;
   r0 = M[r0 + $aac.mem.SBR_v_cbuffer_struc_address];
   Addr2Words(r7);
   r1 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
   call $cbuffer.set_write_address;

   // **********************************************************************************************************************
   // If its a mono stream and we have stereo buffers connected then copy the left channel's output to the right channel
   // **********************************************************************************************************************
   Null = M[r9 + $aac.mem.CONVERT_MONO_TO_STEREO_FIELD];
   if Z jump dont_copy_to_right_channel_eld;
      // *********************************************************************************************************
      // move I5 pointer back 1024 (to start of the frame) before we do the copy
      // *********************************************************************************************************
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         M0 = (-$aacdec.FRAME_SIZE_512*ADDR_PER_WORD);
         r0 = (-$aacdec.FRAME_SIZE_480*ADDR_PER_WORD);
      #else
         M0 = -(2*$aacdec.FRAME_SIZE_512*ADDR_PER_WORD);
         r0 = -(2*$aacdec.FRAME_SIZE_480*ADDR_PER_WORD);
      #endif
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ M0 = r0;
      r1 = M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)];
      L5 = r1;
#ifdef BASE_REGISTER_MODE
      r0 = M[r9 + $aac.mem.TMP + (11*ADDR_PER_WORD)];
      push r0; pop B5;
#endif

      r0 = M[I5,M0];

      // ******************************************************
      // set I1/L1 to point to the right audio output buffer
      // ******************************************************
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
      call $cbuffer.get_write_address_and_size_and_start_address;
      push r2;
      pop B1;
#else
      call $cbuffer.get_write_address_and_size;
#endif
      I1 = r0;
      L1 = r1;

      // ******************************************************
      // Copy the data
      // ******************************************************
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         r10 = $aacdec.FRAME_SIZE_512;
         r0 = $aacdec.FRAME_SIZE_480;
      #else
         r10 = (2*$aacdec.FRAME_SIZE_512);
         r0 = (2*$aacdec.FRAME_SIZE_480);
      #endif
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ r10 = r0;
      do copy_to_right_loop_eld;
         r0 = M[I5, MK1];
         M[I1, MK1] = r0;
      copy_to_right_loop_eld:

      // ******************************************************
      // store updated cbuffer pointer for the right channel
      // ******************************************************
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
      r1 = I1;
      call $cbuffer.set_write_address;
      L1 = 0;
      L5 = 0;

   dont_copy_to_right_channel_eld:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

#endif //AACDEC_ELD_ADDITIONS

.ENDMODULE;

#endif

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
//    $aacdec.sbr_analysis_filterbank
//
// DESCRIPTION:
//    Convert audio samples output from plain AAC decoder into frequency domain
//    information used by the SBR section of the decoder.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - all including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_analysis_filterbank;
   .CODESEGMENT AACDEC_SBR_ANALYSIS_FILTERBANK_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_analysis_filterbank:

   // push rLink onto stack
   push rLink;

   // set aside some temporary storage for X_sbr_shared_real/imag
   r0 = $aacdec.X_SBR_SHARED_SIZE;
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.corruption;

   // set I0 and L0 to correct information for WavBlock
   r5 = M[r9 + $aac.mem.codec_struc];
   r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
   r1 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
   r7 = M[r9 + $aac.mem.CURRENT_CHANNEL];
   if NZ r0 = r1;
   M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = r7;
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   M[r9 + $aac.mem.TMP + (4*ADDR_PER_WORD)] = r2;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   M[r9 + $aac.mem.TMP] = r0;
   M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = r1;


   // set up fft structure for the analysis filterbank
   r1 = M[r9 + $aac.mem.SBR_temp_1_ptr];
   M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.REAL_ADDR_FIELD] = r1;
   r1 = M[r9 + $aac.mem.SBR_temp_2_ptr];
   M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.IMAG_ADDR_FIELD] = r1;
   r1 = 16;
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if NZ r1 = r1 + r1;
   M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.NUM_POINTS_FIELD] = r1;


   // store block loop number in tmp[2]
   r1 = $aacdec.SBR_tHFGen;
#ifdef AACDEC_ELD_ADDITIONS
   r2 = $aacdec.SBR_tHFGen_eld;
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if Z r1 = r2;
#endif
   M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)] = r1;

   block_loop:

      // store constant referring to channel being worked on in r7
      r7 = M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)];


      // get current write address for x_input_buffer
      Words2Addr(r7);
      r1 = r9 + r7;
      r0 = M[r1 + $aac.mem.SBR_x_input_buffer_write_pointers];
      I4 = r0;
      L4 = $aacdec.X_INPUT_BUFFER_LENGTH*ADDR_PER_WORD;
#ifdef BASE_REGISTER_MODE
      r0 = M[r1 + $aac.mem.SBR_x_input_buffer_write_base_pointers];
      push r0;   pop B4;
#endif



      /*
      //       WavBlock[1024] (per channel)
      //             _______________________________________________________________________________
      //            |_______________________________________________________________________________
      //             0     31|32    63|64......
      //               \ /
      //               / \
      //       __32|31_____0|__old_____________________________________
      //      |________________________________________________________| x_input_buffer [320] (per channel)  CIRCULAR BUFFER
      //       0           |                                         319
      //               current write pointer
      //
      */
      // restore WavBlock position and length
      // WavBlock is the PCM audio output from the AAC core decoder
      r0 = M[r9 + $aac.mem.TMP];
      r1 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
      I0 = r0;
      L0 = r1;
#ifdef BASE_REGISTER_MODE
      r2 = M[r9 + $aac.mem.TMP + (4*ADDR_PER_WORD)];
      push r2;  pop B0;
#endif

      // copy and scale 32 elements from WavBlock to x_input_buffer
      r10 = 15;
      r1 = (1.0 / (1 << $aacdec.SBR_ANALYSIS_SHIFT_AMOUNT));

      r0 = M[I0, MK1];
      r0 = r0 * r1 (frac),
       r2 = M[I0, MK1];
      do copy_loop;
         r2 = r2 * r1 (frac),
          M[I4, -MK1] = r0,
          r0 = M[I0, MK1];
         r0 = r0 * r1 (frac),
          M[I4, -MK1] = r2,
          r2 = M[I0, MK1];
      copy_loop:
      r2 = r2 * r1 (frac),
       M[I4, -MK1] = r0;
      M[I4, 0] = r2;


      // store back WavBlock position
      L0 = 0;
      push Null;  pop B0;
      r0 = I0;
      M[r9 + $aac.mem.TMP] = r0;


      // for n=0:63,
      //    u(n) = ( x_input_buffer(ch, n) * QMF_filterbank_window(2*n) ) +  ...
      //           ( x_input_buffer(ch, n+64) * QMF_filterbank_window(2*(n+64))) ) + ...
      //           ( x_input_buffer(ch, n+128) * QMF_filterbank_window(2*(n+128)) ) + ...
      //           ( x_input_buffer(ch, n+192) * QMF_filterbank_window(2*(n+192)) ) + ...
      //           ( x_input_buffer(ch, n+256) * QMF_filterbank_window(2*(n+256)) );
      // end;
      //
      //
      //    sbr_temp_1[32]
      //  -----------------------------------------------------------
      // | u(0) | -u(63) | -u(62) | -u(61) | -u(60) | ......| -u(33) |
      //  -----------------------------------------------------------
      //
      //    sbr_temp_2[32]
      //  -----------------------------------------------------------
      // | u(32) | u(31) | u(30) | u(29) | u(28) | ...........| u(1) |
      //  -----------------------------------------------------------




#ifdef AACDEC_ELD_ADDITIONS
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if NZ jump non_eld_version;

      // *** ELD MODE ***
      // ********************

      // I4 holds the current write address for x_input_buffer
      r0 = M[r9 + $aac.mem.ELD_temp_u_ptr];
      I2 = r0;                                                  // pointer to temp_u buffer(length:64)
      r0 = M[r9 + $aac.mem.ELD_qmf_filterbank_window_ptr];
      I1 = r0;                                                  // pointer to QMF_filterbank_window
      M0 = (64*ADDR_PER_WORD);
      M3 = (-255*ADDR_PER_WORD);
      r2 = I4;
      r10 = 64;
      do window_loop1_eld;
         r0 = M[I4,M0],
         r1 = M[I1,M0];           // win(0)
         rMAC = r0 * r1,
         r0 = M[I4, M0],
         r1 = M[I1, M0];          // win(64)
         rMAC = rMAC + r0 * r1,
         r0 = M[I4, M0],
         r1 = M[I1, M0];          // win(128)
         rMAC = rMAC + r0 * r1,
         r0 = M[I4, M0],
         r1 = M[I1, M0];
         rMAC = rMAC +r0 * r1,    // win(192)
         r0 = M[I4, M3],
         r1 = M[I1, M3];
         rMAC = rMAC + r0 * r1;   // win(256)
         M[I2, MK1] = rMAC;
      window_loop1_eld:

      // *****************************************************************************
      // write back the current input buffer location(saved in r2)
      // *****************************************************************************
      I4 = r2;
      r2 = M[I4, -MK1];
      r2 = I4;
      r0 = r9 + r7;
      M[r0 + $aac.mem.SBR_x_input_buffer_write_pointers] = r2;
      L4 = 0;
#ifdef BASE_REGISTER_MODE
      push Null;   pop B4;
#endif

      // *****************************************************************************
      // Rearrange data from temp_u to form sbr_temp_1 & sbr_temp_2
      // ****************************************************************************
      r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      I1 = r0;
      r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      I4 = r0;
      r0 = M[r9 + $aac.mem.ELD_temp_u_ptr];
      I5 = r0 + (48*ADDR_PER_WORD);
      I3 = r0 + (47*ADDR_PER_WORD);

      // *****************************************************************************
      // Rearrange the input - set sbr_temp_1(0:15) and sbr_temp_2(0:15)
      // *****************************************************************************
      r10 = 16;
      M3 = MK1;
      do rearrange_1_eld;
         r1 = M[I5, MK1], r2 = M[I3, -MK1];
         r4 = r1 + r2;
         M[I1,M3] = r4, r4 = r1 - r2;
         M[I4, MK1] = r4;
      rearrange_1_eld:

      // *****************************************************************************
      // Rearrange the input - set sbr_temp_1(16:31) and sbr_temp_2(16:31)
      // *****************************************************************************
      r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      I1 = r0 + (16*ADDR_PER_WORD);
      r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      I4 = r0 + (16*ADDR_PER_WORD);
      r0 = M[r9 + $aac.mem.ELD_temp_u_ptr];
      I5 = r0 + (31*ADDR_PER_WORD);
      I3 = r0;
      r10 = 16;
      do rearrange_2_eld;
         r1 = M[I5, -MK1] , r2 = M[I3, MK1];
         r4 = r1 - r2;
         r1 = -r1 , M[I1,M3] = r4;
         r4 = r1 - r2;
         M[I4, MK1] = r4;
      rearrange_2_eld:

      // *****************************************************************************
      // swap the sign for the alternate elements of sbr_temp_2
      // *****************************************************************************
      r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      I2 = r0 + (1*ADDR_PER_WORD);
      I6 = I2;
      r10 = 16;
      r1 = M[I2, MK2];
      do sign_adjust_imag_eld;
         r1 = -r1;
         M[I6, MK2] = r1 , r1 = M[I2, MK2];
      sign_adjust_imag_eld:


      // ** do DCT **
      call $aacdec.sbr_analysis_dct_kernel;

      // ** Reorder, scale and put data into the X_sbr matrix **
      /*
      //    X_sbr_shared_real [32*SBR_numTimeSlotsRate] (32 for each loop, here one 32 element section shown)
      //    --------------------------------------------------------------------------------------------------
      //   | 2R(0) | -2I(31) | 2R(1) | -2I(30) | 2R(2) | -2I(29) | 2R(3) | ... | -2I(x) | 2R(y) | 0 | ... | 0 |
      //    --------------------------------------------------------------------------------------------------
      //    /__________________________________________________________________________________\
      //    \                                  Kx                                              /
      //    --------------------------------------------------------------------------------------------------
      //   | 2I(0) | -2R(31) | 2I(1) | -2R(30) | 2I(2) | -2R(29) | 2I(3) | ... | -2R(x) | 2I(y) | 0 | ... | 0 |
      //    --------------------------------------------------------------------------------------------------
      //    X_sbr_shared_imag [32*SBR_numTimeSlotsRate] (32 for each loop, here one 32 element section shown)
      //
      //    R(x) = sbr_temp_3(x)
      //    I(x) = sbr_temp_4(x)
      */

      // I2 = beginning of real
      // I3 = end of real
      // I5 = beginning of imag
      // I6 = end of imag
      r0 = M[r9 + $aac.mem.SBR_temp_3_ptr];
      I2 = r0;
      r0 = M[r9 + $aac.mem.SBR_temp_4_ptr];
      I5 = r0;

      I6 = I5 + (15*ADDR_PER_WORD);
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_7_ptr];
      I7 = r0 + (15*ADDR_PER_WORD);
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_8_ptr];
      I5 = r0;

      // retrieve block loop number from tmp[2]
      r8 = M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)];

      // I1 = write position for X_sbr_shared_real for this loop
      // I4 = write position for X_sbr_shared_imag for this loop
      r0 = r8 * ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD) (int);

      r1 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
      I1 = r1 + r0;

      r1 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
      I4 = r1 + r0;

      // set length = (HF_reconstruction==1) ? Kx : 32
      r4 = 32;
      r1 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = M[r1 + $aacdec.SBR_kx];
      Null = M[r1 + $aacdec.SBR_HF_reconstruction];
      if NZ r4 = r0;


      r10 = r4 ASHIFT -1;
      r5 = r10 ASHIFT 1;

      M0 = MK1;
      r6 = 2;
      r7 = -r6;
      do rearrange_loop_32_eld;
         r0 = M[I2, MK1] , r1 = M[I6, -MK1];
         r0 = r0 * r6 (int) (sat);
         r1 = r1 * r7 (int) (sat), M[I1,M0] = r0;
         M[I1,M0] = r1 , r0 = M[I5, M0];
         r1 = M[I7, -MK1], r0 = r0 * r7 (int) (sat);
         r1 = r1 * r6 (int) (sat), M[I4,M0] = r0;
         M[I4,M0] = r1;
      rearrange_loop_32_eld:


      r10 = 64 - r4;

      Null = r4 - r5;
      if Z jump even_kx_eld;

         // sort out if kx is odd
        r0 = M[I2, MK1];
        r0 = r0 * r6 (int) (sat);
        M[I1,M0] = r0;
        r0 = M[I5, M0];
        r0 = r0 * r7 (int) (sat);
        M[I4,M0] = r0;

      even_kx_eld:
      r0 = 0;
      do zero_pad_loop_eld;
         M[I1, MK1] = r0,
          M[I4, MK1] = r0;
      zero_pad_loop_eld:


      // loop again if not yet done SBR_numTimeSlotsRate passes
      r8 = r8 + 1;
      M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)] = r8;

      r1 = M[r9 + $aac.mem.SBR_numTimeSlotsRate];
      r1 = r1 + $aacdec.SBR_tHFGen_eld;
      Null = r8 - r1;
   if LT jump block_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

   non_eld_version:
#endif // AACDEC_ELD_ADDITIONS



      // *** NON ELD MODE ***
      // ********************

      // do window function and store in sbr_temp_1 and sbr_temp_2
      // ** work out U(0) **
      r0 = M[r9 + $aac.mem.SBR_qmf_filterbank_window_ptr];
      I1 = r0;
      r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      I2 = r0;
      M0 = (64*ADDR_PER_WORD);
      M1 = (128*ADDR_PER_WORD);
      M2 = (-128*ADDR_PER_WORD);
      M3 = (-255*ADDR_PER_WORD);

      r0 = M[I4,M0],
       r1 = M[I1,M1];        // win(0)
      rMAC = r0 * r1,
       r0 = M[I4, M0],
       r2 = M[I1, M1];       // win(128)
      rMAC = rMAC + r0 * r2,
       r0 = M[I4, M0],
       r1 = M[I1, M1];       // win(256)
      rMAC = rMAC + r0 * r1,
       r0 = M[I4, M0];
      rMAC = rMAC - r0 * r1, // win(384) = -win(256) = -r1
       r0 = M[I4, M3];
      rMAC = rMAC - r0 * r2; // win(512) = -win(128) = -r2
      M[I2, 0] = rMAC;



      // ** work out U(1) to U(32) **
      r0 = M[r9 + $aac.mem.SBR_qmf_filterbank_window_ptr];
      I1 = r0 + (2*ADDR_PER_WORD);
      I2 = r0 + (254*ADDR_PER_WORD);
      r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      I5 = r0 + (31*ADDR_PER_WORD);
      M3 = (-254*ADDR_PER_WORD);

      r10 = 32;
      r0 = M[I4,M0],
       r1 = M[I1,M1];
      do window_loop1;
         rMAC = r0 * r1,
          r0 = M[I4, M0],
          r1 = M[I1, M1];
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],
          r1 = M[I1, M3];
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],
          r1 = M[I2, M2];
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],
          r1 = M[I2, M2];
         r2 = M[I4, MK1];          //dummy read
         rMAC = rMAC + r0 * r1,
          r0 = M[I4,M0],
          r1 = M[I1,M1];
         I2 = I2 - M3,
          M[I5, -MK1] = rMAC;
      window_loop1:



      // ** work out -U(33) to -U(63) **
      r2 = M[r9 + $aac.mem.SBR_qmf_filterbank_window_ptr];
      I2 = r2 + (318*ADDR_PER_WORD);
      r2 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      I5 = r2 + (31*ADDR_PER_WORD);
      r10 = 31;
      M3 = (254*ADDR_PER_WORD);

      do window_loop2;
         rMAC = r0 * r1,
          r0 = M[I4, M0],
          r1 = M[I1, M1];
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],
          r1 = M[I2, M2];
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],
          r1 = M[I2, M2];
         rMAC = rMAC + r0 * r1,
          r0 = M[I4, M0],
          r1 = M[I2, M3];
         I1 = I1 - M3,
          r2 = M[I4, MK1];         //dummy read
         rMAC = rMAC + r0 * r1,
          r0 = M[I4,M0],
          r1 = M[I1,M1];
         rMAC = -rMAC;
         M[I5, -MK1] = rMAC;
      window_loop2:

      // write current input buffer location back
      M1 = (191*ADDR_PER_WORD);   //=255-M0
      r1 = M[I4, M1];
      r1 = I4;
      r0 = r9 + r7;
      M[r0 + $aac.mem.SBR_x_input_buffer_write_pointers] = r1;
      L4 = 0;
#ifdef BASE_REGISTER_MODE
      push Null;   pop B4;
#endif



// ** do DCT **
      call $aacdec.sbr_analysis_dct_kernel;

      // ** Reorder, scale and put data into the X_sbr matrix **
      /*
      //    X_sbr_shared_real [32*SBR_numTimeSlotsRate] (32 for each loop, here one 32 element section shown)
      //    --------------------------------------------------------------------------------------------------
      //   | 2R(0) | -2I(31) | 2R(1) | -2I(30) | 2R(2) | -2I(29) | 2R(3) | ... | -2I(x) | 2R(y) | 0 | ... | 0 |
      //    --------------------------------------------------------------------------------------------------
      //    /__________________________________________________________________________________\
      //    \                                  Kx                                              /
      //    --------------------------------------------------------------------------------------------------
      //   | 2I(0) | -2R(31) | 2I(1) | -2R(30) | 2I(2) | -2R(29) | 2I(3) | ... | -2R(x) | 2I(y) | 0 | ... | 0 |
      //    --------------------------------------------------------------------------------------------------
      //    X_sbr_shared_imag [32*SBR_numTimeSlotsRate] (32 for each loop, here one 32 element section shown)
      //
      //    R(x) = sbr_temp_3(x)
      //    I(x) = sbr_temp_4(x)
      */

      // I2 = beginning of real
      // I3 = end of real
      // I5 = beginning of imag
      // I6 = end of imag
      r0 = M[r9 + $aac.mem.SBR_temp_3_ptr];
      I2 = r0;
      r0 = M[r9 + $aac.mem.SBR_temp_4_ptr];
      I5 = r0;

      I3 = I2 + (31*ADDR_PER_WORD);
      I6 = I5 + (31*ADDR_PER_WORD);

      // retrieve block loop number from tmp[2]
      r8 = M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)];

      // I1 = write position for X_sbr_shared_real for this loop
      // I4 = write position for X_sbr_shared_imag for this loop
      r0 = r8 * ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD) (int);

      r1 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
      I1 = r1 + r0;
      r1 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
      I4 = r1 + r0;

      // set length = (HF_reconstruction==1) ? Kx : 32
      r4 = 32;
      r1 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = M[r1 + $aacdec.SBR_kx];
      Null = M[r1 + $aacdec.SBR_HF_reconstruction];
      if NZ r4 = r0;


      r10 = r4 ASHIFT -1;
      r5 = r10 ASHIFT 1;
      r6 = (1 << $aacdec.SBR_ANALYSIS_POST_SHIFT_AMOUNT);
      r7 = -r6;

      r0 = M[I2, MK1],
       r1 = M[I5, MK1];
      do rearrange_loop;
         r0 = r0 * r6 (int) (sat),
          r2 = M[I3, -MK1],
          r3 = M[I6, -MK1];
         r1 = r1 * r6 (int) (sat),
          M[I1, MK1] = r0;
         r3 = r3 * r7 (int) (sat),
          M[I4, MK1] = r1;
         r2 = r2 * r7 (int) (sat),
          M[I1, MK1] = r3,
          r1 = M[I5, MK1];
         M[I4, MK1] = r2,
          r0 = M[I2, MK1];
      rearrange_loop:

      r10 = 64 - r4;

      Null = r4 - r5;
      if Z jump even_kx;

        r0 = r0 * r6 (int) (sat);
         r1 = r1 * r6 (int) (sat),
          M[I1, MK1] = r0;
         M[I4, MK1] = r1;

      even_kx:
      r0 = 0;
      do zero_pad_loop;
         M[I1, MK1] = r0,
          M[I4, MK1] = r0;
      zero_pad_loop:


      // loop again if not yet done SBR_numTimeSlotsRate passes
      r8 = r8 + 1;
      M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)] = r8;

      r1 = M[r9 + $aac.mem.SBR_numTimeSlotsRate];
      r1 = r1 + $aacdec.SBR_tHFGen;
      Null = r8 - r1;
   if LT jump block_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

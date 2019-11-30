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
//    $aacdec.sbr_analysis_dct_kernel
//
// DESCRIPTION:
//    Carry out the DCT for the analysis filterbank
//
//
// INPUTS:
//    sbr_temp_1 (real)
//    sbr_temp_2 (imag)
//
//
// OUTPUTS:
//    sbr_temp_3 (real)
//    sbr_temp_4 (imag)
//
//
// TRASHED REGISTERS:
//    - all
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_analysis_dct_kernel;
   .CODESEGMENT AACDEC_SBR_ANALYSIS_DCT_KERNEL_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_analysis_dct_kernel:

   // push rLink onto stack
   push rLink;

#ifdef AACDEC_ELD_ADDITIONS
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if Z jump eld_version;
#endif

      // *** NON ELD MODE ***
      // ********************

      // ** Perform the following **
      //       sbr_temp_1(i) = [sbr_temp_2(i) * T(i + 64)] + [(sbr_temp_1(i) + sbr_temp_2(i)) * T(i)]
      //       sbr_temp_2(i) = [sbr_temp_1(i) * T(i + 32)] + [(sbr_temp_1(i) + sbr_temp_2(i)) * T(i)]
      // for i = 0-31
      // T = dct4_64_table
      r0 = M[r9 + $aac.mem.SBR_dct4_64_table_ptr];
      I6 = r0;
      r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      I4 = r0;
      r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      I5 = r0;
      I1 = I4;
      I0 = I5;
      M3 = MK1;
      call $aacdec.sbr_analysis_dct_kernel_internal_loop;


      // ** do fft **
      I7 = r9 + $aac.mem.SBR_fft_pointer_struct ;
      push r9;
      call $math.fft;
      pop r9;


      // ** Perform the following **
      //       out_real(i) = [IM(i) * T(i + 64+96)] + [(RE(i) + IM(i)) * T(i +96)]
      //       out_imag(i) = [RE(i) * T(i + 32+96)] + [(RE(i) + IM(i)) * T(i +96)]
      // for i = 0-15 & 17-31
      //
      //       out_real(16) = (IM(16) - RE(16)) * T(16 +96)
      //       out_imag(16) = (IM(16) + RE(16)) * T(16 +96)
      //
      // RE = real output from fft (need to undo bitreversed order)
      // IM = imag output from fft (need to undo bitreversed order)
      // T  = dct4_64_table

      r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];  //BITREVERSE(&$aacdec.sbr_temp_1);
#ifdef BASE_REGISTER_MODE
      push r0;  pop B1;
#endif
      call $math.address_bitreverse;
      I1 = r1;


      r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];  //BITREVERSE(&$aacdec.sbr_temp_2);
#ifdef BASE_REGISTER_MODE
      push r0;  pop B0;
#endif
      call $math.address_bitreverse;
      I0 = r1;

      r0 = M[r9 + $aac.mem.SBR_temp_3_ptr];
      I4 = r0;

      r0 = M[r9 + $aac.mem.SBR_temp_4_ptr];
      I5 = r0;
      r0 = M[r9 + $aac.mem.SBR_dct4_64_table_ptr];
      I6 = r0 + (96*ADDR_PER_WORD);
      M3 = BITREVERSE(16*ADDR_PER_WORD);

      rFLAGS = rFLAGS OR $BR_FLAG;
      call $aacdec.sbr_analysis_dct_kernel_internal_loop;
      rFLAGS = rFLAGS AND $NOT_BR_FLAG;

#ifdef BASE_REGISTER_MODE
      push Null;  pop B1;
      push Null;  pop B0;
#endif


      // fix for 17th element
      r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      r0 = M[r0 + (1*ADDR_PER_WORD)];
      r1 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      r1 = M[r1 + (1*ADDR_PER_WORD)];
      r3 = M[r9 + $aac.mem.SBR_dct4_64_table_ptr];
      r3 = M[r3 + ((96 + 16)*ADDR_PER_WORD)];
      r2 = r1 + r0;
      rMAC = r2 * r3;
      r2 = M[r9 + $aac.mem.SBR_temp_3_ptr];
      M[r2 + (16*ADDR_PER_WORD)] = rMAC;
      r2 = r1 - r0;
      rMAC = r2 * r3;
      r2 = M[r9 + $aac.mem.SBR_temp_4_ptr];
      M[r2 + (16*ADDR_PER_WORD)] = rMAC;

      // pop rLink from stack
      jump $pop_rLink_and_rts;




#ifdef AACDEC_ELD_ADDITIONS
   eld_version:
      // *** ELD MODE ***
      // ****************

      // ******************************************************
      // Rearrange real into real and imag part
      // ******************************************************
      r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      I5 = r0;                                            // real1
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_5_ptr];
      I4 = r0 + (15*ADDR_PER_WORD);                       // imag1
      I2 = I5 + (1*ADDR_PER_WORD);
      I1 = I5;
      r10 = 16;
      do rearrange_before_fft1;
         r1= M[I5, MK2], r2= M[I2, MK2];
         M[I4, -MK1]=r2,  M[I1, MK1] = r1;
      rearrange_before_fft1:

      // ******************************************************
      // Rearrange imag into real and imag part
      // ******************************************************
      r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      I5 = r0;                                             // real2
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_6_ptr];
      I4 = r0 + (15*ADDR_PER_WORD);       // imag2
      I2 = I5 + (1*ADDR_PER_WORD);
      I1 = I5;
      r10 = 16;
      do rearrange_before_fft2;
         r1 = M[I5, MK2], r2 = M[I2, MK2];
         M[I4, -MK1] = r2, M[I1, MK1] = r1;
      rearrange_before_fft2:

      // *****************************************************************************
      // Pre-multiply the inputs
      // *****************************************************************************
      r0 = M[r9 + $aac.mem.ELD_dct4_pre_cos_ptr];
      I6 = r0;
      r0 = M[r9 + $aac.mem.ELD_dct4_pre_sin_ptr];
      I2 = r0;
      r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      I3 = r0;                             // real1
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_5_ptr];
      I4 = r0;                             // imag1
      r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      I1 = r0;                             // real2
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_6_ptr];
      I7 = r0;                             // imag2
      r10 = 16;
      do apply_twiddle_before_fft;
         r1 = M[I3,0], r3 = M[I6, MK1];                              // load real1 , load cos
         rMAC = r1 * r3 , r2 = M[I4,0], r4 = M[I2, MK1];             // calculate new real1 , load imag1 , load sin
         rMAC = rMAC - r4 * r2;                                   // calculate new real1 = RC + SI
         M[I3, MK1] = rMAC , rMAC = r2 * r3;                         // write new real1 , calculate new imag1
         rMAC = rMAC + r4 * r1 , r1 = M[I1,0] , r2 = M[I7,0];     // calculate new imag1 = IC - RS , load real2 , load imag2

         rMAC = r1 * r3 , M[I4, MK1] = rMAC;                         // calculate new real2 , write new imag1
         rMAC = rMAC - r4 * r2;                                   // calculate new real2 = RC + SI
         M[I1, MK1] = rMAC , rMAC = r2 * r3;                         // write new real2 , calculate new imag2
         rMAC = rMAC + r4 * r1;                                   // calculate new imag2 = IC - RS
         M[I7, MK1] = rMAC;                                          // write new imag2
      apply_twiddle_before_fft:

      r1 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.REAL_ADDR_FIELD] = r1;
      r1 = M[r9 + $aac.mem.ELD_sbr_temp_5_ptr];
      M[r9 + $aac.mem.SBR_fft_pointer_struct  + $fft.IMAG_ADDR_FIELD] = r1;


      // ** do fft **
      I7 = r9 + $aac.mem.SBR_fft_pointer_struct ;
      push r9;
      call $math.fft;
      pop r9;

      r1 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.REAL_ADDR_FIELD] = r1;
      r1 = M[r9 + $aac.mem.ELD_sbr_temp_6_ptr];
      M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.IMAG_ADDR_FIELD] = r1;

      // *****************************************************************************
      // Apply FFT
      // *****************************************************************************
      I7 = r9 + $aac.mem.SBR_fft_pointer_struct;
      push r9;
      call $math.fft;
      pop r9;


      // *****************************************************************************
      // Do bitreverse on the FFT output
      // *****************************************************************************
      r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
      I4 = r0;
      r1 = I4;
      r0 = M[r9 + $aac.mem.SBR_temp_3_ptr];
#ifdef BASE_REGISTER_MODE
      push r0;  pop B0;
#endif
      I0 = r0;
      r0 = I0;
      call $math.address_bitreverse;
      I0 = r1;
      r10 = 16;
      call $math.bitreverse_array;
#ifdef BASE_REGISTER_MODE
      push Null;  pop B0;
#endif

      r0 = M[r9 + $aac.mem.ELD_sbr_temp_5_ptr];
      I4 = r0;
      r1 = I4;
      r0 = M[r9 + $aac.mem.SBR_temp_4_ptr];
#ifdef BASE_REGISTER_MODE
      push r0;  pop B0;
#endif
      I0 = r0;
      r0 = I0;
      call $math.address_bitreverse;
      r10 = 16;
      I0 = r1;
      call $math.bitreverse_array;
#ifdef BASE_REGISTER_MODE
      push Null;  pop B0;
#endif

      r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];
      I4 = r0;
      r1 = I4;
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_7_ptr];
      I0 = r0;

#ifdef BASE_REGISTER_MODE
      push r0;  pop B0;
#endif

      call $math.address_bitreverse;
      I0 = r1;
      r10 = 16;
      call $math.bitreverse_array;

      r0 = M[r9 + $aac.mem.ELD_sbr_temp_6_ptr];
      I4 = r0;
      r1 = I4;
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_8_ptr];
      I0 = r0;
#ifdef BASE_REGISTER_MODE
      push r0;  pop B0;
#endif
      call $math.address_bitreverse;
      r10 = 16;
      I0 = r1;
      call $math.bitreverse_array;

#ifdef BASE_REGISTER_MODE
      push Null;  pop B0;
#endif

      // *****************************************************************************
      // Post-multiply the outputs
      // *****************************************************************************
      r0 = M[r9 + $aac.mem.ELD_dct4_post_cos_ptr];
      I6 = r0;
      r0 = M[r9 + $aac.mem.ELD_dct4_post_sin_ptr];
      I2 = r0;
      r0 = M[r9 + $aac.mem.SBR_temp_3_ptr];
      I3 = r0;
      r0 = M[r9 + $aac.mem.SBR_temp_4_ptr];
      I4 = r0;
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_7_ptr];
      I7 = r0;
      r0 = M[r9 + $aac.mem.ELD_sbr_temp_8_ptr];
      I1 = r0;
      r10 = 16;
      do apply_twiddle_after_fft;
         r1 = M[I3,0] , r3 = M[I6, MK1];                          // load real1 , load cos
         rMAC = r1 * r3 , r2 = M[I4,0] , r4 = M[I2, MK1];         // calculate new real1 , load imag1 , load sin
         rMAC = rMAC - r4 * r2;                                   // calculate new real1 = RC + SI
         M[I3, MK1] = rMAC , rMAC = r2 * r3;                      // write new real1 , calculate new imag1
         rMAC = rMAC + r4 * r1 , r1 = M[I7,0], r2 = M[I1,0];      // calculate new imag1 = IC - RS , load real2 , load imag2

         rMAC = r1 * r3 , M[I4, MK1] = rMAC;                      // calculate new real2 , write new imag1
         rMAC = rMAC - r4 * r2;                                   // calculate new real2 = RC + SI
         M[I7, MK1] = rMAC , rMAC = r2 * r3;                      // write new real2 , calculate new imag2
         rMAC = rMAC + r4 * r1;                                   // calculate new imag2 = IC - RS
         M[I1, MK1] = rMAC;                                       // write new imag2
      apply_twiddle_after_fft:

      // pop rLink from stack
      jump $pop_rLink_and_rts;

#endif //AACDEC_ELD_ADDITIONS

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.sbr_analysis_dct_kernel_internal_loop
//
// DESCRIPTION:
//    performs the following windowing calculation:
//
//       real_out(i) = [IM(i) * T(i + 64)] + [(RE(i) + IM(i)) * T(i)]
//       imag_out(i) = [RE(i) * T(i + 32)] + [(RE(i) + IM(i)) * T(i)]
//
//    for i = 0-31
//    where IM = imaginary input
//          RE = real input
//          T = windowing table
//
// INPUTS:
//    I1 - real input
//    I0 - imaginary input
//    I4 - real output
//    I5 - imaginary output
//    I6 - windowing table
//    M3 - modifer for reading input data
//       - 1 for normal use
//       - 1024 if following 32 element fft (bitreversing)
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//    - r0-6, rMAC, I1, I0, I4-I7, M0-2.
//
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_analysis_dct_kernel_internal_loop;
   .CODESEGMENT AACDEC_SBR_ANALYSIS_DCT_KERNEL_INTERNAL_LOOP_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_analysis_dct_kernel_internal_loop:

   // We use I0 rather than I2, so that base register (non-circular aligned buffers) works with bit reversing

   r10 = 15;
   M0 = (1*ADDR_PER_WORD);
   M1 = (-32*ADDR_PER_WORD);
   M2 = (33*ADDR_PER_WORD);
   I7 = I6 + (64*ADDR_PER_WORD);

   r0 = M[I0, M3];                // r0 = IM(i)
   r2 = M[I1, M3],                // r1 = RE(i)
    r3 = M[I6, M0];               // r3 = T(i)

   r6 = r0 + r2,
    r1 = M[I1, M3];               // r1 = RE(i) (for next half loop)
   rMAC = r6 * r3,
    r4 = M[I7, M1];               // r4 = T(i+64)
   rMAC = rMAC + r0 * r4,
    r5 = M[I7, M2];               // r5 = T(i+32)
   rMAC = r6 * r3,
    M[I4, M0] = rMAC,             // real_out(i) = rMAC
    r0 = M[I0, M3];               // r0 = IM(i)
   rMAC = rMAC + r2 * r5,
    r3 = M[I6, M0];               // r3 = T(i)
   M[I5, M0] = rMAC;              // imag_out(i) = rMAC

   do loop;

      r6 = r0 + r1,
       r2 = M[I1, M3];            // r2 = RE(i) (for next half loop)
      rMAC = r6 * r3,
       r4 = M[I7, M1];            // r4 = T(i+64)
      rMAC = rMAC + r0 * r4,
       r5 = M[I7, M2];            // r5 = T(i+32)
      rMAC = r6 * r3,
       M[I4, M0] = rMAC,          // real_out(i) = rMAC
       r0 = M[I0, M3];            // r0 = IM(i)
      rMAC = rMAC + r1 * r5,
       r3 = M[I6, M0];            // r3 = T(i)
      M[I5, M0] = rMAC;           // imag_out(i) = rMAC

      r6 = r0 + r2,
       r1 = M[I1, M3];            // r1 = RE(i) (for next half loop)
      rMAC = r6 * r3,
       r4 = M[I7, M1];            // r4 = T(i+64)
      rMAC = rMAC + r0 * r4,
       r5 = M[I7, M2];            // r5 = T(i+32)
      rMAC = r6 * r3,
       M[I4, M0] = rMAC,          // real_out(i) = rMAC
       r0 = M[I0, M3];            // r0 = IM(i)
      rMAC = rMAC + r2 * r5,
       r3 = M[I6, M0];            // r3 = T(i)
      M[I5, M0] = rMAC;           // imag_out(i) = rMAC

   loop:

   r6 = r0 + r1;
   rMAC = r6 * r3,
    r4 = M[I7, M1];               // r4 = T(i+64)
   rMAC = rMAC + r0 * r4,
    r5 = M[I7, M2];               // r5 = T(i+32)
   rMAC = r6 * r3,
    M[I4, M0] = rMAC;             // real_out(i) = rMAC
   rMAC = rMAC + r1 * r5;
   M[I5, M0] = rMAC;              // imag_out(i) = rMAC
   rts;

.ENDMODULE;


#endif

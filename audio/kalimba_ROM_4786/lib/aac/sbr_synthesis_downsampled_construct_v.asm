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
//    $aacdec.sbr_synthesis_downsampled_construct_v
//
// DESCRIPTION:
//    constructs the v buffer for the sbr synthesis filterbank
//    (if in downsampled mode)
//
// INPUTS:
//    - I0 = beginning of part of X_sbr_real to be used
//    - I4 = beginning of part of X_sbr_imag to be used
//
// OUTPUTS:
//    - v_buffer updated
//    - I4 = new write address for v_buffer
//
// TRASHED REGISTERS:
//    - all
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_synthesis_downsampled_construct_v;
   .CODESEGMENT AACDEC_SBR_SYNTHESIS_DOWNSAMPLED_CONSTRUCT_V_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_synthesis_downsampled_construct_v:

   // push rLink onto stack
   push rLink;

   // I0 = X_sbr_real
   // I4 = X_sbr_imag
   // need I4 to equal next write address at end


   // Pre-process X_sbr
   //
   //
   // if(sbr.header_count == 0),
   //    for k=0:31,
   //       v1_array(k+1) = ( ( real(sbr.X_sbr(ch, k+1, l+1)) * downsampled_synthesis_pre_twiddle(k+1, 1) ) - ( imag(sbr.X_sbr(ch, k+1, l+1)) * downsampled_synthesis_pre_twiddle(k+1, 2) ) ) / 64;
   //
   //       v2_array(k+1) = ( ( imag(sbr.X_sbr(ch, k+1, l+1)) * downsampled_synthesis_pre_twiddle(k+1, 1) ) + ( real(sbr.X_sbr(ch, k+1, l+1)) * downsampled_synthesis_pre_twiddle(k+1, 2) ) ) / 64;
   //    end;
   // else
   //
   //    for k=0:31,
   //
   //       if( k < (sbr.kx_band + sbr.M_band) ),
   //          v1_array(k+1) = ( ( real(sbr.X_sbr(ch, k+1, l+1)) * downsampled_synthesis_pre_twiddle(k+1, 1) ) - ( imag(sbr.X_sbr(ch, k+1, l+1)) * downsampled_synthesis_pre_twiddle(k+1, 2) ) ) / 64;
   //
   //          v2_array(k+1) = ( ( imag(sbr.X_sbr(ch, k+1, l+1)) * downsampled_synthesis_pre_twiddle(k+1, 1) ) + ( real(sbr.X_sbr(ch, k+1, l+1)) * downsampled_synthesis_pre_twiddle(k+1, 2) ) ) / 64;
   //       else
   //          v1_array(k+1) = 0;
   //          v2_array(k+1) = 0;
   //       end;
   //
   //    end;
   //
   // end;
   //
   // Now pad end of temp_array_1 with zeros so it is 64 long for FFT


   r10 = 32;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   Null = M[r0 + $aacdec.SBR_header_count];     //check header_count
   if Z jump got_r10;

      r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r10 = M[r1 + $aacdec.SBR_kx_band];
      r1 = M[r1 + $aacdec.SBR_M_band];
      r10 = r10 + r1;
      r1 = r10 - $aacdec.SBR_DOWNSAMPLED_K;
      if GT r10 = r10 - r1;

   got_r10:

   r8 = r10 - $aacdec.SBR_DOWNSAMPLED_N;
   r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
   I3 = r0;
   r0 = M[r9 + $aac.mem.SBR_temp_3_ptr];
   I6 = r0;
   // downsampled_synthesis_pre_twiddle(:,1) = first half of sbr_sythesis_pre_twiddle_real * -1
   // downsampled_synthesis_pre_twiddle(:,2) = first half of sbr_sythesis_pre_twiddle_imag * -1
   r0 = M[r9 + $aac.mem.SBR_synthesis_pre_process_real_ptr];
   I5 = r0;
   r0 = M[r9 + $aac.mem.SBR_synthesis_pre_process_imag_ptr];
   I2 = r0;

   do pre_process_loop;
      r0 = M[I4, MK1],
       r1 = M[I2, MK1];
      rMAC = r0 * r1,
       r2 = M[I0, MK1],
       r3 = M[I5, MK1];
      rMAC = rMAC - r2 * r3;
      rMAC = r2 * r1,
       M[I3, MK1] = rMAC;
      rMAC = rMAC + r0 * r3;
      rMAC = -rMAC;
      M[I6, MK1] = rMAC;
   pre_process_loop:


   r10 = -r8;
   r0 = 0;

   do pre_process_zeros_loop;
      M[I3, MK1] = r0,
       M[I6, MK1] = r0;
   pre_process_zeros_loop:




   // DCT and DST Pre-processing
   //
   // for nn=0:31,
   //    in(nn+1) = in(nn+1) * exp( i * -pi * nn / 64);
   // end;
   //
   // Note:
   //   exp(i* -pi * nn / 64) is every even entry in sbr_synthesis_post_process


   r0 = $aacdec.SBR_DCT;
   M[r9 + $aac.mem.SBR_dct_dst] = r0;
   r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
   I0 = r0;


   dst_start:

   r10 = 32;
   r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];
   I5 = r0;

   r0 = M[r9 + $aac.mem.SBR_synthesis_post_process_real_ptr];
   I4 = r0;
   r0 = M[r9 + $aac.mem.SBR_synthesis_post_process_imag_ptr];
   I2 = r0;


   do dct_dst_pre_loop;
      r0 = M[I0, 0],
       r1 = M[I4, MK2];
      r1 = r1 * r0 (frac),
       r2 = M[I2, MK2];
      r2 = r2 * r0 (frac),
       M[I0, MK1] = r1;
      M[I5, MK1] = r2;
   dct_dst_pre_loop:

   r10 = 32;
   r0 = 0;
   do zero_temp_2_loop;
      M[I5, MK1] = r0;
   zero_temp_2_loop:




   // do 64 element fft
   r1 = I0 - (32*ADDR_PER_WORD);
   M[r9 + $aac.mem.SBR_fft_pointer_struct + $fft.REAL_ADDR_FIELD] = r1;
   I7 = r9 + $aac.mem.SBR_fft_pointer_struct;

   push r9;
   call $math.fft;
   pop r9;



   // DCT Post-processing
   //
   // for kk=0:31,
   //    v1_array(kk+1) = real( temp_array(kk+1) * exp( i * -pi * (kk + 0.5) / 64 ) );
   // end;
   //
   //
   // DST Post-processing
   //
   // for kk=0:31,
   //    v2_array(kk+1) = -imag( temp_array(kk+1) * exp( i * -pi * (kk + 0.5) / 64 ) );
   // end;
   //
   // Note:
   //  exp(i* -pi * nn / 64) is every odd entry in sbr_synthesis_post_process
   //  output from fft is in bitreverse order


   r10 = $aacdec.SBR_DOWNSAMPLED_K;

   r0 = M[r9 + $aac.mem.SBR_temp_2_ptr];  //BITREVERSE(&$aacdec.sbr_temp_2);
#ifdef BASE_REGISTER_MODE
   push r0;  pop B1;
#endif
   call $math.address_bitreverse;
   I1 = r1;

   r0 = M[r9 + $aac.mem.SBR_synthesis_post_process_real_ptr];
   I4 = r0 + (1*ADDR_PER_WORD);
   r0 = M[r9 + $aac.mem.SBR_synthesis_post_process_imag_ptr];
   I5 = r0 + (1*ADDR_PER_WORD);
   M0 = MK2;
   M1 = BITREVERSE(32*ADDR_PER_WORD);

   r0 = M[r9 + $aac.mem.SBR_dct_dst];
   Null = r0 - $aacdec.SBR_DCT;
   if NZ jump dst_postprocess;


   r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];  //BITREVERSE(&$aacdec.sbr_temp_1);
#ifdef BASE_REGISTER_MODE
   push r0;  pop B0;
#endif
   call $math.address_bitreverse;
   I0 = r1;


   r0 = M[r9 + $aac.mem.SBR_temp_4_ptr];
   I6 = r0;

   rFLAGS = rFLAGS OR $BR_FLAG;

   do write_v_dct_loop;
      r0 = M[I0, M1],
       r1 = M[I4, M0];
      rMAC = r0 * r1,
       r2 = M[I1, M1],
       r3 = M[I5, M0];
      rMAC = rMAC - r2 * r3;
      M[I6, MK1] = rMAC;
   write_v_dct_loop:

   rFLAGS = rFLAGS AND $NOT_BR_FLAG;

#ifdef BASE_REGISTER_MODE
   push Null;  pop B0;
   push Null;  pop B1;
#endif

   r0 = $aacdec.SBR_DST;
   M[r9 + $aac.mem.SBR_dct_dst] = r0;
   r0 = M[r9 + $aac.mem.SBR_temp_3_ptr];
   I0 = r0;
   jump dst_start;



   dst_postprocess:


   r0 = M[r9 + $aac.mem.SBR_temp_3_ptr];  //BITREVERSE(&$aacdec.sbr_temp_3);
#ifdef BASE_REGISTER_MODE
   push r0;  pop B0;
#endif
   call $math.address_bitreverse;
   I0 = r1;


   r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
   I6 = r0;


   rFLAGS = rFLAGS OR $BR_FLAG;

   do write_v_dst_loop;
      r0 = M[I0, M1],
       r1 = M[I5, M0];
      rMAC = r0 * r1,
       r2 = M[I1, M1],
       r3 = M[I4, M0];
      rMAC = rMAC + r2 * r3;
      rMAC = -rMAC;
      M[I6, MK1] = rMAC;
   write_v_dst_loop:

   rFLAGS = rFLAGS AND $NOT_BR_FLAG;

#ifdef BASE_REGISTER_MODE
   push Null;  pop B0;
#endif



   // Final Post-processing for downsampled construct v
   //
   // for n=0:31,
   //    sbr.v(ch, n+1) =  v2_array(n+1) - v1_array(n+1);
   //    sbr.v(ch, 64-n) = v1_array(n+1) + v2_array(n+1);
   // end;


   // Load I4, L4 with v_buffer
   r1 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
   I4 = r1;
   r1 = M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)];
   L4 = r1;

#ifdef BASE_REGISTER_MODE
   r0 = M[r9 + $aac.mem.TMP + (12*ADDR_PER_WORD)];
   push r0;  pop B4;
   push r0;  pop B1;
#endif

   r10 = 32;
   I1 = I4;
   L1 = L4;
   r0 = 1;
   M0 = r0 - $aacdec.SBR_DOWNSAMPLED_N;
   r0 = M[I1, M0];                        // I1 = end of v_buffer
   r0 = M[r9 + $aac.mem.SBR_temp_4_ptr];
   I2 = r0;      // I2 = v1_array
   r0 = M[r9 + $aac.mem.SBR_temp_1_ptr];
   I5 = r0;      // I5 = v2_array
   M0 = -MK1;
   M1 = MK1;

   do writeback_loop;
      r0 = M[I2, MK1],
       r1 = M[I5, MK1];
      r3 = r1 - r0;
      r2 = r1 + r0,
       M[I1, M1] = r3;
      M[I4, M0] = r2;
   writeback_loop:

   r0 = M[I4, 0];
   M0 = ($aacdec.SBR_DOWNSAMPLED_K*ADDR_PER_WORD);
   M0 = -M0;
   r0 = M[I4, M0];

   L1 = 0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

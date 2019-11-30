// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_ELD_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.eld_windowing
//
// DESCRIPTION:
//    AAC ELD windowing
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - r4 - frame size (512|480)
//    - I1 - overlap-add buffer write pointer
//    - I2 - IMDCT buffer read pointer
//    - I3 - overlap-add buffer read pointer
//    - I4 - Low Delay window read pointer
//
// TRASHED REGISTERS:
//    - r0-5, r8-r10, I0-I5, M1
//
// NOTE:
//   A number of General use (r4) and Index registers (I1-I4) will 'tunnel'
//   their way to $aacdec.overlap_add_eld. As this function exits all these
//   registers are set correctly and we can save a good number of cycles not
//   having to reload them.
//
// *****************************************************************************
.MODULE $M.aacdec.eld_windowing;
   .CODESEGMENT AACDEC_WINDOWING_ELD_PM;
   .DATASEGMENT DM;

   $aacdec.eld_windowing:

   // push rLink onto stack
   push rLink;

   // choose buffers depending on channel
   r5 = M[r9 + $aac.mem.codec_struc];
   Null = M[r9 + $aac.mem.CURRENT_CHANNEL];
   if NZ jump right_chan;
   left_chan:
      r0 = M[r9 + $aac.mem.OVERLAP_ADD_LEFT_PTR];
      I3 = r0;
      r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
      // if no buffer connected just exit
      // (eg. only playing 1 channel of a stereo stream)
      if Z jump $pop_rLink_and_rts;
      jump chan_select_done;
   right_chan:
      r0 = M[r9 + $aac.mem.OVERLAP_ADD_RIGHT_PTR];
      I3 = r0;
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
      // if no buffer connected just exit
      // (eg. only playing 1 channel of a stereo stream)
      if Z jump $pop_rLink_and_rts;
   chan_select_done:

   // set output address
   push r0; // store cbuffer for later

   #ifdef BASE_REGISTER_MODE
      call $cbuffer.get_write_address_and_size_and_start_address;
      push r2;
      pop  B5;
   #else
      call $cbuffer.get_write_address_and_size;
   #endif


   I5 = r0;
   L5 = r1;
   // set ovelap_add write address
   I1 = I3;
   M1 = MK1;

   // NUM_ZEROS is constant, delay_shift can vary though
   r10 = 0;
   r2 = M[r9 + $aac.mem.ELD_delay_shift];
   r0 = $aacdec.WINDOWING_ZEROS_512;
   r1 = $aacdec.WINDOWING_ZEROS_480;
   Null = M[r9 + $aac.mem.ELD_frame_length_flag];
   if NZ r0 = r1;
   Null = r0 - r2;
   if LT jump delay_shift_greater;

   r10 = r0 - r2;
   // IMDCT buffer start offset
   r2 = r0;
   // window buffer start offset
   r3 = 0;
   jump set_buffer_pointers;
   delay_shift_greater:
   // window buffer start offset
   r3 = r2 - r0;


   set_buffer_pointers:
   Words2Addr(r3);
   Words2Addr(r2);

   // point into the window buffer
   r0 = M[r9 + $aac.mem.ELD_win_coefs_ptr];
   I4 = r0 + r3;

   // point into the IMDCT buffer
   r4 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   I2 = r4 + r2;

   // set up the frame size
   r4 = $aacdec.FRAME_SIZE_512;
   r1 = $aacdec.FRAME_SIZE_480;
   Null = M[r9 + $aac.mem.ELD_frame_length_flag];
   if NZ r4 = r1;

   // Set up the gain to be applied to the data in r8.
   // This gain is calculated in $aacdec.apply_scalefactors_and_dequantize and is
   // applied in $aacdec.tns_encdec or $aacdec.windowing_eld (the same mechanism
   // is used for all the other profiles; the non-ELD profiles can also adjust the
   // gain calculation in $aacdec.ltp_reconstruction but LTP is not used for AAC_ELD).
   r3 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR];
   r0 = 1;
   r8 = 2;
   Null = M[r3 + (1*ADDR_PER_WORD)];
   if NZ r8 = r0;

   r3 = $aacdec.AUDIO_OUT_SCALE_AMOUNT;
   r0 = $aacdec.AUDIO_OUT_SCALE_AMOUNT/2;
   Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
   if NZ r3 = r0;
   r8 = r8 * r3 (int);

   // the window coefficients are stored in win_512_ld|win_480_ld divided by 2;
   // here we embed the x2 in the gain to be applied to the audio;
   r8 = r8 LSHIFT 1;

   // pass overlap-add buffer to output, after we save the loop counter and scale the signal.
   // Loop counter = | M[$aacdec.delay_shift] - $aacdec.WINDOWING_ZEROS_X for M[$aacdec.delay_shift] > $aacdec.WINDOWING_ZEROS_X
   //                | 0 for M[$aacdec.delay_shift] <= $aacdec.WINDOWING_ZEROS_X
   r1 = r10;
   do pass_overlap_to_output;
      r0 = M[I3,M1];
      r2 = r0 * r8 (int) (sat);
      M[I5,M1] = r2;
   pass_overlap_to_output:

   // generate output = overlap_add + window x IMDCT_buffer
   // Loop counter = $aacdec.FRAME_SIZE_X - 'the amount generated in the loop above'
   r10 = r4 - r1;
   r10 = r10 - 1;

   rMAC = M[I3,M1];
   r0 = M[I2,M1], r2 = M[I4,M1];
   do write_output;
      rMAC = rMAC + r0*r2,    r0 = M[I2,M1],  r2 = M[I4,M1];
      rMAC = rMAC * r8 (int) (sat);
      M[I5,M1] = rMAC;
      rMAC = M[I3,M1];
   write_output:
   rMAC = rMAC + r0*r2;
   rMAC = rMAC * r8 (int) (sat);
   M[I5,M1] = rMAC;

   // set write pointer only if SBR isn't present (highly unlikely for ELD);
   pop r0; // restore cbuffer pointer
   Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
   if NZ jump dont_set_write_ptr;
      r1 = I5;
      call $cbuffer.set_write_address;
   dont_set_write_ptr:

   L5 = 0;
#ifdef BASE_REGISTER_MODE
   push Null;  pop B5;
#endif

   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif //AACDEC_ELD_ADDITIONS

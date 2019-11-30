// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.ltp_decode
//
// DESCRIPTION:
//    Decode LTP (Long Term Prediction) data
//
// INPUTS:
//    - r4 = pointer to ICS info
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume all including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.ltp_decode;
   .CODESEGMENT AACDEC_LTP_DECODE_PM;
   .DATASEGMENT DM;

   $aacdec.ltp_decode:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.LTP_DECODE_ASM.LTP_DECODE.LTP_DECODE.PATCH_ID_0, r1)
#endif
   
   

   M0 = 1;
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   I5 = r1;
   call $aacdec.windowing;


   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];

   // select which channel we're using
   r5 = M[r9 + $aac.mem.codec_struc];
   r0 = r9 + $aac.mem.ICS_LEFT;
   Null = r4 - r0;//&$aacdec.ics_left;
   if NZ jump right_channel;
      // set register for the left channel
      r8 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
      r6 = M[r4 + $aacdec.ics.LTP_INFO_PTR_FIELD];
      if Z jump $pop_rLink_and_rts;
      jump channel_selected;
   right_channel:
      // set register for the right channel
      r8 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
      r6 = M[r4 + $aacdec.ics.LTP_INFO_CH2_PTR_FIELD];
      if Z jump $pop_rLink_and_rts;
   channel_selected:



   // |   last_2048_audio_samples    |  overlap_add   |  zero padding  |
   //  ------------------------------ ---------------- ----------------
   // |<------------2048------------>|<-----1024----->|<-----1024----->|
   //  ------------------------------ ---------------- ----------------
   // |..............................|.................................|
   //                 |<---ltp_lag-->|                  |<---ltp_lag-->|
   //            first_sample                       last_sample
   //                 |<--------------2048------------->|

   // r6 = &ltp_info
   r5 = M[r6 + $aacdec.ltp.COEF_FIELD];
   r0 = M[r6 + $aacdec.ltp.LAG_FIELD];
   r2 = M[r9 + $aac.mem.LTP_COEFS_FIELD];
   Words2Addr(r5);
   r5 = M[r2 + r5];

   // ** Do possible copying of the 'zero padding' **
   r10 = 1024 - r0;   // 1024 - ltp_lag
   if NEG r10 = 0;
   r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   I4 = r2 + (2047*ADDR_PER_WORD);

   // pad end of input to mdct with zeros to make 2048 samples altogether
   r2 = 0;
   do zero_padding_loop;
      M[I4, -MK1] = r2;
   zero_padding_loop:


   r1 = 1024 - r0;
   if NEG r1 = 0;
   r1 = r1 + r0;
   r1 = r1 - (1024 - 1);
   r10 = (1024 + 1) - r1;
   Words2Addr(r1);
   I1 = I5 - r1;


   // copy data from the overlap_add buffer applying ltp_coef gain
   r2 = M[I1, -MK1];
   do overlap_copy_loop;
      // multiply by ltp_coef
      r2 = r2 * r5 (frac);
      r2 = M[I1, -MK1],
       M[I4, -MK1] = r2;
   overlap_copy_loop:


   // ** Do possible copying of the 'last_2048_audio_samples' **

   r10 = r0;

   // get pointer to audio buffer
   r0 = r8;
#ifdef BASE_REGISTER_MODE
    call $cbuffer.get_write_address_and_size_and_start_address;
    push r2;
    pop B1;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   I1 = r0;
   L1 = r1;

   // dummy read to go back ltp_lag samples
   r3 = M[I1, -MK1];

   // copy data from the audio buffer applying ltp_coef gain
   r2 = M[I1, -MK1];
   do audio_copy_loop;
      r2 = r2 * r5 (frac);   // multiply by ltp_coef
      r2 = M[I1, -MK1],
       M[I4, -MK1] = r2;
   audio_copy_loop:
   L1 = 0;


   // perform windowing of both 1024-sample blocks
   call $aacdec.filterbank_analysis_ltp;

   // if(ics(ch).tns_data_present) call tns_encdec;
   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
   M2 = 1;  // flag for encode mode
   Null = M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD];
   if NZ call $aacdec.tns_encdec;
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump frame_corrupt;

   // add scale factor bands of X_est to spec
   // (as defined by ics(ch).ltp_long_used)
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   I4 = r0;
   call $aacdec.ltp_reconstruction;


   frame_corrupt:
   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "stack.h"

#include "a2dp_decode_cap_asm.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


#ifdef INSTALL_A2DP_HEADER_SUPPORT

#include "codec_library.h"

// *****************************************************************************
// MODULE:
//   $a2dp_strip_header
//
// DESCRIPTION:
//    Strip all headers
//
// INPUTS:
//    - r7 =  Pointer to a2dp header structure
//    - r5 =  The codec structure
//    - r3 =  size of total header in bytes to strip   
//
// OUTPUTS:
//    - r6 = 1 on success
//
// TRASHED REGISTERS:
//    - r0, r1, r2
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.a2dp_strip_header;
   .CODESEGMENT PM_FLASH;

$a2dp_strip_header:
   push rLink;
   
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];

#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop B0;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I0 = r0;
   L0 = r1;
      
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
   call $cbuffer.calc_amount_data_in_words;
   r4 = r0 LSHIFT 1; // available data in bytes
   
   header_loop:   
   Null = r4 - r3;
   if LT jump header_not_found;

   
   // Any L2CAP header to extract
   r2 = M[r7 +  $a2dp_decode.HEADER_TYPE_FIELD];
   Null = r2 AND $a2dp_decode.BFRAME_TYPE_MASK;
   if NZ call $a2dp_strip_l2cap_header;


   /* The BYTES_LEFT field is updated while stripping the L2CAP
    * header. If L2CAP header is not presented, there must be
    * some other mechanism (Meta data) to know the packet length
    * in advance. This is not handled right now.
    */

   // recheck again after header extract and RTP syncing
   Null = r4 - r3;
   if LT jump header_not_found;
   
   /* BYTES_LEFT field must not be 0 at this point. It must contain
    * the packet length including the stripped header. Don't allow to
    * proceed if it is not a valid value
    */
   r4 = M[r7 + $a2dp_decode.BYTES_LEFT];
   Null = r4 - r3;
   if LT jump header_not_found;

   /* update BYTES_LEFT */
   r4 = r4 - r3;
   M[r7 + $a2dp_decode.BYTES_LEFT] = r4;

   // Now r3 contains rest of the bytes to strip. Just strip it all
   // together
   r6 = M[r7 + $a2dp_decode.GET_BITS_ADDR_FIELD];
   r10 = r3;
   do readhdrloop;
      call r6;
   readhdrloop:
 
   // now save the pointer
   r6 = $a2dp_decode.SUCCESS;
   jump save_read_pointer;   

 header_not_found:
   M[r7 + $a2dp_decode.BYTES_LEFT] = Null;
   r6 = $a2dp_decode.HEADER_NOT_FOUND;
   
 save_read_pointer:
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
   r1 = I0;
   call $cbuffer.set_read_address;
   L0 = 0;
   
 done:
   jump $pop_rLink_and_rts;
   
.ENDMODULE;
   
// *****************************************************************************
// MODULE:
//   $a2dp_strip_l2cap_header
//
// DESCRIPTION:
//    Strip the L2CAP header
//
// INPUTS:
//    - r7 =  Pointer to a2dp header structure
//    - I0 =  buffer to read
//    - r4 =  Number of bytes left to read    
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    - r0, r1, r2, r6
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.a2dp_strip_l2cap_header;
   .CODESEGMENT PM_FLASH;

$a2dp_strip_l2cap_header:   
   push rLink;
   
   r0 = M[r7 + $a2dp_decode.CID_FIELD];
   Null = r0;
   if Z jump get_L2CAP_hdr;

 force_rtp_sync:
    // We can't Sync with CID, force sync with RTP if exists
    r2 = M[r7 +  $a2dp_decode.HEADER_TYPE_FIELD];
    Null = r2 AND $a2dp_decode.RTP_TYPE_MASK;
    if Z jump get_L2CAP_hdr;
     call $a2dp_sync2_rtp_header;
     r0 = M[I0, -ADDR_PER_WORD];
     r0 = M[I0, -ADDR_PER_WORD];
     r4 = r4 + $a2dp_decode.BFRAME_HDR_SIZE;

 get_L2CAP_hdr:
   Null = r4 - $a2dp_decode.BFRAME_HDR_SIZE;
   if Z jump done;

   pushm <r3, r5>;
   r6 = M[r7 + $a2dp_decode.GET_BITS_ADDR_FIELD];
   call r6;
   r5 = r1;
   call r6;
   r1 = r1 LSHIFT 8;
   r1 = r1 OR r5;
   push r1;

   call r6;
   r5 = r1;
   call r6;
   r1 = r1 LSHIFT 8;
   r1 = r1 OR r5;
   pop r2;
   popm <r3, r5>;

   // reduce r4
   r4 = r4 - $a2dp_decode.BFRAME_HDR_SIZE;

   // r1 now contains CID
   // r2 contains Length
   r0 = M[r7 + $a2dp_decode.CID_FIELD];
   Null = r0;
   if Z jump cid_verified;
   r0 = r0 - r1;
   if NE jump force_rtp_sync;
   
 cid_verified:
   M[r7 + $a2dp_decode.CID_FIELD] = r1;
   M[r7 + $a2dp_decode.BYTES_LEFT] = r2;
   r3 = r3 - $a2dp_decode.BFRAME_HDR_SIZE;
   
 done:
   jump $pop_rLink_and_rts;
   
.ENDMODULE;

// *****************************************************************************
// MODULE:
//   $a2dp_sync2_rtp_header
//
// DESCRIPTION:
//    Force sync to RTP header
//
// INPUTS:
//    - r7 =  Pointer to a2dp header structure
//    - I0 =  buffer to read
//    - r4 =  Number of bytes left to read    
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    - r0, r1, r2, r6
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.a2dp_sync2_rtp_header;
   .CODESEGMENT PM_FLASH;

$a2dp_sync2_rtp_header:   
   push rLink;
   push r3;

   r10 = r4;
   
 find_rtp:   
   r6 = M[r7 + $a2dp_decode.GET_BITS_ADDR_FIELD];
   call r6;
   r4 = r1;
   r10 = r10 - 1;
   do find_rtp_loop;
      r4 = r4 AND 0x00FF;
      r4 = r4 LSHIFT 8;
      call r6;
      r2 = r4 OR r1;
      r4 = r1;
      r2 = r2 AND $a2dp_decode.RTP_HEADER_MASK;
      Null = r2 - $a2dp_decode.RTP_EXPECTED_HEADER;
      if Z jump found;
   find_rtp_loop:
      // we ran out of data
      r4 = 0;
      jump done;
   
 found:
   r0 = M[I0, -ADDR_PER_WORD]; // I0 = 0x80XX
   r4 = r10 + 2;
   r10 = 0;   
   
 done:
   pop r3;
   jump $pop_rLink_and_rts;
   
.ENDMODULE;


// *****************************************************************************
// MODULE:
//   $a2dp_strip_and_frame_decode
//
// DESCRIPTION:
//    Populates the capability structure with pointers to the asm functions of
//    the codec
//
// INPUTS:
//    - r0 =  Number of samples already generated in this kick (or pass)
//    - r5 =  The codec structure
//    - r3 =  Pointer to a2dp header structure
//    - r4 =  frame decode function
//
// OUTPUTS:
//    - r6 = status
//
// TRASHED REGISTERS:
//    - Almost all registers except r5 
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.a2dp_strip_and_frame_decode;
   .CODESEGMENT PM_FLASH;

$a2dp_strip_and_frame_decode:   
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($a2dp_decode_cap.A2DP_DECODE_CAP_ASM.A2DP_STRIP_AND_FRAME_DECODE.A2DP_STRIP_AND_FRAME_DECODE.PATCH_ID_0, r9)     // cap_compander_patchers
#endif
   
   
   r9 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];

   // a2dp header and decode function must not be null
   Null = r3;
   if Z jump done;

   r7 = r3; 
   r8 = M[r7 + $a2dp_decode.DECODE_FIELD];
   if Z jump done;

   // Bytes left must be 0 to strip. It assumes that any fragmented
   // frames are processed or moved out of buffer to expose the
   // starting of header to strip.
   Null = M[r7 + $a2dp_decode.BYTES_LEFT];
   if NZ jump do_decode;

   /* 
    * Preserve r0 for not being trashed while 
    * stripping the header 
    */
   push r0;
   r3 = M[r7 + $a2dp_decode.HEADER_SIZE_FIELD];
   if NZ call $a2dp_strip_header;
   pop r0;

   Null = r6 - $a2dp_decode.SUCCESS;
   if NE jump done;
    
 do_decode:
   push r7;
   /* 
    * call decoder function after stripping header(r8), 
    * assigned by populate_strip_<codec>_asm_funcs().
    * For aptx, it is aptx_a2dp_frame_decode
    * for SBC . it is sbc_a2dp_frame_decode
    * it returns decoder status on r6 and 
    * r0 returns frame length
    */
   call r8;
   pop r7;
   
   Null = r6 - $a2dp_decode.SUCCESS;
   if NE jump done;
   
   r1 =  M[r7 + $a2dp_decode.BYTES_LEFT];
   r0 = r1 - r0;
   if NEG r0= Null;
   M[r7 + $a2dp_decode.BYTES_LEFT] = r0; 

 done:   
   jump $pop_rLink_and_rts;
   
.ENDMODULE;

#endif /* INSTALL_A2DP_HEADER_SUPPORT */
   
// *****************************************************************************
// MODULE:
//    $_a2dp_decode
//
// DESCRIPTION:
//    Common A2DP decoder C wrapper
//
// INPUTS:
//    - r0 = pointer to the codec's Decoder structure
//    - r1 = pointer to the entry function ( decode function or strip decode )
//    - r2 = decoder mode
//    - r3 = A2DP header structure
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.a2dp_decoder_decode;
   .CODESEGMENT PM_FLASH;

$_a2dp_decoder_decode:
   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   
   PUSH_ALL_C
  
   // The codec libraries expect the Decoder structure in r5
   r5 = r0;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($a2dp_decode_cap.A2DP_DECODE_CAP_ASM.A2DP_DECODER_DECODE.A2DP_DECODER_DECODE.PATCH_ID_0, r4)     // cap_compander_patchers
#endif
   
   
   
   // total number of samples generated
   r0 = 0;
   // total number of input octets consumed
   r6 = 0;
   decode_loop:
   pushm <r0, r1, r2, r3, r6>;

   M[r5 + $codec.DECODER_MODE_FIELD] = r2;

   // If a decode isn't possible this isn't touched so initialise it to 0 before
   // trying to decode.
   M[r5 + $codec.DECODER_NUM_OUTPUT_SAMPLES_FIELD] = 0;

   // also reset number of input octets consumed
   M[r5 + $codec.DECODER_NUM_INPUT_OCTETS_CONSUMED_FIELD] = 0;
  
   // Call the codec library to do a decode
   call r1;

   popm <r0, r1, r2, r3, r6>;

   /* accumulates total octets consumed so far */
   r4 = M[r5 + $codec.DECODER_NUM_INPUT_OCTETS_CONSUMED_FIELD];
   r6 = r6 + r4;

   r4 = M[r5 + $codec.DECODER_NUM_OUTPUT_SAMPLES_FIELD]; 
   if Z jump done;
  
   // Total number of samples now
   r0 = r0 + r4;

   r4 = M[r5 + $codec.DECODER_MODE_FIELD];
   Null = r4 - $a2dp_decode.SUCCESS;
   if NE jump done;

   Null = r0 - $a2dp_decode.OUTPUT_BLOCK_SIZE;
   if LT jump decode_loop;  

   done:
   M[r5 + $codec.DECODER_NUM_OUTPUT_SAMPLES_FIELD] = r0;
   M[r5 + $codec.DECODER_NUM_INPUT_OCTETS_CONSUMED_FIELD] = r6;

   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;

.ENDMODULE;

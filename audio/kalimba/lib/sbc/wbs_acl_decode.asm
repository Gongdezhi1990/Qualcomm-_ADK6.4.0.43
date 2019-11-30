// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// Part of ADK 2.0
//
// *****************************************************************************

#include "core_library.h"
#include "stack.h"
#include "sbc.h"


#ifdef SBC_WBS_ACL

// *****************************************************************************
// MODULE:
//    $wbsdec.init_decoder
//
// DESCRIPTION:
//    TODO
//
// INPUTS:
//    - r5 = decoder structure pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// *****************************************************************************
.MODULE $M.wbsdec.init_decoder;
   .CODESEGMENT WBSDEC_INIT_DECODER_PM;
   .DATASEGMENT DM;

   $wbsdec.init_decoder:

   // push rLink onto stack
   push rLink;
   
   // r9 pointer to data object is loaded in reset_decoder
   // -- initialise encoder variables --
   call $sbcdec.reset_decoder;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $wbsdec.wbs_frame_decode
//
// DESCRIPTION:
//    TODO
//
// INPUTS:
//    - r5 = pointer to a $codec.DECODER_STRUC structure
//    - decoder data structure should hold the data memory pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// *****************************************************************************
.MODULE $M.wbsdec.wbs_frame_decode;
   .CODESEGMENT WBS_SBC_DEC_WBS_FRAME_DECODE_FUNC_PM;
   .DATASEGMENT DM;

$wbsdec.wbs_frame_decode:

   // offsets for the local variables:
   .CONST $wbsdec.local_var.CODEC_STRUC_OFFSET      2*ADDR_PER_WORD;  // FP, rLink
   .CONST $wbsdec.local_var.NUM_LOCAL_VARS          1; 

   // make stack frame for local vars in conjunction with saving FP and rLink
   pushm <FP(=SP), rLink>, SP = SP + ($wbsdec.local_var.NUM_LOCAL_VARS * ADDR_PER_WORD);


   
#if defined(PATCH_LIBS)
   // patch point trashes r0 and r9
   LIBS_SLOW_SW_ROM_PATCH_POINT($wbs.WBS_ACL_DECODE_ASM.WBS_FRAME_DECODE.PATCH_ID_0, r9)  
#endif

   // -- Save $codec.DECODER_STRUC pointer --
   M[FP + $wbsdec.local_var.CODEC_STRUC_OFFSET] = r5;
   
   // -- Load memory structure pointer
   // This pointer should have been initialised externally
   r9 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];

   reattempt_decode:

   // -- Setup SBC input stream buffer info --
   // set I0 to point to cbuffer for sbc input stream
   r5 = M[FP + $wbsdec.local_var.CODEC_STRUC_OFFSET];
   
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];

#ifndef BASE_REGISTER_MODE   
   call $cbuffer.get_read_address_and_size;
   I0 = r0;
   L0 = r1;
#else
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop  B0;
#endif
   

   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
   call $cbuffer.calc_amount_data_in_words;
   r10 = r0 + r0;
   r10 = r10 - 2;
   if LE jump buffer_underflow;
   r4 = r10;
   call $wbsdec.find_sync;
   NULL = r6 - $sbc.SBC_NOT_SYNC;
   if EQ jump buffer_underflow;
      // I2 & r6 holding the bitstream buffer pointer of
      // the location before sync word
      r4 = r4 - r10;

      // -- Check that we have enough output audio space --
      r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
      if Z jump output_check_no_left_channel;
         call $cbuffer.calc_amount_space_in_words;
         Null = r0 - $sbc.MAX_AUDIO_FRAME_SIZE_IN_WORDS;
         if POS jump enough_output_space_left;
            jump not_enough_data_exit;
         enough_output_space_left:
      output_check_no_left_channel:

   // -- Store number of bytes of data available in the SBC stream --
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
   call $cbuffer.calc_amount_data_in_words;
   r0 = r0 + r0;
   r0 = r0 - r4;
   // "+2": adjust by the number of bits we've currently read
   Null = r0 - ($sbc.MIN_SBC_FRAME_SIZE_IN_BYTES + 2);
   if POS jump no_buffer_underflow;

   buffer_underflow:
      // indicate that not enough input data
      r5 = M[FP + $wbsdec.local_var.CODEC_STRUC_OFFSET];
      r0 = $codec.NOT_ENOUGH_INPUT_DATA;
      M[r5 + $codec.DECODER_MODE_FIELD] = r0;
      // store updated cbuffer pointers for sbc input stream
      r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
      r1 = I0;
      call $cbuffer.set_read_address;
      jump exit;
   no_buffer_underflow:
   M[r9 + $sbc.mem.NUM_BYTES_AVAILABLE_FIELD] = r0;


   // -- Read in header --
//   call $sbcdec.read_frame_header;
   call $wbsdec.read_frame_header;


   r0 = M[r9 + $sbc.mem.CUR_FRAME_LENGTH_FIELD];
   r1 = M[r9 + $sbc.mem.NUM_BYTES_AVAILABLE_FIELD];
   Null = r0 - r1;

   if LT jump enough_frame_data;

      // restore buffer pointer back to the sync word
      I0 = I2;
      M[r9 + $sbc.mem.GET_BITPOS_FIELD] = r6;

      jump buffer_underflow;

   enough_frame_data:

   // if corruption in frame then deal with it cleanly
   Null = M[r9 + $sbc.mem.FRAME_CORRUPT_FIELD];

   if NZ jump crc_fail_or_corrupt;


   // -- Read in scalefactors --
   call $sbcdec.read_scale_factors;

   // -- Check CRC --
   r0 = M[r9 + $sbc.mem.FRAMECRC_FIELD];
   r1 = M[r9 + $sbc.mem.CRC_CHECKSUM_FIELD];
   r0 = r0 - r1;

   r0 = r0 AND 0xff;
   if Z jump crc_correct;

      crc_fail_or_corrupt:
      #ifdef DEBUG_SBCDEC
         r0 = M[$sbcdec.framecrc_errors];
         r0 = r0 + 1;
         M[$sbcdec.framecrc_errors] = r0;
      #endif

      // -- Save back SBC input stream buffer info --
      // store updated cbuffer pointers for sbc input stream
      r5 = M[FP + $wbsdec.local_var.CODEC_STRUC_OFFSET];
      r1 = I0;
      r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
      call $cbuffer.set_read_address;
      L0 = 0;
#ifdef BASE_REGISTER_MODE
	  push  0;
	  pop  B0;
#endif
	  

      r0 = M[r9 + $sbc.mem.RETURN_ON_CORRUPT_FRAME_FIELD];

      if Z jump reattempt_decode;

      r0 = $codec.FRAME_CORRUPT;
      M[r5 + $codec.DECODER_MODE_FIELD] = r0;
      jump exit;

   crc_correct:


   // -- Calculate bit allocation --
   call $sbc.calc_bit_allocation;


   // -- Read audio samples --
   call $sbcdec.read_audio_samples;


   // -- Read padding bits --
   call $sbcdec.read_padding_bits;


   // -- Save back SBC input stream buffer info --
   // store updated cbuffer pointers for sbc input stream
   r5 = M[FP + $wbsdec.local_var.CODEC_STRUC_OFFSET];
   r1 = I0;
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
   call $cbuffer.set_read_address;
   L0 = 0;
#ifdef BASE_REGISTER_MODE
   push  0;
   pop  B0;
#endif
   


   // -- Skip further decoding if just doing a dummy frame read --
   r0 = M[r5 + $codec.DECODER_MODE_FIELD];
   Null = r0 - $codec.NORMAL_DECODE;
   if NZ jump all_done;

   // -- Subband sample reconstruction --
   call $sbcdec.sample_reconstruction;


   // -- Synthesis subband filtering Left --
   // set I1 to point to cbuffer for left audio output
   r5 = M[FP + $wbsdec.local_var.CODEC_STRUC_OFFSET];
   r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
   if Z jump no_left_buffer;

   
#ifndef BASE_REGISTER_MODE   
      call $cbuffer.get_write_address_and_size;
      I1 = r0;
      L1 = r1;
#else
      call $cbuffer.get_write_address_and_size_and_start_address;
      I1 = r0;
      L1 = r1;
	  push r2;
	  pop  B1;
#endif
	  
	  
	  
      // select left channel
      r5 = 0;

      // generate output samples
      call $sbcdec.synthesis_subband_filter;

      // store updated cbuffer pointers for left audio output
      r5 = M[FP + $wbsdec.local_var.CODEC_STRUC_OFFSET];
      r1 = I1;
      r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
      call $cbuffer.set_write_address;
      L1 = 0;
#ifdef BASE_REGISTER_MODE
	  push  0;
	  pop  B1;
#endif	  
	  
	  
	  
   no_left_buffer:

   all_done:

   #ifdef DEBUG_SBCDEC
      // -- increment frame counter --
      r0 = M[$sbcdec.framecount];
      r0 = r0 + 1;
      M[$sbcdec.framecount] = r0;
   #endif


   // -- update $codec.DECODER_STRUC --
   r0 = $codec.SUCCESS;
   M[r5 + $codec.DECODER_MODE_FIELD] = r0;

   r0 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];
   r1 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

   r0 = r0 * r1 (int);
   M[r5 + $codec.DECODER_NUM_OUTPUT_SAMPLES_FIELD] = r0;

not_enough_data_exit:
   exit:
   L0 = 0;
   
#ifdef BASE_REGISTER_MODE
	  push  0;
	  pop  B0;
#endif   

   // restore the stack frame and return
   SP = SP - ($wbsdec.local_var.NUM_LOCAL_VARS * ADDR_PER_WORD), popm <FP, rLink>;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $wbsdec.find_sync
//
// DESCRIPTION:
//    TODO
//
// INPUTS:
//    - r9 = memory_pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// *****************************************************************************
.MODULE $M.wbsdec.find_sync;
   .CODESEGMENT WBSDEC_FIND_SYNC_PM;
   .DATASEGMENT DM;

   .CONST        SYNC_WORD         0xAD;

   $wbsdec.find_sync:

   // push rLink onto stack
   push rLink;


   // get byte aligned - we should be anyway but for robustness we force it
   call $sbcdec.byte_align;

   // r10 as input
   DO findsyncloop;
      // backup the bitstream buffer pointer before reading next byte
      r6 = M[r9 + $sbc.mem.GET_BITPOS_FIELD];
      I2 = I0;

      call $sbcdec.get8bits;
      Null = r1 - SYNC_WORD;
      if Z jump found_sync;
   findsyncloop:

   // all available data are searched and sync word not found
   r6 = $sbc.SBC_NOT_SYNC;
   jump $pop_rLink_and_rts;



   found_sync:

   // restore bitstream buffer pointer to just before the sync word
   I0 = I2;
   M[r9 + $sbc.mem.GET_BITPOS_FIELD] = r6;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $wbsdec.read_frame_header
//
// DESCRIPTION:
//    TODO
//
// INPUTS:
//    - r9 = memory_pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - TODO
//
// *****************************************************************************
.MODULE $M.wbsdec.read_frame_header;
   .CODESEGMENT WBSDEC_READ_FRAME_HEADER_PM;
   .DATASEGMENT DM;

   $wbsdec.read_frame_header:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   // patch point trashes r0 and r3
   LIBS_SLOW_SW_ROM_PATCH_POINT($wbs.WBS_ACL_DECODE_ASM.WBSDEC_READ_FRAME_HEADER.PATCH_ID_0, r3) 
#endif

   // default is no faults detected
   M[r9 + $sbc.mem.FRAME_CORRUPT_FIELD] = 0;

   // read in the sync word
   call $sbcdec.get8bits;


   // now decode the rest of the header
   // from after the sync word.

   // crc_checksum = 0x0f
   r0 = 0x0f;
   M[r9 + $sbc.mem.CRC_CHECKSUM_FIELD] = r0;

   call $sbcdec.get8bits;
   call $sbc.crc_calc;

   M[r9 + $sbc.mem.SAMPLING_FREQ_FIELD] = Null;
   r1 = 15;
   M[r9 + $sbc.mem.NROF_BLOCKS_FIELD] = r1;
   M[r9 + $sbc.mem.CHANNEL_MODE_FIELD] = Null;
   r1 = 1;
   M[r9 + $sbc.mem.NROF_CHANNELS_FIELD] = r1;
   M[r9 + $sbc.mem.ALLOCATION_METHOD_FIELD] = Null;
   r1 = 8;
   M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD] = r1;

   // read bitpool
   call $sbcdec.get8bits;
   call $sbc.crc_calc;

   r1 = 26;
   M[r9 + $sbc.mem.BITPOOL_FIELD] = r1;

   // read framecrc
   call $sbcdec.get8bits;
   M[r9 + $sbc.mem.FRAMECRC_FIELD] = r1;

   
   // calculate the frame length - 1  (because already handled syncword)
   // and check that we have enough data to do the frame decode
   call $sbc.calc_frame_length;
   M[r9 + $sbc.mem.CUR_FRAME_LENGTH_FIELD] = r0;


   // pop rLink from stack
   jump $pop_rLink_and_rts;



   corrupt_frame_error:
      r0 = 1;
      M[r9 + $sbc.mem.FRAME_CORRUPT_FIELD] = r0;

      // pop rLink from stack
      jump $pop_rLink_and_rts;

.ENDMODULE;
#endif

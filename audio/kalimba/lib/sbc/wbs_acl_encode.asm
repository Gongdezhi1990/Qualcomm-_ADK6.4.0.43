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
//    $wbsenc.init_encoder
//
// DESCRIPTION:
//    TODO
//
// INPUTS:
//    - r5 = encoder structure pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    TODO
//
// *****************************************************************************

.MODULE $M.wbsenc.init_encoder;
   .CODESEGMENT WBSENC_INIT_ENCODER_PM;
   .DATASEGMENT DM;

   $wbsenc.init_encoder:

   // push rLink onto stack
   push rLink;

   // r9 pointer to data object is loaded in reset_encoder
   // -- initialise encoder variables --
   call $sbcenc.reset_encoder;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $wbsenc.wbs_frame_encode
//
// DESCRIPTION:
//    TODO
//
// INPUTS:
//    - r5 = pointer to a $codec.ENCODER_STRUC structure
//    - encoder data structure should hold the data memory pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    TODO
//
// *****************************************************************************
.MODULE $M.wbsenc.wbs_frame_encode;
   .CODESEGMENT WBS_SBC_ENC_WBS_FRAME_ENCODE_FUNC_PM;
   .DATASEGMENT DM;

$wbsenc.wbs_frame_encode:


   // offsets for the local variables:
   .CONST $wbsenc.local_var.CODEC_STRUC_OFFSET      2*ADDR_PER_WORD;  // FP, rLink
   .CONST $wbsenc.local_var.NUM_LOCAL_VARS          1; 

   // make stack frame for local vars in conjunction with saving FP and rLink
   pushm <FP(=SP), rLink>, SP = SP + ($wbsenc.local_var.NUM_LOCAL_VARS * ADDR_PER_WORD);



   // -- Save $codec.ENCODER_STRUC pointer --
   M[FP + $wbsenc.local_var.CODEC_STRUC_OFFSET] = r5;

   // -- Load memory structure pointer
   // This pointer should have been initialised externally
   r9 = M[r5 + $codec.ENCODER_DATA_OBJECT_FIELD];

   // -- Setup encoder parameters --
   // force mSBC settings
   M[r9 + $sbc.mem.SAMPLING_FREQ_FIELD] = Null;
   r1 = 15;
   M[r9 + $sbc.mem.NROF_BLOCKS_FIELD] = r1;
   M[r9 + $sbc.mem.CHANNEL_MODE_FIELD] = Null;
   r1 = 1;
   M[r9 + $sbc.mem.NROF_CHANNELS_FIELD] = r1;
   M[r9 + $sbc.mem.ALLOCATION_METHOD_FIELD] = Null;
   r1 = 8;
   M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD] = r1;
   r1 = 26;
   M[r9 + $sbc.mem.BITPOOL_FIELD] = r1;
   r1 = 1;
   M[r9 + $sbc.mem.FORCE_WORD_ALIGN_FIELD] = r1;
   M[r9 + $sbc.mem.ENC_SETTING_FORCE_WORD_ALIGN_FIELD] = r1;


   // -- Check that we have enough input audio data --
   r4 = 120;
   r0 = M[r5 + $codec.ENCODER_IN_LEFT_BUFFER_FIELD];
   if Z jump input_check_no_left_channel;
      call $cbuffer.calc_amount_data_in_words;
      Null = r0 - r4;
      if POS jump enough_input_data_left;
         r0 = $codec.NOT_ENOUGH_INPUT_DATA;
         M[r5 + $codec.ENCODER_MODE_FIELD] = r0;
         jump exit;
      enough_input_data_left:
   input_check_no_left_channel:



   // -- Check that we have enough output space for sbc data --
   call $sbc.calc_frame_length;
   r4 = r0 ASHIFT -1;
   // add 3 for safety
   r4 = r4 + 3;
   r0 = M[r5 + $codec.ENCODER_OUT_BUFFER_FIELD];
   call $cbuffer.calc_amount_space_in_words;
   Null = r0 - r4;
   if POS jump enough_output_space;
      r0 = $codec.NOT_ENOUGH_OUTPUT_SPACE;
      M[r5 + $codec.ENCODER_MODE_FIELD] = r0;
      jump exit;
   enough_output_space:


   // -- Analysis subband filtering Left --

   // setup variables for share code block
   call channel_processing;

   // -- Calculate scalefactors --
   call $sbc.calc_scale_factors;

   // -- Setup SBC output stream buffer info --
   // set I0 to point to cbuffer for sbc output stream
   r5 = M[FP + $wbsenc.local_var.CODEC_STRUC_OFFSET];
   r0 = M[r5 + $codec.ENCODER_OUT_BUFFER_FIELD];
   call $cbuffer.get_write_address_and_size;
   I0 = r0;
   L0 = r1;

   // Pre-Processing Hook
   r1 = M[r9 + $sbc.mem.PRE_POST_PROC_STRUC_FIELD];
   r5 = M[FP + $wbsenc.local_var.CODEC_STRUC_OFFSET];

   r0 = M[ r1 + $codec.pre_post_proc.PRE_PROC_FUNC_ADDR_FIELD];
   if NZ call r0;

   // -- Write frame header --
   // init crc_checksum = 0x0f
   r0 = 0x0f;
   M[r9 + $sbc.mem.CRC_CHECKSUM_FIELD] = r0;

   // write wbs header
   r0 = 8;
   r1 = 0xAD;
   call $sbcenc.putbits;
   r1 = 0x00;
   call $sbcenc.putbits;
   call $sbc.crc_calc;
   r1 = 0x00;
   call $sbcenc.putbits;
   call $sbc.crc_calc;

   // do crc on scalefactors
   r0 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];
   r1 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
   r6 = r0 * r1 (int);

   r0 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
   I1 = r0;

   r0 = 4;
   scalefactor_loop:
      r1 = M[I1, MK1];
      call $sbc.crc_calc;
      r6 = r6 - 1;
   if NZ jump scalefactor_loop;

   // write crc check word
   r0 = 8;
   r1 = M[r9 + $sbc.mem.CRC_CHECKSUM_FIELD];
   // select just lower 8 bits
   r1 = r1 AND 0xff;
   call $sbcenc.putbits;


   // -- Write scale factors --
   call $sbcenc.write_scale_factors;


   // -- Calculate bit allocation --
   call $sbc.calc_bit_allocation;


   // -- Quantize samples --
   call $sbcenc.quantize_samples;


   // -- Write audio samples --
   call $sbcenc.write_audio_samples;


   // -- Write padding bits --
   call $sbcenc.write_padding_bits;


   // Post Processing Hook
   r1 = M[r9 + $sbc.mem.PRE_POST_PROC_STRUC_FIELD];
   r5 = M[FP + $wbsenc.local_var.CODEC_STRUC_OFFSET];


   r0 = M[r1 + $codec.pre_post_proc.POST_PROC_FUNC_ADDR_FIELD];
   if NZ call r0;


   // -- Save back SBC input stream buffer info --
   // store updated cbuffer pointers for sbc input stream
   r5 = M[FP + $wbsenc.local_var.CODEC_STRUC_OFFSET];

   r1 = I0;
   r0 = M[r5 + $codec.ENCODER_OUT_BUFFER_FIELD];
   call $cbuffer.set_write_address;
   L0 = 0;


   #ifdef DEBUG_SBCENC
      // -- increment frame counter --
      r0 = M[$sbcenc.framecount];
      r0 = r0 + 1;
      M[$sbcenc.framecount] = r0;
   #endif


   // -- update $codec.ENCODER_STRUC --
   r0 = $codec.SUCCESS;
   M[r5 + $codec.ENCODER_MODE_FIELD] = r0;


   exit:
   
   // restore the stack frame and return
   SP = SP - ($wbsenc.local_var.NUM_LOCAL_VARS * ADDR_PER_WORD), popm <FP, rLink>;
   rts;


channel_processing:

   // push rLink onto stack
   push rLink;

   r0 = M[r5 + $codec.ENCODER_IN_LEFT_BUFFER_FIELD];
   call $cbuffer.get_read_address_and_size;
   I1 = r0;
   L1 = r1;

   // select sample increment per block
   //  = 0 for proper mono
   //  = nrof_subbands for stereo and mono (if converted from a stereo input)
   r6 = 0;
   // select left channel
   r5 = 0;

   // process input samples
   call $sbcenc.analysis_subband_filter;

   // store updated cbuffer pointers for left audio input
   r5 = M[FP + $wbsenc.local_var.CODEC_STRUC_OFFSET];

   r1 = I1;
   r0 = M[r5 + $codec.ENCODER_IN_LEFT_BUFFER_FIELD];
   call $cbuffer.set_read_address;
   L1 = 0;


   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;

#endif

// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    AAC Decoder Library: frame_decode
//
// DESCRIPTION:
//    This library contains functions to decode AAC and AAC+SBR. This function
//    decodes one frame.
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.frame_decode
//
// DESCRIPTION:
//    Decode an AAC frame
//
// INPUTS:
//    - r5 = pointer to a $codec.DECODER_STRUC structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// NOTES:
//    This decoder supports the ADTS (*.aac, *.adts) and MP4 (*.mp4, *.m4a) file
//    formats and the LATM (*.latm) streaming format. There are two ways in
//    which the format can be passed to the library depending on the chip
//    generation. Either the '$aacdec.read_frame_function' variable can be set
//    or the 'KALIMBA_MSG_AACDEC_SET_FILE_TYPE' message can be sent.
//
//    On BC3-MM none of the specific code required for the different formats is
//    included in the library by default. This is due to code size limitations.
//    The particular format should be specified as below. This will make the
//    application 'pull in' the specific code required.
//
//    Code to set the '$aacdec.read_frame_function' should be as follows
//    @verbatim
//    r0 = &$aacdec.adts_read_frame;
//    M[$aacdec.read_frame_function] = r0;
//    @endverbatim
//    or
//    @verbatim
//    r0 = &$aacdec.mp4_read_frame;
//    M[$aacdec.read_frame_function] = r0;
//    @endverbatim
//    or
//    @verbatim
//    r0 = &$aacdec.latm_read_frame;
//    M[$aacdec.read_frame_function] = r0;
//    @endverbatim
//
//    On a BC3-MM, the line 'r0 = ...' pulls the code required for that format
//    into the application so care should be taken to only have one line like
//    this. More than one of these lines may result in a project that won't fit
//    on chip.
//
//    On BC5-MM all of the specific code for each format is already included
//    therefore either the method above or the message sending method can be
//    used. The message that should be sent is 'KALIMBA_MSG_AACDEC_SET_FILE_TYPE'
//    from the VM to Kalimba. The first argument is the file type to use where:
//    @verbatim
//    0 -> mp4
//    1 -> adts
//    2 -> latm
//    @endverbatim
//
// *****************************************************************************

.MODULE $M.aacdec.frame_decode;
   .CODESEGMENT AACDEC_FRAME_DECODE_PM;
   .DATASEGMENT DM;

   $aacdec.frame_decode:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.FRAME_DECODER_ASM.FRAME_DECODE.FRAME_DECODE.PATCH_ID_0, r1)
#endif
   
   
   // -- Start overall profiling if enabled --
   PROFILER_START(&$aacdec.profile_frame_decode)

    // -- Load memory structure pointer
   // This pointer should have been initialised externally
   r9 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];
   // -- Save $codec.DECODER_STRUC pointer --
   M[r9 + $aac.mem.codec_struc] = r5;

   // -- Check that we have enough output audio space --
   // only if not GOBBLING though
   r0 = M[r5 + $codec.DECODER_MODE_FIELD];
   Null = r0 - $codec.GOBBLE_DECODE;
   if Z jump no_output_check_needed;

      // -- Check that we have enough output audio space --
      r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
      if Z jump output_check_no_left_channel;
#ifdef KYMERA
        call $cbuffer.calc_amount_space_in_words;
#else
        call $cbuffer.calc_amount_space;
#endif
         Null = r0 - $aacdec.MAX_AUDIO_FRAME_SIZE_IN_WORDS;
         if POS jump enough_output_space_left;
            r0 = $codec.NOT_ENOUGH_OUTPUT_SPACE;
            M[r5 + $codec.DECODER_MODE_FIELD] = r0;
            jump exit;
         enough_output_space_left:
      output_check_no_left_channel:
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
      if Z jump output_check_no_right_channel;
#ifdef KYMERA
        call $cbuffer.calc_amount_space_in_words;
#else
        call $cbuffer.calc_amount_space;
#endif
         Null = r0 - $aacdec.MAX_AUDIO_FRAME_SIZE_IN_WORDS;
         if POS jump enough_output_space_right;
            r0 = $codec.NOT_ENOUGH_OUTPUT_SPACE;
            M[r5 + $codec.DECODER_MODE_FIELD] = r0;
            jump exit;
         enough_output_space_right:
      output_check_no_right_channel:
   no_output_check_needed:

   reattempt_decode:

   #ifdef AACDEC_SBR_ADDITIONS
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         Null = M[r9 + $aac.mem.SBR_in_synth];
         if NZ jump jump_to_synth;
      #endif
   #endif

   // Clear the flag indicating if ics_ics_info has been called successfully
   M[r9 + $aac.mem.ICS_INFO_DONE_FIELD] = 0;

   // -- Setup aac input stream buffer info --
   // set I0 to point to cbuffer for aac input stream
   r5 = M[r9 + $aac.mem.codec_struc];
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

   // -- Store number of bytes of data available in the aac stream --
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
#ifdef KYMERA
   call $cbuffer.calc_amount_data_in_words;
#else
   call $cbuffer.calc_amount_data;
#endif
   r0 = r0 + r0;
   // adjust by the number of bits we've currently read
   r1 = M[r9 + $aac.mem.GET_BITPOS];
   r1 = r1 ASHIFT -3;
   r0 = r0 + r1;
   r0 = r0 - 2;
   r1 = M[r9 + $aac.mem.WRITE_BYTEPOS];
   r0 = r0 - r1;
   if NEG r0 = 0;
   M[r9 + $aac.mem.READ_BIT_COUNT] = Null;
   r1 = r0 ASHIFT 3;
   M[r9 + $aac.mem.FRAME_NUM_BITS_AVAIL] = r1;
   M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;


   r4 = $aacdec.MIN_AAC_FRAME_SIZE_IN_BYTES;
   r3 = M[r9 + $aac.mem.READ_FRAME_FUNCTION];

   // check minimum size for MP4 bitstreams
   #ifdef AACDEC_MP4_FILE_TYPE_SUPPORTED
      r2 = $aacdec.MIN_MP4_HEADER_SIZE_IN_BYTES;
      r1 = $aacdec.MIN_MP4_FRAME_SIZE_IN_BYTES;
      r5 = M[r9 + $aac.mem.MP4_HEADER_PARSED];
      Null = r5 AND 0x1;
      if NZ r2 = r1;
      r6 = M[r9 + $aac.mem.READ_FRAME_FUNC_TABLE + (0*ADDR_PER_WORD)];
      Null = r3 - r6;//M[$aacdec.read_frame_func_table + 0];
      if Z r4 = r2;
   #endif

   // check minimum size for ADTS bitstreams
   #ifdef AACDEC_ADTS_FILE_TYPE_SUPPORTED
      r2 = $aacdec.MIN_ADTS_FRAME_SIZE_IN_BYTES;
      r6 = M[r9 + $aac.mem.READ_FRAME_FUNC_TABLE + (1*ADDR_PER_WORD)];
      Null = r3 - r6;
      if Z r4 = r2;
   #endif

   // check minimum size for LATM bitstream
   #ifdef AACDEC_LATM_FILE_TYPE_SUPPORTED
      r2 = $aacdec.MIN_LATM_FRAME_SIZE_IN_BYTES;
      r6 = M[r9 + $aac.mem.READ_FRAME_FUNC_TABLE + (2*ADDR_PER_WORD)];
      Null = r3 - r6;
      if Z r4 = r2;
   #endif

   // check minimum size
   Null = r0 - r4;
   if POS jump no_buffer_underflow;
      buffer_underflow:
      // free all of the mem pools
      call $aacdec.tmp_mem_pool_free_all;
      call $aacdec.frame_mem_pool_free_all;
      // indicate that not enough input data
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = $codec.NOT_ENOUGH_INPUT_DATA;
      M[r5 + $codec.DECODER_MODE_FIELD] = r0;
      // store updated cbuffer pointers for aac input stream
      r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
      r1 = I0;
      call $cbuffer.set_read_address;
      L0 = 0;
      jump exit;
   no_buffer_underflow:


   // -- Read adts, mp4, or latm frame --
   PROFILER_START(&$aacdec.profile_read_frame)
   r0 = M[r9 + $aac.mem.READ_FRAME_FUNCTION];
   call r0;
   PROFILER_STOP(&$aacdec.profile_read_frame)

   // if corruption in file then deal with it cleanly
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump crc_fail_or_corrupt;

   // if buffer underflow will occur then exit here
   Null = M[r9 + $aac.mem.FRAME_UNDERFLOW];
   if NZ jump buffer_underflow;

   // Check that ics_info has been called successfully before reconstructing channels
   // (otherwise force a frame corrupt)
   // (this is a workaround for B-110537 that was causing a DSP crash)
   null = M[r9 + $aac.mem.ICS_INFO_DONE_FIELD];
   if NZ jump crc_correct;
   // -- Check CRC --
   // currently checking the crc is not supported

      crc_fail_or_corrupt:
      #ifdef DEBUG_AACDEC
         r0 = M[r9 + $aac.mem.FRAME_CORRUPT_ERRORS];
         r0 = r0 + 1;
         M[r9 + $aac.mem.FRAME_CORRUPT_ERRORS] = r0;
      #endif

      // free all of the mem pools
      call $aacdec.tmp_mem_pool_free_all;
      call $aacdec.frame_mem_pool_free_all;

      // -- Save back aac input stream buffer info --
      // store updated cbuffer pointers for aac input stream
      // this will mean that next time we'll look after the crc_fail/corruption
      // and hopefully find a good frame
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
      r1 = I0;
      call $cbuffer.set_read_address;
      L0 = 0;
#ifdef BASE_REGISTER_MODE
      push Null; pop B0;
#endif
      r2 = M[r9 + $aac.mem.GET_BITPOS];   // reset_decoder will reset get_bitpos
      call $aacdec.reset_decoder;   // but we do not want that here
      r1 = r2 AND 0x7;                   // make sure it will stay byte-aligned in case of corrupt frame
      r2 = r2 - r1;
      M[r9 + $aac.mem.GET_BITPOS] = r2;
#ifdef INSTALL_METADATA
      r0 = $codec.FRAME_CORRUPT;
      M[r5 + $codec.DECODER_MODE_FIELD] = r0;
      jump exit;
#else
      jump reattempt_decode;
#endif /* INSTALL_METADATA */
   crc_correct:


   // -- Save back AAC input stream buffer info --
   // store updated cbuffer pointers for aac input stream
   r5 = M[r9 + $aac.mem.codec_struc];
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
   r1 = I0;
   call $cbuffer.set_read_address;
   L0 = 0;
#ifdef BASE_REGISTER_MODE
   push Null; pop B0;
#endif

   // -- Skip further decoding if just doing a dummy frame read --
   r0 = M[r5 + $codec.DECODER_MODE_FIELD];
   Null = r0 - $codec.NORMAL_DECODE;
   if EQ jump jump_to_synth;

   // free all frame memory
   call $aacdec.frame_mem_pool_free_all;
   // free all tmp memory again
   call $aacdec.tmp_mem_pool_free_all;

   jump dummy_decode_tidyup;


   jump_to_synth:

   // call reconstruct_channels()
   call $aacdec.reconstruct_channels;
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump crc_fail_or_corrupt;

   #ifdef AACDEC_SBR_ADDITIONS
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
      Null = M[r9 + $aac.mem.SBR_in_synth];
      if NZ jump exit;
      #endif
   #endif

   dummy_decode_tidyup:


   #ifdef DEBUG_AACDEC
      // -- Increment frame counter --
      r0 = M[r9 + $aac.mem.FRAME_COUNT];
      r0 = r0 + 1;
      M[r9 + $aac.mem.FRAME_COUNT] = r0;
   #endif


   // -- update $codec.DECODER_STRUC --
   r5 = M[r9 + $aac.mem.codec_struc];
   r0 = $aacdec.MAX_AUDIO_FRAME_SIZE_IN_WORDS;
   #ifdef AACDEC_SBR_ADDITIONS
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         r1 = M[r5 + $codec.DECODER_MODE_FIELD];
         Null = r1 - $codec.GOBBLE_DECODE;
         if NE jump not_in_gobble_mode;
            Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
            if Z jump sbr_not_present;
               r0 = r0 * 2 (int);
            sbr_not_present:
         not_in_gobble_mode:
      #endif
   #endif
   M[r5 + $codec.DECODER_NUM_OUTPUT_SAMPLES_FIELD] = r0;
   r0 = $codec.SUCCESS;
   M[r5 + $codec.DECODER_MODE_FIELD] = r0;


   exit:
   // -- Stop overall profiling if enabled --
   PROFILER_STOP(&$aacdec.profile_frame_decode)

   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;

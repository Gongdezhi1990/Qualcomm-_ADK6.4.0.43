// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.mp4_sequence
//
// DESCRIPTION:
//    Run through the 'atoms' in an mp4 file and extract the relavant information
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//   - r0-r8, M0, M1
//
// *****************************************************************************
.MODULE $M.aacdec.mp4_sequence;
   .CODESEGMENT AACDEC_MP4_SEQUENCE_PM;
   .DATASEGMENT DM;

   $aacdec.mp4_sequence:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.MP4_SEQUENCE_ASM.MP4_SEQUENCE.MP4_SEQUENCE.PATCH_ID_0, r1)
#endif
   
   
   // default is no faults detected
   M[r9 + $aac.mem.FRAME_CORRUPT] = Null;

   // re-enter mp4_moov_routine if previously entered and ran out of data
   Null = M[r9 + $aac.mem.MP4_IN_MOOV];
   if NZ jump moov_atom;

   // re-enter mp4_discard_remainder_of_sub_atom if previously entered and ran out of data
   Null = M[r9 + $aac.mem.MP4_IN_DISCARD_ATOM_DATA];
   if NZ call $aacdec.mp4_discard_atom_data;
   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump possible_corruption;


   Null = M[r9 + $aac.mem.MP4_SEQUENCE_FLAGS_INITIALISED];
   if NZ jump flags_already_initialised;
      r0 = 1;
      M[r9 + $aac.mem.MP4_SEQUENCE_FLAGS_INITIALISED] = r0;
      // found_first_mdat = 0;
      M[r9 + $aac.mem.FOUND_FIRST_MDAT] = Null;
      // found_moov = 0;
      M[r9 + $aac.mem.FOUND_MOOV] = Null;
   flags_already_initialised:


   // find moov atom and extract sampling frequency amd no. of channels from it
   // then find first mdat atom and exit
   mp4_sequence_outer_loop:

   error_no_skip_function:

      // check if enough data available to parse next atom
      r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
      Null = r0 - $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
      if POS jump data_available;
         r0 = 1;
         M[r9 + $aac.mem.FRAME_UNDERFLOW] = r0;
         // return back to mp4_parse_header
         jump $pop_rLink_and_rts;
      data_available:

      r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
      r0 = r0 - $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
      M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;

      //    - r4 = most significant bytes of atom_size
      //    - r5 = least significant 3 bytes of atom_size
      //    - r6 = least significant 2 bytes of atom_name
      //    - r7 = most significant 2 bytes of atom_name
      call $aacdec.mp4_read_atom_header;

      // switch( atom_name )

      // case ( mdat )
      Null = r7 - $aacdec.MP4_MDAT_TAG_MS_WORD;
      if NZ jump not_mdat_atom;
         Null = r6 - $aacdec.MP4_MDAT_TAG_LS_WORD;
         if NZ jump not_mdat_atom;
            r2 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)];
            r1 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)];
            r1 = r1 + $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
            r2 = r2 + Carry;
            r4 = M[r9 + $aac.mem.MDAT_OFFSET + (0*ADDR_PER_WORD)];
            r3 = M[r9 + $aac.mem.MDAT_OFFSET + (1*ADDR_PER_WORD)];
            Null = r3 OR r4;
            if Z jump mdat_offset_unknown;
            r1 = r1 - r3;
            r2 = r2 - r4 - Borrow;
            Null = r1 + r2;
            if NZ jump incorrect_mdat;

         mdat_offset_unknown:
            // if(found_moov==1)
            Null = M[r9 + $aac.mem.FOUND_MOOV];
            if NZ jump break_from_loop;

            r0 = 1;
            M[r9 + $aac.mem.FOUND_FIRST_MDAT] = r0;   // found_mdat_first = 1;
         incorrect_mdat:
            r0 = r5;
            r1 = r4;
            call $aacdec.update_mp4_file_offset;
            r6 = M[r9 + $aac.mem.SKIP_FUNCTION];
            if NZ jump do_skip;
               // throw away rest of this atom
               r5 = r5 - $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
               r4 = r4 - Borrow;
               call $aacdec.mp4_discard_atom_data;
               Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
               if NZ jump possible_corruption;
               jump error_no_skip_function;
            do_skip:
               r3 = r5 - $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
               r4 = r4 - Borrow;
               call $aacdec.skip_through_file;
               jump mp4_sequence_outer_loop;

      // case ( moov )
      not_mdat_atom:

      r0 = r7 - $aacdec.MP4_MOOV_TAG_MS_WORD;
      if NZ jump not_moov_atom;
         r0 = r6 - $aacdec.MP4_MOOV_TAG_LS_WORD;
         if NZ jump not_moov_atom;
            M[r9 + $aac.mem.moov_size_lo] = r5;
            M[r9 + $aac.mem.moov_size_hi] = r4;
            r0 = 1;
            M[r9 + $aac.mem.FOUND_MOOV] = r0;   // found_moov = 1;

            moov_atom:

            // extract sampling frequency and no. of channels from moov atom
            call $aacdec.mp4_moov_routine;
            Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
            if NZ jump possible_corruption;
            Null = M[r9 + $aac.mem.FRAME_UNDERFLOW];
            if NZ jump $pop_rLink_and_rts;

            r0 = M[r9 + $aac.mem.moov_size_lo];
            r1 = M[r9 + $aac.mem.moov_size_hi];
            call $aacdec.update_mp4_file_offset;

            // if(found_mdat_first==1)
            Null = M[r9 + $aac.mem.FOUND_FIRST_MDAT];
            if Z jump mp4_sequence_outer_loop;
               r6 = M[r9 + $aac.mem.SKIP_FUNCTION];
               if Z jump error_no_skip_function;
               r2 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)];
               r1 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)];
               r1 = r1 + 8;
               r2 = r2 + Carry;
               r4 = M[r9 + $aac.mem.MDAT_OFFSET + (0*ADDR_PER_WORD)];
               r3 = M[r9 + $aac.mem.MDAT_OFFSET + (1*ADDR_PER_WORD)];
               r3 = r3 - r1;
               r4 = r4 - r2 - Borrow;
               call $aacdec.skip_through_file;
               r1 = M[r9 + $aac.mem.MDAT_OFFSET + (0*ADDR_PER_WORD)];
               r0 = M[r9 + $aac.mem.MDAT_OFFSET + (1*ADDR_PER_WORD)];
               r0 = r0 - $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
               r1 = r1 - Borrow;

               M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)] = r1;
               M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)] = r0;

               jump mp4_sequence_outer_loop;

      // case ( otherwise )
      not_moov_atom:
      r0 = r5;
      r1 = r4;
      call $aacdec.update_mp4_file_offset;

      // throw away rest of this atom
      r5 = r5 - $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
      r4 = r4 - Borrow;
      call $aacdec.mp4_discard_atom_data;
      Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
      if NZ jump possible_corruption;


      jump mp4_sequence_outer_loop;
   break_from_loop:
   r0 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)];
   r1 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)];
   r0 = r0 + $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
   r1 = r1 + Carry;
   M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)] = r1;
   M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)] = r0;

   // set flag to indicate mp4 header parsing is complete
   r0 = 1;
   r0 = r0 AND 0x1;
   M[r9 + $aac.mem.MP4_HEADER_PARSED] = r0;


   possible_corruption:

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $aacdec.skip_through_file
//
// DESCRIPTION:
//    wrapper function to call the skip function
//
// INPUTS:
//    - r4 = MS byte of skip size
//    - r3 = LS 3 bytes of skip size
//    - r6 = external skip function pointer
//
// OUTPUTS:
//
// TRASHED REGISTERS:
// assume everything
//
// *****************************************************************************
.MODULE $M.aacdec.skip_through_file;
   .CODESEGMENT AACDEC_MP4_SEQUENCE_PM;
   .DATASEGMENT DM;

   $aacdec.skip_through_file:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.MP4_SEQUENCE_ASM.SKIP_THROUGH_FILE.SKIP_THROUGH_FILE.PATCH_ID_0, r1)
#endif
   

   // seeking is relative to read pointer, so move the read pointer
   // to the current word
   r5 = M[r9 + $aac.mem.codec_struc];
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
   r1 = I0;
   call $cbuffer.set_read_address;

   // fix bitpos if needed
   // seek_value   bitpos      fix
   // ----------------------------------------------------------------
   // even          x          not required
   // odd           >7         bitpos-=8, seek_value-=1
   // odd           <8         bitpos+=8, seek_value+=1
   Null = r3 AND 1;
   if Z jump no_fix_needed;
      r0 = M[r9 + $aac.mem.GET_BITPOS];
      r0 = r0 - 8;
      if POS jump fix_finished;
         r3 = r3 + 1;
         r4 = r4 + carry;
         r0 = r0 + 16;
      fix_finished:
      M[r9 + $aac.mem.GET_BITPOS] = r0;
      r3 = r3 AND 0xFFFFFE;
   no_fix_needed:

   // decide whether seek is required
   Null = r4;
   if NZ jump seek_required;         // seek if negative or too big
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
#ifdef KYMERA
   call $cbuffer.calc_amount_data_in_words;
#else
   call $cbuffer.calc_amount_data;
#endif
   r1 = r3 LSHIFT -1;
   Null = r0 - r1;
   if LE jump seek_required;         // seek if not enough data
      // just skip words in the input buffer
      Words2Addr(r1);
      M0 = r1;
      r0 = M[I0, M0];
      r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
      r1 = I0;
      call $cbuffer.set_read_address;
      jump seek_done;
   // call external seek function
   seek_required:
   call r6;

   // update to the new state
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
   seek_done:
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
   if NEG r0 = 0;
   M[r9 + $aac.mem.READ_BIT_COUNT] = Null;
   r1 = r0 ASHIFT 3;
   M[r9 + $aac.mem.FRAME_NUM_BITS_AVAIL] = r1;
   M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;
   jump $pop_rLink_and_rts;
.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $aacdec.update_mp4_file_offset
//
// DESCRIPTION:
//    function to update the file offset
//
// INPUTS:
//    - r0 = File offset increment LSW
//    - r1 = File offset increment MSW
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//  r2, r3
//
// *****************************************************************************
.MODULE $M.aacdec.update_mp4_file_offset;
   .CODESEGMENT AACDEC_MP4_SEQUENCE_PM;
   .DATASEGMENT DM;

   $aacdec.update_mp4_file_offset:

   r2 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)];
   r3 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)];
   r3 = r0 + r3;
   r2 = r1 + r2 + Carry;
   M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)] = r2;
   M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)] = r3;
   rts;

.ENDMODULE;

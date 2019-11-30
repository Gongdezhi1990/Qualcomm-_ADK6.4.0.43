// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.mp4_moov_routine
//
// DESCRIPTION:
//    Process the MOOV atom of the mp4 header
//
// INPUTS:
//    - r4 = MS byte of moov_atom_size
//    - r5 = LS 3 bytes of moov_atom_size
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - r0-r8, r10, I1, M3
//
// *****************************************************************************
.MODULE $M.aacdec.mp4_moov_routine;
   .CODESEGMENT AACDEC_MP4_MOOV_ROUTINE_PM;
   .DATASEGMENT DM;

   $aacdec.mp4_moov_routine:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.MP4_MOOV_ROUTINE_ASM.MP4_MOOV_ROUTINE.MP4_MOOV_ROUTINE.PATCH_ID_0, r8)
#endif
  


   // if(already entered mp4_moov_routine previously)
   Null = M[r9 + $aac.mem.MP4_IN_MOOV];
   if NZ jump mp4_moov_routine_resume;

   // set flag to say have entered moov_routine
   r0 = 1;
   M[r9 + $aac.mem.MP4_IN_MOOV] = r0;

   // save moov size. Assuming tmp won't be thrashed while moov is being read
   M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)] = r5;
   M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = r4;

   // store (moov_atom_size - 8) : used as the outer loop bound
   r5 = r5 - $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
   r4 = r4 - Borrow;
   M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_LS] = r5;
   M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_MS] = r4;

   mp4_moov_routine_resume:

   r8 = 0;
   // if (stsz/stz2 parsing not finished)
   r4 = M[r9 + $aac.mem.SAMPLE_COUNT + (0*ADDR_PER_WORD)];
   r5 = M[r9 + $aac.mem.SAMPLE_COUNT + (1*ADDR_PER_WORD)];
   Null = r4 OR r5;
   if Z jump not_stsz;
      // Record bit count
      r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
      M[r9 + $aac.mem.TMP] = r0;
      jump stsz_variable_sample_size;

   not_stsz:

   // if(not finished discarding a sub atom within moov atom)
   Null = M[r9 + $aac.mem.MP4_IN_DISCARD_ATOM_DATA];
   if NZ call $aacdec.mp4_discard_atom_data;
   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump $pop_rLink_and_rts;


   // while(not finished parsing moov atom)
   //    parse next sub-atom
   mp4_moov_routine_outer_loop_bound_test:

      // if(ran out of data when discarding a sub atom of moov atom)
      Null = M[r9 + $aac.mem.MP4_IN_DISCARD_ATOM_DATA];
      if NZ jump $pop_rLink_and_rts;   // return back to mp4_sequence

      r0 = M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_MS];
      if NZ jump mp4_moov_routine_outer_loop;
         r0 = M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_LS];
         if NZ jump mp4_moov_routine_outer_loop;
            // exit
            M[r9 + $aac.mem.MP4_IN_MOOV] = Null;
            // resume with mp4_sequence
            jump $pop_rLink_and_rts;
      mp4_moov_routine_outer_loop:

      // check if reasonable amount of data
      r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
      Null = r0 - $aacdec.MP4_MOOV_ATOM_MIN_NUM_BYTES;
      if POS jump data_available;
         Null = M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_MS];
         if NZ jump underflow;
         r1 = M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_LS];
         Null = r0 - r1;
         if POS jump data_available;
         underflow:
            r0 = 1;
            M[r9 + $aac.mem.FRAME_UNDERFLOW] = r0;
            // return back to mp4_sequence
            jump $pop_rLink_and_rts;
      data_available:

      r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
      r0 = r0 - $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
      M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;

      //    - r4 = most significant byte of sub_atom_size
      //    - r5 = least significant 3 bytes of sub_atom_size
      //    - r6 = least significant 2 bytes of sub_atom_name
      //    - r7 = most significant 2 bytes of sub_atom_name
      call $aacdec.mp4_read_atom_header;

      // switch_case( sub_atom_name )
      Null = r7 - $aacdec.MP4_TRAK_TAG_MS_WORD;
      if NZ jump not_trak;
         Null = r6 - $aacdec.MP4_TRAK_TAG_LS_WORD;
         if Z jump enter_atom;
      not_trak:

      Null = r7 - $aacdec.MP4_MDIA_TAG_MS_WORD;
      if NZ jump not_mdia;
         Null = r6 - $aacdec.MP4_MDIA_TAG_LS_WORD;
         if Z jump enter_atom;
      not_mdia:

      Null = r7 - $aacdec.MP4_MINF_TAG_MS_WORD;
      if NZ jump not_minf;
         Null = r6 - $aacdec.MP4_MINF_TAG_LS_WORD;
         if Z jump enter_atom;
      not_minf:

      Null = r7 - $aacdec.MP4_STBL_TAG_MS_WORD;
      if NZ jump skip_atom;
      //STXX atom. Check if it is STBL
      Null = r6 - $aacdec.MP4_STBL_TAG_LS_WORD;
      if Z jump enter_atom;

      // Check if it is STSD
      Null = r6 - $aacdec.MP4_STSD_TAG_LS_WORD;
      if Z jump parse_stsd;

      // Check if it is STSZ
      Null = r6 - $aacdec.MP4_STSZ_TAG_LS_WORD;
      if Z jump parse_stsz;

      // Check if it is STCO
      Null = r6 - $aacdec.MP4_STCO_TAG_LS_WORD;
      if Z jump parse_stco;

      // Check if it is STSS
      // JUST SKIP DIRECTLY!
      jump skip_atom;


   enter_atom:
      //Enter the atom and search for sub-atoms
      // parsed_counter += 8;
      r5 = 8;
      r4 = 0;
      call update_outer_loop_bound;
      jump mp4_moov_routine_outer_loop_bound_test;

      // case ( stsd )
      parse_stsd:
            // Update remaining moov atom size (assuming we parse this one to the end)
            call update_outer_loop_bound;

            // Record bit count
            r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
            M[r9 + $aac.mem.TMP] = r0;

            // -- Read the stsd atom --
            // discard 57 - 13 bytes till we get to esds fullbox then skip 13 bytes
            // into esds (including header, length, etc.)
            // it is assumed that the file has been parsed by media parser and
            // its compatibility verified (e.g. only 1 entry in stds and that's mp4a)
            r10 = 57;
            call $aacdec.discard_some_bytes;

            // Read and ignore ES_descriptor length
            call $aacdec.read_uintvar4;

            // Read and ignore ES_ID
            call $aacdec.get2bytes;

            // Read some flags and calculate the number of bytes to skip
            call $aacdec.get1byte;
            r10 = 0;
            r0 = 2;
            Null = r1 AND 0x80;
            if NZ r10 = r10 + r0;
            Null = r1 AND 0x40;
            if NZ r10 = r10 + r0;
            Null = r1 AND 0x20;
            if NZ r10 = r10 + r0;
            call $aacdec.discard_some_bytes;

            // Read and ignore DecoderConfigDescriptor Tag
            call $aacdec.get1byte;

            // Read and ignore DecoderConfigDescriptor length
            call $aacdec.read_uintvar4;

            // Read 13 bytes from DecoderConfigDescriptor and ignore them
            // then read the tag for DecoderSpecificInfo
            r10 = 14;
            call $aacdec.discard_some_bytes;

            // Read and ignore DecoderSpecificInfo length
            call $aacdec.read_uintvar4;

            // read the audio_specific_config
            call $aacdec.audio_specific_config;

            call $aacdec.byte_align;

            // Calculate number of bytes read in mp4a atom
            r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
            r2 = M[r9 + $aac.mem.TMP];
            r0 = r0 - r2;
            r1 = r0 ASHIFT -3;

            r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
            r0 = r0 - r1;
            M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;
            // This 8 bytes is the atom header size, deducted from num_bytes_available when
            // header was read:
            r1 = r1 + 8;


            // throw away rest of the atom
            r5 = r5 - r1;
            r4 = r4 - Borrow;
            call $aacdec.mp4_discard_atom_data;
            Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
            if NZ jump $pop_rLink_and_rts;


            jump mp4_moov_routine_outer_loop_bound_test;

      // case ( stsz )
      parse_stsz:
         // Get actual moov size
         r0 = M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)];
         r1 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];

         // calculate bytes read in moov

         r3 = M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_MS];
         r2 = M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_LS];
         r0 = r0 - r2;//M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_LS];
         r1 = r1 - r3 - Borrow;

         // Calculate STSZ offset in the file

         r2 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)];
         r3 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)];
         r0 = r0 + r3;
         r1 = r1 + r2 + Carry;

         // store stsz position
         M[r9 + $aac.mem.STSZ_OFFSET + (0*ADDR_PER_WORD)] = r1;
         M[r9 + $aac.mem.STSZ_OFFSET + (1*ADDR_PER_WORD)] = r0;

         // Update remaining moov atom size (assuming we parse this one to the end)
         call update_outer_loop_bound;

         // -- Read the stsz atom --
         // Record bit count
         r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
         M[r9 + $aac.mem.TMP] = r0;

          // discard 4 bytes
         call $aacdec.get2bytes;
         call $aacdec.get2bytes;

         // Read sample size
         call $aacdec.get1byte;
         r6 = r1;
         call $aacdec.get2bytes;
         r1 = r1 LSHIFT 8;
         r7 = r1;
         call $aacdec.get1byte;
         r7 = r7 + r1;
         // now sample size = {r6,r7}

         // read sample count
         call $aacdec.get1byte;
         r4 = r1;
         call $aacdec.get2bytes;
         r1 = r1 LSHIFT 8;
         r5 = r1;
         call $aacdec.get1byte;
         r5 = r5 + r1;
         // now sample count = {r4,r5}

         M[r9 + $aac.mem.SAMPLE_COUNT + (0*ADDR_PER_WORD)] = r4;
         M[r9 + $aac.mem.SAMPLE_COUNT + (1*ADDR_PER_WORD)] = r5;

         r8 = 12; //bytes read till here
         // if sample size is zero, read size of each sample and add
         Null = r7 OR r6;
         if Z jump stsz_variable_sample_size;

            // fixed sample size
            // sample size = {r6,r7}; - ie. r0 is MSW and r1 is LSW
            // sample count = {r4,r5};
            // Total size = {mdat_size[0](16 LS bits),mdat_size[1],mdat_size[2]}; - ie. Z is 64-bit

            rMAC = r7 * r5 (UU); // Compute LSW

            r1 = rMAC LSHIFT 23;
            M[r9 + $aac.mem.MDAT_SIZE + (2*ADDR_PER_WORD)] = r1;

            rMAC0 = rMAC1; // shift right 24-bits
            rMAC12 = rMAC2 (ZP);
            rMAC = rMAC + r6 * r5 (UU); // compute inner products
            rMAC = rMAC + r4 * r7 (UU);

            r1 = rMAC LSHIFT 23;
            M[r9 + $aac.mem.MDAT_SIZE + (1*ADDR_PER_WORD)] = r1;

            rMAC0 = rMAC1; // shift right 24-bits
            rMAC12 = rMAC2 (ZP);
            rMAC = rMAC + r6 * r4 (UU); // compute MSWs

            r1 = rMAC LSHIFT 23;
            r1 = r1 AND 0xFFFF;
            M[r9 + $aac.mem.MDAT_SIZE] = r1;

              // Calculate number of bytes read in mp4a atom
            r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
            r1 = M[r9 + $aac.mem.TMP];
            r0 = r0 - r1;
            r1 = r0 ASHIFT -3;

            r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
            r0 = r0 - r1;
            M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;
          jump mp4_moov_routine_outer_loop_bound_test;

         stsz_variable_sample_size:

            r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
            //Subtract num bytes used.r8 = 0 if resumed, else 12
            r0 = r0 - r8;
            // Get num samples avaiable. 4 bytes per sample
            r0 = r0 LSHIFT -2;

            // if r4 > 0, r10 = r0, else r10 = min(r5,r0)
            r10 = r5;
            Null = r4;
            //check if r4 > 0
            if NZ r10 = r0;
            Null = r0 - r10;
            if NEG r10 = r0; //underflow


            r6 = r9 + $aac.mem.SAMPLE_COUNT + (0*ADDR_PER_WORD);
            r7 = r9 + $aac.mem.SAMPLE_COUNT + (1*ADDR_PER_WORD);
            M[r7] =  r5 - r10;
            M[r6] = r4 - Borrow;
            r7 = M[r9 + $aac.mem.MDAT_SIZE + (2*ADDR_PER_WORD)];
            r6 = M[r9 + $aac.mem.MDAT_SIZE + (1*ADDR_PER_WORD)];
            r5 = M[r9 + $aac.mem.MDAT_SIZE + (0*ADDR_PER_WORD)];

            do stsz_mdat_loop;
               // Read and add sample size
               call $aacdec.get1byte;
               r6 = r6 + r1;
               r5 = r5 + Carry;
               call $aacdec.get2bytes;
               r1 = r1 LSHIFT 8;
               r8 = r1;
               call $aacdec.get1byte;
               r8 = r8 + r1;
               r7 = r7 + r8;
               r6 = r6 + Carry;
               r5 = r5 + Carry;
            stsz_mdat_loop:

            // store the size
            M[r9 + $aac.mem.MDAT_SIZE + (2*ADDR_PER_WORD)] = r7;
            M[r9 + $aac.mem.MDAT_SIZE + (1*ADDR_PER_WORD)] = r6;
            M[r9 + $aac.mem.MDAT_SIZE + (0*ADDR_PER_WORD)] = r5;

            //check if all done
            r4 = M[r9 + $aac.mem.SAMPLE_COUNT + (0*ADDR_PER_WORD)];
            r5 = M[r9 + $aac.mem.SAMPLE_COUNT + (1*ADDR_PER_WORD)];
            Null = r4 OR r5;
            if NZ jump underflow;

            // Calculate number of bytes read in mp4a atom
            r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
            r1 = M[r9 + $aac.mem.TMP];
            r0 = r0 - r1;
            r1 = r0 ASHIFT -3;

            r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
            r0 = r0 - r1;
            M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;

            jump mp4_moov_routine_outer_loop_bound_test;

      parse_stco:

         call update_outer_loop_bound;

         // discard 8 bytes
         call $aacdec.get2bytes;
         call $aacdec.get2bytes;
         call $aacdec.get2bytes;
         call $aacdec.get2bytes;

         call $aacdec.get1byte;
         r6 = r1;
         call $aacdec.get2bytes;
         r1 = r1 LSHIFT 8;
         r7 = r1;
         call $aacdec.get1byte;
         r7 = r7 + r1;
         // now mdat offset = {r6,r7}

         M[r9 + $aac.mem.MDAT_OFFSET + (0*ADDR_PER_WORD)] = r6;
         M[r9 + $aac.mem.MDAT_OFFSET + (1*ADDR_PER_WORD)] = r7;

         r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
         r0 = r0 - 12;
         M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;

         // throw away rest of the atom
         r5 = r5 - ($aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES + 12);
         r4 = r4 - Borrow;
         call $aacdec.mp4_discard_atom_data;
         Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
         if NZ jump $pop_rLink_and_rts;


         jump mp4_moov_routine_outer_loop_bound_test;

#if 0
      parse_stss:
         // Get actual moov size
         r0 = M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)];
         r1 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];

         // calculate bytes read in moov
         r0 = r0 - M[$aacdec.mp4_moov_atom_size_ls];
         r1 = r1 - M[$aacdec.mp4_moov_atom_size_ms] - Borrow;

         // Calculate STSS offset in the file
         r0 = r0 + M[$aacdec.mp4_file_offset + (1*ADDR_PER_WORD)];
         r1 = r1 + M[$aacdec.mp4_file_offset + (0*ADDR_PER_WORD)] + Carry;

         // store STSS position
         M[r9 + $aac.mem.STSS_OFFSET + (1*ADDR_PER_WORD)] = r0;
         M[r9 + $aac.mem.STSS_OFFSET + (0*ADDR_PER_WORD)] = r1;

         call update_outer_loop_bound;

         // discard 4 bytes
         call $aacdec.get2bytes;
         call $aacdec.get2bytes;

         call $aacdec.get1byte;
         r6 = r1;
         call $aacdec.get2bytes;
         r1 = r1 LSHIFT 8;
         r7 = r1;
         call $aacdec.get1byte;
         r7 = r7 + r1;
         Null = r7 OR r1;
         if NZ jump random_seek_possible;
            // No random seek samples in file
            // Set STSS offset to -1 to indicate this
            r0 = -1;
            M[r9 + $aac.mem.STSS_OFFSET + (1*ADDR_PER_WORD)] = r0;
            M[r9 + $aac.mem.STSS_OFFSET + (0*ADDR_PER_WORD)] = r0;

         random_seek_possible:

         // throw away rest of the atom
         r5 = r5 - ($aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES + 12);
         r4 = r4 - Borrow;
         call $aacdec.mp4_discard_atom_data;
         Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
         if NZ jump $pop_rLink_and_rts;

         jump mp4_moov_routine_outer_loop_bound_test;
#endif

      // case ( otherwise )
      skip_atom:

      call update_outer_loop_bound;

      // throw away rest of the atom
      r5 = r5 - $aacdec.MP4_ATOM_NAME_AND_SIZE_BYTES;
      r4 = r4 - Borrow;
      call $aacdec.mp4_discard_atom_data;
      Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
      if NZ jump $pop_rLink_and_rts;


      jump mp4_moov_routine_outer_loop_bound_test;



// ********************************************************
//  DESCRIPTION:
//    sub-routine to increment parsed_counter by amount specified as :
//
// bit   47               31     24 23                       0
//       |           r4           | |           r5           |
//       |0000000000000000|   mp4_moov_atom_size (32 bits)   |
//
//  INPUTS:
//          - r4
//          - r5
//
//  OUTPUTS:
//          - none
//
// TRASHED REGISTERS:
//    - r0, r1,
//
// ********************************************************
   update_outer_loop_bound:
   push r8;
   push r7;

   r8 = r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_LS;
   r7 = r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_MS;

   r0 = M[r8];
   r1 = M[r7];


   M[r8] = r0 - r5;
   M[r7] = r1 - r4 - Borrow;
   pop r7;
   pop r8;

   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $aacdec.discard_some_bytes
//
// DESCRIPTION:
//    aacdec.discard_some_bytes
//
// INPUTS:
//    - r10 = number of bytes to discard
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r10
//
// *****************************************************************************
.MODULE $M.aacdec.discard_some_bytes;
   .CODESEGMENT AACDEC_DISCARD_SOME_BYTES_PM;
   .DATASEGMENT DM;

   $aacdec.discard_some_bytes:

   // push rLink onto stack
   push rLink;

   do discard_bytes_loop;
      call $aacdec.get1byte;
   discard_bytes_loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $aacdec.read_uintvar4
//
// DESCRIPTION:
//    aacdec.read_uintvar4 reads a uintvar of upto size 4 from the bitstream
//    only upto 23 are actually read and it is assumed that the value of the
//    uint is smaller than 2^23 - 1, otherwise saturates - this really shouldn't
//    happen though.
//
// INPUTS:
//    - what ever needed for $aacdec.get1byte
//
// OUTPUTS:
//    - r0 = uintvar read from the stream
//
// TRASHED REGISTERS:
//    - r0-r3, r10
//
// *****************************************************************************
.MODULE $M.aacdec.read_uintvar4;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $aacdec.read_uintvar4:

   // push rLink onto stack
   push rLink;

   push r4;
   r4 = 0;
   r10 = 4;
   do discard_bytes_loop;
      call $aacdec.get1byte;
      r4 = r4 ASHIFT 7;
      r0 = r1 AND 0x7F;
      r4 = r4 + r0;
      Null = r1 AND 0x80;
      if Z jump uintvar_ended;
   discard_bytes_loop:

   uintvar_ended:
   r0 = r4;
   pop r4;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.huffman_start
//
// DESCRIPTION:
//    Set things up ready for reading huffman values
//
// INPUTS:
//    - I0,L0 = buffer to read words from (for aacdec.getbits)
//
// OUTPUTS:
//    - I0,L0 = updated (for aacdec.huffman)
//    - r2 = current word from bitstream
//    - r3 = current bitmask to use to read from bitstream
//    - M0 = 0
//    - M1 = 1
//
// TRASHED REGISTERS:
//    - none
//
// *****************************************************************************
.MODULE $M.aacdec.huffman_start;
   .CODESEGMENT AACDEC_HUFFMAN_START_PM;
   .DATASEGMENT DM;

   $aacdec.huffman_start:

   // store value or I0 and get_bitpos for later
   r2 = I0;
   M[r9 + $aac.mem.GETBITS_SAVED_I0] = r2;

   // set registers using the aac_getbits state
   // r3 = bitmask - form it from the bitpos
   r2 = M[r9 + $aac.mem.GET_BITPOS];
   M[r9 + $aac.mem.GETBITS_SAVED_BITPOS] = r2;
   if NZ jump dont_read_another_word;
      r2 = M[I0, MK1];
      r2 = 16;

   dont_read_another_word:
   r3 = 1;
   r2 = r2 - r3;
   r3 = r3 LSHIFT r2,
    r2 = M[I0, MK1];    // r2 = current word from bitstream, I0 = ptr to next word

   // set a few constants required for huffman decoding
   M0 = 0;
   M1 = 1;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $aacdec.huffman_finish
//
// DESCRIPTION:
//    Set things up ready for not reading huffman values anymore
//
// INPUTS:
//    - I0,L0 = buffer to read words from (for aacdec.huffman)
//    - r3 = current bitmask to use to read from bitstream
//
// OUTPUTS:
//    - I0 = updated (for aacdec.getbits)
//
// TRASHED REGISTERS:
//    - r0, r2, r3
//
// *****************************************************************************
.MODULE $M.aacdec.huffman_finish;
   .CODESEGMENT AACDEC_HUFFMAN_FINISH_PM;
   .DATASEGMENT DM;

   $aacdec.huffman_finish:

   push r1;
   // set the aac_getbits state from the current registers
   // adjust I0 back 1 word
   r2 = M[I0, -MK1];

   // update get_bitpos from current value of r3
   r3 = SIGNDET r3;
   r3 = (DAWTH-1) - r3;
   M[r9 + $aac.mem.GET_BITPOS] = r3;

   // update 'read_bit_count' for the bits that have been read as huffman data
   r0 = M[r9 + $aac.mem.GETBITS_SAVED_I0];
   r0 = I0 - r0;
   if NEG r0 = r0 + L0;
   r0 = r0 * (16/ADDR_PER_WORD) (int);
   r1 = M[r9 + $aac.mem.GETBITS_SAVED_BITPOS];
   r0 = r0 + r1; /// to check if r1 can be used ravneet
   r2 = M[r9 + $aac.mem.READ_BIT_COUNT];
   r0 = r0 + r2;           // M[$aacdec.read_bit_count];
   r0 = r0 - r3;
   M[r9 + $aac.mem.READ_BIT_COUNT] = r0;
   pop r1;
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.huffman_getpair
//
// DESCRIPTION:
//    Read huffman pair of values
//
// INPUTS:
//    - I0,L0 = buffer to read words from
//    - I1 = buffer to output decoded words to
//    - I2 = pointer to the hufftable to use
//    - r2 = current word from bitstream
//    - r3 = current bitmask to use to read from bitstream
//    - r4 = huffman table to use
//    - r7 = -12;
//    - M0 = 0
//    - M1 = 1
//
// OUTPUTS:
//    - I0,L0 = updated
//    - I1 = updated
//    - r2 = updated
//    - r3 = updated
//
// TRASHED REGISTERS:
//    - r1, r5, r6, r10, I2
//
// *****************************************************************************
.MODULE $M.aacdec.huffman_getpair;
   .CODESEGMENT AACDEC_HUFFMAN_GETPAIR_PM;
   .DATASEGMENT DM;

   $aacdec.huffman_getpair:

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.HUFFMAN_DECODE_ASM.HUFFMAN_GETPAIR.HUFFMAN_GETPAIR.PATCH_ID_0, r1)
#endif
   
   huff_loop:

      // mask out current bit from bitstream
      Null = r2 AND r3,
       r1 = M[I2,M0];       // read huffman node data

      // if bit=1 then take high 12bits of huffman node
      if NZ r1 = r1 LSHIFT r7;

      // form next bitmask
      r3 = r3 LSHIFT -1;
      if NZ jump no_new_word_needed;
         // read next word from bitstream
         r2 = M[I0, MK1];
         // set bitmask to 0x8000
         r3 = Null OR 0x8000;
      no_new_word_needed:

      // AND off the lower 12bits of huffman node
      r1 = r1 AND 0xFFF;

      // if bit 11 set then huffman code has ended
      r0 = r1 - 0x800;
      if POS jump code_ended;

      // move huffman pointer onto the next node in the binary tree
      Words2Addr(r1);
      I2 = I2 + r1;

   jump huff_loop;

   code_ended:
   // separate the 2 values
   // r0 = y,  r1 = z;
   r1 = r0 AND 0x1F;
   r0 = r0 LSHIFT -5;


   // deal with sign bits or offsetting huffman value
   r5 = M[r9 + $aac.mem.HUFFMAN_OFFSETS_FIELD];
   r6 = r4 * ADDR_PER_WORD (int);
   r5 = M[r5 + r6];
   if Z jump read_sign_bits;

      // add the offset to each value
      r0 = r0 + r5;
      r1 = r1 + r5,
       M[I1, MK1] = r0;       // write decoded word y to buffer
      // write decoded word z to buffer
      M[I1, MK1] = r1;
      rts;

   read_sign_bits:
      // read extra sign bits

      r5 = Null - r0;
      if Z jump no_sign_bit_for_y;
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=1 then y = -y
         if NZ r0 = r5;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_sign_bit_for_y;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;

      no_sign_bit_for_y:

      r6 = Null - r1;
      if Z jump no_sign_bit_for_z;
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=1 then z = -z
         if NZ r1 = r6;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_sign_bit_for_z;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;

      no_sign_bit_for_z:


   // if the escape codebook is being used y could = ESC_FLAG
   // if so read the hcod_esc_y field
   Null = r5 + $aacdec.ESC_FLAG;
   if NZ jump no_esc_field_y;

      // read the hcod_esc_y field
      r5 = 0;
      // count the number of 1's in the stream until a 0 is found
      esc_y_length_loop:
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=0 then at end of length field
         if Z jump esc_y_length_end;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_new_word_needed_esc_y_len;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;
         no_new_word_needed_esc_y_len:

         // if > 8 ones then invalid hcod_esc
         r5 = r5 + 1;
         Null = r5 - 8;
      if LE jump esc_y_length_loop;
      // invalid hcod_esc - errored stream, so set possible corruption flag
      #ifdef AACDEC_CALL_ERROR_ON_POSSIBLE_CORRUPTION
         call $error;
      #else
         r0 = 1;
         M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION] = r0;
         jump no_esc_field_y;
      #endif


      esc_y_length_end:
      // form next bitmask
      r3 = r3 LSHIFT -1;
      if NZ jump no_new_word_needed_esc_y_len_end;
         // read next word from bitstream
         r2 = M[I0, MK1];
         // set bitmask to 0x8000
         r3 = Null OR 0x8000;
      no_new_word_needed_esc_y_len_end:


      // now need to read N+4 bits of offset
      r10 = r5 + 4;
      r5 = 2;
      do esc_offset_y_loop;
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=1 then add 1 to the offset word being formed
         if NZ r5 = r5 + M1;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_new_word_needed_esc_offset_y;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;

         no_new_word_needed_esc_offset_y:
         // shift by 1 the offset word being formed
         r5 = r5 LSHIFT 1;

      esc_offset_y_loop:
      // undo the last shift by 1 of the offset word being formed
      r5 = r5 LSHIFT -1;

      // adjust the sign to match the non esc code read
      Null = r0;
      if NEG r5 = -r5;
      r0 = r5;

   no_esc_field_y:



   // if the escape codebook is being used z could = ESC_FLAG
   // if so read the hcod_esc_z field
   Null = r6 + $aacdec.ESC_FLAG;
   if NZ jump no_esc_field_z;

      // read the hcod_esc_z field
      r5 = 0;
      // count the number of 1's in the stream until a 0 is found
      esc_z_length_loop:
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=0 then at end of length field
         if Z jump esc_z_length_end;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_new_word_needed_esc_z_len;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;
         no_new_word_needed_esc_z_len:

         // if > 8 ones then invalid hcod_esc
         r5 = r5 + 1;
         Null = r5 - 8;
      if LE jump esc_z_length_loop;
      // invalid hcod_esc - errored stream, so set possible corruption flag
      #ifdef AACDEC_CALL_ERROR_ON_POSSIBLE_CORRUPTION
         call $error;
      #else
         r0 = 1;
         M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION] = r0;
         jump no_esc_field_z;
      #endif


      esc_z_length_end:
      // form next bitmask
      r3 = r3 LSHIFT -1;
      if NZ jump no_new_word_needed_esc_z_len_end;
         // read next word from bitstream
         r2 = M[I0, MK1];
         // set bitmask to 0x8000
         r3 = Null OR 0x8000;
      no_new_word_needed_esc_z_len_end:


      // now need to read N+4 bits of offset
      r10 = r5 + 4;
      r5 = 2;
      do esc_offset_z_loop;
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=1 then add 1 to the offset word being formed
         if NZ r5 = r5 + M1;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_new_word_needed_esc_offset_z;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;

         no_new_word_needed_esc_offset_z:
         // shift by 1 the offset word being formed
         r5 = r5 LSHIFT 1;

      esc_offset_z_loop:
      // undo the last shift by 1 of the offset word being formed
      r5 = r5 LSHIFT -1;

      // adjust the sign to match the non esc code read
      Null = r1;
      if NEG r5 = -r5;
      r1 = r5;

   no_esc_field_z:

   // write decoded word y to buffer
   M[I1, MK1] = r0;

   // write decoded word z to buffer
   M[I1, MK1] = r1;
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.huffman_getquad
//
// DESCRIPTION:
//    Read huffman quad values
//
// INPUTS:
//    - I0,L0 = buffer to read words from
//    - I1 = buffer to write decoded words to
//    - I2 = pointer to the hufftable to use
//    - r2 = current word from bitstream
//    - r3 = current bitmask to use to read from bitstream
//    - r4 = huffman table to use
//    - r7 = -12;
//    - M0 = 0
//    - M1 = 1
//
// OUTPUTS:
//    - I0,L0 = updated
//    - I1 = updated
//    - r2 = updated
//    - r3 = updated
//
// TRASHED REGISTERS:
//    - r1, r5, r6, I2
//
// *****************************************************************************
.MODULE $M.aacdec.huffman_getquad;
   .CODESEGMENT AACDEC_HUFFMAN_GETQUAD_PM;
   .DATASEGMENT DM;

   $aacdec.huffman_getquad:

   huff_loop:

      // mask out current bit from bitstream
      Null = r2 AND r3,
       r1 = M[I2,M0];       // read huffman node data

      // if bit=1 then take high 12bits of huffman node
      if NZ r1 = r1 LSHIFT r7;

      // form next bitmask
      r3 = r3 LSHIFT -1;
      if NZ jump no_new_word_needed;
         // read next word from bitstream
         r2 = M[I0, MK1];
         // set bitmask to 0x8000
         r3 = Null OR 0x8000;
      no_new_word_needed:

      // AND off the lower 12bits of huffman node
      r1 = r1 AND 0xFFF;

      // if bit 11 set then huffman code has ended
      r0 = r1 - 0x800;
      if POS jump code_ended;

      // move huffman pointer onto the next node in the binary tree
      Words2Addr(r1);
      I2 = I2 + r1;

   jump huff_loop;

   code_ended:

   // deal with sign bits or offsetting huffman value
   r6 = M[r9 + $aac.mem.HUFFMAN_OFFSETS_FIELD];
   r5 = r4 * ADDR_PER_WORD (int);
   r6 = M[r6 + r5];
   if Z jump read_sign_bits;

      // separate the 4 values and add the offset
      // extract w (MS 2 bits)
      r5 = r0 AND 0xC0;
      r5 = r5 LSHIFT -6;
      r5 = r5 + r6;

      // extract x (next 2 bits)
      r1 = r0 AND 0x30;
      r1 = r1 LSHIFT -4;
      r1 = r1 + r6,
       M[I1, MK1] = r5;       // write decoded word w to buffer

      // extract y (next 2 bits)
      r5 = r0 AND 0xC;
      r5 = r5 LSHIFT -2;
      r5 = r5 + r6,
       M[I1, MK1] = r1;       // write decoded word x to buffer

      // extract z (LS 2 bits)
      r1 = r0 AND 0x3;
      r1 = r1 + r6,
       M[I1, MK1] = r5;       // write decoded word y to buffer
      // write decoded word z to buffer
      M[I1, MK1] = r1;

      rts;

   read_sign_bits:
      // read extra sign bits

      // extract w (MS 2 bits)
      r5 = r0 AND 0xC0;
      r5 = r5 LSHIFT -6;
      if Z jump no_sign_bit_for_w;
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=1 then w = -w
         if NZ r5 = -r5;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_sign_bit_for_w;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;

      no_sign_bit_for_w:
      // write decoded word w to buffer
      M[I1, MK1] = r5;


      // extract x (next 2 bits)
      r5 = r0 AND 0x30;
      r5 = r5 LSHIFT -4;
      if Z jump no_sign_bit_for_x;
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=1 then x = -x
         if NZ r5 = -r5;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_sign_bit_for_x;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;

      no_sign_bit_for_x:
      // write decoded word w to buffer
      M[I1, MK1] = r5;


      // extract y (next 2 bits)
      r5 = r0 AND 0xC;
      r5 = r5 LSHIFT -2;
      if Z jump no_sign_bit_for_y;
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=1 then y = -y
         if NZ r5 = -r5;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_sign_bit_for_y;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;

      no_sign_bit_for_y:
      // write decoded word y to buffer
      M[I1, MK1] = r5;


      // extract z (LS 2 bits)
      r5 = r0 AND 0x3;
      if Z jump no_sign_bit_for_z;
         // mask out current bit from bitstream
         Null = r2 AND r3;

         // if bit=1 then z = -z
         if NZ r5 = -r5;

         // form next bitmask
         r3 = r3 LSHIFT -1;
         if NZ jump no_sign_bit_for_z;
            // read next word from bitstream
            r2 = M[I0, MK1];
            // set bitmask to 0x8000
            r3 = Null OR 0x8000;

      no_sign_bit_for_z:
      // write decoded word z to buffer
      M[I1, MK1] = r5;

   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.huffman_getsf
//
// DESCRIPTION:
//  Read huffman scalefactor value
//
// INPUTS:
//    - I0,L0 = buffer to read words from
//    - r2 = current word from bitstream
//    - r3 = current bitmask to use to read from bitstream
//    - M0 = 0
//
// OUTPUTS:
//    - I0,L0 = updated
//    - r0 = the huffman scalefactor
//    - r2 = updated
//    - r3 = updated
//
// TRASHED REGISTERS:
//    - r1, r5, I1
//
// *****************************************************************************
.MODULE $M.aacdec.huffman_getsf;
   .CODESEGMENT AACDEC_HUFFMAN_GETSF_PM;
   .DATASEGMENT DM;

   $aacdec.huffman_getsf:

   // I2 points to the hufftable to use
   r0 = M[r9 + $aac.mem.huffman_cb_table + 0];
   I1 = r0;
   r5 = -12;

   huff_loop:

      // mask out current bit from bitstream
      Null = r2 AND r3,
       r1 = M[I1,M0];       // read huffman node data

      // if bit=1 then take high 12bits of huffman node
      if NZ r1 = r1 LSHIFT r5;

      // form next bitmask
      r3 = r3 LSHIFT -1;
      if NZ jump no_new_word_needed;
         // read next word from bitstream
         r2 = M[I0, MK1];
         // set bitmask to 0x8000
         r3 = Null OR 0x8000;
      no_new_word_needed:

      // AND off the lower 12bits of huffman node
      r1 = r1 AND 0xFFF;

      // if bit 11 set then huffman code has ended
      r0 = r1 - 0x800;
      if POS rts;

      // move huffman pointer onto the next node in the binary tree
      Words2Addr(r1);
      I1 = I1 + r1;
   jump huff_loop;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.huffman_getbits
//
// DESCRIPTION:
//     Get bits from input buffer that can be called when we're in the middle of
//     reading huffman data.   It's needed for reading noise_pcm scalefactors
//
// INPUTS:
//    - I0,L0 = buffer to read words from
//    - r5 = number of bits to get from buffer
//    - r2 = current word from bitstream
//    - r3 = current bitmask to use to read from bitstream
//    - M1 = 1
//
// OUTPUTS:
//    - I0 = updated
//    - r1 = the data read from the buffer
//    - r2 = updated
//    - r3 = updated
//
// TRASHED REGISTERS:
//    - r5
//
// *****************************************************************************
.MODULE $M.aacdec.huffman_getbits;
   .CODESEGMENT AACDEC_HUFFMAN_GETBITS_PM;
   .DATASEGMENT DM;

   $aacdec.huffman_getbits:

   r1 = 0;
   getbits_loop:

      // mask out current bit from bitstream
      Null = r2 AND r3;

      // if bit=1 then set LSB of result
      if NZ r1 = r1 + M1;

      // form next bitmask
      r3 = r3 LSHIFT -1;
      if NZ jump no_new_word_needed;
         // read next word from bitstream
         r2 = M[I0, MK1];
         // set bitmask to 0x8000
         r3 = Null OR 0x8000;
      no_new_word_needed:

      // shift result by 1
      r1 = r1 LSHIFT 1;
      r5 = r5 - 1;
   if NZ jump getbits_loop;

   // shift result by -1
   r1 = r1 LSHIFT -1;
   rts;

.ENDMODULE;

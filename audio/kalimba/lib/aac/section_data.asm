// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.section_data
//
// DESCRIPTION:
//    Get section data
//
// INPUTS:
//    - r4 = pointer to the current ICS structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r8, r10, I1-I5
//
// *****************************************************************************
.MODULE $M.aacdec.section_data;
   .CODESEGMENT AACDEC_SECTION_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.section_data:

   // push rLink onto stack
   push rLink;

   // if (window_sequence == EIGHT_SHORT_SEQUENCE)
      // sect_bits = 3;
   // else (window_sequence != EIGHT_SHORT_SEQUENCE)
      // sect_bits = 5;
   r7 = 3;
   r1 = 5;
   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if NZ r7 = r1;


   // allocate tmp memory for sect_cb    size: (num_window_groups * max_sfb)
   //  - it's not needed after spectral huffman data has been read
   r1 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
   r6 = M[r4 + $aacdec.ics.NUM_WINDOW_GROUPS_FIELD];
   r0 = r6 * r1 (int);
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;
   M[r4 + $aacdec.ics.SECT_CB_PTR_FIELD] = r1;
   // set I1 -> sect_cb[0][0]
   I1 = r1;

   // allocate tmp memory for sect_start size: (num_window_groups * max_sfb)
   //  - it's not needed after spectral huffman data has been read
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;
   M[r4 + $aacdec.ics.SECT_START_PTR_FIELD] = r1;
   // set I2 -> sect_start[0][0]
   I2 = r1;

   // allocate tmp memory for sect_end   size: (num_window_groups * max_sfb)
   //  - it's not needed after spectral huffman data has been read
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;
   M[r4 + $aacdec.ics.SECT_END_PTR_FIELD] = r1;
   // set I3 -> sect_end[0][0]
   I3 = r1;

   // allocate tmp memory for sfb_cb   size: (num_window_groups * max_sfb)
   //  - it's not needed after reading of scalefactors
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;
   M[r4 + $aacdec.ics.SFB_CB_PTR_FIELD] = r1;
   // set I4 -> sfb_cb[0][0]
   I4 = r1;

   // I5 -> num_sec[0]
   I5 = r4 + $aacdec.ics.NUM_SEC_FIELD;

   r6 = 0;
   // for g=0:num_window_groups,
   num_win_groups_loop:

      // i=0;
      r5 = 0;
      // k=0;
      r8 = 0;

      // while (k <max_sfb)
      start_k_while_loop:
         // r0 = max_sfb
         Null = r8 - r0;
         if POS jump end_while_k_loop;

         // sect_cb(g,i) = getbits(4);
         call $aacdec.get4bits;
         // codebook 12 is reserved so probable corruption if it occurs
         Null = r1 - 12;
         if Z jump $aacdec.possible_corruption;
         M[I1,0] = r1;

         // sect_len = 0;
         r10 = 0;

         // sect_len_incr = getbits(sect_bits);
         // while (sect_len_incr == sect_esc_val)
            // sect_len += sect_esc_val;
            // sect_len_incr = getbits(sect_bits);
         // sect_len += sect_len_incr;
         sect_len_while_loop:
            r0 = r7;
            call $aacdec.getbits;
            // sect_len += getbits
            r10 = r10 + r1;
            // form sect_esc_val mask
            r0 = 1 LSHIFT r7;
            r0 = r0 - 1;
            Null = r1 - r0;
         if Z jump sect_len_while_loop;


         // sect_start(g,i) = k;
         r0 = r8;
         M[I2, MK1] = r0;

         // sect_end(g,i) = k + sect_len;
         r0 = r0 + r10;
         M[I3, MK1] = r0;

         // k += sect_len;
         r8 = r8 + r10;

         // i++;
         r5 = r5 + 1;

         // check neither i or k are now > max_sfb
         // otherwise data corruption would occur
         r0 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
         Null = r0 - r8;
         if NEG jump $aacdec.possible_corruption;
         Null = r0 - r5;
         if NEG jump $aacdec.possible_corruption;


         // for sfb = k: k + sect_len -1      (the k here is before k += sect_len)
            // sfb_cb(g,sfb) = sect_cb(g,i);
         // Note: ISO spec doesn't handle case of sect_len = 0 well
         // r0 = sect_cb
         r1 = M[I1, MK1];
         do sfb_cb_loop;
            M[I4, MK1] = r1;
         sfb_cb_loop:

         jump start_k_while_loop;
      end_while_k_loop:

      // num_sec(g) = i;
      M[I5, MK1] = r5;

      r6 = r6 + 1;
      r0 = M[r4 + $aacdec.ics.NUM_WINDOW_GROUPS_FIELD];
      Null = r6 - r0;
   if NZ jump num_win_groups_loop;


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.tns_data
//
// DESCRIPTION:
//    Get TNS (temporal noise shaping) data
//
// INPUTS:
//    - r1 = tns present
//    - r4 = current ics pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r2, r5-r8, r10, I1-I7, M2, M3
//
// *****************************************************************************
.MODULE $M.aacdec.tns_data;
   .CODESEGMENT AACDEC_TNS_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.tns_data:

   // push rLink onto stack
   push rLink;

   r6 = r1; // store whether or not there is tns data in this channel

   // allocate temp memory for tns data allocate both channels at same time so that
   // space allocated for huffman can be freed earlier
   // - it's needed up to just before the filter bank
   Null = M[r9 + $aac.mem.CURRENT_CHANNEL];
   if NZ jump already_allocated;
      r0 = $aacdec.tns.STRUC_SIZE;
      call $aacdec.frame_mem_pool_allocate;
      if NEG jump $aacdec.possible_corruption;
      M[r9 + $aac.mem.ICS_LEFT + $aacdec.ics.TNS_DATA_PTR_FIELD] = r1;

      Null = M[r9 + $aac.mem.NUM_SCEs];
      if NZ jump no_right_ch;
         call $aacdec.frame_mem_pool_allocate;
         if NEG jump $aacdec.possible_corruption;
         M[r9 + $aac.mem.ICS_RIGHT + $aacdec.ics.TNS_DATA_PTR_FIELD] = r1;
      no_right_ch:
   already_allocated:

   Null = r6;
   if Z jump no_tns;

   r1 = M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD];

   I1 = r1 + $aacdec.tns.N_FILT_FIELD;
   I2 = r1 + $aacdec.tns.COEF_RES_FIELD;
   I3 = r1 + $aacdec.tns.LENGTH_FIELD;
   I4 = r1 + $aacdec.tns.ORDER_FIELD;
   I5 = r1 + $aacdec.tns.DIRECTION_FIELD;
   I6 = r1 + $aacdec.tns.COEF_COMPRESS_FIELD;
   I7 = r1 + $aacdec.tns.COEF_FIELD;

   // if (window_sequence == EIGHT_SHORT_SEQUENCE)
   // {
   //    n_filt_num_bits = 1;
   //    length_num_bits = 4;
   //    order_num_bits = 3;
   // }
   // else
   // {
   //    n_filt_num_bits = 2;
   //    length_num_bits = 6;
   //    order_num_bits = 5;
   // }

   // for w=0:num_windows,
   // {
   //    n_filt(w) = getbits(n_filt_num_bits);
   //    if (n_filt(w) != 0)
   //    {
   //       coef_res(w) = getbits(1);
   //       coef_num_bits = 3 + coef_res(w);
   //    }
   //    for filt = 0:n_filt(w)-1,
   //    {
   //       length(w,filt) = getbits(length_num_bits);
   //       order(w,filt) = getbits(order_num_bits);
   //       if (order(w,filt) != 0)
   //       {
   //          direction(w,filt) = getbits(1);
   //          coef_compress(w,filt) = getbits(1);
   //          coef_bits = coef_num_bits - coef_compress(w,filt);
   //          for i=0:order(w,filt)-1,
   //          {
   //             coef(w,filt,i) = getbits(coef_num_bits);
   //          }
   //       }
   //    }
   // }


   // r5 = n_filt_num_bits
   // r6 = length_num_bits
   // r7 = order_num_bits
   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if Z jump eight_short_sequence;
      M2 = 2;
      M3 = 6;
      r6 = 5;
      M1 = $aacdec.TNS_MAX_ORDER_LONG;
      jump endif_win_seq;
   eight_short_sequence:
      M2 = 1;
      M3 = 4;
      r6 = 3;
      M1 = $aacdec.TNS_MAX_ORDER_SHORT;
   endif_win_seq:

   r8 = M[r4 + $aacdec.ics.NUM_WINDOWS_FIELD];
   num_windows_loop:

      // get n_filt
      r0 = M2;
      call $aacdec.getbits;
      M[I1, MK1] = r1;
      r7 = r1;
      if Z jump n_filt_zero;

         // get coef_res(w)
         call $aacdec.get1bit;
         M[I2,0] = r1;

         // coef_num_bits = 3 + coef_res(w)
         r5 = r1 + 3;

         filt_loop:
            // get length(w,filt)
            r0 = M3;
            call $aacdec.getbits;
            M[I3, MK1] = r1;

            // get order(w,filt)
            r0 = r6;
            call $aacdec.getbits;
            Null = r1 - M1;     // check TNS "order" is not out of bounds
            if GT jump $aacdec.possible_corruption;
            M[I4, MK1] = r1;
            r10 = r1;

            if Z jump order_zero;
               // get direction(w,filt);
               call $aacdec.get1bit;
               M[I5, MK1] = r1;

               // get coef_compress(w,filt);
               call $aacdec.get1bit;
               M[I6, MK1] = r1;

               // coefbits = coef_num_bits - coef_compress
               r0 = r5 - r1;

               do order_loop;
                 // get coef(w,filt,i);
                  call $aacdec.getbits;
                  M[I7, MK1] = r1;
               order_loop:
            order_zero:
            r7 = r7 - 1;
         if NZ jump filt_loop;

      n_filt_zero:
      I2 = I2 + ADDR_PER_WORD;
      r8 = r8 - 1;
   if NZ jump num_windows_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


no_tns:
   M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD] = Null;
   jump $pop_rLink_and_rts;

.ENDMODULE;

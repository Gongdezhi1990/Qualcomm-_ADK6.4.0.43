// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.reorder_spec
//
// DESCRIPTION:
//    Reorder the spectral data for short windows
//
// INPUTS:
//    - r4 = current ics pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r5-r8, r10, I1-I7,
//
// *****************************************************************************
.MODULE $M.aacdec.reorder_spec;
   .CODESEGMENT AACDEC_REORDER_SPEC_PM;
   .DATASEGMENT DM;

   $aacdec.reorder_spec:

   // push rLink onto stack
   push rLink;

   // The decoding tools from now on all require the spectral coefficients to be
   // in a non-interleaved fashion. I.e. all spectral coefs to be ordered
   // according to window number and frequency within a window (spec[w][k]).
   // Currently the short window sequences are interleaved as
   // spec[g][w][sfb][bin].  And so need to be converted.

   // start_win_ptr = 0;
   // k = 0;
   // for g=0:ics(ch).num_window_groups-1,
   //   j = 0;
   //   start_k = k;
   //   win_inc = ics(ch).swb_offset(ics(ch).num_swb+1);
   //   for sfb=0:ics(ch).num_swb-1,
   //
   //      width = ics(ch).swb_offset(sfb+2) - ics(ch).swb_offset(sfb+1);
   //      win_ptr = start_win_ptr;
   //
   //      for win=0:ics(ch).window_group_length(g+1)-1,
   //         m = win_ptr + j;
   //
   //         for bin=0:width-1,
   //            spec(ch,m+1) = x_rescale(ch,k+1);
   //            k = k + 1;
   //            m = m + 1;
   //         end
   //
   //         win_ptr = win_ptr + win_inc;
   //
   //      end
   //      j = j + width;
   //
   //   end
   //   start_win_ptr = start_win_ptr + (k - start_k);
   // end

   // allocate tmp memory for reordering into
   r0 = 1024;
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.corruption;
   I2 = r1;
   I7 = r1;
   I5 = r1;

   r0 = M[r9 + $aac.mem.CURRENT_SPEC_PTR];
   I1 = r0;

   // I3 = &window_group_length(g)
   r0 = r4 + $aacdec.ics.WINDOW_GROUP_LENGTH_FIELD;
   I3 = r0;

   r8 = M[r4 + $aacdec.ics.NUM_WINDOW_GROUPS_FIELD];
   win_group_loop:

      // j = 0;
      r6 = 0;
      // I6 = start_k = k;
      I6 = I1;
      // I4 = &swb_offset[0]
      r0 = M[r4 + $aacdec.ics.SWB_OFFSET_PTR_FIELD];
      I4 = r0;
      r5 = M[I4, MK1];

      sfb_loop:
         // r7 = width = swb_offset(sfb+1) - swb_offset(sfb);
         r3 = M[I4, MK1];
         r7 = r3 - r5;
         r5 = r3;
         // win_ptr = start_win_ptr;
         r1 = I5;
         // r2 = window_group_length(g)
         r2 = M[I3,0];
         win_loop:
            // I2 = m = win_ptr + j;
            Words2Addr(r6);
            I2 = r1 + r6;
            Addr2Words(r6);
            r10 = r7;
            do bin_loop;
               r0 = M[I1, MK1];
               M[I2, MK1] = r0;
            bin_loop:
            // r1 = win_ptr = win_ptr + win_inc;
            r1 = r1 + (128*ADDR_PER_WORD);
            r2 = r2 - 1;
         if NZ jump win_loop;
         // r6 = j = j + width;
         r6 = r6 + r7;
         Null = r3 - 128;
      if NZ jump sfb_loop;
      // I5 = start_win_ptr = start_win_ptr + (k - start_k);
      I5 = I5 + I1;
      I5 = I5 - I6,
       r0 = M[I3, MK1];
      r8 = r8 - 1;
   if NZ jump win_group_loop;

   // now copy the reordered data back into the spec buffer
   r10 = 1024;
   r0 = M[r9 + $aac.mem.CURRENT_SPEC_PTR];
   I1 = r0;
   do copy_back_loop;
      r0 = M[I7, MK1];
      M[I1, MK1] = r0;
   copy_back_loop:

   // free previously allocated temporary memory
   r0 = 1024;
   call $aacdec.tmp_mem_pool_free;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


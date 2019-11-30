// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.apply_scalefactors_and_dequantize
//
// DESCRIPTION:
//    Apply the scalefactors and dequantise the samples
//
// INPUTS:
//    - r4 = ICS_ptr
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r8, r10, rMAC, I1-I7, M0-M3
//    - first element of $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.apply_scalefactors_and_dequantize;
   .CODESEGMENT AACDEC_APPLY_SCALEFACTORS_AND_DEQUANTIZE_PM;
   .DATASEGMENT DM;

   $aacdec.apply_scalefactors_and_dequantize:

   // push rLink onto stack
   push rLink;

   // set up r4 as an ics pointer
   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];

   // window = 0;
   // for g = 0:ics(ch).num_window_groups-1,
   //    k = 0;
   //    p = window*128;
   //    for sfb = 0:ics(ch).max_sfb-1,
   //       top = ics(ch).sect_sfb_offset(g+1,sfb+2);
   //       scale = 2^(0.25 * (ics(ch).scale_factors(g+1,sfb+1) - SF_OFFSET));
   //       while k < top,
   //          x_rescale(ch,p+k+1) = x_invquant(ch,p+k+1) * scale;
   //          k = k + 1;
   //       end
   //    end
   //
   //    window = window + ics(ch).window_group_length(g+1);
   // end

   // initialise regs for x^4/3 routine
   r0 = M[r9 + $aac.mem.X43_LOOKUP2_FIELD];
   I3 = r0 - ((DAWTH-15)*ADDR_PER_WORD);
   M3 = 9*ADDR_PER_WORD;
   M2 = 32*ADDR_PER_WORD;
   r7 = 0.25;

   // I5 = &window_group_length(g)
   r0 = r4 + $aacdec.ics.WINDOW_GROUP_LENGTH_FIELD;
   I5 = r0;

   // I1 = &scalefactors(0,0)
   r0 = M[r4 + $aacdec.ics.SCALEFACTORS_PTR_FIELD];
   I1 = r0;
   // r5 = g = 0
   r5 = 0;
   M[r9 + $aac.mem.TMP] = r5;
   // M1 = window = 0
   M1 = 0;

   r8 = 100;

   win_groups_loop:

      // M0 = k = 0
      M0 = 0;

      // I7 = &spec_sample(window * 128);
      r0 = M1;
      r1 = r0 * (128*ADDR_PER_WORD) (int);
      r0 = M[r9 + $aac.mem.CURRENT_SPEC_PTR];
      I7 = r0 + r1;

      // I4 = &sect_sfb_offset(g,1)
      r0 = M[r4 + $aacdec.ics.NUM_SWB_FIELD];
      r0 = r0 * r5 (int);
      r0 = r0 + r5;  // r0 = g*(num_swb+1)
      Words2Addr(r0);
      r1 = M[r4 + $aacdec.ics.SECT_SFB_OFFSET_PTR_FIELD];
      I4 = r0 + r1;
      I4 = I4 + ADDR_PER_WORD;

      r0 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
      // if max_sfb = 0 then skip this loop
      if Z jump sfb_loop_end;
      I6 = r0;
      sfb_loop:

         // read current scalefactor
         r0 = M[I1, MK1];
         r0 = r0 - ($aacdec.SF_OFFSET + $aacdec.REQUANTIZE_EXTRA_SHIFT);
         r1 = r0 AND 3;
         // r5 = scalefactor shift amount
         r5 = r0 ASHIFT -2;
         // r6 = scalefactor multiply factor
         r0 = M[r9 + $aac.mem.TWO2QTRX_LOOKUP_FIELD];
         Words2Addr(r1);
         r6 = M[r0 + r1];

         // r0 = top
         r0 = M[I4, MK1];
         r10 = r0 - M0;
         // k = k + r10;
         M0 = M0 + r10;
         do inner_loop;

            r0 = M[I7,0];
            Null = r0;
            if NEG jump negative_sample;
               call dequantize;
               jump long_write_back;
            negative_sample:
               r0 = -r0;
               call dequantize;
               rMAC = -rMAC;
            long_write_back:
             r8 = BLKSIGNDET rMAC,
              M[I7, MK1] = rMAC;

         inner_loop:

         I6 = I6 - 1;
      if NZ jump sfb_loop;
      sfb_loop_end:

      // M1 = window = window + ics(ch).window_group_length(g+1);
      r0 = M[I5, MK1];
      M1 = M1 + r0;

      // move on to the next window group
      r5 = M[r9 + $aac.mem.TMP];
      r5 = r5 + 1;
      M[r9 + $aac.mem.TMP] = r5;
      r0 = M[r4 + $aacdec.ics.NUM_WINDOW_GROUPS_FIELD];
      Null = r5 - r0;
   if NZ jump win_groups_loop;

   r1 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR];
   M[r1] = r8;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


   // **************************************************************************
   // SUBROUTINE:  Dequantise a sample and apply scalefactor
   //
   // INPUTS:
   //    r0 = sample to requantise
   //    r5 = scalefactor shift amount
   //    r6 = scalefactor multiply fraction
   //    r7 = 0.25
   //    I3 = x43_lookup2[-(DAWTH-15)]
   //    M3 = 9
   //    M2 = 32
   //
   // OUTPUTS:
   //    rMAC = Requantised subband sample
   //           = (r0 ^ (4/3) * scalefactor_faction) << scalefactor_shift
   //
   // TRASHED REGISTERS:
   //    - r0-r3, I2
   //
   // **************************************************************************
   dequantize:

   Null = r0 - 32;
   if NEG jump x43_first32;

      r1 = SIGNDET r0;
      r2 = M[r9 + $aac.mem.X43_LOOKUP1_FIELD];
      r3 = r2 - ((DAWTH-15)*ADDR_PER_WORD);
      r2 = r0 LSHIFT r1;       // r2 = x'
      Words2Addr(r1);
      I2 = r1 + r3;
      Null = r2 AND r7;
      if NZ I2 = I3 + r1;      // I2 = pointer in coef table to use

      r0 = M[I2,M3];           // get exponent coef

      r1 = r2 * r2 (frac),     // r1 = x'^2
       rMAC = M[I2,M3];        // get x'^0 coef

      r3 = r5 + r0,            // r3 = (scalefactor shift amount) + Exponent
       r0 = M[I2,M3];          // get x'^2 coef

      rMAC = rMAC + r1 * r0,
       r0 = M[I2,M3];          // get x'^1 coef
      rMAC = rMAC + r2 * r0;

      rMAC = rMAC * r6;        // now do the * 2^((scalefac+exp)/4)
      rMAC = rMAC ASHIFT r3;
      rts;

   x43_first32:
   Words2Addr(r0);
   r3 = M[r9 + $aac.mem.X43_LOOKUP32_FIELD];
   I2 = r0 + r3;
   r0 = M[I2,M2];           // get exponent coef

   r3 = r5 + r0,            // r3 = (scalefactor shift amount) + Exponent
    rMAC = M[I2,M2];        // get x'^0 coef

   rMAC = rMAC * r6;        // now do the * 2^((scalefac+exp)/4)
   rMAC = rMAC ASHIFT r3;
   rts;

.ENDMODULE;

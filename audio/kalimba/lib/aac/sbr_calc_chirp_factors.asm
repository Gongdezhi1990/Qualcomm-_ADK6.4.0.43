// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_SBR_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.sbr_calc_chirp_factors
//
// DESCRIPTION:
//    Calculate chirp factors required by hf_generation from bitstream information
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - r4 = &$aacdec.sbr_info
//
// TRASHED REGISTERS:
//    - r0-2, r7, r10, rMAC
//    - I0, I1, I4
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_calc_chirp_factors;
   .CODESEGMENT AACDEC_SBR_CALC_CHIRP_FACTORS_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_calc_chirp_factors:

   // push rLink onto stack
   push rLink;

   // store constant referring to channel being worked on in r7
   r7 = M[r9 + $aac.mem.CURRENT_CHANNEL];

   // set up index registers and loop register
   r0 = r7 * (5*ADDR_PER_WORD) (int);
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   r10 = M[r1 + $aacdec.SBR_Nq];
   r1 = r1 + r0;
   I4 = r1 + $aacdec.SBR_bs_invf_mode_prev;
   I0 = r1 + $aacdec.SBR_bwArray;
   r0 = r0 + r9;
   I1 = r0 + $aac.mem.SBR_bs_invf_mode;

   do chirp_loop;

      // newBw = sbr_mapNewBw(ch, i);
      call $aacdec.sbr_map_new_bw;

      // if( newBw < sbr.bwArray_prev(ch,i+1) )
      //    sbr.bwArray(ch,i+1) = (0.75 * newBw) + (0.25 * sbr.bwArray_prev(ch,i+1));
      // else
      //    sbr.bwArray(ch,i+1) = (0.90625 * newBw) + (0.09375 * sbr.bwArray_prev(ch,i+1));
      // end;
      r1 = M[I0, 0];
      Null = r0 - r1;
      if GE jump big_newBw;
         rMAC = r0 * 0.75;
         rMAC = rMAC + r1 * 0.25;
         jump if_2;

      big_newBw:
         rMAC = r0 * 0.90625;
         rMAC = rMAC + r1 * 0.09375;


      // if(sbr.bwArray(ch,i+1) < 0.015625)
      //    sbr.bwArray(ch,i+1) = 0.0;
      // end;
      if_2:

      Null = rMAC - 0.015625;
      if GE jump if_3;
         rMAC = 0;
         jump writeback;


      // if(sbr.bwArray(ch,i+1) >= 0.99609375)
      //    sbr.bwArray(ch,i+1) = 0.99609375;
      // end;
      if_3:

      r0 = rMAC - 0.99609375;
      if GE rMAC = rMAC - r0;


      writeback:

      // write sbr.bwArray value into sbr.bwArray
      M[I0, MK1] = rMAC;

      // The following not performed since sbr.bwArray_prev only used above so just use sbr.bwArray
      // sbr.bwArray_prev(ch,i+1) = sbr.bwArray(ch,i+1);

      // The following now performed in sbr_map_new_bw
      // sbr.bs_invf_mode_prev(ch,i+1) = sbr.bs_invf_mode(ch,i+1);

   chirp_loop:


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $aacdec.sbr_map_new_bw
//
// DESCRIPTION:
//    -
//
// INPUTS:
//    - I1 = &sbr.bs_invf_mode(ch, i+1)
//    - I4 = &sbr.bs_invf_mode_prev(ch, i+1)
//
// OUTPUTS:
//    - r0 = newBw
//    - I1 = &sbr.bs_invf_mode(ch, i+2)
//    - I4 = &sbr.bs_invf_mode_prev(ch, i+2)
//
// TRASHED REGISTERS:
//    -  r1, r2
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_map_new_bw;
   .CODESEGMENT AACDEC_SBR_MAP_NEW_BW_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_map_new_bw:

   r2 = M[I1, MK1],    //r2 = sbr.bs_invf_mode
    r1 = M[I4, 0];     //r1 = sbr.bs_invf_mode_prev


   // switch sbr.bs_invf_mode(ch, i+1)
   //
   //    case 1 % LOW
   //       if(sbr.bs_invf_mode_prev(ch, i+1) == 0) % NONE
   //          newBw = 0.6;
   //       else
   //          newBw = 0.75;
   //       end;
   //
   //    case 2  % MID
   //       newBw = 0.9;
   //
   //    case 3 % HIGH
   //       newBw = 0.98;
   //
   //    otherwise % NONE
   //       if(sbr.bs_invf_mode_prev(ch, i+1) == 1) % LOW
   //          newBw = 0.6;
   //       else
   //          newBw = 0.0;
   //       end;
   //
   // end;
   Null = r2 - 1;
   if NZ jump not_1;
      Null = r1;
      if NZ jump prev_not_0;
         r0 = 0.6;
         jump out;

      prev_not_0:
         r0 = 0.75;
         jump out;


   not_1:
   Null = r2 - 2;
   if NZ jump not_2;
      r0 = 0.9;
      jump out;


   not_2:
   Null = r2 - 3;
   if NZ jump not_3;
      r0 = 0.98;
      jump out;


   not_3:
   Null = r1 - 1;
   if NZ jump prev_not_1;
      r0 = 0.6;
      jump out;

   prev_not_1:
      r0 = 0;


   out:
   // sbr.bs_invf_mode_prev = sbr.bs_invf_mode
   M[I4, MK1] = r2;

   rts;

.ENDMODULE;

#endif

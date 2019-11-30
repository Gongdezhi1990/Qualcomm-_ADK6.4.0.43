// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBC_CALC_SCALE_FACTORS_INCLUDED
#define SBC_CALC_SCALE_FACTORS_INCLUDED

#include "sbc.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $sbc.calc_scale_factors
//
// DESCRIPTION:
//    Calculate Scalefactors
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r2, r4-r6, r10, DoLoop, I0, I1, M0, M1, M2
//
// NOTES:
//    Calculate scalefactors:
//
//     @verbatim
//     maxvals = ones(2,nrof_subbands);
//     scale_factor = zeros(2,nrof_subbands);
//     for ch = 1:nrof_channels,
//        for sb = 1:nrof_subbands,
//           for blk = 1:nrof_blocks,
//              if (abs(sb_sample(blk,ch,sb)) > maxvals(ch,sb))
//                 maxvals(ch,sb) = abs(sb_sample(blk,ch,sb));
//              end
//           end
//           scale_factor(ch,sb) = ceil(log10(maxvals(ch,sb))/log10(2)) - 1;
//        end
//     end
//     @endverbatim
//
// *****************************************************************************
.MODULE $M.sbc.calc_scale_factors;
   .CODESEGMENT SBC_CALC_SCALE_FACTORS_PM;
   .DATASEGMENT DM;

   $sbc.calc_scale_factors:

   r4 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
   I1 = r4;
   r4 = M[r9 + $sbc.mem.AUDIO_SAMPLE_FIELD];
   I0 = r4;
   r4 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];
   r5 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
   r6 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];


   r4 = r4 * r5 (int);
   Words2Addr (r4);
   M0 = r4;                 // M0 = nrof_channels * nrof_subbands
   r0 = r4 * r6 (int);
   M1 = ADDR_PER_WORD - r0;
   // r2 represents the maximum resolution - this is the same for 24-bit and 
   // 32-bit platforms.
   r2 = 16;
   r6 = r6 -1;
   r10 = r6;
   M2 = ADDR_PER_WORD;
   chan_subband_loop:

      // r1 = 16     read audio_sample
      r1 = r2,       r0 = M[I0,M0];
      do block_loop;
         // find minimum number of sign bits
         // read next audio_sample
         r1 = BLKSIGNDET r0,     r0 = M[I0,M0];
      block_loop:
      r1 = BLKSIGNDET r0;      // find minimum number of sign bits
      // calc scalefactor
      // dummy read to put I0 onto next subband/channel
      r1 = r2 - r1,     r0 = M[I0,M1];
      // store scale_factor
      r10 = r6,         M[I1,M2] = r1;

      r4 = r4 - ADDR_PER_WORD;
   if NZ jump chan_subband_loop;

   rts;

.ENDMODULE;

#endif

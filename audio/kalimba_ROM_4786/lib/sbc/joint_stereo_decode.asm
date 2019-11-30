// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBCDEC_JOINT_STEREO_DECODE_INCLUDED
#define SBCDEC_JOINT_STEREO_DECODE_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcdec.joint_stereo_decode
//
// DESCRIPTION:
//    Joint stereo decode processing
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r4, r10, DoLoop, I1, I4, M0-M3
//
// *****************************************************************************
//
//  Reconsturction of subband samples:
//
//    @verbatim
//    for blk = 1:nrof_blocks,
//       for sb = 1:nrof_subbands,
//          if (strcmp(channel_mode,'JOINT_STEREO') & (join(sb)==1))
//             sb_sample(blk,1,sb) =
//                              sb_sample(blk,1,sb) + sb_sample(blk,2,sb);
//             sb_sample(blk,2,sb) =
//                              sb_sample(blk,1,sb) - 2*sb_sample(blk,2,sb);
//          end
//       end
//    end
//    @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcdec.joint_stereo_decode;
   .CODESEGMENT SBCDEC_JOINT_STEREO_DECODE_PM;
   .DATASEGMENT DM;

   $sbcdec.joint_stereo_decode:

   // I1 = audio_sample(blk,ch,sb)
   r3 = M[r9 + $sbc.mem.AUDIO_SAMPLE_FIELD];
   I1 = r3;
   r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
   r3 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];

   Words2Addr(r10);
   M0 = r10;
   M1 = -r10;
   Addr2Words(r10);
   M2 = M1 + ADDR_PER_WORD;
   M3 = ADDR_PER_WORD;
   // load r4 to aid saturation below
   r4 = MININT;

   // for (blk=0, blk<nrof_blocks; blk++)
   blk_loop:

      // I4 = join(sb);

      I4 = r9 + $sbc.mem.JOIN_FIELD;


      // for (sb=0, sb<nrof_subbands; sb++)
      do sb_loop;
         r2 = M[I4, M3],   r0 = M[I1, M0];
         Null = r2,        r1 = M[I1, M1];

         if Z jump not_joint;
            // left  = M+S    and saturate if needed
            r2 = r0 + r1;
            if V r2 = r2 * r4 (int) (sat);
            // right = M-S    and saturate if needed
            r1 = r0 - r1;
            if V r1 = r1 * r4 (int) (sat);
            r0 = r2;
         not_joint:

         M[I1, M0] = r0;
         M[I1, M2] = r1;
      sb_loop:

      r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

      // move ptr onto next block
      I1 = I1 + M0;
      r3 = r3 - 1;
   if NZ jump blk_loop;

   rts;

.ENDMODULE;

#endif
#endif

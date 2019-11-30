// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBCENC_JOINT_STEREO_ENCODE_INCLUDED
#define SBCENC_JOINT_STEREO_ENCODE_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.joint_stereo_encode
//
// DESCRIPTION:
//    Joint stereo encode if needed
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    rMAC, r0-r6, r10, DoLoop, I0-I5, M0-M2
//
// *****************************************************************************
//
//  Joint stereo encode if needed:
//
//   @verbatim
//   % see if joint stereo should be used
//
//   sb_sample_joint(:,1,:) = (sb_sample(:,1,:) + sb_sample(:,2,:))/2;
//   sb_sample_joint(:,2,:) = (sb_sample(:,1,:) - sb_sample(:,2,:))/2;
//
//   maxvals = ones(2,nrof_subbands);
//   scale_factor_SD = zeros(2,nrof_subbands);
//   for ch = 1:nrof_channels,
//      for sb = 1:nrof_subbands,
//         for blk = 1:nrof_blocks,
//            if (abs(sb_sample_joint(blk,ch,sb)) > maxvals(ch,sb))
//               maxvals(ch,sb) = abs(sb_sample_joint(blk,ch,sb));
//            end
//         end
//         scale_factor_SD(ch,sb) = ceil(log10(maxvals(ch,sb))/log10(2)) - 1;
//      end
//   end
//
//   join = sum(scale_factor_SD) < sum(scale_factor);
//   join(nrof_subbands) = 0;
//   for sb = 1:nrof_subbands,
//      if join(sb)
//         scale_factor(:,sb) = scale_factor_SD(:,sb);
//         sb_sample(:,:,sb) = sb_sample_joint(:,:,sb);
//      end
//   end
//
//   scale_factor = max(scale_factor,0);
//   @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcenc.joint_stereo_encode;
   .CODESEGMENT SBCENC_JOINT_STEREO_ENCODE_PM;
   .DATASEGMENT DM;

   $sbcenc.joint_stereo_encode:

   // -- form middle and difference data --
   r5 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
   r6 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];

   // left
   r3 = M[r9 + $sbc.mem.AUDIO_SAMPLE_FIELD];
   I0 = r3;
   // middle (l+r)/2
   r3 = M[r9 + $sbc.mem.AUDIO_SAMPLE_JS_FIELD];
   I2 = r3;

   push r7;
   r7 = r5 * ADDR_PER_WORD (int);
   // right
   I1 = I0 + r7;
   // side (l-r)/2
   I3 = I2 + r7;
   r3 = 0.5;
   r4 = 1.0;
   gen_ms_blkloop:
      r10 = r5;
      do gen_ms_subbandloop;
         r0 = M[I0, MK1];
         rMAC = r0 * r3,            r1 = M[I1, MK1];
         // rMAC = (l+r)/2
         rMAC = rMAC + r1 * r3;
         // rMAC = (l-r)/2
         rMAC = rMAC - r1 * r4,     M[I2, MK1] = rMAC;
         M[I3, MK1] = rMAC;
      gen_ms_subbandloop:
      // hop over the other channel's data
      I0 = I0 + r7;
      I1 = I1 + r7;
      I2 = I2 + r7;
      I3 = I3 + r7;
      r6 = r6 - 1;
   if NZ jump gen_ms_blkloop;


   // -- calculate scalefactors for the middle and side data
   r6 = M[r9 + $sbc.mem.SCALE_FACTOR_JS_FIELD];
   I1 = r6;
   r6 = M[r9 + $sbc.mem.AUDIO_SAMPLE_JS_FIELD];
   I0 = r6;

   // M0 = nrof_channels * nrof_subbands
   M0 = r7 + r7;
   r6 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];


   r0 = r7 * r6 (int);
   r0 = r0 + r0;
   M1 = -r0;
   // M1 = 1 - nrof_channels * nrof_subbands * nrof_blocks
   M1 = M1 + ADDR_PER_WORD;
   r2 = 16;

   // loop through both channels and subbands
   r3 = r5 * 2 (int);
   sf_subbandchloop:

      r1 = 16;
      r10 = r6 - 1;
      // read audio_sample
      r0 = M[I0, M0];
      do sf_blkloop;
         // find minimum number of sign bits
         // and read the next audio_sample
         r1 = BLKSIGNDET r0,     r0 = M[I0, M0];
      sf_blkloop:
      // find minimum number of sign bits
      r1 = BLKSIGNDET r0;
      // calc scalefactor
      // and do dummy read to put I0 onto next subband/channel
      r1 = r2 - r1,        r0 = M[I0, M1];
      // store scale_factor
      M[I1, MK1] = r1;
      r3 = r3 - 1;
   if NZ jump sf_subbandchloop;

   // -- decide which subbands should use joint stereo --
   // middle
   r3 = M[r9 + $sbc.mem.SCALE_FACTOR_JS_FIELD];
   I0 = r3;
   // left
   r3 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
   I2 = r3;




   // side
   I4 = I0 + r7;
   // right
   I3 = I2 + r7;

   I1 = r9 + $sbc.mem.JOIN_FIELD;

   // Set all elements but last of join array accordingly
   r10 = r5 - 1;
   r3 = 1;
   do join_loop;
      r2 = r2 - r2,     r0 = M[I0, MK1],    r1 = M[I4, MK1];
      r0 = r0 + r1,     r1 = M[I2, MK1];
      r0 = r0 - r1,     r1 = M[I3, MK1];
      r0 = r0 - r1;
      if NEG r2 = r3;
      // join = 1 if (scale_factor_js < scale_factor) else = 0
      M[I1, MK1] = r2;
   join_loop:

   // Ensure the last element of join array is 0 (Ref A2DP Sec. 12.5.1)
   r2 = 0;
   // join(nrof_subbands) = 0;
   M[I1, MK1] = r2;

   // -- for joint subbands copy appropriate scale_factors and audio_samples --
   I4 = r9 + $sbc.mem.JOIN_FIELD;
   r0 = M[r9 + $sbc.mem.AUDIO_SAMPLE_JS_FIELD];
   I0 = r0;
   r0 = M[r9 + $sbc.mem.AUDIO_SAMPLE_FIELD];
   I5 = r0;
   r0 = M[r9 + $sbc.mem.SCALE_FACTOR_JS_FIELD];
   I2 = r0;
   r0 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
   I3 = r0;


   // M0 = nrof_subbands
   M0 = r7;
   r0 = r5 * r6 (int);
   r0 = r0 * (-2*ADDR_PER_WORD) (int);
   // M1 = 1 - nrof_subbands * nrof_blocks * 2
   M1 = ADDR_PER_WORD + r0;
   M2 = ADDR_PER_WORD;
   // M2 = 1 - nrof_subbands
   M2 = M2 - M0;

   copy_subbandloop:

      r0 = M[I4, MK1];
      Null = r0;
      if NZ jump joint_subband;
         // if not joint then onto next subband
         I0 = I0 + ADDR_PER_WORD;
         I5 = I5 + ADDR_PER_WORD;
         I2 = I2 + ADDR_PER_WORD;
         I3 = I3 + ADDR_PER_WORD;
         jump continue;

      // if joint then do the copy
      joint_subband:
         r10 = r6 - 1;
         // copy over audio_samples
         r1 = M[I0, M0];
         do copy_blkloop;
            r0 = M[I0, M0],   M[I5, M0] = r1;
            r1 = M[I0, M0],   M[I5, M0] = r0;
         copy_blkloop:
         r0 = M[I0, M0],   M[I5, M0] = r1;
         M[I5, M0] = r0,   r0 = M[I2, M0];

         // copy over scale_factor
         // r0 = M[I2, M0];
         M[I3, M0] = r0;
         // copy over scale_factor
         r0 = M[I2, M2];
         M[I3, M2] = r0;
         // move onto next subband
         I0 = I0 + M1;
         I5 = I5 + M1;

      continue:
         r5 = r5 - 1;
   if NZ jump copy_subbandloop;

      pop r7;
      rts;


.ENDMODULE;

#endif
#endif

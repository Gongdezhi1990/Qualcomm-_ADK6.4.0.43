// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBC_CALC_FRAME_LENGTH_INCLUDED
#define SBC_CALC_FRAME_LENGTH_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbc.calc_frame_length
//
// DESCRIPTION:
//    Calculate the length of the sbc frame in bytes
//
// INPUTS:
//    - r9 = data object pointer
//
// OUTPUTS:
//    - r0 = frame length in bytes
//
// TRASHED REGISTERS:
//    r1-r4
//
// *****************************************************************************
.MODULE $M.sbc.calc_frame_length;
   .CODESEGMENT SBC_CALC_FRAME_LENGTH_PM;
   .DATASEGMENT DM;

   $sbc.calc_frame_length:

   // MONO and DUAL_CHANNEL:
   //  frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +
   //                 ceil(nrof_blocks * nrof_channels * bitpool / 8)
   // STEREO and JOINT_STEREO:
   //  frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +
   //                 ceil(join * nrof_subbands + nrof_blocks * bitpool / 8)
   //
   //ceil(x / 8) = (x - 1) /8  + 1


   r0 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
   r1 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];


   r4 = r0 * r1 (int);
   r4 = r4 LSHIFT - 1;
   r4 = r4 + (4 + 1); // 4 + (4 * nrof_subbands * nrof_channels) / 8  + 1
   r2 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];
   r3 = M[r9 + $sbc.mem.BITPOOL_FIELD];
   r3 = r2 * r3 (int);


   r2 = M[r9 + $sbc.mem.CHANNEL_MODE_FIELD];

   r2 = r2 - $sbc.STEREO;
   if NEG jump mono_or_dual_chan;
   stereo_or_jointstereo:
      r2 = r2 * r0 (int);
      r3 = r3 + r2;
      jump finish_frame_length_calc;

   mono_or_dual_chan:
      r3 = r3 * r1 (int);

   finish_frame_length_calc:
   // do divide by 8 and round up
   r3 = r3 - 1;
   r3 = r3 LSHIFT -3;
//   r3 = r3 + 1;

   r0 = r4 + r3;
   rts;

.ENDMODULE;

#endif
#endif

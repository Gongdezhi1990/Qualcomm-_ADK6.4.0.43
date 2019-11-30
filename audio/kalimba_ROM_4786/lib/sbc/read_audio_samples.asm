// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCDEC_READ_AUDIO_SAMPLES_INCLUDED
#define SBCDEC_READ_AUDIO_SAMPLES_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcdec.read_audio_samples
//
// DESCRIPTION:
//    Read audio samples
//
// INPUTS:
//    - I0 = buffer to read words from
//
// OUTPUTS:
//    - I0 = buffer to read words from (updated)
//
// TRASHED REGISTERS:
//    r0-r5, r10, DoLoop, I1, I2
//
// NOTES:
//  Read in audio samples
//
//     @verbatim
//     audio_samples()
//     (
//        for (blk=0;blk<nrof_blocks;blk++)
//        (
//           for (ch=0;ch<nrof_channels;ch++)
//           (
//              for (sb=0;sb<nrof_subbands;sb++)
//              (
//                 if (bits[ch][sb]!=0)
//                 (
//                    $audio_sample[blk][ch][sb]               2..16 UiMsbf
//                 )
//              )
//           )
//        )
//     )
//     @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcdec.read_audio_samples;
   .CODESEGMENT SBCDEC_READ_AUDIO_SAMPLES_PM;
   .DATASEGMENT DM;

   $sbcdec.read_audio_samples:

   // push rLink onto stack
   push rLink;

   r5 = M[r9 + $sbc.mem.AUDIO_SAMPLE_FIELD];
   I1 = r5;


   r4 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];



   // for (blk=0, blk<nrof_blocks; blk++)
   blk_loop:

      r5 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];
      r10 = M[r9 + $sbc.mem.BITS_FIELD];
      I2 = r10;






      // for (ch=0, chk<nrof_channels; ch++)
      ch_loop:

         r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

         // for (sb=0, sb<nrof_subbands; sb++)
         do sb_loop;
            // if bits[ch][sb]!=0
            r1 = 0,        r0 = M[I2, MK1];
            Null = r0;
            // set audio_sample[blk][ch][sb]
            if NZ call $sbcdec.getbits;
            M[I1, MK1] = r1;
         sb_loop:
#ifndef SBC_WBS_ONLY
         r5 = r5 - 1;
      if NZ jump ch_loop;
#endif
      r4 = r4 - 1;
   if NZ jump blk_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

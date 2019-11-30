// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCENC_WRITE_AUDIO_SAMPLES_INCLUDED
#define SBCENC_WRITE_AUDIO_SAMPLES_INCLUDED

#include "stack.h"

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.write_audio_samples
//
// DESCRIPTION:
//    Write audio samples
//
// INPUTS:
//    - I0 = buffer pointer to write words to
//
// OUTPUTS:
//    - I0 = buffer pointer to write words to (updated)
//
// TRASHED REGISTERS:
//    r0-r6, r10, DoLoop, I2, I4
//
// *****************************************************************************
//
//  Write audio samples
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

.MODULE $M.sbcenc.write_audio_samples;
   .CODESEGMENT SBCENC_WRITE_AUDIO_SAMPLES_PM;
   .DATASEGMENT DM;

   $sbcenc.write_audio_samples:

   // push rLink onto stack
   push rLink;

   r4 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];
   r5 = M[r9 + $sbc.mem.AUDIO_SAMPLE_FIELD];
   I4 = r5;
   r5 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];
   r6 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

   // for (blk=0, blk<nrof_blocks; blk++)
   block_loop:
      r10 = r6 * r5 (int);
      r0 = M[r9 + $sbc.mem.BITS_FIELD];
      I2 = r0;

      // for (ch=0, chk<nrof_channels; ch++)
      do chan_subband_loop;
         // for (sb=0, sb<nrof_subbands; sb++)
         r0 = M[I2,MK1],     r1 = M[I4,MK1];
         // if bits[ch][sb]!=0
         Null = r0;
         // write audio_sample[blk][ch][sb]
         if NZ call $sbcenc.putbits;

      chan_subband_loop:
      r4 = r4 - 1;
   if NZ jump block_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCENC_QUANTIZE_SAMPLES_INCLUDED
#define SBCENC_QUANTIZE_SAMPLES_INCLUDED

#include "sbc.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.quantize_samples
//
// DESCRIPTION:
//    Quantizing of subband samples
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r2, r4-r8, r10, DoLoop, I1-I3, M0, M1, M2, M3
//
// *****************************************************************************
//
//  Quantizing of subband samples:
//
//    @verbatim
//    for ch = 1:nrof_channels,
//       for sb = 1:nrof_subbands,
//          levels(ch,sb) = 2^ bits(ch,sb) - 1;
//       end
//    end
//
//    for blk = 1:nrof_blocks,
//       for ch = 1:nrof_channels,
//          for sb = 1:nrof_subbands,
//             if levels(ch,sb) > 0
//                sb_sample_quant(blk,ch,sb) =
//                     floor((sb_sample(blk,ch,sb)/2^(scale_factor(ch,sb)+1) + 1) *
//                     levels(ch,sb) / 2);
//             else
//                sb_sample_quant(blk,ch,sb) = 0;
//             end
//          end
//       end
//    end
//    @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcenc.quantize_samples;
   .CODESEGMENT SBCENC_QUANTIZE_SAMPLES_PM;
   .DATASEGMENT DM;

   $sbcenc.quantize_samples:

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbcenc.QUANTIZE_SAMPLES_ASM.QUANTIZE_SAMPLES.PATCH_ID_0, r2)
#endif

   r2 = M[r9 + $sbc.mem.LEVEL_COEFS_FIELD];
   r2 = r2 - (1*ADDR_PER_WORD);
   r0 = M[r9 + $sbc.mem.AUDIO_SAMPLE_FIELD];
   I1 = r0; // I1 = sb_sample(blk,ch,sb)
   r0 = M[r9 + $sbc.mem.BITS_FIELD];
   I2 = r0; // I2 = bits(ch,sb)
   r0 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
   I3 = r0; // I3 = scale_factor(ch,sb)


   r8 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];
   r7 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];


   r0 = r8 * r7 (int);
   Words2Addr (r0);
   M0 = r0;                     // M0 = nrof_channels * nrof_subbands
   M2 = -r0;
   M3 = r0 + r0;
   r1 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];
   r1 = r0 * r1 (int);
   M1 = ADDR_PER_WORD - r1;
   chan_loop:


      r7 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

      subband_loop:

         r10 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];

         r0 = M[I2,MK1];                // r0 = bits(ch,sb)
         Null = Null + r0,    r4 = M[I3,MK1];  // r4 = scale_factor(ch,sb)
         if Z jump zero_sample;
            r5 = (1.0/(2 ** 16)) ASHIFT r4; // 2^-16 in fractional
            r4 = (32 - DAWTH) - r4;        // 32-bit calculation needed
            Words2Addr (r0);
            r6 = M[r2 + r0];
            r1 = M[I1,M0];
            r1 = r1 + r5;                // r1 = sb_sample(blk,ch,sb) + 1<<(sf+1)
            do block_loop;
               rMAC = r1 * r6;              // * levels
               // / 2^(sf+1)
               rMAC = rMAC ASHIFT r4,  r1 = M[I1,M2];
               // r1 = sb_sample(blk,ch,sb) + 1<<(sf+1)
               r1 = r1 + r5,           M[I1,M3] = rMAC;
            block_loop:
            I1 = I1 + M2;
            jump block_loop_done;

         zero_sample:
            do zero_loop;                // for (blk=0, blk<nrof_blocks; blk++)
               M[I1,M0] = r0;               // write 0 for this block
            zero_loop:                   // end blk loop

         block_loop_done:
         r0 = M[I1,M1];               // dummy read to reposition I1
         r7 = r7 - 1;
      if NZ jump subband_loop;
      r8 = r8 - 1;
   if NZ jump chan_loop;

   rts;

.ENDMODULE;

#endif

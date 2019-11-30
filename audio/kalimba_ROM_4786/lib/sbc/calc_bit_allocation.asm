// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBC_CALC_BIT_ALLOCATION_INCLUDED
#define SBC_CALC_BIT_ALLOCATION_INCLUDED

#include "stack.h"
#include "sbc.h"


// *****************************************************************************
// MODULE:
//    $sbc.calc_bit_allocation
//
// DESCRIPTION:
//    Calculate Bit Allocation
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r5, r8, r10, DoLoop, I1-I4
//
// NOTES:
//    For Equivalent C code:
//    See AADP bluetooth spec Appendix B - Technical spec of SBC
//
// *****************************************************************************

.MODULE $M.sbc.calc_bit_allocation;
   .CODESEGMENT SBC_CALC_BIT_ALLOCATION_PM;
   .DATASEGMENT DM;

   $sbc.calc_bit_allocation:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbc.CALC_BIT_ALLOCATION_ASM.CALC_BIT_ALLOCATION.PATCH_ID_0, r1)
#endif

   // if strcmp(channel_mode,'MONO') | strcmp(channel_mode,'DUAL_CHANNEL')
   // % ------- Mono and Dual Channel Bit Allocation ---------
   //
   M1 = ADDR_PER_WORD;

#ifdef SBC_WBS_ONLY
      r0 = 0;
      md_chan_loop:

      // % derive bitneed values from the scale_factors
      // if strcmp(allocation_method,'SNR')
#else



   r0 = M[r9 + $sbc.mem.CHANNEL_MODE_FIELD];

   r0 = r0 AND $sbc.STEREO;
   if NZ jump stereo_joint;

      // for ch = 1:nrof_channels
      r0 = 0;
      md_chan_loop:

         // % derive bitneed values from the scale_factors
         // if strcmp(allocation_method,'SNR')

         r1 = M[r9 + $sbc.mem.ALLOCATION_METHOD_FIELD];

         if Z jump md_loudness;

            // for sb = 1:nrof_subbands,
               // bitneed(ch,sb) = scale_factor(ch,sb);
            // end

            r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];


            r1 = r0 * r10 (int);
            Words2Addr (r1);
            r10 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
            I1 = r1 + r10;
            r10 = M[r9 + $sbc.mem.BITNEED_FIELD];
            I2 = r1 + r10;
            r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];


            do md_snr_loop;
               r1 = M[I1, MK1];
               M[I2, MK1] = r1;
            md_snr_loop:
            jump md_maxbitneedindex;


         // else
         md_loudness:
#endif
            // for sb = 1:nrof_subbands,
               // if (scale_factor(ch,sb)==0)
                  // bitneed(ch,sb) = -5;
               // else
                  // if (nrof_subbands == 4)
                     // loudness = scale_factor(ch,sb) - offset4(sampling_freq+1,sb);
                  // else
                     // loudness = scale_factor(ch,sb) - offset8(sampling_freq+1,sb);
                  // end
                  // if (loudness > 0)
                     // bitneed(ch,sb) = floor(loudness/2);
                  // else
                     // bitneed(ch,sb) = loudness;
                  // end
               // end
            // end



            r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];


            r1 = r0 * r10 (int);
            Words2Addr (r1);

            r4 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
            I1 = r1 + r4;
            r4 = M[r9 + $sbc.mem.BITNEED_FIELD];
            I2 = r1 + r4;
            r4 = -1;
            r1 = M[r9 + $sbc.mem.SAMPLING_FREQ_FIELD];


            // I3 = ptr to appropriate LoudnessOffset table
            r1 = r1 * r10 (int);
            Words2Addr (r1);

#ifdef SBC_WBS_ONLY

            // only written for static case. Adding dynamic: point to table for M=8, that was copied from flash
            // if not WBS_ONLY, then needs also table for M=4 which is 16 words before the one for M=8
            r2 = M[r9 + $sbc.mem.LOUDNESS_OFFSET_FIELD]; // for WBS_ONLY, this should have pointer for table for M=8
            I4 = r2;

            //I4 = &$sbc.loudness_offset_m8;
            I3 = r1 + I4;
#else

            r2 = M[r9 + $sbc.mem.LOUDNESS_OFFSET_FIELD];
            I3 = r1 + r2;
            // one buffer is used to store both tables, m4 is first and m8 next
            I4 = r2 + (16 * ADDR_PER_WORD);  // I4 will point in this non-WBS_ONLY case to table for M=8 (which is after table for M=4)

            Null = r10 - 8;
            if Z I3 = r1 + I4;
#endif

            do md_loudness_loop;

               r2 = -5;
               r1 = M[I1, MK1];
               Null = Null + r1,  r3 = M[I3, MK1];
               if Z jump md_loudness_endif;

                  r2 = r1 - r3;
                  if POS r2 = r2 ASHIFT r4;

               md_loudness_endif:
               M[I2, MK1] = r2;

            md_loudness_loop:
         // end



         // % find maximum bitneed index
         // max_bitneed=0;
         // for sb = 1:nrof_subbands,
            // if (bitneed(ch,sb) > max_bitneed)
               // max_bitneed = bitneed(ch,sb);
            // end
         // end

         md_maxbitneedindex:

         r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

         r1 = r0 * r10 (int);
         Words2Addr (r1);

         r2 = M[r9 + $sbc.mem.BITNEED_FIELD];
         I2 = r1 + r2;



         call share_code_1;



         // while 1
            // bitslice = bitslice - 1;
            // bitcount = bitcount + slicecount;
            // slicecount = 0;
            // for sb = 1: nrof_subbands,
               // if ((bitneed(ch,sb) > bitslice+1) & (bitneed(ch,sb) < bitslice+16))
                  // slicecount = slicecount + 1;
               // elseif (bitneed(ch,sb) == bitslice+1)
                  // slicecount = slicecount + 2;
               // end
            // end
            // if (bitcount+slicecount>=bitpool)
               // break;
            // end;
         // end
         md_findbitslice_loop1:



            r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

            r1 = r0 * r10 (int);
            Words2Addr (r1);

            r5 = M[r9 + $sbc.mem.BITNEED_FIELD];
            I2 = r1 + r5;



               call share_code_2;

         if NEG jump md_findbitslice_loop1;



         // % distribute bits until last bitslice is reached
         // for sb = 1:nrof_subbands,
            // if (bitneed(ch,sb)<bitslice+2)
               // bits(ch,sb) = 0;
            // else
               // bits(ch,sb) = min(bitneed(ch,sb) - bitslice, 16);
            // end
         // end


         r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

         r1 = r0 * r10 (int);
         Words2Addr (r1);

         r10 = M[r9 + $sbc.mem.BITS_FIELD];
         I1 = r1 + r10;
         r10 = M[r9 + $sbc.mem.BITNEED_FIELD];
         I2 = r1 + r10;
         r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];


         call share_code_3;



         // % remaining bits are allocated starting at subband 0
         // sb = 0;
         // while ((bitcount < bitpool) && (sb < nrof_subbands))
            // if ((bits(ch,sb)>=2) & (bits(ch,sb)<16))
               // bits(ch,sb) = bits(ch,sb) + 1;
               // bitcount = bitcount + 1;
            // elseif ((bitneed(ch,sb)==bitslice+1) & (bitpool>bitcount+1))
               // bits(ch,sb)=2;
               // bitcount = bitcount + 2;
            // end
            // sb = sb + 1;
         // end

         // sb = 0;
         // while ((bitcount < bitpool) && (sb < nrof_subbands))
            // if (bits(ch,sb)<16)
               // bits(ch,sb) = bits(ch,sb) + 1;
               // bitcount = bitcount + 1;
            // end
            // sb = sb + 1;
         // end

         // r1 = bits
         // r2 = bitslice
         // r3 = bitcount
         // r4 = sb


         r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

         r1 = r0 * r10 (int);
         Words2Addr (r1);

         r4 = M[r9 + $sbc.mem.BITS_FIELD];
         I1 = r1 + r4;
         r4 = M[r9 + $sbc.mem.BITNEED_FIELD];
         I2 = r1 + r4;
         r4 = 0;

         md_remainingbits_loop:

            r1 = M[r9 + $sbc.mem.BITPOOL_FIELD];
            Null = r3 - r1;


               if POS jump md_remainingbits_end;

            call share_code_4;

            r4 = r4 + 1;
            Null = r4 - r10;
            if NZ jump md_remainingbits_loop;
         md_remainingbits_end:


         r1 = r0 * r10 (int);
         Words2Addr (r1);

         r4 = M[r9 + $sbc.mem.BITS_FIELD];
         I1 = r1 + r4;
         r4 = M[r9 + $sbc.mem.BITNEED_FIELD];
         I2 = r1 + r4;
         r4 = 0;

         md_remainingbits_extra_loop:

            r1 = M[r9 + $sbc.mem.BITPOOL_FIELD];
            Null = r3 - r1;

         if POS jump md_remainingbits_extra_end;

            r1 = M[I1, 0];
            r5 = M[I2, MK1];
            Null = r1 - 16;
            if POS jump md_remainingbits_extra_endif;
               r1 = r1 + 1;
               r3 = r3 + 1;
            md_remainingbits_extra_endif:
            M[I1, MK1] = r1;

            r4 = r4 + 1;
            Null = r4 - r10;
            if NZ jump md_remainingbits_extra_loop;
         md_remainingbits_extra_end:


         r0 = r0 + 1;

         r1 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];
         Null = r0 - r1;

      if NZ jump md_chan_loop;

      // pop rLink from stack
      jump $pop_rLink_and_rts;

#ifndef SBC_WBS_ONLY

   // else
   //    % ------- Stereo & Joint Stereo Bit Allocation ---------
   stereo_joint:

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbc.CALC_BIT_ALLOCATION_ASM.CALC_BIT_ALLOCATION.PATCH_ID_1, r1)
#endif

      // % derive bitneed values from the scale_factors of both channels
      // if strcmp(allocation_method,'SNR')

      r1 = M[r9 + $sbc.mem.ALLOCATION_METHOD_FIELD];


      if Z jump sj_loudness;


         // for ch = 1:2,
            // for sb = 1:nrof_subbands,
               // bitneed(ch,sb) = scale_factor(ch,sb);
            // end
         // end

         r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];


         r10 = r10 * 2 (int);
         r1 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
         I1 = r1;
         r1 = M[r9 + $sbc.mem.BITNEED_FIELD];
         I2 = r1;


         do sj_snr_loop2;
            r1 = M[I1, MK1];
            M[I2, MK1] = r1;
         sj_snr_loop2:
         jump sj_maxbitneedindex;


      // else
      sj_loudness:
          // for ch = 1:2,
            // for sb = 1:nrof_subbands,
               // if (scale_factor(ch,sb)==0)
                  // bitneed(ch,sb) = -5;
               // else
                  // if (nrof_subbands == 4)
                     // loudness = scale_factor(ch,sb) - offset4(sampling_freq+1,sb);
                  // else
                     // loudness = scale_factor(ch,sb) - offset8(sampling_freq+1,sb);
                  // end
                  // if (loudness > 0)
                     // bitneed(ch,sb) = floor(loudness/2);
                  // else
                     // bitneed(ch,sb) = loudness;
                  // end
               // end
            // end
         // end
         r0 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
         r4 = -1;
         r1 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
         I1 = r1;
         r1 = M[r9 + $sbc.mem.BITNEED_FIELD];
         I2 = r1;
         r1 = M[r9 + $sbc.mem.SAMPLING_FREQ_FIELD];




         // I4 = ptr to appropriate LoudnessOffset table
         r1 = r1 * r0 (int);
         Words2Addr (r1);

         r2 = M[r9 + $sbc.mem.LOUDNESS_OFFSET_FIELD];
         I4 = r1 + r2;
         // one buffer is used to store both tables, m4 is first and m8 next
         I3 = r2 + (16*ADDR_PER_WORD);

         Null = r0 - 8;
         if Z I4 = r1 + I3;

         r0 = 2;
         r5 = -10;
         r2 = r5,    r1 = M[I1,M1];
         sj_loudness_chan_loop:

            r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

            I3 = I4 + ADDR_PER_WORD;
            r3 = M[I4,0];

            do sj_loudness_sb_loop;

               Null = r1;
               //subtract offset
               if NZ r2 = r1 - r3,        r1 = M[I1,M1];
               //divide by two
               if POS r2 = r2 ASHIFT r4,  r3 = M[I3,M1];
               r2 = r5,                   M[I2,M1] = r2;

            sj_loudness_sb_loop:

            r0 = r0 - 1;
         if NZ jump sj_loudness_chan_loop;

      // end



      // % find maximum bitneed index
      // max_bitneed=0;
      // for ch = 1:2
         // for sb = 1:nrof_subbands,
            // if (bitneed(ch,sb) > max_bitneed)
               // max_bitneed = bitneed(ch,sb);
            // end
         // end
      // end
      sj_maxbitneedindex:

      r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];


      r10 = r10 * 2 (int);

      r2 = M[r9 + $sbc.mem.BITNEED_FIELD];
      I2 = r2;



      call share_code_1;


      // while 1
         // bitslice = bitslice - 1;
         // bitcount = bitcount + slicecount;
         // slicecount = 0;
         // for ch = 1:2,
            // for sb = 1: nrof_subbands,
               // if ((bitneed(ch,sb) > bitslice+1) & (bitneed(ch,sb) < bitslice+16))
                  // slicecount = slicecount + 1;
               // elseif (bitneed(ch,sb) == bitslice+1)
                  // slicecount = slicecount + 2;
               // end
            // end
         // end
         // if (bitcount+slicecount>=bitpool)
            // break
         // end;
      // end
      sj_findbitslice_loop1:

         r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
         r10 = r10 * 2 (int);
         r5 = M[r9 + $sbc.mem.BITNEED_FIELD];
         I2 = r5;






            call share_code_2;

      if NEG jump sj_findbitslice_loop1;



      // % distribute bits until last bitslice is reached
      // for ch = 1:2,
         // for sb = 1:nrof_subbands,
            // if (bitneed(ch,sb)<bitslice+2)
               // bits(ch,sb) = 0;
            // else
               // bits(ch,sb) = min(bitneed(ch,sb) - bitslice, 16);
            // end
         // end
      // end

      r10 = M[r9 + $sbc.mem.BITS_FIELD];
      I1 = r10;
      r10 = M[r9 + $sbc.mem.BITNEED_FIELD];
      I2 = r10;
      r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

      r10 = r10 * 2 (int);

      call share_code_3;



      // % remaining bits are allocated starting at subband 0
      // sb = 0;
      // ch = 0;
      // while ((bitcount < bitpool) && (sb < nrof_subbands))
         // if ((bits(ch,sb)>=2) & (bits(ch,sb)<16))
            // bits(ch,sb) = bits(ch,sb) + 1;
            // bitcount = bitcount + 1;
         // elseif ((bitneed(ch,sb)==bitslice+1) & (bitpool>bitcount+1))
            // bits(ch,sb)=2;
            // bitcount = bitcount + 2;
         // end
         // if (ch==1)
            // ch = 0;
            // sb = sb + 1;
         // else
            // ch = 1;
         // end
      // end

      // sb = 0;
      // ch = 0;
      // while ((bitcount < bitpool) && (sb < nrof_subbands))
         // if (bits(ch,sb)<16))
            // bits(ch,sb) = bits(ch,sb) + 1;
            // bitcount = bitcount + 1;
         // end
         // if (ch==1)
            // ch = 0;
            // sb = sb + 1;
         // else
            // ch = 1;
         // end
      // end

      r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];



      // r4 = sb
      r4 = 0;
      // r0 = ch*nrof_subbands;
      r0 = 0;

      sj_remainingbits_loop:
         r5 = r0 + r4;
         Words2Addr(r5);

         I2 = r5;

         r1 = M[r9 + $sbc.mem.BITS_FIELD];
         I1 = r5 + r1;
         r1 = M[r9 + $sbc.mem.BITNEED_FIELD];
         I2 = r5 + r1;
         r1 = M[r9 + $sbc.mem.BITPOOL_FIELD];
         Null = r3 - r1;


            if POS jump sj_remainingbits_end;


         call share_code_4;


         Null = r0 - r10;
         if Z jump sj_remainingbits_chanchange;
            r0 = r10;
            jump sj_remainingbits_loop;
         sj_remainingbits_chanchange:
            r0 = 0;
            r4 = r4 + 1;
            Null = r4 - r10;
      if NZ jump sj_remainingbits_loop;

      sj_remainingbits_end:

      // r4 = sb
      r4 = 0;
      // r0 = ch*nrof_subbands;
      r0 = 0;

      sj_remainingbits_extra_loop:
         r5 = r0 + r4;
         Words2Addr(r5);

         I2 = r5;

         r1 = M[r9 + $sbc.mem.BITS_FIELD];
         I1 = r5 + r1;
         r1 = M[r9 + $sbc.mem.BITNEED_FIELD];
         I2 = r5 + r1;
         r1 = M[r9 + $sbc.mem.BITPOOL_FIELD];
         Null = r3 - r1;

            if POS jump sj_remainingbits_extra_end;

         r1 = M[I1, 0];
         r5 = M[I2, 0];
         Null = r1 - 16;
         if POS jump sj_remainingbits_extra_endif;
            r1 = r1 + 1;
            r3 = r3 + 1;
         sj_remainingbits_extra_endif:
         M[I1, MK1] = r1;

         Null = r0 - r10;
         if Z jump sj_remainingbits_extra_chanchange;
            r0 = r10;
            jump sj_remainingbits_extra_loop;
         sj_remainingbits_extra_chanchange:
            r0 = 0;
            r4 = r4 + 1;
            Null = r4 - r10;
      if NZ jump sj_remainingbits_extra_loop;

      sj_remainingbits_extra_end:
      // pop rLink from stack
      jump $pop_rLink_and_rts;
#endif


   share_code_1:


         r2 = 0;
         do maxbitneed_loop;
            r1 = M[I2, MK1];
            Null = r2 - r1;
            if NEG r2 = r1;
         maxbitneed_loop:
         // r2 = max_bitneed

         // % iteratively find how many bitslices fit into the bitpool
         // bitcount = 0;
         // slicecount = 0;
         // bitslice = max_bitneed + 1;   % init just above largest sf
         // r3 = bitcount
         r3 = 0;
         // r4 = slicecount
         r4 = 0;
         // r2 = bitslice
         r2 = r2 + 1;

         rts;


   share_code_2:

         r8 = M[r9 + $sbc.mem.BITSLICE_LOOKUP_FIELD];
         r2 = r2 - 1;
         r3 = r3 + r4;
         r4 = 0;
         r8 = r8 + 25*ADDR_PER_WORD;
         
         r1 = M[I2,MK1];
         do findbitslice_loop2;
            // r5 - bitneed - bitslice
            r5 = r1 - r2;
            Words2Addr(r5);
            r5 = M[r5 + r8];
            r4 = r4 + r5,
             r1 = M[I2,MK1];
         findbitslice_loop2:

         r5 = r3 + r4;

         r10 = M[r9 + $sbc.mem.BITPOOL_FIELD];
         Null = r5 - r10;

         rts;


   share_code_3:

            // if (bitcount+slicecount==bitpool)
               // bitcount = bitcount + slicecount;
               // bitslice = bitslice - 1;
            // end

            r1 = M[r9 + $sbc.mem.BITPOOL_FIELD];
            Null = r5 - r1;


            if NZ jump findbitslice_endif2;
               r3 = r3 + r4;
               r2 = r2 - 1;
            findbitslice_endif2:


         do distbits_loop;
            r1 = M[I2, MK1];
            r5 = r1 - r2;
            r1 = 0;
            Null = r5 - 2;
            if POS r1 = r5;
            r5 = 16;
            Null = r1 - 16;
            if POS r1 = r5;
            M[I1, MK1] = r1;
         distbits_loop:

         rts;


   share_code_4:

         r1 = M[I1, 0];
         r5 = M[I2, MK1];
         Null = r1 - 2;
         if NEG jump remainingbits_elseif;
            Null = r1 - 16;
            if POS jump remainingbits_elseif;
            r1 = r1 + 1;
            r3 = r3 + 1;
            jump remainingbits_endif;

         remainingbits_elseif:
            r5 = r5 - r2;
            Null = r5 - 1;
            if NZ jump remainingbits_endif;

            r5 = M[r9 + $sbc.mem.BITPOOL_FIELD];

            r5 = r5 - r3;
            Null = r5 - 2;
            if NEG jump remainingbits_endif;
            r1 = 2;
            r3 = r3 + 2;
         remainingbits_endif:
         M[I1, MK1] = r1;

         rts;



   corrupt_frame_error:
      r0 = 1;

      M[r9 + $sbc.mem.FRAME_CORRUPT_FIELD] = r0;


      // pop rLink from stack
      jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

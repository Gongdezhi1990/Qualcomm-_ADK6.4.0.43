// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.scalefactor_data
//
// DESCRIPTION:
//    Get scale factor data
//
// INPUTS:
//    - r4 = pointer to the current ICS structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r5-r8, r10, I1-I3, M2
//
// *****************************************************************************
.MODULE $M.aacdec.scalefactor_data;
   .CODESEGMENT AACDEC_SCALEFACTOR_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.scalefactor_data:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SCALEFACTOR_DATA_ASM.SCALEFACTOR_DATA.SCALEFACTOR_DATA.PATCH_ID_0, r5)
#endif
   
   

   // scale_factor = global_gain;
   // is_position = 0;
   // noise_pcm_flag = 1;
   // noise_energy = global_gain - 90 - 256;
   //
   // for g = 0:num_window_groups-1,
   //    for sfb = 0:max_sfb-1,
   //       switch (sfb_cb(g,sfb))
   //          case ZERO_HCB:
   //             scale_factors(g,sfb) = 0;
   //             break;
   //
   //          case INTENSITY_HCB:
   //          case INTENSITY_HCB2:
   //             is_position += (huffman_scale_factor() - 60);
   //             // store is_position in sf array
   //             scale_factors(g,sfb) = is_position;
   //             break;
   //
   //          case NOISE_HCB:
   //              if (noise_pcm_flag)
   //                 noise_pcm_flag = 0;
   //                 noise_energy += getbits(9);
   //              else
   //                 noise_energy += huffman_scale_factor() - 60;
   //                 store noise_energy in sf array
   //              scale_factors(g,sfb) = noise_energy;
   //              break;
   //
   //          default:
   //              scale_factor += (huffman_scale_factor() - 60);
   //                 force scale_factor to be between 0 and 255
   //              scale_factors(g,sfb) = scale_factor;
   //              break;
   //       end
   //    end
   // end

   // unpack the huffman table (0) for reading scalefactors
   r5 = 0;
   call $aacdec.huffman_unpack_individual_flash_table;

   // r6 = scale_factor
   r6 = M[r4 + $aacdec.ics.GLOBAL_GAIN_FIELD];
   // r8 = is_position
   r8 = 0;
   // M2 = noise_pcm_flag
   M2 = 1;
   // r7 = noise_energy
   r7 = r6 - (90 + 256);



   r10 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
   r0 = M[r4 + $aacdec.ics.NUM_WINDOW_GROUPS_FIELD];
   r10 = r10 * r0 (int);

   // allocate tmp memory for scalefactors
   r0 = r10;
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;
   M[r4 + $aacdec.ics.SCALEFACTORS_PTR_FIELD] = r1;
   I2 = r1;

   r1 = M[r4 + $aacdec.ics.SFB_CB_PTR_FIELD];
   I3 = r1;

   // set things up for huffman decoding to work
   call $aacdec.huffman_start;

   do num_win_and_sfb_loop;
      r0 = M[I3, MK1];
      Null = r0 - $aacdec.ZERO_HCB;
      if Z jump zero_hcb;
      Null = r0 - $aacdec.NOISE_HCB;
      if Z jump noise_hcb;
      .CONST SCALEFACTOR_DATA_TEMP1 ($aacdec.INTENSITY_HCB & ($aacdec.INTENSITY_HCB2));
      r0 = r0 AND SCALEFACTOR_DATA_TEMP1;
      Null = r0 - SCALEFACTOR_DATA_TEMP1;
      if Z jump intensity_hcb;

      default:
         call $aacdec.huffman_getsf;
         r0 = r0 - 60;
         r6 = r6 + r0;
         r0 = r6;
         if NEG jump possible_corruption_in_huff;
         Null = r0 - 256;
         if POS jump possible_corruption_in_huff;
         jump update_scalefactors;

      zero_hcb:
         r0 = 0;
         jump update_scalefactors;

      intensity_hcb:
         call $aacdec.huffman_getsf;
         r0 = r0 - 60;
         r8 = r8 + r0;
         r0 = r8;
         jump update_scalefactors;

      noise_hcb:
         Null = M2;
         if Z jump noise_pcm_disabled;
            M2 = 0;
            r5 = 9;
            call $aacdec.huffman_getbits;
            r7 = r7 + r1;
            r0 = r7;
            jump update_scalefactors;

         noise_pcm_disabled:
            call $aacdec.huffman_getsf;
            r0 = r0 - 60;
            r7 = r7 + r0;
            r0 = r7;
            jump update_scalefactors;

      update_scalefactors:
      M[I2, MK1] = r0;

   num_win_and_sfb_loop:

   // put things back for getbits to work
   call $aacdec.huffman_finish;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


   possible_corruption_in_huff:
      // put things back for getbits to work
      call $aacdec.huffman_finish;
      jump $aacdec.possible_corruption;

.ENDMODULE;

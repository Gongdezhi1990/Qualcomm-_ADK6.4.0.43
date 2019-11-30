// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_ELD_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.eld_specific_config
//
// DESCRIPTION:
//    Read the Enhanced Low Delay specific config info
//
// INPUTS:
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - r0, r2, r3, r10
//
// *****************************************************************************
.MODULE $M.aacdec.eld_specific_config;
   .CODESEGMENT AACDEC_ELD_SPECIFIC_CONFIG_PM;
   .DATASEGMENT DM;

   $aacdec.eld_specific_config:

   // push rLink onto stack
   push rLink;

   // Set SBR relatated loop count for ELD
   r0 = 16;
   M[r9 + $aac.mem.SBR_in_synth_loops] = r0;

   // read frameLengthFlag
   call $aacdec.get1bit;
   r0 = M[r9 + $aac.mem.ELD_frame_length_flag];
   M[r9 + $aac.mem.ELD_frame_length_flag] = r1;

   Null = r0 - r1;
   if Z jump eld_coefs_loaded;
      // Copy ELD window coefs into RAM from flash
      Null = r1;
      if Z jump win_size_512;
         // win size 480;
         r0 = &$aacdec.eld_win_480_coefs_flash;
         r1 = $aacdec.WIN_480_LD_SIZE;
         r2 = M[r9 + $aac.mem.ELD_win_coefs_ptr];
         call $aacdec.flash_copy_func;
         r0 = &$aacdec.eld_sin_const480_flash;
         r1 = 10;
         r2 = M[r9 + $aac.mem.ELD_sin_const_ptr];
         call $aacdec.flash_copy_func;
         jump eld_coefs_loaded;

      win_size_512:
         // win size 512;
         r0 = &$aacdec.eld_win_512_coefs_flash;
         r1 = $aacdec.WIN_512_LD_SIZE;
         r2 = M[r9 + $aac.mem.ELD_win_coefs_ptr];
         call $aacdec.flash_copy_func;
         r0 = &$aacdec.eld_sin_const_flash;
         r1 = 18;
         r2 = M[r9 + $aac.mem.ELD_sin_const_ptr];
         call $aacdec.flash_copy_func;
   eld_coefs_loaded:


   // read aacSectionDataResilienceFlag
   call $aacdec.get1bit;
   // If aacSectionDataResilienceFlag == 1 the AAC section data is encoded
   // with the Virtual Codebook 11 which we don't support.
   if NZ jump $aacdec.possible_corruption;

   // read aacScalefactorDataResilienceFlag
   call $aacdec.get1bit;
   // If aacScalefactorDataResilienceFlag == 1 the AAC scalefactor data is encoded
   // with the Reversible Variable Length Coding tool which we don't support.
   if NZ jump $aacdec.possible_corruption;

   // read aacSpectralDataResilienceFlag
   call $aacdec.get1bit;
   // If aacSpectralDataResilienceFlag == 1 the AAC spectral data is encoded
   // with Huffman Code Reordering tool which we don't support.
   if NZ jump $aacdec.possible_corruption;

   // read ld sbr present flag
   call $aacdec.get1bit;
   M[r9 + $aac.mem.ELD_sbr_present] = r1;
   // replicate the behaviour of reference decoder by setting audioSpecificConfig -> sbr_present
   // flag to the same value as ELDSpecificConfig->ld_sbr_present_flag
   M[r9 + $aac.mem.SBR_PRESENT_FIELD] = r1;

   if Z jump no_low_delay_sbr;

      // replicate the behaviour of reference decoder - set the extension AOT to SBR
      r0 = 5;
      M[r9 + $aac.mem.EXTENSION_AUDIO_OBJECT_TYPE_FIELD] = r0;

      // read ld sbr sampling rate
      call $aacdec.get1bit;
      M[r9 + $aac.mem.ELD_sbr_sampling_rate] = r1;

      // read ld sbr crc flag
      call $aacdec.get1bit;
      M[r9 +  $aac.mem.ELD_sbr_crc_flag] = r1;

      // read the sbr header
      r0 = M[r9 + $aac.mem.CHANNEL_CONFIGURATION_FIELD];
      call $aacdec.sbr_header;
   no_low_delay_sbr:

   /* read extension type, eld length descriptor, first and second additional lengths
   while (eldExtType != ELDEXT_TERM) {        4b
      eldExtLen;                              4b
      len = eldExtLen;
      if (eldExtLen == 15) {
         eldExtLenAdd;                        8b
         len += eldExtLenAdd;
      }
      if (eldExtLenAdd == 255) {
         eldExtLenAddAdd;                    16b
         len += eldExtLenAddAdd;
      }
   }*/
   extension_loop:
      // initialise r10 here - as this loop exits it will contain len.
      r10 = 0;
      call $aacdec.get4bits;
      Null = r1 - $aacdec.ELDEXT_TERM;
      if EQ jump end_extension_loop;
         // read eldExtLen
         call $aacdec.get4bits;
         r10 = r1;
         r1 = 0;

         Null = r10 - 15;
         // conditional read of eldExtLenAdd
         if EQ call $aacdec.get1byte;
         r2 = r1;
         r10 = r10 + r1;
         r1 = 0;

         // conditional read of eldExtLenAddAdd
         Null = r2 - 255;
         if EQ call $aacdec.get2bytes;
         r10 = r10 + r1;
      jump extension_loop;
   end_extension_loop:

   /* for(cnt=0; cnt<len; cnt++) {
      other_byte;                             8b
   } */

   // r10 is already set by the loop above (it can be 0)
   do discard_data;
      call $aacdec.get1byte;
   discard_data:

   r0 = $aacdec.DEFAULT_DELAY_512;
   r1 = $aacdec.DEFAULT_DELAY_480;
   Null = M[r9 + $aac.mem.ELD_frame_length_flag];
   if NZ r0 = r1;
   M[r9 + $aac.mem.ELD_delay_shift] = r0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif //AACDEC_ELD_ADDITIONS

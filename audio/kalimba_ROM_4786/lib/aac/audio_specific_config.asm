// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.audio_specific_config
//
// DESCRIPTION:
//    Read the audio specific config block
//
// INPUTS:
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - r0-r3, r6, r10, I1
//
// *****************************************************************************
.MODULE $M.aacdec.audio_specific_config;
   .CODESEGMENT AACDEC_AUDIO_SPECIFIC_CONFIG_PM;
   .DATASEGMENT DM;

   $aacdec.audio_specific_config:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.AUDIO_SPECIFIC_CONFIG_ASM.AUDIO_SPECIFIC_CONFIG.AUDIO_SPECIFIC_CONFIG.PATCH_ID_0, r3)
#endif
   
   // audioObjectType = getbits(5);
   call $aacdec.get5bits;
#ifdef AACDEC_ELD_ADDITIONS
   Null = r1 - 31;
   if NE jump set_aot;
      // audioObjectType = 32 + getbits(6);
      call $aacdec.get6bits;
      r1 = r1 + 32;
   set_aot:
#endif // AACDEC_ELD_ADDITIONS
   M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD] = r1;


   // sampling_frequency_index =\ getbits(4);
   call $aacdec.get4bits;
   M[r9 + $aac.mem.SF_INDEX_FIELD]  = r1;

   // if (sampling_frequency_index == 15)
   //    samplingFrequency = getbits(24);
   // end
   Null = r1 - 15;
   if NZ jump no_samplingfreq_value;

      r0 = 24;
      call $aacdec.getbits;

      // find the matching sf_index
      r0 = M[r9 + $aac.mem.SAMPLE_RATE_TAGS_FIELD];
      I1 = r0;
      r10 = 12;

      do find_sf_index_loop;
         r0 = M[I1, MK1];
         Null = r1 - r0;
         if Z jump sf_index_found;
      find_sf_index_loop:

      // Rate not found, so probably corrupt data
      jump corrupt_file_error;

      sf_index_found:
      r0 = M[r9 + $aac.mem.SAMPLE_RATE_TAGS_FIELD];
      r1 = I1 - r0;
      Addr2Words(r1);
      r1 = r1 - 1;
      M[r9 + $aac.mem.SF_INDEX_FIELD]  = Null;

   no_samplingfreq_value:

   // check sampling frequency is a valid value
   Words2Addr(r1);
   r0 = &$aacdec.sampling_freq_lookup;
   r10 = r1;
   call $mem.ext_window_access_as_ram;
   r1 = r10;
   r0 = M[r0 + r1];
   if NEG jump corrupt_file_error;

   // channelConfiguration = getbits(4);
   // if (channelConfiguration>2)
   //    error('Dont support >2 channels of audio');
   // end
   call $aacdec.get4bits;
   M[r9 + $aac.mem.CHANNEL_CONFIGURATION_FIELD] = r1;
   Null = r1 - 2;
   if GT jump corrupt_file_error;


   // sbrPresentFlag = 0;
   M[r9 + $aac.mem.SBR_PRESENT_FIELD] = Null;


   // if (audioObjectType == 5)
   //    extensionAudioObjectType = audioObjectType;
   //    sbrPresentFlag = 1;
   //    extensionSamplingFrequencyIndex = getbits(4);
   //    if extensionSamplingFrequencyIndex == 15
   //       extensionsamplingFrequency = getbits(24);
   //    end
   //    audioObjectType = getbits(5);
   // else
   //    extensionAudioObjectType = 0;
   // end
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.SBR;
   if NZ jump not_sbr;

      M[r9 + $aac.mem.EXTENSION_AUDIO_OBJECT_TYPE_FIELD] = r0;
      r0 = 1;
      M[r9 + $aac.mem.SBR_PRESENT_FIELD]= r0;

      // read and discard extension sampling frequency index
      call $aacdec.get4bits;
      Null = r1 - 15;
      if NZ jump no_extension_samplingfreq_value;
         r0 = 24;
         call $aacdec.getbits;
      no_extension_samplingfreq_value:

      call $aacdec.get5bits;
      M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD] = r1;
      jump sbr_select_done;
   not_sbr:
      M[r9 + $aac.mem.EXTENSION_AUDIO_OBJECT_TYPE_FIELD] = Null;
   sbr_select_done:


   // if (audioObjectType ~= 2) && (audioObjectType ~= 4)&& (audioObjectType ~= 39)
   //    error('Only AAC Object types 2 (AAC LC), 4 (AAC LTP) and 39 (ER AAC ELD) supported.');
   // end
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
#ifndef AACDEC_ELD_ADDITIONS
   Null = r0 - $aacdec.AAC_LC;
   if Z jump object_type_ok;
   Null = r0 - $aacdec.AAC_LTP;
   if NZ jump corrupt_file_error;

   object_type_ok:

   // ga_specific_config()
   call $aacdec.ga_specific_config;
#else
   // select appropriate function for reading specific config
   r1 = Null;
   r2 = $aacdec.ga_specific_config;
   r3 = $aacdec.eld_specific_config;
   Null = r0 - $aacdec.AAC_LC;
   if Z r1 = r2;
   Null = r0 - $aacdec.AAC_LTP;
   if Z r1 = r2;
   Null = r0 - $aacdec.ER_AAC_ELD;
   if Z r1 = r3;

   Null = r1;
   if Z jump corrupt_file_error;
      // read specific config
      call r1;

   // read epConfig
   call $aacdec.get2bits;
   // we may decide to do something different if we will support the
   // Error Protection tool but until then we reject the stream
   Null = r1 - 2;
   if EQ jump corrupt_file_error;
   Null = r1 - 3;
   if EQ jump corrupt_file_error;

#endif //AACDEC_ELD_ADDITIONS

   // pop rLink from stack
   jump $pop_rLink_and_rts;


   corrupt_file_error:
   jump $aacdec.possible_corruption;
.ENDMODULE;


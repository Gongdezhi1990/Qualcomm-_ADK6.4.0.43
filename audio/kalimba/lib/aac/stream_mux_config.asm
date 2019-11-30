// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.stream_mux_config
//
// DESCRIPTION:
//    Read the stream_mux_config
//
// INPUTS:
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - r0-r5, r10, I1
//
// *****************************************************************************
.MODULE $M.aacdec.stream_mux_config;
   .CODESEGMENT AACDEC_STREAM_MUX_CONFIG_PM;
   .DATASEGMENT DM;

   $aacdec.stream_mux_config:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.STREAM_MUX_CONFIG_ASM.STREAM_MUX_CONFIG.STREAM_MUX_CONFIG.PATCH_ID_0, r1)
#endif
   
   

   // latm.audioMuxVersion = getbits(1);
   // if (latm.audioMuxVersion==1)
   //    latm.audioMuxVersionA = getbits(1);
   // else
   //    latm.audioMuxVersionA = 0;
   // end
   call $aacdec.get1bit;
   M[r9 + $aac.mem.latm.AUDIO_MUX_VERSION] = r1;
   if NZ call $aacdec.get1bit;
   M[r9 + $aac.mem.latm.AUDIO_MUX_VERSION_A] = r1;



   // if (latm.audioMuxVersionA ~= 0)
   //    error('audioMuxVersionA non-zero not supported');
   // end
   if NZ jump $aacdec.possible_corruption;


   // if (latm.audioMuxVersion==1)
   //    taraBufferFullnesss = latm_get_value;
   // end
   r4 = 0;
   Null = M[r9 + $aac.mem.latm.AUDIO_MUX_VERSION];
   if NZ call $aacdec.latm_get_value;
   M[r9 + $aac.mem.latm.TARABUFFERFULLNESSS] = r4;


   // latm.allStreamsSameTimeFraming = getbits(1);
   // this bit must be set to be compliant with the A2DP spec
   call $aacdec.get1bit;
   if Z jump $aacdec.possible_corruption;


   // latm.numSubFrames = getbits(6);
   call $aacdec.get6bits;
   M[r9 + $aac.mem.latm.NUM_SUBFRAMES] = r1;


   // numProgram = getbits(4);
   // numLayer = getbits(3);
   // if (numProgram~=0) || (numLayer~=0)
   //    error('Only support single layer and single program streams');
   // end
   call $aacdec.get4bits;
   if NZ jump $aacdec.possible_corruption;
   call $aacdec.get3bits;
   if NZ jump $aacdec.possible_corruption;



   // if (latm.audioMuxVersion == 0)
   //    audio_specific_config;
   // else
   //    ascLen = latm_get_value;
   //    prevbitpos = filebitpos();
   //    audio_specific_config;
   //    ascLen = ascLen - (filebitpos() - prevbitpos);
   //    for i = 1:ascLen,
   //       getbits(1);
   //    end
   // end
   Null = M[r9 + $aac.mem.latm.AUDIO_MUX_VERSION];
   if NZ jump asc_data;
      call $aacdec.audio_specific_config;
      Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
      if NZ jump $aacdec.possible_corruption;

      jump audio_config_read;
   asc_data:
      call $aacdec.latm_get_value;

      M[r9 + $aac.mem.latm.ASC_LEN] = r4;
      r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
      M[r9 + $aac.mem.latm.PREVBITPOS] = r0;

      call $aacdec.audio_specific_config;
      Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
      if NZ jump $aacdec.possible_corruption;

      r10 = M[r9 + $aac.mem.latm.ASC_LEN];
      r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
      r10 = r10 - r0;
      r1 = M[r9 + $aac.mem.latm.PREVBITPOS];
      r10 = r10 + r1;
      if NEG jump $aacdec.possible_corruption;
      do loop;
         call $aacdec.get1bit;
      loop:
   audio_config_read:



   // latm.FrameLengthType = getbits(3);
   // if (latm.FrameLengthType==0)
   //    latm.latmBufferFullness = getbits(8);
   // else
   //    error('FrameLengthType not equal to 0 isn''t suppported');
   // end
   call $aacdec.get3bits;
   if NZ jump $aacdec.possible_corruption;
   call $aacdec.get1byte;
   M[r9 + $aac.mem.latm.LATM_BUFFER_FULLNESS] = r1;



   // latm.otherDataPresent = getbits(1);
   // if (latm.otherDataPresent)
   //    if (latm.audioMuxVersion==1)
   //       latm.otherDataLenBits = latm_get_value;
   //    else
   //       latm.otherDataLenBits = 0;
   //       esc = 1;
   //       while (esc),
   //          latm.otherDataLenBits = latm.otherDataLenBits * 256;
   //          esc = getbits(1);
   //          latm.otherDataLenBits = latm.otherDataLenBits + getbits(8);
   //       end
   //    end
   // end
   call $aacdec.get1bit;
   r4 = r1;
   if Z jump store_other_data_len_bits;

      Null = M[r9 + $aac.mem.latm.AUDIO_MUX_VERSION];
      if Z jump audio_mux_version_zero;
         call $aacdec.latm_get_value;
         jump store_other_data_len_bits;

      audio_mux_version_zero:
         r4 = 0;
         another_word_loop:
            r4 = r4 LSHIFT 8;
            call $aacdec.get1bit;
            r5 = r1;
            call $aacdec.get1byte;
            r4 = r4 + r1;
            Null = r5;
         if NZ jump another_word_loop;

   store_other_data_len_bits:
   M[r9 + $aac.mem.latm.OTHER_DATA_LEN_BITS] = r4;


   // crc_check_present = getbits(1);
   // if (crc_check_present)
   //    latm.crcCheckSum = getbits(8);
   // end
   call $aacdec.get1bit;
   // discard the CRC checkword
   if NZ call $aacdec.get1byte;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


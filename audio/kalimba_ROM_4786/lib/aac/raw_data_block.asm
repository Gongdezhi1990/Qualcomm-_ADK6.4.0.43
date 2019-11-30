// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.raw_data_block
//
// DESCRIPTION:
//    Get a raw data block
//
// INPUTS:
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.raw_data_block;
   .CODESEGMENT AACDEC_RAW_DATA_BLOCK_PM;
   .DATASEGMENT DM;

   $aacdec.raw_data_block:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.RAW_DATA_BLOCK_ASM.RAW_DATA_BLOCK.RAW_DATA_BLOCK.PATCH_ID_0, r1)
#endif

   // zero the SCE and CPE counters
   M[r9 + $aac.mem.NUM_SCEs] = Null;
   M[r9 + $aac.mem.NUM_CPEs] = Null;

   id_loop:

      call $aacdec.get3bits;
      r0 = r9 + $aac.mem.SYNTATIC_ELEMENT_FUNC_TABLE;
      Words2Addr(r1);
      r1 = M[r0 + r1];
      if Z jump $pop_rLink_and_rts;
      if NEG jump $aacdec.possible_corruption;
      call r1;
      Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
      if NZ jump $aacdec.possible_corruption;

#ifdef AACDEC_ENABLE_LATM_GARBAGE_DETECTION
      #ifdef AACDEC_MP4_FILE_TYPE_SUPPORTED
         r0 = M[r9 + $aac.mem.READ_FRAME_FUNCTION];
         Null = r0 - &$aacdec.mp4_read_frame;
         if Z jump id_loop;
      #endif
      // garbage check
      r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
      Null = r0 - ($aacdec.MIN_AAC_FRAME_SIZE_IN_BYTES*8*2);
      if POS jump $aacdec.possible_corruption;
#endif

      jump id_loop;

.ENDMODULE;

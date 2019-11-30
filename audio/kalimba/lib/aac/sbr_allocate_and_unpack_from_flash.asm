// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_SBR_ADDITIONS

// *****************************************************************************
// MODULE:
//    $allocate_and_unpack_from_flash
//
// DESCRIPTION:
//    allocate memory and copy from flash a huffman table
//
// INPUTS:
//    r0 = address of codebook in flash segment
//    r2 = address of flash section
//    r7 = codebook size in 24bit memory
//
// OUTPUTS:
//    r1 = address of huffman table in temporary memory
//
// TRASHED REGISTERS:
//    - r0-r3, r10, I1, I2
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_allocate_and_unpack_from_flash;
   .CODESEGMENT AACDEC_SBR_ALLOCATE_AND_UNPACK_FROM_FLASH_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_allocate_and_unpack_from_flash:

   // push rLink onto stack
   push rLink;

   // map the appropriate flash page into the flash window
   call $mem.ext_window_access_as_ram;
   I1 = r0;

   // allocate tmp memory for unpacked huffman codebook
   //  - these can be destroyed after all huffman data has been read
   r0 = r7;
   call $aacdec.frame_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;


   I2 = r1;
   r10 = r7;

   do flash_copy_loop;
      r0 = M[I1, MK1];
      M[I2, MK1] = r0;
   flash_copy_loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;

#endif




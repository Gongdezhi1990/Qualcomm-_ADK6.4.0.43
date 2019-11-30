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
//    $aacdec.sbr_read_one_word_from_flash
//
// DESCRIPTION:
//    read one word from an SBR table in flash
//
// INPUTS:
//    r0 = address of word in flash segment
//    r2 = address of flash section
//
// OUTPUTS:
//    r0 = the word read from flash
//
// TRASHED REGISTERS:
//    r1, r2, r3
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_read_one_word_from_flash;
   .CODESEGMENT AACDEC_SBR_READ_ONE_WORD_FROM_FLASH_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_read_one_word_from_flash:

   // push rLink onto stack
   push rLink;

   // map the appropriate flash page into the flash window
   r1 = 1;
   call $mem.ext_window_access_as_ram;
   r0 = M[r0];

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif




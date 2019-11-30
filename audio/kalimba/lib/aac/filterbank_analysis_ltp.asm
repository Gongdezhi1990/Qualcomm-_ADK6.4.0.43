// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.filterbank_analysis_ltp
//
// DESCRIPTION:
//    Analysis filterbank (windowing and mdct)
//
// INPUTS:
//    - I2 = pointer to mdct input buffer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume all
//
// *****************************************************************************
.MODULE $M.aacdec.filterbank_analysis_ltp;
   .CODESEGMENT AACDEC_FILTERBANK_ANALYSIS_LTP_PM;
   .DATASEGMENT DM;

   $aacdec.filterbank_analysis_ltp:

   // push rLink onto stack
   push rLink;

   // window the 2048 sample mdct-input
   // flag not to update prev_window_shape
   M0 = -1;
   r4 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   I5 = r4;
   call $aacdec.windowing;
   M0 = 2;
   r4 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r4 = r4 + (1024*ADDR_PER_WORD);
   I5 = r4;
   call $aacdec.windowing;

   // initialise input structure for mdct
   r6 = r9 + $aac.mem.MDCT_INFORMATION;
   r0 = 2048;
   M[r6 + $aacdec.mdct.NUM_POINTS_FIELD] = r0;
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r6 + $aacdec.mdct.INPUT_ADDR_FIELD] = r0;
   call $math.address_bitreverse;
   r0 = r1;
   M[r6 + $aacdec.mdct.INPUT_ADDR_BR_FIELD] = r0;

   // do the mdct
   call $aacdec.mdct;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

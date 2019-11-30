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
//    $aacdec.eld_sbr_block
//
// DESCRIPTION:
//    Based on the "channelconfiguration" value , configure the inputs to
//    "low_delay_sbr_data" function and call it
//
// INPUTS:
//    r0 - channel configuration
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    - All
//
// *****************************************************************************
.MODULE $M.aacdec.eld_sbr_block;
   .CODESEGMENT AACDEC_ER_LD_SBR_PM;
   .DATASEGMENT DM;

   $aacdec.eld_sbr_block:

   push rLink;

   Null = r0 - 1;
   if NZ jump case_2;
   case_1:
      r4 = $aacdec.ID_SCE;
      call $aacdec.eld_sbr_data;
      jump low_delay_sbr_block_done;
   case_2:
      r4 = $aacdec.ID_CPE;
      call $aacdec.eld_sbr_data;

   low_delay_sbr_block_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif


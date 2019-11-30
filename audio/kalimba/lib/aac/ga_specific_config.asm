// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.ga_specific_config
//
// DESCRIPTION:
//    Read the ga specific config info
//
// INPUTS:
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - r0-r3, r6, r10
//
// *****************************************************************************
.MODULE $M.aacdec.ga_specific_config;
   .CODESEGMENT AACDEC_GA_SPECIFIC_CONFIG_PM;
   .DATASEGMENT DM;

   $aacdec.ga_specific_config:

   // push rLink onto stack
   push rLink;


   // frameLengthFlag = getbits(1);
   // dependsOnCoreCoder = getbits(1);
   // if (dependsOnCoreCoder)
   //    coreCoderDelay = getbits(14);
   // end
   //
   // extensionFlag = getbits(1);
   // if (channelConfiguration==0)
   //    program_element_config;
   // end

   // discard frameLengthFlag
   call $aacdec.get1bit;


   // read coreCoderDelay if required
   call $aacdec.get1bit;
   Null = r1;
   if Z jump no_corecoderdelay;
      r0 = 14;
      call $aacdec.getbits;
   no_corecoderdelay:


   // discard extensionFlag
   call $aacdec.get1bit;


   // read program_element_config if required
   r0 = M[r9 + $aac.mem.CHANNEL_CONFIGURATION_FIELD];
   if Z call $aacdec.program_element_config;


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;



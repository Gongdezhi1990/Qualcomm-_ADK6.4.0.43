// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBCDEC_INIT_DECODER_INCLUDED
#define SBCDEC_INIT_DECODER_INCLUDED

#include "stack.h"
#include "profiler.h"

#ifndef KYMERA
#include "kalimba_standard_messages.h"
#endif

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcdec.init_decoder
//
// DESCRIPTION:
//    Initialise variables for sbc decoding. This is the initialisation function
//    to call when using dynamic external tables.
//
// INPUTS:
//    - r9 = pointer to decoder structure
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r3, r9, r10, DoLoop, I0
//
// *****************************************************************************
.MODULE $M.sbcdec.init_decoder;
   .CODESEGMENT SBCDEC_INIT_DECODER_PM;
   .DATASEGMENT DM;

   $sbcdec.init_decoder:

   // push rLink onto stack
   push rLink;


   // -- reset decoder variables --
   call $sbcdec.reset_decoder;

   call $sbcdec.init_tables;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_sbc_decode_init
//
// DESCRIPTION:
//    C - callable version of $sbcdec.init_decoder.
//
// INPUTS:
//    - r0 = pointer to decoder structure
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r3, r10, DoLoop
//
// *****************************************************************************
.MODULE $M.sbcdec.init_decoder;
   .CODESEGMENT SBCDEC_INIT_DECODER_PM;
   .DATASEGMENT DM;

   $_sbc_decode_lib_init:
   // Save the registers C expects us to preserve that get trashed
   pushm <r5, r9, rLink>;
   push I0;

   r9 = r0;

   call $sbcdec.init_decoder;

   // restore preserved registers
   pop I0;
   popm <r5, r9, rLink>;

   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $sbcdec.deinit_decoder
//
// DESCRIPTION:
//    It is kept for consistency in terms of lifecycle functions. Functionality
//    needed in deinit stage can be added here. 
//    This function's static variant (deinit_static_decoder) is the one where
//    message handlers are removed. 
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3
//
// *****************************************************************************
.MODULE $M.sbcdec.deinit_decoder;
   .CODESEGMENT SBCDEC_DEINIT_DECODER_PM;
   .DATASEGMENT DM;

   $sbcdec.deinit_decoder:

   rts;

.ENDMODULE;
#endif
#endif

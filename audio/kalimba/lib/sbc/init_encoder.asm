// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBCENC_INIT_ENCODER_INCLUDED
#define SBCENC_INIT_ENCODER_INCLUDED

#include "stack.h"
#include "profiler.h"

#ifndef KYMERA
#include "kalimba_standard_messages.h"
#endif

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.init_encoder
//
// DESCRIPTION:
//    Initialise variables for sbc encoding. This is the initialisation function
//    to call when using dynamic external tables.
//
// INPUTS:
//    - r9 = pointer to encoder structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r3, r10, DoLoop, I0
//
// *****************************************************************************
.MODULE $M.sbcenc.init_encoder;
   .CODESEGMENT SBCENC_INIT_ENCODER_PM;
   .DATASEGMENT DM;

   $sbcenc.init_encoder:

   // push rLink onto stack
   push rLink;

   // -- initialise encoder variables --
   call $sbcenc.reset_encoder;

   call $sbcenc.init_tables;
   
   // Initialise some working data 
   r0 = 16;
   M[r9 + $sbc.mem.PUT_BITPOS_FIELD] = r0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;



.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_sbc_encode_init
//
// DESCRIPTION:
//    C - callable version of $sbcenc.init_decoder.
//
// INPUTS:
//    - r0 = pointer to encoder structure
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r3, r10, DoLoop
//
// *****************************************************************************
.MODULE $M.sbcenc.init_encoder;
   .CODESEGMENT SBCENC_INIT_ENCODER_PM;
   .DATASEGMENT DM;

   $_sbc_encode_lib_init:
   // Save the registers C expects us to preserve that get trashed
   pushm <r5, r9, rLink>;
   push I0;

   r9 = r0;

   call $sbcenc.init_encoder;

   // restore preserved registers
   pop I0;
   popm <r5, r9, rLink>;

   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $sbcenc.deinit_encoder
//
// DESCRIPTION:
//    It is kept for consistency in terms of lifecycle functions. Functionality
//    needed in deinit stage can be added here. 
//    This function's static variant (deinit_static_encoder) is the one where
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
.MODULE $M.sbcenc.deinit_encoder;
   .CODESEGMENT SBCENC_DEINIT_ENCODER_PM;
   .DATASEGMENT DM;

   $sbcenc.deinit_encoder:

   rts;

.ENDMODULE;
#endif
#endif

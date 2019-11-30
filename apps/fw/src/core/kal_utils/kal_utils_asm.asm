// Copyright (c) 2016 Qualcomm Technologies International, Ltd.
//   %%version
//!
//! \file kal_utils_asm.asm
//!  Assembler functions called from C code
//!
//! \ingroup platform
//!


// *****************************************************************************
//!
//! \fn $pl_sign_detect_asm
//! \brief  Calls the SIGNDET assembly function
//!
//! \param rMAC value on which to find number of redundant sign bits
//!
//! \return rMAC number of redundant sign bits
//!
//! \note
//! TRASHED REGISTERS:
//!    rMac only - other registers used are restored.
//!
// *****************************************************************************
.MODULE $M.pl_intrinsics_asm;
   .CODESEGMENT PM;
   .DATASEGMENT DM;
   .MINIM;

$_pl_sign_detect_asm:
        r0= SIGNDET r0;
        rts;
 
.ENDMODULE;

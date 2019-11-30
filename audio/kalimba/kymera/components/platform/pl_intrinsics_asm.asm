/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
//!
//! \file pl_intrinsics_asm.asm
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

// *****************************************************************************
// MODULE:
//    $_pl_fractional_divide
//    unsigned pl_fractional_divide(unsigned num,unsigned den);
//
// INPUTS:
//      r0 = Numerator
//      r0 = Denominator
// OUTPUTS:
//      none
// DESCRIPTION:
//    Perform fraction devide.  Function is C compatible
//
// *****************************************************************************
.MODULE $M.pl_intrinsics_asm;
   .CODESEGMENT EXT_DEFINED_PM;
   .DATASEGMENT DM;

$_pl_sign_detect_asm:
        r0= SIGNDET r0;
        rts;

$_pl_fractional_divide:
   // Normalize numerator and denominator
   r2 = SIGNDET r0;
   r3 = r2 - 1;
   rMAC = r0 ASHIFT r3;
   r3 = SIGNDET r1;
   r1 = r1 ASHIFT r3;
   // Perform Divide
   Div = rMAC/r1;
   r3 = r3 - r2;
   r0 = DivResult;
   // DeNormalize result
   r0 = r0 ASHIFT r3;
   rts;

.ENDMODULE;

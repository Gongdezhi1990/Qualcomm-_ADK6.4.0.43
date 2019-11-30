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
//    $aacdec.sbr_fp_mult_frac
//
// DESCRIPTION:
//    Multiply a floating point number by a fractional and format the answer as
//    lower 12 (or 20 if K32) bits mantissa, upper 12 exponent
//
// INPUTS:
//    - r0 - exponent
//    - rMAC - mantissa
//    - r4 - fractional
//
// OUTPUTS:
//    - r2 - answer
//
// TRASHED REGISTERS:
//    - rMAC
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_fp_mult_frac;
   .CODESEGMENT AACDEC_SBR_FP_MULT_FRAC_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_fp_mult_frac:

   rMAC = r4 * rMAC;

   r2 = SIGNDET rMAC;
   r2 = r2 - 11;
   rMAC = rMAC LSHIFT r2;
   r2 = r0 - r2;

   r2 = r2 ASHIFT (DAWTH-12);
   r2 = rMAC + r2;
   rts;

.ENDMODULE;

#endif

// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef MATH_LOG2_INCLUDED
#define MATH_LOG2_INCLUDED

#include "math_library.h"

//******************************************************************************
// MODULE:
//    $math.log2_taylor
//
// DESCRIPTION:
//    Calculate y = log2(x) in taylor series method
//
// INPUTS:
//    - rMAC = x (double presicion), in Q9.47 (KAL_ARCH3) / Q9.63 (KAL_ARCH4)
//
// OUTPUTS:
//    - r0 = log2(x), in Q8.16 (KAL_ARCH3) or Q8.24 (KAL_ARCH4)
//
// TRASHED REGISTERS:
//    rMAC, r0, r1, r6, I7
//
// CPU USAGE:
//    23 cycles   KAL_ARCH3
//
// NOTES:
//    For log2(0) this routine returns = log2(1/2^(NBITS*2)).
//
//******************************************************************************
.MODULE $M.math.log2_taylor;
   .CODESEGMENT MATH_LOG2_TAYLOR_PM;
   .DATASEGMENT DM;


#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)
   .VAR/DM_P1_RW log2_coefs[7] = 
#elif defined(USE_SHARE_MEM)
   .VAR/DM_P0_RW log2_coefs[7] = 
#else
   .VAR/DM1 log2_coefs[7] = 
#endif
                        -0.0124136209,   0.0589549541,  -0.1361793280,
                         0.2269296646,  -0.3584938049,   0.7211978436,
                         0.0000063181;


$math.log2_taylor:
   // 20 cycles.  saves 2 cycles .  if can use I0:I3 and M1=1 could save 4 more
   push I0;
   push M1;
   // load table address
   I0 = &log2_coefs;
   M1 = MK1;

   // -P
   r6 = SIGNDET rMAC;
   // M, C6
   rMAC = rMAC ASHIFT r6, r0 = M[I0,M1];
   // 4*(M - 1/2), unsigned
   r1 = rMAC LSHIFT 2;
   // x = 2*M - 1
   rMAC = r1 LSHIFT -1;

   // 1 - P
   r6 = r6 + 1,         r1=M[I0,M1];
   // integer part: negative
//   r6 = r6 ASHIFT 16;
   r6 = r6 ASHIFT (DAWTH - 8);     // Q8.24 == OR == Q8.32

   // C5 + C6*x
   r1 = r1 + r0*rMAC ,   r0=M[I0,M1];
   // C4 + (C6*x + C5)*x
   r0 = r0 + r1*rMAC ,   r1=M[I0,M1];
   // C3 + (C6*x^2 + C5*x + C4)*x
   r1 = r1 + r0*rMAC ,   r0=M[I0,M1];
   // C2 + (C6*x^3 + C5*x^2 + C4*x + C3)*x
   r0 = r0 + r1*rMAC ,   r1=M[I0,M1];
   // C1 + (C6*x^4 + C5*x^5 + C4*x^2 + C3*x + C2)*x
   r1 = r1 + r0*rMAC ,   r0=M[I0,M1];
   // C0 + (C6*x^5 + C5*x^4 + C4*x^3 + C3*x^2 + C2*x + C1)*x
   r0 = r0 + r1*rMAC ;
   // fractional part
   r1 = r0 ASHIFT -6;
   // Q8.16 / Q8.24 format
   r0 = r1 - r6;
   pop M1;
   pop I0;
   rts;

.ENDMODULE;





//******************************************************************************
// MODULE:
//    $math.log2_table
//
// DESCRIPTION:
//    Calculate y = log2(x) in table look up method
//
// INPUTS:
//    - rMAC = x (double presicion), in Q9.47 (KAL_ARCH3) / Q9.63 (KAL_ARCH4)
//
// OUTPUTS:
//    - r0 = log2(x), in Q8.16 (KAL_ARCH3) or Q8.24 (KAL_ARCH4)
//
// TRASHED REGISTERS:
//    rMAC, r1, r6
//
// CPU USAGE:
//    14 cycles
//
// NOTES:
//    For log2(0) this routine returns = log2(1/2^(NBITS*2)).
//
//******************************************************************************
.MODULE $M.math.log2_table;
   .CODESEGMENT MATH_LOG2_TABLE_PM;
   .DATASEGMENT DM;

#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)
   .VAR/DM_P1_RW tab32_log2[] =
#elif defined(USE_SHARE_MEM)
   .VAR/DM_P0_RW tab32_log2[] =
#else
   .VAR/DM1 tab32_log2[] =
#endif
      Qfmt_(0.0000000000, 8),    Qfmt_(0.0443954560, 8),    Qfmt_(0.0874633728, 8),
      Qfmt_(0.1292801024, 8),    Qfmt_(0.1699218688, 8),    Qfmt_(0.2094497792, 8),
      Qfmt_(0.2479248000, 8),    Qfmt_(0.2854003968, 8),    Qfmt_(0.3219299328, 8),
      Qfmt_(0.3575515648, 8),    Qfmt_(0.3923187200, 8),    Qfmt_(0.4262619136, 8),
      Qfmt_(0.4594345216, 8),    Qfmt_(0.4918518016, 8),    Qfmt_(0.5235595648, 8),
      Qfmt_(0.5545883264, 8),    Qfmt_(0.5849609344, 8),    Qfmt_(0.6147079552, 8),
      Qfmt_(0.6438598656, 8),    Qfmt_(0.6724243200, 8),    Qfmt_(0.7004394496, 8),
      Qfmt_(0.7279205376, 8),    Qfmt_(0.7548904448, 8),    Qfmt_(0.7813568128, 8),
      Qfmt_(0.8073577856, 8),    Qfmt_(0.8328933632, 8),    Qfmt_(0.8579788160, 8),
      Qfmt_(0.8826446592, 8),    Qfmt_(0.9068908672, 8),    Qfmt_(0.9307403520, 8),
      Qfmt_(0.9541931136, 8),    Qfmt_(0.9772796672, 8),    Qfmt_(1.0000000000, 8);

    .CONST   $LOG2_TBL_INDX   5;

$math.log2_abs_table:
   // number of leading zeros
   r6 = SIGNDET rMAC;
   // normalised
   rMAC = rMAC ASHIFT r6;
   // ensure positive (absolute value)
   if NEG rMAC = NULL - rMAC;
   jump continue_jp;

$math.log2_table:
   // number of leading zeros
   r6 = SIGNDET rMAC;
   // normalised
   rMAC = rMAC ASHIFT r6;
continue_jp:

   // 1 - P
   r6 = r6 + 1;
   // integer part: negative
   r6 = r6 ASHIFT (DAWTH - 8);

   // 4*(M - 1/2), unsigned
   r1 = rMAC LSHIFT 2;
   // table index
   r1 = r1 LSHIFT -(DAWTH - $LOG2_TBL_INDX);

   Words2Addr(r1);      // KAL_ARCH 4 words to address conversion macro

   r0 = rMAC LSHIFT ($LOG2_TBL_INDX + 2);
   // interpolation factor
   r0 = r0 LSHIFT -1;

   // base value
   rMAC = M[r1 + (&tab32_log2 + 0)];            // base value
   r1 = M[r1 + (&tab32_log2 + ADDR_PER_WORD)];
   // interpolation range
   r1 = r1 - rMAC;
   // interpolation bits and fractional part
   rMAC = rMAC + r0 * r1;
   // Q8.16 / Q8.24 format
   r0 = rMAC - r6;
   rts;

.ENDMODULE;

#endif // MATH_LOG2_INCLUDED

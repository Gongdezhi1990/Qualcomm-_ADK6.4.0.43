// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// *****************************************************************************

// ASM functions for manipulating volume
// The functions obey the C compiler calling convention (see documentation, CS-124812-UG)


#include "portability_macros.h"

#ifdef PATCH_LIBS
#include "patch_library.h"
#endif



// *****************************************************************************
// MODULE:
//    $_dB60toLinearQ5
//
// DESCRIPTION:
//    Convert dB gain (1/60th of a dB resolution) into a linear
//    gain in Q23/31 format (range -1.0 to +1.0)
//         dB = 20*log10(linear gain)
//           maximum dB gain supported is 96.0 dB
//
// INPUTS:
//    - r0 = 60ths of a dB
//
// OUTPUTS:
//    - r0 = linear gain
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
.MODULE $M.GainConversion;
    .CODESEGMENT    PM_FLASH;

#if (DAWTH==24)
#define DB_CONVERT_FRACTIONAL 0.4215663554485
#define DB_CONVERT_INTEGER    181
#elif (DAWTH==32)
#define DB_CONVERT_FRACTIONAL 0.92098699481
#define DB_CONVERT_INTEGER    46443
#else
#error "DAWTH not supported!"
#endif

$_dB60toLinearFraction:
    // protect registers trashed by pow2_taylor
    pushm <r6,r7,rLink>;

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($common_cap.GAIN_CONVERSION_ASM.GAINCONVERSION.DB60TOLINEARFRACTION.PATCH_ID_0, r1)
#endif

    // What we want to calculate is: 10^(Lp/20)
    // We do this by converting the maths to base 2 to make it simpler and faster
    // in binary operations.
    // log2(x) = log10(x)/20*log10(2)
    // log2 format is Q8.xx ,   Conversion from 1/60dB is (2^xx/60)
    // (2^16)/(20*log10(2)*60) =  181.4215663554485
    // (2^24)/(20*log10(2)*60) =  46443.92098699481
    r1 = DB_CONVERT_FRACTIONAL;
    rMAC = r0 * DB_CONVERT_INTEGER (int);
    rMAC = rMAC + r0 * r1;

    r0 = rMAC;
    if POS r0 = 0;
    call $math.pow2_taylor;

    // Restore registers
    popm <r6,r7,rLink>;
    rts;

// *****************************************************************************
// MODULE:
//    $_dB60toLinearQ5
//
// DESCRIPTION:
//    Convert dB gain (1/60th of a dB resolution) into a linear
//    gain in Q5.xx format (range -16.0 to +16.0)
//         dB = 20*log10(linear gain)
//           maximum dB gain supported is 96.0 dB
//
// INPUTS:
//    - r0 = dB
//
// OUTPUTS:
//    - r0 = linear gain
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
$_dB60toLinearQ5:
    // protect registers trashed by pow2_taylor
    pushm <r6,r7,rLink>;

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($common_cap.GAIN_CONVERSION_ASM.GAINCONVERSION.DB60TOLINEARQ5.PATCH_ID_0, r1)
#endif

    // log2(x) = log10(x)/20*log10(2)
    // log2 format is Q8.xx ,   Conversion from 1/60dB is (2^xx/60)
    // (2^16)/(20*log10(2)*60) =  181.4215663554485
    // (2^24)/(20*log10(2)*60) =  46443.92098699481
    r1 = DB_CONVERT_FRACTIONAL;
    rMAC = r0 * DB_CONVERT_INTEGER (int);
    rMAC = rMAC + r0 * r1;

    // 4-bits integer in result (Q5.19)
    //    log2(volume/16.0) = log2(volume) - log2(16.0)
    r0 = rMAC - (1<<(DAWTH-6));
    if POS r0 = 0;
    call $math.pow2_taylor;

    // Restore registers
    popm <r6,r7,rLink>;
    rts;

// *****************************************************************************
// MODULE:
//    $_gain_linear2dB60
//
// DESCRIPTION:
//    gain_dB60 = 20*log10(gain)*60
//
// MODIFICATIONS:
//
// INPUTS:
//    - r0 : gain (Q1.N, linear)
//
// OUTPUTS:
//    - r0 : dB60 (integer)
//
// TRASHED REGISTERS:
//    - rMAC, r1
//
// CPU USAGE:
// *****************************************************************************
$_gain_linear2dB60:
   // protect registers trashed by pow2_taylor
   pushm <r6,rLink>;

   // log2(gain): Q8.N
   rMAC = r0;
   call $math.log2_taylor;

   // 20*log10(2)*60 = 0.705539052337455950 : Q10.N
   // log2(gain)*20*log10(2)*60             : Q17.N
   rMAC = r0 * 0.705539052337455950;

   // make it integer
   r0 = rMAC ASHIFT -(DAWTH-17);

   // Restore registers
   popm <r6,rLink>;
   rts;

.ENDMODULE;

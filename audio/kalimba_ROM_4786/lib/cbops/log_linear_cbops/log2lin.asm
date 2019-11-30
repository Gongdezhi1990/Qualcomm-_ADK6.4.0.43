// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// *****************************************************************************

// *****************************************************************************
// MODULE:
//    $g711.alaw2linear
//
// DESCRIPTION:
//    Encodes an A-law value as a linear PCM value
//
// INPUTS:
//    - r0 = A-law value
//
// OUTPUTS:
//    - r0 = Linear PCM value
//
// TRASHED REGISTERS:
//    r1-3
//
// NOTES:
//
// For further information see ITU-T Recommendation G.711
// *****************************************************************************

.MODULE $M.alaw2linear;
   .CODESEGMENT PM_FLASH;

   $_g711_alaw2linear:  // C entry point so that it can see the address
   $g711.alaw2linear:

// toggle even bits
   r1 = r0 XOR 0x55;
   r1 = r1 AND 0x7f;
   r2 = r1 ASHIFT -4;
   r3 = r1 AND 0x0f;
// TODO optimise this if statement
   Null = r2 - 1;
   if NEG jump endif_iexp_gt_0;
      r3 = r3 + 16;
   endif_iexp_gt_0:
   r3 = r3 ASHIFT 4;
   r3 = r3 + 0x08;
   Null = r2 - 2;
   if NEG jump endif_iexp_gt_1;
      r2 = r2 - 1;
      r3 = r3 ASHIFT r2;
   endif_iexp_gt_1:
   Null = r0 - 128;
   if NEG r3 = -r3;

   // Shift up to msb align the output (13 bit up to 24bits required by A-law)
   r0 = r3 ASHIFT (DAWTH - 13);

   rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $g711.ulaw2linear
//
// DESCRIPTION:
//    Encodes a u-law value as a linear PCM value
//
// INPUTS:
//    - r0 = u-law value
//
// OUTPUTS:
//    - r0 = Linear PCM value
//
// TRASHED REGISTERS:
//    r1-5
//
// NOTES:
//
// For further information see ITU-T Recommendation G.711
// *****************************************************************************

.MODULE $M.ulaw2linear;
   .CODESEGMENT PM_FLASH;

   $_g711_ulaw2linear:  // C entry point so that it can see the address
   $g711.ulaw2linear:

//  long            n;     /* aux.var. */
//  short           segment;  /* segment (Table 2/G711, column 1) */
//  short           mantissa; /* low  nibble of log companded sample */
//  short           exponent; /* high nibble of log companded sample */
//  short           sign;     /* sign of output sample */
//  short           step;
//
//  for (n = 0; n < lseg; n++)
//  {
//    sign = logbuf[n] < (0x0080)   /* sign-bit = 1 for positiv values */
//      ? -1
//      : 1;
   r1 = 1;
   Null = r0 - 0x80;
   if NEG r1 = -r1;

//    mantissa = ~logbuf[n];  /* 1's complement of input value */
   r2 = r0 XOR -1;
//    exponent = (mantissa >> 4) & (0x0007); /* extract exponent */
   r3 = r2 ASHIFT -4;
   r3 = r3 AND 0x7;
//    segment = exponent + 1; /* compute segment number */
   r4 = r3 + 1;
//    mantissa = mantissa & (0x000F);  /* extract mantissa */
   r2 = r2 AND 0xf;
//
//    /* Compute Quantized Sample (14 bit left justified!) */
//    step = (4) << segment;  /* position of the LSB */
   r5 = 4;
   r5 = r5 ASHIFT r4;
//    /* = 1 quantization step) */
//    linbuf[n] = sign *      /* sign */
//      (((0x0080) << exponent)  /* '1', preceding the mantissa */
//       + step * mantissa /* left shift of mantissa */
//       + step / 2     /* 1/2 quantization step */
   r0 = r5 ASHIFT -1;
   r5 = r5 * r2 (int);
   r0 = r0 + r5;
//       - 4 * 33
   r0 = r0 - 132;
   r3 = 0x80 ASHIFT r3;
   r0 = r0 + r3;

   Null = r1;
   if NEG r0 = -r0;
//      );
//  }
//}

   // Shift up to msb align the output (14 bit up to 24bits required by U-law)
   r0 = r0 ASHIFT (DAWTH - 14);

   rts;

.ENDMODULE;


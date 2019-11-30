// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// *****************************************************************************

// *****************************************************************************
// MODULE:
//    $g711.linear2alaw
//
// DESCRIPTION:
//    Encodes a linear PCM value as an A-law value
//
// INPUTS:
//    - r0 = Linear PCM value
//
// OUTPUTS:
//    - r0 = A-law value
//
// TRASHED REGISTERS:
//    r1-3
//
// NOTES:
//
// For further information see ITU-T Recommendation G.711
// *****************************************************************************

.MODULE $M.linear2alaw;
   .CODESEGMENT PM_FLASH;

   $_g711_linear2alaw:  // C entry point so that it can see the address
   $g711.linear2alaw:

   // Shift down since input is msb aligned (24 bit down to 13bits required by A-law)
   r0 = r0 ASHIFT (13 - DAWTH);

   Null = r0;
   if POS jump if_input_gte_0;
      // Set the sign bit for use later on
      r1 = 0x00;
      // Calculate ix
      r2 = r0 XOR -1;    // Invert r0
      r2 = r2 ASHIFT -4;
      jump endif_input_gte_0;
   if_input_gte_0:
      // Set the sign bit for use later on
      r1 = 0x80;
      // Calculate ix
      r2 = r0 ASHIFT -4;
   endif_input_gte_0:

   Null = r2 - 16;
   if NEG jump endif_ix_gt_15;
      r3 = 1;
      mant_exp_loop:
         Null = r2 - 32;
         if NEG jump mant_exp_loop_break;

         r2 = r2 ASHIFT -1;
         r3 = r3 + 1;
         jump mant_exp_loop;
      mant_exp_loop_break:
      r2 = r2 - 16;
      r3 = r3 ASHIFT 4;
      r2 = r2 + r3;
   endif_ix_gt_15:

   r2 = r2 OR r1; // sign bit is stored in r1

// toggle even bits
   r0 = r2 XOR 0x55; // Return value in r0
   r0 = r0 AND 0xff; // Mask out any other values

   rts;
.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $g711.linear2ulaw
//
// DESCRIPTION:
//    Encodes a linear PCM value as a u-law value
//
// INPUTS:
//    - r0 = Linear PCM value
//
// OUTPUTS:
//    - r0 = u-law value
//
// TRASHED REGISTERS:
//    r1-6
//
// NOTES:
//
// For further information see ITU-T Recommendation G.711
// *****************************************************************************
.MODULE $M.linear2ulaw;
   .CODESEGMENT PM_FLASH;

   $_g711_linear2ulaw:  // C entry point so that it can see the address
   $g711.linear2ulaw:

   // Assume positive and set bit 7
   r6 = 0x80;

   // Shift down since input is msb aligned (24 bit down to 14bits required by U-law)
   // (combined shift: -10 for linear scaling -2 for algorithm)
   r1 = r0 ASHIFT (12 - DAWTH);

   if POS jump if_input_gte_0;
      r1 = r1 XOR -1;       // Invert data

      // Negative so reset bit 7
      r6 = 0;
   if_input_gte_0:

   // Calculate absno
   r1 = r1 + 33;

   // Make sure the value's not too large
   r2 = 0x1fff;
   Null = r1 - 0x2000;
   if POS r1 = r2; // (r2 = 0x1fff)

   r3 = 1;
   r2 = r1 ASHIFT -6;
   segloop:
      if Z jump segloop_break;
      r3 = r3 + 1;
      r2 = r2 ASHIFT -1;
      jump segloop;
   segloop_break:

   r4 = 8 - r3;

   r3 = -r3;
   r5 = r1 ASHIFT r3;
   r5 = r5 AND 0xf;

   r5 = 0xf - r5;

   // OR together the high and low nibbles
   r0 = r4 ASHIFT 4;
   r0 = r0 OR r5;
   // Now OR in the sign bit
   r0 = r0 OR r6;
   // Mask out any unused bits
   r0 = r0 AND 0xff;

   rts;

.ENDMODULE;


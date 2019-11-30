// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Interleave operator
//
// DESCRIPTION:
//    This routine interleaves multiple streams into a single interleaved multichannel audio buffer
//
// When using the operator the following data structure is used:
//    - $cbops.interleave.NUM_CHANNELS_FIELD = the number of audio output channels
// *****************************************************************************

#include "stack.h"
#include "cbops.h"
#include "cbops_interleave.h"

.MODULE $M.cbops.interleave;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.interleave[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.interleave.reset,                    // reset function
      &$cbops.interleave.amount_to_use,            // amount to use function
      &$cbops.interleave.main;                     // main function

.ENDMODULE;

// Expose the location of this table to C
.set $_cbops_interleave_table,  $cbops.interleave


// *****************************************************************************
// MODULE:
//    $cbops.interleave.reset
//
// DESCRIPTION:
//    Reset routine for the mix operator, see $cbops.interleave.main
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************

.MODULE $M.cbops.interleave.reset;
   .CODESEGMENT CBOPS_INTERLEAVE_RESET_PM;
   .DATASEGMENT DM;

   // ** reset routine **
   $cbops.interleave.reset:
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//   $cbops.interleave.amount_to_use
//
// DESCRIPTION:
// This function will limit the amount of data that will be interleaved.
//
// INPUTS:
//    - r5 = the minimum of the number of input samples available and the
//      amount of output space available
//    - r6 = the number of input samples available
//    - r7 = the amount of output space available
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - r5 = the number of samples to process
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.cbops.interleave.amount_to_use;
   .CODESEGMENT CBOPS_INTERLEAVE_AMOUNT_TO_USE_PM;
   .DATASEGMENT DM;

   // ** amount to use function **
   $cbops.interleave.amount_to_use:

   // if no read limit then ignore it
   r0 = M[r8 + $cbops.interleave.NUM_CHANNELS_FIELD];
   // set r0 = max amount of input data to use
   r1 = r6 * r0 (int);

   // set r5 to min(min(amount_in[0..n_ch-1])*n_ch, amount_out)
   r5 = r7;
   Null = r5 - r1;
   if POS r5 = r1;
   Div = r5 / r0;
   r5 = DivResult;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.interleave.main
//
// DESCRIPTION:
//    interleaves a multichannel stream
//
// INPUTS:
//    - r5 = pointer to the list of input and output buffer start addresses
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3
//
// *****************************************************************************
.MODULE $M.cbops.interleave.main;
   .CODESEGMENT CBOPS_INTERLEAVE_MAIN_PM;
   .DATASEGMENT DM;

   // ** main function **
   $cbops.interleave.main:

   r3 = M[r8 + $cbops.interleave.NUM_CHANNELS_FIELD];
   M0 = r3;
   M1 = 1;

   // get the interleaving (output) buffer length
   r1 = M[r7 + r3];
   // store the value in L0
   L0 = r1;
   #ifdef BASE_REGISTER_MODE
      // get the start address
      r1 = M[r5 + r3];
      push r1;
      pop B0;
   #endif

   // save the number of samples to process
   push r10;

   // interleave loop
   r2 = 0;
   interleave_channel:

      // get the output buffer write address
      r1 = M[r6 + r3];
      // update channel index and store the value in I0
      // I0 = B0 + (((r1 + r2) - B0) mod L0);
      I0 = r1;
      M2 = r2;
      r1 = M[I0, M2];

      // get the input buffer read address
      r1 = M[r6 + r2];
      // store the value in I4
      I4 = r1;
      // get the input buffer length
      r1 = M[r7 + r2];
      // store the value in L4
      L4 = r1;
      #ifdef BASE_REGISTER_MODE
         // get the start address
         r1 = M[r5 + r2];
         push r1;
         pop B4;
      #endif

      r10 = M[SP-1];
      r10 = r10 - 1;
      r0 = M[I4,M1];

      // copy loop
      do loop;
         r0 = M[I4,M1], M[I0,M0] = r0;
   loop:
      // write the last sample
      M[I0,M0] = r0;
      
   // update the offset to the read buffer to use
   r2 = r2 + 1;

   Null = M0 - r2;
   if NZ jump interleave_channel;

   // zero the length and base registers
   L0 = 0;
   L4 = 0;
   #ifdef BASE_REGISTER_MODE
      // Zero the base registers
      push Null;
      B4 = M[SP-1];
      pop B0;
   #endif

   pop r0;
   r0 = r0 * r3 (int);
   M[r4 + $cbops.fw.AMOUNT_WRITTEN_FIELD] = r0;
   
   rts;

.ENDMODULE;


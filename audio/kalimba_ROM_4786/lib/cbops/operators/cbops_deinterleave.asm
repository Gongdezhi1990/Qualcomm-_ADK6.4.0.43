// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Deinterleave operator
//
// DESCRIPTION:
//    This routine deinterleaves a multichannel audio buffer into separate contiguous audio buffers
//
// When using the operator the following data structure is used:
//    - $cbops.interleave.NUM_CHANNELS_FIELD = the number of audio output channels
// *****************************************************************************

#include "stack.h"
#include "cbops.h"
#include "cbops_interleave.h"

.MODULE $M.cbops.deinterleave;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.deinterleave[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.deinterleave.reset,                    // reset function
      &$cbops.deinterleave.amount_to_use,            // amount to use function
      &$cbops.deinterleave.main;                     // main function

.ENDMODULE;

// Expose the location of this table to C
.set $_cbops_deinterleave_table,  $cbops.deinterleave


// *****************************************************************************
// MODULE:
//    $cbops.deinterleave.reset
//
// DESCRIPTION:
//    Reset routine for the mix operator, see $cbops.deinterleave.main
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

.MODULE $M.cbops.deinterleave.reset;
   .CODESEGMENT CBOPS_DEINTERLEAVE_RESET_PM;
   .DATASEGMENT DM;

   // ** reset routine **
   $cbops.deinterleave.reset:
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//   $cbops.deinterleave.amount_to_use
//
// DESCRIPTION:
// This function will limit the amount of data that will be deinterleaved.
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
.MODULE $M.cbops.deinterleave.amount_to_use;
   .CODESEGMENT CBOPS_DEINTERLEAVE_AMOUNT_TO_USE_PM;
   .DATASEGMENT DM;

   // ** amount to use function **
   $cbops.deinterleave.amount_to_use:

   // if no read limit then ignore it
   r0 = M[r8 + $cbops.interleave.NUM_CHANNELS_FIELD];
   // set r0 = max amount of input data to use
   r5 = r7 * r0 (int);

   // set r5 to min(min(amount_out[0..n_ch-1])*n_ch, amount_in)
   // WARNING: make sure the available input data is a multiple of num_channels
   Null = r5 - r6;
   if POS r5 = r6;

   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.deinterleave.main
//
// DESCRIPTION:
//    Deinterleaves a multichannel stream 
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
//    r0, r1, r2
//
// *****************************************************************************
.MODULE $M.cbops.deinterleave.main;
   .CODESEGMENT CBOPS_DEINTERLEAVE_MAIN_PM;
   .DATASEGMENT DM;

   // ** main function **
   $cbops.deinterleave.main:

   r2 = M[r8 + $cbops.interleave.NUM_CHANNELS_FIELD];
   M0 = r2;
   M1 = 1;
   
   // get the input buffer length
   r1 = M[r7];
   // store the value in L0
   L0 = r1;
   #ifdef BASE_REGISTER_MODE
      // get the start address
      r1 = M[r5];
      push r1;
      pop B0;
   #endif

   // save the number of samples to process
   Div = r10 / r2;
   r2 = DivResult; 
   push r2;

   // deinterleave loop
   r2 = 0;
   deinterleave_channel:
   
      // get the input buffer read address
      r1 = M[r6];
      // update channel index and store the value in I0
      // I0 = B0 + (((r1 + r2) - B0) mod L0);
      I0 = r1;
      M2 = r2;
      r1 = M[I0, M2];

      // update the offset to the write buffer to use
      r2 = r2 + 1;
      // get the output buffer write address
      r1 = M[r6 + r2];
      // store the value in I4
      I4 = r1;
      // get the output buffer length
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
      r0 = M[I0,M0];
      
      // copy loop
      do loop;
         r0 = M[I0,M0], M[I4,M1] = r0;
   loop:
      // write the last sample
      M[I4,M1] = r0;

   Null = M0 - r2;
   if NZ jump deinterleave_channel;

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
   M[r4 + $cbops.fw.AMOUNT_WRITTEN_FIELD] = r0;
   
   rts;

.ENDMODULE;


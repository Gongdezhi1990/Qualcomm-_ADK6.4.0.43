// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Status check gain operator
//
// DESCRIPTION:
//    This operator will copy samples from a port, apply a gain to them and
// manage the connection and disconnection of the port. This operator fills the
// output buffer with silence when the input port is not available.  This is
// useful when attempting to shield a process from ports which are continuously
// connected and disconnected.
//
// For example, when reading from a port:
//
//    PORT_DISCONNECTED - Fill the available space in the buffer with silence
//
//    PORT_CONNECTED - Copy samples to write position in buffer
//
// When using the operator the following data structure is used:
//    - $cbops.status_check_gain.INPUT_START_INDEX_FIELD = index of the
//       input, must be a port
//    - $cbops.status_check_gain.OUTPUT_START_INDEX_FIELD = index of the
//       output buffer
//    - $cbops.status_check_gain.GAIN_ADDRESS_FIELD = address of the gain
//       to be applied to each sample
//    - $cbops.status_check_gain.PORT_ID_FIELD = ID of the input port
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M_cbops.status_check_gain;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.status_check_gain[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,      // reset vector
      &$cbops.status_check_gain.amount_to_use, // amount to use function
      &$cbops.status_check_gain.main;          // main function

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $cbops.status_check_gain.amount_to_use
//
// DESCRIPTION:
//    Amount to use function for the status check gain operator.To ensure that
// the cbops framework still allows the main routine to be called it is
// necessary to return a non-zero value for the number of samples to process.
//
// INPUTS:
//    - r5 = the minimum of the number of input samples available and the
//      amount of output space available
//    - r6 = the number of input samples available
//    - r7 = the amount of output space available
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r4, r5
//
// *****************************************************************************
.MODULE $M.cbops.status_check_gain.amount_to_use;
   .CODESEGMENT CBOPS_STATUS_CHECK_GAIN_AMOUNT_TO_USE_PM;
   .DATASEGMENT DM;

   // ** reset function **
   $cbops.status_check_gain.amount_to_use:

   push rLink;
   // if the port is not connected, set the amount of data to the amount of
   // space in the destination cbuffer
   r0 = M[r8 + $cbops.status_check_gain.PORT_ADDRESS_FIELD];
   call $cbuffer.is_it_enabled;
   if Z r5 = r7;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.status_check_gain.main
//
// DESCRIPTION:
//    Main function for the status check gain operator.  This operator fills the
// output buffer with silence when the input port is not available.
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, I0, I4, L0, L4
//
// *****************************************************************************
.MODULE $M.cbops.status_check_gain.main;
   .CODESEGMENT CBOPS_STATUS_CHECK_GAIN_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.status_check_gain.main:

   // push rLink onto stack
   push rLink;


   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.status_check_gain.INPUT_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the write buffer to use
   r0 = M[r8 + $cbops.status_check_gain.OUTPUT_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;


   // check the status of the port
   r0 = M[r8 + $cbops.status_check_gain.PORT_ADDRESS_FIELD];
   call $cbuffer.is_it_enabled;
   // If the port is enabled we can copy samples into our buffer
   // otherwise jump to padding with silence
   if Z jump pad_with_silence;
      r10 = r10 - 1;
      // get the address of the gain we are applying
      r1 = M[r8 + $cbops.status_check_gain.GAIN_ADDRESS_FIELD];
      // get the first sample
      r0 = M[I0,1];
      // get the actual gain value
      r1 = M[r1];
      // generate the first output sample
      r0 = r0 * r1 (frac);
      do copy_samples_loop;
         r0 = M[I0,1], M[I4,1] = r0;
         // Apply the gain
         r0 = r0 * r1 (frac);
      copy_samples_loop:
      // do the last write
      M[I4,1] = r0;

   jump done;

   pad_with_silence:

      // write zeros into the cbuffer instead
      r0 = 0;
      do pad_silence_loop;
         // Write silence to the output
         M[I4,1] = r0;
      pad_silence_loop:

   done:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


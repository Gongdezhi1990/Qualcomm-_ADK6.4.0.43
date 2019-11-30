// *****************************************************************************
// Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    User filter operator
//
// DESCRIPTION:
//    This operator transfers signal with a shift from a source buffer into a
//    output buffer just as the shift operator does, and then applies an
//    externally user defined filter to the signal on the output buffer.
//
//    The data structure used in this operator is basically the same as 'shift'
//    operator, except an extra field of pointer's pointer to the function of
//    the user filter. As initialization, user should set this field with a
//    USER POINTER, which itself would point to the actual filter function.
//
//    The user filter could be activated or deactivated. To activate, explicitly
//    sets the USER POINTER with the address of the actual filter function.
//    To deactivate, clears the USER POINTER.
//
//    When the user filter is not activated, this operator functions exactly
//    like a 'shift' operator.
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.user_filter;
   .DATASEGMENT DM;

   // ** operator function vector **
   .VAR $cbops.user_filter[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,      // reset function
      $cbops.function_vector.NO_FUNCTION,      // amount to use function
      &$cbops.user_filter.main;                // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.user_filter.main
//
// DESCRIPTION:
//    Operator that shifts data in the input buffer into output buffer, and
//    applies an externally user defined filter function to the output signal.
//
//    The interface of the user filter:
//       - r0 = input buffer address
//       - r1 = input buffer length
//       - r2 = output buffer address
//       - r3 = output buffer length
//       - r5 = number of samples to process
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
//    - same as $cbops.shift, and those trashed by the user filter function
//
// NOTE:
//    - user filter should be implemented allowing in-place processing
// *****************************************************************************
.MODULE $M_cbops.user_filter.main;
   .CODESEGMENT CBOPS_USER_FILTER_MAIN_PM;
   .DATASEGMENT DM;

$cbops.user_filter.main:

   push rLink;

   // save number of samples
   r5 = r10;

   // transfer data from input buffer into output buffer
   call $cbops.shift.main;

   // load user filter function pointer into r4
   // and exits if no filter is activated
   r0 = M[r8 + $cbops.user_filter.FUNCTION_PTR_PTR];
   r4 = M[r0];
   if Z jump $pop_rLink_and_rts;

   // get the offset to the data buffer for filtering
   r1 = M[r8 + $cbops.shift.OUTPUT_START_INDEX_FIELD];
   // get the filter input buffer address
   r0 = M[r6 + r1];
   // get the filter input buffer length
   r1 = M[r7 + r1];
   // filter output buffer address/length, in-place processing
   r2 = r0;
   r3 = r1;

   // execute user filter function
   call r4;

   jump $pop_rLink_and_rts;

.ENDMODULE;

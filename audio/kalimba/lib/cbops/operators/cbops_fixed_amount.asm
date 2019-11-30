// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Limited copy operator
//
// DESCRIPTION:
//    The CBOPS library copies the minimum of the amount of data in the input
// buffers and the amount of space in the output buffers. However this may not
// always be desireable. This sets the amount to use to be the value in the
// $cbops.fixed_amount.AMOUNT_FIELD field or zero if there isn't this much space
// or data
//
// NOTE:
//    To ensure that this operator is useful it should be at the end of the
// chain of operators.
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.fixed_amount;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.fixed_amount[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      &$cbops.fixed_amount.amount_to_use,   // amount to use function
      $cbops.function_vector.NO_FUNCTION;   // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//   $cbops.fixed_amount.amount_to_use
//
// DESCRIPTION:
//    Operator to limit the amount of data copied in each call, NOTE it does not
// actually copy any data, it simply limits the amount of data other operators
// will copy.
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
.MODULE $M.cbops.fixed_amount.amount_to_use;
   .CODESEGMENT CBOPS_FIXED_AMOUNT_AMOUNT_TO_USE_PM;
   .DATASEGMENT DM;


   // ** amount to use function **
   $cbops.fixed_amount.amount_to_use:

   // if no amount then exit
   r0 = M[r8 + $cbops.fixed_amount.AMOUNT_FIELD];
   Null = r0 - $cbops.fixed_amount.NO_AMOUNT;
   if Z rts;

   // if r5 >= amount then set r5 to amount
   Null = r5 - r0;
   if LT jump small_r5;
   r5 = r0;
   rts;

   small_r5:
   // r5 < amount so zero amount to use
   r5 = 0;
   rts;

.ENDMODULE;


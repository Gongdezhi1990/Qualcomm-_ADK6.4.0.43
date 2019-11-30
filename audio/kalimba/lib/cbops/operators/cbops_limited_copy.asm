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
// always be desireable. For example, when reading data from a SCO buffer, the
// firmware will start to create new samples if the buffer is emptied. So it is
// desirable to limit the amount copied in a single pass.
//
//    The limited copy operator is only called during the amount to use
// calculation. It is NOT an operator as such, it does no processing in the
// main processing loop.
//
// When using the operator the following data structure is used:
//    - $cbops.limited_copy.READ_DATA_LIMIT_FIELD = set the limit or to
//       $cbops.limited_copy.NO_READ_LIMIT_SET for no limit.
//    - $cbops.limited_copy.WRITE_SPACE_LIMIT_FIELD = set the limit or to
//       $cbops.limited_copy.NO_WRITE_LIMIT_SET for no limit.
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.limited_copy;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.limited_copy[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      &$cbops.limited_copy.amount_to_use,   // amount to use function
      $cbops.function_vector.NO_FUNCTION;   // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//   $cbops.limited_copy.amount_to_use
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
.MODULE $M.cbops.limited_copy.amount_to_use;
   .CODESEGMENT CBOPS_LIMITED_COPY_AMOUNT_TO_USE_PM;
   .DATASEGMENT DM;


   // ** amount to use function **
   $cbops.limited_copy.amount_to_use:

   // if no read limit then ignore it
   r0 = M[r8 + $cbops.limited_copy.READ_LIMIT_FIELD];
   Null = r0 - $cbops.limited_copy.NO_READ_LIMIT;
   if Z jump no_read_limit;
      // set r0 = max amount of input data to use
      r0 = r6 - r0;
      if NEG r0 = 0;
      // limit r5
      Null = r5 - r0;
      if POS r5 = r0;
   no_read_limit:

   // if no write limit then ignore it
   r0 = M[r8 + $cbops.limited_copy.WRITE_LIMIT_FIELD];
   Null = r0 - $cbops.limited_copy.NO_WRITE_LIMIT;
   if Z jump no_write_limit;
      // set r0 = max amount of output space to use
      r0 = r7 - r0;
      if NEG r0 = 0;
      // limit r5
      Null = r5 - r0;
      if POS r5 = r0;
   no_write_limit:
   rts;

.ENDMODULE;


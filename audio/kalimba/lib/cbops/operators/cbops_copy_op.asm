// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    copy operator
//
// DESCRIPTION:
//    This operator does a straight forward copy of samples from the input
// buffer to the output buffer.
//
// When using the operator the following data structure is used:
//    - $cbops.copy_op.INPUT_START_INDEX_FIELD = the index of the input
//       buffer
//    - $cbops.copy_op.OUTPUT_START_INDEX_FIELD = the index of the output
//       buffer
//
// *****************************************************************************

#include "cbops.h"
#include "stack.h"

.MODULE $M.cbops.copy_op;
   .DATASEGMENT DM;

   // ** function vector ** - recommendation is to standardise presence of a create(), too
   .VAR $cbops.copy_op[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.basic_multichan_amount_to_use,   // amount to use function
      &$cbops.copy_op.main;                 // main function

.ENDMODULE;

// Expose the location of this table to C
.set $_cbops_copy_table,  $cbops.copy_op

// *****************************************************************************
// MODULE:
//    $cbops.copy_op.main
//
// DESCRIPTION:
//    Operator that copies the input sample to the output (multi-channel version)
//
// INPUTS:
//    - r4 = buffer table
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-2, r10, I0, L0, I4, L4, DoLoop
//
// *****************************************************************************
.MODULE $M.cbops.copy_op.main;
   .CODESEGMENT CBOPS_COPY_OP_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.copy_op.main:

   push rLink;
   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_copy_op[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_copy_op;
      call $profiler.start;
   #endif

   call $cbops.get_transfer_and_update_multi_channel;
   r6 = r0 - 1;
   if NEG jump jp_done;
   
   // Channel counter, r9=num_channels
   r3 = 0;

   // r6 = amount-1, r9=num_chans in addresses, r3=chan_num in addresses

 process_channel:
   // get the input index for current channel
   r5 = r3 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r5];     // input index
   r5 = r5 + r9;
   r5 = M[r8 + r5];     // output index
   
   // Check for Inplace
   NULL = r0 - r5;
   if Z jump next_channel;
   
   // Setup Input Buffer
   call $cbops.get_buffer_address_and_length;
   I0 = r0;
   if Z jump next_channel;
   L0 = r1;
   push r2;
   pop B0;

   // Setup Output Buffer
   r0 = r5;
   call $cbops.get_buffer_address_and_length;
   I4 = r0;
   if Z jump next_channel;
   L4 = r1;
   push r2;
   pop B4;

   // get pre-decremented amount to use;
   // for speed pipeline the: read -> shift -> write
   r10 = r6;
   r0 = M[I0, MK1];
   do loop;
      r0 = M[I0, MK1], M[I4, MK1] = r0;
   loop:
   // write the last sample
   M[I4, MK1] = r0;

 next_channel:

   // we move to next channel. In the case of this cbop, it is enough to 
   // count based on input channels here.
   r3 = r3 + 1*ADDR_PER_WORD;
   Null = r3 - r9;
   if LT jump process_channel;

   // zero the length registers
   L4 = 0;
   L0 = 0;
   // Zero the base registers
   push Null;
   B4 = M[SP - 1*ADDR_PER_WORD];
   pop B0;

jp_done:
   // restore registers
   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_copy_op;
      call $profiler.stop;
   #endif
   pop rLink;
   rts;

.ENDMODULE;


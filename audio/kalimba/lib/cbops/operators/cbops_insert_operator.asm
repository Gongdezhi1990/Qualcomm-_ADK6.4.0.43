// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Buffer Insert operator
//
// DESCRIPTION:
//    This operator keeps an cbops chain fed.
//
//    If the input is below the threshold, then insert data
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"
#include "cbuffer_asm.h"
#include "cbops_insert_op_asm_defs.h"

// Private Library Exports
.PUBLIC $cbops.insert_op;

.MODULE $M.cbops.insert_op;
   .CODESEGMENT CBOPS_INSERT_OPERATOR_PM;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.insert_op[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,      // reset function
      &$cbops.insert_op.amount_to_use,         // amount to use function
      &$cbops.insert_op.main;                  // main function

// Expose the location of this table to C
.set $_cbops_insert_table , $cbops.insert_op

// *****************************************************************************
// MODULE:
//   $cbops.insert_op.amount_to_use
//
// DESCRIPTION:
//   Get the amount to use, across all channels (it acts "in sync").
//
// INPUTS:
//    - r4 = buffer table
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    r5 - amount to use
//
// TRASHED REGISTERS:
//    r0, r5, r6
//
// *****************************************************************************
$cbops.insert_op.amount_to_use:
    push rlink;
    // get number of input channels - transform it into addresses, so that we don't do the latter
    // for every channel. But let's keep calling it subsequently the "number of channels"
    r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
    Words2Addr(r9);

    // Get space at output 
    r0 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
    r0 = M[r8 + r0];                                             // Output index (first channel)
    call $cbops.get_amount_ptr;
    r5 = M[r0];

    // Get data at input
    r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];     // input index (first channel)
    call $cbops.get_amount_ptr;
    r7 = M[r0];

    // Save data at input
    r1 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
    M[r1 + $cbops_insert_op.insert_op_struct.AMOUNT_DATA_FIELD] = r7;

    // Set input to amount to process
    M[r0]=r5;

    pop rlink;
    rts;

// *****************************************************************************
// MODULE:
//    $cbops.insert_op.main
//
// DESCRIPTION:
//    Restore Transfer amount and insert data if needed. 
//    It acts "in sync" across all channels
//
// INPUTS:
//    - r4 = pointer to internal framework object
//    - r8 = pointer to operator structure:
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-3, r5, r6, r10, rMAC, I0, I4, L0, L4, DoLoop
//
// *****************************************************************************
   // define some humane names for FP offsets
   .CONST $cbops.insert_op.TEMP_AMOUNT_OFFSET              2*ADDR_PER_WORD;
   .CONST $cbops.insert_op.TEMP_AMT_TO_INSERT              3*ADDR_PER_WORD;
   .CONST $cbops.insert_op.NUM_TEMP_VARS                   2*ADDR_PER_WORD;

$cbops.insert_op.main:
   pushm <FP(=SP), rlink>;
   SP = SP + $cbops.insert_op.NUM_TEMP_VARS;

   // Get amounter of data to consume, and update produced
   call $cbops.get_transfer_and_update_multi_channel;
   // r9 = num_channels
   r10 = r0;
   if LE jump jp_done;

    // Get Data Pointer
    r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

    // Get amount of data and threshold
    r2 = M[r7 + $cbops_insert_op.insert_op_struct.THRESHOLD_FIELD];
    r3 = M[r7 + $cbops_insert_op.insert_op_struct.AMOUNT_DATA_FIELD];

    // Amount to produce vs amount at input
    r6 = r10;
    NULL = r10 - r3;
    if LE jump update_amounts;
       // Insufficient data, limit transfer to threshold
       r6 = MIN r2;
       // Check for insertion even with reduced transfer
       r10 = r6;
       r10 = MIN r3;

       // Save amount consumed (r10)
       r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];     // input index (first channel) 
       call $cbops.get_amount_ptr;
       M[r0]=r10;

       // Save amount produced (r6)
       r0 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
       r0 = M[r8 + r0];     // Output index (first channel)
       call $cbops.get_amount_ptr;
       M[r0]=r6;
 update_amounts:

    // r10 is amount consumed, r6 is amount produced
    // Remember amount to insert
    r6 = r6 - r10;
    M[FP + $cbops.insert_op.TEMP_AMT_TO_INSERT] = r6;    // amount to insert
    M[FP + $cbops.insert_op.TEMP_AMOUNT_OFFSET] = r10;   // amount to copy

   // Channel counter
   r3 = 0;

 process_channel:
   // get the input index for current channel
   r5 = r3 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;

   // Setup Input Buffer
   r0 = M[r8 + r5];     // input index
   call $cbops.get_buffer_address_and_length;
   I0 = r0;
   if Z jump next_channel;
   L0 = r1;
   push r2;
   pop B0;
   
   // Setup Output Buffer
   r0 = r5 + r9;
   r0 = M[r8 + r0];     // output index
   call $cbops.get_buffer_address_and_length;
   I4 = r0;
   if Z jump next_channel;
   L4 = r1;
   push r2;
   pop B4;

    // Copy available input to output r10 times
    r10 = M[FP + $cbops.insert_op.TEMP_AMOUNT_OFFSET];
    r0 = M[I0, MK1];
    do lp_copy;
        r0 = M[I0, MK1],   M[I4, MK1]=r0;
    lp_copy:

    // Skip Insert?
    r10 = M[FP + $cbops.insert_op.TEMP_AMT_TO_INSERT];
    if LE jump next_channel;
       // Repeat last available sample for insertion, r10 times
       r2 = M[r7 + $cbops_insert_op.insert_op_struct.NUM_INSERTS_FIELD];
       r2 = r2 + r10;
       M[r7 + $cbops_insert_op.insert_op_struct.NUM_INSERTS_FIELD] = r2;

       r0 = NULL;
       do lp_insert;
           M[I4, MK1]=r0;
       lp_insert:

 next_channel:
    // update channel counter
    r3 = r3 + 1*ADDR_PER_WORD;
    Null = r3 - r9;
    if LT jump process_channel;

    // Reset Buffering control
    L0 = 0;
    L4 = 0;
    push NULL;
    B4 = M[SP - 1*ADDR_PER_WORD];
    pop B0;

jp_done:
    SP = SP - $cbops.insert_op.NUM_TEMP_VARS;
    popm <FP, rlink>;
    rts;

// unsigned get_cbops_insert_op_inserts(cbops_op);
$_get_cbops_insert_op_inserts:
    r0 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
    r0 = M[r0 + $cbops_insert_op.insert_op_struct.NUM_INSERTS_FIELD];
    rts;

.ENDMODULE;







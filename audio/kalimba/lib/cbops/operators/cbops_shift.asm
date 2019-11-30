// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Shift operator
//
// DESCRIPTION:
//  This operator shifts the data by the supplied amount, using the arithmetic
//  shift operator (ASHIFT).
//
//  Note as Kalimba typically reads data from periphals as 16 bit words using
//  the ASHIFT operator can generate unexpected results.
//
//  If a positive number is shifted and the result becomes negative, the ASHIFT
//  operator will saturate the result. If external data is read by Kalimba as
//  16 bit data and zero padded to 24 bit, the values will all be interpreted as
//  positive numbers, even if they were negative 16 bit numbers.
//
//  Therefore if data read from peripherals is to be shifted to 24 bit data it
//  is recommended you use the $cbuffer.FORCE_SIGN_EXTEND flag to force Kalimba
//  to sign extend the data as it reads it.
//
// When using the multichannel operator the following data structure is used:
//    - header:
//              nr inputs
//              nr outputs (equal in this case)
//              <nr inputs> indexes for input channels (some may be marked as unused)
//              <nr outputs> indexes for output channels (some may be marked as unused)
//    - $cbops.shift.SHIFT_AMOUNT_FIELD = amount to shift input value by
//       (eg. 8, -8). This is a zero-based index value into parameters that follow
//      after the header part of the param struct.
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.shift;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.shift[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.basic_multichan_amount_to_use,   // amount to use function
      &$cbops.shift.main;                   // main function

.ENDMODULE;

// Expose the location of this table to C.
// Recommendation would be to standardise create() and possibly some configure() functionss in this table.
.set $_cbops_shift_table,  $cbops.shift

// *****************************************************************************
// MODULE:
//    $cbops.shift.main
//
// DESCRIPTION:
//    Operator that shifts the input word (multi-channel version)
//
// INPUTS:
//    - r4 = pointer to list of buffers
//    - r8 = pointer to cbops object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-2, r10, I0, I4, L0, L4, DoLoop
//
// *****************************************************************************
.MODULE $M.cbops.shift.main;
   .CODESEGMENT CBOPS_SHIFT_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.shift.main:

   // get the parameter (shift amount) if zero do simple compy op
   r0 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
   r7 = M[r0 + $cbops.shift.SHIFT_AMOUNT_FIELD];
   if Z jump $cbops.copy_op.main;

   push rlink;
   call $cbops.get_transfer_and_update_multi_channel;
   r6 = r0 - 1;
   if NEG jump jp_done;
   // r6 = amount-1, r9=num_chans in addresses, r3=chan_num in addresses
   

   // start counting channels (in the case of this cbop, in and out channels move in tandem)
   // Again, to save tedium per channel, this will be kept and updated with 1*ADDR_PER_WORD,
   // and compared with the "number of channels" (above also converted to addresses)
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

   // for speed pipeline the: read -> shift -> write
   // Grab the pre-decremented amount to process value for all channels
   r10 = r6;
   r0 = M[I0, MK1];
   r0 = r0 ASHIFT r7;
   do loop;
      r0 = M[I0, MK1],  M[I4, MK1] = r0;
      r0 = r0 ASHIFT r7;
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
   L0 = 0;
   L4 = 0;
   // Zero the base registers
   push Null;
   B4 = M[SP - 1*ADDR_PER_WORD];
   pop B0;

jp_done:
   pop rlink;
   rts;

.ENDMODULE;



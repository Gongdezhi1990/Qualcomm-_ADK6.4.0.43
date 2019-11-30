// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// *****************************************************************************

#include "core_library.h"
#include "cbops.h"
#include "log_linear_cbops.h"

.MODULE $M.cbops.copy_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.g711[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.basic_multichan_amount_to_use,   // amount to use function
      &$cbops.g711.main;                    // main function

.ENDMODULE;

// Expose the location of this table to C
.set $_cbops_g711_table,  $cbops.g711

// *****************************************************************************
// MODULE:
//    $cbops.g711.main
//
// DESCRIPTION:
//    Operator that applies one of the g711 functions
//
// INPUTS:
//    - r4 Buffer table
//    - r8 = pointer to operator structure
//
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume all except r8
//
// *****************************************************************************
.MODULE $M.cbops.g711.main;
   .CODESEGMENT PM_FLASH;

   $cbops.g711.main:

   push rLink;

   call $cbops.get_transfer_and_update_multi_channel;
   r10 = r0 - 1;
   if NEG jump $pop_rLink_and_rts;
   
   // Channel count, r9=num channels
	
   r3 = 0;
   
   // get the g711 function pointer - it is channel-independent parameter,
   // after the header part of the cbop param struct. We exploit the fact that
   // number of in and out channels in this cbop's case is the same.
   r0 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];   
   r7 = M[r0 + $cbops.g711.G711_FUNC_PTR_FIELD];

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

   // salvage what the conversion functions trash.
   // An area of temp vars could also be used for this, saving a popm effort,
   // but adding other tedium (albeit prior to channel-dependent loop).
   pushm <r3,r4,r10>;


   // read the first sample
   r0 = M[I0, MK1];
   // Apply the g711 function
   call r7;

   do loop;
      M[I4, MK1] = r0,r0 = M[I0, MK1];
      call r7;
   loop:

   // write the last sample
    M[I4, MK1] = r0;

   // restore stuff that conversion functions messed with
   popm <r3, r4, r10>;

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

   jump $pop_rLink_and_rts;

.ENDMODULE;

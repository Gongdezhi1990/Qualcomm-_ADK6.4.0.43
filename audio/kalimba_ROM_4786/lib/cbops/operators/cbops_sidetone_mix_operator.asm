// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Frame Sync Sidetone mix operator
//
// DESCRIPTION:
//    In a headset type application it is often desirable to feed some of the
// received microphone signal back out on the speaker. This gives the user some
// indication the headset is on. This operator mixes data from a supplied buffer
// in with the "main" data stream.
//
// @verbatim
//      Input data       +         Data to output
//      ---------------------->O---------------------->
//                             ^
//                             |
//                             O x g
//                             |
//                             |
//                    Received sidetone data
// @endverbatim
//
//
//    The number of samples in the sidetone buffer is monitored at this point.
// If there are too few, extra samples are created by repeating the last sample
// from the buffer. If too many samples are present, samples will be
// discarded during each call.
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"
#include "cbuffer_asm.h"
#include "cbops_sidetone_mix_op_asm_defs.h"


// Private Library Exports
.PUBLIC $cbops.sidetone_mix_op;

.MODULE $M.cbops.sidetone_mix_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.sidetone_mix_op[$cbops.function_vector.STRUC_SIZE] =
      // reset function
      $cbops.function_vector.NO_FUNCTION,
      // amount to use function
      &$cbops.basic_multichan_amount_to_use,
      // main function
      &$cbops.sidetone_mix_op.main;
.ENDMODULE;

// two entries only in the index table - it only ever has one in and one out channel
.CONST $cbops.sidetone_mix_op.NR_INDEX_ENTRIES			2*ADDR_PER_WORD;

.MODULE $M.cbops.sidetone_mix_op;
   .CODESEGMENT CBOPS_SIDETONE_MIX_OPERATOR_PM;
   .DATASEGMENT DM;

// *****************************************************************************
// MODULE:
//    $cbops.sidetone_mix_op.main
//
// DESCRIPTION:
//    Operator that copies the output/input word and reads/writes the samples
//    to/from the sidetone buffer
//
// INPUTS:
//    - r4 = pointer to list of buffers
//    - r8 = pointer to cbops object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, r5, r6, r10, rMAC, M1, I0, I1, I4, L0, L1, L4, DoLoop
//
// *****************************************************************************
$cbops.sidetone_mix_op.main:
   // push rLink onto stack
   push rLink;

   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // Get amount of data in sidetone buffer (r6)
   r0 = M[r7 +  $cbops_sidetone_mix_op.st_mix_struct.ST_IN_IDX_FIELD];
   call $cbops.get_amount_ptr;
   I3 = r0; 
   r6  = M[r0];

   // Get transfer amount (r10)
   call $cbops.get_transfer_and_update_multi_channel;
   r10 = r0;
   if LE jump jp_dump_sidetone;

   // Setup Input Buffer
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];     // input index
   call $cbops.get_buffer_address_and_length;
   I0 = r0;
   if Z jump jp_dump_sidetone;
   L0 = r1;
   push r2;
   pop B0;

   // Setup Output Buffer
   r0 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r0];                                            // output index
   call $cbops.get_buffer_address_and_length;
   I4 = r0;
   if Z jump jp_dump_sidetone;
   L4 = r1;
   push r2;
   pop B4;

   r0 = M[r7 +  $cbops_sidetone_mix_op.st_mix_struct.ST_IN_IDX_FIELD];
   call $cbops.get_buffer_address_and_length;
   push r2;
   pop B1;
   // set the length & get the first DAC sample
   M1 = MK1;
   I1 = r0;
   L1 = r1, rMAC = M[I0,M1];

   // Check if we need to insert samples (later)
   // Limit transfer to available data
   r3 = r10 - r6;
   if NEG r3 = Null;

   // r10 is the amount of data to use
   // r6  is the amount of data in the sidetone buffer
   // r3  is number of samples to insert into sidetone
   // I1 = SideTone
   // I0 = Input
   // I4 = Output

   // Sidetone attenuation (0dB if not specified)
   r5 = 1.0;

   // Save amount in sidetone buffer
   r1 = r6;

   // Amount to blend
   r10 = r10 - r3;
   r6  = r6 - r10;

   do write_loop;
      // calculate the current output sample and read a side tone sample
      r0 = M[I1,MK1];
      rMAC = rMAC + r0 * r5;
      // get the next DAC value and write the result
      rMAC = M[I0,M1], M[I4,M1] = rMAC;
write_loop:

   r0 = M[r7 + $cbops_sidetone_mix_op.st_mix_struct.NUM_INSERTS_FIELD];
   r0 = r0 + r3;
   M[r7 +  $cbops_sidetone_mix_op.st_mix_struct.NUM_INSERTS_FIELD]=r0;

   // Repeat Last sample of sidetone buffer (insert sidetone samples)
   r0 = M[I1,-MK1];
   r10 = r3, r0 = M[I1,M1];
   // Ensure sidetone is connected repeat last sample, else mix silence
   NULL = r1;
   if Z r0=NULL;
   do write_loop2;
      // calculate the current output sample
      rMAC = rMAC + r0 * r5;
      // get the next DAC value and write the result
      rMAC = M[I0,M1], M[I4,M1] = rMAC;
   write_loop2:

   // zero the remaining length registers we have used
   L0 = 0;
   L4 = 0;
   push Null;
   B0 = M[SP - 1*ADDR_PER_WORD];
   pop B4;

jp_dump_sidetone:
   // r6 = amount of sidetone data remaining (after transfer)
   // I3 = pointer to sidetone amount

   // If amount of sidetone after transfer is > 2*SIDETONE_MAX_SAMPLES_FIELD
   // drop sidetone to leave SIDETONE_MAX_SAMPLES_FIELD samples
   // r3 is amount to drop
   r1 = M[r7 + $cbops_sidetone_mix_op.st_mix_struct.MAX_SAMPLES_FIELD];
   r3 = r6 - r1;            // remaining data - SIDETONE_MAX_SAMPLES_FIELD
   NULL = r3 - r1;
   if NEG r3 = NULL;        // remaining data < 2*SIDETONE_MAX_SAMPLES_FIELD (no drop)

   r2 = M[r7 + $cbops_sidetone_mix_op.st_mix_struct.NUM_DROPS_FIELD];
   r2 = r2 + r3;
   M[r7 + $cbops_sidetone_mix_op.st_mix_struct.NUM_DROPS_FIELD]=r2;

   // Set side-tone consumption
   r5 = M[I3,0];
   r5 = r5 - r6;     // Amount consumed
   r5 = r5 + r3;     // Amount dropped
   M[I3,0]=r5;

   // pop rLink from stack
   pop rLink;
   rts;

.ENDMODULE;

// Expose the location of this table to C
.set $_cbops_sidetone_mix_table,  $cbops.sidetone_mix_op





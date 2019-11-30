// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// ASM function for base multi-channel operator data processing
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)
#include "cbuffer_asm.h"
#include "portability_macros.h"
#include "base_multi_chan_op_asm_defs.h"


.MODULE $M.base_multi_chan_asm;
   .CODESEGMENT BASE_MULTI_CHAN_PM;
// *****************************************************************************
// MODULE:
//    $_multi_channel_copy_mute
// void multi_channel_copy_mute(MULTI_CHANNEL_CHANNEL_STRUC *first_stream, unsigned samples_to_process,unsigned bMute);
//
// DESCRIPTION:
//    Data processing function.
//
// INPUTS:
//    - r0 = Pointer to first channel
//    - r1 = number of samples to process
//    - r2 = Mute Flag
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
$_multi_channel_copy_mute:
   pushm <I0, I4>;
   pushm <M1,L0, L4>;
   pushm <B0, B4>;
   pushm <r4, r5, r6, rLink>;

   // Validate can copy or mute
   r6 = r0;
   if Z jump multi_channel_copy_mute_done;
   r4 = r1 - 1;
   if NEG jump multi_channel_copy_mute_done;

   // Copy or mute
   r5 = multi_channel_copy;
   r1 = multi_channel_mute;
   NULL = r2;
   if NZ r5 = r1;

   M1=MK1;
   // Loop channels
   multi_channel_copy_mute_next:
      // Get input buffer
      r0 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];
      call $cbuffer.get_read_address_and_size_and_start_address;
      I0 = r0;
      push r2;
      pop B0;
      L0 = r1;
      // Get output buffer
      r0 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];
      call $cbuffer.get_write_address_and_size_and_start_address;
      I4 = r0;
      push r2;
      pop B4;
      L4 = r1;
      // Perform operation
      call r5;
      // Update input buffer
      r0 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];
      r1 = I0;
      call $cbuffer.set_read_address;
      // Update output buffer
      r0 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];
      r1 = I4;
      call $cbuffer.set_write_address;
      // Next channel
      r6 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.NEXT_ACTIVE_FIELD];
      if NZ jump multi_channel_copy_mute_next;

multi_channel_copy_mute_done:
   popm <r4, r5, r6, rLink>;
   popm <B0, B4>;
   popm <M1,L0, L4>;
   popm <I0, I4>;
   rts;


multi_channel_copy:
   r10 = r4,   r0=M[I0,M1];
   do multi_channel_copy_lp;
      M[I4,M1] = r0; r0 = M[I0,M1];
   multi_channel_copy_lp:
   M[I4,M1] = r0;
   rts;

multi_channel_mute:
   r10 = r4,   r0=M[I0,M1];
   r1 = NULL;
   do multi_channel_mute_lp;
      M[I4,M1] = r1; r0 = M[I0,M1];
   multi_channel_mute_lp:
   M[I4,M1] = r1;
   rts;

// *****************************************************************************
// MODULE:
//    $_multi_channel_advance_buffers
// void multi_channel_advance_buffers(MULTI_CHANNEL_CHANNEL_STRUC *first_stream, unsigned consumed,unsigned produced);
//
// DESCRIPTION:
//    Data processing function.
//
// INPUTS:
//    - r0 = Pointer to first channel
//    - r1 = number of samples to consumed
//    - r2 = number of samples to produced
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
$_multi_channel_advance_buffers:
   pushm <M2,M3, L0>;
   pushm <r6, rLink>;
   push B0;
   push I0;

   // Validate can copy or mute
   r6 = r0;
   if Z jump multi_channel_advance_buffers_done;
   r1 = r1 * MK1 (int);
   if LE jump multi_channel_advance_buffers_done;

   // Advance in addresses
   M3 = r1;
   r2 = r2 * MK1 (int);
   M2 = r2;

   // Loop channels
   multi_channel_advance_buffers_next:
      // Get input buffer
      r0 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];
      call $cbuffer.get_read_address_and_size_and_start_address;
      I0 = r0;
      NULL = r1 - MK1;
      if Z jump multi_channel_advance_buffers_radv;
         push r2;
         pop B0;
         L0 = r1;
         // Dummy read to advance pointer
         r0 = M[I0,M3];
      multi_channel_advance_buffers_radv:
      // Update input buffer
      r0 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.SINK_BUFFER_PTR_FIELD];
      r1 = I0;
      call $cbuffer.set_read_address;

      // Get output buffer
      r0 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];
      call $cbuffer.get_write_address_and_size_and_start_address;
      I0 = r0;
      NULL = r1 - MK1;
      if Z jump multi_channel_advance_buffers_wadv;
         push r2;
         pop B0;
         L0 = r1;
         // Dummy read to advance pointer
         r0 = M[I0,M2];
      multi_channel_advance_buffers_wadv:
      // Update output buffer
      r0 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.SOURCE_BUFFER_PTR_FIELD];
      r1 = I0;
      call $cbuffer.set_write_address;
      // Next channel
      r6 = M[r6 + $base_multi_chan_op.multi_chan_channel_struc_struct.NEXT_ACTIVE_FIELD];
      if NZ jump multi_channel_advance_buffers_next;
multi_channel_advance_buffers_done:

   pop I0;
   pop B0;
   popm <r6, rLink>;
   popm <M2,M3, L0>;
   rts;

// *****************************************************************************
// MODULE:
//    $_multi_channel_get_channel_count
//unsigned multi_channel_get_channel_count(OPERATOR_DATA *op_data);
//
// DESCRIPTION:
//    Data processing function.
//
// INPUTS:
//    - r0 = Pointer to operator data
//
// OUTPUTS:
//    - r0 - number of active channels
//
// TRASHED REGISTERS:
//    C compliant
//
// *****************************************************************************
$_multi_channel_get_channel_count:
   r0 = M[r0 + $base_multi_chan_op.OPERATOR_DATA_struct.CAP_CLASS_EXT_FIELD];
   r0 = M[r0 + $base_multi_chan_op.multi_channel_def_struct.ACTIVE_CHAN_MASK_FIELD];
   r0 = ONEBITCOUNT r0;
   rts;

.ENDMODULE;

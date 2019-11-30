// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Underrun compensation. It must be at the very start of the chain.
//       Inputs must not be ports.  Buffer read pointer is advanced to
//       discard data
//
// DESCRIPTION:
//
// When using the multichannel operator the following data structure is used:
//    - header:
//          - nr inputs of the section at the start of which it is used (!)
//          - nr outputs that it operates on (for consistency / future proofing,
//                        should match nr. of outputs of the section)
//          - <nr inputs> indexes for input channels (some may be marked as unused)
//          - <nr outputs> indexes for output channels
//                         (their values don't matter in this case,
//                          operator only concerns itself with the input buffers)
// *****************************************************************************

#include "cbops.h"
#include "cbops_discard_op_asm_defs.h"

.MODULE $M.cbops.discard_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.discard[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,  // reset function
      &$cbops.discard_op.amount_to_use,       // amount to use function
      &$cbops.discard_op.main;                // main function

.ENDMODULE;

// Expose the location of this table to C.
// Recommendation would be to standardise create() and possibly some configure() functionss in this table.
.set $_cbops_discard_table,  $cbops.discard


// *******************************************************************************
// MODULE:
//    $cbops.discard_op.amount_to_use
//
// DESCRIPTION:
//    Amount to use function. Most of this used to be located at endpoint level,
//    where it performed the entire tedium several times, i.e. every channel.
//
// INPUTS:
//    - r4 = pointer to list of buffers
//    - r8 = pointer to cbops object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-3, r10
//
// *******************************************************************************
.MODULE $M.cbops.discard_op.amount_to_use;
   .CODESEGMENT PM;

   // ** amount to use function **
$cbops.discard_op.amount_to_use:

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.discard_op.amount_to_use.PATCH_ID_0, r1)
#endif

   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
   // has data started across channels? if not, nothing to do here.
   // A few vital pointers are not forgiven for being NULL at this point - something very
   // nasty happened if not initialised still. These pointers are used to communicate via the
   // endpoint level between the underrun cbop at the end of the chain and this cbop at the start
   // of the chain. Endpoint by now had to set these up.
   r1 = M[r7 + $cbops_discard_op.discard_struct.SYNC_START_PTR_FIELD];
   if Z call $error;

   Null = M[r1];
   if Z rts;

   // threshold = data_block estimation + sink block_size + RM headroom amount
   r3 = M[r7 + $cbops_discard_op.discard_struct.RM_HEADROOM_FIELD];
   r1 = M[r7 + $cbops_discard_op.discard_struct.DATA_BLOCK_SIZE_PTR_FIELD];
   if Z call $error;

   r1 = M[r1];
   r3 = r3 + r1;

   r1 = M[r7 + $cbops_discard_op.discard_struct.BLOCK_SIZE_PTR_FIELD];
   if Z call $error;

   r1 = M[r1];
   r3 = r3 + r1;

   // zero the discard amount
   M[r7 + $cbops_discard_op.discard_struct.DISCARD_AMOUNT_FIELD] = Null;

   // if we have insertions to compensate for, and we are above input amount threshold,
   // then calculate discard amount

   // first see if input amount > threshold
   push rLink;
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];
   call $cbops.get_amount_ptr;
   pop rLink;
   r6 = M[r0];
   r3 = r6 - r3;
   if LE rts;

   // any so far accumulated insertions to reduce?
   r1 = M[r7 + $cbops_discard_op.discard_struct.TOTAL_INSERTS_PTR_FIELD];
   if Z call $error;

   r1 = M[r1];
   if Z rts;

   // discard_amount = MIN(total inserts so far, amount_data - threshold)
   Null = r1 - r3;

   if GE jump done;
   r3 = r1;

 done:
   M[r7 + $cbops_discard_op.discard_struct.DISCARD_AMOUNT_FIELD] = r3;

#ifdef CBOPS_DEBUG_LOGGING
   // log the discard amount when latter is not zero
   pushm <r0, r1>;
   r0 = M[$cbops.debug_logidx];
   Null = r0 - ($cbops.DEBUG_LOG_SIZE - 2);
   if GT jump nolog;
   r1 = 0xDFDFDF;
   M[r0 + $cbops.debug_log] = r1;
   r0 = r0 + 1;
   M[r0 + $cbops.debug_log] = r3;
   r0 = r0 + 1;
   M[$cbops.debug_logidx] = r0;
   nolog:
   popm <r0, r1>;
#endif

   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.discard_op.main
//
// DESCRIPTION:
//    Operator that compensates for underrun by inserting samples, in sync across
//    all of its channels.
//
// INPUTS:
//    - r4 = pointer to list of buffers
//    - r8 = pointer to cbops object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r9, I0, L, B0
//
// *****************************************************************************
.MODULE $M.cbops.discard_op.main;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $cbops.discard_op.main:

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.discard_op.main.PATCH_ID_0, r1)
#endif

   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_discard[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_discard;
      call $profiler.start;
   #endif
   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];


   // has data started across channels? if not, nothing to do here.
   // Vital pointers are not (re-)checked here, as amount_to_use is executed
   // before each time we end up here. It should have caught anything nasty with still NULL ptrs in
   // the cbop param struct. If in-between something corrupted things, well...
   r0 = M[r7 + $cbops_discard_op.discard_struct.SYNC_START_PTR_FIELD];
   Null = M[r0];
   if Z rts;

   // do the discarding for each channel with same amount (if amount is non-zero)
   r3 = M[r7 + $cbops_discard_op.discard_struct.DISCARD_AMOUNT_FIELD];
   if Z rts;

   // Save registers
   push rlink;

   // Get amount data at input
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];
   call $cbops.get_amount_ptr;
   r1 = M[r0];

   // limit the amount to discard to amount available
   // this should already be limited, just for peace of mind
   r3 = MIN r1;

   // set the new amount left in buffer(s) for next operator to process
   r1 = r1 - r3;
   M[r0] = r1;

#ifdef CBOPS_DEBUG_LOGGING
   // log the discard amount (again) at this point of processing it
   pushm <r0, r1, r3>;
   r0 = M[$cbops.debug_logidx];
   Null = r0 - ($cbops.DEBUG_LOG_SIZE - 2);
   if GT jump nolog;
   r1 = 0xCFCFCF;
   M[r0 + $cbops.debug_log] = r1;
   r0 = r0 + 1;
   M[r0 + $cbops.debug_log] = r3;
   r0 = r0 + 1;
   M[$cbops.debug_logidx] = r0;
   nolog:
   popm <r0, r1, r3>;
#endif

   // the input pointer modify amount is the discard amount
   Words2Addr(r3);   // discard amount (arch4: in addrs)
   M3 = r3;

   // Number of channels
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   // Buffer index array
   I1 = r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;

 process_channel:

   // get the input index for current channel
   r0 = M[I1,MK1];
   call $cbops.advance_buffer_address;
   // we move to next input channel
   r9 = r9 - 1;
   if GT jump process_channel;

   // zero length and base
   L0 = 0;
   push NULL;
   pop B0;

   // Restore registers
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_discard;
      call $profiler.stop;
   #endif

   pop rLink;
   rts;

.ENDMODULE;


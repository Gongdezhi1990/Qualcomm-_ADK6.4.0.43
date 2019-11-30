// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Sink Overflow disgard.  It must be the last operator referencing the sink
//
// DESCRIPTION:
//
// When using the multichannel operator the following data structure is used:
//    - header:
//          - nr inputs of the section at the end of which it is used (!)
//          - nr outputs that it operates on (== normally nr outs of section)
//          - <nr inputs> indexes for input channels (their values don't matter in this case,
//                        operator only concerns itself with the output buffers)
//          - <nr outputs> indexes for output channels (some may be marked as unused)
// *****************************************************************************

#include "cbops.h"
#include "cbops_sink_overflow_disgard_op_asm_defs.h"


.MODULE $M.cbops.sink_overflow_disgard_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.sink_overflow_disgard_op[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,           // reset function
      &$cbops.sink_overflow_disgard_op.amount_to_use,   // amount to use function
      $cbops.function_vector.NO_FUNCTION;            // main function

.ENDMODULE;

// Expose the location of this table to C.
// Recommendation would be to standardise create() and possibly some configure() functionss in this table.
.set $_cbops_sink_overflow_disgard_table,  $cbops.sink_overflow_disgard_op



// *****************************************************************************
// MODULE:
//    $_get_sink_overflow_disgard_drops
//    unsigned get_sink_overflow_disgard_drops(cbops_op);
//
// DESCRIPTION:
//    Get number of samples operator dropped
//
// INPUTS:
//    - r0 = pointer to operator structure
//
// OUTPUTS:
//    - r0 - number of drops
//
// TRASHED REGISTERS:
//    r0, r1
//
// *****************************************************************************
.MODULE $M.cbops.sink_overflow_disgard_op.drops;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   // ** reset routine **
$_get_sink_overflow_disgard_drops:
   // here we got pointer to the cbops structure, so need to get to the
   // parameter struct first.
   r0 = M[r0 + ($cbops_c.cbops_op_struct.PARAMETER_AREA_START_FIELD+$cbops.param_hdr.OPERATOR_DATA_PTR_FIELD)];
   r0 = M[r0 + $cbops_sink_overflow_disgard_op.sink_overflow_disgard_struct.NUM_DROPS_FIELD];
   rts;

.ENDMODULE;


// *******************************************************************************
// MODULE:
//    $cbops.sink_overflow_disgard_op.amount_to_use
//
// DESCRIPTION:
//    Amount to use function. Most of this used to be located at endpoint level,
//    where it performed the entire tedium several times, i.e. every channel.
//    "Down here" at cbop level one has direct visibility of data amounts and,
//    in case of a sync group, it can do this once for the whole chorus of channels.
//    One major quirk is that if an amount verdict is zero, that dictates it for the
//    chain, and main() funcs of cbops will not get called.
//    In such case, considering extra special role of this cbop, one needs to get to
//    its main() so that insertions can happen even if everything else in chain is
//    whining about total data starvation!
//
// INPUTS:
//    - r4 = pointer to list of buffers
//    - r8 = pointer to cbops object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-3, r9, r10
//
// *******************************************************************************
.MODULE $M.cbops.sink_overflow_disgard_op.amount_to_use;
   .CODESEGMENT PM;

   // ** amount to use function **
$cbops.sink_overflow_disgard_op.amount_to_use:

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.sink_overflow_disgard_op.amount_to_use.PATCH_ID_0, r7)
#endif

   push rlink;

   r7 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // Get space at sink
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];
   call $cbops.get_amount_ptr;
   r10 = M[r0];

   // Is there sufficient space
   r2 = M[r7 + $cbops_sink_overflow_disgard_op.sink_overflow_disgard_struct.MIN_SPACE_FIELD];
   r3 = r10 - r2;
   if POS jump jp_done;

   // Reset Space to minimum
   M[r0]=r2;

   r0 = M[r7 + $cbops_sink_overflow_disgard_op.sink_overflow_disgard_struct.NUM_DROPS_FIELD];
   r0 = r0 - r3;
   M[r7 + $cbops_sink_overflow_disgard_op.sink_overflow_disgard_struct.NUM_DROPS_FIELD]=r0;

   Words2Addr(r3);   // discard amount (arch4: in addrs)
   M3 = r3;

   // number of channels 
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   // Buffer index array
   I1 = r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
process_channel:
      // Advance buffer
      r0 = M[I1,MK1];
      call $cbops.advance_buffer_address;
      // we move to next channel. 
      r9 = r9 - 1;
      if GT jump process_channel;

   // Reset cicular buffer registers
   L0 = 0;
   push NULL;
   pop B0;

jp_done:

   pop rlink;
   rts;


.ENDMODULE;



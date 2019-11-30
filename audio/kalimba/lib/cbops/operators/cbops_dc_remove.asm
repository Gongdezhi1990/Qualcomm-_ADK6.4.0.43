// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    DC remove operator
//
// DESCRIPTION:
//    It is useful to remove any dc component from the signal going to the DAC
// (as this is bad for speakers) and also from the ADC (as this might affect
// the operation of the compression codec used).
//
// The dc component is estimated as follows:
// @verbatim
//    dc_est = old_dc_est * (1 - 1/n) + current_sample * (1/n)
//
// where: old_dc_est is initialised to 0
//        1/n = $cbuffer.dc_remove.FILTER_COEF;
// @endverbatim
//
// The dc is removed from the sample as follows:
// @verbatim
//    sample = sample - dc_est
// @endverbatim
//
//
// When using the multichannel operator the following data structure is used:
//    - header:
//              nr inputs
//              nr outputs (equal in this case)
//              <nr inputs> indexes for input channels (some may be marked as unused)
//              <nr outputs> indexes for output channels (some may be marked as unused)
//    - DC estimate values for each channel.
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.dc_remove;
   .DATASEGMENT DM;

   // ** function vector for multi-channel cbop **
   // Recommendation is to standardise this table to include create() and such
   .VAR $cbops.dc_remove[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.dc_remove.reset,        // reset function
      $cbops.basic_multichan_amount_to_use,   // amount to use function
      &$cbops.dc_remove.main;               // main function

.ENDMODULE;

// Expose the location of this table to C
.set $_cbops_dc_remove_table, $cbops.dc_remove


// *****************************************************************************
// MODULE:
//    $cbops.dc_remove.reset
//
// DESCRIPTION:
//    Reset routine for the DC remove operator multi-channel version, 
//    see $cbops.dc_remove.main
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    DoLoop
//
// *****************************************************************************
.MODULE $M.cbops.dc_remove.reset;
   .CODESEGMENT CBOPS_DC_REMOVE_RESET_PM;
   .DATASEGMENT DM;

   // ** reset routine **
   $cbops.dc_remove.reset:


   // get number of input channels and start with first channel
   r10 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];  

   r0 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];  
   I0 = r0;
   r0 = NULL;

   do reset_channel;
      M[I0,MK1] = r0;
   reset_channel:

   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $cbops.dc_remove.main
//
// DESCRIPTION:
//    Operator that removes any DC component from the input data (multi-channel version)
//
// INPUTS:
//    - r4 = buffer table
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    rMAC, r0-4, r10, I0, L0, I4, L4, DoLoop
//
// *****************************************************************************
.MODULE $M.cbops.dc_remove.main;
   .CODESEGMENT CBOPS_DC_REMOVE_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.dc_remove.main:
   
   push rLink;
   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_dc_remove[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_dc_remove;
      call $profiler.start;
   #endif

   call $cbops.get_transfer_and_update_multi_channel;
   M3 = r0 - 1;
   if NEG jump jp_done;

   // Offset to filter coefficients
   r6 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
   // needed for saturation and used as -1.0 below
   r3 = MININT;
   
   // channel counter, r9=num channels
   r7 = Null;

   // M3 = amount-1, r9=num_chans in addresses, r7=chan_num in addresses
 process_channel:
   // get the input index for current channel
   r5 = r7 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;

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

   // Get the current dc estimate for the current channel.
   // In this case the params are right after the header part, and we don't really need to
   // use the $cbops.dc_remove.DC_ESTIMATE_FIELD. In general, this would have been
   // retrieved by adding this constant, too to r0 if there were multiple params per
   // each channel.
   rMAC = M[r6 + r7];

   // r2 = 1.0 - (1/n)
   // Note: fix point -1.0 represents +1.0 so long as you subtract something from it
   // i.e. this works as long as $cbops.dc_remove.FILTER_COEF is not 0
   r1 = $cbops.dc_remove.FILTER_COEF;
   r2 = r3 - r1;

   // for speed pipeline the: read -> update -> write
   // Grab the pre-decremented amount to process value for all channels
   r10 = M3;

   // new_dc_est = old_dc_est * (1 - 1/n) + current_sample * (1/n)
   // and read the first sample
   rMAC = rMAC * r2, r0 = M[I0, MK1];

   rMAC = rMAC + r0 * r1;
   r0 = r0 - rMAC;
   // saturate if overflow has occured
   if V r0 = r0 * r3 (int) (sat);
   do loop;
      rMAC = rMAC * r2, r0 = M[I0, MK1], M[I4, MK1] = r0;
      rMAC = rMAC + r0 * r1;
      r0 = r0 - rMAC;
      if V r0 = r0 * r3 (int) (sat);
   loop:

   // write the last sample
   M[I4, MK1] = r0;

   // store updated dc estimate for next time. We pushed the offset for the
   // current channel's parameter so we don't calculate it again here.
   M[r6 + r7] = rMAC;

 next_channel:

   // we move to next channel. In the case of this cbop, it is enough to
   // count based on input channels here.
   // both current and total channel number is in ADD_PER_WORDs so just compare them
   r7 = r7 + 1*ADDR_PER_WORD;
   Null = r7 - r9;
   if LT jump process_channel;

   // zero the length registers
   L0 = 0;
   L4 = 0;
   // Zero the base registers
   push Null;
   B4 = M[SP - 1*ADDR_PER_WORD];
   pop B0;

jp_done:

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_dc_remove;
      call $profiler.stop;
   #endif

   pop rlink;
   rts;


.ENDMODULE;

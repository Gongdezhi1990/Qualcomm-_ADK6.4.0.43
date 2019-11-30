   // *****************************************************************************
// Copyright (c) 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    cbops mute operator
//
// DESCRIPTION:
//    cbops mute operator can be used to mute/unmute multi-channel audio. This
//    operator always works in-place. This has the benefit that it won't
//    add any computational complexity when in un-mute mode (but it makes it
//    unsuitable to work on ports-like buffers). The mute flag can be changed
//    any time on the fly, the content of buffer is linearly faded in transition
//    from mute to unmute as well as from unmute to mute.
//
// When using the mute operator the following data structure is used:
//    - header:
//              nr inputs
//              <nr inputs> indexes for input channels (some may be marked as unused)
//    - MUTE_ENABLE_FIELD: flag set by the user, any non-zero value will mean muting
//    - MUTE_STATE_FIELD:  current mute state (internal)
// *****************************************************************************

#include "stack.h"
#include "cbops.h"
#include "cbops_mute_c_asm_defs.h"

.MODULE $M.cbops.mute;
   .DATASEGMENT DM;

   // ** function vector for cbops_mute **
   .VAR $cbops.mute[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,        // reset function
      $cbops.function_vector.NO_FUNCTION,        // amount to use function
      &$cbops.mute.main;                         // main function
.ENDMODULE;

// Expose the location of this table to C
.set $_cbops_mute_table, $cbops.mute

// *****************************************************************************
// MODULE:
//    $cbops.mute.main
//
// DESCRIPTION:
//    cbops mute operator that can be used to mute/unmute multi-channel audio.
//
// INPUTS:
//    - r4 = buffer table
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    rMAC, r0-7, r9, r10, I0, L0, DoLoop, M3
//
// *****************************************************************************
.MODULE $M.cbops.mute.main;
   .CODESEGMENT CBOPS_MUTE_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.mute.main:

   push rLink;

   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_mute[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_mute;
      call $profiler.start;
   #endif

   /* get the params */
   r6 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   /* get the current state of mute */
   r2 = M[r6 +$cbops_mute_c.cbops_mute_struct.MUTE_STATE_FIELD];

   /* get the intended state of mute */
   r3 = M[r6 +$cbops_mute_c.cbops_mute_struct.MUTE_ENABLE_FIELD];

   /* store new state */
   M[r6 +$cbops_mute_c.cbops_mute_struct.MUTE_STATE_FIELD] = r3;

   /* no action if not in mute state and we don't
    * want to switch to mute state either
    */
   NULL = r2 OR r3;
   if Z jump end;

   // Get amount data at input (first channel)
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];
   call $cbops.get_amount_ptr;
   r10 = M[r0];

   /* if no data exit, there won't be
    * any fading in/out if this happens
    * right at transition time
    */
   if Z jump end;

   /* full mute if already in mute state and
    * we want to stay mute
    */
   r5 = 0;
   NULL = r2 XOR r3;
   if Z jump process_mute;

   /* in transition mode, calculate a ramp factor
    * based on the amount of data to process */
   rMAC = 1;
   rMAC = rMAC ASHIFT -1 (56bit);
   Div = rMAC / r10;
   r5 = DivResult;

   /* just if r10 was 1 */
   r5 = r5 - 1;

   /* see if it is transition from mute to unmute
    * in that case we will do fading in on whatever
    * we have.
    * Note: r3=0 will be used later for ramping gain
    *       start value.
    */
   Null = r3;
   if Z jump process_mute;
      /* Transition from unmute to mute,
       * Fading out on whatever we have
       * Ramping gain starts at 1.0 down to 0.
       */
      r5 = -r5;
      r3 = 1.0;
   process_mute:

   // number of channels
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   /* loop counter will be incremented by addrs */
   Words2Addr(r9);

   // channel counter
   r7 = 0;

   // save amount to process
   M3 = r10;

   process_channel:

      // get the input index for current channel
      r2 = r7 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;

      // Get Input Buffer
      r0 = M[r8 + r2];     // input index
      call $cbops.get_buffer_address_and_length;
      I0 = r0;
      if Z jump next_channel;
      L0 = r1;
      push r2;
      pop B0;

      // get the amount to process
      r10 = M3;

      // r5=0 means full mute, no fading.
      Null = r5;
      if Z jump do_full_mute;

      /* r3 shouldn't change so can be used for next channel */
      r1 = r3;

      /* Fading (in or out) will be done once
       * on the amount available
       */
      do mute_loop;
         r0 = M[I0, 0];                 // read next sample
         r0 = r0 * r1 (frac);           // apply fading gain
         r1 = r1 + r5, M[I0, MK1] = r0; // write and update fading gain
      mute_loop:

      jump next_channel;

      /* Normal mute process */
      do_full_mute:
      /* Note: r5 is already 0 at this point */
      do full_mute_loop;
         M[I0, MK1] = r5;
      full_mute_loop:

      next_channel:
      // move to next channel.
      r7 = r7 + ADDR_PER_WORD;
      Null = r7 - r9;
   if LT jump process_channel;

   // zero the length registers
   L0 = 0;

   // Zero the base registers
   push Null;
   pop B0;

   end:

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_mute;
      call $profiler.stop;
   #endif

   pop rlink;
   rts;

.ENDMODULE;

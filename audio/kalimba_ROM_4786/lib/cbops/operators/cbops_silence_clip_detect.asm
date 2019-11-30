// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Silence And Clip Detect Operator
//
// DESCRIPTION:
//    This operator monitors the sample values in the input buffer and compares
// it to the lower (silence) and upper (clip) thresholds.  If the signal clips
// or is silent for the specified duration a message is sent to the VM.
//
//    This operator does not copy any samples.  Hence, it does not require an
// output buffer.  It just analyses the contents of the input buffer.
//
//    In most applications each instance will start with its parameter area
// virtually empty. The only elements populated will be the
// INPUT_START_INDEX_FIELD and the INSTANCE_NO_FIELD (see below). The remaining
// fields will be populated by a message from the VM with levels and/or
// durations.
//
// When using the operator the following data structure is used:
//    - $cbops.silence_clip_detect.INPUT_START_INDEX_FIELD = the input
//       buffer that is being analysed
//    - $cbops.silence_clip_detect.INSTANCE_NO_FIELD = The instance number
//       of this detector operator.  Up to 8 instances can be used (one
//       for each channel of audio)
//    - $cbops.silence_clip_detect.SILENCE_LIMIT_FIELD = the threshold value
//       used for the silence detector.  If the sample values are below this
//       threshold the time counter is started.  If the sample values exceed
//       this threshold the time counter is reset. Initialise to zero.
//    - $cbops.silence_clip_detect.CLIP_LIMIT_FIELD = the threshold value
//       used for the clip detector.  If the signal exceeds this value a
//       message is sent to the VM. Initialise to zero.
//    - $cbops.silence_clip_detect.SILENCE_PERIOD_LSW_FIELD = The LSW of the
//       current duration of silence that has been detected. Initialise to
//       zero.
//    - $cbops.silence_clip_detect.SILENCE_PERIOD_MSW_FIELD = The MSW of the
//       current duration of silence that has been detected. Initialise to zero.
//    - $cbops.silence_clip_detect.PREVIOUS_TIME_FIELD = the time this
//       operator was previously called. Initialise to zero.
//    - $cbops.silence_clip_detect.SILENCE_AMOUNT_LSW_FIELD = The LSW of the
//       silence duration after which a message is sent to the VM. Initialise to
//       zero.
//    - $cbops.silence_clip_detect.SILENCE_AMOUNT_MSW_FIELD = The MSW of the
//       silence duration after which a message is sent to the VM. Initialise to
//       zero.
//
// *****************************************************************************

#include "stack.h"
#include "message.h"
#include "kalimba_standard_messages.h"
#include "cbops.h"

.MODULE $M.cbops.silence_clip_detect;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.silence_clip_detect[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.silence_clip_detect.reset,    // reset vector
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.silence_clip_detect.main;     // main function

   // ** allocate memory for the look-up table **
   .VAR $cbops.silence_clip_detect.lookup[$cbops.silence_clip_detect.LOOK_UP_SIZE];

   // ** allocate memory for the message handler **
   .VAR $cbops.silence_clip_detect.message_struc[$message.STRUC_SIZE];

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.silence_clip_detect.reset
//
// DESCRIPTION:
//    Reset routine for the silence and clipping detect operator, see
//    $cbops.silence_clip_detect.main
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.cbops.silence_clip_detect.reset;
   .CODESEGMENT CBOPS_SILENCE_CLIP_DETECT_RESET_PM;
   .DATASEGMENT DM;

   // ** reset function **
   $cbops.silence_clip_detect.reset:

   // we need to reset the silence counter and store the current time
   r0 = M[$TIMER_TIME];
   M[r8 + $cbops.silence_clip_detect.PREVIOUS_TIME_FIELD] = r0;

   // check if we should reset the timers
   Null = M[r8 + $cbops.silence_clip_detect.SILENCE_PERIOD_MSW_FIELD];
   if NEG rts;

   // reset the silence timers
   r0 = M[r8 + $cbops.silence_clip_detect.SILENCE_AMOUNT_MSW_FIELD];
   M[r8 + $cbops.silence_clip_detect.SILENCE_PERIOD_MSW_FIELD] = r0;
   r0 = M[r8 + $cbops.silence_clip_detect.SILENCE_AMOUNT_LSW_FIELD];
   M[r8 + $cbops.silence_clip_detect.SILENCE_PERIOD_LSW_FIELD] = r0;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.silence_clip_detect.main
//
// DESCRIPTION:
//    Operator that monitors stream for silence or possible clipping
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0, r1, r2, r3, r4, r5, r10, DoLoop, I0, L0
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.cbops.silence_clip_detect.main;
   .CODESEGMENT CBOPS_SILENCE_CLIP_DETECT_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.silence_clip_detect.main:

   // push rLink onto stack
   push rLink;

   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_silence_clip_detect[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_silence_clip_detect;
      call $profiler.start;
   #endif

   #ifdef CBOPS_DEBUG
      // check we have been initialised
      r0 = M[r8 + $cbops.silence_clip_detect.INSTANCE_NO_FIELD];
      Null = r0 - &$cbops.silence_clip_detect.look_up_size;
      if NEG call error;
      r0 = M[$cbops.silence_clip_detect.lookup + r0];
      Null = r8 - r0;
      if NZ call error;
   #endif

   // get the offset to the read buffer to use
   r2 = M[r8 + $cbops.warp_and_shift.INPUT_START_INDEX_FIELD];

   // get the input buffer read address
   r3 = M[r6 + r2];
   // store the value in I0
   I0 = r3;
   // get the input buffer length
   r4 = M[r7 + r2];
   // store the value in L0
   L0 = r4;

   // read in state fields
   r2 = M[r8 + $cbops.silence_clip_detect.SILENCE_LIMIT_FIELD];
   r4 = M[r8 + $cbops.silence_clip_detect.CLIP_LIMIT_FIELD];
   // store the number of samples, used to check all were silent
   r5 = r10;
   r1 = r10;

   do loop;
      // get the first sample
      r0 = M[I0,1];
      r0 = r0 * r0 (frac);
      // check for silence
      Null = r0 - r2;
      if POS jump clip_detect;
         r1 = r1 - 1;
      jump next_sample;

      clip_detect:
      // reset the silence count
      r1 = r5;

      // check for clipping
      Null = r0 - r4;
      if NEG jump next_sample;
         // should we send a message
         Null = r4 + 0;
         if LE jump next_sample;

         // clip detected, mark to send a message to VM
         r4 = -1;

      next_sample:
      nop;
   loop:

   // get the current time
   r0 = M[$TIMER_TIME];

   // store the instance number in case we send a message
   r3 = M[r8 + $cbops.silence_clip_detect.INSTANCE_NO_FIELD];

   M[r8 + $cbops.silence_clip_detect.CLIP_LIMIT_FIELD] = r4;
   if POS jump dont_send_clip_message;
      r2 = $MESSAGE_CBOPS_SILENCE_CLIP_DETECT_CLIP_DETECTED_ID;
      call $message.send;
      M[r8 + $cbops.silence_clip_detect.CLIP_LIMIT_FIELD] = Null;
      jump reset_timers;
   dont_send_clip_message:

   // did we find silence
   Null = r1;
   if POS jump reset_timers;
      // silence found should we send a message
      r4 = M[r8 + $cbops.silence_clip_detect.SILENCE_AMOUNT_MSW_FIELD];
      if NEG jump done;

      // we are in silence mode, how long have we been silenced
      r1 = M[r8 + $cbops.silence_clip_detect.SILENCE_PERIOD_LSW_FIELD];

      // get the previous call time
      r2 = M[r8 + $cbops.silence_clip_detect.PREVIOUS_TIME_FIELD];

      // store the current call time
      M[r8 + $cbops.silence_clip_detect.PREVIOUS_TIME_FIELD] = r0;

      // how long is the interval between calls
      r0 = r0 - r2;

      // set r2 in case we send a message
      r2 = $MESSAGE_CBOPS_SILENCE_CLIP_DETECT_SILENCE_DETECTED_ID;

      // subtract the interval from the running total
      r1 = r1 - r0;

      // do we need to decrement the MSW
      r4 = r4 - Borrow;

      // update the counters
      M[r8 + $cbops.silence_clip_detect.SILENCE_PERIOD_LSW_FIELD] = r1;
      M[r8 + $cbops.silence_clip_detect.SILENCE_PERIOD_MSW_FIELD] = r4;
      if NEG call $message.send;


   jump done;

   reset_timers:
      // store the current call time
      M[r8 + $cbops.silence_clip_detect.PREVIOUS_TIME_FIELD] = r0;
      // check if we should reset the timers
      Null = M[r8 + $cbops.silence_clip_detect.SILENCE_PERIOD_MSW_FIELD];
      if NEG jump done;
      // reset the silence timers
      r0 = M[r8 + $cbops.silence_clip_detect.SILENCE_AMOUNT_MSW_FIELD];
      M[r8 + $cbops.silence_clip_detect.SILENCE_PERIOD_MSW_FIELD] = r0;
      r0 = M[r8 + $cbops.silence_clip_detect.SILENCE_AMOUNT_LSW_FIELD];
      M[r8 + $cbops.silence_clip_detect.SILENCE_PERIOD_LSW_FIELD] = r0;
   done:

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_silence_clip_detect;
      call $profiler.stop;
   #endif

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.silence_clip_detect.initialise
//
// DESCRIPTION:
//    Initialisation routine for the silence and clipping detect operator.
// During its first call the routine initialises a message handler to receive
// all messages from the VM to start/stop monitoring.
//
//    NOTE - this routine must be called once for each instance to be used -
// ie if this is to be used on a stereo stream one instance will be required for
// each channel. Each instance must be assigned a different instance number and
// registered with the initialise routine seperately.
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0 - r4
//
// *****************************************************************************
.MODULE $M.cbops.silence_clip_detect.initialise;
   .CODESEGMENT CBOPS_SILENCE_CLIP_DETECT_INITIALISE_PM;
   .DATASEGMENT DM;

   $cbops.silence_clip_detect.initialise:

   // push rLink onto stack
   push rLink;

   // Load the instance number from the parameter area
   r0 = M[r8 + $cbops.silence_clip_detect.INSTANCE_NO_FIELD];
   // register this entry in the table at the index of the instance number
   M[$cbops.silence_clip_detect.lookup + r0] = r8;

   // check if the handler has already been registered
   Null = M[$cbops.silence_clip_detect.message_struc + $message.ID_FIELD];
   if NZ jump handler_initialised;

      // register the message handler
      r1 = &$cbops.silence_clip_detect.message_struc;
      r2 = $MESSAGE_CBOPS_SILENCE_CLIP_DETECT_INITIALISE_ID;
      r3 = &$cbops.silence_clip_detect.message_handler;
      call $message.register_handler;

   handler_initialised:

   // set the MSW to -1 to stop the operator flagging silence
   r0 = -1;
   M[r8 + $cbops.silence_clip_detect.SILENCE_AMOUNT_MSW_FIELD] = r0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;




// *****************************************************************************
// MODULE:
//    $cbops.silence_clip_detect.message_handler
//
// DESCRIPTION:
//    The message handler for messages from the VM to set the silence period
//    for the silence detector and to set the clip threshold sample value.
//
// INPUTS:
//    - r1 = instance number
//    - r2 = silence level (a signed 16 bit number represeting -1.0 -> +1.0)
//           negative values will be intepreted as a message to switch the
//           detection off
//    - r3 = duration of silence in seconds
//    - r4 = clipping level (a signed 16 bit number represeting -1.0 -> +1.0)
//           negative values will be intepreted as a message to switch the
//           detection off.
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r8
//
// *****************************************************************************
.MODULE $M.cbops.silence_clip_detect.message_handler;
   .CODESEGMENT CBOPS_SILENCE_CLIP_DETECT_MESSAGE_HANDLER_PM;
   .DATASEGMENT DM;

   $cbops.silence_clip_detect.message_handler:

   // get the structure address
   r8 = M[$cbops.silence_clip_detect.lookup + r1];

   // set r1 in case we need to switch silence detection off
   r1 = -1;


   // silence data - convert the limit to a 24 bit number
   r2 = r2 LSHIFT 8;
   if NEG jump no_silence_data;
      if Z jump do_not_set_silence_period;
         // set the silence period
         r1 = r3 LSHIFT -4;
         r3 = r3 LSHIFT 20;
      do_not_set_silence_period:

      M[r8 + $cbops.silence_clip_detect.SILENCE_LIMIT_FIELD     ] = r2;
      M[r8 + $cbops.silence_clip_detect.SILENCE_AMOUNT_MSW_FIELD] = r1;
      M[r8 + $cbops.silence_clip_detect.SILENCE_AMOUNT_LSW_FIELD] = r3;

   no_silence_data:

   // clipping data - convert the limit to a 24 bit number
   r4 = r4 LSHIFT 8;
   if NEG jump no_clipping_data;
      M[r8 + $cbops.silence_clip_detect.CLIP_LIMIT_FIELD] = r4;
   no_clipping_data:

   rts;


.ENDMODULE;


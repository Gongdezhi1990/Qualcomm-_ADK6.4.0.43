// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// $****************************************************************************
// NAME:
//    Audio Processing Library Delay Utility
//
// DESCRIPTION:
//    Utilities to delay an audio stream by a specified length in samples.
//
// MODULES:
//    - $audio_proc.delay.initialize
//    - $audio_proc.delay.process
// *****************************************************************************

#ifndef AUDIO_DELAY_INCLUDED
#define AUDIO_DELAY_INCLUDED

#include "core_library.h"
#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "cbuffer.h"
#endif
#include "delay.h"


// *****************************************************************************
// MODULE:
//    $audio_proc.delay.initialize
//
// DESCRIPTION:
//    Clear the delay buffer
//
// INPUTS:
//    - r8 = pointer to delay data object, with the following fields being set
//       - $audio_proc.delay.DBUFF_ADDR_FIELD
//       - $audio_proc.delay.DBUFF_SIZE_FIELD
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r10, I0, L0, LOOP
//
// NOTES:
// *****************************************************************************

.MODULE $M.audio_proc.delay.initialize;
   .CODESEGMENT AUDIO_PROC_DELAY_INITIALIZE_PM;
   .DATASEGMENT DM;

$audio_proc.delay.initialize:

   r0  = M[r8 + $audio_proc.delay.DBUFF_ADDR_FIELD];
   push rLink;
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   B0 = M[SP-MK1];
   pop  B0;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I0  = r0;
   L0  = r1;
   r10 = r1;
   pop rLink;
   // Clear delay buffer
   r0 = 0;
   do loop_delay_init;
      M[I0,MK1] = r0;
loop_delay_init:
   L0 = 0;
#ifdef BASE_REGISTER_MODE
   push Null;
   pop  B0;
#endif
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.delay.process
//
// DESCRIPTION:
//    Delaying input audio stream into output audio stream via delay buffer
//
// INPUTS:
//    - r8 = pointer to delay data object, with every field being set
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r10, I0-I2, I4-I5, L0-L1, L4-L5, M1-M2, LOOP
//
// NOTES:
//    - Delay buffer must be circular. Due to a known Kalimba chip bug, the
//      size of the buffer (DBUFF_SIZE_FIELD) must be at least 2.
//
//    - A '0' set in the 'DELAY_FIELD' means 'no delay'.
//
//    - Maximum legal delay is the size of the delay buffer (DBUFF_SIZE_FIELD).
// *****************************************************************************

.MODULE $M.audio_proc.delay.process;
   .CODESEGMENT AUDIO_PROC_DELAY_PROCESS_PM;
   .DATASEGMENT DM;

$audio_proc.delay.process:

   push rLink;
   //**** Get Input Buffers *****
   r0 = M[r8 + $audio_proc.delay.INPUT_ADDR_FIELD];
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B4;
#else
   call $frmbuffer.get_buffer;
#endif
   I4  = r0;
   L4  = r1;

   // Use input frame size
   r10 = r3;

// ***** Get output buffers ******
// Update output frame size from input
   r0 = M[r8 + $audio_proc.delay.OUTPUT_ADDR_FIELD];
   call $frmbuffer.set_frame_size;
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B5;
#else
   call $frmbuffer.get_buffer;
#endif
   I5 = r0;
   L5 = r1;

   // Get read pointer of delayed stream, point to input stream if no delay
   // Depending on DELAY_FIELD
   r0 = M[r8 + $audio_proc.delay.DELAY_FIELD];
   Words2Addr(r0);
   M2 = NULL - r0;
   if Z jump jp_no_delay;

   // Get write pointer to delay buffer (DBUFF_ADDR_FIELD / DBUFF_SIZE_FIELD)
   r0 = M[r8 + $audio_proc.delay.DBUFF_ADDR_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   B0 = M[SP-1];
   pop  B1;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   I1 = r0;
   L1 = r1;
   I0 = r0;
   L0 = r1;

   // Dummy read to set delay 
   r1 = M[I0,M2];

   // Delaying input stream into output stream via delay buffer
   do loop_delay;
      r0 = M[I4,MK1], r1 = M[I0,MK1];
      M[I1,MK1] = r0, M[I5,MK1] = r1;
loop_delay:

   // Update Delay Buffer
   r0 = M[r8 + $audio_proc.delay.DBUFF_ADDR_FIELD];
   r1 = I1;
   call $cbuffer.set_write_address;

jp_done:
   // clear circular buffer registers
   L0 = 0;
   L1 = 0;
   L4 = 0;
   L5 = 0;
#ifdef BASE_REGISTER_MODE
   push Null;
   B4 = M[SP-MK1];
   B5 = M[SP-MK1];
   B1 = M[SP-MK1];
   pop  B0;
#endif
  // pop rLink from stack
  jump $pop_rLink_and_rts;

jp_no_delay:
  // Copy input to output
   do loop_copy;
      r1 = M[I4,MK1];
      M[I5,MK1] = r1;
loop_copy:

  jump jp_done;


.ENDMODULE;

#endif // AUDIO_DELAY_INCLUDED

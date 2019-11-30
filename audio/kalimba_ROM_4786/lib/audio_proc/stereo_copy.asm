// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************


// *****************************************************************************
// MODULE:
//    $M.stream_copy
//
// DESCRIPTION:
//   Function to copy data from one buffer into another.
//
// INPUTS:
//   r7 -  data object containing pointers to input and output data.  Also
//         contains the number of samples to process.
//
// OUTPUTS:
//   - none
//
// CPU USAGE:
//   CODE memory:   27  words
//   DATA memory:    0  words (externally defined in data object)
// *****************************************************************************

#include "core_library.h"
#include "stereo_copy.h"

.MODULE $M.audio_proc.stereo_copy.Process;
 .codesegment AUDIO_PROC_STEREO_COPY_PROCESS_PM;

func:

   push rLink;

   // **** Get Input Buffers ****
   r0  = M[r7 + $M.audio_proc.stereo_copy.INPUT_CH1_PTR_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   I0  = r0;
   L0  = r1;

   r0  = M[r7 + $M.audio_proc.stereo_copy.INPUT_CH2_PTR_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B4;
#else
   call $frmbuffer.get_buffer;
#endif
   I4  = r0;
   L4  = r1;

   // **** Get output buffers ****
   // Use input frame size
   r10 = r3;
   // Update output frame size from input
   r0 = M[r7 + $M.audio_proc.stereo_copy.OUTPUT_CH1_PTR_BUFFER_FIELD];
   call $frmbuffer.set_frame_size;

#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B1;
#else
   call $frmbuffer.get_buffer;
#endif
   I1 = r0;
   L1 = r1;

   // Update output frame size from input
   r3 = r10;
   r0 = M[r7 + $M.audio_proc.stereo_copy.OUTPUT_CH2_PTR_BUFFER_FIELD];
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
   pop rLink;

   // **** Perform Copy Operation ****
   do loop_copy_data;
      r0 = M[I0,MK1], r2 = M[I4,MK1];
      M[I1,MK1] = r0, M[I5,MK1] = r2;
   loop_copy_data:

// clear circular buffer registers
   L0 = Null;
   L1 = Null;
   L4 = Null;
   L5 = Null;
#ifdef BASE_REGISTER_MODE
   push Null;
   B4 = M[SP-MK1];
   B5 = M[SP-MK1];
   B1 = M[SP-MK1];
   pop  B0;
#endif

   rts;
.ENDMODULE;

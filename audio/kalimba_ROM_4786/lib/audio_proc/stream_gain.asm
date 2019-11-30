// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// MODULE:
//    $M.audio_proc.stream_gain
//
//DESCRIPTION:
//    Used for copying data across stream buffers. This module was written to
//    facilitate copying of stream data
//
// INPUTS:
//    - r7 = Pointer to the stream copy data structure
//
// OUTPUT:
//    None
//
// TRASHED REGISTERS:
//      r0,r7, I0,I2,I4, M1, L0,L4
// *****************************************************************************

#include "core_library.h"
#include "stream_gain.h"
#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif
#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "cbuffer.h"
#endif

.MODULE $M.audio_proc.stream_gain.Process;

   .CODESEGMENT   AUDIO_PROC_STREAM_GAIN_PROCESS_PM;

func:

   push rLink;
   // Get Input Buffer
   r0  = M[r7 + $M.audio_proc.stream_gain.OFFSET_INPUT_PTR];

#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   I0  = r0;
   L0  = r1;

   // Use input frame size
   r10 = r3;
   // Update output frame size from input
   r0 = M[r7 + $M.audio_proc.stream_gain.OFFSET_OUTPUT_PTR];
   call $frmbuffer.set_frame_size;

   // Get output buffer
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B4;
#else
   call $frmbuffer.get_buffer;
#endif
   I4 = r0;
   L4 = r1;
   pop rLink;

   r1 = M[r7 + $M.audio_proc.stream_gain.OFFSET_PTR_MANTISSA];
   r2 = M[r7 + $M.audio_proc.stream_gain.OFFSET_PTR_EXPONENT];
   // dereference pointer (mantissa)
   r1 = M[r1];
   // dereference pointer (exponent),
   r2 = M[r2];
   // r0 = first value of i/p buffer
   r0 = M[I0,MK1];
   do lp_stream_copy;
      // rMAC = i/p * mantissa
      rMAC = r0 * r1;
      // r0 = o/p = rMAC * 2^Exponent
      r0 = rMAC ASHIFT r2;
      // Save o/p value, load next i/p value
      M[I4,MK1] = r0, r0 = M[I0,MK1];
lp_stream_copy:

   // clear L0/L4
   L0 = Null;
   L4 = Null;
#ifdef BASE_REGISTER_MODE
   push Null;
   B4 = M[SP-MK1];
   pop  B0;
#endif

   rts;
.ENDMODULE;

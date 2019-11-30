// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// MODULE:
//    $M.audio_proc.stream_mixer
//
// DESCRIPTION:
//    Used for mix data across stream buffers. This module was written to
//    facilitate mixing of stream data
//
// INPUTS:
//    - r7 = Pointer to the stream mixer data structure
//
// OUTPUT:
//    None
// *****************************************************************************

#include "core_library.h"
#include "stream_mixer.h"
#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif
#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "cbuffer.h"
#endif

.MODULE $M.audio_proc.stream_mixer.Process;

   .CODESEGMENT   AUDIO_PROC_STREAM_MIXER_PROCESS_PM;

func:
// Pointer to the stream copy data struc
   I2 = r7;    
   M1 = MK1;
   
// Get Input Buffers
   push rLink;
   r0 = M[I2,M1];                       // OFFSET_INPUT_CH1_PTR
#ifdef BASE_REGISTER_MODE  
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   I0  = r0;
   L0  = r1,        r0 = M[I2,M1];     // OFFSET_INPUT_CH2_PTR
   
#ifdef BASE_REGISTER_MODE  
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B1;
#else
   call $frmbuffer.get_buffer;
#endif
   I1  = r0;
   L1  = r1;   
   
   // Use input frame size
   r10 = r3,        r0 = M[I2,M1];     // OFFSET_OUTPUT_PTR
   
   // Update output frame size from input
   call $frmbuffer.set_frame_size;
   
   // Get output buffer
#ifdef BASE_REGISTER_MODE  
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B4;
#else
   call $frmbuffer.get_buffer;
#endif
   I4 = r0,     r0 = M[I2,M1];      // OFFSET_PTR_CH1_MANTISSA
   L4 = r1,     r5 = M[I2,M1];      // OFFSET_PTR_CH2_MANTISSA
   pop rLink;
   
   r4 = M[r0], r2 = M[I2,M1];       // OFFSET_PTR_EXPONENT               
   // r0 = first input ch1
   r5 = M[r5], r0 = M[I0,M1];
   // r1 = first input ch2
   r6 = M[r2], r1 = M[I1,M1];

   // Sum & Copy
   do lp_stream_copy;
      // CH1 * CH1_Mantisa
      rMAC = r0 * r4, r0 = M[I0,M1];
      // CH2 * CH2_Mantisa
      rMAC = rMAC + r1 * r5, r1 = M[I1,M1];
      // Shift for exponent
      r2 = rMAC ASHIFT r6;
      // Save Output (r2)
      M[I4,M1] = r2;
lp_stream_copy:

   // clear L-regs
   L0 = 0;
   L1 = 0;
   L4 = 0;
#ifdef BASE_REGISTER_MODE  
   push Null;
   B1 = M[SP-MK1];
   B4 = M[SP-MK1];
   pop  B0;
#endif   
   rts;
.ENDMODULE;

// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// MODULE:
//    $M.stereo_3d_enhancement
//
// DESCRIPTION:
//    Initialize function gets called when system -reinit is called.
//    This function clears the 2 delay buffers and makes sure that the correct
//    reflection delay value gets used.
//    Process function copies data from one buffer into another.
//
// INPUTS:
//   r8 -  data object containing pointers to input and output data.  Also
//         contains the number of samples to process.
//
// OUTPUTS:
//   - none
//
// TRASHED REGISTERS:
// Almost all r-regs and index registers with their M and L values
//
// *****************************************************************************

#include "core_library.h"
#include "stereo_3d_enhancement.h"
#ifdef KYMERA
#include "cbuffer_asm.h"
#else
#include "cbuffer.h"
#endif

.MODULE $M.stereo_3d_enhancement;
 .CODESEGMENT AUDIO_PROC_STEREO_3D_ENHANCEMENT_PM;
 .DATASEGMENT DM;

$stereo_3d_enhancement.initialize:
   push rLink;

   // get the delay buffer struc address
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_1_STRUC_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   B0 = M[SP-MK1];
   pop  B1;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I1 = r0;
   L1 = r1;
   I0 = r0;
   L0 = r1;

   r0 = M[r8 + $stereo_3d_enhancement.DELAY_2_STRUC_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   B4 = M[SP-MK1];
   pop  B5;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I4 = r0;
   L4 = r1;
   I5 = r0;
   L5 = r1;

   // Clear up the buffers
   r10 = &$stereo_3d_enhancement.DELAY_BUFFER_SIZE;
   r0 = NULL;
   do loop_clear_buffers;
    M[I1,MK1] = r0, M[I5,MK1] = r0;
   loop_clear_buffers:

   // Update the reflection delay value in the delay line cbuffers write pointers
   r2 = M[r8 + $stereo_3d_enhancement.REFLECTION_DELAY_SAMPLES_FIELD];
   Words2Addr(r2);
   M0 = r2;

   // offset the index pointer REFLECTION_DELAY_SAMPLES from read pointer
   r0 = M[I0,M0], r1 = M[I4,M0];

   // Update the write addr field with the new value in r0
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_1_STRUC_FIELD];
   r1 = I0;
   call $cbuffer.set_write_address;
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_2_STRUC_FIELD];
   r1 = I4;
   call $cbuffer.set_write_address;

   // Clear the length registers
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
   rts;

$stereo_3d_enhancement:

   // check if SE is bypassed
   r0 = M[r8 + $stereo_3d_enhancement.SE_CONFIG_FIELD];
   r1 = M[r8 + $stereo_3d_enhancement.ENABLE_BIT_MASK_FIELD];
   r7 = r8;
   r1 = r1 AND r0;
   if NZ jump $M.audio_proc.stereo_copy.Process.func;

   push rLink;

   //**** Get Input Buffers *****
   r0 = M[r8 + $stereo_3d_enhancement.INPUT_CH1_PTR_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   I0  = r0;
   L0  = r1;

   r0 = M[r8 + $stereo_3d_enhancement.INPUT_CH2_PTR_BUFFER_FIELD];
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

// get the delay buffer struc address
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_1_STRUC_FIELD];
   // get the delay buffer write address
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop  B1;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   // I4 for the delay buffer write
   I1 = r0;
   L1 = r1;

   // get the delay buffer struc address
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_2_STRUC_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop  B5;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   // I4 for the delay buffer write
   I5 = r0;
   L5 = r1;

   // loop ONE starts

   // preserve the r10 value
   r5 = r10;

   do loop_copy;

      r0 = M[I0,MK1], r1 = M[I4,MK1];
      M[I1,MK1] = r0, M[I5,MK1] = r1;

loop_copy:

   // loop ONE ends
   // restore input cbuffer read index
      // store back the current delay buffer write address
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_1_STRUC_FIELD];
   r1 = I1;
   call $cbuffer.set_write_address;
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_2_STRUC_FIELD];
   r1 = I5;
   call $cbuffer.set_write_address;

   // zero the length registers and write the last sample
   L0 = 0;
   L4 = 0;
   L1 = 0;
   L5 = 0;
#ifdef BASE_REGISTER_MODE
   push Null;
   B4 = M[SP-MK1];
   B5 = M[SP-MK1];
   B1 = M[SP-MK1];
   pop  B0;
#endif


   // *** Sound processing loop
   //     Connections
   //     AG4 == output_left
   //     AG0 == output_right
   //     AG5 == delay_left
   //     AG1 == delay_right
   //

   // restore loop counter here to free r5
   r10 = r5 - 1;

   // set up modify registers
   r0 = M[r8 + $stereo_3d_enhancement.REFLECTION_DELAY_SAMPLES_FIELD];
   Words2Addr(r0);
   r1 = MK1;
   M0 = r1 - r0;
   M1 = r0;
   M2 = r1;
   M3 = (-7 * MK1);


   // get the coeficients buffer read index
   r0 = M[r8 + $stereo_3d_enhancement.COEFF_STRUC_FIELD];
   I2 = r0;

   // get the delay buffer (right channel) struc address
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_1_STRUC_FIELD];
   // get the delay buffer read address
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop  B1;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I1 = r0;
   L1 = r1;

   // get the delay buffer (left channel) struc address
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_2_STRUC_FIELD];
   // get the delay buffer read address
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop  B5;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I5 = r0;
   L5 = r1;


// ***** Get output buffers ******
// Update output frame size from input
   r0 = M[r8 + $stereo_3d_enhancement.OUTPUT_CH1_PTR_BUFFER_FIELD];
   r3 = r5;
   call $frmbuffer.set_frame_size;
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   I0 = r0;
   L0 = r1;
// Update output frame size from input
   r0 = M[r8 + $stereo_3d_enhancement.OUTPUT_CH2_PTR_BUFFER_FIELD];
   r3 = r5;
   call $frmbuffer.set_frame_size;
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B4;
#else
   call $frmbuffer.get_buffer;
#endif
   I4 = r0, r5 = M[I2,M2];
   L4 = r1, r3 = M[I1,M1];


   // * Processing loop starts


   //   First process the first element
   r6 = -1.0;

   // left channel processing read delay left
   r1 = M[I1,M0];
   // Already loaded r5 with first coef alot earlier

   rMAC = r3 * r5 (SS), r2 = M[I5,M1], r5 = M[I2,M2];
   rMAC = rMAC + r2 * r5 (SS), r5 = M[I2,M2];
   //                      read cur left
   rMAC = rMAC + r1 * r5 (SS), r0 = M[I5,M0], r5 = M[I2,M2];
   rMAC = rMAC + r0 * r5 (SS), r5 = M[I2,M2];
   r4 = r0 + rMAC;
   // saturate if needed
   if V r4 = r4 * r6 (int) (sat);


   // right channel processing
   // write this left result
   rMAC = r2 * r5 (SS), M[I4,M2] = r4, r5 = M[I2,M2];
   rMAC = rMAC + r3 * r5 (SS), r5 = M[I2,M2];
   // jump back to start
   rMAC = rMAC + r0 * r5 (SS), r5 = M[I2,M3];
   rMAC = rMAC + r1 * r5 (SS), r5 = M[I2,M2];
   // read delay right
   r4 = r1 + rMAC, r3 = M[I1,M1];
   // saturate if needed
   // and read cur right
   if V r4 = r4 * r6(int)(sat), r1 = M[I1,M0];


   do loop_dsp;

      // * The algorithm starts here
      //   Adding the terms from small to large, in order to avoid unneccessary
      //   overflow which might happen in the intermediate results

      // left channel processing read delay left
      rMAC = r3 * r5 (SS), r2 = M[I5,M1], r5 = M[I2,M2];
      rMAC = rMAC + r2 * r5 (SS), r5 = M[I2,M2];
      // read cur left
      rMAC = rMAC + r1 * r5 (SS), r0 = M[I5,M0], r5 = M[I2,M2];
      // write last right result
      rMAC = rMAC + r0 * r5 (SS), M[I0,M2] = r4;

      r4 = r0 + rMAC, r5 = M[I2,M2];
      // saturate if needed
      if V r4 = r4 * r6 (int) (sat);


      // right channel processing
      // write this left result
      rMAC = r2 * r5 (SS), M[I4,M2] = r4, r5 = M[I2,M2];
      rMAC = rMAC + r3 * r5 (SS), r5 = M[I2,M2];
      // jump back to start
      rMAC = rMAC + r0 * r5 (SS), r5 = M[I2,M3];
      rMAC = rMAC + r1 * r5 (SS), r5 = M[I2,M2];
      // read delay right
      r4 = r1 + rMAC, r3 = M[I1,M1];
      // saturate if needed
      // and read cur right
      if V r4 = r4 * r6 (int) (sat), r1 = M[I1,M0];
      // * end of algorithm

loop_dsp: // Loop dsp ends

   // zero L4
   // and write the last right result
   L4 = 0, M[I0,M2] = r4;

   // zero L0
   // and move the pointer back by 1 word
   L0 = 0, r1 = M[I1,-MK1];

   // store the current delay buffer read pointer
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_2_STRUC_FIELD];
   r1 = I5;
   call $cbuffer.set_read_address;
   r0 = M[r8 + $stereo_3d_enhancement.DELAY_1_STRUC_FIELD];
   r1 = I1;
   call $cbuffer.set_read_address;
   L5 = 0;
   L1 = 0;
#ifdef BASE_REGISTER_MODE
   push Null;
   B4 = M[SP-MK1];
   B5 = M[SP-MK1];
   B1 = M[SP-MK1];
   pop  B0;
#endif

   // pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;

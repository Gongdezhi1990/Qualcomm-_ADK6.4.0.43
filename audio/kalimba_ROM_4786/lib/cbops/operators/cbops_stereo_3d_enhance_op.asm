// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    copy operator
//
// DESCRIPTION:
//    This operator does a straight forward copy of samples from the input
// buffer to the output buffer.
//
// When using the operator the following data structure is used:
//    - $cbops.stereo_3d_enhance_op.INPUT_1_START_INDEX_FIELD = the index of the input
//       buffer 1
//    - $cbops.stereo_3d_enhance_op.INPUT_2_START_INDEX_FIELD = the index of the input
//       buffer 2
//    - $cbops.stereo_3d_enhance_op.OUTPUT_1_START_INDEX_FIELD = the index of the output
//       1 buffer
//    - $cbops.stereo_3d_enhance_op.OUTPUT_2_START_INDEX_FIELD = the index of the output
//       2 buffer
//    - $cbops.stereo_3d_enhance_op.DELAY_1_STRUC_INDEX_FIELD = the index of the delay
//       buffer 1 structure
//    - $cbops.stereo_3d_enhance_op.DELAY_2_STRUC_INDEX_FIELD = the index of the delay
//       buffer 2 structure
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.stereo_3d_enhance_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.stereo_3d_enhance_op[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.stereo_3d_enhance_op.reset,   // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.stereo_3d_enhance_op.main;    // main function

   // ** Algorithm coefficients buffer
   // $coef_1_le  0.31; // should be 1.31, the value here is minused 1
   // $coef_2_le  -0.44;
   // $coef_1_ri  0.31;
   // $coef_2_ri  -0.44;
   // $coef_3_le  0.74;
   // $coef_4_le  -0.38;
   // $coef_3_ri  0.68;
   // $coef_4_ri  -0.34;
   // Inside buffer, they must be placed in the order that used by the algorithm
   // From left to right, coef 4 to 1
   .VAR/DM1 $cbops.stereo_3d_enhance_op.coef_buf[8] = -0.38, 0.74, -0.44, 0.31, -0.34, 0.68, -0.44, 0.31;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $cbops.stereo_3d_enhance.reset
// DESCRIPTION:
//    This reset function will clear the delay buffers
// INPUTS:
//    None.
// OUTPUTS:
//    None.
// *****************************************************************************

.MODULE $M.cbops.stereo_3d_enhance_op.reset;
   .CODESEGMENT CBOPS_STEREO_3D_ENHANCE_RESET_PM;
   .DATASEGMENT DM;


$cbops.stereo_3d_enhance_op.reset:

   // push rLink onto stack
   push rLink;

   //** function starts


   // setup the AG for delay buffer writting
   // get the delay buffer struc address
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_1_STRUC_FIELD];
   // get the delay buffer write address
   call $cbuffer.get_write_address_and_size;
   // I4 for the delay buffer write
   I4 = r0;
   // backup this write address
   r3 = r0;
   L4 = r1;


   // setup the AG for delay buffer writting
   // get the delay buffer struc address
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_2_STRUC_FIELD];
   // get the delay buffer write address
   call $cbuffer.get_write_address_and_size;
   // I4 for the delay buffer write
   I0 = r0;
   // backup this write address
   r5 = r0;
   L0 = r1;

   r10 = r1 + 1;
   r0 = 0;

   // write 0 to the whole buffer
   do reset_loop;
      M[I4,1] = r0,  M[I0,1] = r0;
reset_loop:

   // no need to restore delay buffer write address
   L4 = 0;
   L0 = 0;


   //** function ends
   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $cbops.stereo_copy_op.main
//
// DESCRIPTION:
//    Operator that copies the input sample to the output
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Everything
//
// *****************************************************************************
.MODULE $M.cbops.stereo_3d_enhance_op.main;
   .CODESEGMENT CBOPS_STEREO_3D_ENHANCE_MAIN_PM;
   .DATASEGMENT DM;

$cbops.stereo_3d_enhance_op.main:

   push rLink;

#ifdef ENABLE_PROFILER_MACROS
   // start profiling if enabled
   .VAR/DM1 $cbops.profile_stereo_3d_enhance_op[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
   r0 = &$cbops.profile_stereo_3d_enhance_op;
   call $profiler.start;
#endif


   // *** Loop ONE, fill up the delay buffer
   //     Connections
   //     AG0 - r0 -> AG1
   //     AG4 - r1 -> AG5


   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.INPUT_1_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.INPUT_2_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;


   // get the delay buffer struc address
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_1_STRUC_FIELD];
   // get the delay buffer write address
   call $cbuffer.get_write_address_and_size;
   // I4 for the delay buffer write
   I1 = r0;
   L1 = r1;

   // get the delay buffer struc address
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_2_STRUC_FIELD];
   // get the delay buffer write address
   call $cbuffer.get_write_address_and_size;
   // I4 for the delay buffer write
   I5 = r0;
   L5 = r1;

   // loop ONE starts

   // preserve the r10 value
   r5 = r10;

   do loop_copy;

      r0 = M[I0,1], r1 = M[I4,1];
      M[I1, 1] = r0, M[I5,1] = r1;

loop_copy:


   // zero the length registers and write the last sample
   L0 = 0;
   L4 = 0;

   // loop ONE ends
   // restore input cbuffer read index
      // store back the current delay buffer write address
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_1_STRUC_FIELD];
   r1 = I1;
   call $cbuffer.set_write_address;
   L1 = 0;

   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_2_STRUC_FIELD];
   r1 = I5;
   call $cbuffer.set_write_address;
   L5 = 0;


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
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.REFLECTION_DELAY_SAMPLES_FIELD];
   r1 = 1;
   M0 = r1 - r0;
   M1 = r0;
   M2 = r1;
   M3 = -7;


   // get the coeficients buffer read index
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.COEF_STRUC_FIELD];
   I2 = r0;


   // get the delay buffer (right channel) struc address
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_1_STRUC_FIELD];
   // get the delay buffer read address
   call $cbuffer.get_read_address_and_size;
   I1 = r0;
   L1 = r1;


   // get the delay buffer (left channel) struc address
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_2_STRUC_FIELD];
   // get the delay buffer read address
   call $cbuffer.get_read_address_and_size;
   I5 = r0;
   L5 = r1;


   // get the offset to the output left
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.OUTPUT_2_START_INDEX_FIELD];
   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I0
   I4 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L4 = r1;


   // get the offset to the output right
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.OUTPUT_1_START_INDEX_FIELD];
   // get the output buffer write address
   r1 = M[r6 + r0], r3 = M[I1,M1];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0], r5 = M[I2,M2];
   // store the value in L0
   L0 = r1;



   // * Processing loop starts


   //   First process the first element
   r6 = 0x800000;

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
   L0 = 0, r1 = M[I1,-1];


   // store the current delay buffer read pointer
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_2_STRUC_FIELD];
   r1 = I5;
   call $cbuffer.set_read_address;
   L5 = 0;
   r0 = M[r8 + $cbops.stereo_3d_enhance_op.DELAY_1_STRUC_FIELD];
   r1 = I1;
   call $cbuffer.set_read_address;
   L1 = 0;


#ifdef ENABLE_PROFILER_MACROS
   // stop profiling if enabled
   r0 = &$cbops.profile_stereo_3d_enhance_op;
   call $profiler.stop;
#endif


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \File buffer_interleave.asm
 *
 *
 * \section sec1 Contains:
 */
#ifndef INTERLEAVE_OP_ASM
#define INTERLEAVE_OP_ASM
#include "stack.h"

#ifdef PATCH_LIBS
#include "patch_library.h"
#endif

// *****************************************************************************
// MODULE:
//    $buffer_interleave.interleave_input_buffers
//
// DESCRIPTION:
//    copy multiple buffers into a single interleaved buffer
//
// INPUTS:
//    - r0 = output interleaved buffer
//    - r1 = array of input buffers
//    - r2 = number of buffers
//    - r3 = max number of samples to interleave
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.interleave.interleave_buffers;
   .CODESEGMENT BUFFER_INTERLEAVE_PM;
   .DATASEGMENT DM;
$_interleave_buffers:
$buffer_interleave.interleave_buffers:

   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   PUSH_ALL_C

   // Store input params
   // r8: output buffer (interleaved)
   // r7: input buffers (de-interleaved)
   // r6: number of buffers
   r8 = r0;
   r7 = r1;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($buffer_interleave.BUFFER_INTERLEAVE_ASM.INTERLEAVE.INTERLEAVE_BUFFERS.INTERLEAVE_BUFFERS.PATCH_ID_0, r1)   
#endif   
   
   
   r6 = r2;
   M0 = ADDR_PER_WORD;

   // get the space available in the interleaved buffer
   r0 = r8;
   call $cbuffer.calc_amount_space_in_words;
   r4 = r0;

   // find amount data that can be read from
   // de-interleaved buffers
   I6 = r7;
   r10 = r6;
   r5 = 1.0; // just a big number
   do find_amount_data_loop;
       // see how much data is in next buffer
       r0 = M[I6, M0];
       call $cbuffer.calc_amount_data_in_words;
       // update min data available
       r5 = MIN r0;
   find_amount_data_loop:
   find_amount_data_loop_end:

   // calculate how many words can be
   // copied from each buffer
   // r4 = amount space available
   // r5 = min amount data available
   // r6 = number of buffers
   // amount to copy this time = min(r5, r4/6)
   rMAC = 0;
   rMAC0 = r4;
   Div = rMAC / r6;
   r4 = DivResult;
   r4 = MIN r5;
   r4 = MIN r3;
   if Z jump end;

   // there are something to copy
   // open the output buffer
   r0 = r8;
   #ifdef BASE_REGISTER_MODE
      call $cbuffer.get_write_address_and_size_and_start_address;
      push r2;
      pop  B0;
      push r2;
      pop  B1;
   #else
      call $cbuffer.get_write_address_and_size;
   #endif
   I1 = r0;
   L0 = r1;
   L1 = r1;
   I6 = r7;
   r0 = M0;
   r0 = r0 * r6 (int);
   M1 = r0;
   // at this point
   // r6 = number of buffers
   // I1 = firts word of output space
   // M0 = move one word
   // M1 = move r6 words
   // r4 = number of words to read from each input buffer
   interleave_outer_loop:
      r6 = r6 - 1;
      if NEG jump end_interleave_outer_loop;
          // in each outer loop iteration one
          // buffer is read and copied into output buffer
          // I1 holds the first address to copy this buffer
          I0 = I1;

          /* open next input buffer */
          r0 = M[I6, 0];
          #ifdef BASE_REGISTER_MODE
              call $cbuffer.get_read_address_and_size_and_start_address;
              push r2;
              pop  B4;
          #else
              call $cbuffer.get_read_address_and_size;
          #endif
          L4 = r1;
          I4 = r0;
          r10 = r4 - 1;

          /* copy this buffer to output */
          r0 = M[I4, M0];
          do interleave_copy_loop;
             r0 = M[I4, M0], M[I0, M1] = r0;
          interleave_copy_loop:
          M[I0, M1] = r0;

          /* close the input buffer */
          r0 = M[I6, M0];
          r1 = I4;
          call $cbuffer.set_read_address;

          /* for output buffer:
           * move to next word so we can copy
           * the next buffer
           */
          r0 = M[I1, M0];

   jump interleave_outer_loop;
   end_interleave_outer_loop:

   /* close the output buffer */
   M1 = M0 - M1;
   r0 = M[I0, M1];
   r1 = I0;
   r0 = r8;
   call $cbuffer.set_write_address;

end:
   r0 = r4;
   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $buffer_interleave.deinterleave_buffers
//
// DESCRIPTION:
//    deinterleave a single buffer to multiple buffers
//
// INPUTS:
//    - r0 = input interleaved buffer
//    - r1 = array of output buffers
//    - r2 = number of output buffers
//    - r3 = mux number of samples to de-interleave
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.interleave.deinterleave_buffers;
   .CODESEGMENT BUFFER_INTERLEAVE_PM;
   .DATASEGMENT DM;
$_deinterleave_buffers:
$buffer_interleave.deinterleave_buffers:

   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   PUSH_ALL_C

   // Store input params
   // r8: input buffer (interleaved)
   // r7: output buffers (deinterleaved)
   // r6: number of buffers
   r8 = r0;
   r7 = r1;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($buffer_interleave.BUFFER_INTERLEAVE_ASM.INTERLEAVE.DEINTERLEAVE_BUFFERS.DEINTERLEAVE_BUFFERS.PATCH_ID_0, r1)   
#endif   
   
   
   r6 = r2;
   M0 = ADDR_PER_WORD;

   // get the data available in the interleaved buffer
   r0 = r8;
   call $cbuffer.calc_amount_data_in_words;
   r4 = r0;

   // find amount space that can be written to
   // de-interleaved buffers
   I6 = r7;
   r10 = r6;
   r5 = 1.0; // just a big number
   do find_amount_data_loop;
       /* see how much space is in next buffer */
       r0 = M[I6, M0];
       call $cbuffer.calc_amount_space_in_words;
       /* update min space available */
       r5 = MIN r0;
   find_amount_data_loop:
   find_amount_data_loop_end:
   // calculate how many words can be
   // copied to each buffer
   // r4 = amount data available
   // r5 = min amount space available
   // r6 = number of buffers
   // amount to copy this time = min(r5, r4/6)
   rMAC = 0;
   rMAC0 = r4;
   Div = rMAC / r6;
   r4 = DivResult;
   r4 = MIN r5;
   r4 = MIN r3;
   if Z jump end;

   // there are something to copy
   // open the input buffer
   r0 = r8;
   #ifdef BASE_REGISTER_MODE
      call $cbuffer.get_read_address_and_size_and_start_address;
      push r2;
      pop  B0;
      push r2;
      pop  B1;
   #else
      call $cbuffer.get_read_address_and_size;
   #endif
   I1 = r0;
   L0 = r1;
   L1 = r1;
   I6 = r7;
   r0 = M0;
   r0 = r0 * r6 (int);
   M1 = r0;
   // at this point
   // r6 = number of buffers
   // I1 = firts word of input
   // M0 = move one word
   // M1 = move r6 words
   // r4 = number of words to write to each output buffer
   interleave_outer_loop:
      r6 = r6 - 1;
      if NEG jump end_interleave_outer_loop;
          // in each outer loop iteration one
          // buffer is read and copied into output buffer
          // I1 holds the first address to read this buffer
          I0 = I1;

          /* open next output buffer */
          r0 = M[I6, 0];
          #ifdef BASE_REGISTER_MODE
              call $cbuffer.get_write_address_and_size_and_start_address;
              push r2;
              pop  B4;
          #else
              call $cbuffer.get_write_address_and_size;
          #endif
          L4 = r1;
          I4 = r0;
          r10 = r4 - 1;
          r0 = M[I0, M1];
          do interleave_copy_loop;
             r0 = M[I0, M1], M[I4, M0] = r0;
          interleave_copy_loop:
          M[I4, M0] = r0;

          /* close the output buffer*/
          r0 = M[I6, M0];
          r1 = I4;
          call $cbuffer.set_write_address;

          /* for input buffer:
          * move to next word so we can write
          * the next buffer
          */
           r0 = M[I1, M0];

   jump interleave_outer_loop;
   end_interleave_outer_loop:
   /* close the input buffer */
   M1 = M0 - M1;
   r0 = M[I0, M1];
   r1 = I0;
   r0 = r8;
   call $cbuffer.set_read_address;
end:
   r0 = r4;
   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;
.ENDMODULE;

#endif //#ifndef INTERLEAVE_OP_ASM

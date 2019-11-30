// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// *****************************************************************************
#include "cbuffer_asm.h"        // for conversion macros

// *****************************************************************************
// NAME:
//      MMU buffer
//
// DESCRIPTION:
//      Efficient routines for copying to/from a circular buffer.

// *****************************************************************************
#ifndef CIRCULAR_BUFFER_COPY_INCLUDED
#define CIRCULAR_BUFFER_COPY_INCLUDED

// *****************************************************************************
// MODULE:
//    $circular_buffer.copyfrom
//
// DESCRIPTION:
//    Read words from a circular buffer in an efficient way.
//
// INPUTS:
//    - r0 = Pointer to destination buffer
//    - r1 = Pointer to start of source buffer
//    - r2 = Offset into the source buffer to copy from
//    - r3 = Size of the source buffer
// M[SP-1] = Number of words to copy.
//
// OUTPUT:
//    None
//
// TRASHED REGISTERS:
//    r10
//
// *****************************************************************************

// void circular_buffer_copyfrom(int* dest_buf, const int* src_buf_start, int src_buf_offset, int src_buf_size, int length)
.MODULE $M.circular_buffer.copyfrom;
   .CODESEGMENT CIRCULAR_BUFFER_COPYFROM_PM;

   $_circular_buffer_copyfrom:
   $circular_buffer.copyfrom:

   // We use some extra stack to set up base register mode
   pushm <FP(=SP)>;
   pushm <I0, I2, L0>;

   // Get the buffer base address into B0
   push r1;
   pop B0;

   // Set the index register for use with source pointer (start address + offset)
   BUFFER_WORDS_TO_ADDRS_ASM(r2);
   I0 = r1 + r2;
   // Set index register for use with destination pointer
   I2 = r0;

   // Get (source) buffer size into L0
   L0 = r3;

   // Setup loop counter
   r10 = M[FP-1*ADDR_PER_WORD];

   do read_loop;
      r1 = M[I0,ADDR_PER_WORD];
      M[I2,ADDR_PER_WORD] = r1;     // optimise?
   read_loop:

   // Restore index & length registers
   popm <I0, I2, L0>;
   popm <FP>;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $circular_buffer.copyto
//
// DESCRIPTION:
//    Write words to a circular buffer in an efficient way.
//
// INPUTS:
//    - r0 = Pointer to start of destination buffer
//    - r1 = Offset into the destination buffer to copy from
//    - r2 = Size of the destination buffer
//    - r3 = Pointer to source buffer
// M[SP-1] = Number of words to copy.
//
// OUTPUT:
//    None
//
// TRASHED REGISTERS:
//    r10
//
// *****************************************************************************

// circular_buffer_copyto(int* dest_buf_start, int dest_buf_offset, int dest_buf_size, const int* src_buf, int length)
.MODULE $M.circular_buffer.copyto;
   .CODESEGMENT CIRCULAR_BUFFER_COPYTO_PM;

   $_circular_buffer_copyto:
   $circular_buffer.copyto:

   // We use some extra stack to set up base register mode
   pushm <FP(=SP)>;
   pushm <I0, I2, L0>;

   // Get the buffer base address into B0
   push r0;
   pop B0;

   // Set index register for use with source pointer
   I2 = r3;
   // Set the index register for use with destination pointer (start address + offset)
   BUFFER_WORDS_TO_ADDRS_ASM(r1);
   I0 = r0 + r1;

   // Get (destination) buffer size into L0
   L0 = r2;

   // Setup loop counter
   r10 = M[FP-1*ADDR_PER_WORD];

   do read_loop;
      r1 = M[I2,ADDR_PER_WORD];
      M[I0,ADDR_PER_WORD] = r1;     // optimise?
   read_loop:

   // Restore index & length registers
   popm <I0, I2, L0>;
   popm <FP>;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $circular_buffer.copybetween
//
// DESCRIPTION:
//    Copy between circular buffers in an efficient way.
//
// INPUTS:
//    - r0 = Pointer to start of destination buffer
//    - r1 = Offset into the destination buffer to copy to
//    - r2 = Size of the destination buffer
//    - r3 = Pointer to source buffer
// M[SP-1] = Offset into the source buffer to copy from
// M[SP-2] = Size of the source buffer
// M[SP-3] = Number of words to copy.
//
// OUTPUT:
//    None
//
// TRASHED REGISTERS:
//    r10
//
// *****************************************************************************

// void circular_buffer_copybetween(int* dest_buf_start, int dest_buf_offset, int dest_buf_size,
//                                   const int* src_buf_start, int src_buf_offset, int src_buf_size, int length);
.MODULE $M.circular_buffer.copybetween;
   .CODESEGMENT CIRCULAR_BUFFER_COPYBETWEEN_PM;

   $_circular_buffer_copybetween:
   $circular_buffer.copybetween:

   // We use some extra stack to set up base register mode
   pushm <FP(=SP)>;
   pushm <I0, I1, L0, L1>;

   // Get the source buffer base address into B0
   push r3;
   pop B0;
   // Get the destination buffer base address into B1
   push r0;
   pop B1;

   // Set index register for use with source pointer (start address + offset)
   r10 = M[FP-1];
   BUFFER_WORDS_TO_ADDRS_ASM(r10);
   I0 = r3 + r10;
   // Set the index register for use with destination pointer (start address + offset)
   BUFFER_WORDS_TO_ADDRS_ASM(r1);
   I1 = r0 + r1;

   // Get source buffer size into L0
   L0 = M[FP-2];
   // Get destination buffer size into L1
   L1 = r2;

   // Setup loop counter
   r10 = M[FP-3*ADDR_PER_WORD];

   do copy_loop;
      r1 = M[I0,ADDR_PER_WORD];
      M[I1,ADDR_PER_WORD] = r1;     // optimise?
   copy_loop:

   // Restore index & length registers
   popm <I0, I1, L0, L1>;
   popm <FP>;
   rts;

.ENDMODULE;


#endif   // CIRCULAR_BUFFER_COPY_INCLUDED


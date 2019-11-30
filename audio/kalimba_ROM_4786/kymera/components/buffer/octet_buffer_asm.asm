// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Octet base Buffer Library
//
// DESCRIPTION:
//    This library provides an API for dealing with octet base buffers in Kalimba 
//    (memory which are called octet_buffers or obuffers). These buffers may be 
//    managed purely by software. No in-place support implemented for these 
//    functions.
//
// *****************************************************************************

#include "cbuffer_asm.h"
#include "octet_buffer_asm_defs.h"
#include "faultids_asm_defs.h"




/* High-level data access functions
 * No in-place support implemented for these functions.
 */
.MODULE $M.obuffer_c_stubs;
   .CODESEGMENT BUFFER_PM;


// extern void obuffer_copy_aligned_16bit(tCbuffer *dest, tCbuffer *src, unsigned int amount, unsigned int write_offset);
// Additional inputs:
// M1 = ADDR_PER_WORD
// M2 = -ADDR_PER_WORD
$_obuffer_copy_aligned_16bit:
   // save the input paramerters for later
   pushm <FP(=SP), r0, r1, r2, r5,rLink>;
   pushm <I0, I4, L0, L4>;

   // get dest buffer write address and size
   call $cbuffer.get_write_address_and_size_and_start_address;
   I4 = r0;
   L4 = r1;
   push r2;
   pop B4;

   // get src buffer read address and size
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   // get the read address ans save it to the stack
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   r5 =  M[FP + 3*ADDR_PER_WORD];               // copy amount
   Null = r3;                                   // r3 is the write offset

   if Z jump no_offset;
   // read the second part of the word, combine it with the first one.

   r1 = M[I0, ADDR_PER_WORD];
   r1 = r1 AND 0xff;                            // The new octet is the LSB.
   r2 = M[I4, -ADDR_PER_WORD];                  // Go back one address at the src
   r2 = M[I4, 0];                               // Read the msb
   r2 = r2 AND 0xff00;                          // Mask the lsb
   r1 = r1 + r2;                                // combine it
   M[I4, ADDR_PER_WORD] = r1;                   // write it to the buffer
   r5 = r5 - 1;                                 // one octet already written, decrement
                                                // the copy amount

   no_offset:
   r10 =  r5 LSHIFT -1;                         // convert the copy amount to words
   if Z jump copy_aligned_write_last_octet;     // could be that only one octet we need to copy.

   r10 = r10 - 1;                   // decrement the amount due to the initial read and last write
   r1 = M[I0,ADDR_PER_WORD];        // initial read
   do copy_aligned_loop;
      r1 = M[I0,ADDR_PER_WORD],
       M[I4,ADDR_PER_WORD] = r1;    // read and write
   copy_aligned_loop:
   M[I4,ADDR_PER_WORD] = r1;        // last write


   copy_aligned_write_last_octet:

   Null = r5 AND 0x1;
   if Z jump copy_aligned_done;
   //read last word.
   r1 = M[I0,ADDR_PER_WORD];
   r1 = r1 AND 0xff00;

   r2 = M[I4,0];
   r2 = r2 AND 0xff;

   r1 = r1 + r2;
   M[I4,ADDR_PER_WORD] = r1;


   copy_aligned_done:
   // Update the write address
   r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
   r1 = I4;
   call $cbuffer.set_write_address;


   Null = r5 AND 0x1;
   if Z jump no_read_decrement;
   r1 = M[I0,-ADDR_PER_WORD];
   no_read_decrement:

   // Update the read address
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   r1 = I0;
   call $cbuffer.set_read_address;

   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r5, rLink>;
   rts;

// extern void obuffer_copy_unaligned_16bit(tCbuffer *dest, tCbuffer *src, unsigned int amount, unsigned int write_offset);
// Additional inputs:
// M1 = ADDR_PER_WORD
// M2 = -ADDR_PER_WORD
$_obuffer_copy_unaligned_16bit:
   // save the input paramerters for later
   pushm <FP(=SP), r0, r1, r2, r5, r7, rLink>;
   pushm <I0, I4, L0, L4>;

   // get dest buffer write address and size
   call $cbuffer.get_write_address_and_size_and_start_address;
   I4 = r0;
   L4 = r1;
   push r2;
   pop B4;

   // get src buffer read address and size
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   // get the read address ans save it to the stack
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;


   r10 =  M[FP + 3*ADDR_PER_WORD];              // copy amount
   r7 = r3 + r10;                               // r3 is the write offset
   push r7;
   r10 = r7 LSHIFT -1;                          // translate to words

   //r10 = r10 - 1;                             // decrement with
   Null = r3 - 0;
   if Z jump read_offset;
   // read the carry over from the dest buffer
   r5 = M[I4, -ADDR_PER_WORD];
   r5 = M[I4, 0];
   r5 = r5 LSHIFT -8; // carry over. yes, it will be shifted back in the unaligned_copy_loop loop


   jump copy_unaligned_internal;

   read_offset:
   // read the carry over from the src buffer
   r5 = M[I0, ADDR_PER_WORD];
   r5 = r5 AND 0xff; // carry over.


   copy_unaligned_internal:

   r0 = 8;                           // set r0 to a constant for masking the bottom octets
   r5 = r5 LSHIFT r0,r1 = M[I0, M1]; // read the value and

   do unaligned_copy_loop;
      r2 = r1 LSHIFT -8;            // get the msb which will be combined with the carry over
      r2 = r2 + r5;                 // combine wiht the carry over
      r5 = r1 AND 0xff;             // calculate the new carry over
      r5 = r5 LSHIFT r0,            // shift the carry over for the next use
          M[I4,M1] = r2,r1 = M[I0, M1]; // read and write to the memory
   unaligned_copy_loop:

   r1 = M[I0, M2];                  // compensate for the additional read
   r5 = r5 LSHIFT -8;               // compensate for the additional shift

   // write the carry over if neccessary
   pop r3;
   Null = r3 AND 0x1;
   if Z jump unaligned_copy_read_decrement;
   // write last octet
   r3 = M[I4,0];
   //r3 = M[I4,ADDR_PER_WORD];
   r3 = r3 AND 0xff;
   r5 = r5 LSHIFT 8;
   r3 = r3 + r5;
   M[I4,ADDR_PER_WORD] = r3;

   jump unaligned_copy_done;

   unaligned_copy_read_decrement:
   r0 = M[I0,-ADDR_PER_WORD];

   unaligned_copy_done:

   // Update the write address
   r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
   r1 = I4;
   call $cbuffer.set_write_address;


   // Update the read address
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   r1 = I0;
   call $cbuffer.set_read_address;

   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r5, r7, rLink>;
   rts;

// extern void obuffer_unpack_16bit(int *dest, tCbuffer *src, unsigned int amount, unsigned int read_offset);
// Additional inputs:
// M1 = ADDR_PER_WORD
// M2 = -ADDR_PER_WORD
$_obuffer_unpack_16bit:
   // save the input paramerters for later
   pushm <FP(=SP), r0, r1, r2, r5, r7, rLink>;
   pushm <I0, I4, L0, L4>;

   // uset the dest as a buffer with size of the amount
   I4 = r0;
   r2 = r2 LSHIFT LOG2_ADDR_PER_WORD;
   L4 = r2;
   push r0;
   pop B4;

   // get src buffer read address and size
   r0 = r1;                // cbuffer_src
   // get the read address ans save it to the stack
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;


   r7 =  M[FP + 3*ADDR_PER_WORD];              // copy amount
   Null = r3;                                  // r3 is the read offset

   if Z jump copy_unpacke_no_read_offset;
   // read the first byte
   r5 = M[I0, ADDR_PER_WORD];
   r5 = r5 AND 0xff;
   M[I4, ADDR_PER_WORD] = r5;
   r7 = r7 - 1;

   copy_unpacke_no_read_offset:


   r10 = r7 LSHIFT -1;                          // translate to words


   r0 = 0xFF;                           // set r0 to a constant for masking the bottom octets
   r1 = M[I0,M1];                       // read the first wird

   do copy_unpack_loop;
      r2 = r1 LSHIFT -8;                // set r2 as the higher octet from the read word
      r1 = r1 AND r0, M[I4,M1] = r2;    // set r1 as the lower octet from the read word
                                        // write the higher octet to the memory
      M[I4,M1] = r1, r1 = M[I0,M1];     // write the lower octet to the memory
                                        // read the next word
   copy_unpack_loop:

   r1 = M[I0,M2];                       // compensate for the additional read

   // write the carry over if neccessary
   Null = r7 AND 0x1;
   if Z jump copy_unpack_copy_done;

   r2 = M[I0,0];
   r2 = r2 LSHIFT -8;
   M[I4,ADDR_PER_WORD] = r2;

   copy_unpack_copy_done:


   // Update the read address
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   r1 = I0;
   call $cbuffer.set_read_address;

   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r5, r7, rLink>;
   rts;

// extern void obuffer_unpack_from_offset_16bit
// (int *dest, tCbuffer *src, unsigned int amount, unsigned int read_offset, unsigned advance_cbuffer);
// M1 = ADDR_PER_WORD
// M2 = -ADDR_PER_WORD
$_obuffer_unpack_from_offset_16bit:
   // save the input paramerters for later
   pushm <FP(=SP), r0, r1, r2, r5, r7, rLink>;
   pushm <I0, I4, L0, L4>;

   // uset the dest as a buffer with size of the amount
   I4 = r0;
   r2 = r2 LSHIFT LOG2_ADDR_PER_WORD;
   L4 = r2;
   push r0;
   pop B4;

   // get src buffer read address and size
   r0 = r1;                // cbuffer_src
   // get the read address ans save it to the stack
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   // Advance the read pointer
   r7 = M[FP - ADDR_PER_WORD]; // advance_cbuffer input
   r7 = r7 LSHIFT LOG2_ADDR_PER_WORD;
   M3 = r7;
   r0 = M[I0, M3]; // advancing the read pointer.


   r7 =  M[FP + 3*ADDR_PER_WORD];              // copy amount
   Null = r3;                                  // r3 is the read offset

   if Z jump copy_unpack_from_offset_no_read_offset;
   // read the first byte
   r5 = M[I0, ADDR_PER_WORD];
   r5 = r5 AND 0xff;
   M[I4, ADDR_PER_WORD] = r5;
   r7 = r7 - 1;

   copy_unpack_from_offset_no_read_offset:


   r10 = r7 LSHIFT -1;                          // translate to words


   r0 = 0xFF;                           // set r0 to a constant for masking the bottom octets
   r1 = M[I0,M1];                       // read the first wird

   do copy_unpack_from_offset_loop;
      r2 = r1 LSHIFT -8;                // set r2 as the higher octet from the read word
      r1 = r1 AND r0, M[I4,M1] = r2;    // set r1 as the lower octet from the read word
                                        // write the higher octet to the memory
      M[I4,M1] = r1, r1 = M[I0,M1];     // write the lower octet to the memory
                                        // read the next word
   copy_unpack_from_offset_loop:

   r1 = M[I0,M2];                       // compensate for the additional read

   // write the carry over if neccessary
   Null = r7 AND 0x1;
   if Z jump copy_unpack_from_offset_copy_done;

   r2 = M[I0,0];
   r2 = r2 LSHIFT -8;
   M[I4,ADDR_PER_WORD] = r2;

   copy_unpack_from_offset_copy_done:

   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r5, r7, rLink>;
   rts;




.ENDMODULE;





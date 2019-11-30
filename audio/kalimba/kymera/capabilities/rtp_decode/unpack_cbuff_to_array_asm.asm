// *****************************************************************************
// Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Functions needed by the RTP decode capability
//
// DESCRIPTION:
//    These helper functions for the RTP decode capability unpack the contents
//    of a 16-bit unpacked buffer into an array for easier processing.
//
// *****************************************************************************


/* High-level data access functions
 * No in-place support implemented for these functions.
 */
.MODULE $M.unpack_cbuff_to_array_c_stubs;
   .CODESEGMENT UNPACK_CBUFF_TO_ARRAY_PM;

#ifdef RUNNING_ON_KALSIM
// Use 'new' cbuffer function in the support lib
#include "../support_lib/cbuffer/cbuffer_ex_kalsim.h"
#endif

// extern void unpack_cbuff_to_array_16bit(int *dest, tCbuffer *src, unsigned int amount);
// Additional inputs:
// M1 = ADDR_PER_WORD
// M2 = -ADDR_PER_WORD
// trashed r3
$_unpack_cbuff_to_array_16bit:
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
   // get the read address and save it to the stack
   call $cbuffer.get_read_address_ex;
   I0 = r0;
   L0 = r2;
   push r3;
   pop B0;

   r7 =  M[FP + 3*ADDR_PER_WORD];       // copy amount
   Null = r1;                           // r1 is the read offset

   if Z jump copy_unpacked_no_read_offset;
   // read the first byte
   r5 = M[I0, ADDR_PER_WORD];
   r5 = r5 AND 0xff;
   M[I4, ADDR_PER_WORD] = r5;
   r7 = r7 - 1;                         // one octet was read so subtract it

   copy_unpacked_no_read_offset:

   r10 = r7 LSHIFT -1;                  // translate to words

   r0 = 0xFF;                           // set r0 to a constant for masking the bottom octets
   r1 = M[I0, M1];                      // read the first word

   do copy_unpack_loop;
      r2 = r1 LSHIFT -8;                // set r2 as the higher octet from the read word
      r1 = r1 AND r0,                   // set r1 as the lower octet from the read word
       M[I4, M1] = r2;                  // write the higher octet to the memory
      M[I4, M1] = r1,                   // write the lower octet to the memory
       r1 = M[I0, M1];                  // read the next word
   copy_unpack_loop:

   r1 = M[I0, M2];                       // compensate for the additional read

   // write the carry over if neccessary
   Null = r7 AND 0x1;
   if Z jump copy_unpack_copy_done;

   r2 = M[I0, 0];
   r2 = r2 LSHIFT -8;
   M[I4,ADDR_PER_WORD] = r2;

   copy_unpack_copy_done:


   // Update the read address
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   r1 = I0;
   r2 = r7 AND 0x1;                             // offset
   call $cbuffer.set_read_address_ex;

   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r5, r7, rLink>;
   rts;

// extern void unpack_cbuff_to_array_from_offset_16bit
// (int *dest, tCbuffer *src, unsigned int amount_to_copy, unsigned offset);
// M1 = ADDR_PER_WORD
// M2 = -ADDR_PER_WORD
$_unpack_cbuff_to_array_from_offset_16bit:
   // save the input paramerters for later
   pushm <FP(=SP), r0, r1, r2, r3, r5, r7, rLink>;
   pushm <I0, I4, L0, L4>;

   // use the dest as a buffer with size of the amount
   I4 = r0;
   r2 = r2 LSHIFT LOG2_ADDR_PER_WORD;
   L4 = r2;
   push r0;
   pop B4;

   // get src buffer read address and size
   r0 = r1;                // cbuffer_src
   // get the read address and save it to the stack
   call $cbuffer.get_read_address_ex;
   I0 = r0;
   L0 = r2;
   push r3;
   pop B0;

   // Advance the read pointer
   r7 = M[FP + 4*ADDR_PER_WORD];        // offset input
   r7 = r7 + r1;
   r1 = r7 AND 0x1;                     // get the new offset
   r7 = r7 LSHIFT -1;                   // get the words
   r7 = r7 LSHIFT LOG2_ADDR_PER_WORD;   // get the octets from words - only 16bit unpacked
   M3 = r7;
   r0 = M[I0, M3];  // advancing the read pointer.


   r7 =  M[FP + 3*ADDR_PER_WORD];              // copy amount
   Null = r1;                                  // r1 is the read offset

   if Z jump copy_unpack_from_offset_no_read_offset;
   // read the first byte
   r5 = M[I0, ADDR_PER_WORD];
   r5 = r5 AND 0xff;
   M[I4, ADDR_PER_WORD] = r5;
   r7 = r7 - 1;                                 // one octet copied so subtract it

   copy_unpack_from_offset_no_read_offset:

   r10 = r7 LSHIFT -1;                          // translate to words

   r0 = 0xFF;                            // set r0 to a constant for masking the bottom octets
   r1 = M[I0, M1];                       // read the first word

   do copy_unpack_from_offset_loop;
      r2 = r1 LSHIFT -8;                // set r2 as the higher octet from the read word
      r1 = r1 AND r0,                   // set r1 as the lower octet from the read word
       M[I4, M1] = r2;                  // write the higher octet to the memory

      M[I4, M1] = r1,                   // write the lower octet to the memory
       r1 = M[I0, M1];                  // read the next word
   copy_unpack_from_offset_loop:

   r1 = M[I0, M2];                      // compensate for the additional read

   // write the carry over if neccessary
   Null = r7 AND 0x1;
   if Z jump copy_unpack_from_offset_copy_done;

   r2 = M[I0, 0];
   r2 = r2 LSHIFT -8;
   M[I4, ADDR_PER_WORD] = r2;

   copy_unpack_from_offset_copy_done:

   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r3, r5, r7, rLink>;
   rts;

// extern void unpack_cbuff_to_array_32bit(int *dest, tCbuffer *src, unsigned int amount);
// Additional inputs:
// M1 = ADDR_PER_WORD
// M2 = -ADDR_PER_WORD
// trashed r3
$_unpack_cbuff_to_array_32bit:
   // save the input paramerters for later
   pushm <FP(=SP), r0, r1, r2, r3, r5, rLink>;
   pushm <I0, I4, L0, L4>;

   // uset the dest as a buffer with size of the amount
   I4 = r0;
   r2 = r2 LSHIFT LOG2_ADDR_PER_WORD;
   L4 = r2;
   push r0;
   pop B4;

   // get src buffer read address and size
   r0 = r1;                // cbuffer_src
   // get the read address and save it to the stack
   call $cbuffer.get_read_address_ex;
   I0 = r0;
   L0 = r2;
   push r3;
   pop B0;

   r3 = 0x3;
   r10 =  M[FP + 3*ADDR_PER_WORD];    // copy amount
   r0 = 0xFF;
   do copy_unpack_loop_32bit;
       r5 = r3 - r1;
       r5 = r5 LSHIFT 3;
       r5 = -r5;                      // shift amount for source word

       r2 = M[I0, 0];
       r2 = r2 LSHIFT r5;             // shift octet left to LS position
       r1 = r1 + 1;
       Null = ADDR_PER_WORD - r1;
       if NZ jump continue_masking;
           r1 = 0;
           r5 = M[I0, M1];            // advance one word

       continue_masking:
       r2 = r2 AND r0;

       M[I4, M1] = r2;

   copy_unpack_loop_32bit:

   // Update the read address
   r0 = M[FP + 2*ADDR_PER_WORD];      // cbuffer_src
   r2 = r1;                           // offset
   r1 = I0;
   call $cbuffer.set_read_address_ex;

   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r3, r5, rLink>;
   rts;

// extern void unpack_cbuff_to_array_from_offset_32bit
// (int *dest, tCbuffer *src, unsigned int amount_to_copy, unsigned offset);
// M1 = ADDR_PER_WORD
// M2 = -ADDR_PER_WORD
$_unpack_cbuff_to_array_from_offset_32bit:
   // save the input paramerters for later
   pushm <FP(=SP), r0, r1, r2, r3, r5, r7, rLink>;
   pushm <I0, I4, L0, L4>;

   // use the dest as a buffer with size of the amount
   I4 = r0;
   r2 = r2 LSHIFT LOG2_ADDR_PER_WORD;
   L4 = r2;
   push r0;
   pop B4;

   // get src buffer read address and size
   r0 = r1;                           // cbuffer_src
   // get the read address and save it to the stack
   call $cbuffer.get_read_address_ex;
   I0 = r0;
   L0 = r2;
   push r3;
   pop B0;


   // Advance the read pointer
   r7 = M[FP + 4*ADDR_PER_WORD];      // offset input
   r7 = r7 + r1;

   r3 = 0x3;
   r1 = r7 AND r3;                    // get the new offset

   r7 = r7 AND 0xFFFFFFFC;            // remove the offset from the octets
   M3 = r7;
   r0 = M[I0, M3];                    // advancing the read pointer.

   r10 =  M[FP + 3*ADDR_PER_WORD];    // copy amount
   r0 = 0xFF;
   do copy_unpack_from_offset_loop_32bit;
       r5 = r3 - r1;
       r5 = r5 LSHIFT 3;
       r5 = -r5;                      // shift amount for source word

       r2 = M[I0, 0];
       r2 = r2 LSHIFT r5;             // shift octet left to LS position
       r1 = r1 + 1;
       Null = ADDR_PER_WORD - r1;
       if NZ jump continue_masking_from_offset;
           r1 = 0;
           r5 = M[I0, M1];

       continue_masking_from_offset:
       r2 = r2 AND r0;

       M[I4, M1] = r2;

   copy_unpack_from_offset_loop_32bit:

   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r3, r5, r7, rLink>;
   rts;

.ENDMODULE;

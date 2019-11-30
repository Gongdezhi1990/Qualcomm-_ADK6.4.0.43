// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
// *****************************************************************************




// *****************************************************************************
// NAME:
//    Memory operations
//
// DESCRIPTION:
//    This provides a set of functions that abstract certain memory operations on
//    the a7da_kas platform.      
//       $mem.ext_window_unpack_to_ram
//       $mem.ext_window_copy_to_ram
//       $_mem_unpack16
//
// *****************************************************************************

#include "stack.h"
#include "const_data_asm.h"


// *****************************************************************************
// MODULE:
//    $mem.ext_window_unpack_to_ram
//
//
// DESCRIPTION:
//    Will copy data "FORMAT_16BIT_SIGN_EXT" to RAM.
//
//
// INPUTS:
//    - r0 = address of data block to be copied
//    - r1 = size of data block in destination RAM
//    - r2 = destination address
//
// OUTPUTS:
//    - r0 = result:  (r0==0) FAILED, else PASSED 
//
// TRASHED REGISTERS:
//    r0, r1, r2
//
// *****************************************************************************

// *****************************************************************************
// MODULE:
//    $mem.ext_window_copy_to_ram
//
// DESCRIPTION:
//    Will copy data "FORMAT_PACKED16" to RAM.
//
// INPUTS:
//    - r0 = address of data block to be copied
//    - r1 = size of data block in destination RAM
//    - r2 = destination address
//
// OUTPUTS:
//    - r0 = result:  (r0==0) FAILED, else PASSED 
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3
//
// *****************************************************************************

.CONST $mem_a27d_local.FP_OFFSET               2*ADDR_PER_WORD;  // FP, rLink

.MODULE $M.mem.ext_window_unpack_to_ram;
   .CODESEGMENT EXT_DEFINED_PM;

$mem.ext_window_copy_to_ram:
   r3 = DEFINE_CONST_TYPE_FORMAT(MEM_TYPE_DMCONST_WINDOWED16,FORMAT_16BIT_SIGN_EXT);
   jump mem.ext_window_transfer;

$mem.ext_window_unpack_to_ram:
   r3 = DEFINE_CONST_TYPE_FORMAT(MEM_TYPE_DMCONST_WINDOWED16,FORMAT_PACKED16);
   // jump mem.ext_window_transfer;


mem.ext_window_transfer:
   // make stack frame for above local vars in conjunction with saving FP & rLink
   pushm <FP(=SP), rLink>, SP = SP + (DSC_SIZE * ADDR_PER_WORD);

   // Protect Registers calling into C
   pushm <rMAC,r10>; 
   pushm <I3, I7>;                    
   pushm <M0, M1, M2, L0, L1, L4, L5>;
   M0 = 0;M1 = ADDR_PER_WORD;M2 = -ADDR_PER_WORD;
   L0 = 0;L1 = 0;L4 = 0;L5 = 0;
   r10 = M[$ARITHMETIC_MODE];          
   push r10;                           
   M[$ARITHMETIC_MODE] = NULL;

   // Setup memory Descriptor
   M[FP + ($mem_a27d_local.FP_OFFSET+0)] = r3;    // type_format
   M[FP + ($mem_a27d_local.FP_OFFSET+1)] = r0;   // address
   M[FP + ($mem_a27d_local.FP_OFFSET+2)] = NULL;
   M[FP + ($mem_a27d_local.FP_OFFSET+3)] = NULL;

   r0 = FP + $mem_a27d_local.FP_OFFSET;      // r0 = source ptr
                                             // r2 = destination address
   r3 = r1;                                  // r3 = size
   r1 = 0;                                   // r1 = offset
   call $_const_data_copy;

   // Restore registers after C call
   pop r10;                           
   M[$ARITHMETIC_MODE] = r10;         
   popm <M0, M1, M2, L0, L1, L4, L5>;
   popm <I3, I7>;                    
   popm <rMAC, r10>; 

   // Release local variable
   SP = SP - (DSC_SIZE* ADDR_PER_WORD), popm <FP, rLink>;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_mem_unpack16
//
//    void mem_unpack16(unsigned *src,unsigned size,unsigned *dest);
//
// DESCRIPTION:
//    Given the address of a segment variable and its size this routine
// will copy and unpack 24-bit data to a place in data memory ram.
//
// 24-bit data is packed as follows:
//    addr 0:  (word 0) bits 23-8
//    addr 1:  (word 0) bits 7-0    (word 1) bits 23-16
//    addr 2:  (word 1) bits 15-0
//    addr 3:  (word 2) bits 23-8
//    addr 4:  (word 2) bits 7-0    (word 3) bits 23-16
//    addr 5:  (word 3) bits 15-0
//    ...
//    etc
//
// INPUTS:
//    - r0 = address of source data
//    - r1 = size of variable in RAM
//    - r2 = address to copy data to in DM
//
// OUTPUTS:
//    - r0 = last word unpacked
//
// TRASHED REGISTERS:
//    r0,r1, r2,r3, r10, DoLoop
//
// NOTE: Routine follows C calling convention for register usage
//
// *****************************************************************************
.MODULE $M.mem.mem_unpack16;
   .CODESEGMENT EXT_DEFINED_PM;

   $_mem_unpack16:   

   pushm <I0,I1>;

   // set input and output pointers
   I0 = r2;
   I1 = r0;

   r2  = r1 AND 1;        // is the output an odd size
   r10 = r1 ASHIFT - 1;   // r4 = num pairs of words to output

   // copy data from "external" memory to DM and pack into 24-bit words
   do loop;
      // read MSB 0 (0-15)
      r0 = M[I1,1];
      // read LSB 0 (8-15), MSB 1 (0-7)
      r1 = M[I1,1];

      // -- reassemble 1st 24bit word --
      r0 = r0 LSHIFT 8;
      r3 = r1 LSHIFT -8;

      // mask off possible sign extension of reads
      r3 = r3 AND 0xFF;
      r3 = r3 OR r0,
      r0 = M[I1,1]; // read LSB 1 (0-15)

      // -- reassemble 2nd 24bit word --
      // mask off possible sign extension of reads
      r0 = r0 AND 0xFFFF;
      r1 = r1 LSHIFT 16;
      r0 = r0 + r1,
      M[I0,1] = r3;       // store 1st 24bit word

      // store 2nd 24bit word
      M[I0,1] = r0;
   loop:

   // if number of samples to be read is odd, read the last sample separately
   Null = r2;
   if Z jump jp_unpack_done;
      // handle non-paired output
      r0 = M[I1,1];
      r1 = M[I1,1];
      r0 = r0 LSHIFT 8;
      r1 = r1 LSHIFT -8;
      r1 = r1 AND 0xFF;
      r0 = r1 OR r0;
      M[I0, 1] = r0;
jp_unpack_done:

   popm <I0,I1>;
   rts;

.ENDMODULE;


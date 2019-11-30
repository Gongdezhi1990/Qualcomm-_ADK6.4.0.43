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
//    the hydra platform.      
//       $mem.ext_window_unpack_to_ram
//       $mem.ext_window_copy_to_ram
//
// *****************************************************************************


#include "architecture.h"
#include "stack.h"
#include "portability_macros.h"

// *****************************************************************************
// MODULE:
//    $mem.ext_window_unpack_to_ram
//
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
//    r0, r1, r2
//
// *****************************************************************************
.MODULE $M.mem.ext_window_unpack_to_ram;
   .CODESEGMENT EXT_DEFINED_PM;

   $mem.ext_window_unpack_to_ram:
   $_mem_ext_window_unpack_to_ram:

   pushm <r3, r4, r5, r6, r10, rLink>;
   pushm <I0, I1>;


   // Amber case: well, here it can take the source table address as is, no window mapping - 
   I0 = r2;
   call $flash.copy_to_dm_24_nonwindowed;



   popm <I0, I1>;
   popm <r3, r4, r5, r6, r10, rLink>;
   r0 = 1;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $mem.ext_window_copy_to_ram
//
// DESCRIPTION:
//    Will copy data "FORMAT_16BIT_SIGN_EXT" to RAM.
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
.MODULE $M.mem.ext_window_copy_to_ram;
   .CODESEGMENT EXT_DEFINED_PM;

   $mem.ext_window_copy_to_ram:

   // TODO: at the moment all tables are in an ext mem window that is without sign extension.
   // So for Hydra build, WHILE this is the case, use a straight copy function that does
   // a sign extension of each copied 16-bit value. Later when Kymera build environment
   // sorts out all 3 ext mem windows and possibly someone decides to put such unpacked tables in
   // a sign-extended window, then one can do a straight copy.

   pushm <r10, rLink>;
   pushm <I0, I4>;

   I0 = r2;
   call $flash.copy_to_dm_signext;

   popm <I0, I4>;
   popm <r10, rLink>;

   r0 = 1;
   rts;

.ENDMODULE;

#if defined(CHIP_BASE_CRESCENDO) || defined (CHIP_NAPIER)
// Stub function may be of general use on Hydra, but limit to Crescendo for now.

// *****************************************************************************
// MODULE:
//    $mem.ext_window_access_as_ram
//
// DESCRIPTION:
//    NULL function on Crescendo. On other platforms maps a variable into a
// window for access.
//
// INPUTS:
//    - r0 = address of variable to be mapped
//    - r1 = size of variable 
//
// OUTPUTS:
//    - r0 = same as input r0 (address in the window on other platforms)
//    - r1 = same as input r1 (on other platforms <= input r1)
//
// TRASHED REGISTERS:
//    None.  Normal implementation trashes r2,r3
//
//
// *****************************************************************************
.MODULE $M.mem.ext_window_access_as_ram;
   .CODESEGMENT EXT_DEFINED_PM;

$mem.ext_window_access_as_ram:
   rts;

.ENDMODULE;
#endif

// *****************************************************************************
// MODULE:
//    $flash.copy_to_dm_24_nonwindowed
//
// DESCRIPTION:
//    Given the address of a 'flash segment' variable and its size this routine
// will copy and unpack 24-bit data to a place in data memory ram. It does it without
// windowing, hence it is to be used only in Hydra / Amber case. TODO: once there is
// more differentiation between chips within Hydra, make sure the platform & chip conditioning
// is updated.
//
// 24-bit data in flash is packed as follows:
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
//    - r0 = address of variable in 'flash segment'
//    - r1 = size of variable in RAM
//    - I0 = address to copy flash data to in DM
//
// OUTPUTS:
//    - r0 = last word read from flash (useful if you're just reading 1 word)
//    - I0 = end address of copied data + 1
//
// TRASHED REGISTERS:
//    r1, r3, r4, r5, r6, I1, r10, DoLoop
//
// NOTE: This routine is only available on Hydra
//
// *****************************************************************************
.MODULE $M.flash.copy_to_dm_24_nonwindowed;
   .CODESEGMENT EXT_DEFINED_PM;

   $flash.copy_to_dm_24_nonwindowed:

#ifdef K32 
   // Unpacked data for 32-bit Kalimba 
   // so just copy it        
   
   I1 = r0;
   r10 = r1;
   do loop;
      r0 = M[I1, ADDR_PER_WORD];
      M[I0, ADDR_PER_WORD] = r0;
   loop:

#else // K32

   // Data needs to be unpacked
   r6 = r0;       // "flash" address, we'll keep it also in r6 as it will get updated
   r5 = r1 AND 1; // is size in RAM an odd number of 24-bit words?
   r4 = r1 ASHIFT - 1;// r4 = (even part of size in 24-bit word RAM) / 2

   r1 = r4 * 3 (int);

   // make sure even number of data is read each time. Set counter to number of
   // even pairs of 16-bit words to be read from "flash". Initialise the source address
   // to the unwindowed "flash" address we received in R0.
   I1 = r0;
   r10 = r4;

   // update flash address we keep in a jar, it will be address after the
   // set of even pairs of words. For any odd case (literally), we shall do one more
   // compacting from this address onwards.
   r0 = r10 * 3 (int);
   r6 = r6 + r0;

   // copy data from "flash" to DM and pack into 24-bit words
   do loop;
      // read MSB 0 (0-15)
      r0 = M[I1,1];
      // read LSB 0 (8-15), MSB 1 (0-7)
      r1 = M[I1,1];

      // -- reassemble 1st 24bit word --
      r0 = r0 LSHIFT 8;
      r3 = r1 LSHIFT -8;

      // mask off possible sign extension of flash reads
      r3 = r3 AND 0xFF;
      r3 = r3 OR r0,
       r0 = M[I1,1]; // read LSB 1 (0-15)

      // -- reassemble 2nd 24bit word --
      // mask off possible sign extension of flash reads
      r0 = r0 AND 0xFFFF;
      r1 = r1 LSHIFT 16;
      r0 = r0 + r1,
      M[I0,1] = r3;       // store 1st 24bit word

      // store 2nd 24bit word
      M[I0,1] = r0;
   loop:

   // if number of samples to be read is odd, read the last sample separately
   Null = r5;
   if Z rts;

      // get the flash address that is after the even number of pairs we read and
      // packed in above loop. So one more to go...
      I1 = r6;
      r0 = M[I1,1];
      r1 = M[I1,1];
      r0 = r0 LSHIFT 8;
      r1 = r1 LSHIFT -8;
      r1 = r1 AND 0xFF;
      r0 = r1 OR r0;
      M[I0, 1] = r0;

#endif // K32

      rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $flash.copy_to_dm_signext
//
// DESCRIPTION:
//    Copy 16-bit data from source to destination(32-bits) and sign extend    
//
// INPUTS:
//    - r0 = address of variable in 'flash'
//    - r1 = size of variable in 'flash'
//    - I0 = address to copy flash data to in DM
//
// OUTPUTS:
//
// TRASHED REGISTERS:
//    I4, r10, DoLoop
//
// *****************************************************************************
.MODULE $M.flash.copy_to_dm_signext;
   .CODESEGMENT EXT_DEFINED_PM;

   $flash.copy_to_dm_signext:

   I4 = r0;

   r10 = r1;

   do copy_loop1;
      r0 = M[I4, MK1];
#ifndef K32
      // sign extend to 24 bits
      r0 = r0 LSHIFT 8;
      r0 = r0 ASHIFT -8;
#endif
      M[I0, MK1] = r0;
   copy_loop1:
   rts;

.ENDMODULE;



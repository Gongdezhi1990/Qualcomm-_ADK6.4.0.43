/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
// *****************************************************************************
// NAME:
//    MMU buffer access functions
//
// DESCRIPTION:
//    This is the Hydra specific part of cbuffer library (in cbuffer_asm.asm)
//    dealing with BAC local buffers.
//
//    On Hydra, sometimes an MMU buffer will have an 'auxiliary' buffer handle
//    associated with it. In that case we should use the aux handle when
//    calculating the amount of space/data in the buffer (see
//    http://wiki/AudioCPU/architecture/Buffers for more details).
//
// *****************************************************************************

#include "cbuffer_asm.h"        // for conversion macros
#ifdef KAL_ARCH4
#include "io_defs.asm"
#endif
#include "patch_asm_macros.h"

// *****************************************************************************
// MODULE:
//    $mmu.get_address_offset_from_byte_offset
//
// DESCRIPTION:
//    Reads the byte offset field in the specified MMU handle and returns the read
// or write address offset
//
// INPUTS:
//    - r0 = MMU handle address
//
// OUTPUTS:
//    - r0 = address offset
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************
.MODULE $M.mmu.get_address_offset_from_byte_offset;
   .CODESEGMENT EXT_DEFINED_PM;

   $mmu.get_address_offset_from_byte_offset:

   pushm <r1, r2, r3, rLink>;

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.get_address_offset_from_byte_offset.PATCH_ID_0, r1)

   r3 = r0; // move the MMU handle address
   // Read the buffer size and flags field
   r1 = M[r3 + $cbuffer.MMU_HANDLE_BUFFER_SIZE];
   // Read the offset value to the output register
   r0 = M[r3 + $cbuffer.MMU_HANDLE_BUFFER_BYTE_OFFSET];
#ifdef KAL_ARCH4
   // BAC handle is a virtual register with 96-bits rather than 3 separate
   // set of definitions per word. This also means some of the masks
   // for bitfields, especially the higher ones are useless.
   // Find packing mode within buffer size word
   r2 = r1 LSHIFT ($BAC_BUFFER_SIZE_POSN-$BAC_BUFFER_PACKING_MODE_POSN);
   r2 = r2 AND 0x1;
   if NZ jump pop_and_continue; // No change
      // Find sample size field position within buffer size word
      r2 = r1 LSHIFT ($BAC_BUFFER_SIZE_POSN-$BAC_BUFFER_SAMPLE_SIZE_POSN);
      // Use 32-bit enum as mask.
      r2 = r2 AND $BAC_BUFFER_SAMPLE_32_BIT_ENUM;
      Null = r2 - $BAC_BUFFER_SAMPLE_32_BIT_ENUM;
      if EQ jump pop_and_continue; // 32-bit samples are same as packed.
      Null = r2 - $BAC_BUFFER_SAMPLE_24_BIT_ENUM;
      if EQ jump divide_by_3; // divide by 3 is special
         r2 = Null - r2; // Get ready to right shift
         r0 = r0 LSHIFT r2; // divide offset to get number of samples
         jump adjust_offset;
      divide_by_3:
         // Easy way to divide by 3 on 32-bit platforms.
         r1 = 0x55555556;
         push rMAC;
         rMAC = r0 * r1 (int);
         r0 = rMAC LSHIFT -32;
         pop rMAC;
   adjust_offset:
      r0 = r0 * ADDR_PER_WORD (int);
#endif
   pop_and_continue:
#ifndef KAL_ARCH4
   // Base address here is from MMU handle, which is byte address. So is the offset.
   // Convert byte address into word address for 24b/24b only
   r0 = r0 ASHIFT -1;
#endif

   // return address offset in r0;
   popm <r1, r2, r3, rLink>;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $mmu.set_byte_offset_from_address
//
// DESCRIPTION:
//    Sets the byte offset field in the specified MMU handle given the read
// or write offset address
//
// INPUTS:
//    - r0 = pointer to cbuffer structure wrapping the mmu buffer
//    - r1 = offset address
//    - r2 = MMU handle address
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************
.MODULE $M.mmu.set_byte_offset_from_address;
   .CODESEGMENT EXT_DEFINED_PM;

   $mmu.set_byte_offset_from_address:

#ifdef KAL_ARCH4
   pushm <r3, r4>;

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.set_byte_offset_from_address.PATCH_ID_0, r3)

   // Read the buffer size and flags field
   r3 = M[r2 + $cbuffer.MMU_HANDLE_BUFFER_SIZE];
   // BAC handle is a virtual register with 96-bits rather than 3 separate
   // set of definitions per word. This also means some of the masks
   // for bitfields, especially the higher ones are useless.
   // Find packing mode within buffer size word
   r4 = r3 LSHIFT ($BAC_BUFFER_SIZE_POSN-$BAC_BUFFER_PACKING_MODE_POSN);
   r4 = r4 AND 0x1;
   if NZ jump pop_and_continue; // No change for packed buffers
      // Find sample size field position within buffer size word
      r4 = r3 LSHIFT ($BAC_BUFFER_SIZE_POSN-$BAC_BUFFER_SAMPLE_SIZE_POSN);
      // Use 32-bit enum as mask.
      r4 = r4 AND $BAC_BUFFER_SAMPLE_32_BIT_ENUM;
      Null = r4 - $BAC_BUFFER_SAMPLE_32_BIT_ENUM;
      if EQ jump pop_and_continue; // 32-bit samples are same as packed.
      // Change offset to number of samples
      r1 = r1 LSHIFT -LOG2_ADDR_PER_WORD;
      // Adjust sample size to give number of octets
      // This depends that 8-bit enum value is 0, 16-bit is 1 & 24 bit is 2.
      r4 = r4 + 1;
      // offset = samples * octets_per_sample
      r1 = r1 * r4 (int);

   pop_and_continue:
   popm <r3, r4>;
#endif
   M[r2 + $cbuffer.MMU_HANDLE_BUFFER_BYTE_OFFSET] = r1;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $mmu.calc_amount_space_local_buff
//
// DESCRIPTION:
//    Calculates the amount of space for new data in a cbuffer.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//
// OUTPUTS:
//    - r0 = amount of space (for new data) in addresses (locations)
//    - r2 = buffer size in addresses (locations)
//
// TRASHED REGISTERS:
//    r1
//
// NOTES:
//    Return the amount of space (for new data) in the c/MMUbuffer.
//
// *****************************************************************************
.MODULE $M.mmu.calc_amount_space_local_buff;
   .CODESEGMENT BUFFER_PM;

   $mmu.calc_amount_space_local_buff:

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.calc_amount_space_local_buff.PATCH_ID_0, r1)

   // We also use these registers, but need to preserve them to maintain
   // compatibility with Previn libraries
   pushm <r3, r4, rLink>;
   push I7;

   I7 = M[r0 + $cbuffer.AUX_ADDR_FIELD];
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   r4 = M[r0 + $cbuffer.START_ADDR_FIELD];
   // r2 = M[r0 + $cbuffer.SIZE_FIELD];
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r0);
   r1 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
   r0 = M[r0 + $cbuffer.READ_ADDR_FIELD];

   // Work out if either of the pointers are to MMU buffers
   Null = r3 AND $cbuffer.WRITE_PTR_TYPE_MASK;
   if NE jump its_an_mmu_write_handle;
   BUFFER_MASK_WRITE_ADDR(r1);
   end_mmu_write_adjust:

   Null = r3 AND $cbuffer.READ_PTR_TYPE_MASK;
   if NE jump its_an_mmu_read_handle;
   BUFFER_MASK_READ_ADDR(r0);
   end_mmu_read_adjust:

   // calculate the amount of space
   r0 = r0 - r1;
   if LE r0 = r0 + r2;

   // always say it's one word less so that buffer never gets totally filled up
   // (read pointer == write pointer means the buffer is empty, not full!)
   r0 = r0 - 1*ADDR_PER_WORD;

   pop I7;
   popm <r3, r4, rLink>;
   rts;

   its_an_mmu_write_handle:
      Null = r3 AND $cbuffer.AUX_PTR_PRESENT_MASK;
      if EQ jump no_aux_write;
      Null = r3 AND $cbuffer.AUX_PTR_TYPE_MASK;
      if NE r1 = I7;

   no_aux_write:
   // Prepare to call function that calculates buffer offset
   push r0;
   r0 = r1;
   call $mmu.get_address_offset_from_byte_offset;
   // r0 has offset address;
   r1 = r0 + r4;
   pop r0;
   jump end_mmu_write_adjust;

   its_an_mmu_read_handle:
      Null = r3 AND $cbuffer.AUX_PTR_PRESENT_MASK;
      if EQ jump no_aux_read;
      Null = r3 AND $cbuffer.AUX_PTR_TYPE_MASK;
      if EQ r0 = I7;

   no_aux_read:
   // r0 already has the mmu handle address. Call function to find buffer offset
   call $mmu.get_address_offset_from_byte_offset;
   // r0 has offset address;
   r0 = r0 + r4;
   jump end_mmu_read_adjust;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $mmu.calc_amount_data_local_buff
//
// DESCRIPTION:
//    Calculates the amount of data already in a cbuffer.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//
// OUTPUTS:
//    - r0 = amount of data available in addresses (locations)
//    - r2 = buffer size in addresses (locations)
//
// TRASHED REGISTERS:
//    r1
//
// *****************************************************************************
.MODULE $M.mmu.calc_amount_data_local_buff;
   .CODESEGMENT BUFFER_PM;

   $mmu.calc_amount_data_local_buff:

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.calc_amount_data_local_buff.PATCH_ID_0, r1)

   // We also use these registers, but need to preserve them to maintain
   // compatibility with Previn libraries
   pushm <r3, r4, rLink>;
   pushm <I3, I7>;

   I7 = M[r0 + $cbuffer.AUX_ADDR_FIELD];
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   r4 = M[r0 + $cbuffer.START_ADDR_FIELD];
   // r2 = M[r0 + $cbuffer.SIZE_FIELD];
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r0);
   r1 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
   r0 = M[r0 + $cbuffer.READ_ADDR_FIELD];

   // Work out if either of the pointers are to MMU buffers
   Null = r3 AND $cbuffer.WRITE_PTR_TYPE_MASK;
   if NE jump its_an_mmu_write_handle;
   BUFFER_MASK_WRITE_ADDR(r1);
   end_mmu_write_adjust:

   Null = r3 AND $cbuffer.READ_PTR_TYPE_MASK;
   if NE jump its_an_mmu_read_handle;
   BUFFER_MASK_READ_ADDR(r0);
   end_mmu_read_adjust:

   // calculate the amount of data
   r0 = r1 - r0;
   if NEG r0 = r0 + r2;

   popm <I3, I7>;
   popm <r3, r4, rLink>;
   rts;

   its_an_mmu_write_handle:
      Null = r3 AND $cbuffer.AUX_PTR_PRESENT_MASK;
      if EQ jump no_aux_write;
      Null = r3 AND $cbuffer.AUX_PTR_TYPE_MASK;
      if NE r1 = I7;

   no_aux_write:
   // Prepare to call function that calculates buffer offset
   push r0;
   r0 = r1;
   call $mmu.get_address_offset_from_byte_offset;
   // r0 has offset address;
   r1 = r0 + r4;
   pop r0;
   jump end_mmu_write_adjust;

   its_an_mmu_read_handle:
      Null = r3 AND $cbuffer.AUX_PTR_PRESENT_MASK;
      if EQ jump no_aux_read;
      Null = r3 AND $cbuffer.AUX_PTR_TYPE_MASK;
      if EQ r0 = I7;

   no_aux_read:
   // r0 already has the mmu handle address. Call function to find buffer offset
   call $mmu.get_address_offset_from_byte_offset;
   // r0 has offset address;
   r0 = r0 + r4;
   jump end_mmu_read_adjust;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $mmu.get_read_address_local_buff
//
// DESCRIPTION:
//    Get a read address, size and start address for a mmu buffer
//  so that it can read in a generic way.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//
// OUTPUTS:
//    - r0 = read address
//    - r1 = buffer size in addresses (locations)
//    - r2 = buffer start address
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************
.MODULE $M.mmu.get_read_address_local_buff;
   .CODESEGMENT BUFFER_PM;

   $mmu.get_read_address_local_buff:

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.get_read_address_local_buff.PATCH_ID_0, r1)

   r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
#ifdef DEBUG_ON
   // Work out if read address is an MMU buffer
   Null = r1 AND $cbuffer.READ_PTR_TYPE_MASK;
   if Z call $error;          // private, this should not be called on SW buffers
   // recommended call $cbuffer.get_read_address_and_size_and_start_address:
#endif

   pushm <r3, rLink>;   // compatibility with legacy libraries
   r3 = M[r0 + $cbuffer.READ_ADDR_FIELD];
   // For SCO MMU buffers AUX pointer can be in use
   Null = r1 AND $cbuffer.AUX_PTR_PRESENT_MASK;
   if EQ jump no_aux_read;
      Null = r1 AND $cbuffer.AUX_PTR_TYPE_MASK;
      if NE jump no_aux_read;
         r3 = M[r0 + $cbuffer.AUX_ADDR_FIELD];

   no_aux_read:
      // r1 = M[r0 + $cbuffer.SIZE_FIELD];
      BUFFER_GET_SIZE_IN_ADDRS_ASM(r1, r0);
      r2 = M[r0 + $cbuffer.START_ADDR_FIELD];
      // Finally get address from byte offset
      r0 = r3;
      call $mmu.get_address_offset_from_byte_offset;
      r0 = r0 + r2;
      // r0 has offset address;
   popm <r3, rLink>;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $mmu.get_write_address_local_buff
//
// DESCRIPTION:
//    Get write address, size and start address for a mmu buffer
//  so that it can written in a generic way.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//
// OUTPUTS:
//    - r0 = write address
//    - r1 = buffer size in addresses (locations)
//    - r2 = buffer start address
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************
.MODULE $M.mmu.get_write_address_local_buff;
   .CODESEGMENT BUFFER_PM;

   $mmu.get_write_address_local_buff:

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.get_write_address_local_buff.PATCH_ID_10, r1)

   r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
#ifdef DEBUG_ON
   // Work out if write address is an MMU buffer
   Null = r1 AND $cbuffer.WRITE_PTR_TYPE_MASK;
   if Z call $error;          // private, this should not be called on SW buffers
   // recommended call $cbuffer.get_write_address_and_size_and_start_address:
#endif

   pushm <r3, rLink>;   // compatibility with legacy libraries
   r3 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
   // For SCO MMU buffers AUX pointer can be in use
   Null = r1 AND $cbuffer.AUX_PTR_PRESENT_MASK;
   if EQ jump no_aux_write;
      Null = r1 AND $cbuffer.AUX_PTR_TYPE_MASK;
      if EQ jump no_aux_write;
         r3 = M[r0 + $cbuffer.AUX_ADDR_FIELD];

   no_aux_write:
      // r1 = M[r0 + $cbuffer.SIZE_FIELD];
      BUFFER_GET_SIZE_IN_ADDRS_ASM(r1, r0);
      r2 = M[r0 + $cbuffer.START_ADDR_FIELD];
      // Finally get address from byte offset
      r0 = r3;
      call $mmu.get_address_offset_from_byte_offset;
      r0 = r0 + r2;
      // r0 has offset address;
   popm <r3, rLink>;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $mmu.set_read_address_local_buff
//
// DESCRIPTION:
//    Set the read address for a mmu buffer.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure wrapping the mmu buffer
//    - r1 = read address
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//   (r1   if it's a hw cbuffer)
//
// *****************************************************************************
.MODULE $M.mmu.set_read_address_local_buff;
   .CODESEGMENT BUFFER_PM;

$mmu.set_read_address_local_buff:

   push r2;       // preserve for compatibility with legacy libraries

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.set_read_address_local_buff.PATCH_ID_0, r2)

   r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
#ifdef DEBUG_ON
   // Work out if read address is an MMU buffer
   // MMU buffer will be under hardware control.
   Null = r2 AND $cbuffer.READ_PTR_TYPE_MASK;
   if Z call $error;          // private, this should not be called on SW buffers
   // recommended call $cbuffer.set_read_address:
#endif

    // For SCO MMU buffers AUX pointer can be in use,
    // but we shouldn't ever try to modify it
    Null = r2 AND $cbuffer.AUX_PTR_PRESENT_MASK;
    if EQ jump no_aux_read;
       Null = r2 AND $cbuffer.AUX_PTR_TYPE_MASK;
       if NE jump no_aux_read;
         call $error;         // panic

   no_aux_read:
       // compute the offset (in bytes)
       r2 = M[r0 + $cbuffer.START_ADDR_FIELD];
       r1 = r1 - r2;
#ifdef DEBUG_ON
       // assert the offset is not larger than the size of the cbuffer
       // r2 = M[r0 + $cbuffer.SIZE_FIELD];
       BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r0);
       Null = r1 - r2;
       if LT jump offset_is_valid;
         call $error;         // offset larger than the size of the buffer
   offset_is_valid:
#endif
#ifndef KAL_ARCH4
       // r1 has offset in word address.
       // Convert word address into byte address for 24b/24b only
       r1 = r1 LSHIFT 1;
#endif
       push rLink;
       r2 = M[r0 + $cbuffer.READ_ADDR_FIELD];
       call $mmu.set_byte_offset_from_address;
       pop rLink;
       pop r2;
       rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbuffer.set_write_address_local_buff
//
// DESCRIPTION:
//    Set the write address for a mmu buffer.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//    - r1 = write address
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//   (r1   if it's a hw cbuffer)
//
// *****************************************************************************
.MODULE $M.mmu.set_write_address_local_buff;
   .CODESEGMENT BUFFER_PM;

$mmu.set_write_address_local_buff:

   push r2;       // preserve for compatibility with legacy libraries

   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.set_write_address_local_buff.PATCH_ID_0, r2)

   r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
#ifdef DEBUG_ON
   // Work out if write address is an MMU buffer
   // MMU buffer will be under hardware control.
   Null = r2 AND $cbuffer.WRITE_PTR_TYPE_MASK;
   if Z call $error;          // private, this should not be called on SW buffers
   // recommended call $cbuffer.set_write_address:
#endif

    // For SCO MMU buffers AUX pointer can be in use,
    // but we shouldn't ever try to modify it
    Null = r2 AND $cbuffer.AUX_PTR_PRESENT_MASK;
    if EQ jump no_aux_write;
       Null = r2 AND $cbuffer.AUX_PTR_TYPE_MASK;
       if EQ jump no_aux_write;
          call $error;         // panic

   no_aux_write:
       // compute the offset (in bytes)
       r2 = M[r0 + $cbuffer.START_ADDR_FIELD];
       r1 = r1 - r2;
#ifdef DEBUG_ON
       // assert the offset is not larger than the size of the cbuffer
       // r2 = M[r0 + $cbuffer.SIZE_FIELD];
       BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r0);
       Null = r1 - r2;
       if LT jump offset_is_valid;
          call $error;     // offset larger than the size of the cbuffer
   offset_is_valid:
#endif
#ifndef KAL_ARCH4
       // r1 has offset in word address.
       // Convert word address into byte address for 24b/24b only
       r1 = r1 LSHIFT 1;
#endif
       push rLink;
       r2 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
       call $mmu.set_byte_offset_from_address;
       pop rLink;
       pop r2;
       rts;
.ENDMODULE;



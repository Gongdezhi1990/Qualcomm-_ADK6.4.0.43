// *****************************************************************************
// Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
// *****************************************************************************

// *****************************************************************************

//----Summary----//
// MMU Buffer (_ex) library functions supporting octet access.
// *****************************************************************************
#ifdef INSTALL_CBUFFER_EX
#include "cbuffer_asm.h"
#include "io_defs.asm"
#include "patch/patch_asm_macros.h"

//MMU buffer write offsets are never rounded up.
//All functions in this file accept and return
//true write and read addresses.

.MODULE $M.mmu.get_address_offset_from_byte_offset_ex;
   .CODESEGMENT BUFFER_PM;
// in r0 mmu handle address
// out r0 word address
//     r1 octet offset
// trashed none
// only works for 8, 16 and 32 bit unpacked modes
// does not work for 24-bit unpacked mode and other packed modes
$mmu.get_address_offset_from_byte_offset_ex:
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.get_address_offset_from_byte_offset_ex.PATCH_ID_0, r1)
   pushm <r2, r3, rLink>;
   r3 = r0; // move the MMU handle address
   // Read the buffer size and flags field
   r1 = M[r3 + $cbuffer.MMU_HANDLE_BUFFER_SIZE];
   // Read the offset value to the output register
   r0 = M[r3 + $cbuffer.MMU_HANDLE_BUFFER_BYTE_OFFSET];
   // Find sample size field position within buffer size word
   r2 = r1 LSHIFT ($BAC_BUFFER_SIZE_POSN-$BAC_BUFFER_SAMPLE_SIZE_POSN);
   r2 = r2 AND $BAC_BUFFER_SAMPLE_32_BIT_ENUM;
   r1 = r0 AND r2;
   // Use 32-bit enum as mask.
   Null = r2 - $BAC_BUFFER_SAMPLE_32_BIT_ENUM;
   if EQ jump pop_and_continue; //32-bit
   r2 = Null - r2; // Get ready to right shift
   r0 = r0 LSHIFT r2; // divide offset to get number of samples
   r0 = r0 * ADDR_PER_WORD (int);
   pop_and_continue:

   // return offset address in r0;
   popm <r2, r3, rLink>;
   rts;
.ENDMODULE;

.MODULE $M.mmu.get_read_address_local_buff_ex;
   .CODESEGMENT BUFFER_PM;
// in r0 cbuffer address
// out r0 read address word aligned
//     r1 read octet offset
//     r2 size in locations
//     r3 base address
// trashed none
$mmu.get_read_address_local_buff_ex:
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.get_read_address_local_buff_ex.PATCH_ID_0, r1)
   push rLink;
   r2 = r0;
   r3 = M[r0 + $cbuffer.READ_ADDR_FIELD];
   // Finally get address from byte offset
   r0 = r3;
   call $mmu.get_address_offset_from_byte_offset_ex;
   //r0, r1 has read address offset and read octet offset
   r3 = M[r2 + $cbuffer.START_ADDR_FIELD];
   r0 = r0 + r3;
   // r2 = M[r2 + $cbuffer.SIZE_FIELD];
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r2);
   pop rLink;
   rts;
.ENDMODULE;

.MODULE $M.mmu.get_write_address_local_buff_ex;
   .CODESEGMENT BUFFER_PM;
// in r0 cbuffer address
// out r0 write address word aligned, rounded up
// out r1 write octet offset
// out r2 size in locations
// out r3 base address
// trashed none
$mmu.get_write_address_local_buff_ex:
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($mmu.get_write_address_local_buff_ex.PATCH_ID_0, r1)
   push rLink;
   r2 = r0;
   r3 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
   // Finally get address from byte offset
   r0 = r3;
   call $mmu.get_address_offset_from_byte_offset_ex;
   //r0, r1 has write address offset and write octet offset
   r3 = M[r2 + $cbuffer.START_ADDR_FIELD];
   r0 = r0 + r3;
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r2);
   pop rLink;
   rts;
.ENDMODULE;

.MODULE $M.mmu.set_byte_offset_from_address_ex;
   .CODESEGMENT BUFFER_PM;
// in r0 mmu handle
// in r1 offset in locations
// in r2 octet offset
// trashed none
// only works for 8, 16 and 32 bit unpacked modes
// does not work for 24-bit unpacked mode and other packed modes
$mmu.set_byte_offset_from_address_ex:
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($mmu.set_byte_offset_from_address_ex.PATCH_ID_0)
   pushm <r3, r4>;
   // Read the buffer size and flags field
   r3 = M[r0 + $cbuffer.MMU_HANDLE_BUFFER_SIZE];
   r4 = r3 LSHIFT ($BAC_BUFFER_SIZE_POSN-$BAC_BUFFER_SAMPLE_SIZE_POSN);
   // Use 32-bit enum as mask.
   r4 = r4 AND $BAC_BUFFER_SAMPLE_32_BIT_ENUM;
   Null = r4 - $BAC_BUFFER_SAMPLE_32_BIT_ENUM;
   if EQ jump pop_and_continue; // 32-bit
   // Change offset to number of samples
   r1 = r1 LSHIFT -LOG2_ADDR_PER_WORD;
   // Adjust sample size to give number of octets
   // This depends that 8-bit enum value is 0, 16-bit is 1
   r4 = r4 + 1;
   // offset = samples * octets_per_sample
   r1 = r1 * r4 (int);
   pop_and_continue:
   popm <r3, r4>;
   r1 = r1 OR r2;
   M[r0 + $cbuffer.MMU_HANDLE_BUFFER_BYTE_OFFSET] = r1;
   rts;
.ENDMODULE;
#endif /* INSTALL_CBUFFER_EX */

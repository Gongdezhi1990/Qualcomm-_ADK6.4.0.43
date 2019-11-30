// *****************************************************************************
// Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
// *****************************************************************************

                     //----Summary----//
// Cbuffer (_ex) library functions supporting octet access. This cbuffer library
// extension can track octets within a word. Not all cbuffer functions are extended.

                     //----What's supported?----//
// 16-bit unpacked MMU and Software buffers are supported.
// Only supports 32-bit KAL_ARCH4 platform.
// AUX pointers are not supported.

                     //----Read pointers----//
// For Software read pointers, the number of octets read within a word are stashed in the 2 LSBs
// of the read pointer. We support only 16-bit unpacked and in this case there can only be one odd octet.
// We use 2 LSBs so we are equipped to support 32-bit at a later time.
// The SW read pointer is not incremented until all octets in the word are read.
// The SW read pointer always points to an unread or a partially read word.
// For MMU read pointers (handles), the octet information is available in the BAC buffer offset and
// the read pointer (that points to an MMU handle) in the cbuffer structure is not touched.
// The _ex cbuffer and the standard cbuffer functions always a rounded down version of the read pointer.
// The _ex cbuffer functions additionally return octet information (in a separate register).

                     //----Write pointers----//
// For Software write pointers, the number of octets written into a word are stashed in the 2 LSBS of
// the write pointer. If a partial word is written the write pointer is immediately rounded up
// (incremented by ADDR_PER_WORD). Any further odd octets written to to the same word will
// keep the write pointer pointing to the same (rounded up) location. The SW write pointer always
// points to the next whole word to write to.
// For MMU write pointers (handles) the octet information is available in the BAC buffer offset.
// The internal BAC offsets are obviously not rounded up as these offsets can be used by entities
// outside the audio subsystem.
// Note that the _ex cbuffer functions always return the truth.
// For example, $cbuffer.get_write_address_and_size_and_start_address_ex) will return
// the true (not rounded up) write pointer. Octet information is returned separately (for example, in a separate register).
// For SW write pointers the standard cbuffer functions ($cbuffer.get_write_address_and_size_and_start_address)
// will return the rounded up version of the write pointer. Obviously no octet information
// is returned. This is so codecs such as SBC that use the standard cbuffer functions can see that
// there is enough data in their input. They may see some extra garbage octets too (maximum of 3 garbage
// octets in the 32-bit unpacked case, maximum of 1 garbage octet in the 16-bit unpacked case).
// These libaries are self-framing and they normally see whole packets. They are expected to start decode only
// when all data required for a frame is available. Therefore, seeing extra garbage octets due to a rounded up software
// write pointer is generally not an issue. Further we only tend to receive odd octets at the end of the stream.
// We normally expect the apps DMA engine to transfer data in multiples of 2 (or possibly even powers of 2) chunks.
// Therefore we don't expect the problem of interpreting a garbage octet to hit us.

                  //----Misc----//
// The standard cbuffer functions will mask out the 2 LSBs from the Software read and the Software write pointers.
// It is important further changes to the standard cbuffer functions (that can operate on octet buffers) mask out
// the LSBs. Macros for these have been provided in cbuffer_asm.h (BUFFER_GET/MASK_READ/WRITE_ADDR)

                  //----Aberrations----//
// The standard cbuffer functions won't return rounded up write pointers for the MMU write handles.
// We don't expect MMU buffers to be connected to libraries directly so this is generally not an issue.
// This aberration is because of factors not under our control.
// For pure software buffers  (both read and write pointers are software) it is possible
// to maintain an invariant that the write and read pointers are at least one word apart so one can differentiate
// a buffer full and a buffer empty condition. For example a 16-bit unpacked buffer of 128 words can in theory hold 255 octets.
// It is possible to limit the maximum data in this buffer to 254 octets (by making calc_amount_space_ex return
// one word (2 octets) less than the actual space available). This will prevent the buffer holding 255 octets and will
// therefore prevent the write pointer being rounded (so the buffer won't appear empty when it is actually full).
// If the write pointer is an MMU handle written by an external entity it is not possible to maintain this invariant.
// The 16-bit unpacked buffer discussed above can hold 255 octets. This is what prevents the standard cbuffer functions returning
// a rounded up  write pointer when there is an underlying MMU handle.
// This also results in calc_amount_space_ex returning space - 1 (1 - 1 = 0) when the buffer has 255 octets worth data
// rather than the usual space - 2 (which will otherwise be a negative number).

                  //----Functions callable from C and ASM ----//
// $M.cbuffer.get_read_address_ex(r0 = cbuffer address, r1 = read octet offset pointer)
// returns read-address (r0) and read octet offset written into address pointed to by offset

// $M.cbuffer.get_write_address_ex(r0 = cbuffer address, r1 = write octet offset pointer)
// returns true write-address (r0) and write octet offset written into address pointed to by offset

// $M.cbuffer.set_read_address_ex(r0 = cbuffer address, r1 = true read address, r2 = read octet offset)
// returns void

// $M.cbuffer.set_write_address_ex(r0 = cbuffer address, r1 = true write address, r2 = write octet offset)
// returns void

// $M.cbuffer.calc_amount_data_ex(r0 = cbuffer address)
// returns amount of data in octets

// $M.cbuffer.calc_amount_space_ex
// returns amount of space in octets - 2, clipped to 0

// $M.cbuffer_advance_write_ptr_ex(r0 = cbuffer address, r1 = octets to advance by)
// returns void

//$M.cbuffer_advance_read_ptr_ex(r0 = cbuffer address, r1 = octets to advance by)
// returns void

                  //----Functions callable only from ASM----//
//$M.cbuffer.get_read_address_and_size_and_start_address_ex(r0 = cbuffer address)
// returns true true read address (r0), read octet offset (r1), size in locations (r2), base (r3)

//$M.cbuffer.get_write_address_and_size_and_start_address_ex(r0 = cbuffer address)
// returns true write address (r0), write octet offset (r1), size in locations (r2), base (r3)

// ***********************************************************************************************************************

#ifdef INSTALL_CBUFFER_EX
#include "cbuffer_asm.h"
#include "io_defs.asm"
#include "patch/patch_asm_macros.h"

.MODULE $M.cbuffer.get_read_address_and_size_and_start_address_ex;
   .CODESEGMENT BUFFER_PM;
// in r0 cbuffer address
// out r0 true read address word aligned
//     r1 octet offset
//     r2 size in addresses
//     r3 buffer start address
// trashed - none
$cbuffer.get_read_address_and_size_and_start_address_ex:
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.get_read_address_and_size_and_start_address_ex.PATCH_ID_0, r1)
   r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_rd_ptr;

   Null = r1 AND $cbuffer.READ_PTR_TYPE_MASK;
   if Z jump its_a_sw_rd_ptr;

   jump $mmu.get_read_address_local_buff_ex;

its_a_sw_rd_ptr:
   r3 = M[r0 + $cbuffer.START_ADDR_FIELD];
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r0);
   r0 = M[r0 + $cbuffer.READ_ADDR_FIELD];
   r1 = r0 AND 0x3;
   r0 = r0 AND 0xFFFFFFFC;
   rts;
.ENDMODULE;

.MODULE $M.cbuffer.get_write_address_and_size_and_start_address_ex;
   .CODESEGMENT BUFFER_PM;
// in r0 cbuffer address
// out r0 true write address word aligned
// out r1 write octet offset
// out r2 size in locations
// out r3 base address
// trashed B0
$cbuffer.get_write_address_and_size_and_start_address_ex:
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.get_write_address_and_size_and_start_address_ex.PATCH_ID_0, r1)
   r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_wr_ptr;
   // mmu only if it's not a SW pointer
   Null = r1 AND $cbuffer.WRITE_PTR_TYPE_MASK;
   if Z jump its_a_sw_wr_ptr;
   // write address is an MMU buffer
   jump $mmu.get_write_address_local_buff_ex;
its_a_sw_wr_ptr:
   r3 = M[r0 + $cbuffer.START_ADDR_FIELD];
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r0);
   r0 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
   r1 = r0 AND 0x3;
   if Z jump no_round_down;
   r0 = r0 AND 0xFFFFFFFC;
   pushm <I0, L0>;
   I0 = r0;
   L0 = r2;
   push r3;
   pop B0;
   r0 = M[I0, -ADDR_PER_WORD];
   r0 = I0;
   popm <I0, L0>;
no_round_down:
   rts;
.ENDMODULE;

//extern unsigned int * cbuffer_get_read_address_ex(tCbuffer * cbuffer, unsigned * offset);
.MODULE $M.cbuffer.get_read_address_ex;
   .CODESEGMENT BUFFER_PM;
//in r0 cbuffer address
//in r1 pointer to octet offset
//out r0 read address
//trashed r2, r3
$cbuffer.get_read_address_ex:
$_cbuffer_get_read_address_ex:
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($cbuffer.get_read_address_ex.PATCH_ID_0)
   push rLink;
   push r1;
   call $cbuffer.get_read_address_and_size_and_start_address_ex;
   pop r2;
   M[r2] = r1;
   pop rLink;
   rts;
.ENDMODULE;

//extern unsigned int * cbuffer_get_write_address_ex(tCbuffer * cbuffer, unsigned * offset);
.MODULE $M.cbuffer.get_write_address_ex;
   .CODESEGMENT BUFFER_PM;
//in r0 cbuffer address
//in r1 pointer to octet offset
//out r0 true write address
//trashed r2, r3, B0
$M.cbuffer.get_write_address_ex:
$_cbuffer_get_write_address_ex:
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($cbuffer.get_write_address_ex.PATCH_ID_0)
   push rLink;
   push r1;
   call $cbuffer.get_write_address_and_size_and_start_address_ex;
   pop r2;
   M[r2] = r1;
   pop rLink;
   rts;
.ENDMODULE;

//extern unsigned cbuffer_calc_amount_data_ex(tCbuffer * cb);
.MODULE $M.cbuffer.calc_amount_data_ex;
   .CODESEGMENT BUFFER_PM;
//in  r0 cbuffer address
//out r0 available data in octets
//trashed r1, r2, r3, B0
//only supports 16-bit unpacked mode
$cbuffer.calc_amount_data_ex:
$_cbuffer_calc_amount_data_ex:
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.calc_amount_data_ex.PATCH_ID_0, r1)
   pushm <r4, r5, rLink>;
   push r0;
   call $cbuffer.get_write_address_and_size_and_start_address_ex;
   r4 = r0;
   r5 = r1;
   pop r0;
   call $cbuffer.get_read_address_and_size_and_start_address_ex;
   r3 = r4 - r0;
   if NEG r3 = r3 + r2;
   r3 = r3 LSHIFT - 1; // only support 16-bit unpacked
   if Z jump rd_equals_wr;
   r0 = r3 - r1; // subtract read octets
   r0 = r0 + r5; // add written octets
   jump end;
rd_equals_wr:
   r0 = r5 - r1;
   if NEG jump buffer_full;
   jump end;
buffer_full:
   // This is expected only if the buffer has an MMU write/read handle
   // For normal software buffers there will always be a gap of at least 2 octets
   // (considering we support only 16-bit unpacked operation). This is because how
   // much we allow to write into the buffer is completely under the control of SW
   // running on the DSP.
   // For MMU buffers written to or read by other sub-systems this gap can be just 1 octet.
   r2 = r2 LSHIFT -1; // only supports 16-bit unpacked mode
   r0 = r2 - 1;
end:
   popm <r4, r5, rLink>;
   rts;
.ENDMODULE;

//extern unsigned cbuffer_calc_amount_space_ex(tCbuffer * cb);
// only supports 16-bit unpacked mode
// return space - 2, clipped to zero
.MODULE $M.cbuffer.calc_amount_space_ex;
   .CODESEGMENT BUFFER_PM;
//in  r0 cbuffer address
//out r0 available space in octets - 1
//trashed r1, r2, r3, B0
$cbuffer.calc_amount_space_ex:
$_cbuffer_calc_amount_space_ex:
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.calc_amount_space_ex.PATCH_ID_0, r1)
   push rLink;
   push r0;
   call $cbuffer.calc_amount_data_ex;
   pop r1;
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r1);
   r2 = r2 LSHIFT - 1; // only supports 16 bit unpacked mode
   r0 = r2 - r0;
   r0 = r0 - 1;
   // When the buffer has an MMU read or write handle
   // it is possible that at this point r0 is zero. See note
   // in cbuffer.calc_amount_data_ex. Therefore we need to clip
   // the return value to zero so we don't return a negative value, ie: -1.
   if NZ r0 = r0 - 1;
   pop rLink;
   rts;
.ENDMODULE;

//extern void cbuffer_set_read_address_ex(tCbuffer * cbuffer, unsigned int * ra, unsigned ro);
.MODULE $M.cbuffer.set_read_address_ex;
   .CODESEGMENT BUFFER_PM;
//in r0 cbuffer address
//in r1 true read address
//in r2 read octet offset
//trashed none
$cbuffer.set_read_address_ex:
$_cbuffer_set_read_address_ex:
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($cbuffer.set_read_address_ex.PATCH_ID_0)
   push r3;
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_wr_ptr;
   Null = r3 AND $cbuffer.READ_PTR_TYPE_MASK;
   if Z jump its_a_sw_wr_ptr;
   push rLink;
   r3 = M[r0 + $cbuffer.START_ADDR_FIELD];
   r1 = r1 - r3;
   r0 = M[r0 + $cbuffer.READ_ADDR_FIELD];
   call $mmu.set_byte_offset_from_address_ex;
   pop rLink;
   jump end;
its_a_sw_wr_ptr:
   r1 = r1 or r2;
   M[r0 + $cbuffer.READ_ADDR_FIELD] = r1;
end:
   pop r3;
   rts;
.ENDMODULE;

//extern void cbuffer_set_write_address_ex(tCbuffer * cbuffer, unsigned int * wa, unsigned wo);
.MODULE $M.cbuffer.set_write_address_ex;
   .CODESEGMENT BUFFER_PM;
//in r0 cbuffer address
//in r1 true write address
//in r2 write octet offset
//trashed B0
$cbuffer.set_write_address_ex:
$_cbuffer_set_write_address_ex:
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($cbuffer.set_write_address_ex.PATCH_ID_0)
   push r3;
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_wr_ptr;
   Null = r3 AND $cbuffer.WRITE_PTR_TYPE_MASK;
   if Z jump its_a_sw_wr_ptr;
   push rLink;
   r3 = M[r0 + $cbuffer.START_ADDR_FIELD];
   r1 = r1 - r3;
   r0 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
   call $mmu.set_byte_offset_from_address_ex;
   pop rLink;
   jump end;
its_a_sw_wr_ptr:
   Null = r2 and 0x3;
   if Z jump no_round_up;
   pushm <I0, L0>;
   I0 = r1;
   BUFFER_GET_SIZE_IN_ADDRS_ASM(L0, r0);
   r3 = M[r0 + $cbuffer.START_ADDR_FIELD];
   push r3;
   pop B0;
   r3 = M[I0, ADDR_PER_WORD];
   r1 = I0;
   r1 = r1 or r2;
   popm <I0, L0>;
no_round_up:
   M[r0 + $cbuffer.WRITE_ADDR_FIELD] = r1;
end:
   pop r3;
   rts;
.ENDMODULE;

//extern void cbuffer_advance_write_ptr_ex(tCbuffer * cbuffer, unsigned num_octets);
.MODULE $M.cbuffer_advance_write_ptr_ex;
   .CODESEGMENT BUFFER_PM;
//in r0 cbuffer address
//in r1 octets to advance by
//trashed r2, r3, B0
//only supports 16-bit unpacked mode
$cbuffer.advance_write_ptr_ex:
$_cbuffer_advance_write_ptr_ex:
    LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($cbuffer.advance_write_ptr_ex.PATCH_ID_0)
    push rLink;
    push M0;
    pushm <I0, L0>;
    pushm <r0, r1>;
    call $cbuffer.get_write_address_and_size_and_start_address_ex;
    L0 = r2;
    push r3;
    pop B0;
    I0 = r0;
    r2 = r1; // existing octet offset
    popm <r0, r1>; //get cbuffer address and octets to advance by
    r1 = r1 + r2; //add existing octet offset and octets to advance by
    r2 = r1 LSHIFT - 1; //get samples (only support 16-bit unpacked mode)
    r2 = r2 * ADDR_PER_WORD (int); //convert to locations
    M0 = r2;
    r2 = r1 AND 0x1; //get residue (only support 16-bit unpacked mode)
    r1 = M[I0, M0];
    r1 = I0;
    call $cbuffer.set_write_address_ex;
    popm <I0, L0>;
    pop M0;
    pop rLink;
    rts;
.ENDMODULE;

//extern void cbuffer_advance_read_ptr_ex(tCbuffer * cbuffer, unsigned num_octets);
.MODULE $M.cbuffer_advance_read_ptr_ex;
   .CODESEGMENT BUFFER_PM;
//in r0 cbuffer address
//in r1 octets to advance by
//trashed r2, r3
//only supports 16-bit unpacked mode
$cbuffer.advance_read_ptr_ex:
$_cbuffer_advance_read_ptr_ex:
    LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($cbuffer.advance_read_ptr_ex.PATCH_ID_0)
    push rLink;
    push M0;
    pushm <I0, L0>;
    pushm <r0, r1>;
    call $cbuffer.get_read_address_and_size_and_start_address_ex;
    L0 = r2;
    push r3;
    pop B0;
    I0 = r0;
    r2 = r1; // existing octet offset
    popm <r0, r1>; //get cbuffer address and octets to advance by
    r1 = r1 + r2; //add existing octet offset and octets to advance by
    r2 = r1 AND 0x1; //get residue (only support 16-bit unpacked mode)
    r1 = r1 LSHIFT - 1; //get samples (only support 16-bit unpacked mode)
    r1 = r1 * ADDR_PER_WORD (int); //convert to locations
    M0 = r1;
    r1 = M[I0, M0];
    r1 = I0;
    call $cbuffer.set_read_address_ex;
    popm <I0, L0>;
    pop M0;
    pop rLink;
    rts;
.ENDMODULE;

//extern unsigned cbuffer_copy_16bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets);
.MODULE $M.cbuffer_copy_16bit_be_zero_shift_ex;
   .CODESEGMENT BUFFER_PM;
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of octets to copy
// trashed r3, B0
$cbuffer.copy_16bit_be_zero_shift_ex:
$_cbuffer_copy_aligned_16bit_be_zero_shift_ex:
   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($cbuffer.copy_16bit_be_zero_shift_ex.PATCH_ID_0)
   // save the input paramerters for later
   pushm <FP(=SP), r0, r1, r2, r5, rLink>;
   pushm <I0, I4, L0, L4>;

   // get dest buffer true write address and size
   call $cbuffer.get_write_address_and_size_and_start_address_ex;
   I4 = r0;
   L4 = r2;
   push r3;
   pop B4;

   // get src buffer read address and size
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   // get the read address ans save it to the stack
   call $cbuffer.get_read_address_and_size_and_start_address_ex;
   I0 = r0;
   L0 = r2;
   push r3;
   pop B0;

   r5 =  M[FP + 3*ADDR_PER_WORD];               // copy amount
   Null = r1;                                   // r1 is the octet offset

   if Z jump no_offset;
   // read the second part of the word, combine it with the first one.

   r1 = M[I0, ADDR_PER_WORD];
   r1 = r1 AND 0xff;                            // The new octet is the LSB.
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
   r1 = M[I0, 0];
   r1 = r1 AND 0xff00;

   r2 = M[I4,0];
   r2 = r2 AND 0xff;

   r1 = r1 + r2;
   M[I4,0] = r1;


   copy_aligned_done:
   // Update the write address
   r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
   r1 = I4;
   r2 = r5 AND 0x1;
   call $cbuffer.set_write_address_ex;

   // Update the read address
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   r1 = I0;
   call $cbuffer.set_read_address_ex;

   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r5, rLink>;
   rts;
.ENDMODULE;

#endif /* INSTALL_CBUFFER_EX */

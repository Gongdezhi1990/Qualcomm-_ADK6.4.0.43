/*****************************************************************************
 Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*****************************************************************************/

#include "cbuffer_asm.h"
#include "../cbuffer/dw_cbuffer_asm.h"

// Maximum amount of data we can read [in words] from an external RAM without de-asserting Chip Select
// Keep it power of 2 to de-assert Chip Select on SRAM page boundaries!
.CONST $AP_SRAM_MAX_READ_BURST          64;

// EXTMEM_CLK enum values
.CONST $EXTMEM_CLK_PLL_OFF              0;
.CONST $EXTMEM_CLK_OFF                  1;
.CONST $EXTMEM_CLK_32                   2;
.CONST $EXTMEM_CLK_80                   3;

.CONST $240MHZ                          240;       

.CONST $NO_EXTRA_NOPS                   0;
.CONST $240MHZ_EXTRA_NOPS               20;

//extern unsigned cbuffer_calculate_extra_nops(void);
//out r0 available data in octets
//trashed none
// Calculates the amount of nops (based on the CPU clock) needed to break tight loops when accessing SPI RAM 

.MODULE $M.ext_buffer_calculate_extra_nops;
    .CODESEGMENT BUFFER_PM;
$cbuffer.calculate_extra_nops:
$_cbuffer_calculate_extra_nops:
    pushm <r1, r2>;
    r0 = $NO_EXTRA_NOPS;
    r2 = $240MHZ_EXTRA_NOPS;
    r1 = M[$profiler.cpu_speed_mhz];
    Null = r1 - $240MHZ;
    if EQ r0 = r2;
    popm <r1, r2>;
    rts;
    
.ENDMODULE;

// --------- copy_aligned 32-bit ----------
//void ext_buffer_copy_aligned_32bit_ex(unsigned char * dst, unsigned char *src, unsigned num_octets, EXTMEM_CLK mem_speed);
// in r0 destination address
// in r1 source address
// in r2 number of octets to copy
// in r3 EXT memory speed
// trashed r10
.MODULE $M.ext_buffer_copy_aligned_32bit_ex;
    .CODESEGMENT PM;
$_ext_buffer_copy_aligned_32bit_ex:
    // save the input parameters for later
    pushm <FP(=SP), r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, rLink>;
    pushm <I0, I4>;

    // align src & dest addresses on 32-bit boundary
    r2 = 0xFFFFFFFC;
    r3 = r0 AND r2;
    r5 = r1 AND r2;
    I4 = r3;
    I0 = r5;
   
    r1 = r1 AND 0x3;                           // BUFFER_EX_OFFSET_MASK

    r8 = ADDR_PER_WORD;                        // usable_octets
    r9 = 0xFFFFFFFF;

    r5 =  M[FP + 3*ADDR_PER_WORD];             // copy amount
    if Z jump no_trailing_data;
    Null = r1;                                 // r1 is the src octet offset

    if Z jump no_leading_offset;
        r6 = r8 - r1;
        // check if data to copy is smaller than amt_space in current word
        Null = r5 - r6;     
        if GE jump amount_data_ge;
            // copy less data than amt_space available in a word
            r2 = r1 - r8;
            r2 = r2 + r5;
            r6 = r1 LSHIFT 3;
            r7 = r2 LSHIFT 3;
            r6 = r9 LSHIFT r6;
            r7 = r9 LSHIFT r7;
            r6 = r6 AND r7;                    // src word mask
            
            r7 = r6 XOR r9;                    // dest word mask

            r2 = M[I0, 0];                     // read src word and mask
            r2 = r2 AND r6;

            r3 = M[I4, 0];                     // read dest word and mask 
            r3 = r3 AND r7;
            r2 = r3 + r2;                      // combine src & dest and save
            M[I4, 0] = r2;
            jump no_trailing_data;

        amount_data_ge:
        r5 = r5 - r6;                          // decrement amount left to copy by amt_space available in current word
        r6 = r1 LSHIFT 3;
        r6 = r9 LSHIFT r6;                     // src word mask

        r7 = r6 XOR r9;                        // dest word mask

        r2 = M[I0, ADDR_PER_WORD];             // read src word and mask
        r2 = r2 AND r6;

        r1 = M[I4, 0];                         // read dest word and mask 
        r1 = r1 AND r7;
        r1 = r1 + r2;                          // combine src & dest and save
        M[I4, ADDR_PER_WORD] = r1;

    no_leading_offset:
    r7 = r5 AND 0x3;

    call $cbuffer.calculate_extra_nops;
    r4 = r0;

    r6 = -2;                                     
    r0 = M[FP + 4*ADDR_PER_WORD];                // get EXT RAM speed
    r2 = $AP_SRAM_MAX_READ_BURST;
    Null = r0 - $EXTMEM_CLK_32;
    if EQ r2 = r2 LSHIFT r6;                     // if low speed, we want to break the loop 4x more often
    r3 = I0;
    r0 = r2 LSHIFT 2;                            // create a mask for EXT RAM address
    r0 = r0 - 1;
    
    r3 = r3 AND r0;                              // and apply it
    r10 = r2;
    r3 = r3 LSHIFT -LOG2_ADDR_PER_WORD;          // r3 contains the amount to read up to a submultiple of a EXT RAM page size
    if GT r10 = r10 - r3;
    r5 = r5 LSHIFT -LOG2_ADDR_PER_WORD;          // convert the copy amount to words
    if Z jump copy_aligned_write_last_octet;     // if copy amount is less than a word skip the loop
    reinit_loop:
        r3 = r10;                                // amount to copy reduction after loop
        Null = r5 - r10;
        if LT r10 = r5;
            r10 = r10 - 1;                       // decrement the amount due to the initial read and last write
            r1 = M[I0, ADDR_PER_WORD];           // initial read
            do copy_aligned_loop;
                r1 = M[I0, ADDR_PER_WORD],
                 M[I4, ADDR_PER_WORD] = r1;      // read and write
            copy_aligned_loop:
            M[I4, ADDR_PER_WORD] = r1;           // last write

        r10 = r4;
        do dummy_loop;
            Null = r1;                           // nop loop;
        dummy_loop:

        r10 = r2;                              
        r5 = r5 - r3;
        if GT jump reinit_loop;

    copy_aligned_write_last_octet:

    Null = r7;
    if Z jump no_trailing_data;
        r6 = r7 LSHIFT 3;
        r6 = r9 LSHIFT r6;                     // mask for dest
        r7 = r6 XOR r9;                        // mask for src

        r1 = M[I0, 0];                         // read src word and mask
        r1 = r1 AND r7;
        r2 = M[I4, 0];                         // read dest word and mask
        r2 = r2 AND r6;

        r1 = r1 + r2;                          // combine src & dest and save
        M[I4, 0] = r1;

    no_trailing_data:

    // Restore all registers
    popm <I0, I4>;
    popm <FP, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, rLink>;
    rts;
.ENDMODULE;

// --------- copy_unaligned 32-bit ----------
//void ext_buffer_copy_unaligned_32bit_ex(unsigned char * dst, unsigned char *src, unsigned num_octets);
// in r0 destination address
// in r1 source address
// in r2 number of octets to copy
// trashed r3, r10
.MODULE $M.ext_buffer_copy_unaligned_32bit_ex;
    .CODESEGMENT BUFFER_PM;
$_ext_buffer_copy_unaligned_32bit_ex:
    // save the input parameters for later
    pushm <FP(=SP), r0, r1, r2, r4, r5, r6, r7, r8, rLink>;
    pushm <I0, I4>;

    // align the addresses on 32-bit boundary
    r2 = 0xFFFFFFFC;
    r3 = r0 AND r2;
    r4 = r1 AND r2;
    I4 = r3;
    r6 = r0 AND 0x3;                      // BUFFER_EX_OFFSET_MASK
    I0 = r4;
    r1 = r1 AND 0x3;                      // BUFFER_EX_OFFSET_MASK

    r4 = ADDR_PER_WORD;
    r7 = 0xFF;                            // mask for destination
    r8 = 0xFFFFFFFF;
    r10 = M[FP + 3*ADDR_PER_WORD];        // amount of data to be copied
    do copy_loop;

        r2 = r1 LSHIFT 3;
        r3 = -r2;
        r2 = r7 LSHIFT r2;
        r5 = M[I0, 0];
        r5 = r5 AND r2;                   // octet read

        r5 = r5 LSHIFT r3;                // move octet read into the LSB - shift right

        r3 = r6 LSHIFT 3;
        r5 = r5 LSHIFT r3;                //source octet in the right position for source

        r3 = r6 LSHIFT 3;
        r3 = r7 LSHIFT r3;

        r3 = r3 XOR r8;                   // mask for src

        r2 = M[I4, 0];
        r2 = r2 AND r3;                   // protect the bits that aren't to be changed

        r2 = r2 + r5;                     // combine src octet in dest word and save
        M[I4, 0] = r2;

        r1 = r1 + 1;                      // update src offset
        Null = r4 - r1;
        if NZ jump check_dest_offset;
            r1 = 0;
            r3 = M[I0, ADDR_PER_WORD];

        check_dest_offset:
        r6 = r6 + 1;                      // update dest offset
        Null = r4 - r6;
        if NZ jump loop_end;
            r6 = 0;
            r3 = M[I4, ADDR_PER_WORD];

        loop_end:
        nop;
    copy_loop:

    // Restore all registers
    popm <I0, I4>;
    popm <FP, r0, r1, r2, r4, r5, r6, r7, r8, rLink>;
    rts;

.ENDMODULE;

// --------- copy_aligned 32-bit ----------
//void ext_buffer_copy_aligned_32bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer * src, unsigned num_octets, EXTMEM_CLK mem_speed);
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of octets to copy
// in r3 EXT memory speed
// trashed r10, B0, B4
.MODULE $M.ext_buffer_copy_aligned_32bit_be_zero_shift_ex;
    .CODESEGMENT BUFFER_PM;
$_ext_buffer_copy_aligned_32bit_be_zero_shift_ex:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, rLink>;
    pushm <I0, I4, L0, L4>;

    r8 = ADDR_PER_WORD;     // usable_octets is clearly 4 for external buffer
    r9 = 0xFFFFFFFF;

    call $cbuffer.get_write_address_ex;
    I4 = r0;
    L4 = r2;
    push r3;
    pop B4;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_ex;
    I0 = r0;
    L0 = r2;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_ex;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_ex;

        jump cp_pop_and_exit;

    not_in_place_copy:
    // init base for src ahead of doloop
    push r3;
    pop B0;

    r5 = M[FP + 3*ADDR_PER_WORD];               // copy amount
    if Z jump cp_pop_and_exit;
    Null = r1;                                   // r1 is the octet offset

    if Z jump no_offset;
        r6 = r8 - r1;
        Null = r5 - r6;     // check if amount of data is smaller than the space in the current word
        if POS jump amount_data_greater;
            // less data to be copied than space
            r6 = r8 - r5;
            r6 = r6 LSHIFT 3;
            r6 = r9 LSHIFT r6;      // mask for source in the LSB

            r7 = r1 LSHIFT 3;
            r7 = -r7;
            r6 = r6 LSHIFT r7;      // mask source in the right position

            r2 = M[I0, 0];          // read the source word
            r2 = r2 AND r6;

            r3 = M[I4, 0];          // Read the dest
            r6 = r6 XOR r9;         // invert r6
            r3 = r3 AND r6;         // zero only the bits that need to be copied

            r2 = r3 + r2;

            M[I4, 0] = r2;

            r7 = r1 + r5;
            Null = ADDR_PER_WORD - r7;
            if NZ jump copy_aligned_done;
                r7 = 0;
                r3 = M[I4, ADDR_PER_WORD];
                r3 = M[I0, ADDR_PER_WORD];
                jump copy_aligned_done;

        amount_data_greater:
        r5 = r5 - r6;               // decrement the copy amount
        r6 = r1 LSHIFT 3;
        r6 = -r6;
        r6 = r9 LSHIFT r6;          // mask for source

        r7 = r8 - r1;
        r7 = r7 LSHIFT 3;
        r7 = r9 LSHIFT r7;          // mask for destination

        // read the second part of the word, combine it with the first one.
        r2 = M[I0, ADDR_PER_WORD];
        r2 = r2 AND r6;                        // The new octets are in the LSBs.

        r1 = M[I4, 0];                         // Read the msb
        r1 = r1 AND r7;                        // Mask the lsb
        r1 = r1 + r2;                          // combine it
        M[I4, ADDR_PER_WORD] = r1;             // write it to the buffer

    no_offset:
    r7 = r5 AND 0x3;

    call $cbuffer.calculate_extra_nops;
    r4 = r0;
    
    r6 = -2;                                     
    r0 = M[FP + 4*ADDR_PER_WORD];                // get EXT RAM speed
    r2 = $AP_SRAM_MAX_READ_BURST;
    Null = r0 - $EXTMEM_CLK_32;
    if EQ r2 = r2 LSHIFT r6;                     // if low speed, we want to break the loop 4x more often
    r3 = I0;
    r0 = r2 LSHIFT 2;                            // create a mask for EXT RAM address
    r0 = r0 - 1;
    
    r3 = r3 AND r0;                              // and apply it
    r10 = r2;
    r3 = r3 LSHIFT -LOG2_ADDR_PER_WORD;          // r3 contains the amount to read up to a submultiple of a EXT RAM page size
    if GT r10 = r10 - r3;
    r5 = r5 LSHIFT -LOG2_ADDR_PER_WORD;          // convert the copy amount to words
    if Z jump copy_aligned_write_last_octet;     // if copy amount is less than a word skip the loop
    reinit_loop:
        r3 = r10;                                // amount to copy reduction after loop
        Null = r5 - r10;
        if LT r10 = r5;
            r10 = r10 - 1;                       // decrement the amount due to the initial read and last write
            r1 = M[I0, ADDR_PER_WORD];           // initial read
            do copy_aligned_loop;
                r1 = M[I0, ADDR_PER_WORD],
                 M[I4, ADDR_PER_WORD] = r1;      // read and write
            copy_aligned_loop:
            M[I4, ADDR_PER_WORD] = r1;           // last write

        r10 = r4;
        do dummy_loop;
            Null = r1;                           // nop loop;
        dummy_loop:

        r10 = r2;                              
        r5 = r5 - r3;
        if GT jump reinit_loop;

    copy_aligned_write_last_octet:

    Null = r7;
    if Z jump copy_aligned_done;
        r6 = r8 - r7;           // us_octets - amount_data left
        r6 = r6 LSHIFT 3;
        r6 = r9 LSHIFT r6;      // mask for source

        r8 = r7 LSHIFT 3;
        r8 = -r8;
        r8 = r9 LSHIFT r8;      // mask for destination

        r1 = M[I0, 0];
        r1 = r1 AND r6;
        r2 = M[I4, 0];
        r2 = r2 AND r8;

        r1 = r1 + r2;
        M[I4, 0] = r1;

    copy_aligned_done:
    // Update the write address
    r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
    r1 = I4;
    r2 = r7;
    call $cbuffer.set_write_address_ex;

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I0;
    call $cbuffer.set_read_address_ex;

    cp_pop_and_exit:
    // Restore index & length registers
    popm <I0, I4, L0, L4>;
    popm <FP, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, rLink>;
    rts;
.ENDMODULE;

// --------- copy_packed_to_unpacked ----------
//void ext_buffer_copy_packed_to_audio_shift(tCbuffer * dst, tCbuffer *src, unsigned num_words, EXTMEM_CLK mem_speed); ----------------------------------------
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of words to copy
// trashed r3, r10, B0, B4
.MODULE $M.ext_buffer_copy_packed_to_audio_shift;
    .CODESEGMENT BUFFER_PM;
$_ext_buffer_copy_packed_to_audio_shift:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, rLink>;
    pushm <I0, I4, M0, L0, L4>;

    // load a few constants
    M0 = ADDR_PER_WORD;
    // get dest buffer true write address and size
    call $cbuffer.get_write_address_ex;
    I0 = r0;
    L0 = r2;
    push r3;
    pop B0;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_ex;
    I4 = r0;
    r6 = r1;        // save read octet offset
    L4 = r2;

    call $cbuffer.calculate_extra_nops;
    r9 = r0;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_ex;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_ex;

        jump cp_pop_and_exit;

    not_in_place_copy:
    // init base for src ahead of doloop
    push r3;
    pop B4;

    // get the amount of data to copy and set up a few masks
    r5 = M[FP + 3*ADDR_PER_WORD];
    if Z jump upd_ptrs;
    r3 = 0xFFFF0000;

    Null = r6;
    if Z jump src_word_aligned;
        // the previous read left the offset mid-word;
        // we first get word aligned to set up an efficient packing loop
        r0 = M[I4, M0];
        r0 = r0 AND r3;
        r5 = r5 - 1, M[I0, M0] = r0;
    src_word_aligned:

    // loop count is half the number of words to pack
    r6 = 0;

    r7 = r5 ASHIFT -1;
    if Z jump unpack_check_last_sample;          // if copy amount is less than a word skip the loop

    r1 = -2;                                     
    r0 = M[FP + 4*ADDR_PER_WORD];                // get EXT RAM speed
    r4 = $AP_SRAM_MAX_READ_BURST;
    Null = r0 - $EXTMEM_CLK_32;
    if EQ r4 = r4 LSHIFT r1;                     // if low speed, we want to break the loop 4x more often
    r1 = I4;
    r0 = r4 LSHIFT 2;                            // create a mask for EXT RAM address
    r0 = r0 - 1;
    
    r1 = r1 AND r0;                              // and apply it
    r10 = r4;
    r1 = r1 LSHIFT -LOG2_ADDR_PER_WORD;          // r3 contains the amount to read up to a submultiple of a EXT RAM page size
    if GT r10 = r10 - r1;

    reinit_loop:
        r8 = r10;                                // amount to copy reduction after loop
        Null = r7 - r10;
        if LT r10 = r7;
        do copy_loop;
            r2 = M[I4, M0];
            r0 = r2 LSHIFT 16;
            r1 = r2 AND r3, M[I0,  M0] = r0;
            M[I0,  M0] = r1;
        copy_loop:

        r10 = r9;
        do dummy_loop;
            Null = r1;                           // nop loop;
        dummy_loop:
 
        r10 = r4;                              
        r7 = r7 - r8;
        if GT jump reinit_loop;

    unpack_check_last_sample:
    // if any leftover sample unpack it here
    Null = r5 AND 0x1;
    if Z jump upd_ptrs;
        r0 = M[I4, 0];
        r0 = r0 LSHIFT 16;
        M[I0, M0] = r0;
        r6 = 2;

    upd_ptrs:
    // Update the write address
    r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
    r2 = 0;
    r1 = I0;
    call $cbuffer.set_write_address_ex;

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I4;
    r2 = r6;
    call $cbuffer.set_read_address_ex;

    cp_pop_and_exit:
    // Restore index, modify & length registers
    popm <I0, I4, M0, L0, L4>;
    popm <FP, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, rLink>;
    rts;

.ENDMODULE;


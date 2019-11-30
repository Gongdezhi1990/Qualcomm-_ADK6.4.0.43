// *****************************************************************************
// Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
// *****************************************************************************

                     //----Summary----//
// Cbuffer (_ex) library functions supporting octet access. This cbuffer library
// extension can track octets within a word.
// Not all cbuffer functions are extended.

                     //----What's supported?----//
// 16-bit unpacked MMU and Software buffers and 32-bit packed SW buffers are supported.
// Only supports 32-bit KAL_ARCH4 platform.
// AUX pointers are not supported.


                     //----Read pointers----//
// For Software read pointers, the number of octets read within a word are
// stashed in the 2 LSBs of the read pointer. There can only be one odd octet for 16-bit unpacked
// and 2 odd octets for 32-bit packed.
//
// The SW read pointer is not incremented until all octets in the word are read.
// The SW read pointer always points to an unread or a partially read word.

// For MMU read pointers (handles), the octet information is available in the
// BAC buffer offset and the read pointer (that points to an MMU handle) in the
// cbuffer structure is not touched.

// The _ex cbuffer and the standard cbuffer functions always a rounded down
// version of the read pointer.
// The _ex cbuffer functions additionally return octet information
// (in a separate register).

                     //----Write pointers----//
// For Software write pointers, the number of octets written into a word are
// stashed in the 2 LSBS of the write pointer. If a partial word is written
// the write pointer is immediately rounded up (incremented by ADDR_PER_WORD).
// Any further odd octets written to to the same word will keep the write
// pointer pointing to the same (rounded up) location. The SW write pointer
// always points to the next whole word to write to.

// For MMU write pointers (handles) the octet information is available in the
// BAC buffer offset.
// The internal BAC offsets are obviously not rounded up as these offsets can
// be used by entities outside the audio subsystem.

// Note that the _ex cbuffer functions always return the "truth" (*).
// For example, $cbuffer.get_write_address_exx)
// will return the "true" (*) (not rounded up) write pointer. Octet information
// is returned separately (for example, in a separate register).

// (*) "truth/true pointer" in this document/header and throughout cbuffer_ex
// are meant to differentiate from the plain/traditional cbuffer API that would
// "lie", i.e. return incremented/rounded-up write pointers for partially used
// words.
// Be aware that the pointers _are_ in reality rounded-up when an odd octet is
// written (if you look into the cbuffer structure - write_address)
// but the _ex API hides this from user (returns "true" pointer).

// For SW write pointers the standard cbuffer functions
// ($cbuffer.get_write_address_and_size_and_start_address)
// will return the rounded up version of the write pointer. Obviously no octet
// information is returned. This is so codecs such as SBC that use the standard
// cbuffer functions can see that there is enough data in their input. They may
// see some extra garbage octets too (maximum of 3 garbage octets in the 32-bit
// unpacked case, maximum of 1 garbage octet in the 16-bit unpacked case).
// These libaries are self-framing and they normally see whole packets.
// They are expected to start decode only when all data required for a frame is
// available. Therefore, seeing extra garbage octets due to a rounded up
// software write pointer is generally not an issue. Further we only tend to
// receive odd octets at the end of the stream.

// We normally expect the apps DMA engine to transfer data in multiples of 2
// (or possibly even powers of 2) chunks. Therefore we don't expect the problem
// of interpreting a garbage octet to hit us.


                  //----Misc----//
// The standard cbuffer functions will mask out the 2 LSBs from the Software
// read and the Software write pointers.
// It is important further changes to the standard cbuffer functions (that can
// operate on octet buffers) mask out the LSBs. Macros for these have been
// provided in cbuffer_asm.h (BUFFER_GET/MASK_READ/WRITE_ADDR)


                  //----Aberrations----//
// The standard cbuffer functions won't return rounded up write pointers for
// the MMU write handles. We don't expect MMU buffers to be connected to
// libraries directly so this is generally not an issue. This aberration
// is because of factors not under our control.

// For pure software buffers  (both read and write pointers are software)
// it is possible to maintain an invariant that the write and read pointers are
// at least one word apart so one can differentiate a buffer full and a buffer
// empty condition. For example a 16-bit unpacked buffer of 128 words can in
// theory hold 255 octets.
// It is possible to limit the maximum data in this buffer to 254 octets
// (by making calc_amount_space_exx return one word (2 octets) less than the
// actual space available). This will prevent the buffer holding 255 octets and
// will therefore prevent the write pointer being rounded (so the buffer won't
// appear empty when it is actually full).
// If the write pointer is an MMU handle written by an external entity it is
// not possible to maintain this invariant.
// The 16-bit unpacked buffer discussed above can hold 255 octets. This is what
// prevents the standard cbuffer functions returning a rounded up  write
// pointer when there is an underlying MMU handle. This also results in
// calc_amount_space_exx returning space - 1 (1 - 1 = 0) when the buffer has 255
// octets worth data rather than the usual space - 2 (which will otherwise be a
// negative number).


                  //----Functions callable from C and ASM ----//
// $M.cbuffer.get_read_address_exx(r0 = cbuffer address, r1 = read octet offset pointer)
// returns read-address (r0) and read octet offset written into address pointed to by offset

// $M.cbuffer.get_write_address_exx(r0 = cbuffer address, r1 = write octet offset pointer)
// returns true write-address (r0) and write octet offset written into address pointed to by offset

// $M.cbuffer.set_read_address_exx(r0 = cbuffer address, r1 = true read address, r2 = read octet offset)
// returns void

// $M.cbuffer.set_write_address_exx(r0 = cbuffer address, r1 = true write address, r2 = write octet offset)
// returns void

// $M.cbuffer.calc_amount_data_exx(r0 = cbuffer address)
// returns amount of data in octets

// $M.cbuffer.calc_amount_space_exx
// returns amount of space in octets - 2, clipped to 0

// $M.cbuffer_advance_write_ptr_exx(r0 = cbuffer address, r1 = octets to advance by)
// returns void

//$M.cbuffer_advance_read_ptr_exx(r0 = cbuffer address, r1 = octets to advance by)
// returns void

                  //----Functions callable only from ASM----//
//$M.cbuffer.get_read_address_exx(r0 = cbuffer address)
// returns true true read address (r0), read octet offset (r1), size in locations (r2), base (r3)

//$M.cbuffer.get_write_address_exx(r0 = cbuffer address)
// returns true write address (r0), write octet offset (r1), size in locations (r2), base (r3)

// *****************************************************************************

#ifdef INSTALL_CBUFFER_EX
#include "dw_cbuffer_asm.h"

#define US_OCTETS_16BIT 2;

.MODULE $M.cbuffer.get_read_address_exx;
    .CODESEGMENT BUFFER_PM;

// in r0 cbuffer address
// out r0 true read address word aligned
//     r1 octet offset
//     r2 size in addresses
//     r3 buffer start address
// trashed - none
$cbuffer.get_read_address_exx:
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
    r1 = r0 AND BUFFER_EX_OFFSET_MASK;
    BUFFER_MASK_READ_ADDR(r0);
    rts;

// and its C API (NOT returning size and start_address)
//extern unsigned int * cbuffer_get_read_address_exx(tCbuffer * cbuffer, unsigned * offset);
//in r0 cbuffer address
//in r1 pointer to octet offset
//out r0 read address
//trashed r2, r3
$_cbuffer_get_read_address_exx:
    push rLink;
    push r1;
    call $cbuffer.get_read_address_exx;
    pop r2;
    M[r2] = r1;
    pop rLink;
    rts;

.ENDMODULE;


.MODULE $M.cbuffer.get_write_address_exx;
    .CODESEGMENT BUFFER_PM;

// in r0 cbuffer address
// out r0 true write address word aligned
// out r1 write octet offset
// out r2 size in locations
// out r3 base address
// trashed B0
// $cbuffer.get_write_address_ex is for the benefit of support_lib/ext_buffer
// The function is not in earlier cbuffer versions but to keep naming consistent
// this file uses '_exx' throughout and just for ext_buffer '_ex' is added.
$cbuffer.get_write_address_exx:
$cbuffer.get_write_address_ex:
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
    r1 = r0 AND BUFFER_EX_OFFSET_MASK;
    if Z rts;
        BUFFER_MASK_WRITE_ADDR(r0);     // returns word aligned no offset
        pushm <I0, L0>;
        I0 = r0;
        L0 = r2;
        push r3;
        pop B0;
        r0 = M[I0, -ADDR_PER_WORD];
        r0 = I0;                        // go back one word
        popm <I0, L0>;
        rts;

// and its C API (NOT returning size and start_address)
//extern unsigned int * cbuffer_get_write_address_exx(tCbuffer * cbuffer, unsigned * offset);
//in r0 cbuffer address
//in r1 pointer to octet offset
//out r0 true write address
//trashed r2, r3, B0
$_cbuffer_get_write_address_exx:
    push rLink;
    push r1;
    call $cbuffer.get_write_address_exx;
    pop r2;
    M[r2] = r1;
    pop rLink;
    rts;

.ENDMODULE;


//extern unsigned cbuffer_calc_amount_data_exx(tCbuffer * cb);
//in  r0 cbuffer address
//out r0 available data in octets
//trashed r1, r2, r3, B0
//only supports 16-bit unpacked/32-bit packed modes
.MODULE $M.cbuffer.calc_amount_data_exx;
    .CODESEGMENT BUFFER_PM;
$cbuffer.calc_amount_data_exx:
$_cbuffer_calc_amount_data_exx:
    pushm <r4, r5, r6, rLink>;

    BUFFER_GET_USABLE_OCTETS(r6, r0);
    Null = r6 - ADDR_PER_WORD;         // usable_octets
    if Z r6 = 0;// this function relies on 0 when usable octets is 4. Diff between stre rom and stre dev

    push r0;
    call $cbuffer.get_write_address_exx;
    r4 = r0;
    r5 = r1;
    pop r0;
    call $cbuffer.get_read_address_exx;
    // side-effect returns r2 = buffer_size
    r3 = r4 - r0;       // data = wrp - rdp
    if NEG r3 = r3 + r2;

    r4 = -1;
    Null = r6;
    if Z jump check_if_rd_eq_wr;
        r3 = r3 LSHIFT r4;

    check_if_rd_eq_wr:
    Null = r3;
    if Z jump rd_equals_wr;
        r0 = r3 - r1;   // subtract read octets
        r0 = r0 + r5;   // add written octets
        jump end;
    rd_equals_wr:
    r0 = r5 - r1;
    if POS jump end;
        // Buffer_full:
        // This is expected only if the buffer has an MMU write/read handle
        // For normal software buffers there will always be a gap of at least 2 octets.
        // This is because how much we allow to write into the buffer is completely
        //  under the control of SW running on the DSP.
        // For MMU buffers written to or read by other sub-systems this gap can be just 1 octet.
        Null = r6;
        if Z jump subtract_mmu_gap;
            r2 = r2 LSHIFT r4;  // divide by ADDR_PER_WORD - transform in usable_octets size
        subtract_mmu_gap:
        r0 = r2 + r4;
    end:
    popm <r4, r5, r6, rLink>;
    rts;
.ENDMODULE;

//extern unsigned cbuffer_calc_amount_space_exx(tCbuffer * cb);
//in  r0 cbuffer address
//out r0 available space in octets -2/4, clipped to zero
//trashed r1, r2, r3, B0
// (still trashes r3 because of get_*_address_ex)
// only supports 16-bit unpacked/32-bit packed modes
.MODULE $M.cbuffer.calc_amount_space_exx;
    .CODESEGMENT BUFFER_PM;
$cbuffer.calc_amount_space_exx:
$_cbuffer_calc_amount_space_exx:
    pushm <r4, r5, r6, rLink>;

    BUFFER_GET_USABLE_OCTETS(r6, r0);
    Null = r6 - ADDR_PER_WORD;         // usable_octets
    if Z r6 = 0;

    push r0;
    call $cbuffer.get_write_address_exx;
    r4 = r0;
    r5 = r1;

    // check if the buffer is part of an in-place chain.
    pop r0;
    r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
    Null = r1 AND $cbuffer.IN_PLACE_MASK;
    if Z jump get_rd_addr;
        // its_in_place
        r2 = M[r0 + $cbuffer.AUX_ADDR_FIELD];
        if NZ r0 = r2;  // use the rdp of the head of the in-place chain
    get_rd_addr:
    call $cbuffer.get_read_address_exx;
    // side-effect, the asm API returns r2 = buffer_size
    r0 = r0 - r4;               // space = rdp - wrp
    if Z jump rd_equals_wr;
        if NEG r0 = r0 + r2;

        Null = r6;
        if Z jump account_for_offsets;
            r0 = r0 LSHIFT -1;
        account_for_offsets:
        r0 = r0 + r1;           // add read octets
        r0 = r0 - r5;           // subtract written octets
        jump end;

    rd_equals_wr:
        r0 = r1 - r5;
        if GT r0 = -1;          /* Buffer_empty:
            This is expected only if the buffer has an MMU write/read handle.
            For normal software buffers there will always be a gap of at least
            2 octets.
            This is because how much we allow to write into the buffer is
            completely under the control of SW running on the DSP.
            For MMU buffers written to or read by hardware this gap can be
            just 1 octet. */
        Null = r6;
        if Z jump add_octets;
            r2 = r2 LSHIFT -1;
        add_octets:
        r0 = r0 + r2;

    end:
    Null = r6;
    if Z r6 = ADDR_PER_WORD;
    // space in octets -2/4, clipped to zero
    r0 = r0 - r6;
    if NEG r0 = 0;
    // When the buffer has an MMU read or write handle
    // it is possible that at this point r0 is zero. See note
    // in cbuffer.calc_amount_data_exx. Therefore we need to clip
    // the return value to zero so we don't return a negative value, ie: -1.


    popm <r4, r5, r6, rLink>;
    rts;
.ENDMODULE;

//extern void cbuffer_set_read_address_exx(tCbuffer * cbuffer, unsigned int * ra, unsigned ro);
//in r0 cbuffer address
//in r1 true read address
//in r2 read octet offset
//trashed r3, (r10 if mmu buffer)
.MODULE $M.cbuffer.set_read_address_exx;
    .CODESEGMENT BUFFER_PM;
$cbuffer.set_read_address_exx:
$_cbuffer_set_read_address_exx:
    r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
    Null = r3 AND $cbuffer.BUFFER_TYPE_MASK;
    if Z jump its_a_sw_wr_ptr;
        Null = r3 AND $cbuffer.READ_PTR_TYPE_MASK;
        if Z jump its_a_sw_wr_ptr;
            r3 = M[r0 + $cbuffer.START_ADDR_FIELD];
            r1 = r1 - r3;
            r0 = M[r0 + $cbuffer.READ_ADDR_FIELD];
            jump $mmu.set_byte_offset_from_address_ex;

    its_a_sw_wr_ptr:
    r1 = r1 OR r2;
    M[r0 + $cbuffer.READ_ADDR_FIELD] = r1;
    rts;
.ENDMODULE;

//extern void cbuffer_set_write_address_exx(tCbuffer * cbuffer, unsigned int * wa, unsigned wo);
//in r0 cbuffer address
//in r1 true write address
//in r2 write octet offset
//trashed r3, B0, (r10 if mmu buffer)
.MODULE $M.cbuffer.set_write_address_exx;
    .CODESEGMENT BUFFER_PM;
$cbuffer.set_write_address_exx:
$_cbuffer_set_write_address_exx:
    r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
    Null = r3 AND $cbuffer.BUFFER_TYPE_MASK;
    if Z jump its_a_sw_wr_ptr;
        Null = r3 AND $cbuffer.WRITE_PTR_TYPE_MASK;
        if Z jump its_a_sw_wr_ptr;
            r3 = M[r0 + $cbuffer.START_ADDR_FIELD];
            r1 = r1 - r3;
            r0 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
            jump $mmu.set_byte_offset_from_address_ex;

    its_a_sw_wr_ptr:
    Null = r2 AND BUFFER_EX_OFFSET_MASK;
    if Z jump no_round_up;
        pushm <I0, L0>;
        I0 = r1;
        BUFFER_GET_SIZE_IN_ADDRS_ASM(L0, r0);
        r3 = M[r0 + $cbuffer.START_ADDR_FIELD];
        push r3;
        pop B0;
        r3 = M[I0, ADDR_PER_WORD];
        r1 = I0;
        r1 = r1 OR r2;
        popm <I0, L0>;
    no_round_up:
    M[r0 + $cbuffer.WRITE_ADDR_FIELD] = r1;
    rts;
.ENDMODULE;

//extern void cbuffer_advance_write_ptr_exx(tCbuffer * cbuffer, unsigned num_octets);
//in r0 cbuffer address
//in r1 octets to advance by
//trashed r1, r2, r3, B0, (r10 if mmu buffer)
//only supports 16-bit unpacked/32-bit packed modes
.MODULE $M.cbuffer_advance_write_ptr_exx;
    .CODESEGMENT BUFFER_PM;
$cbuffer.advance_write_ptr_exx:
$_cbuffer_advance_write_ptr_exx:
    push rLink;
    push M0;
    push r4;
    pushm <I0, L0>;
    pushm <r0, r1>;

    BUFFER_GET_USABLE_OCTETS(r4, r0);
    Null = r4;         // usable_octets
    if Z r4 = ADDR_PER_WORD;

    call $cbuffer.get_write_address_exx;
    L0 = r2;

    push r3;
    pop B0;
    I0 = r0;
    r2 = r1;            // existing octet offset
    popm <r0, r1>;      // get cbuffer address and octets to advance by
    r1 = r1 + r2;       // add existing octet offset and octets to advance by
    r3 = -1;
    r2 = r1 LSHIFT r3;  // get samples (for 16-bit unpacked)
    Null = ADDR_PER_WORD - r4;
    if NZ jump prepare_offset_index;
        r2 = r2 LSHIFT r3;      // further divide by 2 for 32-bit packed

    prepare_offset_index:
    r2 = r2 ASHIFT LOG2_ADDR_PER_WORD;  // *4 to restore the initial write ptr without offset
    M0 = r2;
    r4 = r4 + r3;    // mask for offset: max 1 for 16-bit and max 3 for 32-bit

    r2 = r1 AND r4; // get the offset
    r1 = M[I0, M0];

    r1 = I0;
    call $cbuffer.set_write_address_exx;
    popm <I0, L0>;
    pop r4;
    pop M0;
    pop rLink;
    rts;
.ENDMODULE;

//extern void cbuffer_advance_read_ptr_exx(tCbuffer * cbuffer, unsigned num_octets);
//in r0 cbuffer address
//in r1 octets to advance by
//trashed r1, r2, r3, (r10 if mmu buffer)
//only supports 16-bit unpacked/32-bit packed modes
.MODULE $M.cbuffer_advance_read_ptr_exx;
   .CODESEGMENT BUFFER_PM;
$cbuffer.advance_read_ptr_exx:
$_cbuffer_advance_read_ptr_exx:
    push rLink;
    push r4;
    push M0;
    pushm <I0, L0>;
    pushm <r0, r1>;

    BUFFER_GET_USABLE_OCTETS(r4, r0);
    Null = r4;         // usable_octets
    if Z r4 = ADDR_PER_WORD;

    call $cbuffer.get_read_address_exx;
    L0 = r2;

    push r3;
    pop B0;
    I0 = r0;
    r2 = r1;            // existing octet offset
    popm <r0, r1>;      // get cbuffer address and octets to advance by
    r1 = r1 + r2;       // add existing octet offset and octets to advance by
    r3 = -1;
    r2 = r4 + r3;
    r2 = r1 AND r2;

    r1 = r1 LSHIFT r3;
    Null = ADDR_PER_WORD - r4;
    if NZ jump set_mod_reg;
        r1 = r1 LSHIFT r3;

    set_mod_reg:
    r1 = r1 ASHIFT LOG2_ADDR_PER_WORD;
    M0 = r1;
    r1 = M[I0, M0];
    r1 = I0;
    call $cbuffer.set_read_address_exx;
    popm <I0, L0>;
    pop M0;
    pop r4;
    pop rLink;
    rts;
.ENDMODULE;

// --------- copy_aligned 16-bit ----------
//void cbuffer_copy_aligned_16bit_be_zero_shift_exx(tCbuffer * dst, tCbuffer *src, unsigned num_octets);
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of octets to copy
// trashed r3, r10, B0, B4
.MODULE $M.cbuffer_copy_aligned_16bit_be_zero_shift_exx;
    .CODESEGMENT BUFFER_PM;
$cbuffer.copy_aligned_16bit_be_zero_shift_exx:
$_cbuffer_copy_aligned_16bit_be_zero_shift_exx:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r5, rLink>;
    pushm <I0, I4, L0, L4>;

    // get dest buffer true write address and size
    call $cbuffer.get_write_address_exx;
    I4 = r0;
    L4 = r2;
    push r3;
    pop B4;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_exx;
    I0 = r0;
    L0 = r2;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_exx;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_exx;

        jump cp_pop_and_exit;

    not_in_place_copy:
    // init base for src ahead of doloop
    push r3;
    pop B0;

    r5 =  M[FP + 3*ADDR_PER_WORD];               // copy amount
    Null = r1;                                   // r1 is the octet offset

    if Z jump no_offset;
        // read the second part of the word, combine it with the first one.
        r1 = M[I0, ADDR_PER_WORD];
        r1 = r1 AND 0xff;                        // The new octet is the LSB.
        r2 = M[I4, 0];                           // Read the msb
        r2 = r2 AND 0xff00;                      // Mask the lsb
        r1 = r1 + r2;                            // combine it
        M[I4, ADDR_PER_WORD] = r1;               // write it to the buffer
        r5 = r5 - 1;                             // one octet already written, decrement
                                                 // the copy amount
    no_offset:
    r10 = r5 LSHIFT -1;                          // convert the copy amount to words
    if Z jump copy_aligned_write_last_octet;     // could be that only one octet we need to copy.
        r10 = r10 - 1;                    // decrement the amount due to the initial read and last write
        r1 = M[I0,ADDR_PER_WORD];         // initial read
        do copy_aligned_loop;
            r1 = M[I0,ADDR_PER_WORD],
             M[I4,ADDR_PER_WORD] = r1;    // read and write
        copy_aligned_loop:
        M[I4,ADDR_PER_WORD] = r1;         // last write

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
    call $cbuffer.set_write_address_exx;

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I0;
    call $cbuffer.set_read_address_exx;

    cp_pop_and_exit:
    // Restore index & length registers
    popm <I0, I4, L0, L4>;
    popm <FP, r0, r1, r2, r5, rLink>;
    rts;
.ENDMODULE;

// --------- copy_unaligned 16-bit ----------
//void cbuffer_copy_unaligned_16bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets);
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of octets to copy
// trashed r3, r10, B0, B4
.MODULE $M.cbuffer_copy_unaligned_16bit_be_zero_shift_ex;
    .CODESEGMENT BUFFER_PM;
$cbuffer.copy_unaligned_16bit_be_zero_shift_ex:
$_cbuffer_copy_unaligned_16bit_be_zero_shift_ex:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r5, r6, r8, rLink>;
    pushm <I0, I4, L0, L4>;

    // get dest buffer true write address and size
    call $cbuffer.get_write_address_exx;
    I4 = r0;
    L4 = r2;
    r6 = r1;        // save write octet offset
    push r3;
    pop B4;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_exx;
    I0 = r0;
    L0 = r2;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_exx;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_exx;

        jump cp_pop_and_exit;

    not_in_place_copy:
    // init base for src ahead of doloop
    push r3;
    pop B0;

    r3 = 0xFF;                         // mask for keeping LSByte
    r8 = 8;                            // shift value for faster access
    push r1;                           // save read octet offset
    r5 =  M[FP + 3*ADDR_PER_WORD];     // copy amount
    Null = r1;                         // read octet offset
    if Z jump check_wr_offset;
        // unaligned: rd offset
        Null = r6;                     // check write octet offset
        if NZ call $error;  // same_offset should be captured by the caller
                            // TODO grace fail, return octets_copied and make the caller check
        // unaligned, different offsets, rd offset (no wr offset)
        r1 = M[I0, ADDR_PER_WORD];      // read word to be copied
        r1 = r1 AND r3;                 // LSByte only to be mixed
        r1 = r1 LSHIFT r8,               // on the MSByte position
         r0 = M[I0, ADDR_PER_WORD];      // read in advance next word to be copied
        r2 = r0 AND r3;                 // keep LSByte to be mixed to next word
        r0 = r0 LSHIFT -8;              // on the LSByte position
        r1 = r1 OR r0;                  // mix it
        r2 = r2 LSHIFT r8,              // value to be mixed next on the MSByte position
         M[I4, ADDR_PER_WORD] = r1,      // write a whole word back
         r0 = M[I0, ADDR_PER_WORD];      // read next word to be copied
        r5 = r5 - 2;                    // two octets already written
        // keep in mind src buffer (rdp) runs ahead one word
        jump copy_unaligned_loop;

    check_wr_offset:
        // no rd offset
        Null = r6;                           // write octet offset
        if Z call $error;   // same_offset should be captured by the caller TODO
            // unaligned, different offsets, no rd but wr
            r1 = M[I0, ADDR_PER_WORD],       // read word to be copied
             r0 = M[I4, 0];                   // read what's currently written
            r2 = r1 AND r3;                  // keep LSByte to be mixed to next word
            r0 = r0 AND 0xFF00;              // mask out the LSByte of what's there
            r1 = r1 LSHIFT -8;               // octet to be written on LSByte position
            r1 = r1 OR r0;                   // mix it
            r2 = r2 LSHIFT r8,               // value to be mixed next on the MSByte position
             M[I4, ADDR_PER_WORD] = r1,       // write it back
             r0 = M[I0, ADDR_PER_WORD];       // read next word to be copied
            r5 = r5 - 1;                     // one octet already written

        copy_unaligned_loop:
        r10 = r5 LSHIFT -1;              // convert the copy amount to words
        if Z jump write_last_octet;      // could be that only one octet we need to copy.

        // next octet to be mixed in r2 on MSByte position
        do cp_unld_loop;
            r1 = r0 AND r3;              // keep LSByte to be mixed to next word
            r0 = r0 LSHIFT -8;           // octet to be written on LSByte position
            r0 = r0 OR r2;               // mix with octet stored in previous iteration
            r2 = r1 LSHIFT r8;            // on the MSByte position
            M[I4, ADDR_PER_WORD] = r0,   // write a whole word back
             r0 = M[I0, ADDR_PER_WORD];  // read next word to be copied
        cp_unld_loop:

    write_last_octet:
        r1 = M[I0, -ADDR_PER_WORD]; // adjust back src buffer incremented in the last loop

    Null = r5 AND 0x01;         // check if doloop has copied even number of octets
    if Z jump adj_src_ptr;
        r0 = M[I4, 0];          // read what's currently written
        // mix with LSByte stored in last doloop iteration
        r0 = r0 AND r3;         // keep only LSByte of what's there
        r0 = r0 OR r2;          // mix
        M[I4,0] = r0;           // and write a whole word back
        jump upd_ptrs;

    adj_src_ptr:
        r1 = M[I0, -ADDR_PER_WORD];   // adjust back src buffer when rdp was running ahead

    upd_ptrs:
    // Update the write address
    r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
    r1 = I4;
    r5 = M[FP + 3*ADDR_PER_WORD];                // copy amount
    r2 = r5 XOR r6;     // new offset is a combination of old one and amt_copied
    r2 = r2 AND 0x01;   // play safe, only lsb
    call $cbuffer.set_write_address_exx;
    // r2 is not trashed

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I0;
    pop r2;             // restore read offset
    r2 = r2 XOR r5;     // new offset is a combination of old one and amt_copied
    r2 = r2 AND 0x01;   // play safe, only lsb
    call $cbuffer.set_read_address_exx;

    cp_pop_and_exit:
    // Restore index & length registers
    popm <I0, I4, L0, L4>;
    popm <FP, r0, r1, r2, r5, r6, r8, rLink>;
    rts;

.ENDMODULE;

// --------- copy_aligned 32-bit ----------
//void cbuffer_copy_aligned_32bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets);
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of octets to copy
// trashed r3, r10, B0, B4
.MODULE $M.cbuffer_copy_aligned_32bit_be_zero_shift_ex;
    .CODESEGMENT BUFFER_PM;
$cbuffer.copy_aligned_32bit_be_zero_shift_ex:
$_cbuffer_copy_aligned_32bit_be_zero_shift_ex:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r5, r6, r7, r8, r9, rLink>;
    pushm <I0, I4, L0, L4>;

    BUFFER_GET_USABLE_OCTETS(r8, r0);
    Null = r8 - ADDR_PER_WORD;
    if NZ call $error;      // usable octets must be 4

    // r8 = ADDR_PER_WORD;     // usable_octets
    r9 = 0xFFFFFFFF;

    call $cbuffer.get_write_address_exx;
    I4 = r0;
    L4 = r2;
    push r3;
    pop B4;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_exx;
    I0 = r0;
    L0 = r2;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_exx;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_exx;

        jump cp_pop_and_exit;

    not_in_place_copy:
    // init base for src ahead of doloop
    push r3;
    pop B0;

    r5 =  M[FP + 3*ADDR_PER_WORD];               // copy amount
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

    r10 = r5 LSHIFT -LOG2_ADDR_PER_WORD;         // convert the copy amount to words
    if Z jump copy_aligned_write_last_octet;     // maybe the copy amount is less than a word
        r10 = r10 - 1;                    // decrement the amount due to the initial read and last write
        r1 = M[I0, ADDR_PER_WORD];         // initial read
        do copy_aligned_loop;
            r1 = M[I0, ADDR_PER_WORD],
             M[I4, ADDR_PER_WORD] = r1;    // read and write
        copy_aligned_loop:
        M[I4, ADDR_PER_WORD] = r1;         // last write

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
    call $cbuffer.set_write_address_exx;

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I0;
    call $cbuffer.set_read_address_exx;

    cp_pop_and_exit:
    // Restore index & length registers
    popm <I0, I4, L0, L4>;
    popm <FP, r0, r1, r2, r5, r6, r7, r8, r9, rLink>;
    rts;
.ENDMODULE;

// --------- copy_unaligned 32-bit ----------
//void cbuffer_copy_unaligned_32bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets);
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of octets to copy
// trashed r3, r10, B0, B4
.MODULE $M.cbuffer_copy_unaligned_32bit_zero_shift_ex;
    .CODESEGMENT BUFFER_PM;
$cbuffer.copy_unaligned_32bit_be_zero_shift_ex:
$_cbuffer_copy_unaligned_32bit_be_zero_shift_ex:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r4, r5, r6, r7, r8, rLink>;
    pushm <I0, I4, L0, L4>;

    BUFFER_GET_USABLE_OCTETS(r4, r0);
    Null = r4 - ADDR_PER_WORD;
    if NZ call $error;
    // r4 = ADDR_PER_WORD;

    // get dest buffer true write address and size
    call $cbuffer.get_write_address_exx;
    I4 = r0;
    L4 = r2;
    r6 = r1;        // save write octet offset
    push r3;
    pop B4;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_exx;
    I0 = r0;
    L0 = r2;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_exx;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_exx;

        jump cp_pop_and_exit;

    not_in_place_copy:
    // init base for src ahead of doloop
    push r3;
    pop B0;
    //push r1;                            // save read octet offset
    r7 = 0xFF; // mask for destination
    r8 = 0xFFFFFFFF;
    r10 = M[FP + 3*ADDR_PER_WORD];        // amount of data to be copied
    do copy_loop;

        r2 = r4 - r1;                     // source
        r2 = r2 - 1;
        r2 = r2 LSHIFT 3;
        r2 = r7 LSHIFT r2;
        r5 = M[I0, 0];
        r5 = r5 AND r2;                   // octet read

        r2 = r1 LSHIFT 3;
        r5 = r5 LSHIFT r2;                // move octet read into the LSB - shift right

        r3 = r6 LSHIFT 3;
        r3 = -r3;
        r5 = r5 LSHIFT r3;                //source octet in the right position for source

        r3 = r4 - r6;                     // us_octets dest
        r3 = r3 - 1;
        r3 = r3 LSHIFT 3;
        r3 = r7 LSHIFT r3;

        r3 = r3 XOR r8;                   // mask for source

        r2 = M[I4, 0];
        r2 = r2 AND r3;                   // protect the bits that aren't to be changed

        r2 = r2 + r5;

        M[I4, 0] = r2;

        r1 = r1 + 1;
        Null = r4 - r1;
        if NZ jump check_dest_offset;
            r1 = 0;
            r3 = M[I0, ADDR_PER_WORD];

        check_dest_offset:
        r6 = r6 + 1;
        Null = r4 - r6;
        if NZ jump loop_end;
            r6 = 0;
            r3 = M[I4, ADDR_PER_WORD];

        loop_end:
        nop;
    copy_loop:

    upd_ptrs:
    r5 = r1;    // save final offset source
    // Update the write address
    r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
    r2 = r6;
    r1 = I4;
    call $cbuffer.set_write_address_exx;
    // r2 is not trashed

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I0;
    r2 = r5;
    call $cbuffer.set_read_address_exx;

    cp_pop_and_exit:
    // Restore index & length registers
    popm <I0, I4, L0, L4>;
    popm <FP, r0, r1, r2, r4, r5, r6, r7, r8, rLink>;
    rts;

.ENDMODULE;
// --------- copy_unpacked_to_packed ----------
//void cbuffer_copy_unpacked_to_packed_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets);
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of octets to copy
// trashed r3, r10, B0, B4
.MODULE $M.cbuffer_copy_unpacked_to_packed_ex;
    .CODESEGMENT BUFFER_PM;
$cbuffer.copy_unpacked_to_packed_ex:
$_cbuffer_copy_unpacked_to_packed_ex:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r4, r5, r6, r7, r8, r9, rLink>;
    pushm <I0, I4, L0, L4>;

    BUFFER_GET_USABLE_OCTETS(r9, r0);           // destination
    Null = r9 - ADDR_PER_WORD;         // usable_octets destination
    if NZ call $error;
    // r9 = ADDR_PER_WORD;

    BUFFER_GET_USABLE_OCTETS(r4, r1);           //source
    Null = r4 - US_OCTETS_16BIT;
    if NZ call $error;


    // get dest buffer true write address and size
    call $cbuffer.get_write_address_exx;
    I4 = r0;
    L4 = r2;
    r6 = r1;        // save write octet offset
    push r3;
    pop B4;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_exx;
    I0 = r0;
    L0 = r2;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_exx;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_exx;

        jump cp_pop_and_exit;

    not_in_place_copy:
    // init base for src ahead of doloop
    push r3;
    pop B0;
    r7 = 0xFFFFFFFF;                // mask for destination
    r8 = r9 - r4;                   // us_octets_dest - us_octets_source
    r10 = M[FP + 3*ADDR_PER_WORD];  // amount of data to be copied
    do copy_loop;

        r2 = r4 - 1;           // source
        r2 = r2 - r1;
        r2 = r2 LSHIFT 3;
        r2 = 0xFF LSHIFT r2;
        r5 = M[I0, 0];
        r5 = r5 AND r2;        // octet read

        r2 = r8 + r1;
        r2 = r2 LSHIFT 3;
        r5 = r5 LSHIFT r2;     // move octet read into the LSB - shift left

        r3 = r6 LSHIFT 3;
        r3 = -r3;
        r5 = r5 LSHIFT r3;     //source octet in the right position for source


        r3 = r9 - r6;          // us_octets dest
        r3 = r3 - 1;
        r3 = r3 LSHIFT 3;
        r3 = 0xFF LSHIFT r3;   // mask for octets before

        r3 = r3 XOR r7;

        r2 = M[I4, 0];
        r2 = r2 AND r3;        // mask the source

        r2 = r2 + r5;          // combine octet read with the source word partially masked

        M[I4, 0] = r2;

        r1 = r1 + 1;
        Null = r4 - r1;
        if NZ jump check_dest_offset;
            r1 = 0;
            r3 = M[I0, ADDR_PER_WORD];

        check_dest_offset:
        r6 = r6 + 1;
        Null = r9 - r6;
        if NZ jump loop_end;
            r6 = 0;
            r3 = M[I4, ADDR_PER_WORD];

        loop_end:
        nop;
    copy_loop:

    upd_ptrs:
    r5 = r1;    // save final offset source
    // Update the write address
    r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
    r2 = r6;
    r1 = I4;
    call $cbuffer.set_write_address_exx;
    // r2 is not trashed

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I0;
    r2 = r5;
    call $cbuffer.set_read_address_exx;

    cp_pop_and_exit:
    // Restore index & length registers
    popm <I0, I4, L0, L4>;
    popm <FP, r0, r1, r2, r4, r5, r6, r7, r8, r9, rLink>;
    rts;

.ENDMODULE;

// --------- copy_packed_to_unpacked ----------
//void cbuffer_copy_packed_to_unpacked_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets);
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of octets to copy
// trashed r3, r10, B0, B4
.MODULE $M.cbuffer_copy_packed_to_unpacked_ex;
    .CODESEGMENT BUFFER_PM;
$cbuffer.copy_packed_to_unpacked_ex:
$_cbuffer_copy_packed_to_unpacked_ex:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r4, r5, r6, r7, r8, r9, rLink>;
    pushm <I0, I4, L0, L4>;

    BUFFER_GET_USABLE_OCTETS(r9, r0);           // destination
    Null = r9 - US_OCTETS_16BIT;
    if NZ call $error;

    BUFFER_GET_USABLE_OCTETS(r4, r1);           // source
    Null = r4 - ADDR_PER_WORD;                                  // usable_octets destination
    if NZ call $error;
    //r4 = ADDR_PER_WORD;

    // get dest buffer true write address and size
    call $cbuffer.get_write_address_exx;
    I4 = r0;
    L4 = r2;
    r6 = r1;        // save write octet offset
    push r3;
    pop B4;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_exx;
    I0 = r0;
    L0 = r2;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_exx;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_exx;

        jump cp_pop_and_exit;

    not_in_place_copy:
    // init base for src ahead of doloop
    push r3;
    pop B0;
    r7 = 0xFF;
    r8 = r9 - r4; // us_octets_dest - us_octets_source

    r10 = M[FP + 3*ADDR_PER_WORD];  // amount of data to be copied
    do copy_loop;
        r2 = ADDR_PER_WORD - r1;    // source
        r2 = r2 - 1;
        r2 = r2 LSHIFT 3;
        r2 = r7 LSHIFT r2;
        r5 = M[I0, 0];
        r5 = r5 AND r2;             // octet read

        r2 = r8 + r1;               // overwritten
        r2 = r2 LSHIFT 3;
        r5 = r5 LSHIFT r2;          // move octet read into the LSB - shift left

        r3 = r6 LSHIFT 3;
        r3 = -r3;
        r5 = r5 LSHIFT r3;          //source octet in the right position for source

        r3 = -r3;
        r3 = r7 LSHIFT r3;          // mask dest

        r2 = M[I4, 0];
        r2 = r2 AND r3;

        r2 = r2 + r5;

        M[I4, 0] = r2;

        r1 = r1 + 1;
        Null = r4 - r1;
        if NZ jump check_dest_offset;
            r1 = 0;
            r3 = M[I0, ADDR_PER_WORD];

        check_dest_offset:
        r6 = r6 + 1;
        Null = r9 - r6;
        if NZ jump loop_end;
            r6 = 0;
            r3 = M[I4, ADDR_PER_WORD];

        loop_end:
        nop;
    copy_loop:

    upd_ptrs:
    r5 = r1;    // save final offset source
    // Update the write address
    r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
    r2 = r6;
    r1 = I4;
    call $cbuffer.set_write_address_exx;
    // r2 is not trashed

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I0;
    r2 = r5;
    call $cbuffer.set_read_address_exx;

    cp_pop_and_exit:
    // Restore index & length registers
    popm <I0, I4, L0, L4>;
    popm <FP, r0, r1, r2, r4, r5, r6, r7, r8, r9, rLink>;
    rts;

.ENDMODULE;

// --------- copy_unpacked_to_packed ----------
//void cbuffer_copy_audio_shift_to_packed(tCbuffer * dst, tCbuffer *src, unsigned num_words); -----------------------------------------------------
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of words to copy
// trashed r3, r10, B0, B4
.MODULE $M.cbuffer_copy_audio_shift_to_packed;
    .CODESEGMENT BUFFER_PM;
$_cbuffer_copy_audio_shift_to_packed:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r4, r5, r6, rLink>;
    pushm <I0, I4, M0, L0, L4>;

    // load a few constants
    M0 = ADDR_PER_WORD;
    r4 = -16;
    // get dest buffer true write address and size
    call $cbuffer.get_write_address_exx;
    I4 = r0;
    L4 = r2;
    r6 = r1;        // save write octet offset
    push r3;
    pop B4;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_exx;
    I0 = r0;
    L0 = r2;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_exx;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_exx;

        jump cp_pop_and_exit;

    not_in_place_copy:
    
    // init base for src ahead of doloop
    push r3;
    pop B0;
    // get the amount of data to copy and set up a few masks
    r5 = M[FP + 3*ADDR_PER_WORD];
    r3 = 0xFFFF0000;
    r2 = 0x0000FFFF;

    Null = r6;
    if Z jump dst_word_aligned;
        // the previous write left the offset mid-word;
        // we first get word aligned to set up an efficient packing loop
        r1 = M[I4, 0];
        r1 = r1 AND r2, r0 = M[I0, M0];
        r0 = r0 AND r3;
        r1 = r1 OR r0;
        M[I4, M0] = r1;
        r5 = r5 - 1;
    dst_word_aligned:
    
    // loop count is half the number of words to pack
    r10 = r5 ASHIFT -1;
    r6 = 0;

    do copy_loop;
        r0 = M[I0, M0];
        r0 = r0 LSHIFT r4, r1 = M[I0, M0];
        r1 = r1 AND r3;
        r1 = r1 OR r0;
        M[I4,  M0] = r1;
    copy_loop:
    
    // if any leftover sample pack it here
    Null = r5 AND 0x1;
    if Z jump upd_ptrs;
        r1 = M[I4, 0];
        r0 = M[I0, M0];
        r1 = r1 AND r3;
        r0 = r0 LSHIFT r4;
        r1 = r1 OR r0;
        M[I4, 0] = r1;
        r6 = 2;

    upd_ptrs:
    // Update the write address
    r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
    r2 = r6;
    r1 = I4;
    call $cbuffer.set_write_address_exx;

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I0;
    r2 = 0;
    call $cbuffer.set_read_address_exx;

    cp_pop_and_exit:
    // Restore index, modify & length registers
    popm <I0, I4, M0, L0, L4>;
    popm <FP, r0, r1, r2, r4, r5, r6, rLink>;
    rts;

.ENDMODULE;

// --------- copy_packed_to_unpacked ----------
//void cbuffer_copy_packed_to_audio_shift(tCbuffer * dst, tCbuffer *src, unsigned num_words); -----------------------------------------------------
// in r0 destination cbuffer address
// in r1 source cbuffer address
// in r2 number of words to copy
// trashed r3, r10, B0, B4
.MODULE $M.cbuffer_copy_packed_to_audio_shift;
    .CODESEGMENT BUFFER_PM;
$_cbuffer_copy_packed_to_audio_shift:
    // save the input paramerters for later
    pushm <FP(=SP), r0, r1, r2, r4, r5, r6, rLink>;
    pushm <I0, I4, M0, L0, L4>;

    // load a few constants
    M0 = ADDR_PER_WORD;
    // get dest buffer true write address and size
    call $cbuffer.get_write_address_exx;
    I0 = r0;
    L0 = r2;
    push r3;
    pop B0;
    r5 = r3;        // prepare for checking for in-place

    // get src buffer read address and size
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    // get the read address and size
    call $cbuffer.get_read_address_exx;
    I4 = r0;
    r6 = r1;        // save read octet offset
    L4 = r2;

    // check if cbuffer base addresses are the same
    Null = r5 - r3;
    if NZ jump not_in_place_copy;
        // only advance pointers, no copy for in-place
        r5 = M[FP + 3*ADDR_PER_WORD];            // copy amount
        r0 = M[FP + 1*ADDR_PER_WORD];            // cbuffer_dest
        r1 = r5;
        call $cbuffer.advance_write_ptr_exx;

        r0 = M[FP + 2*ADDR_PER_WORD];            // cbuffer_src
        r1 = r5;
        call $cbuffer.advance_read_ptr_exx;

        jump cp_pop_and_exit;

    not_in_place_copy:
    // init base for src ahead of doloop
    push r3;
    pop B4;

    // get the amount of data to copy and set up a few masks
    r5 = M[FP + 3*ADDR_PER_WORD];
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
    r10 = r5 ASHIFT -1;
    r6 = 0;

    do copy_loop;
        r2 = M[I4, M0];
        r0 = r2 LSHIFT 16;
        r1 = r2 AND r3, M[I0,  M0] = r0;
        M[I0,  M0] = r1;
    copy_loop:

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
    call $cbuffer.set_write_address_exx;

    // Update the read address
    r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
    r1 = I4;
    r2 = r6;
    call $cbuffer.set_read_address_exx;

    cp_pop_and_exit:
    // Restore index, modify & length registers
    popm <I0, I4, M0, L0, L4>;
    popm <FP, r0, r1, r2, r4, r5, r6, rLink>;
    rts;

.ENDMODULE;

#endif /* INSTALL_CBUFFER_EX */

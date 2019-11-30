// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Connection Buffer Library
//
// DESCRIPTION:
//    This library provides an API for dealing with buffers in Kalimba memory
//    (which are called cbuffers). These buffers may be managed purely by
//    software, or partially controlled by the MMU. The library deals with
//    both cases transparently.
//
//    Functions in this library are split into two types: those that can
//    be called from 'C', and those that are accessed purely from other
//    assembly routines (e.g. capability code). As such, this file forms
//    part of the API as much as cbuffer_c.h does.
//    Note. On Hydra, asm API please be cautious with the TRASHED REGISTERS
//    as many of the functions call into C code for remote buffers (possibly
//    even for local MMU buffers).
//
//    Software buffers (SW - buffers allocated in DSP's own memory) are platform/target
//    independent and handled here, but hardware buffers (HW, MMU buffers
//     - have HW access) are handled in a target specific separate file.
//
//    Once a cbuffer has been initialised, it may be referenced using its
//    buffer structure alone; the size, read and write pointers are all stored
//    and used appropriately.
//
//    When copying data to/from a cbuffer in ASM, operations such as bit-width
//    conversion, DC removal, volume adjustment, filtering and equalisation
//    may be performed using the CBOPS library routines.
//
//    Here is a typical assembly language example of use
//    @verbatim
//       // get the read pointer for $my_cbuffer_struc
//       r0 = $my_cbuffer_struc;
//       call $cbuffer.get_read_address_and_size_and_start_address;
//       I0 = r0;
//       L0 = r1;
//       push r2;
//       pop B0;
//
//       // now read some data from it
//       // NOTE: Should already have checked that there is enough data
//       // in the buffer to be able to read these 10 samples, ie.
//       // using $cbuffer.calc_amount_data.
//       r10 = 10;
//       r5 = 0;
//       do sum_10_samples_loop;
//          r1 = M[I0,ADDR_PER_WORD];
//          r5 = r5 + r1;
//       sum_10_samples_loop:
//       ....
//
//       // now update the stored pointers
//       r0 = $my_cbuffer_struc;
//       r1 = I0;
//       call $cbuffer.set_read_address;
//    @endverbatim
//
//    C use guidance:
//    @verbatim
//        src_cbuffer_ptr = cbuffer_create_with_malloc(buffer_size);
//        ...
//        // check there is enough data in the input buffer
//        amount_data = cbuffer_calc_amount_data_in_words(src_cbuffer_ptr);
//        ...
//        // copy block_size across, assuming block_size > min(amount_data, amount_space)
//        data_processed = cbuffer_copy(dest_cbuffer_ptr, src_cbuffer_ptr, block_size);
//    @endverbatim
//
// NOTE about "words" on different architectures:
//    cbuffers are best viewed as collections of "words" of the processor's
//    native size (24 bits on KAL_ARCH1/2/3/5, 32 bits on KAL_ARCH4). This
//    is the natural unit that the processor is best at dealing with; for
//    instance, there is usually one audio sample per word in a cbuffer. The
//    cbuffer API does not provide for tracking sub-word quantities of data.
//
//    On the 24-bit architecture, each word occupies a single data address
//    (location).
//    However, on 32-bit Kalimbas, data is octet-addressed, so addresses must
//    be incremented by 4 to point to the next word.
//
//    For this reason, functions which talk about relative amounts of data in
//    cbuffers are provided in two variants, e.g.
//      calc_amount_data_in_words  returns the number of words of data
//      calc_amount_data_in_addrs  returns the number of addresses/locations
//                                 (rarely needed)
//    On 24-bit platforms only, unsuffixed names such as
//      calc_amount_data
//    are also provided, for backward compatibility with existing code written
//    only for these platforms (where the distinction doesn't matter). When
//    porting such code to 32-bit platforms, it is necessary to use one of
//    the suffixed names to specify the intended units.
//    When writing new code, use of the suffixed names is recommended.
//
// *****************************************************************************

#include "cbuffer_asm.h"
#include "faultids_asm_defs.h"

#ifdef PATCH_LIBS
#include "patch_asm_macros.h"
#endif

// *****************************************************************************
// The following functions are supported for both SW and MMU buffers (local and remote).
// *****************************************************************************



// *****************************************************************************
// MODULE:
//    asm: $cbuffer.calc_amount_space_in_words
//         $cbuffer.calc_amount_space_in_addrs
//         $cbuffer.calc_amount_space (24-bit only)
//    C: unsigned int cbuffer_calc_amount_space_in_words(tCbuffer *cbuffer);
//       unsigned int cbuffer_calc_amount_space_in_addrs(tCbuffer *cbuffer);
//       unsigned int cbuffer_calc_amount_space(tCbuffer *cbuffer); (24-bit only)
//
// DESCRIPTION:
//    Calculates the amount of space for new data in a cbuffer.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//
// OUTPUTS:
//    - r0 = amount of space (for new data) in words or addresses
//    - r2 = buffer size in words or addresses
//
// TRASHED REGISTERS:
//    r1, (pay attention to r2 "returned", i.e. trashed)
//    CAUTION with Hydra calls from asm for remote buffers (implemented in C),
//             possibly trashing much more (r4,... I0,... ).
//             Caller responsible for preserving what is needed,
//             if necessary use PUSH_ALL_ASM (but don't abuse it).
//
// NOTES:
//    See comments at top of file for the distinction between the three
//    entry points.
//
//    asm version returns r2=size as well for compatibility with legacy asm code
//
// *****************************************************************************
.MODULE $M.cbuffer.calc_amount_space;
   .CODESEGMENT BUFFER_PM;

#ifndef KAL_ARCH4
// On 24-bit Kalimba, there is one addr per word, so we can use the
// same implementation for all entry points.
$_cbuffer_calc_amount_space_in_words:
$cbuffer.calc_amount_space_in_words:
// The ambiguous unsuffixed entry point is only provided on
// 24-bit Kalimba, where the distinction doesn't matter.
$_cbuffer_calc_amount_space:
$cbuffer.calc_amount_space:
#endif // KAL_ARCH4
$_cbuffer_calc_amount_space_in_addrs:
$cbuffer.calc_amount_space_in_addrs:
// KAL_ARCH4 is 32-bit core, 8-bit memory location (32b/8b)
// for KAL_ARCH3 or 5 (24-bit core and memory) the suffix makes no difference

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.calc_amount_space.PATCH_ID_0, r1)
#endif

   // If the buffer is remote, need to call into chip-specific layer
   r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_buff;

#ifdef CHIP_BASE_BC7
   // we are interested in space for a BC write port, this means HW RD port
#ifdef DEBUG_ON
      Null = r1 AND $cbuffer.READ_PTR_TYPE_MASK;
      if Z call $error;
         Null = r1 AND $cbuffer.WRITE_PTR_TYPE_MASK;
         if NZ call $error;
#endif
      // a HW RD port has a wr_ptr = port nr
      r0 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
      jump $cbuffer.calc_amount_space.its_a_port;

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)

      // if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor))
      Null = r1 AND $cbuffer.IS_REMOTE_MMU_MASK;
      if Z jump $mmu.calc_amount_space_local_buff;
         // call into the the C function, r0 is already set
         jump $_cbuffer_calc_amount_space_remote_buff;

#elif defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
      // A7DA_KAS only has sw_buff, so we should not be here.
      call $error;
#endif
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_BC7

   its_a_sw_buff:
   // r2 = M[r0 + $cbuffer.SIZE_FIELD];
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r0);

   // check if the buffer is part of an inplace chain.
   Null = r1 AND $cbuffer.IN_PLACE_MASK;
   if NZ jump its_an_in_place;

   BUFFER_GET_WRITE_ADDR(r1, r0);
   BUFFER_GET_READ_ADDR(r0, r0);

   // calculate the amount of space
   r0 = r0 - r1;
   if LE r0 = r0 + r2;

   // always say it's one word less so that buffer never gets totally filled up
   r0 = r0 - 1*ADDR_PER_WORD;
   rts;

   its_an_in_place:

   r1 = M[r0 +$cbuffer.AUX_ADDR_FIELD];

   //  Not the head of the in pace chain. Use the read pointer
   if Z r1 = r0;

   BUFFER_GET_READ_ADDR(r1, r1);
   BUFFER_GET_WRITE_ADDR(r0, r0);

   // calculate the amount of dat
   // Attention! r1 and r0 is swapped compared with the no in place case.
   r0 = r1 - r0;
   if LE r0 = r0 + r2;

   // always say it's one word less so that buffer never gets totally filled up
   r0 = r0 - 1*ADDR_PER_WORD;
   rts;

#ifdef KAL_ARCH4
// On 32-bit Kalimbas, _in_words returns a different amount from _in_addrs,
// so needs a separate implementation. (See above for entry point on
// 24-bit Kalimbas.)
// FIXME: _in_words is the overwhelmingly common case, so it's a shame it's
//   the one with the extra overhead.

$_cbuffer_calc_amount_space_in_words:
$cbuffer.calc_amount_space_in_words:
    push rLink;
    call $cbuffer.calc_amount_space_in_addrs;
    r0 = r0 ASHIFT -LOG2_ADDR_PER_WORD;
    pop rLink;
    rts;
#endif // KAL_ARCH4

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    asm: $cbuffer.calc_amount_data_in_words
//         $cbuffer.calc_amount_data_in_addrs
//         $cbuffer.calc_amount_data (24-bit only)
//    C: unsigned int cbuffer_calc_amount_data_in_words(tCbuffer *cbuffer);
//       unsigned int cbuffer_calc_amount_data_in_addrs(tCbuffer *cbuffer);
//       unsigned int cbuffer_calc_amount_data(tCbuffer *cbuffer); (24-bit only)
//
// DESCRIPTION:
//    Calculates the amount of data already in a cbuffer.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//
// OUTPUTS:
//    - r0 = amount of data available in words or addresses
//    - r2 = buffer size in words or addresses
//
// TRASHED REGISTERS:
//    r1, (pay attention to r2 "returned", i.e. trashed)
//    CAUTION with Hydra calls from asm for remote buffers (implemented in C),
//             possibly trashing much more (r4,... I0,... ).
//             Caller responsible for preserving what is needed,
//             if necessary use PUSH_ALL_ASM (but don't abuse it).
//
// NOTES:
//    See comments at top of file for the distinction between the three
//    entry points.
//
//    asm version returns r2=size as well for compatibility with legacy asm code
//
// *****************************************************************************
.MODULE $M.cbuffer.calc_amount_data;
   .CODESEGMENT BUFFER_PM;

#ifndef KAL_ARCH4
// On 24-bit Kalimba, there is one addr per word, so we can use the
// same implementation for all entry points.
$_cbuffer_calc_amount_data_in_words:
$cbuffer.calc_amount_data_in_words:
// The ambiguous unsuffixed entry point is only provided on
// 24-bit Kalimba, where the distinction doesn't matter.
$_cbuffer_calc_amount_data:
$cbuffer.calc_amount_data:
#endif // KAL_ARCH4
$_cbuffer_calc_amount_data_in_addrs:
$cbuffer.calc_amount_data_in_addrs:
// KAL_ARCH4 is 32-bit core, 8-bit memory location (32b/8b)
// for KAL_ARCH3 or 5 (24-bit core and memory) the suffix makes no difference

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.calc_amount_data.PATCH_ID_0, r1)
#endif

   // If the buffer is remote, need to call into chip-specific layer
   r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_buff;

#ifdef CHIP_BASE_BC7
   // we are interested in data for a BC read port, this means HW WR port
#ifdef DEBUG_ON
      Null = r1 AND $cbuffer.READ_PTR_TYPE_MASK;
      if NZ call $error;
         Null = r1 AND $cbuffer.WRITE_PTR_TYPE_MASK;
         if Z call $error;
#endif
      // a HW WR port has a rd_ptr = port nr
      r0 = M[r0 + $cbuffer.READ_ADDR_FIELD];
      jump $cbuffer.calc_amount_data.its_a_port;

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)

      // if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor))
      Null = r1 AND $cbuffer.IS_REMOTE_MMU_MASK;
      if Z jump $mmu.calc_amount_data_local_buff;
         // call into the C function, r0 is already set
         jump $_cbuffer_calc_amount_data_remote_buff;

#elif defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
      // A7DA_KAS only has sw_buff, so we should not be here.
      call $error;
#endif
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_BC7

   its_a_sw_buff:
   // r2 = M[r0 + $cbuffer.SIZE_FIELD];
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r2, r0);
   BUFFER_GET_WRITE_ADDR(r1, r0);
   BUFFER_GET_READ_ADDR(r0, r0);

   // calculate the amount of data
   r0 = r1 - r0;
   if NEG r0 = r0 + r2;
   rts;


#ifdef KAL_ARCH4
// On 32-bit Kalimbas, _in_words returns a different amount from _in_addrs,
// so needs a separate implementation. (See above for entry point on
// 24-bit Kalimbas.)
// FIXME: _in_words is the overwhelmingly common case, so it's a shame it's
//   the one with the extra overhead.
$_cbuffer_calc_amount_data_in_words:
$cbuffer.calc_amount_data_in_words:
    push rLink;
    call $cbuffer.calc_amount_data_in_addrs;
    r0 = r0 ASHIFT -LOG2_ADDR_PER_WORD;
    pop rLink;
    rts;
#endif // KAL_ARCH4

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    asm: $cbuffer.advance_read_ptr
//    C: void cbuffer_advance_read_ptr(tCbuffer *cbuffer, int amount_in_spl);
//
// DESCRIPTION:
//    Advances read pointer of a cbuffer by a supplied value
//
// INPUTS:
//    - r0 = pointer to cbuffer struc
//    - r1 = number by which read pointer would be advanced (in samples)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-2, (r3, r10 if port), M3
//    CAUTION with Hydra calls from asm for non-SW buffers (implemented in C),
//             possibly trashing much more (r4,... I0,... ).
//             Caller responsible for preserving what is needed,
//             if necessary use PUSH_ALL_ASM (but don't abuse it).
//
// *****************************************************************************
.MODULE $M.cbuffer.advance_read_ptr;
   .CODESEGMENT EXT_DEFINED_PM;

$_cbuffer_advance_read_ptr:
$cbuffer.advance_read_ptr:

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.advance_read_ptr.PATCH_ID_0, r2)
#endif

   r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r2 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump sw_handle;

#ifdef CHIP_BASE_BC7
       // convert the amount so that it is in the same units as addresses (pointers)
       BUFFER_WORDS_TO_ADDRS_ASM(r1);
       M3 = r1;
       push rLink;

#ifdef DEBUG_ON
       // we should not change RD pointer of an output port (HW RD)
       Null = r2 AND $cbuffer.READ_PTR_TYPE_MASK;
       if NZ call $error;   // panic if attempt to modify a HW handle
#endif

       r10 = r1;
#ifdef DEBUG_ON
       if NEG call $error;  // negative do-loop counters not a good idea
#endif

       push r0;
       call $cbuffer.get_read_address.its_a_port;
       do read_port_loop;
          Null = M[r0];
       read_port_loop:

       pop r0;
       call $cbuffer.set_read_address.its_a_port;
       pop  rLink;
       rts;

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
   // make sure it is a RD handle or remote,
   // we will not check again in platform specific layer
   Null = r2 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ jump $_cbuffer_advance_read_ptr_mmu_buff;

   // for Hydra the concept of "owning" the buffer is not as strict as in BC;
   // an MMU buffer (controlled by the BAC hardware) is not necessarily given
   // to a HW instance, so we are allowed to use mmu functions to operate
   // on those handles
   Null = r2 AND $cbuffer.READ_PTR_TYPE_MASK;
   if NZ jump $_cbuffer_advance_read_ptr_mmu_buff;
       // since it's not a pure SW buffer, it must be a HW WR
       // handle it the same way nevertheless
       // so just trickle through to SW pointer advancing below
#elif defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
      // A7DA_KAS only has sw_buff, so we should not be here.
      call $error;
#endif
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_HYDRA

   sw_handle:
   // convert the amount so that it is in the same units as addresses (pointers)
   BUFFER_WORDS_TO_ADDRS_ASM(r1);
   M3 = r1;
   // pure SW buffer or a hydra SW handle
   push rLink;
   pushm <I0, L0>;
   push r0;     // save cbuffer pointer for later
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   r0 = M[I0,M3];

   pop r0;     // restore cbuffer pointer
   r1 = I0;
   call $cbuffer.set_read_address;

       popm <I0, L0>;
       pop  rLink;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    asm: $cbuffer.advance_write_ptr
//    C: void cbuffer_advance_write_ptr(tCbuffer *cbuffer, int amount_in_spl);
//
// DESCRIPTION:
//    Advances write pointer of a cbuffer by a supplied value
//
// INPUTS:
//    - r0 = pointer to cbuffer struc
//    - r1 = number by which write pointer would be advanced (in samples)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-2, (r3, r10 if port), M3
//    CAUTION with Hydra calls from asm for non-SW buffers (implemented in C),
//             possibly trashing much more (r4,... I0,... ).
//             Caller responsible for preserving what is needed,
//             if necessary use PUSH_ALL_ASM (but don't abuse it).
//
// *****************************************************************************
.MODULE $M.cbuffer.advance_write_ptr;
   .CODESEGMENT EXT_DEFINED_PM;

$_cbuffer_advance_write_ptr:
$cbuffer.advance_write_ptr:

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.advance_write_ptr.PATCH_ID_0, r2)
#endif

   r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r2 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump sw_handle;

#ifdef CHIP_BASE_BC7
       // convert the amount so that it is in the same units as addresses (pointers)
       BUFFER_WORDS_TO_ADDRS_ASM(r1);
       M3 = r1;
       push rLink;

#ifdef DEBUG_ON
       // we should not change WR pointer of an input port (HW WR)
       Null = r2 AND $cbuffer.WRITE_PTR_TYPE_MASK;
       if NZ call $error;   // panic if attempt to modify a HW handle
#endif

       r10 = r1;
#ifdef DEBUG_ON
       if NEG call $error;  // negative do-loop counters not a good idea
#endif
       push r0;
       call $cbuffer.get_write_address.its_a_port;
       do write_port_loop;
          M[r0] = Null;     // effectively writing zeros to port; a/u-Law silence, TODO B-182237
       write_port_loop:

       pop r0;
       call $cbuffer.set_write_address.its_a_port;
       pop  rLink;
       rts;

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
   // make sure it is a WR handle or remote,
   // we will not check again in platform specific layer
   Null = r2 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ jump $_cbuffer_advance_write_ptr_mmu_buff;

   // for Hydra the concept of "owning" the buffer is not as strict as in BC;
   // an MMU buffer (controlled by the BAC hardware) is not necessarily given
   // to a HW instance, so we are allowed to use mmu functions to operate
   // on those handles
   Null = r2 AND $cbuffer.WRITE_PTR_TYPE_MASK;
   if NZ jump $_cbuffer_advance_write_ptr_mmu_buff;
       // since it's not a pure SW buffer, it must be a HW RD
       // handle it the same way nevertheless
       // so just trickle through to SW pointer advancing below
#elif defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
      // A7DA_KAS only has sw_buff, so we should not be here.
      call $error;
#endif
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP

   sw_handle:
   BUFFER_WORDS_TO_ADDRS_ASM(r1);
   M3 = r1;
   // pure SW buffer or a hydra SW handle
   push rLink;
   pushm <I0, L0>;
   push r0;     // save cbuffer pointer for later
   call $cbuffer.get_write_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   r0 = M[I0,M3];

   pop r0;     // restore cbuffer pointer
   r1 = I0;
   call $cbuffer.set_write_address;

   popm <I0, L0>;
   pop  rLink;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    C: void cbuffer_block_fill(tCbuffer *cbuffer, unsigned int amount_in_spl, unsigned int value);
//
// DESCRIPTION:

//    Fill a number of locations in a buffer with a specified value, from current
//    write pointer onwards. It is equivalent to allocating some pre-filled buffer then
//    performing cbuffer write/copy operation to target buffer, obviously much faster.
//
// INPUTS:
//    - r0 = pointer to cbuffer struc
//    - r1 = number of locations to fill from current write pointer onwards (in samples)
//    - r2 = value to write
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-3, r10
//    CAUTION with Hydra calls from asm for remote buffers (implemented in C),
//             possibly trashing much more (r4,... I0,... ).
//             Caller responsible for preserving what is needed,
//             if necessary use PUSH_ALL_ASM (but don't abuse it).
//
// NOTES:
//    Local buffers only (remote on Hydra not supported yet)
//
// *****************************************************************************
.MODULE $M.cbuffer.cbuffer_block_fill;
   .CODESEGMENT EXT_DEFINED_PM;

$_cbuffer_block_fill:
$cbuffer.block_fill:
    r3 = r1;            // save for later

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.cbuffer_block_fill.PATCH_ID_0, r1)
#endif

#ifdef CHIP_BASE_BC7
    // shouldn't fill an input port
    r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
    Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
    if Z jump not_rd_port;
    Null = r1 AND $cbuffer.WRITE_PTR_TYPE_MASK;
        if Z jump not_rd_port;
            // log_fault_and_exit
            r0 = $faultids.FAULT_AUDIO_DEBUG_BUFFER_ACCESS;
            jump $_fault_diatribe;  // return will take us back to caller
    not_rd_port:
#endif

    push rLink;
    push r0;

    // save registers trashed by calc_amount_x
    r10 = r2;
    call $cbuffer.calc_amount_space_in_words;
    Null = r0 - r3;
#ifdef DEBUG_ON
    if NEG call $error;     // a bit harsh, caller should normally make sure
                          // there is enough space
#endif
    if NEG jump log_fault_and_exit;  // return doing nothing if not enough space

        pop r0;
        r1 = r3;
        r2 = r10;
        call $cbuffer.private.block_fill_unsafe;

    block_fill_exit:
        pop rLink;
        rts;

    log_fault_and_exit:
        r1 = r0;
        pop r0;
        r0 = $faultids.FAULT_AUDIO_DEBUG_BUFFER_NOT_ENOUGH_SPACE;
        call $_fault_diatribe;
        jump block_fill_exit;


// private function
// di: r0 = cbuffer pointer
//     r1 = block_size fo fill (in samples)
//     r2 = fill value
// trash: r0-3, r10
$cbuffer.private.block_fill_unsafe:

#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.cbuffer_block_fill.PATCH_ID_1, r3)
#endif

    push rLink;
    pushm <I0, L0>;

    r10 = r1;
#ifdef DEBUG_ON
    if NEG call $error;  // negative do-loop counters not a good idea
#endif
    r3 = r2;

    r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];

#ifdef CHIP_BASE_BC7

#ifdef DEBUG_ON
    // we should not write to an input port (HW WR)
    Null = r2 AND $cbuffer.WRITE_PTR_TYPE_MASK;
    if NZ call $error;   // panic if attempt to modify a HW handle
#endif      // DEBUG_ON

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_A7DA_KAS) || defined(CHIP_BASE_NAPIER)
    Null = r2 AND $cbuffer.IS_REMOTE_MMU_MASK;
    if Z jump do_fill_local_buff;
        // remote buffers not supported yet
        r1 = r0;
        r0 = $faultids.FAULT_AUDIO_UNSUPPORTED;
        call $_fault_diatribe;
        jump pop_and_exit;
    do_fill_local_buff:
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP

    push r0;
    call $cbuffer.get_write_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   // write the value to the local buffer
   do fill_local_buff;
      M[I0,1*ADDR_PER_WORD] = r3;
   fill_local_buff:

    pop r0;     // restore cbuffer pointer
   r1 = I0;
    call $cbuffer.set_write_address;

    pop_and_exit:
    popm <I0, L0>;
    pop  rLink;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    asm: $cbuffer.discard_data
//    C: void cbuffer_discard_data(tCbuffer *cbuffer, unsigned int amount);
//
// DESCRIPTION:
//    Discard a certain amount of data from a buffer,
//    assuming there is that much data available,
//    otherwise does nothing (FAULT logged)
//    It moves the read pointer by the specified amount.
// INPUTS:
//    - r0 = pointer to cbuffer struc
//    - r1 = number of samples to discard.
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-3, (r10 if port), M3
//    CAUTION with Hydra calls from asm for non-SW buffers (implemented in C),
//             possibly trashing much more (r4,... I0,... ).
//             Caller responsible for preserving what is needed,
//             if necessary use PUSH_ALL_ASM (but don't abuse it).
//
// NOTES:
//    - Based on advance_read_ptr.
//    - If somebody owns that read pointer, it needs to be
//    updated about its position.
//
// *****************************************************************************
.MODULE $M.cbuffer.discard_data;
   .CODESEGMENT EXT_DEFINED_PM;

$cbuffer.discard_data:
$_cbuffer_discard_data:

#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.discard_data.PATCH_ID_0, r3)
#endif

    r3 = r1;        // save for later

#ifdef CHIP_BASE_BC7
    // shouldn't discard data from an output port
    r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
    Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
    if Z jump not_wr_port;
    Null = r1 AND $cbuffer.READ_PTR_TYPE_MASK;
        if Z jump not_wr_port;
            // log_fault_and_exit
            r0 = $faultids.FAULT_AUDIO_DEBUG_BUFFER_ACCESS;
            jump $_fault_diatribe;  // return will take us back to caller
    not_wr_port:
#endif

   push rLink;
    push r0;

    call $cbuffer.calc_amount_data_in_words;
    Null = r0 - r3;
#ifdef DEBUG_ON
    if NEG call $error;     // a bit harsh, caller should normally make sure
                            // there is enough data
#endif
    if NEG jump log_fault_and_exit;  // return doing nothing if not enough data
        r1 = r3;
        pop r0;
        call $cbuffer.advance_read_ptr;

    discard_exit:
   pop rLink;
   rts;


    log_fault_and_exit:
        r1 = r0;
        pop r0;             // just discard stack
        r0 = $faultids.FAULT_AUDIO_DEBUG_BUFFER_NOT_ENOUGH_DATA;
        call $_fault_diatribe;
        jump discard_exit;


.ENDMODULE;



/* High-level data access functions
 */
.MODULE $M.buffer_c_stubs;
   .CODESEGMENT BUFFER_PM;


// unsigned int cbuffer_read(tCbuffer *cbuffer, int* buffer, unsigned int size);
// reads data from a cbuffer into 'buffer';
// returns the amount read as min(size,amount_in_cbuffer)

$_cbuffer_read:

#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.cbuffer_read.PATCH_ID_0, r3)
#endif

#if defined(CHIP_BASE_BC7) || defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ call $error;    // no remote buffers on BC7 or KAS
#endif

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
#ifndef TODO_CRESCENDO_BUFFER
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ jump $_cbuffer_read_remote_buff;
#endif      // TODO_CRESCENDO_BUFFER
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_BC7

   // save r0=*cbuffer for later (this stack location will hold the return value as well)
   pushm <FP(=SP), r0, rLink>;
   pushm <I0, I4, L0>;
   I4 = r1;
   r10 = r2 LSHIFT LOG2_ADDR_PER_WORD;
   call $cbuffer.calc_amount_data_in_addrs;
   Null = r10 - r0;
   if POS r10 = r0;
   r0 = M[FP + 1*ADDR_PER_WORD];                // restore cbuffer pointer

   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   r0 = M[FP + 1*ADDR_PER_WORD];                // restore cbuffer pointer needed for updating pointer
   BUFFER_ADDRS_TO_WORDS_ASM(r10);              // loop counter in CPU units
   M[FP + 1*ADDR_PER_WORD] = r10;               // store the return value in the stack
#ifdef DEBUG_ON
   if NEG call $error;  // something went wrong, we cannot possibly doloop such a big block of data
#endif
   if Z jump rd_pop_and_exit;
      r10 = r10 - 1;
      r1 = M[I0,ADDR_PER_WORD];
      do rd_loop;
         r1 = M[I0,ADDR_PER_WORD],
          M[I4,ADDR_PER_WORD] = r1;
      rd_loop:
      M[I4,ADDR_PER_WORD] = r1;
      r1 = I0;
      call $cbuffer.set_read_address;
   rd_pop_and_exit:
   popm <I0, I4, L0>;
   popm <FP, r0, rLink>;
   rts;


// unsigned int cbuffer_write(tCbuffer *cbuffer, int* buffer, unsigned int size);
// writes data from a 'buffer' into cbuffer;
// returns the amount written as min(size,space_in_cbuffer)

$_cbuffer_write:

#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.cbuffer_write.PATCH_ID_0, r3)
#endif

#if defined(CHIP_BASE_BC7) || defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ call $error;    // no remote buffers on BC7 or KAS
#endif

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
#ifndef TODO_CRESCENDO_BUFFER
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ jump $_cbuffer_write_remote_buff;
#endif      // TODO_CRESCENDO_BUFFER
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_BC7

   // save r0=*cbuffer for later (this stack location will hold the return value as well)
   pushm <FP(=SP), r0, rLink>;
   pushm <I0, I4, L0>;
   I4 = r1;
   r10 = r2 LSHIFT LOG2_ADDR_PER_WORD;
   call $cbuffer.calc_amount_space_in_addrs;
   Null = r10 - r0;
   if POS r10 = r0;
   r0 = M[FP + 1*ADDR_PER_WORD];                // restore cbuffer pointer

   call $cbuffer.get_write_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   r0 = M[FP + 1*ADDR_PER_WORD];                // restore cbuffer pointer needed for updating pointer
   BUFFER_ADDRS_TO_WORDS_ASM(r10);              // loop counter in CPU units
   M[FP + 1*ADDR_PER_WORD] = r10;               // store the return value in the stack
#ifdef DEBUG_ON
   if NEG call $error;  // something went wrong, we cannot possibly doloop such a big block of data
#endif
   if Z jump wr_pop_and_exit;
      r10 = r10 - 1;
      r1 = M[I4,ADDR_PER_WORD];
      do wr_loop;
         r1 = M[I4,ADDR_PER_WORD],
          M[I0,ADDR_PER_WORD] = r1;
      wr_loop:
      M[I0,ADDR_PER_WORD] = r1;
      r1 = I0;
      call $cbuffer.set_write_address;
   wr_pop_and_exit:
   popm <I0, I4, L0>;
   popm <FP, r0, rLink>;
   rts;


// unsigned int cbuffer_copy(tCbuffer *cbuffer_dest, tCbuffer *cbuffer_src, unsigned int amount_to_copy);
// copies amount_to_copy words from cbuffer_src to cbuffer_dest;
// returns amount successfully copied across

$_cbuffer_copy:

#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.cbuffer_copy.PATCH_ID_0, r3)
#endif

#if defined(CHIP_BASE_BC7) || defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ call $error;    // no remote buffers on BC7 or KAS
   r3 = M[r1 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ call $error;    // no remote buffers on BC7 or KAS
#endif

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
#ifndef TODO_CRESCENDO_BUFFER
   r3 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ jump $_cbuffer_copy_remote_buff;
   r3 = M[r1 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r3 AND $cbuffer.IS_REMOTE_MMU_MASK;
   if NZ jump $_cbuffer_copy_remote_buff;
#endif      // TODO_CRESCENDO_BUFFER
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_BC7

   // save r0=*dest and r1=*src cbuffers for later
   //    also first (after FP, i.e. r0) stack location will hold the return value as well
   pushm <FP(=SP), r0, r1, rLink>;
   pushm <I0, I4, L0, L4>;

   // validate amount_to_copy
   r10 = r2 LSHIFT LOG2_ADDR_PER_WORD;
   call $cbuffer.calc_amount_space_in_addrs;
   Null = r10 - r0;
   if POS r10 = r0;
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   call $cbuffer.calc_amount_data_in_addrs;
   Null = r10 - r0;
   if POS r10 = r0;

   r0 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
   call $cbuffer.get_write_address_and_size_and_start_address;
   I4 = r0;
   L4 = r1;
   push r2;
   pop B4;
   // save the base address in r3
   r3 = r2;

   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   // check if base is the same
   Null = r2 - r3;
   if NZ jump not_an_in_place_copy;


#ifdef DEBUG_ON
   // check if the destination is marked as in place
   r1 = M[FP + 1*ADDR_PER_WORD];                // cbuffer_dest
   r0 = M[r1 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r0 AND $cbuffer.IN_PLACE_MASK;
   if Z call $error;
   // The destination must not be the beginning of the in place chain.
   r0 = M[r1 + $cbuffer.AUX_ADDR_FIELD];
   if NZ call $error;
   // save the write pointer for further analysis
   BUFFER_GET_WRITE_ADDR(r3, r1)

   // check if the source is marked as in place
   r1 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   r0 = M[r1 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r0 AND $cbuffer.IN_PLACE_MASK;
   if Z call $error;

   // check if one buffer follows the other in the in place chain. The read address
   // of the destination is the same as the write address of the source)
   BUFFER_GET_READ_ADDR(r0, r1);
   Null = r0 - r3;
   if NZ call $error;
#endif

   // update the buffer and exit.
   BUFFER_ADDRS_TO_WORDS_ASM(r10);

   r0 = M[FP + 1*ADDR_PER_WORD];             // cbuffer_dest
   r1 = r10;
   call $_cbuffer_advance_write_ptr;

   r0 = M[FP + 2*ADDR_PER_WORD];             // cbuffer_src
   r1 = r10;
   call $_cbuffer_advance_read_ptr;

   M[FP + 1*ADDR_PER_WORD] = r10;               // store the return value in the stack
   jump cp_pop_and_exit;


not_an_in_place_copy:


   r0 = M[FP + 1*ADDR_PER_WORD];                // early restore cbuffer_dest pointer needed for updating pointer
   BUFFER_ADDRS_TO_WORDS_ASM(r10);              // loop counter in CPU units
   M[FP + 1*ADDR_PER_WORD] = r10;               // store the return value in the stack
#ifdef DEBUG_ON
   if NEG call $error;  // something went wrong, we cannot possibly doloop such a big block of data
#endif
   if Z jump cp_pop_and_exit;
      r10 = r10 - 1;
      r1 = M[I0,ADDR_PER_WORD];
      do copy_loop;
         r1 = M[I0,ADDR_PER_WORD],
          M[I4,ADDR_PER_WORD] = r1;
      copy_loop:
      M[I4,ADDR_PER_WORD] = r1;

      r1 = I4;
      call $cbuffer.set_write_address;
      r0 = M[FP + 2*ADDR_PER_WORD];             // cbuffer_src
      r1 = I0;
      call $cbuffer.set_read_address;

   cp_pop_and_exit:
   // Restore index & length registers
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, rLink>;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    cbuffer_move_write_to_read_point(tCbuffer*, unsigned int distance)
//
// DESCRIPTION:
//    Moves the write pointer toward the read pointer, leaving a certain
//    distance between them.
//    Evidently, this function has to be called in cases where the write pointer
//    can be modified by SW and owned by the caller entity.
//    NOTE: On BC, if it is a port, then it will have to write zeros to move the write
//    pointer along.
//    Also, one BC side-effect is that unless this function is used to make a port
//    buffer look empty (and then copy a block of data to it), the writing is done with
//    a zero value that is not conditioned by data type in use. Therefore under the
//    cbuffer tidy-up, whilst rationalising a number of functions for advancing, filling,
//    and other such operations, this could be also done under this or some other
//    function that takes a "zero value" argument if these move functions are deprecated.
//    NOTE: If distance > buffer size, it will limit it to buffer size!
//
// INPUTS:
//    - r0 = Cbuffer structure pointer
//    - r1 = distance between read and write position
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, (r10 on ports)
//
// *****************************************************************************
.MODULE $M.cbuffer.move_write_to_read_point;
   .CODESEGMENT EXT_DEFINED_PM;

 $_cbuffer_move_write_to_read_point:

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.move_write_to_read_point.PATCH_ID_0, r3)
#endif

   pushm <r4, r5, rLink>;
   r4 = r0;
   r5 = r1 ASHIFT LOG2_ADDR_PER_WORD;
   pushm <I0, M0, L0>;

#ifdef CHIP_BASE_BC7

   // is it pure SW buffer or we have a port wrapped in it?
   r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r2 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump not_port;

      // for a port it's a bit more work, we actually have to write the data
      // You can't calc amount of data on a write port because of paging. So
      // we'll assume buffer size - calc_amount_space is a valid course of action.
      // This is only valid when the port buffer is not paged.

      //r0 is still the same as r4 so no need to do: r0 = r4;
      call $cbuffer.calc_amount_space;
      r2 = r2 ASHIFT -1; // port buffer size is in octets convert to words

#ifdef DEBUG_ON
      if Z call $error;  // this happens when port is not configured yet
#endif

      Null = r2 - r5;
      if GE jump move_port;
         // changed distance to size-1, as if we were filling the buffer
         pushm <r0, r1, r2>;
         r0 = $faultids.FAULT_AUDIO_DEBUG_BUFF_PRIMING_ABOVE_BUFF_SIZE;
         r1 = r5;
         call $_fault_diatribe;
         popm <r0, r1, r2>;
         r5 = r2 - 1;

      move_port:
      r1 = r2 - r0;
      r1 = r1 - 1;  // Amount of space returns real space -1 so remove the extra sample

      // if amount of data is less than desired distance, wr pointer moves forward
      // if more than the distance, then wr ptr has to move backward so we make it wrap
      Null = r1 - r5;
      if GT jump more_data;
      if EQ jump exit;

         r10 = r5 - r1; // number of writes: distance - amount_data
         jump move_wr_ptr;

         more_data:
         r10 = r0 + r5; // number of writes: amount of space + desired distance + 2,
         r10 = r10 + 1; // because amount of space is returned as real space - 1...
         // The extra one position (hence +2 instead of +1) is currently needed,
         // otherwise amount of space returns
         // not what is expected... e.g. for distance 0 i.e. align rd and wr pointers, one
         // expects amount of space to be size-1 right after this, i.e. totally empty buffer.
         // Instead, we see amount of space 0... which then has bad side-effects for copy
         // operations that try to figure out whether there is space to write into.

   move_wr_ptr:
      // get the write address. R0 is trashed by previous cbuffer func call, restore it
      r0 = r4;
      call $cbuffer.get_write_address_and_size_and_start_address;

#ifdef DEBUG_ON
      Null = r10 - 0;
      if NEG call $error;           // negative do-loop counters not a good idea
#endif

      // go round and write the dummy data
      do empty_port_loop;
         M[r0] = Null;
      empty_port_loop:

      jump set_address_and_exit;

   not_port:

#elif !defined(CHIP_BASE_HYDRA) && !defined(CHIP_BASE_A7DA_KAS) && !defined(CHIP_BASE_NAPIER)

#error "Buffer TAL not implemented for this chip"

#endif
   // move the read pointer to the write pointer

   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   Null = r1 - r5;
   if GE jump move_ptr;
   // distance is bigger than size, so limit it to size-1 as it we were filling it

   r5 = r1 - 1*ADDR_PER_WORD;

   move_ptr:
       // Calculate new write point (read point + distance)
   M0 = r5;
   r1 = M[I0, M0];
   r1 = I0;

   set_address_and_exit:
       r0 = r4;
   call $cbuffer.set_write_address;

   exit:
   popm <I0, M0, L0>;
   popm <r4, r5, rLink>;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    cbuffer_move_read_to_write_point(tCbuffer*, unsigned int distance)
//
// DESCRIPTION:
//    Moves the read pointer toward the write pointer, leaving a certain
//    distance between them. Normally one would use a distance of zero, e.g. for
//    emptying the buffer, so then read positions snaps to the current write
//    position.
//    Evidently, this function has to be called in cases where the read pointer
//    can be modified by SW and owned by the caller entity.
//
// INPUTS:
//    - r0 = Cbuffer structure pointer
//    - r1 = distance between read and write position
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, (r10 on ports)
//
// *****************************************************************************
.MODULE $M.cbuffer.move_read_to_write_point;
   .CODESEGMENT EXT_DEFINED_PM;

   $_cbuffer_move_read_to_write_point:

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.move_read_to_write_point.PATCH_ID_0, r3)
#endif

   pushm <r4, r5, rLink>;
   r4 = r0;
   r5 = r1 ASHIFT LOG2_ADDR_PER_WORD;
   pushm <I0, M0, L0>;

#ifdef CHIP_BASE_BC7

   // is it pure SW buffer or we have a port wrapped in it?
   r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r2 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump not_port;

      // for a port it's a bit more work, we actually have to read the data.
      // Is the desired distance less or more than amount of data in buffer?
      // Also, we get size in octets in r2
      call $cbuffer.calc_amount_data;
      r10 = r0 - r5;

      // move pointer to the specified distance.
      // Get the read address. The number of times to read is in r10
      // (which was amount of data minus required distance).
      // If negative, we intentionally wrap around to achieve what we need
      if GT jump do_move;

      // size in words
      r2 = r2 ASHIFT -1;
      // make the wrap happen: number of reads will be buffer_size + (amount_data - distance)
          r10 = r10 + r2;
      do_move:
      r0 = r4;
      call $cbuffer.get_read_address_and_size_and_start_address;

      // go round and read the data
      do empty_port_loop;
         Null = M[r0];
      empty_port_loop:

      jump set_address_and_exit;

   not_port:

#elif !defined(CHIP_BASE_HYDRA) && !defined(CHIP_BASE_A7DA_KAS) && !defined(CHIP_BASE_NAPIER)

#error "Buffer TAL not implemented for this chip"

#endif

   // move the read pointer to the write pointer
   call $cbuffer.get_write_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   // set read point to be (write point - distance)
      M0 = -r5;
   r1 = M[I0, M0];
   r1 = I0;

   set_address_and_exit:
      // update the read address, r1 doesn't matter in port case
   r0 = r4;
   call $cbuffer.set_read_address;

   exit:
   popm <I0, M0, L0>;
   popm <r4, r5, rLink>;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    void cbuffer_empty_buffer(tCbuffer *cbuffer);
//
// DESCRIPTION:
//    Empties all the available data in the supplied cbuffer.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-2, (r3, r10 if port), M3
//    CAUTION with Hydra calls from asm for non-SW buffers (implemented in C),
//             possibly trashing much more (r4,... I0,... ).
//             Caller responsible for preserving what is needed,
//             if necessary use PUSH_ALL_ASM (but don't abuse it).
//
// NOTES:
//    - Interrupts should be blocked before this call, unless you "own" the
// cbuffer read pointer.
//    - Based on cbuffer.advance_read_ptr
//
// *****************************************************************************
.MODULE $M.cbuffer.empty_buffer;
   .CODESEGMENT EXT_DEFINED_PM;

$_cbuffer_empty_buffer:
$cbuffer.empty_buffer:

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.empty_buffer.PATCH_ID_0, r1)
#endif

#ifdef CHIP_BASE_BC7
    // shouldn't empty an output port
    r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
    Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
    if Z jump not_wr_port;
    Null = r1 AND $cbuffer.READ_PTR_TYPE_MASK;
        if Z jump not_wr_port;
            // log_fault_and_exit
            r0 = $faultids.FAULT_AUDIO_DEBUG_BUFFER_ACCESS;
            jump $_fault_diatribe;  // return will take us back to caller
    not_wr_port:
#endif

   push rLink;
    push r0;

    call $cbuffer.calc_amount_data_in_words;

   r1 = r0;
   pop r0;
    call $cbuffer.advance_read_ptr;

    pop rLink;
    rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    void cbuffer_fill_buffer(tCbuffer *cbuffer, int fill_value);
//
// DESCRIPTION:
//    Fills all the available space in a cbuffer with a supplied value.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//    - r1 = value to write into buffer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-3, r10
//    CAUTION with Hydra calls from asm for remote buffers (implemented in C),
//             possibly trashing much more (r4,... I0,... ).
//             Caller responsible for preserving what is needed,
//             if necessary use PUSH_ALL_ASM (but don't abuse it).
//
// NOTES:
//    - Interrupts must be blocked during this call.
//    - Based on cbuffer.fill_buffer
//
// *****************************************************************************
.MODULE $M.cbuffer.fill_buffer;
   .CODESEGMENT EXT_DEFINED_PM;

$_cbuffer_fill_buffer:
$cbuffer.fill_buffer:

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.fill_buffer.PATCH_ID_0, r3)
#endif

   push rLink;

    r3 = r1;    // fill value
    r10 = r0;

#ifdef CHIP_BASE_BC7
    // shouldn't fill an input port
    r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
    Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
    if Z jump not_rd_port;
    Null = r1 AND $cbuffer.WRITE_PTR_TYPE_MASK;
        if Z jump not_rd_port;
            // log_fault_and_exit
            r0 = $faultids.FAULT_AUDIO_DEBUG_BUFFER_ACCESS;
            call $_fault_diatribe;
            jump fill_end;
    not_rd_port:
#endif

   call $cbuffer.calc_amount_space_in_words;
    r1 = r0;
    r0 = r10;
    r2 = r3;
    call $cbuffer.private.block_fill_unsafe;

    fill_end:
    pop rLink;
    rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    asm: $cbuffer.flush_and_fill
//    C:   void cbuffer_flush_and_fill(tCbuffer *cbuffer, int fill_value);
//
// DESCRIPTION:
//    Completely fills a cbuffer with a supplied value, empties it,
//    disregarding any data that might be in there.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//    - r1 = value to write into buffer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0 - r3, r10, DoLoop
//    CAUTION with Hydra calls from asm for remote buffers (implemented in C),
//             possibly trashing much more (r4,... I0,... ).
//             Caller responsible for preserving what is needed,
//             if necessary use PUSH_ALL_ASM (but don't abuse it).
//
// NOTES:
//    This routine fills a cbuffer with a supplied value, trashing any data
//    already in it.
//
//    Based on cbuffer.block_fill, extending the block to whole size.
//    It doesn't care about available space though, just flushes.
//
// *****************************************************************************
.MODULE $M.cbuffer.flush_and_fill;
   .CODESEGMENT EXT_DEFINED_PM;

$_cbuffer_flush_and_fill:
$cbuffer.flush_and_fill:

#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.flush_and_fill.PATCH_ID_0, r3)
#endif

    push rLink;
    r3 = r0;         // save for later

    // get the buffer size
#ifdef CHIP_BASE_BC7
    // ports will report size=1
      r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
      Null = r2 AND $cbuffer.BUFFER_TYPE_MASK;
    if Z jump not_port;
        push r1;
        // not really interested in amount_data / space,
        // just MMU buffer size (in octets)
        Null = r2 AND $cbuffer.WRITE_PTR_TYPE_MASK;
        if Z jump output_port;
            // input port (HW WR) log_fault_and_exit
            pop r1;
            r1 = r2;        // descriptor diatribe
            r0 = $faultids.FAULT_AUDIO_DEBUG_BUFFER_ACCESS;
            call $_fault_diatribe;
            jump fill_end;

        output_port:
            // output port (HW RD) get the port number from the write addr
            r0 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
            call $cbuffer.calc_amount_space.its_a_port;
            r1 = r2 ASHIFT -1;        // size in words
            pop r2;                   // fill value
            jump block_fill;
   not_port:
#endif

   r2 = r1;         // fill value for block_fill

   call $_cbuffer_get_size_in_words;
   r1 = r0;         // block_size: the whole buffer

   block_fill:
   r0 = r3;         // restore cbuffer struc pointer
   call $cbuffer.private.block_fill_unsafe;

   fill_end:
   pop rLink;
   rts;

.ENDMODULE;



// ***********************************************************************************
// The following private functions are supported for both SW and MMU buffers (but local only).
// for remote buffer, use cbuffer_[set|get]_[read|write]_offset
// No C interface
// ***********************************************************************************

/* obsolete BC5 aka KAL_ARCH2 that didn't have a base_register
 *
 *  // void cbuffer_get_read_address_and_size(tCbuffer *cbuffer, int **read_address, int *buffer_size);
 *  $_cbuffer_get_read_address_and_size:
 *
 *
 *  // void cbuffer_get_write_address_and_size(tCbuffer *cbuffer, int **write_address, int *buffer_size);
 *  $_cbuffer_get_write_address_and_size:
 */

/* obsolete returning more than one parameters, not really used so far
 *
 *  // void cbuffer_get_read_address_and_size_and_start_address(tCbuffer *cbuffer, int **read_address, int *buffer_size, int *start_addr);
 *  $_cbuffer_get_read_address_and_size_and_start_address:
 *
 *  // void cbuffer_get_write_address_and_size_and_start_address(tCbuffer *cbuffer, int **write_address, int *buffer_size, int *start_addr);
 *  $_cbuffer_get_write_address_and_size_and_start_address:
 *
 * TODO could provide a private get_read_address (and get_write_address...) returning only one int value
 *    while setting the index, length and base registers internally
 */

/* C support (audio-cpu) obsoleted for some cbuffer_ methods (no more universal access for all types - SW, MMU, remote ) */
/* OBSOLETE all calls to cbuffer_get_read_address_and_size_and_start_address, get_write_address_and_size_and_start_address,
 *     for remote buffers or mmu HW buffers if absolutely necessary to have low-level access
       should use mmu_buffer_get_handle_offset, mmu_buffer_set_handle_offset
 */
/* OBSOLETE all calls to cbuffer_get_read_address_and_size, get_write_address_and_size
 *    they only work for pre-BC7 chips
 */

// *****************************************************************************
// MODULE:
//    $cbuffer.get_read_address_and_size_and_start_address
//
// DESCRIPTION:
//    Get a read address, size and start address for a cbuffer
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
.MODULE $M.cbuffer.get_read_address_and_size_and_start_address;
   .CODESEGMENT BUFFER_PM;

$cbuffer.get_read_address_and_size_and_start_address:

#ifdef PATCH_LIBS
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.get_read_address_and_size_and_start_address.PATCH_ID_0, r1)
#endif

   // If the buffer is remote, need to call into chip-specific layer
   r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_rd_ptr;

#ifdef CHIP_BASE_BC7
#ifdef DEBUG_ON
      // get/set_read_address only on HW WR port
      Null = r1 AND $cbuffer.READ_PTR_TYPE_MASK;
      if NZ call $error;
         Null = r1 AND $cbuffer.WRITE_PTR_TYPE_MASK;
         if Z call $error;
#endif
      r0 = M[r0 + $cbuffer.READ_ADDR_FIELD];
      jump $cbuffer.get_read_address.its_a_port;

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)

      // mmu only if it's not a SW pointer
      Null = r1 AND $cbuffer.READ_PTR_TYPE_MASK;
      if Z jump its_a_sw_rd_ptr;
#ifdef DEBUG_ON
      // if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor))
      Null = r1 AND $cbuffer.IS_REMOTE_MMU_MASK;
      if Z jump $mmu.get_read_address_local_buff;
         call $error;                    // it's a remote buffer
         /*  not supposed to call remote buff from asm, they have offsets,
                you probably want mmu_buffer_get_handle_offset
          */
#else
      // read address is an MMU buffer
      jump $mmu.get_read_address_local_buff;
#endif

#elif defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
      // A7DA_KAS only has sw_buff, so we should not be here.
      call $error;
#endif
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_BC7

   its_a_sw_rd_ptr:
   r2 = M[r0 + $cbuffer.START_ADDR_FIELD];
   // r1 = M[r0 + $cbuffer.SIZE_FIELD];
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r1, r0);
   BUFFER_GET_READ_ADDR(r0, r0);
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbuffer.get_write_address_and_size_and_start_address
//
// DESCRIPTION:
//    Get a write address, size and start address for a cbuffer
//  so that it can write in a generic way.
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
.MODULE $M.cbuffer.get_write_address_and_size_and_start_address;
   .CODESEGMENT BUFFER_PM;

$cbuffer.get_write_address_and_size_and_start_address:

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.get_write_address_and_size_and_start_address.PATCH_ID_0, r1)
#endif

   // If the buffer is remote, need to call into chip-specific layer
   r1 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r1 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_wr_ptr;

#ifdef CHIP_BASE_BC7
      r0 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
      jump $cbuffer.get_write_address.its_a_port;
#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)

      // mmu only if it's not a SW pointer
      Null = r1 AND $cbuffer.WRITE_PTR_TYPE_MASK;
      if Z jump its_a_sw_wr_ptr;
#ifdef DEBUG_ON
      // if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor))
      Null = r1 AND $cbuffer.IS_REMOTE_MMU_MASK;
      if Z jump $mmu.get_write_address_local_buff;
         call $error;                    // it's a remote buffer
         /*  not supposed to call remote buff from asm, they have offsets,
                you probably want mmu_buffer_get_handle_offset
          */
#endif
      // write address is an MMU buffer
      if NZ jump $mmu.get_write_address_local_buff;

#elif defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
      // A7DA_KAS only has sw_buff, so we should not be here.
      call $error;
#endif
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_BC7

   its_a_sw_wr_ptr:
   r2 = M[r0 + $cbuffer.START_ADDR_FIELD];
   // r1 = M[r0 + $cbuffer.SIZE_FIELD];
   BUFFER_GET_SIZE_IN_ADDRS_ASM(r1, r0);
   BUFFER_GET_WRITE_ADDR(r0, r0);
   rts;

.ENDMODULE;




// *****************************************************************************
// MODULE:
//    C: cbuffer_set_read_address
//    asm: $cbuffer.set_read_address
//
// DESCRIPTION:
//    Set the read address for a cbuffer. C callable version expects C calling
//    conventions to be respected.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//    - r1 = read address
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//   (r1   if it's a hw hydra cbuffer)
//    If a port: r0, r1, r2, r3, r10, DoLoop
//
//
// *****************************************************************************
.MODULE $M.cbuffer.set_read_address;
   .CODESEGMENT BUFFER_PM;
$_cbuffer_set_read_address:
$cbuffer.set_read_address:

   push r2;       // preserve for compatibility with legacy libraries

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.set_read_address.PATCH_ID_0, r2)
#endif

   r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r2 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_rd_ptr;

#ifdef CHIP_BASE_BC7
      pop r2;
      r0 = M[r0 + $cbuffer.READ_ADDR_FIELD];
      jump $cbuffer.set_read_address.its_a_port;

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)

      // mmu only if it's not a SW pointer
      Null = r2 AND $cbuffer.READ_PTR_TYPE_MASK;
      if Z jump its_a_sw_rd_ptr;
#ifdef DEBUG_ON
      // if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor))
      Null = r2 AND $cbuffer.IS_REMOTE_MMU_MASK;
      if NZ call $error;                    // it's a remote buffer
         /*  not supposed to call remote buff from asm, they have offsets,
                you probably want cbuffer_set_read_offset
          */
#endif
      pop r2;
      // read address is an MMU buffer
      jump $mmu.set_read_address_local_buff;

#elif defined(CHIP_BASE_A7DA_KAS)
      // Noting to do for A7DA_KAS.
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_BC7


   its_a_sw_rd_ptr:

#ifdef CBUFFER_RW_ADDR_DEBUG
   // Temporary debug validation of read address
   // Save some state in case the error condition occurs
   pushm <FP(=SP), r0, r1, r2, r3, rLink>;
   r2 = M[r0 + $cbuffer.START_ADDR_FIELD];
   Null = r1 - r2;
   if NC call $error;
   r3 = M[r0 + $cbuffer.SIZE_FIELD];
   r2 = r2 + r3;
   Null = r1 - r2;
   if C call $error;
   popm <FP, r0, r1, r2, r3, rLink>;
#endif /* CBUFFER_RW_ADDR_DEBUG */

   M[r0 + $cbuffer.READ_ADDR_FIELD] = r1;
   pop r2;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    C: cbuffer_set_write_address
//    asm: $cbuffer.set_write_address
//
// DESCRIPTION:
//    Set the write address for a cbuffer. C callable version expects C calling
//    conventions to be respected.
//
// INPUTS:
//    - r0 = pointer to cbuffer structure
//    - r1 = write address
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//   (r1   if it's a hw hydra cbuffer)
//    If a port: r0, r1, r2, r3, r10, DoLoop
//
// *****************************************************************************
.MODULE $M.cbuffer.set_write_address;
   .CODESEGMENT BUFFER_PM;
$_cbuffer_set_write_address:
$cbuffer.set_write_address:

   push r2;       // preserve for compatibility with legacy libraries

#ifdef PATCH_LIBS
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($cbuffer.set_write_address.PATCH_ID_0, r2)
#endif

   r2 = M[r0 + $cbuffer.DESCRIPTOR_FIELD];
   Null = r2 AND $cbuffer.BUFFER_TYPE_MASK;
   if Z jump its_a_sw_wr_ptr;

#ifdef CHIP_BASE_BC7

#ifdef DEBUG_ON
      // get/set_write_address only on HW RD port
      Null = r2 AND $cbuffer.READ_PTR_TYPE_MASK;
      if Z call $error;
         Null = r2 AND $cbuffer.WRITE_PTR_TYPE_MASK;
         if NZ call $error;
#endif
      pop r2;
      r0 = M[r0 + $cbuffer.WRITE_ADDR_FIELD];
      jump $cbuffer.set_write_address.its_a_port;

#elif defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)

      // mmu only if it's not a SW pointer
      Null = r2 AND $cbuffer.WRITE_PTR_TYPE_MASK;
      if Z jump its_a_sw_wr_ptr;
#ifdef DEBUG_ON
      // if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor))
      Null = r2 AND $cbuffer.IS_REMOTE_MMU_MASK;
      if NZ call $error;                    // it's a remote buffer
         /*  not supposed to call remote buff from asm, they have offsets,
                you probably want cbuffer_set_write_offset
          */
#endif
      pop r2;
      // write address is an MMU buffer
      jump $mmu.set_write_address_local_buff;
#elif defined(CHIP_BASE_A7DA_KAS)
#ifdef DEBUG_ON
      // A7DA_KAS only has sw_buff, so we should not be here.
      call $error;
#endif
#else

#error "Buffer TAL not implemented for this chip"

#endif      // CHIP_BASE_BC7


   its_a_sw_wr_ptr:

#ifdef CBUFFER_RW_ADDR_DEBUG
   // Temporary debug validation of write address
   // Save some state in case the error condition occurs
   pushm <FP(=SP), r0, r1, r2, r3, rLink>;
   r2 = M[r0 + $cbuffer.START_ADDR_FIELD];
   Null = r1 - r2;
   if NC call $error;
   r3 = M[r0 + $cbuffer.SIZE_FIELD];
   r2 = r2 + r3;
   Null = r1 - r2;
   if C call $error;
   popm <FP, r0, r1, r2, r3, rLink>;
#endif /* CBUFFER_RW_ADDR_DEBUG */

   M[r0 + $cbuffer.WRITE_ADDR_FIELD] = r1;
   pop r2;
   rts;

.ENDMODULE;

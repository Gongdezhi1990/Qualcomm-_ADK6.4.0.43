// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// 
// *****************************************************************************

#ifndef CBUFFER_ASM_HEADER_INCLUDED
#define CBUFFER_ASM_HEADER_INCLUDED

.CONST    $cbuffer.SIZE_FIELD           0;
.CONST    $cbuffer.READ_ADDR_FIELD      $cbuffer.SIZE_FIELD       + ADDR_PER_WORD;  // 1*ADDR_PER_WORD
.CONST    $cbuffer.WRITE_ADDR_FIELD     $cbuffer.READ_ADDR_FIELD  + ADDR_PER_WORD;  // 2*ADDR_PER_WORD
.CONST    $cbuffer.START_ADDR_FIELD     $cbuffer.WRITE_ADDR_FIELD + ADDR_PER_WORD;  // 3*ADDR_PER_WORD
.CONST    $cbuffer.AUX_ADDR_FIELD       $cbuffer.START_ADDR_FIELD + ADDR_PER_WORD;  // 4*ADDR_PER_WORD
.CONST    $cbuffer.DESCRIPTOR_FIELD     $cbuffer.AUX_ADDR_FIELD   + ADDR_PER_WORD;  // 5*ADDR_PER_WORD
#ifdef INSTALL_METADATA
.CONST    $cbuffer.METADATA_FIELD       $cbuffer.DESCRIPTOR_FIELD + ADDR_PER_WORD;  // 6*ADDR_PER_WORD
.CONST    $cbuffer.STRUC_SIZE           $cbuffer.METADATA_FIELD/ADDR_PER_WORD + 1;  // asm struc sizes always in words
#else
.CONST    $cbuffer.STRUC_SIZE           $cbuffer.DESCRIPTOR_FIELD/ADDR_PER_WORD + 1;  // asm struc sizes always in words
#endif


// macros

// asm accessor for cbuffer->size
#define BUFFER_GET_SIZE_IN_ADDRS_ASM(dest_reg, cbuffer_ptr_reg)    dest_reg = M[cbuffer_ptr_reg + $cbuffer.SIZE_FIELD]
#define BUFFER_GET_SIZE_IN_WORDS_ASM(dest_reg, cbuffer_ptr_reg)    dest_reg = M[cbuffer_ptr_reg + $cbuffer.SIZE_FIELD]; \
                                                                   dest_reg = dest_reg LSHIFT -LOG2_ADDR_PER_WORD

#ifdef KAL_ARCH4
#define BUFFER_EX_OFFSET_MASK    3      // "offset" in _ex context is offset in a word,
                                        // (range 0...3 for 32-bit)

#define BUFFER_GET_WRITE_ADDR(dest_reg, cb_reg)    dest_reg = M[cb_reg + $cbuffer.WRITE_ADDR_FIELD]; \
                                                   dest_reg = dest_reg AND ~BUFFER_EX_OFFSET_MASK
#define BUFFER_GET_READ_ADDR(dest_reg, cb_reg)     dest_reg = M[cb_reg + $cbuffer.READ_ADDR_FIELD]; \
                                                   dest_reg = dest_reg AND ~BUFFER_EX_OFFSET_MASK
#define BUFFER_MASK_WRITE_ADDR(reg)                reg = reg AND ~BUFFER_EX_OFFSET_MASK
#define BUFFER_MASK_READ_ADDR(reg)                 reg = reg AND ~BUFFER_EX_OFFSET_MASK
#define BUFFER_GET_USABLE_OCTETS(dest_reg, cb_reg) pushm <r0, r1, r2, r3>; \
                                                   r0 = cb_reg; \
                                                   call $_cbuffer_get_usable_octets; \
                                                   dest_reg = r0; \
                                                   popm <r0, r1, r2, r3>

#else /* KAL_ARCH4 */
#define BUFFER_GET_WRITE_ADDR(dest_reg, cb_reg) dest_reg = M[cb_reg + $cbuffer.WRITE_ADDR_FIELD]
#define BUFFER_GET_READ_ADDR(dest_reg, cb_reg)  dest_reg = M[cb_reg + $cbuffer.READ_ADDR_FIELD]
#define BUFFER_MASK_WRITE_ADDR(reg)
#define BUFFER_MASK_READ_ADDR(reg)
#endif


#ifdef KAL_ARCH4

// get cbuffer size macro so that size is in the same units as addresses (pointers)
#define BUFFER_WORDS_TO_ADDRS_ASM(reg)    reg = reg ASHIFT LOG2_ADDR_PER_WORD
#define BUFFER_ADDRS_TO_WORDS_ASM(reg)    reg = reg ASHIFT -LOG2_ADDR_PER_WORD
// NB. ASHIFT used although a negative number doesn't make sense, there are places
//      where we call $error on NEG counters in DEBUG mode

#else

// translates to 'nothing' for 24b/24b architectures
#define BUFFER_WORDS_TO_ADDRS_ASM(reg)
#define BUFFER_ADDRS_TO_WORDS_ASM(reg)

// keep a generic (no suffix) macro for legacy (although the code would probably grab the 'size' directly anyway)
#define BUFFER_GET_SIZE_ASM(dr, cb)    BUFFER_GET_SIZE_IN_ADDRS_ASM(dr, cb)

#endif      // KAL_ARCH4

// TODO temp define a mask while io_defs autogen gets fixed (B-250950)
#define $BAC_BUFFER_SAMPLE_SIZE_MASK    3

// $cbuffer.DESCRIPTOR field definitions - keep in sync with cbuffer_c.h
//
// bit 0 :      0 - Pure SW buffer
//              1 - MMU buffer (local or remote)
.CONST    $cbuffer.BUFFER_TYPE_MASK          0x000001;
.CONST    $cbuffer.BUFFER_TYPE_SHIFT         0;
// bit 1 :      0 - cbuffer struc wraps a local MMU buffer
//              1 - cbuffer struc wraps a remote MMU buffer
.CONST    $cbuffer.IS_REMOTE_MMU_MASK        0x000002;
.CONST    $cbuffer.IS_REMOTE_MMU_SHIFT       1;
// bit 2:       0 - $cbuffer.READ_ADDR_FIELD points to SW buffer
//              1 - $cbuffer.READ_ADDR_FIELD points to MMU buffer
.CONST    $cbuffer.READ_PTR_TYPE_MASK        0x000004;
.CONST    $cbuffer.READ_PTR_TYPE_SHIFT       2;
// bit 3:       0 - $cbuffer.WRITE_ADDR_FIELD points to SW buffer
//              1 - $cbuffer.WRITE_ADDR_FIELD points to MMU buffer
.CONST    $cbuffer.WRITE_PTR_TYPE_MASK       0x000008;
.CONST    $cbuffer.WRITE_PTR_TYPE_SHIFT      3;
// bits 4 and 5 are used for remote buffers only, so skip them.
// bit 6:       0 - $cbuffer.AUX_ADDR_FIELD unused
//              1 - $cbuffer.AUX_ADDR_FIELD in use for MMU buffer
.CONST    $cbuffer.AUX_PTR_PRESENT_MASK      0x000040;
.CONST    $cbuffer.AUX_PTR_PRESENT_SHIFT     6;
// bit 7:       0 - $cbuffer.AUX_ADDR_FIELD used for MMU read
//              1 - $cbuffer.AUX_ADDR_FIELD used for MMU write
.CONST    $cbuffer.AUX_PTR_TYPE_MASK         0x000080;
.CONST    $cbuffer.AUX_PTR_TYPE_SHIFT        7;

// bits 8-13 not needed here as they only pertain to MMU buffer creation.

// bit 14:      0 - $cbuffer.CBOPS_IS_SCRATCH_FIELD unused
//              1 - $cbuffer.CBOPS_IS_SCRATCH_FIELD used for CBOPs scratch buffers
.CONST    $cbuffer.CBOPS_IS_SCRATCH_MASK     0x004000;
.CONST    $cbuffer.CBOPS_IS_SCRATCH_SHIFT    14;

// bit 15 :      0 - cbuffer does NOT have metadata
//               1 - cbuffer has metadata associated
.CONST    $cbuffer.METADATA_MASK             0x008000;
.CONST    $cbuffer.METADATA_SHIFT            15;

// bit 16 :      0 - cbuffer is NOT part of an in place chain
//               1 - cbuffer is part of an in place chain
.CONST    $cbuffer.IN_PLACE_MASK             0x010000;
.CONST    $cbuffer.IN_PLACE_SHIFT            16;

// bits 17-18 :  usable_octets:2; number of usable octets in a word (0 means all)
.CONST    $cbuffer.USABLE_OCTETS_MASK        0x060000;
.CONST    $cbuffer.USABLE_OCTETS_SHIFT       17;
// C equivalents defined in USABLE_OCTETS enum (keep in sync with C counterpart header)
.CONST    $cbuffer.USABLE_OCTETS_32BIT       0;
.CONST    $cbuffer.USABLE_OCTETS_8BIT        1;
.CONST    $cbuffer.USABLE_OCTETS_16BIT       2;
.CONST    $cbuffer.USABLE_OCTETS_24BIT       3;


// This is the structure of MMU handles
// defined as audio_buf_handle_struc in C header hydra_mmu_private.h (keep in sync)
.CONST   $cbuffer.MMU_HANDLE_BUFFER_START        0;
.CONST   $cbuffer.MMU_HANDLE_BUFFER_BYTE_OFFSET  1*ADDR_PER_WORD;
.CONST   $cbuffer.MMU_HANDLE_BUFFER_SIZE         2*ADDR_PER_WORD;

#endif  // CBUFFER_ASM_HEADER_INCLUDED

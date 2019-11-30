/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file hydra_mmu_private.h
 * \ingroup buffer
 */

#ifndef HYDRA_MMU_PRIVATE_H
#define HYDRA_MMU_PRIVATE_H

#include "buffer.h"
#include "hydra_mmu.h"
#include "hydra_mmu_buff.h"
#include "pmalloc/pl_malloc.h" /* for DM1_ADDRESS() etc (ick) */

/****************************************************************************
Private Type Declarations
*/

/* TODO: This is over-simplistic. Each word in the struct has its own internal
 * structure including flags and things. */
typedef struct
{
    int buffer_start_p; /* octet address */
    volatile int buffer_offset;  /* Measured in octets. I really hope 'volatile' works here. */
    int buffer_size;   /* bits 0..11 contain either the number of pages, or log2(number of pages),
                        * depending on whether or not MSB is set. */
} audio_buf_handle_struc;


/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/

/* Macros for word - octet - pointers conversion
 * 1. converting a real pointer <-> the start address 'pointer'
 * stored in an MMU handle (see description of buffer_start_p above).
 * 2. macros for converting from/to cbuffer internal sizes
 */
#ifndef KAL_ARCH4

/* in 24b/24b architectures words are mmu-words, 16-bit */
#define ptr_to_octet(x) ((uintptr_t)(DM1_ADDRESS(x))*2)
#define octet_to_ptr(x) ((uintptr_t)((x)/2))

/* macros for converting mmu page size in octets to cbuffer internal sizes
    TODO_CONSIDER DELETE only used in the context of circular_buffer_copyto/from that are good candidates for rethink */
#define mmu_page_buffer_size_octets_to_internal(x) ((x)/2)

#else

/* in 32b/8b architectures words are octet addressable, no need for conversion  */
#define ptr_to_octet(x) ((uintptr_t)DM1_ADDRESS(x))
#define octet_to_ptr(x) ((uintptr_t)(x))

#endif

/* It's handy to have a quick(er) way to convert between BUF_DESC_WR_PTR_HOSTXS_PROT_MASK
 * et al and MMU_BUF_HOSTXS_PROT_MASK et al. */
#define CBUF_TO_MMU_RD_PROT_FLAGS(flags)   ((((flags) & BUF_DESC_RD_PTR_PROT_MASK) >> BUF_DESC_RD_PTR_PROT_SHIFT) << MMU_BUF_PROT_SHIFT)
#define CBUF_TO_MMU_WR_PROT_FLAGS(flags)   ((((flags) & BUF_DESC_WR_PTR_PROT_MASK) >> BUF_DESC_WR_PTR_PROT_SHIFT) << MMU_BUF_PROT_SHIFT)
#ifdef BAC32
#define CBUF_TO_MMU_SAMP_SIZE_FLAGS(flags) ((((flags) & BUF_DESC_SAMP_SIZE_MASK) >> BUF_DESC_SAMP_SIZE_SHIFT) << MMU_BUF_SAMP_SZ_SHIFT)
#else
#define CBUF_TO_MMU_SAMP_SIZE_FLAGS(flags) (0)
#endif

/* BAC_REMOTE_WINDOW_START_ADDR and BAC_REMOTE_WINDOW_SIZE are defined in a config file */
#define BAC_START_REMOTE_WINDOW_0   BAC_REMOTE_WINDOW_START_ADDR
#define BAC_START_REMOTE_WINDOW_1   (BAC_REMOTE_WINDOW_START_ADDR + BAC_REMOTE_WINDOW_SIZE)
#define BAC_START_REMOTE_WINDOW_2   (BAC_REMOTE_WINDOW_START_ADDR + BAC_REMOTE_WINDOW_SIZE * 2)
#define BAC_START_REMOTE_WINDOW_3   (BAC_REMOTE_WINDOW_START_ADDR + BAC_REMOTE_WINDOW_SIZE * 3)

#if defined(BAC32) || defined(BAC24)
    #define BAC_SIZE_CODE_TO_BUF_SIZE(sc) (MMU_PAGE_TABLE_PAGES((sc)) * MMU_PAGE_BYTES)
#else
    #error "undefined BAC_SIZE_CODE_TO_BUF_SIZE(sc) for this BAC"
#endif

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private functions
*/

extern mmu_handle mmu_buffer_get_handle_from_ptr(audio_buf_handle_struc* buf_handle_ptr);
extern mmu_index mmu_buffer_get_handle_idx_from_ptr(audio_buf_handle_struc* buf_handle_ptr);
extern audio_buf_handle_struc* mmu_buffer_get_handle_ptr_from_idx(mmu_index index);

   /* [in] Logical MMU handle to pack */
#define mmu_handle_pack(h) \
    ((mmu_handle_ssid(h) << 8) | mmu_handle_index(h))


   /** [in] packed MMU handle; [out] hydra MMU handle */
#define mmu_handle_unpack(val, h) \
    mmu_handle_init((h), (system_bus)(((uintptr_t)val) >> 8) , (mmu_index)(((uintptr_t)val) & 0xFF))


/* release read or write and aux handle */
void mmu_release_handle(tCbuffer *cbuffer);


#endif   /* HYDRA_MMU_PRIVATE_H */

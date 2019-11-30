/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer Buffer Subsystem
 * \file  hydra_mmu_buff.h
 * Public header file for the mmu_buffer interface
 * \ingroup buffer
 *
 */

#ifndef HYDRA_MMU_BUFF_H
#define HYDRA_MMU_BUFF_H

/****************************************************************************
Include Files
*/

#include "types.h"
#include "hydra_mmu.h" /* Defines some mmu_buffer types, such as struct mmu_handle */

/****************************************************************************
Public Macro Declarations
*/


#if defined(KAL_ARCH4)
/**
 * More infos http://wiki/Kymera/Crescendo/Hardware#Buffer_access_controller_.28BAC.29
 */

/* B'mode flag. We require this to be set to 0. */
#define MMU_BUF_B_MODE_SHIFT                31
#define MMU_BUF_XS_PROT_SHIFT               30
#define MMU_BUF_HOSTXS_PROT_SHIFT           29
#define MMU_BUF_BSWAP_SHIFT                 28
#define MMU_BUF_SE_SHIFT                    27
#define MMU_BUF_SHIFT_SHIFT                 22
#define MMU_BUF_PKG_EN_SHIFT                21
#define MMU_BUF_SAMP_SZ_SHIFT               19
#define MMU_BUF_SAMP_SZ_WIDTH               2
#define MMU_BUF_SHIFT_WIDTH                 5
#else /* defined(KAL_ARCH4) */
/**
 * Configuration flags
 * These are used to set control flags in a buffer handle, such as
 * XS Prot or B'swap. They are defined by the BAC hardware.
 * For more details see http://wiki/AudioCPU/architecture/Buffers/BAC
 */

/* B'mode flag. We require this to be set to 0. */
#define MMU_BUF_B_MODE_SHIFT                23
#define MMU_BUF_XS_PROT_SHIFT               22
#define MMU_BUF_HOSTXS_PROT_SHIFT           21
#define MMU_BUF_BSWAP_SHIFT                 18
#define MMU_BUF_SE_SHIFT                    17
#define MMU_BUF_SHIFT_SHIFT                 13
#define MMU_BUF_SHIFT_WIDTH                 4
#endif /* defined(KAL_ARCH4) */


   /* XS Prot. When set gives remote subsystems (not including hostio)
    *          read-only access.
    */
#define MMU_BUF_XS_PROT_MASK                (1 << MMU_BUF_XS_PROT_SHIFT)
#define MMU_BUF_XS_PROT_SET(x)              ((x) |= MMU_BUF_XS_PROT_MASK)
#define MMU_BUF_XS_PROT(x)                  ((x) & MMU_BUF_XS_PROT_MASK)
   /* Host XS Prot. When set gives Hostio subsystem read-only access.
    */
#define MMU_BUF_HOSTXS_PROT_MASK            (1 << MMU_BUF_HOSTXS_PROT_SHIFT)
#define MMU_BUF_HOSTXS_PROT_SET(x)          ((x) |= MMU_BUF_HOSTXS_PROT_MASK)
#define MMU_BUF_HOSTXS_PROT(x)              ((x) & MMU_BUF_HOSTXS_PROT_MASK)
    /* Both prot flags */
#define MMU_BUF_PROT_MASK                   (MMU_BUF_HOSTXS_PROT_MASK | MMU_BUF_XS_PROT_MASK)
#define MMU_BUF_PROT_SHIFT                  (MMU_BUF_HOSTXS_PROT_SHIFT)

   /* B'swap. Puts bytes written with an even address into the
    *           MS part of the data word when high. Otherwise even-addressed
    *           bytes go to the LS part of the word.
    */
#define MMU_BUF_BSWAP_MASK                  (1 << MMU_BUF_BSWAP_SHIFT)
#define MMU_BUF_BSWAP_SET(x)                ((x) |= MMU_BUF_BSWAP_MASK)
#define MMU_BUF_BSWAP_UNSET(x)              ((x) &= ~MMU_BUF_BSWAP_MASK)
#define MMU_BUF_BSWAP(x)                    ((x) & MMU_BUF_BSWAP_MASK)
   /* SE: Enables sign extension for writes and saturation for reads.
    */
#define MMU_BUF_SE_MASK                     (1 << MMU_BUF_SE_SHIFT)
#define MMU_BUF_SE_SET(x)                   ((x) |= MMU_BUF_SE_MASK)
#define MMU_BUF_SE_UNSET(x)                 ((x) &= ~MMU_BUF_SE_MASK)
#define MMU_BUF_SE(x)                       ((x) & MMU_BUF_SE_MASK)
   /* Shift. Each 16-bit write value is written to an arbitrary offset
    *               in the 24-bit RAM word.
    */
#define MMU_BUF_SHIFT_WIDTH_MASK            ((1U << MMU_BUF_SHIFT_WIDTH) - 1)
#define MMU_BUF_SHIFT_MASK                  (MMU_BUF_SHIFT_WIDTH_MASK << MMU_BUF_SHIFT_SHIFT)
#define MMU_BUF_SHIFT_SET(x, val)           ((x) = ((x) & ~MMU_BUF_SHIFT_MASK) | (((val) & MMU_BUF_SHIFT_WIDTH_MASK) << MMU_BUF_SHIFT_SHIFT))
#define MMU_BUF_SHIFT_GET(x)                (((x) & MMU_BUF_SHIFT_MASK) >> MMU_BUF_SHIFT_SHIFT)

#ifdef BAC32
#define MMU_BUF_PKG_EN_MASK                 (1U << MMU_BUF_PKG_EN_SHIFT)
#define MMU_BUF_PKG_EN_SET(x)               ((x) |= MMU_BUF_PKG_EN_MASK)
#define MMU_BUF_PKG_EN_UNSET(x)             ((x) &= ~MMU_BUF_PKG_EN_MASK)
#define MMU_BUF_PKG_EN_GET(x)               (((x) & MMU_BUF_PKG_EN_MASK) >> MMU_BUF_PKG_EN_SHIFT)

#define MMU_BUF_SAMP_SZ_WIDTH_MASK          ((1U << MMU_BUF_SAMP_SZ_WIDTH) - 1)
#define MMU_BUF_SAMP_SZ_MASK                (MMU_BUF_SAMP_SZ_WIDTH_MASK << MMU_BUF_SAMP_SZ_SHIFT)
#define MMU_BUF_SAMP_SZ_SET(x, val)         ((x) = ((x) & ~MMU_BUF_SAMP_SZ_MASK) | (((val) & MMU_BUF_SAMP_SZ_WIDTH_MASK) << MMU_BUF_SAMP_SZ_SHIFT))
#define MMU_BUF_SAMP_SZ_GET(x)              (((x) & MMU_BUF_SAMP_SZ_MASK) >> MMU_BUF_SAMP_SZ_SHIFT)

#define MMU_BUF_UNPACKED_8BIT_MASK          (SAMPLE_SIZE_8BIT << MMU_BUF_SAMP_SZ_SHIFT)
#define MMU_BUF_UNPACKED_16BIT_MASK         (SAMPLE_SIZE_16BIT << MMU_BUF_SAMP_SZ_SHIFT)
#define MMU_BUF_UNPACKED_24BIT_MASK         (SAMPLE_SIZE_24BIT << MMU_BUF_SAMP_SZ_SHIFT)
#define MMU_BUF_UNPACKED_32BIT_MASK         (SAMPLE_SIZE_32BIT << MMU_BUF_SAMP_SZ_SHIFT)

#define MMU_BUF_PACKED_8BIT_MASK            (1 << MMU_BUF_PKG_EN_SHIFT)
#else /* BAC32 */
/* Macros do nothing for other BACs */
#define MMU_BUF_PKG_EN_SET(x)
#define MMU_BUF_PKG_EN_UNSET(x)
#define MMU_BUF_SAMP_SZ_SET(x,val)
#endif /* BAC32 */

   /* bits 11-0:    The buffer size. We need to mask these bits out when accessing
    *               the size of a buffer handle.
    */
#define MMU_BUF_SIZE_MASK                   0x7FF

#ifdef KAL_ARCH4
#define MMU_BUF_KAL_ARCH4_MASKS             MMU_BUF_PKG_EN_MASK | MMU_BUF_SAMP_SZ_MASK
#else
#define MMU_BUF_KAL_ARCH4_MASKS             0
#endif

#ifdef CHIP_BASE_NAPIER

#define DM1_TO_AUDIO_AHB_ADDRESS_SPACE(DM1_ADDRESS) (((uintptr_t)(DM1_ADDRESS)&AUDIO_AHB_SPACE_KALIMBA_DM_SIZE)|AUDIO_AHB_SPACE_KALIMBA_DM_START_ADDRESS)
#define AUDIO_AHB_ADDRESS_SPACE_TO_DM1(AHB_ADDRESS) (((uintptr_t)(AHB_ADDRESS)&AUDIO_AHB_SPACE_KALIMBA_DM_SIZE))
#endif

  /* This only includes flags we currently support. */
#define MMU_BUF_ALL_FLAGS_MASK              (MMU_BUF_KAL_ARCH4_MASKS | MMU_BUF_PROT_MASK | MMU_BUF_BSWAP_MASK | MMU_BUF_SE_MASK | MMU_BUF_SHIFT_MASK)
/****************************************************************************
Public Function Declarations
*/

/**
 * Initialise the MMU buffer hardware
 */
extern void mmu_buffer_initialise(void);

/**
 * Create an MMU buffer.
 *
 * This mallocs a buffer able to hold buffer_size octets of data, and
 * associates it with a single MMU handle. If you require a buffer with
 * multiple MMU handles, you need to use mmu_buffer_claim_handle (or the
 * cbuffer API) instead.
 *
 * If successful, handlep will be set to the new handle and
 * TRUE returned. Note that buffer_size may be updated, if the exact size
 * couldn't be used.
 *
 * \param buffer_size is the requested capacity of the buffer in octets
 *        (or looked at another way, the maximum offset of the buffer).
 *        This may be updated if the exact size couldn't be used.
 * \param handlep - will be set to a newly-created handle that encapsulates the buffer.
 * \param config_flags - flags to configure the BAC hardware for this buffer.
 * \param preference - MALLOC_PREFERENCE to allocate the data memory from.
 * \return buffer address if successful, NULL otherwise
 */
extern int *mmu_buffer_create(unsigned *buffer_size, mmu_handle *handlep, unsigned config_flags, unsigned preference);

/**
 * Close an MMU buffer, freeing associated local resources.
 *
 * The function tests whether the supplied handle is non-NULL and local,
 * and if so frees it.  A remotely-owned buffer will be freed at the far end.
 *
 * Note that this will attempt to free the memory being managed by the MMU buffer.
 * If there are multiple MMU handles pointing to the same block of RAM,
 * you need to use a different API for this, e.g. mmu_buffer_release_handle.
 *
 * \param handle - handle encapsulating the buffer to be destroyed
 */
extern void mmu_buffer_destroy(mmu_handle handle);

/**
 * Read data into an array from an buffer.
 *
 * \param handle is the source MMU buffer
 * \param src_buf_offset is the location of the data in the MMU buffer
 *        (in octets)
 * \param len_octets is the number of octets to read
 * \param dest_buf is the destination array of length at least \c len_octets
 *        octets. Note it's an array of words, i.e. pairs of octets.
 * \param byte_swap indicates whether the data should undergo a byte-swap
 *        process during the read. (This is only necessary for remote
 *        buffers; local buffers can be permanently configued via
 *        mmu_buffer_set_flags.)
 *
 * N.B.: \c offset_octets, \c buffer_max and \c len_octets are in octets
 * (surprised?), consistent with the MMU interface.
 *
 * \return TRUE if everything was ok, FALSE if something went wrong.
 */
extern bool mmu_buffer_read(mmu_handle handle, unsigned src_buf_offset,
                            unsigned len_octets, void *dest, bool byte_swap);

/**
 * Read data into a circular buffer from an MMU buffer.
 *
 * \param handle is the source MMU buffer
 * \param src_buf_offset is the location of the data in the MMU buffer
 *        (in octets)
 * \param len_octets is the number of octets to read
 * \param dest_buf is the destination array
 * \param dest_buf_offset is the offset (in octets) into the destination
 *        array at which to start copying.
 * \param dest_buf_size is the size (in octets) of the destination array.
 *        This must be at least \c len_octets big.
 * \param byte_swap indicates whether the data should undergo a byte-swap
 *        process during the read. (This is only necessary for remote
 *        buffers; local buffers can be permanently configued via
 *        mmu_buffer_set_flags.)
 *
 * N.B.: All sizes and lengths are in octets, consistent with the MMU interface.
 *
 * \return TRUE if everything was ok, FALSE if something went wrong.
 */
extern bool mmu_buffer_read_circular(mmu_handle handle, unsigned src_buf_offset,
                   unsigned len_octets, void *dest, unsigned dest_buf_offset,
                   unsigned dest_buf_size, bool byte_swap);

/**
 * Read the wallclock from its remote MMU buffer.
 *
 * \param handle is the wallclock's MMU buffer
 * \param src_buf_offset is the location of the wallclock in the MMU buffer
 *        (in octets)
 * \param dest_buf is the destination array
 *
 * N.B.: All sizes and lengths are in octets, consistent with the MMU interface.
 *
 * \return TRUE if everything was ok, Panic if something went wrong (FALSE in
 * unit test)
 */
extern bool mmu_buffer_read_wallclock(mmu_handle handle, unsigned src_buf_offset,
        uint16 *dest_buf);

/**
 * Write data to an MMU buffer from an array.
 *
 * \param handle is the destination MMU buffer
 * \param dest_buf_offset is the location in the MMU buffer to write to,
 *        in octets
 * \param len_octets is the length of the data in octets.
 * \param src_buf is the source array of length at least \c len_octets octets.
 *        Note it's an array of words, i.e. pairs of octets.
 *
 * N.B.: \c offset_octets and \c len_octets are in octets
 * (surprised?), consistent with the MMU interface.
 *
 * \return TRUE if everything was ok, FALSE if something went wrong.
 */
extern bool mmu_buffer_write(mmu_handle handle, unsigned dest_buf_offset,
                             unsigned len_octets, const void *src);

/**
 * Write data to an MMU buffer from a circular buffer.
 *
 * \param handle is the source MMU buffer
 * \param dest_buf_offset is the location in the MMU buffer to write to,
 *        in octets
 * \param len_octets is the length of the data in octets.
 * \param src_buf is the source array of length at least \c len_octets octets.
 *        Note it's an array of words, i.e. pairs of octets.
 * \param src_buf_offset is the offset (in octets) into the source array
 *        from which to start copying.
 * \param src_buf_size is the size (in octets) of the source array.
 *        This must be at least \c len_octets big.
 * \param byte_swap indicates whether the data should undergo a byte-swap
 *        process during the write. (This is only necessary for remote
 *        buffers; local buffers can be permanently configued via
 *        mmu_buffer_set_flags.)
 *
 * N.B.: All sizes and lengths are in octets, consistent with the MMU interface.
 *
 * \return TRUE if everything was ok, FALSE if something went wrong.
 */
extern bool mmu_buffer_write_circular(mmu_handle handle, unsigned dest_buf_offset,
                   unsigned len_octets, const void *src, unsigned src_buf_offset,
                   unsigned src_buf_size, bool byte_swap);

/**
 * Read an offset from an MMU handle.
 *
 * This is the offset stored in the MMU handle structure, measured in octets.
 * The operation works for both local and remote mmu buffers.
 *
 * Note that in principle the underlying call can fail.
 * This should never happen here, as it implies the remote
 * subsystem is turned off or incommunicado.
 *
 * \param handle - handle to query
 * \return the handle offset, in octets
 */
extern unsigned mmu_buffer_get_handle_offset(mmu_handle handle);

/**
 * Set the offset stored in an MMU handle.
 *
 * \param handle is the MMU buffer to modify
 * \param offset_octets is the offset we want to set, measured in octets.
 *
 * This function doesn't check the validity of the offset (i.e. that it
 * is less than the size of the buffer), because that's not possible to
 * do for remote buffers.
 */
extern void mmu_buffer_set_handle_offset(mmu_handle handle, unsigned offset_octets);

/**
 * Get the size of an MMU buffer, in octets.
 * \note this only works for local buffers; there's no way to work it out for remote buffers.
 *
 * \param handle is the MMU buffer to query
 * \return size of the buffer, in octets
 */
extern unsigned mmu_buffer_get_size(mmu_handle handle);

/**
 * Create an mmu_handle structure to wrap an mmu_index.
 * This is useful if the caller has an index to a local buffer,
 * and wants to call an mmu_buffer function that requres a full mmu_handle.
 * \param index - index of the local mmu buffer
 * \return the handle created
 */
extern mmu_handle mmu_handle_from_idx(mmu_index index);

/*****
 * We originally thought the following methods should be private, since they only work for local buffers.
 * However, we need them in our arsenal for funky buffer setups via streams.
 *****/

/**
 * Take a pre-existing buffer and give it to the MMU by allocating a buffer handle.
 *
 * N.B.: This function expects to be called from the LOWEST scheduling priority.
 *       Calling it from any other context may expose a re-entrancy problem,
 *       see B-128582.
 *
 * \param buf_mem - pre-allocated buffer
 * \param buffer_size - size of the buffer, as it will be seen by the MMU hardware.
 * \param config_flags - flags to configure the BAC hardware for this buffer.
 * \return index of the buffer handle created, or 0 if the process failed.
 */
extern mmu_index mmu_buffer_claim_handle(void* buf_mem, mmu_buffer_size buffer_size, unsigned config_flags);

/**
 * Take a pre-existing mmu_handle, claim a new one and copy the buffer parameters from the existing to the
 * newly-created mmu_handle. This allows us to tie multiple buffer handles to the same
 * actual buffer.
 *
 * N.B.: This function expects to be called from the LOWEST scheduling priority.
 *       Calling it from any other context may expose a re-entrancy problem,
 *       see B-128582.
 *
 * \param handle is the MMU buffer to clone
 * \return the cloned handle
 */
extern mmu_handle mmu_buffer_clone_handle(mmu_handle handle);

/**
 * Remove a local buffer from MMU control by freeing its buffer handle. Does NOT free the buffer memory itself.
 *
 * N.B.: This function expects to be called from the LOWEST scheduling priority.
 *       Calling it from any other context may expose a re-entrancy problem,
 *       see B-128582.
 *
 * \param index - index of the buffer handle to free
 * \return Physical address (DM offset) of the buffer memory 
 */
extern unsigned  mmu_buffer_release_handle(mmu_index index);

/**
 * Read the config flags for an MMU buffer.
 * \note this only works for local buffers.
 * \param handle is the MMU buffer to query
 * \return configuration flags set for this buffer.
 */
extern unsigned mmu_buffer_get_flags(mmu_handle handle);

/**
 * Set the config flags for an MMU buffer.
 * \note this only works for local buffers.
 *
 * \param handle is the MMU buffer to modify
 * \param config_flags - configuration flags to set within this buffer.
 *                       (overrides existing flags).
 */
extern void mmu_buffer_set_flags(mmu_handle handle, unsigned config_flags);

#endif /* HYDRA_MMU_BUFF_H */

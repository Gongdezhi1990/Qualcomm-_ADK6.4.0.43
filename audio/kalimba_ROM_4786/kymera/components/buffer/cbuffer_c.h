/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer Buffer Subsystem
 * \file  cbuffer_c.h
 *
 * \ingroup buffer
 *
 * Public C header file for the cbuffer interface
 *
 */

#ifndef CBUFFER_C_H
#define CBUFFER_C_H

/****************************************************************************
Include Files
*/
#include "types.h"

/****************************************************************************
Public Type Declarations
*/

/* Forward declarations for metadata types */
typedef struct metadata_tag_struct metadata_tag;
typedef struct metadata_list_struct metadata_list;


/**
 * Cbuffer structure
 *
 * The cbuffer structure can encapsulate a simple SW buffer, a local MMU buffer,
 * or a remote MMU buffer (this is mainly Hydra terminology).
 * Bluecore has "ports" to MMU buffers that are in the MMU hardware module
 * - shared between DSP and MCU (XAP); we still consider BC MMU buffers as "local",
 * they are similar to hydra BAC buffers, although "owned" by the DSP they are a separate HW module.
 *
 * In the case of a remote buffer (hydra), we interpret the read_ptr and write_ptr somewhat liberally;
 * we use the field to store a compressed Hydra mmu_handle. (They're guaranteed to fit,
 * because mmu_handles only contain 12 useful bits - this is a hardware constraint.)
 *
 * \note See http://wiki/AudioCPU/architecture/Buffers for a better explanation of this.
 */
typedef struct
{
    unsigned int size;     /**< Size in memory access unit */
    int *read_ptr;         /**< read pointer, for Hydra see note (*) */
    int *write_ptr;        /**< write pointer for Hydra see note (*) */
    int *base_addr;        /**< base address, for circular addressing */
    int *aux_ptr;          /**< Hydra: aux pointer(to a buffer handle), or NULL if not required (BCx) */
    unsigned int descriptor;  /**< Buffer descriptor (bitfield).
                                  \sa BUF_DESC_BUFFER_TYPE_MMU_SHIFT et al */
#ifdef INSTALL_METADATA
    metadata_list *metadata;
#endif
    /** Note (*) Hydra: Pointer into RAM, or pointer to a buffer handle, or compressed mmu_handle */
} tCbuffer;

/****************************************************************************
Public Constant Declarations
*/
/** The number of octets in a native word size sample */
#define OCTETS_PER_SAMPLE (DAWTH / 8)

/** Round down to the first multiply of OCTETS_PER_SAMPLE. */
#define MULTIPLE_OF_OCTETS_PER_SAMPLE(X) ((X/OCTETS_PER_SAMPLE)* OCTETS_PER_SAMPLE)

/****************************************************************************
Public Macro Declarations
*/

/**
 * \name tCbuffer descriptor definitions
 */
/*@{*/
/*
 * Buffer flag definitions to pass to cbuffer_create_mmu_buffer and cbuffer_wrap_remote.
 * These need to be kept in sync with the definitions in cbuffer_asm.h, as they
 * are also used internally.
 * NB. Although most BUF_DESC make sense for hydra only we keep them here so that
 * Descriptor is defined completely in one place.
 */


   /* bit 0:       0 - cbuffer wraps a pure SW buffer
    *              1 - cbuffer wraps an MMU buffer (local or remote)
    */
#define BUF_DESC_BUFFER_TYPE_MMU_SHIFT      0
#define BUF_DESC_BUFFER_TYPE_MMU_MASK       (1 << BUF_DESC_BUFFER_TYPE_MMU_SHIFT)
#define BUF_DESC_BUFFER_TYPE_MMU_SET(x)     ((x) |= BUF_DESC_BUFFER_TYPE_MMU_MASK)
#define BUF_DESC_BUFFER_TYPE_MMU(x)         ((x) & BUF_DESC_BUFFER_TYPE_MMU_MASK)

   /* bit 1:       0 - if bit 0 is set, cbuffer struct wraps a local MMU buffer
    *              1 - if bit 0 is set, cbuffer struct wraps a remote MMU buffer
    */
#define BUF_DESC_IS_REMOTE_MMU_SHIFT        1
#define BUF_DESC_IS_REMOTE_MMU_MASK         (1 << BUF_DESC_IS_REMOTE_MMU_SHIFT)
#define BUF_DESC_IS_REMOTE_MMU_SET(x)       ((x) |= BUF_DESC_IS_REMOTE_MMU_MASK)
#define BUF_DESC_IS_REMOTE_MMU(x)           ((x) & BUF_DESC_IS_REMOTE_MMU_MASK)

   /* bit 2:       0 - cbuffer read_ptr points to SW buffer
    *              1 - cbuffer read_ptr points to a local MMU buffer
    *              Note - we don't set this bit for remote MMU buffers.
    */
#define BUF_DESC_RD_PTR_TYPE_MMU_SHIFT      2
#define BUF_DESC_RD_PTR_TYPE_MMU_MASK       (1 << BUF_DESC_RD_PTR_TYPE_MMU_SHIFT)
#define BUF_DESC_RD_PTR_TYPE_MMU_SET(x)     ((x) |= BUF_DESC_RD_PTR_TYPE_MMU_MASK)
#define BUF_DESC_RD_PTR_TYPE_MMU(x)         ((x) & BUF_DESC_RD_PTR_TYPE_MMU_MASK)

   /* bit 3:       0 - cbuffer write_ptr points to SW buffer
    *              1 - cbuffer write_ptr points to a local MMU buffer
    *              Note - we don't set this bit for remote MMU buffers.
    */
#define BUF_DESC_WR_PTR_TYPE_MMU_SHIFT      3
#define BUF_DESC_WR_PTR_TYPE_MMU_MASK       (1 << BUF_DESC_WR_PTR_TYPE_MMU_SHIFT)
#define BUF_DESC_WR_PTR_TYPE_MMU_SET(x)     ((x) |= BUF_DESC_WR_PTR_TYPE_MMU_MASK)
#define BUF_DESC_WR_PTR_TYPE_MMU(x)         ((x) & BUF_DESC_WR_PTR_TYPE_MMU_MASK)

   /* bit 4:       0 - AudioCPU can not modify the remote MMU handle pointed to by read_ptr
    *              1 - AudioCPU can modify the remote MMU handle pointed to by read_ptr
    */
#define BUF_DESC_REMOTE_RDH_MOD_SHIFT       4
#define BUF_DESC_REMOTE_RDH_MOD_MASK        (1 << BUF_DESC_REMOTE_RDH_MOD_SHIFT)
#define BUF_DESC_REMOTE_RDH_MOD_SET(x)      ((x) |= BUF_DESC_REMOTE_RDH_MOD_MASK)
#define BUF_DESC_REMOTE_RDH_MOD(x)          ((x) & BUF_DESC_REMOTE_RDH_MOD_MASK)

   /* bit 5:       0 - AudioCPU can not modify the remote MMU handle pointed to by write_ptr
    *              1 - AudioCPU can modify the remote MMU handle pointed to by write_ptr
    */
#define BUF_DESC_REMOTE_WRH_MOD_SHIFT       5
#define BUF_DESC_REMOTE_WRH_MOD_MASK        (1 << BUF_DESC_REMOTE_WRH_MOD_SHIFT)
#define BUF_DESC_REMOTE_WRH_MOD_SET(x)      ((x) |= BUF_DESC_REMOTE_WRH_MOD_MASK)
#define BUF_DESC_REMOTE_WRH_MOD(x)          ((x) & BUF_DESC_REMOTE_WRH_MOD_MASK)

   /* bit 6:       0 - cbuffer aux_ptr absent
    *              1 - cbuffer aux_ptr present
    *              Note - to be used only for three-handle local MMU buffers.
    *              Note - if set, implies bits 2 & 3 are also both set.
    */
#define BUF_DESC_AUX_PTR_PRESENT_SHIFT      6
#define BUF_DESC_AUX_PTR_PRESENT_MASK       (1 << BUF_DESC_AUX_PTR_PRESENT_SHIFT)
#define BUF_DESC_AUX_PTR_PRESENT_SET(x)     ((x) |= BUF_DESC_AUX_PTR_PRESENT_MASK)
#define BUF_DESC_AUX_PTR_PRESENT(x)         ((x) & BUF_DESC_AUX_PTR_PRESENT_MASK)

   /* bit 7:       0 - if bit 6 is set, the aux_ptr points to the MMU read handle
    *              1 - if bit 6 is set, the aux_ptr points to the MMU write handle
    *              Note - to be used only for three-handle local MMU buffers
    */
#define BUF_DESC_AUX_PTR_TYPE_SHIFT         7
#define BUF_DESC_AUX_PTR_TYPE_MASK          (1 << BUF_DESC_AUX_PTR_TYPE_SHIFT)
#define BUF_DESC_AUX_PTR_TYPE_SET(x)        ((x) |= BUF_DESC_AUX_PTR_TYPE_MASK)
#define BUF_DESC_AUX_PTR_TYPE(x)            ((x) & BUF_DESC_AUX_PTR_TYPE_MASK)

   /* bit  8:      0 - The HostIO subsystem can modify the read handle(s)
    *              1 - The HostIO subsystem cannot modify the read handle(s)
    *              Note - only valid for local MMU buffers
    */
#define BUF_DESC_RD_PTR_HOSTXS_PROT_SHIFT   8
#define BUF_DESC_RD_PTR_HOSTXS_PROT_MASK    (1 << BUF_DESC_RD_PTR_HOSTXS_PROT_SHIFT)
#define BUF_DESC_RD_PTR_HOSTXS_PROT_SET(x)  ((x) |= BUF_DESC_RD_PTR_HOSTXS_PROT_MASK)
#define BUF_DESC_RD_PTR_HOSTXS_PROT(x)      ((x) & BUF_DESC_RD_PTR_HOSTXS_PROT_MASK)

   /* bit 9:       0 - Remote subsystems can modify the read handle(s)
    *              1 - Remote subsystems cannot modify the read handle(s)
    *              Note - only valid for local MMU buffers
    */
#define BUF_DESC_RD_PTR_XS_PROT_SHIFT       9
#define BUF_DESC_RD_PTR_XS_PROT_MASK        (1 << BUF_DESC_RD_PTR_XS_PROT_SHIFT)
#define BUF_DESC_RD_PTR_XS_PROT_SET(x)      ((x) |= BUF_DESC_RD_PTR_XS_PROT_MASK)
#define BUF_DESC_RD_PTR_XS_PROT(x)          ((x) & BUF_DESC_RD_PTR_XS_PROT_MASK)

/* Mask to extract both read handle protection bits */
#define BUF_DESC_RD_PTR_PROT_SHIFT          (BUF_DESC_RD_PTR_HOSTXS_PROT_SHIFT)
#define BUF_DESC_RD_PTR_PROT_MASK           (BUF_DESC_RD_PTR_HOSTXS_PROT_MASK | BUF_DESC_RD_PTR_XS_PROT_MASK)

   /* bit 10:      0 - The HostIO subsystem can modify the write handle(s)
    *              1 - The HostIO subsystem cannot modify the write handle(s)
    *              Note - only valid for local MMU buffers
    */
#define BUF_DESC_WR_PTR_HOSTXS_PROT_SHIFT    10
#define BUF_DESC_WR_PTR_HOSTXS_PROT_MASK     (1 << BUF_DESC_WR_PTR_HOSTXS_PROT_SHIFT)
#define BUF_DESC_WR_PTR_HOSTXS_PROT_SET(x)   ((x) |= BUF_DESC_WR_PTR_HOSTXS_PROT_MASK)
#define BUF_DESC_WR_PTR_HOSTXS_PROT(x)       ((x) & BUF_DESC_WR_PTR_HOSTXS_PROT_MASK)

   /* bit 11:      0 - Remote subsystems can modify the write handle(s)
    *              1 - Remote subsystems cannot modify the write handle(s)
    *              Note - only valid for local MMU buffers
    */
#define BUF_DESC_WR_PTR_XS_PROT_SHIFT       11
#define BUF_DESC_WR_PTR_XS_PROT_MASK        (1 << BUF_DESC_WR_PTR_XS_PROT_SHIFT)
#define BUF_DESC_WR_PTR_XS_PROT_SET(x)      ((x) |= BUF_DESC_WR_PTR_XS_PROT_MASK)
#define BUF_DESC_WR_PTR_XS_PROT(x)          ((x) & BUF_DESC_WR_PTR_XS_PROT_MASK)

/* Mask to extract both write handle protection bits */
#define BUF_DESC_WR_PTR_PROT_SHIFT          (BUF_DESC_WR_PTR_HOSTXS_PROT_SHIFT)
#define BUF_DESC_WR_PTR_PROT_MASK           (BUF_DESC_WR_PTR_HOSTXS_PROT_MASK | BUF_DESC_WR_PTR_XS_PROT_MASK)

/* NB No flags for the aux handle - assume it has the same host access protection as the
 * handle it is mirroring (read or write) */

   /* bit 12:      0 - Remote read handle has no byte-swap
    *              1 - Remote read handle has a byte-swap applied
    *              Note - only valid for remote MMU buffers
    */
#define BUF_DESC_REMOTE_RDH_BSWAP_SHIFT     12
#define BUF_DESC_REMOTE_RDH_BSWAP_MASK      (1 << BUF_DESC_REMOTE_RDH_BSWAP_SHIFT)
#define BUF_DESC_REMOTE_RDH_BSWAP_SET(x)    ((x) |= BUF_DESC_REMOTE_RDH_BSWAP_MASK)
#define BUF_DESC_REMOTE_RDH_BSWAP_UNSET(x)  ((x) &= ~BUF_DESC_REMOTE_RDH_BSWAP_MASK)
#define BUF_DESC_REMOTE_RDH_BSWAP(x)        ((x) & BUF_DESC_REMOTE_RDH_BSWAP_MASK)

   /* bit 13:      0 - Remote write handle has no byte-swap
    *              1 - Remote write handle has a byte-swap applied
    *              Note - only valid for remote MMU buffers
    */
#define BUF_DESC_REMOTE_WRH_BSWAP_SHIFT     13
#define BUF_DESC_REMOTE_WRH_BSWAP_MASK      (1 << BUF_DESC_REMOTE_WRH_BSWAP_SHIFT)
#define BUF_DESC_REMOTE_WRH_BSWAP_SET(x)    ((x) |= BUF_DESC_REMOTE_WRH_BSWAP_MASK)
#define BUF_DESC_REMOTE_WRH_BSWAP_UNSET(x)  ((x) &= ~BUF_DESC_REMOTE_WRH_BSWAP_MASK)
#define BUF_DESC_REMOTE_WRH_BSWAP(x)        ((x) & BUF_DESC_REMOTE_WRH_BSWAP_MASK)

   /* bit 14:      0 - CBOPS is NOT scratch
    *              1 - CBOPS_IS_SCRATCH
    *              when set the buffer is scratch used by CBOPs
    */
#define BUF_DESC_CBOPS_IS_SCRATCH_SHIFT     14
#define BUF_DESC_CBOPS_IS_SCRATCH_MASK      (1 << BUF_DESC_CBOPS_IS_SCRATCH_SHIFT)
#define BUF_DESC_CBOPS_IS_SCRATCH_SET(x)    ((x) |= BUF_DESC_CBOPS_IS_SCRATCH_MASK)
#define BUF_DESC_CBOPS_IS_SCRATCH_UNSET(x)  ((x) &= ~BUF_DESC_CBOPS_IS_SCRATCH_MASK)
#define BUF_DESC_CBOPS_IS_SCRATCH(x)        ((x) & BUF_DESC_CBOPS_IS_SCRATCH_MASK)


   /* bit 15:      0 - Buffer has no metadata associated (metadata field should be ignored)
    *              1 - Buffer may have metadata associated (metadata field should point
    *                  to a valid metadata list, or be NULL)
    */
#define BUF_DESC_METADATA_SHIFT             15
#define BUF_DESC_METADATA_MASK              (1 << BUF_DESC_METADATA_SHIFT)
#define BUF_DESC_METADATA_SET(x)            ((x) |= BUF_DESC_METADATA_MASK)
#define BUF_DESC_METADATA_UNSET(x)          ((x) &= ~BUF_DESC_METADATA_MASK)
#define BUF_DESC_METADATA(x)                ((x) & BUF_DESC_METADATA_MASK)

    /* bit 16:       0 - if bit 0 is set, cbuffer is not part of an in place chain
     *              1 - if bit 0 is set, cbuffer is part of an in place chain
     */
#define BUF_DESC_IN_PLACE_SHIFT             16
#define BUF_DESC_IN_PLACE_MASK              (1 << BUF_DESC_IN_PLACE_SHIFT)
#define BUF_DESC_IN_PLACE_SET(x)            ((x) |= BUF_DESC_IN_PLACE_MASK)
#define BUF_DESC_IN_PLACE(x)                ((x) & BUF_DESC_IN_PLACE_MASK)


#ifdef BAC32
#define BUF_DESC_SAMP_SIZE_SHIFT            24
#define BUF_DESC_SAMP_SIZE_WIDTH            2
#define BUF_DESC_SAMP_SIZE_MASK             (((1 << BUF_DESC_SAMP_SIZE_WIDTH) - 1) << BUF_DESC_SAMP_SIZE_SHIFT)
#endif

/*
 * In platforms with BAC32, the sample size field if required needs to be read from the
 * BAC handle. The sample size field (SAMPLE_SIZE_8BIT etc) is not stored in the cbuffer
 * descriptor.
 * cbuffer constructors however require the sample size to be specified as
 * the maximum buffer offset in the BAC handle depends on the sample size.
 * Bit 24 & bit 25 of the "flags" argument to cbuffer_create_mmu_buffer()
 * are used to specify the sample size.
 * These sample size bits unlike other flags are not stashed in the cbuffer descriptor.
 * All flags other than the sample size are sticky. So the sticky flag mask is 0xFFFFFF.
 *
 * In platforms with BAC24, there is no notion of sample size so this feature is not used.
 * (Also, integer width being 24 bits, one cannot use bit 24 and bit 25.)
 */
#define BUF_DESC_STICKY_FLAGS_MASK          (0xFFFFFFu)

/*
 * Flags shortcut values - saves the effort of constructing the flags from individual bits
 * A pure SW buffer (Note - a SW buffer can be created with cbuffer_create()
 * or cbuffer_create_with_malloc())
 */
#define BUF_DESC_SW_BUFFER                      0

/* This is the base set of flags we use for most local MMU buffers.
 * By default we give the ability to modify MMU buffers to all remote subsystems apart from hostIO */
#define BUF_DESC_LOCAL_MMU_BUFFER               (BUF_DESC_BUFFER_TYPE_MMU_MASK | BUF_DESC_RD_PTR_HOSTXS_PROT_MASK | BUF_DESC_WR_PTR_HOSTXS_PROT_MASK)
/* cbuffer which wraps a local MMU handle - the read handle is controlled by the MMU HW.
 * We don't permit remote subsystems to modify the read handle. */
#define BUF_DESC_MMU_BUFFER_HW_RD               (BUF_DESC_LOCAL_MMU_BUFFER | BUF_DESC_RD_PTR_TYPE_MMU_MASK | BUF_DESC_RD_PTR_XS_PROT_MASK)
/* cbuffer which wraps a local MMU handle - the write handle is controlled by the MMU HW.
 * We don't permit remote subsystems to modify the write handle. */
#define BUF_DESC_MMU_BUFFER_HW_WR               (BUF_DESC_LOCAL_MMU_BUFFER | BUF_DESC_WR_PTR_TYPE_MMU_MASK | BUF_DESC_WR_PTR_XS_PROT_MASK)
/* cbuffer which wraps two local MMU handles - the MMU hardware is both reading and writing.
 * Can't infer the protection flags, so permit remote read & write. */
#define BUF_DESC_MMU_BUFFER_HW_RD_WR            (BUF_DESC_LOCAL_MMU_BUFFER | BUF_DESC_RD_PTR_TYPE_MMU_MASK | BUF_DESC_WR_PTR_TYPE_MMU_MASK)

#ifdef BAC32
#define BUF_DESC_UNPACKED_8BIT_MASK  (SAMPLE_SIZE_8BIT  << BUF_DESC_SAMP_SIZE_SHIFT)
#define BUF_DESC_UNPACKED_16BIT_MASK (SAMPLE_SIZE_16BIT << BUF_DESC_SAMP_SIZE_SHIFT)
#define BUF_DESC_UNPACKED_24BIT_MASK (SAMPLE_SIZE_24BIT << BUF_DESC_SAMP_SIZE_SHIFT)
#define BUF_DESC_UNPACKED_32BIT_MASK (SAMPLE_SIZE_32BIT << BUF_DESC_SAMP_SIZE_SHIFT)
#endif

/*@}*/

/* For chips with a 32 bit BAC, this assumes unpacked */
#define SAMPLES_TO_CHARS(s) ((s) * sizeof(int))
#define CHARS_TO_SAMPLES(c) ((c) / sizeof(int))

/****************************************************************************
Public Variable Definitions
*/

/****************************************************************************
Public Function Declarations
*/

/* create / destroy functions are implemented in cbuffer.c */

/**
 * Create a SW cbuffer.
 *
 * This function uses externally allocated memory for the cbuffer data storage.
 * It creates a tCbuffer object and populates its fields.
 *
 * \param cbuffer_data_ptr pointer to externally allocated buffer.
 * \param buffer_size size of externally allocated buffer in words.
 * \param descriptor is the bit field cbuffer descriptor.
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 */
extern tCbuffer *cbuffer_create(void *cbuffer_data_ptr, unsigned int buffer_size, unsigned int descriptor);

/**
 * Create a SW cbuffer with malloc.
 *
 * This function mallocs a buffer buffer_size words big for the cbuffer data storage,
 * creates a tCbuffer object and populates its fields.
 * This function allocates memory from SLOW ram if it exists on the device.
 *
 * \param buffer_size is the requested size of the buffer in words.
 * \param descriptor is the bit field cbuffer descriptor.
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 */
extern tCbuffer *cbuffer_create_with_malloc(unsigned int buffer_size, unsigned int descriptor);

/**
 * Create a SW cbuffer with malloc.
 *
 * This function mallocs a buffer buffer_size words big for the cbuffer data storage,
 * creates a tCbuffer object and populates its fields.
 * This function allocates memory from FAST ram if it exists on the device.
 *
 * \param buffer_size is the requested size of the buffer in words.
 * \param descriptor is the bit field cbuffer descriptor.
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 */
extern tCbuffer *cbuffer_create_with_malloc_fast(unsigned int buffer_size, unsigned int descriptor);

/**
 * Configure cbuffer structure.
 *
 * This function uses the information in buffer, buffer_size and descriptor
 * to update the cbuffer structure.
 *
 * \param cbuffer structure to be updated.
 * \param buffer points to data space allocated for the cbuffer.
 * \param buffer_size is the size of the buffer in words.
 * \param descriptor is the bit field cbuffer descriptor.
 * \return none
 */
extern void cbuffer_buffer_configure (tCbuffer *cbuffer, int *buffer, unsigned int buffer_size, unsigned int descriptor);


/**
 * Destroys a cbuffer.
 *
 * Frees up all memory allocated for the cbuffer data and cbuffer structure.
 *
 * Note: for remote buffers the function only frees up the the space allocated
 * for the cbuffer structure.
 *
 * \param cbuffer pointer to the cbuffer structure to be destroyed.
 * \return none.
 */
extern void cbuffer_destroy(tCbuffer *cbuffer);

/**
 * Destroys a cbuffer structure only.
 *
 * Frees up memory allocated for the cbuffer structure, the underlying data buffer
 * is the responsibility of the caller..
 *
 * \param cbuffer pointer to the cbuffer structure to be destroyed.
 * \return none.
 */
extern void cbuffer_destroy_struct(tCbuffer *cbuffer);

/**
 * Updates a cbuffer structure that wraps up scratch memory after a scratch commit
 *
 * \param cbuffer the cbuffer structure to update to reflect the newly commited
 * scratch memory.
 * \param scratch_mem the address where the scratch memory starts that will be
 * used for the buffer.
 */
extern void cbuffer_scratch_commit_update(tCbuffer *cbuffer, void *scratch_mem);


/*******************************************************************************************/
/* the following target abstract functions are implemented in asm top layer (cbuffer_asm)
 * with ramifications (jump) to target specific layer <arch>_mmu_buff_asm for local buffers,
 * or <arch>_cbuff.c for remote buffers (no bluecore remote)
 */

/**
 * Calculate amount of space in a buffer, in words.
 *
 * Access function - Determine free space in the buffer.
 *
 * Note: this function will return (amount of space in the buffer minus one word)
 * so that the buffer cannot be filled up completely.
 *
 * \param cbuffer structure to extract information from.
 * \return free space in the buffer in CPU processing words
 */
extern unsigned int cbuffer_calc_amount_space_in_words (tCbuffer *cbuffer);

/**
 * Calculate amount of space in a buffer, in addresses.
 *
 * Access function - Determine free space in the buffer.
 *
 * Note: this function will return (amount of space in the buffer minus a word worth
 * of locations) so that the buffer cannot be filled up completely.
 *
 * \param cbuffer structure to extract information from.
 * \return free space in the buffer in addresses (or locations)
 */
extern unsigned int cbuffer_calc_amount_space_in_addrs (tCbuffer *cbuffer);

#ifndef KAL_ARCH4
/**
 * Calculate amount of space in a buffer.
 *
 * Access function - Determine free space in the buffer.
 *
 * This deprecated entry point is provided only on 24-bit platforms,
 * for backward compatibility. On 32-bit platforms its meaning would
 * be ambiguous.
 * In new code it is recommended that one of the above entry points
 * be used, for compatibility with 32-bit platforms.
 *
 * Note: this function will return (amount of space in the buffer - 1)
 * so that the buffer cannot be filled up completely.
 *
 * \param cbuffer structure to extract information from.
 * \return free space in the buffer in words/addresses
 */
extern unsigned int cbuffer_calc_amount_space (tCbuffer *cbuffer);
#endif /* KAL_ARCH4 */

/**
 * Calculate amount of data in a buffer, in words.
 *
 * Access function - Determine amount of data in the buffer.
 *
 * \param cbuffer structure to extract information from.
 * \return amount of data in the buffer in CPU processing words
 */
extern unsigned int cbuffer_calc_amount_data_in_words (tCbuffer *cbuffer);

/**
 * Calculate amount of data in a buffer, in addresses.
 *
 * Access function - Determine amount of data in the buffer.
 *
 * \param cbuffer structure to extract information from.
 * \return amount of data in the buffer in addresses (or locations)
 */
extern unsigned int cbuffer_calc_amount_data_in_addrs (tCbuffer *cbuffer);

#ifndef KAL_ARCH4
/**
 * Calculate amount of data in a buffer.
 *
 * Access function - Determine amount of data in the buffer.
 *
 * This deprecated entry point is provided only on 24-bit platforms,
 * for backward compatibility. On 32-bit platforms its meaning would
 * be ambiguous.
 * In new code it is recommended that one of the above entry points
 * be used, for compatibility with 32-bit platforms.
 *
 * \param cbuffer structure to extract information from.
 * \return amount of data in the buffer in words/addresses
 */
extern unsigned int cbuffer_calc_amount_data (tCbuffer *cbuffer);
#endif /* KAL_ARCH4 */

/**
 * \brief Get the size of a cbuffer, in words.
 *
 * \param cbuffer structure to extract information from.
 * \return size of the buffer, in words
 */
extern unsigned int cbuffer_get_size_in_words(tCbuffer *cbuffer);

/**
 * \brief Get the size of a cbuffer, in addresses.
 *
 * \param cbuffer structure to extract information from.
 * \return size of the buffer, in addresses (or locations)
 */
extern unsigned int cbuffer_get_size_in_addrs(tCbuffer *cbuffer);

/**
 * \brief Get the size of a cbuffer, in octets.
 *
 * \param cbuffer structure to extract information from.
 * \return size of the buffer, in octets
 */
extern unsigned int cbuffer_get_size_in_octets(tCbuffer *cbuffer);

/**
 * Read data from cbuffer.
 *
 * Reads min(amount_to_read, data available in buffer) words of data from cbuffer to buffer.
 *
 * \param cbuffer structure to read data from.
 * \param buffer destination of data.
 * \param amount_to_read words to read.
 * \return how much data we actually read (can be different from amount_to_read).
 */
extern unsigned int cbuffer_read(tCbuffer *cbuffer, int * buffer, unsigned int amount_to_read);

/**
 * Write data to cbuffer.
 *
 * Writes min(amount_to_write, space available in buffer) words of data from buffer to cbuffer.
 *
 * \param cbuffer structure to write data to.
 * \param buffer source of data.
 * \param amount_to_write words to write.
 * \return how much data we actually wrote (can be different from amount_to_write).
 */
extern unsigned int cbuffer_write(tCbuffer *cbuffer, int * buffer, unsigned int amount_to_write);

/**
 * Copy data from one cbuffer to another.
 *
 * Copies min(amount_to_copy, data available in cbuffer_src, space_available in
 * cbuffer_dest) words of data from cbuffer_src to cbuffer_dest.
 *
 * \param cbuffer_dest structure to write data to.
 * \param cbuffer_src structure to read data from.
 * \param amount_to_copy words to copy.
 * \return how much data we actually copied (can be different from amount_to_copy).
 */
extern unsigned int cbuffer_copy(tCbuffer *cbuffer_dest, tCbuffer *cbuffer_src, unsigned int amount_to_copy);

/**
 * Moves read pointer to the write pointer in the cbuffer, keeping a specified
 * distance behind the write pointer.
 * Only use it on buffer where write pointer is modifiable / owned by caller.
 *
 * \param cbuffer  pointer to cbuffer structure to change pointers in.
 * \param distance  distance to be left between read and write positions.
 */
extern void cbuffer_move_read_to_write_point(tCbuffer* cbuffer, unsigned int distance);

/**
 * Moves write pointer to the read pointer in the cbuffer, keeping a specified
 * distance ahead of the read pointer.
 * Only use it on buffer where write pointer is modifiable / owned by caller.
 *
 * \param cbuffer  pointer to cbuffer structure to change pointers in.
 * \param distance  distance to be left between read and write positions.
 */
extern void cbuffer_move_write_to_read_point(tCbuffer* cbuffer, unsigned int distance);

/**
 * Empties all the available data in the supplied cbuffer.
 *
 * \param cbuffer pointer to a cbuffer structure.
 * \return none
 */
extern void cbuffer_empty_buffer(tCbuffer *cbuffer);

#ifdef INSTALL_METADATA
/**
 * cbuffer_empty_buffer_and_metadata
 * \brief clearing cbuffer, and if it has associated metadata, it will be cleared too.
 * \param cbuffer input buffer
 * Note: - Having metadata isn't necessary, if the buffer doesn't have metadata
 *         this will be same as buffer_empty_buffer.
 *         We could extend cbuffer_empty_buffer function instead of having
 *         new function, however at this time we didn't want to change the
 *         behaviour of cbuffer_empty_buffer function.
 */
extern void cbuffer_empty_buffer_and_metadata(tCbuffer *cbuffer);
#endif  /* INSTALL_METADATA */

/**
 * Fills all the available space in a cbuffer with a supplied value.
 *
 * \param cbuffer pointer to a cbuffer structure.
 * \param fill_value Value to be filled in.
 * \return none
 */
extern void cbuffer_fill_buffer(tCbuffer *cbuffer, int fill_value);

/**
 * Completely fills a cbuffer with a supplied value, empties it,
 * disregarding any data that might be in there.
 *
 * This is different to cbuffer_fill_buffer because that function
 * only fills up the available free space in the buffer. This fills the
 * _entire_ buffer, starting at the current write pointer.
 * Q. Why would we want to do that?
 * A. Because we might have some bit of hardware (e.g. PCM) looking at the
 * buffer, and we want to clear out stale data and replace it with (say)
 * silence.
 *
 * \param cbuffer pointer to a cbuffer structure.
 * \param fill_value Value to be filled in.
 * \return none
 */
extern void cbuffer_flush_and_fill(tCbuffer *cbuffer, int fill_value);

/**
 * Discard a specified amount of data.
 *
 * \param cbuffer pointer to a cbuffer structure.
 * \param discard_amount Amount of data to discard.
 * \return none
 */
extern void cbuffer_discard_data(tCbuffer *cbuffer, unsigned int discard_amount);

/**
 * Fill a number of locations in a buffer with a value, from current write pointer onwards.
 * It also advances write pointer accordingly.
 *
 * \param cbuffer pointer to a cbuffer structure.
 * \param amount Amount of buffer locations to fill.
 * \param value  Value to write to the specified number of locations.
 * \return none
 */
extern void cbuffer_block_fill(tCbuffer *cbuffer, unsigned int amount, unsigned int value);

/**
 * \brief Used to set the write address of a buffer after an operation has been
 * performed on it.
 *
 * \param cbuffer pointer to a cbuffer structure.
 * \param write_address the address to set as the write pointer on the buffer.
 */
extern void cbuffer_set_write_address(tCbuffer *cbuffer, unsigned int *write_address);

/**
 * \brief Used to set the read address of a buffer after an operation has been
 * performed on it.
 *
 * \param cbuffer pointer to a cbuffer structure.
 * \param read_address the address to set as the read pointer on the buffer.
 */
extern void cbuffer_set_read_address(tCbuffer *cbuffer, unsigned int *read_address);

/**
 *  Advances read pointer of a cbuffer by a supplied value
 *
 * \param cbuffer pointer to cbuffer to modify
 * \param amount value by which the read pointer advances
 * \return none
 *
 */
extern void cbuffer_advance_read_ptr(tCbuffer *cbuffer, unsigned int amount);


/**
 *  Advances write pointer of a cbuffer by a supplied value
 *
 * \param cbuffer pointer to cbuffer to modify
 * \param amount value by which the write pointer advances
 * \return none
 *
 */
extern void cbuffer_advance_write_ptr(tCbuffer *cbuffer, unsigned int amount);

/*******************************************************************************************

Target specific

*******************************************************************************************/

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
/* Hydra target specific */

/**
 * Create a cbuffer that wraps a local MMU buffer.
 *
 * This function mallocs a buffer buffer_size words big for the MMU buffer data storage.
 * If buffer_size is not a multiple of a page size (64 octets)
 * buffer_size will be altered by the function.
 * It creates a tCbuffer object and populates its fields.
 *
 * \param flags aggregated flags referring to read_ptr, write_ptr and aux_ptr
 * \param buffer_size is the requested size of the buffer in words.
 * \note Any valid combination of flags is permitted here; we could use this
 *       function to create a pure SW buffer if we wanted to.
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 * \note target specific implementation; bluecore ports, will ignore buffer_size argument
 */
extern tCbuffer *cbuffer_create_mmu_buffer(unsigned int flags, unsigned int *buffer_size);

/**
 * Create a cbuffer that wraps a local MMU buffer.
 *
 * As cbuffer_create_mmu_buffer but the underlying memory is allocated from FAST RAM.
 *
 * \param flags aggregated flags referring to read_ptr, write_ptr and aux_ptr
 * \param buffer_size is the requested size of the buffer in words.
 * \note Any valid combination of flags is permitted here; we could use this
 *       function to create a pure SW buffer if we wanted to.
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 * \note target specific implementation; bluecore ports, will ignore buffer_size argument
 */
extern tCbuffer *cbuffer_create_mmu_buffer_fast(unsigned int flags, unsigned int *buffer_size);

#endif      /* CHIP_BASE_HYDRA || CHIP_BASE_NAPIER*/


#ifdef CHIP_BASE_BC7
/*******************************************************************************************/
/* Bluecore target specific */

/**
 * Checks a Bluecore port valid / enabled.
 *
 * \param cbuffer pointer to tCbuffer object wrapping the port.
 * \param port read or write port number (from zero).
 * \return TRUE if port is valid and anabled.
 */
extern bool cbuffer_is_port_enabled(tCbuffer *cbuffer, unsigned combined_port_number);

/**
 * Create a cbuffer that wraps a Bluecore port
 *
 * This function creates a tCbuffer object and populates its fields.
 *
 * \param flags aggregated flags referring to either HW read_ptr or write_ptr
 * \param port read or write port number (from zero)
 * \note flags must be specific for MMU access, only one direction (either RD or WR port)
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 */
extern tCbuffer *cbuffer_wrap_bc_port(unsigned int flags, unsigned int port);

/**
 * Reconfigures a Bluecore read port
 *
 * \param port read port number (from zero)
 * \param config bitfield to go to READ_PORTx_CONFIG
 */
extern void cbuffer_reconfigure_read_port(unsigned int port, unsigned int config);

/**
 * Reconfigures a Bluecore write port
 *
 * \param port write port number (from WRITE_PORT_OFFSET)
 * \param config bitfield to go to WRITE_PORTx_CONFIG
 */
extern void cbuffer_reconfigure_write_port(unsigned int port, unsigned int config);

/**
 * \brief Used to calculate that amount of data through a write port. This is a risky operation and should ONLY
 *  used when the caller can guarantee that the buffer is fully backed permanently.
 *
 * \param cbuffer pointer to write port buffer.
 * \return amount of data in write port.
 */

extern unsigned int cbuffer_write_port_level(tCbuffer *cbuffer);
#endif      /* CHIP_BASE_BC7 */


#ifdef PL_TRACE_PRINTF
/**
 * Print out the contents of a cbuffer pointer
 *
 * Prints out the useful bits of a buffer, working out where offsets
 * are and taking into account things like MMU handles for hydra platforms.
 *
 * \param cbuffer pointer to the cbuffer structure.
 */
extern void cbuffer_print(tCbuffer *cbuffer);
#endif

#ifdef INSTALL_CBUFFER_EX
/*
 * Cbuffer extension library that can track octets within a word.
 * Using this library extension with buffers other 16-bit unpacked is undefined.
 * See cbuffer_ex.asm for further details.
 */

/*
 * \brief get the true read address and the read octet offset of a cbuffer
 * \param cb cbuffer pointer
 * \param offset pointer to the read octet offset. cbuffer_get_read_address_ex stores
    the read octet offset at this address.
 * \return true read address
 */
extern unsigned int * cbuffer_get_read_address_ex(tCbuffer * cbuffer, unsigned * offset);

/*
 * \brief get the true write address and the write octet offset of a cbuffer
 * \param cb cbuffer pointer
 * \param offset pointer to the write octet offset. cbuffer_get_write_address_ex stores
 *  the write octet offset at this address.
 * \return true write address
 */
extern unsigned int * cbuffer_get_write_address_ex(tCbuffer * cbuffer, unsigned * offset);

/*
 * \brief calculate data available in octets
 * \param  cb cbuffer pointer
 * \return data available in octets
 */
extern unsigned cbuffer_calc_amount_data_ex(tCbuffer * cb);

/*
 * \brief calculate (space available in octets) - (max octets in word)
    example: max octets in word is 2 for 16-bit unpacked buffers. For 16-bit
    unpacked buffers this will cause a buffer of size 128 words (256 octets)
    to hold a maximum of 254 octets. It is possible to maintain this buffer
    fullness invariant when audio writes into the buffer. However if an external
    entity like apps writes into the buffer the maximum data in the 16-bit unpacked
    buffer discussed above can be 255 octets. In this scenario
    (space available in octets) - (max octets in word) can become a negative number
    ie: -1. In this case space available is clipped to zero.
 * \param  cb cbuffer pointer
 * \return true space available in octets less (max octets in word), clipped to zero.
 */
extern unsigned cbuffer_calc_amount_space_ex(tCbuffer *cbuffer);

/*
 * \brief set the true read address and read octet offset of a cbuffer.
 * \param cb cbuffer pointer
 * \param ra read address that is word aligned
 * \param ro read octet offset
 */
extern void cbuffer_set_read_address_ex(tCbuffer * cbuffer, unsigned int * ra, unsigned ro);

/*
 * \brief set the true write address and write octet offset of a cbuffer
 * \param cb cbuffer pointer
 * \param wa true write address that is word aligned
 * \param wo write octet offset
 */
extern void cbuffer_set_write_address_ex(tCbuffer * cbuffer, unsigned int * wa, unsigned wo);

/*
 * \brief advance the read pointer by a certain number of octets
 * \param cb cbuffer pointer
 * \param num_octets number of octets to advance the read pointer by
 */
extern void cbuffer_advance_read_ptr_ex(tCbuffer * cbuffer, unsigned num_octets);

/*
 * \brief advance the write pointer by a certain number of octets
 * \param cb cbuffer pointer
 * \param num_octets number of octets to advance the write pointer by
 */
extern void cbuffer_advance_write_ptr_ex(tCbuffer * cbuffer, unsigned num_octets);

/*
 * \brief copy octets from one cbuffer to another. The cbuffers must be unpacked, 16-bit
 *  big endian without any shift configured.
 * \param dst destination cbuffer pointer
 * \param src source cbuffer pointer
 * \param num_octets number of octets to copy
 * \return number of octets copied.
 */
extern unsigned cbuffer_copy_16bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets);

/*
 * \brief return the size of a cbuffer (in valid number of octets)
    For a 16-bit unpacked buffer of 128 words, this function will return
    256.
 * \param in cb cbuffer address
 * \return size of a cbuffer in valid number of octets
 */
static inline unsigned cbuffer_get_size_ex(tCbuffer * cb)
{
    /* Only supports 16-bit unpacked */
    return cbuffer_get_size_in_words(cb) << 1;
}
#endif /* INSTALL_CBUFFER_EX */

#endif /* CBUFFER_C_H */

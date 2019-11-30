/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer Buffer Subsystem
 * \file  hydra_cbuff.h
 * Public header file for the cbuffer interface
 * \ingroup buffer
 *
 */

#ifndef HYDRA_CBUFFER_H
#define HYDRA_CBUFFER_H

/****************************************************************************
Include Files
*/

#include "types.h"
#include "cbuffer_c.h"
#include "hydra_mmu_buff.h"

/****************************************************************************
Public Type Declarations
*/

/* Enum for sample size.
 * TODO: Future hardware implementation is set to combine these values with packing in
 * some way. The following define is correct in current (18/June/2014) image.
 */
typedef enum buffer_sample_size_enum
{
    SAMPLE_8_BIT,
    SAMPLE_16_BIT,
    SAMPLE_24_BIT,
    SAMPLE_32_BIT
} buffer_sample_size;

/****************************************************************************
Public Constant Declarations
*/

/****************************************************************************
Public Macro Declarations
*/

/* cbuffer presents buffers to code running on the Kalimba as stores
 * of "words", where a word is dependent on the Kalimba core
 * (24 or 32 bits).
 *
 * The MMU/BAC hardware, and lower layers of the buffer software
 * (e.g. mmu), present buffers as stores of octets (to the outside
 * world, e.g. PCM hardware, and to local software).
 * The amount of significant data in each 'word' might not be the
 * full RAM occupancy of that word; e.g. on Amber, to the buffer
 * hardware, only 16 bits of each 24-bit RAM word contain 'real'
 * data, and the buffer offset only increments by 2 octets per
 * word of RAM occupied.
 *
 * These macros represent the relationship between these two levels
 * of abstraction -- "offsets per word of RAM".
 *
 * Also these macro names are a bit ropey for their purpose.
 * Also mind the ptr_to_octet and octet_to_ptr in hydra_mmu_private.h
 *
 * Note: These macros are only used in code that is not expected to
 * execute on platforms with BAC32 (eg: cbuffer_copy_remote_buff)
 *
 */

/* TODO macros for converting 16-bit (half)words to octets and back */
#define buffer_words_to_octets(x) ((x)*2)
#define buffer_octets_to_words(x) ((x)/2)

/**
 * \name tCbuffer descriptor definitions Hydra specific
 */
/*@{*/

/* This is the base set of flags we use for all remote MMU buffers. */
#define BUF_DESC_REMOTE_MMU_BUFFER              (BUF_DESC_BUFFER_TYPE_MMU_MASK | BUF_DESC_IS_REMOTE_MMU_MASK)
/* cbuffer which wraps two remote MMU handles - we are allowed to modify the read handle */
#define BUF_DESC_WRAP_REMOTE_MMU_MOD_RD         (BUF_DESC_REMOTE_MMU_BUFFER | BUF_DESC_REMOTE_RDH_MOD_MASK)
/* cbuffer which wraps two remote MMU handles - we are allowed to modify the write handle */
#define BUF_DESC_WRAP_REMOTE_MMU_MOD_WR         (BUF_DESC_REMOTE_MMU_BUFFER | BUF_DESC_REMOTE_WRH_MOD_MASK)
/* cbuffer which wraps two remote MMU handles - we are allowed to modify the read and write handles */
#define BUF_DESC_WRAP_REMOTE_MMU_MOD_RD_WR      (BUF_DESC_REMOTE_MMU_BUFFER | BUF_DESC_REMOTE_RDH_MOD_MASK | BUF_DESC_REMOTE_WRH_MOD_MASK)

/* cbuffer which wraps three local MMU handles - the AUX handle is used for reading.
 * We permit remote subsystems to modify the read and aux-read handles, but not the write handle. */
#define BUF_DESC_MMU_BUFFER_AUX_RD              (BUF_DESC_MMU_BUFFER_HW_RD_WR | BUF_DESC_AUX_PTR_PRESENT_MASK | BUF_DESC_WR_PTR_XS_PROT_MASK)
/* cbuffer which wraps three local MMU handles - the AUX handle is used for writing.
 * We permit remote subsystems to modify the write and aux-write handles, but not the read handle. */
#define BUF_DESC_MMU_BUFFER_AUX_WR              (BUF_DESC_MMU_BUFFER_HW_RD_WR | BUF_DESC_AUX_PTR_PRESENT_MASK | BUF_DESC_AUX_PTR_TYPE_MASK | BUF_DESC_RD_PTR_XS_PROT_MASK)

/*@}*/

/*
 * Definitions for each of the possible types of aux handle returned by
 * cbuffer_aux_in_use()
 */
/* There is no aux handle */
#define BUFF_AUX_PTR_TYPE_NONE                  0
/* The aux handle is used for read */
#define BUFF_AUX_PTR_TYPE_READ                  1
/* The aux handle is used for write */
#define BUFF_AUX_PTR_TYPE_WRITE                 2

/*
 * Chars are 'C' chars (smallest addressable unit of the machine).
 * BAC Offsets are "packed octets".
      eg: Two 8-bit packed samples = 2 BAC offsets
          Two 16-bit packed samples = 4 BAC offsets
          Two 16-bit unpacked samples = 4 BAC offsets
          Two 32-bit unpacked samples = 8 BAC offsets
 * Samples are Audio Samples (Words)
 */
#if defined (BAC24)
    /* Reasonably assumes char/sample size to be 24 bits
     * (int) on platforms that use a 24 bit BAC.
     *
     * No notion of packing or sample size for a 24 bit BAC.
     * The BAC is hardcoded to see only 16 bits out of 24.
     */
    #define CHARS_TO_BAC_OFFSET(c, flags) ((c) << 1)
    #define SAMPLES_TO_BAC_OFFSET(s, flags) ((s) << 1)
    #define BAC_OFFSET_TO_CHARS(b, flags) ((b) >> 1)
    #define BAC_OFFSET_TO_SAMPLES(b, flags) ((b) >> 1)
#elif defined(BAC32)
    /* Not supporting 24 bit unpacked as that requires
     * the stack to understand B' mode 1
     */
    #define IS_32BIT_UNPACKED(flags) (MMU_BUF_SAMP_SZ_GET(flags) == SAMPLE_SIZE_32BIT)
    #define BAC_OFFSET_TO_SAMPLES(b, flags) ((b) >> (IS_32BIT_UNPACKED(flags) ? 2 : MMU_BUF_SAMP_SZ_GET(flags)))
    #define BAC_OFFSET_TO_CHARS(b, flags)   ((MMU_BUF_PKG_EN_GET(flags) || IS_32BIT_UNPACKED(flags)) ? (b) :  ((b) << (2 - MMU_BUF_SAMP_SZ_GET(flags))))
    /* Assumes unpacked */
    #define SAMPLES_TO_BAC_OFFSET(s, flags) ((s) * (MMU_BUF_SAMP_SZ_GET(flags) + 1))
    #define CHARS_TO_BAC_OFFSET(c, flags)   (MMU_BUF_PKG_EN_GET(flags) ? (c) : (((c) >> 2) * (MMU_BUF_SAMP_SZ_GET(flags) + 1)))
#else  /* Unknown BAC */
    #error "undefined char<->sample<->BAC Offset conversion macros for this BAC"
#endif

/****************************************************************************
Public Variable Definitions
*/

/****************************************************************************
Public Function Declarations
*/

/* These are implemented in cbuffer.c */

/**
 * Create a cbuffer that wraps a remote MMU buffer.
 *
 * It creates a tCbuffer object and populates its fields.
 *
 * \param flags aggregated flags referring to permissions to modify read and write handles
 * \param read_handle mmu handle used for read (in Hydra sense).
 * \param write_handle mmu handle used for write (in Hydra sense).
 * \param buffer_size is the size of the buffer in words.
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 */
extern tCbuffer *cbuffer_wrap_remote(int flags, mmu_handle read_handle, mmu_handle write_handle, unsigned int buffer_size);

#ifndef TODO_BUFFER_ONE_OCTET
/****************************************************************************
 *
 * cbuffer_copy_one_octet - copy a single octet from one cbuffer to another
 *
 * \param cbuffer_src pointer to a cbuffer structure to read from
 * \param cbuffer_dest pointer to a cbuffer structure to write to
 * \param append if TRUE, copy the octet to word before cbuffer_dest->wrp
 * \return TRUE if we copied an octet, FALSE otherwise
 *
 * \note Unlike the other cbuffer read/write functions, this one also adjusts
 * (or not) the write pointer of cbuffer_dest so that it points to the next
 * full word to be written.
 */
extern bool cbuffer_copy_one_octet(tCbuffer *cbuffer_dest, tCbuffer *cbuffer_src, bool append);
#endif

/**
 * Get read offset.
 *
 * Access function - gets the offset into the buffer of the read pointer.
 *
 * \param cbuffer structure to extract information from.
 * \return read offset, in words
 *
 */
extern unsigned int cbuffer_get_read_offset(tCbuffer *cbuffer);

/**
 * Get write offset.
 *
 * Access function - gets the offset into the buffer of the write pointer.
 *
 * \param cbuffer structure to extract information from.
 * \return write offset, in words
 */
extern unsigned int cbuffer_get_write_offset(tCbuffer *cbuffer);

/**
 * Set read offset.
 *
 * Access function - sets the offset into the buffer of the read pointer.
 * NOTE - Only use this if you know what you're doing! The caller must
 * take into account how the cbuffer will wrap.
 * Consider using cbuffer_advance_read_pointer instead.
 *
 * \param cbuffer structure to modify.
 * \param offset read offset, in words.
 * \return none
 */
extern void cbuffer_set_read_offset(tCbuffer *cbuffer, unsigned int offset);

/**
 * Set write offset.
 *
 * Access function - sets the offset into the buffer of the write pointer.
 * NOTE - Only use this if you know what you're doing! The caller must
 * take into account how the cbuffer will wrap.
 * Consider using cbuffer_advance_write_pointer instead.
 *
 * \param cbuffer structure to modify.
 * \param offset write offset, in words.
 * \return none
 */
extern void cbuffer_set_write_offset(tCbuffer *cbuffer, unsigned int offset);

/**
 * Is read pointer pointing to a buffer handle?
 *
 * Determines if the read pointer points to a buffer handle or a data space.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \return TRUE if read_ptr points to a MMU buffer handle, FALSE otherwise.
 */
extern bool cbuffer_is_read_handle_mmu(tCbuffer *cbuffer);


/**
 * Is write pointer pointing to a buffer handle?
 *
 * Determines if the write pointer points to a buffer handle or a data space.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \return TRUE if write_ptr points to MMU buffer handle, FALSE otherwise.
 */
extern bool cbuffer_is_write_handle_mmu(tCbuffer *cbuffer);


/**
 * Is the cbuffer wrapping a remote MMU buffer?
 *
 * Determines if the cbuffer structure wraps a remote MMU handle.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \return TRUE if cbuffer wraps a remote MMU buffer, FALSE otherwise.
 */
extern bool cbuffer_is_remote(tCbuffer *cbuffer);

/**
 * Does this cbuffer contain an aux handle?
 *
 * Determines if the cbuffer structure contains an auxiliary MMU handle,
 * and reports whether it's in use for either read or write
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \return BUFF_AUX_PTR_TYPE_NONE if there is no aux handle,
 *         BUFF_AUX_PTR_TYPE_READ if there is an aux handle used for read,
 *         BUFF_AUX_PTR_TYPE_WRITE if there is an aux handle for write.
 */
extern int cbuffer_aux_in_use(tCbuffer *cbuffer);

/**
 * Get read mmu handle
 *
 * Constructs a mmu handle(in hydra sense) from a cbuffer structure.
 *
 * If read_ptr in cbuffer structure does not point to a buffer handle
 * the function returns an empty handle.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \return mmu handle understood by hydra code.
 */
extern mmu_handle cbuffer_get_read_mmu_handle(tCbuffer *cbuffer);

/**
 * Get write mmu handle
 *
 * Constructs a mmu handle(in hydra sense) from a cbuffer structure.
 *
 * If write_ptr in cbuffer structure does not point to a buffer handle
 * the function returns an empty handle.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \return mmu handle understood by hydra code.
 */
extern mmu_handle cbuffer_get_write_mmu_handle(tCbuffer *cbuffer);

/**
 * Get auxiliary mmu handle
 *
 * Constructs a mmu handle(in hydra sense) from a cbuffer structure.
 *
 * If the cbuffer does not contain an auxiliary buffer handle,
 * the function returns an empty handle.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \return mmu handle understood by hydra code.
 */
extern mmu_handle cbuffer_get_aux_mmu_handle(tCbuffer *cbuffer);

/**
 * Sets the shift amount for the read mmu handle
 *
 * Sets up the BAC hardware for a local read mmu handle to automatically right
 * shift read data by a number of bits.
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * read handle, it will be configured likewise.
 *
 * If read_ptr in cbuffer structure does not point to a local MMU buffer
 * handle the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param amount size of bit shift.
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_read_shift(tCbuffer *cbuffer, int amount);

/**
 * Sets the shift amount for the write mmu handle
 *
 * Sets up the BAC hardware for a local write mmu handle to automatically left
 * shift written data by a number of bits.
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * write handle, it will be configured likewise.
 *
 * If write_ptr in cbuffer structure does not point to a local MMU buffer
 * handle the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param amount size of bit shift.
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_write_shift(tCbuffer *cbuffer, int amount);

/**
 * Sets the byte swap for the read mmu handle
 *
 * Sets up the BAC hardware for a remote or local mmu handle to
 * automatically swap the bytes in a 16 bit read-buffer.
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * read handle, it will be configured likewise.
 *
 * If read_ptr in cbuffer structure does not point to a local MMU buffer
 * handle or compatible remote buffer, the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param set a bool indicating whether to set or unset byte swap feature, TRUE
 *        is a set, FALSE is a clear
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_read_byte_swap(tCbuffer *cbuffer, bool set);

/**
 * Sets the byte swap for the write mmu handle
 *
 * Sets up the BAC hardware for a remote or local mmu handle to
 * automatically swap the bytes in a 16 bit write-buffer.
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * write handle, it will be configured likewise.
 *
 * If write_ptr in cbuffer structure does not point to a local MMU buffer
 * handle or compatible remote buffer, the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param set a bool indicating whether to set or unset byte swap feature, TRUE
 *        is a set, FALSE is a clear
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_write_byte_swap(tCbuffer *cbuffer, bool set);

/**
 * Gets the shift amount currently set for the read mmu handle)
 *
 * If read_ptr in cbuffer structure does not point to a local MMU buffer
 * handle the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \return amount size of bit shift.
 */
extern unsigned int cbuffer_get_read_shift(tCbuffer *cbuffer);

/**
 * Gets the shift amount currently set for the write mmu handle
 *
 * If write_ptr in cbuffer structure does not point to a local MMU buffer
 * handle the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \return amount size of bit shift.
 */
extern unsigned int cbuffer_get_write_shift(tCbuffer *cbuffer);

/**
 * Sets the sample size for the read mmu handle
 *
 * Sets up the BAC hardware for a local mmu handle to access data according to
 * sample size.
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * read handle, it will be configured likewise.
 *
 * If read_ptr in cbuffer structure does not point to a local MMU buffer
 * handle, the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param enum indicating the size of data stored in buffer
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_read_sample_size(tCbuffer *cbuffer, buffer_sample_size sample_size);

/**
 * Sets the sample size for the write mmu handle
 *
 * Sets up the BAC hardware for a local mmu handle to access data according to
 * sample size.
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * write handle, it will be configured likewise.
 *
 * If write_ptr in cbuffer structure does not point to a local MMU buffer
 * handle, the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param enum indicating the size of data stored in buffer
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_write_sample_size(tCbuffer *cbuffer, buffer_sample_size sample_size);

/**
 * Sets the sign extend for the read mmu handle
 *
 * Sets up the BAC hardware for a local mmu handle to automatically sign extend samples.
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * read handle, it will be configured likewise.
 *
 * If read_ptr in cbuffer structure does not point to a local MMU buffer
 * handle the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param set a bool indicating whether to set or unset sign extend feature, TRUE
 *        is a set, FALSE is a clear
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_read_sign_extend(tCbuffer *cbuffer, bool set);

/**
 * Sets the sign extend for the write mmu handle
 *
 * Sets up the BAC hardware for a local mmu handle to automatically sign extend samples.
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * write handle, it will be configured likewise.
 *
 * If write_ptr in cbuffer structure does not point to a local MMU buffer
 * handle, the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param set a bool indicating whether to set or unset sign extend feature, TRUE
 *        is a set, FALSE is a clear
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_write_sign_extend(tCbuffer *cbuffer, bool set);

/**
 * Sets the packing for the read mmu handle
 *
 * Sets up the BAC hardware for a local mmu handle to automatically pack samples. (only
 * relevant for 8 & 16-bit samples)
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * read handle, it will be configured likewise.
 *
 * If read_ptr in cbuffer structure does not point to a local MMU buffer
 * handle the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param set a bool indicating whether to set or unset packing feature, TRUE
 *        is a set, FALSE is a clear
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_read_packing(tCbuffer *cbuffer, bool set);

/**
 * Sets the packing for the write mmu handle
 *
 * Sets up the BAC hardware for a local mmu handle to automatically pack samples. (only
 * relevant for 8 & 16-bit samples)
 *
 * If the aux mmu handle associated with the cbuffer is also configured as a
 * write handle, it will be configured likewise.
 *
 * If write_ptr in cbuffer structure does not point to a local MMU buffer
 * handle, the function returns FALSE.
 *
 * \param cbuffer pointer to the cbuffer structure.
 * \param set a bool indicating whether to set or unset packing feature, TRUE
 *        is a set, FALSE is a clear
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool cbuffer_set_write_packing(tCbuffer *cbuffer, bool set);


/**
 * Gets the mmu flags associated with a Cbuffer
 *
 * It is the callers responsibility to ensure that the cbuffer
 * read and/or write pointer is an MMU handle
 *
 * \param cb pointer to cbuffer structure
 * \param read_flags TRUE if read flags are to be returned, FALSE
 *        if write flags are to be returned
 * \return mmu flags
 */
extern unsigned cbuffer_get_mmu_flags(const tCbuffer *cb, bool read_flags);

#if 0
// TODO not needed, added for metadata / audio_data_service (queue),
// answer-me do we keep it?
/*
 * \brief compares circular buffer offsets, off_1 and off_2, returns +/- samples
 *        difference between off_1 and off_2. off_1 and off_2 are
 *        assumed to lag the write pointer or in the worst
 *        case be equal to the write pointer
 * \param off_1 offset 1 in the circular buffer
 * \param off_2 offset 2 in the circular buffer
 * return number of samples between off_1 and off_2, -ve if off_1 lags off_2
 */
extern int cbuffer_compare_offsets(tCbuffer * cb, unsigned off_1, unsigned off_2);
#endif  /* #if 0 */

#endif /* HYDRA_CBUFFER_H */

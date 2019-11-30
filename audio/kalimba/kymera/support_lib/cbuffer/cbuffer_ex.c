/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbuffer_copy_ex.c
 * \ingroup buffer
 *
 *  cbuffer_ex octet handling extension functions.
 *
 *  TODO currently only 16-bit and 32-bit big endian packing without any shift.
 *  Mainly used by the a2dp and the file endpoint to copy encoded data
 *  from one cbuffer to another.
 */

#ifdef INSTALL_CBUFFER_EX

#include "buffer.h"
#include "pl_assert.h"
#include "util.h"
#include "subsys3_patch0_fw000012B2_map_public.h"

#if !defined(PATCH_ENTRY_CBUFFER_COPY_EX)

unsigned cbuffer_get_usable_octets(tCbuffer *buff);
void cbuffer_set_usable_octets(tCbuffer *buff, unsigned usable_octets);
unsigned int cbuffer_get_read_offset_ex(tCbuffer *cbuffer);

/****************************************************************************
Public Function Definitions
*/
unsigned cbuffer_get_usable_octets(tCbuffer *buff)
{
    unsigned usable_octets;
    usable_octets = buff_metadata_get_usable_octets(buff);
    return (usable_octets == 0 ? 4 : usable_octets);
}

void cbuffer_set_usable_octets(tCbuffer *buff, unsigned usable_octets)
{
    buff_metadata_set_usable_octets(buff, usable_octets);
}

/****************************************************************************
 * cbuffer_get_read_offset_ex - returns the read offset of the read pointer wrt
 *                              to the base
 *
 * Input arguments:
 *      cbuffer             - the buffer whose rd offset is to be returned
 *
 * Return value
 *      The read offset in octets.
 */
unsigned int cbuffer_get_read_offset_ex(tCbuffer *cbuffer)
{
    int *read_ptr;

    read_ptr = cbuffer->read_ptr;

    PL_ASSERT(!BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor) &&
              !BUF_DESC_RD_PTR_TYPE_MMU(cbuffer->descriptor));

    return ((uintptr_t)read_ptr - (uintptr_t)cbuffer->base_addr);
}
/*
 * \brief copy octets from one cbuffer to another. The input cbuffer must be
 *  16-bit unpacked and the output buffer 32-bit packed, both big endian and
 *  without any shift configured;
 *
 * \param dst destination cbuffer pointer
 * \param src source cbuffer pointer
 * \param num_octets number of octets to copy
 */
extern void cbuffer_copy_unpacked_to_packed_ex(tCbuffer * dst, tCbuffer * src, unsigned num_octets);
/*
 * \brief copy octets from one cbuffer to another. The input cbuffer must be
 *  32-bit packed and the output buffer 16-bit unpacked, both big endian and
 *  without any shift configured;
 *
 * \param dst destination cbuffer pointer
 * \param src source cbuffer pointer
 * \param num_octets number of octets to copy
 */
extern void cbuffer_copy_packed_to_unpacked_ex(tCbuffer * dst, tCbuffer * src, unsigned num_octets);
/*
 * \brief copy octets from one cbuffer to another. The cbuffers must be unpacked,
 * 16-bit, big endian, without any shift configured and _aligned_;
 * (aligned: the source read octet offset (within a word) and the destination
 * write octet offset (within a word) must be equal.
 *
 * \param dst destination cbuffer pointer
 * \param src source cbuffer pointer
 * \param num_octets number of octets to copy
 */
extern void cbuffer_copy_aligned_16bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer * src, unsigned num_octets);
/*
 * \brief copy octets from one cbuffer to another. The cbuffers must be unpacked,
 * 16-bit, big endian, without any shift configured and _unaligned_;
 * the unaligned version is MIPS heavier, so keep the aligned variant separate
 *
 * \param dst destination cbuffer pointer
 * \param src source cbuffer pointer
 * \param num_octets number of octets to copy
*/
extern void cbuffer_copy_unaligned_16bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer * src, unsigned num_octets);
/*
 * \brief copy octets from one cbuffer to another. The cbuffers must be packed,
 * 32-bit, big endian, without any shift configured and _aligned_;
 * (aligned: the source read octet offset (within a word) and the destination
 * write octet offset (within a word) must be equal.
 * \param dst destination cbuffer pointer
 * \param src source cbuffer pointer
 * \param num_octets number of octets to copy
 */
extern void cbuffer_copy_aligned_32bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer * src, unsigned num_octets);
/*
 * \brief copy octets from one cbuffer to another. The cbuffers must be packed,
 * 32-bit, big endian, without any shift configured and _unaligned_;
 * the unaligned version is MIPS heavier, so keep the aligned variant separate
 *
 * \param dst destination cbuffer pointer
 * \param src source cbuffer pointer
 * \param num_octets number of octets to copy
*/
extern void cbuffer_copy_unaligned_32bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer * src, unsigned num_octets);


/****************************************************************************
 * cbuffer_copy_ex - copies from a 16-bit unpacked/32-bit packed buffer to a
 *                   16-bit unpacked/32-bit packed buffer
 *
 * Input arguments:
 *      dst                 - destination buffer
 *      src                 - source buffer
 *      num_octets          - number of octets to be copied
 *
 * Return value
 *      The number of copied octets. (NOTE: the ASM implementation does not
 *      return the copied octets)
 */
unsigned cbuffer_copy_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets);
unsigned cbuffer_copy_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets)
{
    unsigned src_data, dest_space, octets_to_copy, r_octet_offset, w_octet_offset;
    unsigned us_octets_source = cbuffer_get_usable_octets(src);
    unsigned us_octets_dest = cbuffer_get_usable_octets(dst);

    /* Only 2 or 4 usable octets are supported. */
    PL_ASSERT((us_octets_source == 2) || (us_octets_source == 4));
    PL_ASSERT((us_octets_dest == 2) || (us_octets_dest == 4));

    src_data = cbuffer_calc_amount_data_ex(src);
    dest_space = cbuffer_calc_amount_space_ex(dst);

    octets_to_copy = MIN(src_data, num_octets);
    octets_to_copy = MIN(octets_to_copy, dest_space);

    if (octets_to_copy == 0)
    {
        return 0;
    }

    cbuffer_get_read_address_ex(src, &r_octet_offset);
    cbuffer_get_write_address_ex(dst, &w_octet_offset);

    if(us_octets_source == 2)
    {
        if(us_octets_dest == 2)
        {
            /* the unaligned version is MIPS heavier, so keep the aligned variant separate */
           if (r_octet_offset == w_octet_offset)
           {
               cbuffer_copy_aligned_16bit_be_zero_shift_ex(dst, src, octets_to_copy);
           }
           else
           {
               cbuffer_copy_unaligned_16bit_be_zero_shift_ex(dst, src, octets_to_copy);
           }
        }
        else
        {
            cbuffer_copy_unpacked_to_packed_ex(dst, src, octets_to_copy);
        }
    }
    else
    {
        if(us_octets_dest == 2)
        {
            cbuffer_copy_packed_to_unpacked_ex(dst, src, octets_to_copy);
        }
        else
        {
            /* the unaligned version is MIPS heavier, so keep the aligned variant separate */
            if (r_octet_offset == w_octet_offset)
            {
                cbuffer_copy_aligned_32bit_be_zero_shift_ex(dst, src, octets_to_copy);
            }
            else
            {
                cbuffer_copy_unaligned_32bit_be_zero_shift_ex(dst, src, octets_to_copy);
            }
        }
    }

    /* TODO currently the asm implementation doesn't supply octets_copied */
    return octets_to_copy;
}

#endif /* !defined(PATCH_ENTRY_CBUFFER_COPY_EX) */

#endif /* INSTALL_CBUFFER_EX */

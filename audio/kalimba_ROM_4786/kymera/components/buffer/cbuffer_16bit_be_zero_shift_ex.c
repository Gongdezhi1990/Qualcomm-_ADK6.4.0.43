/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************//**
 * \file cbuffer_16bit_zero_shift_ex.c
 * \ingroup buffer
 *
 *  cbuffer_ex functions that only work with 16-bit big endian packing without
 *  any shift. Mainly used by the a2dp and the file endpoint to copy encoded data
 *  from one cbuffer to another.
 */

#ifdef INSTALL_CBUFFER_EX
#include "cbuffer_16bit_be_zero_shift_ex.h"
#include "util.h"
#include "pl_assert.h"
#include "buffer_private.h"
#include "panic/panic.h"
#include "patch/patch.h"

unsigned cbuffer_copy_16bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer *src, unsigned num_octets)
{
    unsigned r_octet_offset, w_octet_offset, src_data, dest_space, octets_to_copy;
    
    patch_fn_shared(cbuffer_copy_16bit_be_zero_shift);

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
    if (r_octet_offset == w_octet_offset)
    {
        cbuffer_copy_aligned_16bit_be_zero_shift_ex(dst, src, octets_to_copy);
    }
    else
    {
        patch_fn_shared(cbuffer_copy_16bit_be_zero_shift);
        panic(PANIC_AUDIO_CBUFFER_EX_COPY_UNEQUAL_OFFSET);
    }
    return octets_to_copy;
}
#endif /* INSTALL_CBUFFER_EX */

/****************************************************************************
 * Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file unpack_cbuff_to_array.c
 * \ingroup rtp_decode
 *
 * unpack_cbuff_to_array 'C' code.
 */

/****************************************************************************
Include Files
*/

#include "buffer.h"
#include "unpack_cbuff_to_array.h"
#include "rtp_decode_private.h"
#include "patch/patch.h"

extern unsigned cbuffer_get_usable_octets(tCbuffer *buff);
/****************************************************************************
Private Function Definitions
*/

/* unpack_cbuff_to_array */
unsigned int unpack_cbuff_to_array(int *dest, tCbuffer *cbuffer_src, unsigned int amount_to_copy)
{
    unsigned int available;
    unsigned int usable_octets;

    usable_octets = cbuffer_get_usable_octets(cbuffer_src);

    /* check amount */
    available = cbuffer_calc_amount_data_ex(cbuffer_src);
    if (available < amount_to_copy)
    {
        amount_to_copy = available;
    }

    /* nothing to copy */
    if (amount_to_copy == 0)
    {
        return amount_to_copy;
    }

    if(usable_octets == 2)
    {
        /* 16-bit unpacked buffers */
        unpack_cbuff_to_array_16bit(dest, cbuffer_src, amount_to_copy);
    }
    else
    {
        /* 32-bit packed buffers */
        unpack_cbuff_to_array_32bit(dest, cbuffer_src, amount_to_copy);
    }


    return amount_to_copy;
}

/* unpack_cbuff_to_array_from_offset */
unsigned int unpack_cbuff_to_array_from_offset(int *dest, tCbuffer *cbuffer_src,
        unsigned int offset, unsigned int amount_to_copy)
{
    unsigned int available;
    unsigned int usable_octets;

    usable_octets = cbuffer_get_usable_octets(cbuffer_src);

    /* check amount */
    available = cbuffer_calc_amount_data_ex(cbuffer_src);

    if (available <= offset)
    {
        return 0;
    }

    if (available < amount_to_copy + offset)
    {
        amount_to_copy = available - offset;
    }

    /* nothing to copy */
    if (amount_to_copy == 0)
    {
        return amount_to_copy;
    }

    if(usable_octets == 2)
    {
        /* 16-bit unpacked buffers */
        unpack_cbuff_to_array_from_offset_16bit(dest, cbuffer_src, amount_to_copy,
                                                offset);
    }
    else
    {
        /* 32-bit packed buffers */
        unpack_cbuff_to_array_from_offset_32bit(dest, cbuffer_src, amount_to_copy,
                                                offset);
    }
    return amount_to_copy;
}


/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file obuffer.c
 * \ingroup buffer
 *
 * obuffer 'C' code.
 */

/****************************************************************************
Include Files
*/

#include "buffer_private.h"
#include "buffer.h"
#include "octet_buffer.h"
#include "pmalloc/pl_malloc.h"
#include "panic/panic.h"
#include "platform/pl_trace.h"

/****************************************************************************
Private Function Definitions
*/
/**
 * Convert words to usable octets given the packing.
 */
static inline unsigned int get_size_in_octets(unsigned int size_in_words, PACKING packing)
{
    if (packing != USE_16BIT_PER_WORD)
    {
        /* Only 16 bit unpacked buffers are supported. */
        panic(PANIC_AUDIO_OCTET_BUFFER_UNSUPPORTED_PACKING);
    }
    return size_in_words<<1;

}

/**
 * Convert usable octets to words without rounding up.
 */
static inline unsigned int  get_size_in_words(unsigned int size_in_octets, PACKING packing)
{
    if (packing != USE_16BIT_PER_WORD)
    {
        /* Only 16 bit unpacked buffers are supported. */
        panic(PANIC_AUDIO_OCTET_BUFFER_UNSUPPORTED_PACKING);
    }
    return size_in_octets>>1;

}

/**
 * A simple module function optimised for the packing size.
 */
static inline unsigned int get_offset_modulo_useful_octets(unsigned int offset,
        PACKING packing)
{
    if (packing != USE_16BIT_PER_WORD)
    {
        /* Only 16 bit unpacked buffers are supported. */
        panic(PANIC_AUDIO_OCTET_BUFFER_UNSUPPORTED_PACKING);
    }
    /*Because the nr of useful octets is two a simple masking is faster.*/
    return offset & 0x1;
}


/****************************************************************************
Public Function Definitions
*/

/* obuffer_calc_amount_space_in_octets */
unsigned int obuffer_calc_amount_space_in_octets (octet_buffer *obuffer)
{
    return get_size_in_octets(cbuffer_calc_amount_space_in_words(obuffer->buffer),obuffer->packing)
            + obuffer->read_offset + obuffer->write_offset
            - (NR_OF_OCTETS_IN_WORD(obuffer->packing)-1);
}

/* obuffer_calc_amount_data_in_octets */
unsigned int obuffer_calc_amount_data_in_octets (octet_buffer *obuffer)
{
    return get_size_in_octets(cbuffer_calc_amount_data_in_words(obuffer->buffer),obuffer->packing)
            - obuffer->read_offset - obuffer->write_offset;
}

/* obuffer_get_size_in_octets */
unsigned int obuffer_get_size_in_octets(octet_buffer *obuffer)
{
    return cbuffer_get_size_in_words(obuffer->buffer)*NR_OF_OCTETS_IN_WORD(obuffer->packing);
}


/* obuffer_copy_octets */
unsigned int obuffer_copy_octets(octet_buffer *obuffer_dest, octet_buffer *obuffer_src, unsigned int amount_to_copy)
{
    unsigned int available;

    patch_fn_shared(octet_buffer);

    /* check amount */
    available = obuffer_calc_amount_data_in_octets(obuffer_src);
    if (available < amount_to_copy)
    {
        amount_to_copy = available;
    }

    available = obuffer_calc_amount_space_in_octets(obuffer_dest);
    if (available < amount_to_copy)
    {
        amount_to_copy = available;
    }

    /* nothing to copy */
    if (amount_to_copy == 0)
    {
        return amount_to_copy;
    }

    /* Only 16 bit unpacked buffers are supported. */
    if ((obuffer_src->packing == USE_16BIT_PER_WORD) &&
        (obuffer_dest->packing == USE_16BIT_PER_WORD))
    {
        /* Check the alignment */
        if (obuffer_dest->write_offset == obuffer_src->read_offset)
        {
            obuffer_copy_aligned_16bit(obuffer_dest->buffer, obuffer_src->buffer,
                    amount_to_copy, obuffer_dest->write_offset);

        }
        else
        {
            obuffer_copy_unaligned_16bit(obuffer_dest->buffer, obuffer_src->buffer,
                    amount_to_copy, obuffer_dest->write_offset);
        }

    }
    else
    {

        panic(PANIC_AUDIO_OCTET_BUFFER_UNSUPPORTED_PACKING);
    }

    /* The new read offset will be equal with the old offset plus the amount
     * written/read, modulo nr of useful octets in word.*/
    obuffer_dest->write_offset += amount_to_copy;
    obuffer_dest->write_offset = get_offset_modulo_useful_octets(obuffer_dest->write_offset,
                                                                 obuffer_dest->packing);

    obuffer_src->read_offset += amount_to_copy;
    obuffer_src->read_offset = get_offset_modulo_useful_octets(obuffer_src->read_offset,
                                                               obuffer_src->packing);

    return amount_to_copy;
}

/*obuffer_unpack */
unsigned int obuffer_unpack(int *dest, octet_buffer *obuffer_src, unsigned int amount_to_copy)
{
    unsigned int available;

    patch_fn_shared(octet_buffer);

    /* check amount */
    available = obuffer_calc_amount_data_in_octets(obuffer_src);
    if (available < amount_to_copy)
    {
        amount_to_copy = available;
    }

    /* nothing to copy */
    if (amount_to_copy == 0)
    {
        return amount_to_copy;
    }

    /* Only 16 packed buffers are supported. */
    if (obuffer_src->packing == USE_16BIT_PER_WORD)
    {
        obuffer_unpack_16bit(dest, obuffer_src->buffer,
                amount_to_copy, obuffer_src->read_offset);
    }
    else
    {
        panic(PANIC_AUDIO_OCTET_BUFFER_UNSUPPORTED_PACKING);
    }

    /* The new read offset will be equal with the old offset plus the amount
     * written/read, modulo nr of useful octets in word.*/
    obuffer_src->read_offset += amount_to_copy;
    obuffer_src->read_offset = get_offset_modulo_useful_octets(obuffer_src->read_offset,
                                                               obuffer_src->packing);

    return amount_to_copy;
}

/*obuffer_unpack_from_offset */
unsigned int obuffer_unpack_from_offset(int *dest, octet_buffer *obuffer_src,
        unsigned int offset, unsigned int amount_to_copy)
{
    unsigned int available;
    unsigned advance_cbuffer;
    unsigned read_offset;

    patch_fn_shared(octet_buffer);

    /* check amount */
    available = obuffer_calc_amount_data_in_octets(obuffer_src);

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

    advance_cbuffer = get_size_in_words(offset + obuffer_src->read_offset, USE_16BIT_PER_WORD);
    read_offset = obuffer_src->read_offset + offset;
    read_offset = get_offset_modulo_useful_octets(read_offset, obuffer_src->packing);


    /* Only 16 packed buffers are supported. */
    if (obuffer_src->packing == USE_16BIT_PER_WORD)
    {
        obuffer_unpack_from_offset_16bit(dest, obuffer_src->buffer,
                amount_to_copy, read_offset, advance_cbuffer);
    }
    else
    {
        panic(PANIC_AUDIO_OCTET_BUFFER_UNSUPPORTED_PACKING);
    }

    return amount_to_copy;
}

/* obuffer_discard_data */
void obuffer_discard_data(octet_buffer *obuffer, unsigned int discard_amount)
{
    patch_fn_shared(octet_buffer);

    /* Add the already read octet to the amount and convert it to words.
     * The available data is not check against the discard amount because
     * otherwise cbuffer_discard_data does not give a fault. */
    cbuffer_discard_data(obuffer->buffer,
            get_size_in_words(discard_amount + obuffer->read_offset, USE_16BIT_PER_WORD));

    /* The new read offset will be equal with the old offset plus the amount
     * written/read, modulo nr of useful octets in word. Because the nr of useful
     * octets is two a simple masking is faster. */
    obuffer->read_offset += discard_amount;
    obuffer->read_offset = get_offset_modulo_useful_octets(obuffer->read_offset,
                                                               obuffer->packing);
}

/* obuffer_get_read_index */
unsigned int obuffer_get_read_index(octet_buffer *obuffer)
{
    unsigned int read_index;

    /* TODO: Find a better way to get the read index from a Cbuffer. */
    read_index = (uintptr_t)obuffer->buffer->read_ptr - (uintptr_t)obuffer->buffer->base_addr;

    /*Convert the adresses to words. */
    read_index = read_index >> LOG2_ADDR_PER_WORD;

    /* Convert the words to usable octets. */
    read_index = get_size_in_octets(read_index, USE_16BIT_PER_WORD);

    /* Add the read offset. */
    read_index += obuffer->read_offset;

    return read_index;
}

void obuffer_advance_write_ptr(octet_buffer *obuffer, unsigned int amount)
{
    /* Use the unwritten octets from the previous word.*/
    amount -= obuffer->write_offset;
    /* Calculate the next write offset. */
    obuffer->write_offset = get_offset_modulo_useful_octets(amount, obuffer->packing);
    if (obuffer->write_offset)
    {
        cbuffer_advance_write_ptr( obuffer->buffer, (amount /NR_OF_OCTETS_IN_WORD(obuffer->packing)) + 1);
    }
    else
    {
        cbuffer_advance_write_ptr( obuffer->buffer, (amount /NR_OF_OCTETS_IN_WORD(obuffer->packing)));
    }

}


/* octet_buffer_create */
octet_buffer *octet_buffer_create(PACKING packing, ENDIANNESS endianness)
{
    octet_buffer *obuffer = xzpnew(octet_buffer);

    if (obuffer)
    {
        obuffer->packing = packing;
        obuffer->endianness = endianness;
    }
    return obuffer;
}

/* octet_buffer_destroy */
void octet_buffer_destroy(octet_buffer *buffer)
{
    pdelete(buffer);
}



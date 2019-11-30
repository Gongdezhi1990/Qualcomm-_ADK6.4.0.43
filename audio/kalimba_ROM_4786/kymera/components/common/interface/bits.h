/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file bits.h
 *
 * Generic bit mask and field utilities.
 */

#ifndef BITS_H
#define BITS_H

/** Sets mask-ed bits of word to 1. Other bits unchanged. */
/* TODO_CRESCENDO  This is only used in sub_host_wake.c to access uin16s. But is generic
 * Needs some thought to see if its_set is for uint16, uint24 uint32 etc?
 */
#define bits_set(word_ptr, mask) ((void) (*(word_ptr) = (uint16)(*(word_ptr) | (mask))))

/** Sets mask-ed bits of word to 0. Other bits unchanged. */
#define bits_clear(word_ptr, mask) ((void) (*(word_ptr) = (uint16)(*(word_ptr) & ~(mask))))

/** Flips mask-ed bits of word. Other bits unchanged. */
#define bits_flip(word_ptr, mask) ((void)(*(word_ptr) ^= (mask)))

/** True if any of the mask-ed bits are set. */
#define bits_test(word_ptr, mask) ((*(word_ptr) & (mask)) != 0)

/*
 * The following macros evaluate to, or depend on, a word of specific
 * size. To avoid replication of logic for each size [SPOT] each macro has
 * a common implementation (see bitstt_*()) templated over a uint type.
 *
 * Specialisations are available for 16 and 32 bit words
 * (see bits16_*() & bits32_*()). These should be used in code.
 */

/**
 * Evaluate to word with (only) the specified bit set.
 *
 * Examples
 *
 * bits16_bit_mask(0) == 0000000000000001 b
 * bits16_bit_mask(4) == 0000000000010000 b
 *
 * \pre which_bit < bits in word!
 *
 */
#define bitstt_bit_mask(which_type, which_bit) ((which_type)1 << (which_bit))
#define bits16_bit_mask(which_bit) bitstt_bit_mask(uint16, (which_bit))
#define bits32_bit_mask(which_bit) bitstt_bit_mask(uint32, (which_bit))

/**
 * Alias for bits16_bit_mask().
 */
#define BIT16(which_bit) bits16_bit_mask(which_bit)

/**
 * Alias for bits16_bit_mask().
 */
#define BIT(which_bit) bits16_bit_mask(which_bit)

/**
 * Alias for bits32_bit_mask().
 */
#define BIT32(which_bit) bits32_bit_mask(which_bit)

/**
 * Evaluate to word with the n LS bits set.
 *
 * bits16_lsb_mask(2) == 0000000000000011 b
 * bits16_lsb_mask(5) == 0000000000011111 b
 *
 * \use Use to generate nbit sub-field masks.
 *
 * \pre nbits < bits in word!
 */
#define bitstt_lsb_mask(which_type, nbits) \
    (bitstt_bit_mask(which_type,(nbits)) - 1)
#define bits16_lsb_mask(nbits) bitstt_lsb_mask(uint16, (nbits))
#define bits32_lsb_mask(nbits) bitstt_lsb_mask(uint32, (nbits))

/**
 * Evaluate to word with bit field_size contiguous bits set,
 * starting at bit field_origin bit.
 *
 * Examples
 *
 * bits16_field_mask(1, 3) == 0000000000001110 b
 * bits16_field_mask(8, 4) == 0000111100000000 b
 *
 * \pre field_bits < bits in word!
 */
#define bitstt_field_mask(which_type, field_origin, field_size) \
    (bitstt_lsb_mask(which_type, (field_size))  << (field_origin))
#define bits16_field_mask(origin, size) bitstt_field_mask(uint16, origin, size)
#define bits32_field_mask(origin, size) bitstt_field_mask(uint32, origin, size)

/**
 * Can a positive integer i be represented with the given number of bits?
 *
 * \use Validation of values destined for bit sub-fields.
 *
 * \pre nbits < bits in word!
 */
#define bitstt_can_represent(which_type, nbits, i)  \
    (bitstt_lsb_mask(which_type, (nbits)) >= (which_type)(i))
#define bits16_can_represent(nbits, i)  bitstt_can_represent(uint16,(nbits),(i))
#define bits32_can_represent(nbits, i)  bitstt_can_represent(uint32,(nbits),(i))

#endif  /* ifndef BITS_H */

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implements an extremely simple interface for bitwise operations.
 */

#ifndef UTILS_BIT_H_
#define UTILS_BIT_H_

#ifdef DESKTOP_TEST_BUILD
#include "hydra/hydra_types.h"
#else
#include "kal_utils/kal_utils.h"
#endif

/**
 * Clears the bits indicated by the mask.
 */
#define BIT_CLEAR_MASK(a, mask) ((a) & (~(mask)))

/**
 * Sets the bits indicated by the mask.
 */
#define BIT_SET_MASK(a, mask) ((a) | (mask))

/**
 * Writes the given masked value leaving the unmasked bits untouched.
 */
#define BIT_WRITE_MASK(a, mask, val) \
    BIT_SET_MASK(BIT_CLEAR_MASK(a, mask), (mask) & (val))

/**
 * Clears the bit on the indicated position.
 */
#define BIT_CLEAR_POS(a, pos) \
    BIT_CLEAR_MASK(a, 1 << pos)

/**
 * Sets the bit on the indicated position.
 */
#define BIT_SET_POS(a, pos) \
    BIT_SET_MASK(a, 1 << pos)

/**
 * Writes the bit on the indicated position with the indicated value.
 */
#define BIT_WRITE_POS(a, pos, val) \
    BIT_WRITE_MASK(a, 1 << pos, val)

/**
 * Gets the bit on the indicated position.
 */
#define BIT_GET_POS(a, pos) \
    (((a) >> (pos)) & 0x01)

/**
 * Gets the position of the top most set bit.
 */
#ifdef DESKTOP_TEST_BUILD
uint8 bit_get_top_set_pos(uint32 bitset);
#define BIT_GET_TOP_SET_POS(bitset) bit_get_top_set_pos(bitset)
#else
#define BIT_GET_TOP_SET_POS(bitset) MAX_BIT_POS(bitset)
#endif

/**
 * Returns 1 if only one bit is set in a bitmask, 0 otherwise.
 */
#define BIT_IS_ONLY_ONE_BIT_SET(a) ((a) && !((a) & ((a)-1)))

#endif /* UTILS_BIT_H_ */

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Includes headers that contain defines for bit and bit array management.
 * Implements an extremely simple interface for bitwise operations.
 */

#ifndef UTILS_BITS_AND_BOBS_H_
#define UTILS_BITS_AND_BOBS_H_

#include "utils/utils_bit.h"
#include "utils/utils_bitarray.h"
#include "hydra/hydra_types.h"

uint16 bitmap_to_array(uint16* barray,
                       uint16 barray_len,
                       uint16 offset,
                       uint16f bitmap);

/**
 * @brief Round up the given value to the nearest multiple of 2 to the power
 * of the given exponent.
 *
 * If the given value is already a multiple it is returned as it is.
 *
 * This function is useful for alignment calculations. e.g. Round the given
 * value to a 32-bit word boundary (4-bytes):
 * <code>
   addr_aligned = UTILS_ROUND_UP_LOG2(addr, 2); // 2^2 = 4 bytes = 32 bits
 * </code>
 *
 * @param [in] _value       The value to round up.
 * @param [in] _align_log2  log2 of the multiple to align @p _value up to.
 * @return @p _value rounded up to the nearest multiple of 2^ @p _align_log2.
 */
#define UTILS_ROUND_UP_LOG2(_value, _align_log2) \
        (((_value) + ((1 << (_align_log2)) - 1)) & ~((1 << (_align_log2)) - 1))

#endif /*UTILS_BITS_AND_BOBS_H_*/

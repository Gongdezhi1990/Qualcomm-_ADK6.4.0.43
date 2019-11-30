/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Advertise the services of the optim library
 */
#ifndef OPTIM_H
#define OPTIM_H

#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"

/**
 * Copy memory turning uint16 into 2 * uint8
 *
 * Like memcpy put the source is a uint16 array and the destination
 * is a uint8 array. The first word of the uint16 array goes into
 * the first 2 words of the uint8 array (LSB first). This function
 * does not guarantee to clear the upper 8 bits of the uint8 array.
 *
 * The length given is the length of the uint16 array.
 *
 * Returns
 *
 * Unlike memcpy this function returns void.
 */
/*extern void memcpy_unpack(uint8 *dest, const uint16 *source, uint16 length);*/
#define memcpy_unpack(dest, source, length) memcpy(dest, source, 2 * (length))

/**
 * Copy memory turning uint16 into 2 * uint8
 *
 * Identical to memcpy_unpack but clears the upper 8 bits of the uint8 array.
 *
 * The \p length given is the length of the uint16 array.
 */
#define memcpy_unpack_clean(dest, source, length) memcpy(dest, source, 2 * (length))

/**
 * Copy memory turning 2 * uint8 into uint16
 *
 * Like memcpy but the source is a uint8 array and the destination
 * is a uint16 array. The first word of the uint16 array is built
 * from the  first 2 words of the uint8 array (LSB first).
 *
 * The length given is the length of the uint16 array.
 *
 * Returns
 *
 * Unlike memcpy this function returns void.
 */
#define memcpy_pack(dest, source, length) memcpy(dest, source, 2 * (length))

/**
 * Copy memory turning 2 * uint8 into uint16 in little endian format
 *
 * Like memcpy but the source is a uint8 array and the destination
 * is a uint16 array. The first word of the uint16 array is built
 * from the first 2 bytes of the uint8 array in little endian format.
 *
 * \param dest destination array pointer.
 * \param source Source pointer.
 * \param len_bytes Length of the source data.
 * \return None.
 */
extern void mempack_le(uint16 *dest, const uint8 *source, uint32 len_bytes);

/**
 * Divide a 32-bit number by a 16-bit number returning 16 bit's
 *
 * Divide a 32-bit number by a 16-bit number returning a 16-bit
 * result.  This function does no checking for overflow so could be
 * described as dangerous.  (In many cases the result from a udiv32
 * is cast directly to a uint16, which is just as bad).
 *
 * Returns
 *     a / b
 */
extern int16 divx2(int32 a, int16 b);

#ifndef XAP
#define divx2(a, b) ((int16)((int32)(a) / (b)))
#endif

/**
 * Do a 32 by 16 bit division return quotient and remainder
 * \returns Return value is value (n/d). If r is not NULL on entry then the
 * remainder is written into r.
 */
extern uint32 udiv3216(uint16 *r, uint16 d, uint32 n);

#endif /* OPTIM_H */

/****************************************************************************
 * Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup rtp_decode
 * \file  unpack_cbuff_to_array.h
 * \ingroup capabilities
 *
 * Rtp decode operator header of helper functions. <br>
 *
 */

#ifndef RTP_DECODE_UNPACK_CBUFF_TO_ARRAY_H_
#define RTP_DECODE_UNPACK_CBUFF_TO_ARRAY_H_

/**
 * Reads data from a buffer, unpacks it and puts to the destination memory address.
 *
 * NOTE: This function advances the read pointer of the buffer.
 *
 * \param dest              destination of data.
 * \param cbuffer_src       structure to read data from.
 * \param amount_to_read    octets to read.
 * \return how much data we actually read (can be different from amount_to_read).
 */
extern unsigned int unpack_cbuff_to_array(int *dest, tCbuffer *cbuffer_src,
                                          unsigned int amount_to_copy);

/**
 * Reads data from a buffer starting at a given offset, unpacks it and puts it to the
 * destination memory address.
 *
 * NOTE: This function does NOT advance the read pointer of the buffer.
 *
 * \param dest          destination of data.
 * \param cbuffer_src   buffer to read data from.
 * \param offset .
 * \param amount_to_read octets to read.
 * \return how much data we actually read (can be different from amount_to_read).
 */
extern unsigned int unpack_cbuff_to_array_from_offset(int *dest, tCbuffer *cbuffer_src,
                                                      unsigned int offset,
                                                      unsigned int amount_to_copy);

#endif /* RTP_DECODE_UNPACK_CBUFF_TO_ARRAY_H_ */

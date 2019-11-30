/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer Buffer Subsystem
 * \file  cbuffer_16bit_be_zero_shift_ex.h
 *
 * \ingroup buffer
 *
 * Functions that can do octet based cbuffer operations for 16 bit unpacked,
 * big endian buffers without any shift configured.
 *
 */

#ifdef INSTALL_CBUFFER_EX
#ifndef CBUFFER_EX_16BIT_BE_ZERO_SHIFT_H
#define CBUFFER_EX_16BIT_BE_ZERO_SHIFT_H

#include "cbuffer_c.h"
/*
 * \brief copy octets from one cbuffer to another. The cbuffers must be unpacked, 16-bit
 *  big endian without any shift configured. The source read octet offset (within a word)
 *  and the destination write octet offset (within a word) must be aligned (equal).
 * \param dst destination cbuffer pointer
 * \param src source cbuffer pointer
 * \param num_octets number of octets to copy
 * \return number of octets copied.
 */
unsigned cbuffer_copy_aligned_16bit_be_zero_shift_ex(tCbuffer * dst, tCbuffer * src, unsigned num_octets);

#endif /* CBUFFER_EX_16BIT_BE_ZERO_SHIFT_H */
#endif /* INSTALL_CBUFFER_EX */

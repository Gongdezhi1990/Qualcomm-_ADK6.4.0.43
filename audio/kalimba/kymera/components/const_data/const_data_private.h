/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file const_data_private.h
 * \ingroup const_data
 *
 * Header file for some of the internals of the const data module
 */

#ifndef CONSTANT_DATA_PRIVATE_H
#define CONSTANT_DATA_PRIVATE_H

#include "const_data.h"
#include "pmalloc/pl_malloc.h"
#include "hal.h"
#include "string.h"
#include "panic/panic.h"

#ifdef CHIP_BASE_A7DA_KAS
/** \brief Access constant data.
 * A pointer to a block of constant data described by 'source' will be returned.
 * \param source - address of packed data
 * \param size - the size of the data to be unpacked
 * \param dest - address to unpack data into
 * 
 \return last word read upacked
 */
extern void mem_unpack16(unsigned *src,unsigned size,unsigned *dest);
#else
extern unsigned mem_ext_window_unpack_to_ram(char *src,unsigned size,unsigned *dest);
extern unsigned mem_ext_window_copy_to_ram(char *src,unsigned size,unsigned *dest);
#endif

#endif /* CONSTANT_DATA_PRIVATE_H */


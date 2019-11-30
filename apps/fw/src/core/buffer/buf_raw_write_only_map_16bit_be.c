/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

#if CHIP_HAS_MMU_BIG_ENDIAN_MAPPING  /* This is not supported by the hardware */
uint16 *
buf_raw_write_only_map_16bit_be(BUFFER *buf)
{
    mmu_write_port_open(buf->handle);
    return mmu_write_port_map_16bit_be(buf->handle, buf->index);
}
#endif

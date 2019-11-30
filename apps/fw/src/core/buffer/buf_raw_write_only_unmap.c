/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    buf_raw_write_only_unmap.c - Contains the corresponding function.

CONTAINS
    buf_raw_write_only_unmap - unmap the buffer only from writing.
*/

#include "buffer/buffer_private.h"

/**
 * Unmap the the buffer only from writing.
 */
void buf_raw_write_only_unmap(void)
{
    mmu_write_port_close();
}

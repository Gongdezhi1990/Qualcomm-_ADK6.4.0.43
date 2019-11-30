/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    buf_raw_read_write_unmap.c - Contains the corresponding function.

CONTAINS
    buf_raw_read_write_unmap - unmap the buffer from reading and writing.
*/

#include "buffer/buffer_private.h"

/**
 * Unmap the from from reading and writing.
 */
void buf_raw_read_write_unmap(void)
{
    mmu_read_port_close();
    mmu_write_port_close();
}

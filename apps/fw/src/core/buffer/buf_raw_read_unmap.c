/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    buf_raw_read_unmap.c - Contains the corresponding function.

CONTAINS
    buf_raw_read_unmap - unmap the buffer from reading.
*/

#include "buffer/buffer_private.h"

/**
 * Unmap the from ONLY reading.
 */
void buf_raw_read_unmap(void)
{
    mmu_read_port_close();
}


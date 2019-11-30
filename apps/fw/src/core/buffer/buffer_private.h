/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Application Subsystem buffer subsystem main private header file
*/

#ifndef BUFFER_PRIVATE_H
#define BUFFER_PRIVATE_H

#include "buffer/buffer.h"
#include "panic/panic.h"
#include "pmalloc/pmalloc.h"
#include "mmu/mmu.h"
#include "io/io.h"
#include "ipc/ipc.h"

/**
 * Destroy a newly created buffer.
 *
 * \param buf is the buffer to destroy.
 *
 * \param do_free indicates whether the buffer structure itself needs to
 * be freed.
 *
 * This is private to the buffer module.
 */
extern void buf_destroy_buffer_on_creation(BUFFER *buf, bool do_free);

#endif /* BUFFER_PRIVATE_H */

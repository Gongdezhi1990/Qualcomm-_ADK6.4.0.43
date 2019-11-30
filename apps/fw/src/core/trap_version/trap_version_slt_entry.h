/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/*
 * \file
 *
 *
 * This file contains the magic to make a few entries appear in the
 * static SLT
 */

#ifndef VM_SLT_ENTRY_H
#define VM_SLT_ENTRY_H

#include "trap_version/trap_version.h"

#define CORE_TRAP_VERSION_SLT_ENTRY(m)                          \
    SLT_ENTRY(m, (APPS_SLT_TRAP_VERSION, trap_version))         \
    SLT_ENTRY(m, (APPS_SLT_TRAPSET_BITMAP, trapset_bitmap))     \
    SLT_ENTRY(m, (APPS_SLT_TRAPSET_BITMAP_LENGTH, &trapset_bitmap_length))

#endif /* TRAP_API_SLT_ENTRY_H */

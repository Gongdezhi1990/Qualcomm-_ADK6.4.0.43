/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * SLT entries that give the address of registers.
 */

#ifndef IO_SLT_ENTRY_H
#define IO_SLT_ENTRY_H

#define IO_DEFS_MODULE_APPS_SYS_SYS /* Work around compiler bugs */
#include "io/io_map.h"

/*
 * Bizarre cast is to avoid compiler complaining that the pointer
 * is to a volatile --- turning it into a straight uint32 first
 * fixes this.
 */
#define CORE_IO_SLT_ENTRY(m) \
    SLT_ENTRY(m, (APPS_SLT_ID_SUB_SYS_CHIP_VERSION,\
                  ((uint32)&SUB_SYS_CHIP_VERSION)))

#endif /* IO_SLT_ENTRY_H */

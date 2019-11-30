/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  reserved_ram.c
 * \ingroup mem_utils
 *
 * Allocation of a small block of static RAM for special customer use
 */

#include "reserved_ram.h"
#include "hal_dm_sections.h"

#ifdef RESERVED_STATIC_BLOCK_SIZE
DM_SHARED_ZI unsigned reserved_static_ram_block[RESERVED_STATIC_BLOCK_SIZE];
#endif

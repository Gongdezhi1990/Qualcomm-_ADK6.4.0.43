/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * The hidden static SLT table
 *
 */

#include "slt/slt_private.h"

#define SLT_ENTRY(m,n) \
    SLT_ENTRY_EXPAND_##m n

#define SLT_ENTRY_EXPAND_TABLE(i, s) { i, (const void *)s },

/**
 * The SLT table that will end up in const space
 */
const static_slt_entry slt_table_start[] GCC_ATTRIBUTE(section ("SLT_DATA")) =
{
    /* xapgcc ignore cast discards qualifiers from pointer target type */
    SLT_ENTRY_LIST(TABLE)
    { APPS_SLT_ID_NONE, NULL }
};

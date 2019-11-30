/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/*
 * \file
 *
 * Read the combined effect of all the flags in the kip table
 */

#include "dorm/dorm_private.h"

/** Get the overall kip_flags
 * \return the inclusive or of all the components' settings
 */
uint16 dorm_get_combined_kip_flags(void)
{
    unsigned int kip_flags, i; /* Avoid compiler warnings */

    kip_flags = 0;
    for (i = 0; i < DORMID_COUNT; i++)
    {
        kip_flags |= kip_table[i];
    }
    return (uint16)kip_flags;
}

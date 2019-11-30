/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include "region.h"

#include <string.h>

const static uint8 uuid_suffix[12] = 
{ 
    0x00, 0x00, 0x10, 0x00, 
    0x80, 0x00, 0x00, 0x80, 
    0x5F, 0x9B, 0x34, 0xFB
};

bool RegionMatchesUUID32(const Region *r, uint32 uuid)
{
    /* Valid sizes are 2, 4, and 16 so this is good enough */
    return RegionReadUnsigned(r) == uuid && (RegionSize(r) != 16 || memcmp(uuid_suffix, r->begin, 12) == 0);
}

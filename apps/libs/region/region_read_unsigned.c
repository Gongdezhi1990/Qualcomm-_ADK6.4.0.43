/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include "region.h"

uint32 RegionReadUnsigned(const Region *r)
{
    uint32 v = 0;
    const uint8 *p;
    for(p = r->begin; p != r->end; ++p)
        v = (v<<8) | *p;
    return v;
}

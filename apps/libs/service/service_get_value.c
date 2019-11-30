/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include "service_private.h"

bool ServiceGetValue(Region *r, ServiceDataType *type, Region *out)
{
    uint8 b;
    uint16 size;

    if(RegionSize(r) < 1) return 0;

    b = *r->begin++;
    *type = (ServiceDataType) (b>>3);

    switch(b & 7)
    {
        default: case 0:
            size = *type == sdtNil ? 0 : 1;
        break;
        case 1:
            size = 2;
        break;
        case 2:
            size = 4;
        break;
        case 3:
            size = 8;
        break;
        case 4:
            size = 16;
        break;
        case 5:
            if(RegionSize(r) < 1) return 0;
            size = *r->begin;
            r->begin += 1;
        break;
        case 6:
            if(RegionSize(r) < 2) return 0;
            size = serviceUnpack16(r->begin);
            r->begin += 2;
        break;
        case 7:
            if(RegionSize(r) < 4) return 0;
            size = (uint16) serviceUnpack32(r->begin);
            r->begin += 4;
        break;
    }
    if(RegionSize(r) < size) return 0;
    out->begin = r->begin;
    r->begin  += size;
    out->end   = r->begin;
    return 1;
}

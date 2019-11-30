/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#ifndef SERVICE_PRIVATE_H_
#define SERVICE_PRIVATE_H_

#include "service.h"

static uint16 __inline__ serviceUnpack16(const uint8 *s)
{ return (((uint16)s[0])<<8) | (uint16)s[1]; }

static uint32 __inline__ serviceUnpack32(const uint8 *s)
{
    uint32 r = s[0];
    r <<= 8; r |= s[1];
    r <<= 8; r |= s[2];
    r <<= 8; r |= s[3];
    return r;
}

#endif /* SERVICE_PRIVATE_H_ */

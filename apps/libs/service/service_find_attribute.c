/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include "service.h"

bool ServiceFindAttribute(Region *r, uint16 id, ServiceDataType *type, Region *out)
{
    uint16 found;
    while(ServiceNextAttribute(r, &found, type, out))
        if(found == id)
            return 1;
    return 0;
}


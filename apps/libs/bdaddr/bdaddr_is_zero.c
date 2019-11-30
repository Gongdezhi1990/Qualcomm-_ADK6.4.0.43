/* Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <bdaddr.h>

bool BdaddrIsZero(const bdaddr *in)
{ 
    return !in->nap && !in->uap && !in->lap; 
}

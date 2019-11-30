/* Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <bdaddr.h>

void BdaddrConvertBluestackToVm(bdaddr *out, const BD_ADDR_T *in)
{
    out->lap = (uint32)(in->lap);
    out->uap = (uint8)(in->uap);
    out->nap = (uint16)(in->nap);
}

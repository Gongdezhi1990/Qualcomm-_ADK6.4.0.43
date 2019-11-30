/* Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <bdaddr.h>

void BdaddrConvertVmToBluestack(BD_ADDR_T *out, const bdaddr *in)
{

    out->lap = (uint24_t)(in->lap);
    out->uap = (uint8_t)(in->uap);
    out->nap = (uint16_t)(in->nap);
}

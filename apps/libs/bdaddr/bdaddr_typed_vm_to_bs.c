/* Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <bdaddr.h>

void BdaddrConvertTypedVmToBluestack(
        TYPED_BD_ADDR_T         *out,
        const typed_bdaddr      *in
        )
{
    switch(in->type)
    {
        case TYPED_BDADDR_PUBLIC:
            out->type = TBDADDR_PUBLIC;
            break;
        case TYPED_BDADDR_RANDOM:
            out->type = TBDADDR_RANDOM;
            break;
        default:
            out->type = TBDADDR_INVALID;
            break;
    }
    BdaddrConvertVmToBluestack(&out->addr, &in->addr);
}

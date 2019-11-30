/* Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <bdaddr.h>

void BdaddrConvertTypedBluestackToVm(
        typed_bdaddr            *out, 
        const TYPED_BD_ADDR_T   *in
        )
{
    switch (in->type)
    {
        case TBDADDR_PUBLIC:
            out->type = TYPED_BDADDR_PUBLIC;
            break;
        case TBDADDR_RANDOM:
            out->type = TYPED_BDADDR_RANDOM;
            break;
        default:
            out->type = TYPED_BDADDR_INVALID;
            break;
    }
    BdaddrConvertBluestackToVm(&out->addr, &in->addr);
}

/* Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <bdaddr.h>

bool BdaddrTpIsEmpty(const tp_bdaddr *in)
{
    return  in->transport == TRANSPORT_NONE &&
            BdaddrTypedIsEmpty(&in->taddr);
}

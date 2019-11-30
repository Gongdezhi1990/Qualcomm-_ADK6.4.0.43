/* Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <bdaddr.h>

void BdaddrTpSetEmpty(tp_bdaddr *in)
{
    in->transport = TRANSPORT_NONE;
    BdaddrTypedSetEmpty(&in->taddr);
}


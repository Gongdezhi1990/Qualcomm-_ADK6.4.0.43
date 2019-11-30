/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    afh_channel_map.c

DESCRIPTION
      A utility library to handle hardware specific dependency of AFH channel map update.
*/

#include <broadcast_context.h>
#include <erasure_coding.h>
#include "afh_channel_map.h"

/*!
    @brief Updates the Hydracore Packetiser that AFH channel map is pending
*/

/******************************************************************************/
void afhChannelMapChangeIsPending(void)
{
    ec_handle_tx_t handle = BroadcastContextGetEcTxHandle();
    if(handle)
    {
        ErasureCodingTxAFHChannelMapChangeIsPending(handle);
    }
}



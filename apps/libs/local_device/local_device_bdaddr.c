/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device_bdaddr.c

DESCRIPTION
    Store Bluetooth Address
*/

#include "local_device_bdaddr.h"

static bdaddr bd_addr;

/******************************************************************************/
void localDeviceBdAddrSetRequest(bdaddr addr)
{
    bd_addr = addr;
}

/******************************************************************************/
void localDeviceBdAddrGetRequest(bdaddr* addr)
{
    *addr = bd_addr;
} 

/******************************************************************************/
void localDeviceBdAddrReset(void)
{
    bdaddr addr;
    BdaddrSetZero(&addr);
    localDeviceBdAddrSetRequest(addr);
}

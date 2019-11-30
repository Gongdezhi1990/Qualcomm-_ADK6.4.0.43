/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device_bdaddr.h

DESCRIPTION
    Store Bluetooth Address
*/

#ifndef LOCAL_DEVICE_BDADDR_
#define LOCAL_DEVICE_BDADDR_

#include <bdaddr.h>
#include <csrtypes.h>

/******************************************************************************/
void localDeviceBdAddrSetRequest(bdaddr addr);

/******************************************************************************/
void localDeviceBdAddrGetRequest(bdaddr* addr);

/******************************************************************************/
void localDeviceBdAddrReset(void);

#endif /* LOCAL_DEVICE_BDADDR_ */

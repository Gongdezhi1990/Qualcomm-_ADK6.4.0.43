/*****************************************************************
Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
FILE NAME
    sink_iap2_transport.h
  
DESCRIPTION
   Header for transport layer for connecting to a Bluetooth or USB transport
*/

#ifndef __SINK_IAP2_TRANSPORT_H__
#define __SINK_IAP2_TRANSPORT_H__


#include <iap2.h>
#include <connection.h>
#include <stdio.h>
#include <message.h>
#include <source.h>
#include <region.h>
#include <service.h>
#include <string.h>
#include <stdlib.h>

#include "sink_accessory.h"

void sinkIap2ConnectRequest(bdaddr * pAddr);
void sinkIap2DisconnectRequest(iap2_link *link);
void sinkIap2ConnectResponse(iap2_link *link, bool accept);


#endif /* __SINK_IAP2_TRANSPORT_BT_H__ */

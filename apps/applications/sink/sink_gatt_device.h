/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_device.h

DESCRIPTION
    General GATT specific device functionality.
    
NOTES

*/

#ifndef _SINK_GATT_DEVICE_H_
#define _SINK_GATT_DEVICE_H_


#include "sink_gatt_client.h"
#include "sink_gatt_server.h"

#include <csrtypes.h>


#ifdef GATT_ENABLED

#define GATT gBleData->ble.gatt

typedef struct __gatt_data_t
{
    gatt_client_t   	  client;             /* Client data */
    gatt_server_t   	  server;          /* Server data */
    uint16                cid;                        /*connection identifier */
    bool                   is_bonding;
} gatt_data_t;

#endif /* GATT_ENABLED */


#endif /* _SINK_GATT_DEVICE_H_ */


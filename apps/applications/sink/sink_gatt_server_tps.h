/*******************************************************************************
Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_tps.h

DESCRIPTION
    Routines to handle messages sent from the GATT Transmit Power Server Task.
    
NOTES

*/


#ifndef _SINK_GATT_SERVER_TPS_H_
#define _SINK_GATT_SERVER_TPS_H_


#ifndef GATT_ENABLED
#undef GATT_TPS_SERVER
#endif


/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* Library headers */
#include <gatt_transmit_power_server.h>

#ifdef GATT_TPS_SERVER
#define sinkGattTxPowerServerGetSize() sizeof(GTPSS)
#else
#define sinkGattTxPowerServerGetSize() 0
#endif

/*******************************************************************************
NAME
    sinkGattTxPowerServerInitialiseTask
    
DESCRIPTION
    Initialise the Transmit Power server task.

PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.

RETURNS
    TRUE if the Transmit Power server task was initialised, FALSE otherwise.
*/
#ifdef GATT_TPS_SERVER
bool sinkGattTxPowerServerInitialiseTask(uint16 **ptr);
#else
#define sinkGattTxPowerServerInitialiseTask(ptr) (TRUE)
#endif


/*******************************************************************************
NAME
    sinkGattTxPowerServiceEnabled
    
DESCRIPTION
    Returns whether TxPower service is enabled or not.
*/
bool sinkGattTxPowerServiceEnabled(void);

#endif /* _SINK_GATT_SERVER_TPS_H_ */

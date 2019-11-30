/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_tps.c

DESCRIPTION
    Routines to handle messages sent from the GATT Transmit Power Server Task.
*/


/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* Application headers */
#include "sink_gatt_db.h"
#include "sink_gatt_server_tps.h"
#include "sink_ble.h"

#include "sink_debug.h"
#include "sink_gatt_server.h"

#ifdef GATT_TPS_SERVER


#ifdef DEBUG_GATT
#define GATT_DEBUG(x) DEBUG(x)
#else
#define GATT_DEBUG(x) 
#endif

/*******************************************************************************/
bool sinkGattTxPowerServerInitialiseTask(uint16 **ptr)
{
    if (GattTransmitPowerServerInitTask(sinkGetBleTask(), (GTPSS *)*ptr,
                                            HANDLE_TRANSMIT_POWER_SERVER_SERVICE,
                                            HANDLE_TRANSMIT_POWER_SERVER_SERVICE_END))
    {
        GATT_DEBUG(("GATT Tx Power Server initialised\n"));
        /* The size of TPS is also calculated and memory is alocated.
         * So advance the ptr so that the next Server while initializing.
         * shall not over write the same memory */
        *ptr += ADJ_GATT_STRUCT_OFFSET(GTPSS);
        return TRUE;
    }
    
    /* Tx Power Service library initialization failed */
    GATT_DEBUG(("GATT Tx Power Server init failed \n"));
    return FALSE;
}

#endif /* GATT_TPS_SERVER */

bool sinkGattTxPowerServiceEnabled(void)
{
#ifdef GATT_TPS_SERVER
    return TRUE;
#else
    return FALSE;
#endif
}

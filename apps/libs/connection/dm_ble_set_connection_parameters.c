/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_set_connection_parameters.c      

DESCRIPTION
    Implements ConnectionDmBleSetConnectionParametersReq().

NOTES
    DM_SET_BLE_CONNECTION_PARAMETERS_CFM is implemented in dm_ble_handler.c
*/

#ifndef DISABLE_BLE

/****************************************************************************
    Header files
*/

#include "connection.h"
#include "connection_private.h"

#include <vm.h>

void ConnectionDmBleSetConnectionParametersReq(
    const ble_connection_params *params)
{
    MAKE_PRIM_T(DM_SET_BLE_CONNECTION_PARAMETERS_REQ);

    prim->scan_interval = params->scan_interval;
    prim->scan_window = params->scan_window;
    prim->conn_interval_min = params->conn_interval_min;
    prim->conn_interval_max = params->conn_interval_max;
    prim->conn_latency = params->conn_latency;
    prim->supervision_timeout = params->supervision_timeout;
    prim->conn_attempt_timeout = params->conn_attempt_timeout;
    prim->conn_latency_max = params->conn_latency_max;
    prim->supervision_timeout_min = params->supervision_timeout_min;
    prim->supervision_timeout_max = params->supervision_timeout_max;
    prim->own_address_type = params->own_address_type;

    VmSendDmPrim(prim);    
}

#else /* !DISBALE_BLE */
#   include <bdaddr_.h>
#endif 

/* End-of-File */

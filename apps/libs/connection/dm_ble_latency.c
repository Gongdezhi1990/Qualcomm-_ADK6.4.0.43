/****************************************************************************
Copyright (c) 2018 - 2019 Qualcomm Technologies International, Ltd 


FILE NAME
    dm_ble_latency.c      

DESCRIPTION
    This file is a stub for configuration of latency control.
    Builds requiring this should include CONFIG_LATENCY in the
    makefile. e.g.
        CONFIG_FEATURES:=CONFIG_LATENCY
NOTES

*/

#include "connection.h"
#include "connection_private.h"
#include "bdaddr.h"
#include "dm_ble_latency.h"

#include <vm.h>

#ifndef DISABLE_BLE

/****************************************************************************
NAME    
    connectionBluestackHandlerDmLatency

DESCRIPTION
    Handler for Latency messages

RETURNS
    void
*/
bool connectionBluestackHandlerDmLatency(const DM_UPRIM_T *message)
{
    UNUSED(message);
    /* Not supported */
    return FALSE;
}

#endif

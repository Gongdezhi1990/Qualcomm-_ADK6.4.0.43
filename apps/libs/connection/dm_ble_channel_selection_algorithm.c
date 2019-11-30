/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_channel_selection_algorithm.c      

DESCRIPTION
   
    This file is a stub for Channel Selection Algorithm.
    Builds requiring this should include CONFIG_CHANNEL_SELECTION in the
    makefile. e.g.
        CONFIG_FEATURES:=CONFIG_CHANNEL_SELECTION

NOTES

*/

/****************************************************************************
    Header files
*/

#include "connection.h"
#include "connection_private.h"
#include "bdaddr.h"
#include "dm_ble_channel_selection_algorithm.h"

#include <vm.h>

#ifndef DISABLE_BLE
/****************************************************************************
NAME    
    connectionBluestackHandlerDmChannelSelectionAlgorithm

DESCRIPTION
    Handler for Channel Selection Algorithm messages

RETURNS
    void
*/
bool connectionBluestackHandlerDmChannelSelectionAlgorithm(const DM_UPRIM_T *message)
{
    UNUSED(message);
    /* Not supported */
    return FALSE;
}

#endif
/* End-of-File */

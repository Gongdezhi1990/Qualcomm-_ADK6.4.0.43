/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_phy_preferences.c      

DESCRIPTION
   
    This file is a stub for 2Mbps LE.
    Builds requiring this should include CONFIG_2LE in the
    makefile. e.g.
        CONFIG_FEATURES:=CONFIG_PHY_REFERENCES

NOTES

*/

/****************************************************************************
    Header files
*/

#include "connection.h"
#include "connection_private.h"
#include "bdaddr.h"
#include "dm_ble_phy_preferences.h"

#include <vm.h>

#ifndef DISABLE_BLE
/****************************************************************************
NAME    
    connectionBluestackHandlerDm2le

DESCRIPTION
    Handler for PHY Preferences messages

RETURNS
    TRUE if message handled, otherwise FALSE
*/
bool connectionBluestackHandlerDmPhyPreferences(const DM_UPRIM_T *message)
{
    UNUSED(message);
    /* Not supported */
    return FALSE;
}

#endif
/* End-of-File */

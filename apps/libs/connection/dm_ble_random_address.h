/****************************************************************************
Copyright (c) 2011 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_random_address.h      

DESCRIPTION
    This file contains the prototypes for BLE DM Random Address functions.

NOTES

*/

#include "connection.h"
#include "connection_private.h"

#include <app/bluestack/dm_prim.h>
/****************************************************************************
NAME    
    connectionHandleDmSmAutoConfigureLocalAddressCfm

DESCRIPTION
    Handle the DM_SM_AUTO_CONFIGURE_LOCAL_ADDRESS_CFM from Bluestack.

RETURNS
   void
*/
void connectionHandleDmSmAutoConfigureLocalAddressCfm(
        const DM_SM_AUTO_CONFIGURE_LOCAL_ADDRESS_CFM_T* cfm
        );

/****************************************************************************
NAME    
    connectionHandleSmReadRandomAddressCfm

DESCRIPTION
    Handle the DM_SM_READ_RANDOM_ADDRESS_CFM_T from Bluestack.

RETURNS
   void
*/
void connectionHandleDmSmReadRandomAddressCfm(
        const DM_SM_READ_RANDOM_ADDRESS_CFM_T* cfm
        );

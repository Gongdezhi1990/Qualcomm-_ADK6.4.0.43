
/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_white_list.c      

DESCRIPTION
    This file contains functions for manipulating the BLE  White List. 

NOTES

*/

#include "connection.h"
#include "connection_private.h"

#include <bdaddr.h>
#include <vm.h>

#ifndef DISABLE_BLE
/****************************************************************************
NAME    
    ConnectionDmBleReadWhiteListSizeReq

DESCRIPTION
    Read the total size of the BLE White List that can be stored in the 
    controller (Bluestack).

RETURNS
   void
*/
void ConnectionDmBleReadWhiteListSizeReq(void)
{
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_READ_WHITE_LIST_SIZE_REQ);
        VmSendDmPrim(prim);
    }
}

/****************************************************************************
NAME    
    ConnectionDmBleReadWhiteListSizeReq

DESCRIPTION
    Clear all devices in the BLE White List stored in the controller.

RETURNS
   void
*/
void ConnectionDmBleClearWhiteListReq(void)
{
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_CLEAR_WHITE_LIST_REQ);
        VmSendDmPrim(prim);
    }
}

/****************************************************************************
NAME    
    ConnectionDmBleAddDeviceToWhiteListReq

DESCRIPTION
    Add a single device to the BLE White List stored in the controller.

RETURNS
   void
*/
void ConnectionDmBleAddDeviceToWhiteListReq(
        uint8 bd_addr_type, 
        const bdaddr *bd_addr
        )
{
#ifdef CONNECTION_DEBUG_LIB
    if (bd_addr_type > TYPED_BDADDR_RANDOM)
    {
        CL_DEBUG(("'bd_addr_type' must be TYPED_BDADDR_PUBLIC or _RANDOM\n"));
    }
    if (!bd_addr)
    {
        CL_DEBUG(("'addr' cannot be NULL\n"));
    }
#endif
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_ADD_DEVICE_TO_WHITE_LIST_REQ);
        prim->address_type = bd_addr_type;
        BdaddrConvertVmToBluestack(&prim->address, bd_addr);
        VmSendDmPrim(prim);
    }
}


/****************************************************************************
NAME    
    ConnectionDmBleRemoveDeviceFromWhiteListReq

DESCRIPTION
    Remove a single device from the BLE White List stored in the controller.

RETURNS
   void
*/
void ConnectionDmBleRemoveDeviceFromWhiteListReq(
        uint8 bd_addr_type, 
        const bdaddr *bd_addr
        )
{
#ifdef CONNECTION_DEBUG_LIB
    if (bd_addr_type > TYPED_BDADDR_RANDOM)
    {
        CL_DEBUG(("'bd_addr_type' must be TYPED_BDADDR_PUBLIC or _RANDOM\n"));
    }
    if (!bd_addr)
    {
        CL_DEBUG(("'addr' cannot be NULL\n"));
    }
#endif
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_REMOVE_DEVICE_FROM_WHITE_LIST_REQ);
        prim->address_type = bd_addr_type;
        BdaddrConvertVmToBluestack(&prim->address, bd_addr);
        VmSendDmPrim(prim);
    }
}
#endif /* DISABLE_BLE */

/****************************************************************************
Copyright (c) 2011 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_random_address.c

DESCRIPTION
    This file contains functions for setting the Bluetooth low energy random
    address.
NOTES

*/

#ifndef DISABLE_BLE

#include "dm_ble_random_address.h"
#include "common.h"
#include "bdaddr.h"

#include <vm.h>
#include <app/bluestack/dm_prim.h>

/****************************************************************************
NAME
    ConnectionDmBleConfigureLocalAddressAutoReq

DESCRIPTION
    Configure the local device address used for BLE connections, with a TGAP
    timer for use if the address type is ble_local_addr_generate_resolvable 
    (RPA).

RETURNS
   void
*/
void ConnectionDmBleConfigureLocalAddressAutoReq(
        ble_local_addr_type     addr_type,
        const typed_bdaddr*     static_taddr,
        uint16                  rpa_tgap_timeout
        )
{

    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else if (addr_type >= ble_local_addr_last)
    {
        CL_DEBUG(("addr_type parameter out of range: %d\n", addr_type));
    }
    else
    {
        connectionState *cstate = (connectionState*)connectionGetCmTask();
        MAKE_PRIM_T(DM_SM_AUTO_CONFIGURE_LOCAL_ADDRESS_REQ);

        switch (addr_type)
        {
            case ble_local_addr_write_static:
                prim->set_random_addr_type = DM_SM_ADDRESS_WRITE_STATIC;
                break;
            case ble_local_addr_generate_static:
                prim->set_random_addr_type = DM_SM_ADDRESS_GENERATE_STATIC;
                break;
            case ble_local_addr_generate_non_resolvable: 
                prim->set_random_addr_type = 
                    DM_SM_ADDRESS_GENERATE_NON_RESOLVABLE;
                break;
            case ble_local_addr_generate_resolvable:
                prim->set_random_addr_type = 
                    DM_SM_ADDRESS_GENERATE_RESOLVABLE;
                break;
            default:
                break;
        }

        /* Store the configured address type so that:
         * 1) It can be returned in the Cfm message.
         * 2) It can be used to determine what to do when receiving 
         *    DM_SM_READ_RANDOM_ADDRESS_CFM.
         */
        cstate->configAddrType = addr_type;

        if (static_taddr)
        {
            /* Transport type is ALWAYS BLE. */
            prim->static_addrt.tp_type = LE_ACL;
            BdaddrConvertTypedVmToBluestack(
                    &prim->static_addrt.addrt, 
                    static_taddr
                    );
        }
        else
        {
            prim->static_addrt.tp_type = NO_PHYSICAL_TRANSPORT;
            prim->static_addrt.addrt.type = TBDADDR_INVALID;
            memset(&prim->static_addrt.addrt.addr, 0, sizeof(BD_ADDR_T));
        }

        prim->rpa_timeout = 
            (rpa_tgap_timeout < BLE_RPA_TIMEOUT_MINIMUM) ? 
                BLE_RPA_TIMEOUT_MINIMUM : 
                    (rpa_tgap_timeout > BLE_RPA_TIMEOUT_MAXIMUM) ?
                        BLE_RPA_TIMEOUT_MAXIMUM : rpa_tgap_timeout;

        VmSendDmPrim(prim);
    }
}


/****************************************************************************
NAME
    ConnectionDmBleConfigureLocalAddressReq

DESCRIPTION
    Configure the local device address used for BLE connections

RETURNS
   void
*/
void ConnectionDmBleConfigureLocalAddressReq(
        ble_local_addr_type     addr_type,
        const typed_bdaddr*     static_taddr
        )
{
    ConnectionDmBleConfigureLocalAddressAutoReq(
            addr_type,
            static_taddr,
            BLE_RPA_TIMEOUT_DEFAULT
            );
}

/****************************************************************************
NAME    
    sendConfigureLocalAddressCfm

DESCRIPTION
    Construct and send a CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM message.

RETURNS
   void
*/
static void sendConfigureLocalAddressCfm(
        uint8                   status,
        ble_local_addr_type     addr_type,
        const typed_bdaddr*     taddr
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM);

    message->status = (status) ? fail : success;
    message->addr_type = addr_type;

    if (taddr)
    {
        message->random_taddr = *taddr;
    }
    else
    {
        message->random_taddr.type =  TYPED_BDADDR_INVALID;
        memset(&message->random_taddr.addr, 0, sizeof(typed_bdaddr));
    }

    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM,
            message
            );
}

/****************************************************************************
NAME    
    connectionHandleDmSmAutoConfigureLocalAddressCfm

DESCRIPTION
    Handle the DM_SM_AUTO_CONFIGURE_LOCAL_ADDRESS_CFM message from Bluestack.

RETURNS
   void
*/
void connectionHandleDmSmAutoConfigureLocalAddressCfm(
        const DM_SM_AUTO_CONFIGURE_LOCAL_ADDRESS_CFM_T*  cfm
        )
{
    connectionState *cstate = (connectionState*)connectionGetCmTask();

    /* If the Configure failed (status is not 0) send the cfm immediately. 
     */
   if (cfm->status)
    {
        sendConfigureLocalAddressCfm(cfm->status, cstate->configAddrType, NULL);
        cstate->configAddrType = ble_local_addr_last;
    }
    /* Trigger a Read Random Address to get the local RPA. */
    else
    {
        ConnectionSmBleReadRandomAddressReq(
                ble_read_random_address_local,
                NULL
                );
    }
}

/****************************************************************************
NAME    
    ConnectionSmBleReadRandomAddressReq

DESCRIPTION
    Send the DM_SM_READ_RANDOM_ADDRESS_REQ to Bluestack.

RETURNS
   void
*/
void ConnectionSmBleReadRandomAddressReq(
    ble_read_random_address_flags   flags,
    const tp_bdaddr                 *peer_tpaddr
    )
{
    /* The tpaddr can be all 0 for reading the current local rpa. */
    tp_bdaddr   temp_tpaddr = {{0, {0, 0, 0}}, 0};
    MAKE_PRIM_T(DM_SM_READ_RANDOM_ADDRESS_REQ);

    prim->flags = DM_SM_READ_ADDRESS_LOCAL;

    switch(flags) {
    case ble_read_random_address_peer:
        /* This CANNOT be null for reading the Peer address. */
        PanicNull((void *)peer_tpaddr);
        prim->flags = DM_SM_READ_ADDRESS_PEER;
        /* DROP THROUGH */
    case ble_read_random_address_local:
        /* if peer_tpaddr is NULL, read the current random address, else if
         * peer_tpaddr is set, read this device's RPA that was used to connect
         * with the peer. */
        if (peer_tpaddr) {
            temp_tpaddr = *peer_tpaddr;
        }
        break;
    default:
        CL_DEBUG(("Read Random Address: flags out of range.\n"));
        Panic();
        break;
    }

    BdaddrConvertTpVmToBluestack(&prim->tp_peer_addrt, &temp_tpaddr);
    VmSendDmPrim(prim);
}


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
        )
{
    connectionState *cstate = (connectionState*)connectionGetCmTask();

    /* if this is set, then the Read Random Address was done in the context
     * of Configuring the Local Address, so need to return that instead.
     */
    if (cstate->configAddrType < ble_local_addr_last)
    {
        typed_bdaddr temp_taddr;

        /* This should never happen */
        if (cfm->flags != DM_SM_READ_ADDRESS_LOCAL)
        {
            CL_DEBUG((
                "Read Peer Address in context of Local Address Config\n"
                ));
        }
    
        BdaddrConvertTypedBluestackToVm(&temp_taddr, &cfm->tp_addrt.addrt);

        sendConfigureLocalAddressCfm( 
                cfm->status,
                cstate->configAddrType,
                &temp_taddr
                );

        /* Reset the context now that the Local Addr Config CFM has been sent. 
         */
        cstate->configAddrType = ble_local_addr_last;
    } 
    /* Otherwise, this is in response to Read Random Address from the app. */
    else
    {
        MAKE_CL_MESSAGE(CL_SM_BLE_READ_RANDOM_ADDRESS_CFM);

        message->status = (cfm->status) ? fail : success;
        switch(cfm->flags)
        {
            case DM_SM_READ_ADDRESS_LOCAL:
                message->flags = ble_read_random_address_local;
                break;
            case DM_SM_READ_ADDRESS_PEER:
                message->flags = ble_read_random_address_peer;
                break;
            default:
                message->flags = 0;
                CL_DEBUG((
                    "Read Random Addr: Invalid flags returned from Bluestack\n"
                    ));
                break;
        }
        BdaddrConvertTpBluestackToVm(
                &message->peer_tpaddr, 
                &cfm->tp_peer_addrt
                );
        BdaddrConvertTpBluestackToVm(&message->random_tpaddr, &cfm->tp_addrt);

        MessageSend(
            connectionGetAppTask(), 
            CL_SM_BLE_READ_RANDOM_ADDRESS_CFM,
            message
            );
    }
}

#else

#include <bdaddr_.h>

#endif /* DISABLE_BLE */


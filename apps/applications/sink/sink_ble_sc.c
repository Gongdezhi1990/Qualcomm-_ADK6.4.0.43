/****************************************************************************
Copyright (c) 2015 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ble_sc.c

DESCRIPTION
    BLE SC functionality
*/

#include <vm.h>
#include "sink_ble_sc.h"
#include "sink_ble_gap.h"
#include "sink_main_task.h"

#ifdef GATT_ENABLED

#ifdef DEBUG_BLE_SC
#define BLE_SC_DEBUG(x) DEBUG(x)
#else
#define BLE_SC_DEBUG(x)
#endif

/***************************************************************************/
void sinkBleLinkSecurityInd(const CL_SM_BLE_LINK_SECURITY_IND_T *ind)
{
    sink_attributes attributes;
    BLE_SC_DEBUG(("LE_SC : sinkBleLinkSecurityInd with mode [%x]\n", ind->le_link_sc));

    deviceManagerClearAttributes(&attributes);

    if (deviceManagerGetAttributes(&attributes, &ind->bd_addr))
    {
        attributes.le_mode = ind->le_link_sc ? sink_le_secure_connection_mode: sink_no_le_secure_connection;
        deviceManagerStoreAttributes(&attributes, &ind->bd_addr);
    }
}

/***************************************************************************/
static bool sinkBleLeLinkIsSecure(const typed_bdaddr *tp_addr)
{
    sink_attributes attributes;
    tp_bdaddr public_addr;
    BLE_SC_DEBUG(("LE_SC : sinkBleLeLinkIsSecure\n"));

    if(tp_addr->type == TYPED_BDADDR_RANDOM)
    {
        tp_bdaddr current_addr;
        current_addr.transport = TRANSPORT_BLE_ACL;
        memcpy(&current_addr.taddr,tp_addr, sizeof(typed_bdaddr));
        VmGetPublicAddress(&current_addr, &public_addr);
    }
    else
    {
        public_addr.transport = TRANSPORT_BLE_ACL;
        memcpy(&public_addr.taddr,tp_addr, sizeof(typed_bdaddr));
    }

    deviceManagerClearAttributes(&attributes);

    /* Get the attributes for the given bd address*/
    deviceManagerGetAttributes(&attributes, &public_addr.taddr.addr);

    BLE_SC_DEBUG(("LE_SC : LE Link Mode:[%x]\n", attributes.le_mode));

    return attributes.le_mode;
}

/****************************************************************************
NAME
    sinkBleLinkWriteApt

DESCRIPTION
    Writes APT value on LE link if link is Secure

PARAMETERS
    @param cid Connection Id.

RETURNS
    None.
*/
void sinkBleWriteApt(uint16 cid)
{
    tp_bdaddr current_addr;
    BLE_SC_DEBUG(("LE_SC : sinkBleWriteApt\n"));

    if(cid != INVALID_CID)
    {
        if(VmGetBdAddrtFromCid(cid,&current_addr))
        {
            if(sinkBleLeLinkIsSecure(&current_addr.taddr))
            {
                BLE_SC_DEBUG(("LE_SC : Write APT Value to the controller.\n"));

                /* Write the APT value to the controller for the link */
                ConnectionWriteAPT(&theSink.task, &current_addr,sinkBLEGetLeAuthenticatedPayloadTO(), cl_apt_bluestack);
            }
        }
    }
}

/****************************************************************************
NAME
    sinkBleSetLeLinkIndication

DESCRIPTION
    Indicatation to the user if the link is encrypted and is secure

PARAMETERS
    @param tp_addr The remote device address with which LE link is established

RETURNS
    None.
*/
void sinkBleSetLeLinkIndication(const typed_bdaddr  *tp_addr)
{
    BLE_SC_DEBUG(("LE_SC : sinkBleSetLeLinkIndication\n"));

    if(sinkBleLeLinkIsSecure(tp_addr))
    {
       /* Notify the app that LE link is secure */
       MessageSend(&theSink.task, EventSysLESecureLink, 0);
    }
}

/****************************************************************************
NAME
    sinkBleSetEncryptionChangeInd

DESCRIPTION
    Handle notification from Bluestack of a possible change in the encryption status 
    of a link due to an encryption procedure initiated by the remote device.

PARAMETERS
   @param ind Handle received as a result of encryption change indication

RETURNS
    None
*/
void sinkHandleBleEncryptionChangeInd(const CL_SM_ENCRYPTION_CHANGE_IND_T* ind)
{
    BLE_SC_DEBUG(("LE_SC : sinkHandleBleEncryptionChangeInd\n"));

    if(ind->encrypted && ind->tpaddr.transport == TRANSPORT_BLE_ACL)
    {
        /* if the link is encrypted and transport is TRANSPORT_BLE_ACL,
        * inform the user that LE link is Secure (i.e. link is encrypted 
        * using AES-CCM encryption) if LE pairing was Secure
        */
        sinkBleSetLeLinkIndication(&ind->tpaddr.taddr);
    }
}

#endif /* GATT_ENABLED */


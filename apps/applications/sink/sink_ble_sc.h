/****************************************************************************
Copyright (c) 2015 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ble_sc.h

DESCRIPTION
    BLE SC functionality
*/
#include "sink_devicemanager.h"

#include <connection.h>

/****************************************************************************
NAME
    sinkBleLinkSecurityInd

DESCRIPTION
    Handle received CL_SM_BLE_LINK_SECURITY_IND_T as a result of BLE pairing completion.

PARAMETERS
    @param ind Handle received as a result of BLE pairing completeion to tell about LE link SC or not.

RETURNS
    None.
*/
#ifdef GATT_ENABLED
void sinkBleLinkSecurityInd(const CL_SM_BLE_LINK_SECURITY_IND_T *ind);
#else
#define sinkBleLinkSecurityInd(message) ((void)(0))
#endif

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
#ifdef GATT_ENABLED
void sinkBleWriteApt(uint16 cid);
#else
#define sinkBleWriteApt(cid) ((void)(0))
#endif

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
#ifdef GATT_ENABLED
void sinkHandleBleEncryptionChangeInd(const CL_SM_ENCRYPTION_CHANGE_IND_T* ind);
#else
#define sinkHandleBleEncryptionChangeInd(message) ((void)(0))
#endif

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
#ifdef GATT_ENABLED
void sinkBleSetLeLinkIndication(const typed_bdaddr  *tp_addr);
#else
#define sinkBleSetLeLinkIndication(tp_addr) ((void)(0))
#endif


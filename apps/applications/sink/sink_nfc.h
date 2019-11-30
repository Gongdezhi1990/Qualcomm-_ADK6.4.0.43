/****************************************************************************
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.
  
  
*/

/*****************************************************************************/
/*!

\ingroup sink_app

\brief  This header files provide function prototype to configure basic NFC 
functionality.  It contains function illustration how to handle Rx messages
from nfc connection library.

@{
*/
#ifndef SINK_NFC_H
#define SINK_NFC_H

#include <message.h>
#include <library.h>

/**
 * "ENABLE_ADK_NFC" used to enable the NFC subsystem and 
 * configure the NFC TAG with an NFC Bluetooth Static 
 * Handover. It should be defined in the ADK for product with 
 * NFC enabled */ 
#if defined(ENABLE_ADK_NFC)

#include <connection.h>
#include "bdaddr_.h"

/* PUBLIC MACROS *************************************************************/
#define NFC_APP "NFC_APP: "

/* PUBLIC FUNCTION DECLARATIONS **********************************************/

/*****************************************************************************/
/*!
    @brief configures the NFC subsystem as a TAG.

    @param client_nfc_cl_task task that will receive the NFC_CL indication and
    confirm messages.
*/
extern void sinkNfcTagConfigReq(Task client_nfc_cl_task);

/*****************************************************************************/
/*!
    @brief handles local bluetooth address to be programmed in the NFC handover

    @param cl_dm_task that has received the CL message 
    (e.g. CL_DM_LOCAL_BD_ADDR_CFM_T).
    @param bdaddr_cfm pointer to the received to the CL_DM_LOCAL_BD_ADDR_CFM_T
*/
extern void sinkNfcHandleLocalBDAddr(Task cl_dm_task,
                                     const CL_DM_LOCAL_BD_ADDR_CFM_T *bdaddr_cfm);

/*****************************************************************************/
/*!
    @brief NFC handles local name to be programmed in the NFC handover

    @param cl_dm_task that has received the CL message
    (e.g. CL_DM_LOCAL_NAME_COMPLETE_T)
    @param name_complete pointer to the received CL_DM_LOCAL_NAME_COMPLETE_T
*/
extern void sinkNfcHandleLocalName(Task cl_dm_task,
                                   const CL_DM_LOCAL_NAME_COMPLETE_T *name_complete);

/*****************************************************************************/
/*!
    @brief NFC sets the device class to be programmed in the NFC handover
*/
extern void sinkNfcSetClassOfDevice(Task cl_dm_task);

/*****************************************************************************/
/*!
    @brief writes the selected carrier to program for the NFC handover

    @warning this function relies on the mandatory information to be already 
    encoded first.  For instance: CL_DM_LOCAL_BD_ADDR_CFM_T and 
    CL_DM_LOCAL_NAME_COMPLETE_T.

    @param carrier (i.e. Bluetooth)
*/
extern void sinkNfcWriteChCarriersReq(void);

/*!
    @brief Disable Fast Connect 
    @warning Experimental code
*/
extern void sinkNfcDisableFastConnect(void);
/*!
    @brief Enable Fast Connect 
    @warning Experimental code
*/
extern void sinkNfcEnableFastConnect(void);

/*!
    @brief Indicates to NFC that a BT connection with the device is completed 

    When received for a single AG
        - Stop Fast Connect procedure if it is still running
    When received for the second AG
       - Page request should be ignored after
       - Paring request should be ignored

    @warning Experimental code

    @param any running timers will be cancelled using the "task"
*/
extern void sinkNfcBTDeviceConnectedInd(Task task);
extern void sinkNfcBTDeviceDisconnectedInd(Task task);

/*****************************************************************************/
/*!
    @brief Function to handle the NFC CL Lib messages - these are independent
    of state.

    @param task the Sink App task
    @param id NFC message Id
    @param message Body of the message.  NULL when there is no message body.

*/
extern void handleNfcClMessage(Task task, MessageId id, Message message);

/*!
    @brief returns TRUE if the connected page scan timer is running.
    
    - The page scan should be running to accept page request.
    - Pairing request should be accepted.
    
    @return TRUE if running
*/
extern bool sinkNfcIsSecondAGPairingAllowed(void);

/*!
    @brief indicates to NFC that the authentication has been successfull
 
    when received in any connected states
    - Page request should be ignored after
    - Paring request should be ignored
 
    @param any running timers will be cancelled using the "task"
*/
extern void sinkNfcAuthenticationSuccessInd(Task task);

bool sinkNfcIsMessageIdNfc(MessageId id);

#else
#define sinkNfcTagConfigReq(task) ((void)0)
#define sinkNfcHandleLocalBDAddr(task, bdaddr_cfm) ((void)0)
#define sinkNfcHandleLocalName(task, name_complete) ((void)0)
#define sinkNfcWriteChCarriersReq(carrier) ((void)0)
#define sinkNfcSetClassOfDevice(task) ((void)0)
#define sinkNfcBTDeviceConnectedInd(task) ((void)0)
#define sinkNfcBTDeviceDisconnectedInd(task) ((void)0)
#define sinkNfcIsSecondAGPairingAllowed() (FALSE)
#define sinkNfcAuthenticationSuccessInd(task) ((void)0)
#define sinkNfcIsMessageIdNfc(id) (FALSE)

#endif /* ENABLE_ADK_NFC */

/*!
    @brief This function is called when the NFC RF field has been detected
    It is provided for ADK4.0 backward compatibility.

    @param task the Sink App task
    @param message detection message

    @return void
*/
void handleUsrNFCTagDetected(Task task,  Message message);

/** @}*/
#endif


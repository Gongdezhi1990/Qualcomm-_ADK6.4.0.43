/****************************************************************************

Copyright (c) 2015 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_sc.h

DESCRIPTION
    header file for BR/EDR SC functionality

NOTES

*/
#ifndef _SINK_SC_H_
#define _SINK_SC_H_

#include "sink_devicemanager.h"

#ifdef ENABLE_BREDR_SC
#define SC_CONNECTION_LIB_OPTIONS sinkSCIsBREdrGetSecureConnectionMode()
#else
#define SC_CONNECTION_LIB_OPTIONS CONNLIB_OPTIONS_NONE
#endif

#if defined ENABLE_BREDR_SC && defined TEST_SCOM
void test_sinkHandleUserConfirmation(bool accept);
#else
#define test_sinkHandleUserConfirmation(message) ((void)(0))
#endif

/****************************************************************************
NAME
    scWriteApt

DESCRIPTION
    Write the APT value for the BR/EDR link. The application writes the minimum 
    of the (configurable)default value or AUTHENTICATED_PAYLOAD_TIMEOUT_HFP_SC_MAX
    if APT value is more than AUTHENTICATED_PAYLOAD_TIMEOUT_HFP_SC_MAX in case
    of HFP. Different profile may have different APT requirement, currently 
    HFP1.7 Spec Section6.2 defines APT be less than or equal to 10s

PARAMETERS
    bd_addr The Bluetooth address of the remote device.

RETURNS
    None
*/
#ifdef ENABLE_BREDR_SC
void scWriteApt(const bdaddr* bd_addr);
#else
#define scWriteApt(bd_addr) ((void)(0))
#endif


/****************************************************************************
NAME
    scSetHfpLinkMode

DESCRIPTION
    Informs hfp library about the link is secure.
    Notifies the app the HFP slc is secure.

PARAMETERS
    bd_addr     The Bluetooth address to match.
    mode         Secure connection mode.

RETURNS
    None
*/
#ifdef ENABLE_BREDR_SC
void scSetHfpLinkMode(const bdaddr* bd_addr, sink_link_mode mode);
#else
#define scSetHfpLinkMode(bd_addr,mode) ((void)(0))
#endif


/****************************************************************************
NAME
     isLinkSecure

DESCRIPTION
     This function is called to check if the given hfp link is secure or not.

PARAMETERS
    priority        Priority of the remote device.

RETURNS
     TRUE if BR/EDR link is Secure, FALSE if not
*/
#ifdef ENABLE_BREDR_SC
bool  isLinkSecure(hfp_link_priority priority);
#else
#define isLinkSecure(priority) FALSE
#endif

/*******************************************************************************
NAME
    sinkHandleBrEdrEncryptionCfm

DESCRIPTION
    Handles encryption change Indication on BR/EDR Link when encryption is 
    initiated by local device and confirm is received.

PARAMETERS
    cfm     Pointer to encrypt confirmation message.

RETURNS
    None
*/
#ifdef ENABLE_BREDR_SC
void sinkHandleBrEdrEncryptionCfm(const CL_SM_ENCRYPT_CFM_T* cfm);
#else
#define sinkHandleBrEdrEncryptionCfm(cfm) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkHandleBrEdrEncryptionChangeInd

DESCRIPTION
    Handles encryption change Indication on BR/EDR Link when encryption change
    indication is received as a result of encryption initiated by remote device

PARAMETERS
    ind     Pointer to encryption change indication message.

RETURNS
    None
*/
#ifdef ENABLE_BREDR_SC
void sinkHandleBrEdrEncryptionChangeInd(const CL_SM_ENCRYPTION_CHANGE_IND_T* ind);
#else
#define sinkHandleBrEdrEncryptionChangeInd(ind) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkSCIsBREdrGetSecureConnectionMode
    
DESCRIPTION
    Check if BR/EDR Secure Connection Mode is Enabled 
    
RETURNS
    uint8
*/
#ifdef ENABLE_BREDR_SC
uint8 sinkSCIsBREdrGetSecureConnectionMode(void);
#else
#define sinkSCIsBREdrGetSecureConnectionMode(void)  (0)
#endif

/*******************************************************************************
NAME
    sinkSCGetBrEdrAuthenticationPaylodTO
    
DESCRIPTION
    Get BR/EDR Secure Connection Authentication Payload Time Out
    
RETURNS
    uint16
*/
#ifdef ENABLE_BREDR_SC
uint16 sinkSCGetBrEdrAuthenticationPaylodTO(void);
#else
#define sinkSCGetBrEdrAuthenticationPaylodTO(void)  (0)
#endif

/*******************************************************************************
NAME
    sinkSCSetBrEdrAuthenticationPaylodTO
    
DESCRIPTION
    Set BR/EDR Secure Connection Authentication Payload Time Out
    
PARAMS
    uint16

RETURNS
    bool
*/
#ifdef ENABLE_BREDR_SC
bool sinkSCSetBrEdrAuthenticationPaylodTO(uint16 timeout);
#else
#define sinkSCSetBrEdrAuthenticationPaylodTO(timeout) (FALSE)
#endif

#endif /* _SINK_SC_H_ */

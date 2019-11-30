/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_security.h      

DESCRIPTION
    This file contains the prototypes for BLE Dm Security functions.

NOTES

*/
#ifndef DISABLE_BLE
/****************************************************************************
NAME    
    connectionHandleInternalBleDmSecurityReq

DESCRIPTION
    Deals with the internal message (see above). State lock for the task is 
    not required as it can be passed as the context for the Security message
    scenario.

RETURNS
   void
*/
void connectionHandleInternalBleDmSecurityReq(
        const CL_INTERNAL_SM_DM_SECURITY_REQ_T *req
        );

/****************************************************************************
NAME    
    connectionHandleDmSmSecurityCfm

DESCRIPTION
    Handle the DM_SM_SECURITY_CFM message from Bluestack.

RETURNS
   void
*/
void connectionHandleDmSmSecurityCfm(const DM_SM_SECURITY_CFM_T *cfm);        

#endif /* DISABLE_BLE */

/****************************************************************************
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_handler.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_DM_SECURITY_HANDLER_H_
#define	CONNECTION_DM_SECURITY_HANDLER_H_
		
		
/****************************************************************************
NAME	
    connectionHandleSmAddDeviceCfm	

DESCRIPTION
    This function is called whenever a device is added to the Bluestack
    Security Managers device database and the connection library is
	initialising.

RETURNS

*/
void connectionHandleSmAddDeviceCfm(
        connectionSmState* smState,
        const DM_SM_ADD_DEVICE_CFM_T* cfm
        );


/****************************************************************************
NAME	
    connectionHandleSmAccessInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_ACCESS_IND from Bluestack

RETURNS
	
*/
void connectionHandleSmAccessInd(
        connectionSdpState* sdpState,
        const DM_SM_ACCESS_IND_T* ind
        );


/****************************************************************************
NAME	
    connectionHandleSmAddDeviceCfmReady	

DESCRIPTION
    This function is called whenever a device is added to the Bluestack
    Security Managers device database and the connection library has been
	initialised.

RETURNS

*/
void connectionHandleSmAddDeviceCfmReady(
        connectionSmState* smState, 
        const DM_SM_ADD_DEVICE_CFM_T* cfm
        );

/****************************************************************************
NAME	
    connectionHandleSmKeyReqInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_KEY_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmKeyReqInd(const DM_SM_KEY_REQUEST_IND_T* ind);

/****************************************************************************
NAME	
    connectionHandleSmLinkKeyReqIndOld	

DESCRIPTION
    This function is called on receipt of a DM_SM_KEY_IND
    from the Bluestack Security Manager. 

RETURNS

*/
void connectionHandleSmKeysInd(
        Task theAppTask,
        connectionSmState* smState,
        const DM_SM_KEYS_IND_T* ind
        );

/****************************************************************************
NAME	
    connectionHandleSmBondingCfm	

DESCRIPTION
    This function is called on receipt of a DM_SM_BONDING_CFM
    from the Bluestack Security Manager.

RETURNS

*/

void connectionHandleSmBondingCfm(
        connectionSmState* smState,
        const DM_SM_BONDING_CFM_T* cfm
        );


/****************************************************************************
NAME	
    connectionHandleSmPinReqInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_PIN_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmPinReqInd(
        Task theAppTask,
        const DM_SM_PIN_REQUEST_IND_T* ind
        );


/****************************************************************************
NAME	
    connectionHandleSmChangeLinkKey

DESCRIPTION
    This function is called on receipt of a CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ

RETURNS
	
*/
void connectionHandleSmChangeLinkKeyReq(
        const CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ_T* req
        );

	
/****************************************************************************
NAME	
    connectionHandleSmIoCapReqInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_IO_CAPABILITY_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmIoCapReqInd(
        Task theAppTask,
        connectionSmState* smState,
        const DM_SM_IO_CAPABILITY_REQUEST_IND_T* ind
        );


/****************************************************************************
NAME
    connectionHandleSmIoCapResInd

DESCRIPTION
    This function is called on receipt of a DM_SM_IO_CAPABILITY_RESPONSE_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmIoCapResInd(
        Task theAppTask,
        connectionSmState* smState,
        const DM_SM_IO_CAPABILITY_RESPONSE_IND_T* ind
        );


/****************************************************************************
NAME
    connectionHandleSmUserConfirmationReqInd

DESCRIPTION
    This function is called on receipt of a DM_SM_USER_CONFIRMATION_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmUserConfirmationReqInd(
        Task theAppTask,
        const DM_SM_USER_CONFIRMATION_REQUEST_IND_T* ind
        );


/****************************************************************************
NAME	
    connectionHandleSmUserPasskeyReqInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_USER_PASSKEY_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmUserPasskeyReqInd(
        Task theAppTask,
        const DM_SM_USER_PASSKEY_REQUEST_IND_T* ind
        );
		
		
/****************************************************************************
NAME	
    connectionHandleSmUserPasskeyNotificationInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_USER_PASSKEY_NOTIFICATION_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmUserPasskeyNotificationInd(
        Task theAppTask,
        const DM_SM_USER_PASSKEY_NOTIFICATION_IND_T* ind
        );


/****************************************************************************
NAME	
    connectionHandleSmKeypressNotificationInd	

DESCRIPTION
	This function is called on receipt of a DM_SM_KEYPRESS_NOTIFICATION_IND
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmKeypressNotificationInd(
        Task theAppTask,
        const DM_SM_KEYPRESS_NOTIFICATION_IND_T* ind
        );


/****************************************************************************
NAME	
    connectionHandleSmAuthoriseInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_AUTHORISE_IND from the 
    Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmAuthoriseInd(
        Task theAppTask,
        const DM_SM_AUTHORISE_IND_T* ind
        );


/****************************************************************************
NAME	
    connectionHandleSmSimplePairingCompleteInd	

DESCRIPTION
    This function is called on receipt of a DM_SM_SIMPLE_PAIRING_COMPLETE_IND 
    from the Bluestack Security Manager.

RETURNS
	
*/
void connectionHandleSmSimplePairingCompleteInd(
        Task theAppTask,
        connectionSmState* smState,
        const DM_SM_SIMPLE_PAIRING_COMPLETE_IND_T* ind
        );


/****************************************************************************
NAME	
    connectionHandleDmAclOpenCfm	

DESCRIPTION
    This function is called when confirmation that a previously requested
    open ACL connection has completed
RETURNS
	
*/
void connectionHandleDmAclOpenCfm(
        connectionSmState* smState,
        const DM_ACL_OPEN_CFM_T* cfm
        );

/****************************************************************************
NAME	
    connectionHandleDmAclOpenInd

DESCRIPTION
    An indication from BlueStack that an ACL has been opened. Some clients
	may need this information so pass the indication up to the task 
	registered as the "app task" (we don't know who else to pass this to!).

RETURNS
    void
*/
void connectionHandleDmAclOpenInd(
        Task task,
        connectionSmState* smState,
        const DM_ACL_OPENED_IND_T *ind
        );


/****************************************************************************
NAME	
    connectionHandleDmAclClosedInd	

DESCRIPTION
    This function is called when an ACL connection is closed.  If Authentication 
    is active and we have just paired with the psecified peer device
RETURNS
	
*/
void connectionHandleDmAclClosedInd(
        Task task,
        connectionSmState* smState,
        const DM_ACL_CLOSED_IND_T* ind
        );
		
		
/****************************************************************************
NAME	
    connectionHandleReadLocalOobDataReq	

DESCRIPTION
    This function is called on receipt of a CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ 
	message

RETURNS
	
*/
void connectionHandleReadLocalOobDataReq(
        connectionReadInfoState* infoState,
        connectionSmState* smState,
        const CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ_T* req
        );


/****************************************************************************
NAME	
    connectionHandleReadLocalOobDataCfm	

DESCRIPTION
    This function is called on receipt of a DM_SM_READ_LOCAL_OOB_DATA_CFM message

RETURNS
	
*/
void connectionHandleReadLocalOobDataCfm(
        connectionSmState* smState,
        const DM_SM_READ_LOCAL_OOB_DATA_CFM_T* cfm
        );

/****************************************************************************
NAME	
    connectionHandleAuthenticationReq	

DESCRIPTION
    This function is called on receipt of a CM_AUTHENTICATE_REQ message

RETURNS
	
*/
void connectionHandleAuthenticationReq(
        connectionSmState* smState,
        const CL_INTERNAL_SM_AUTHENTICATION_REQ_T* req
        );



/****************************************************************************
NAME	
    connectionHandleCancelAuthenticationReq	

DESCRIPTION
    This function is called on receipt of a CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ message

RETURNS
	
*/
void connectionHandleCancelAuthenticationReq(
        connectionReadInfoState* infoState,
        connectionSmState* smState,
        const CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ_T* req
        );


/****************************************************************************
NAME	
    connectionHandleAuthenticationTimeout	

DESCRIPTION
    This function is called on receipt of a 
    CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND message.  This indicates that
    that a request to pair to a remote device has times out.  Let the source
    task know

RETURNS
	
*/
void connectionHandleAuthenticationTimeout(connectionSmState* smState);


/****************************************************************************
NAME	
    handleSecurityInitReq    

DESCRIPTION
    This function is called on receipt of an CL_INTERNAL_SM_INIT_REQ message,
    which only occurs when connection library is in the initialising state.

RETURNS
    
*/
void handleSecurityInitReq(
        connectionReadInfoState             *state, 
        const CL_INTERNAL_SM_INIT_REQ_T     *req
        );


/****************************************************************************
NAME    
    connectionHandleSecurityInitCfm    

DESCRIPTION
    This function is called on receipt of an CL_INTERNAL_SM_INIT_REQ message,
    which only occurs when connection library is in the initialising state.

    It then send an internal state message so that the intialisation can move
    to the next state.
RETURNS
    
*/
void handleSecurityInitCfm(
        connectionReadInfoState *state, 
        const DM_SM_INIT_CFM_T  *cfm
        );



/****************************************************************************
NAME	
    handleSetSecurityModeReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_SET_SC_MODE_REQ message.

RETURNS
	
*/
void handleSetSecurityModeReq(
        connectionSmState* smState,
        const CL_INTERNAL_SM_SET_SC_MODE_REQ_T* req
        );


/****************************************************************************
NAME	
    handleSetDefaultSecurityLevelReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_SET_DEFAULT_SECURITY_LEVEL_REQ message.

RETURNS
	
*/
void handleSetSspSecurityLevelReq(
        connectionSmState* smState,
        const CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ_T* req
        );
		
	
/****************************************************************************
NAME	
    handleWriteAuthEnableReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ message.

RETURNS
	
*/
void handleSecModeConfigReq(
        connectionSmState* smState,
        connectionReadInfoState* infoState,
        const CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ_T* req
        );


/****************************************************************************
NAME	
    connectionHandleSetSecurityModeCfm	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_SET_SC_MODE_CFM message.

RETURNS
	
*/
void connectionHandleSetSecurityModeCfm(connectionSmState* smState, const DM_SM_INIT_CFM_T* req);


/****************************************************************************
NAME	
    connectionHandleConfigureSecurityCfm	

DESCRIPTION
    This function is called on receipt of an 
    DM_SM_SEC_MODE_CONFIG_CFM message.

RETURNS
	
*/
void connectionHandleConfigureSecurityCfm(
        connectionSmState* smState,
        const DM_SM_INIT_CFM_T* cfm
        );
		
		
/****************************************************************************
NAME	
    handleRegisterReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_REGISTER_REQ message.

RETURNS
	
*/
void handleRegisterReq(const CL_INTERNAL_SM_REGISTER_REQ_T* req);


/****************************************************************************
NAME	
    handleUnRegisterReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_UNREGISTER_REQ message.

RETURNS
	
*/
void handleUnRegisterReq(const CL_INTERNAL_SM_UNREGISTER_REQ_T* req);


/****************************************************************************
NAME	
    handleRegisterOutgoingReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_REGISTER_OUTGOING_REQ message.

RETURNS
	
*/
void handleRegisterOutgoingReq(
        const CL_INTERNAL_SM_REGISTER_OUTGOING_REQ_T* req
        );


/****************************************************************************
NAME	
    handleUnRegisterOutgoingReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ message.

RETURNS
	
*/
void handleUnRegisterOutgoingReq(
        const CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ_T* req
        );


/****************************************************************************
NAME	
    handleRegisterOutgoingCfm    

DESCRIPTION
    This function is called on receipt of an DM_SM_REGISTER_OUTGOING_CFM 
    message. Only CFM for an outgoing RFCOMM security channel are sent
    back to the task identified in the 'context' field. 

RETURNS
    
*/
void handleRegisterOutgoingCfm(const DM_SM_REGISTER_OUTGOING_CFM_T* cfm);


/****************************************************************************
NAME	
    handleEncryptReq

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_ENCRYPT_REQ message.

RETURNS
	
*/
void handleEncryptReq(
        connectionSmState *smState,
        const CL_INTERNAL_SM_ENCRYPT_REQ_T* req
        );


/****************************************************************************
NAME	
    connectionHandleEncryptCfm

DESCRIPTION
    Handle the confirm message informing us of the outcome of the encrypt 
	request.

RETURNS
	
*/
void connectionHandleEncryptCfm(
        connectionSmState *smState,
        const DM_SM_ENCRYPT_CFM_T *cfm
        );


/****************************************************************************
NAME	
    handleEncryptionKeyRefreshReq

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ message.

RETURNS
	
*/
void handleEncryptionKeyRefreshReq(
        connectionReadInfoState* infoState,
        const CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ_T* req
        );


/****************************************************************************
NAME	
    connectionHandleEncryptionKeyRefreshInd

DESCRIPTION
    Handle the encryption key refresh indication

RETURNS
	
*/
void connectionHandleEncryptionKeyRefreshInd(
        const DM_HCI_REFRESH_ENCRYPTION_KEY_IND_T *ind
        );
		
		
/****************************************************************************
NAME	
    handlePinRequestRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_PIN_REQUEST_RES message.

RETURNS
	
*/
void handlePinRequestRes(const CL_INTERNAL_SM_PIN_REQUEST_RES_T* res);


/****************************************************************************
NAME	
    handleIoCapabilityRequestRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES message.

RETURNS
	
*/
void handleIoCapabilityRequestRes(
        connectionSmState* smState,
        const CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES_T* res
        );


/****************************************************************************
NAME	
    handleUserConfirmationRequestRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_USER_CONFIRMATION_REQUEST_RES message.

RETURNS
	
*/
void handleUserConfirmationRequestRes(
        const CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES_T* res
        );
		
		
/****************************************************************************
NAME	
    handleUserPasskeyRequestRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_USER_PASSKEY_REQUEST_RES message.

RETURNS
	
*/
void handleUserPasskeyRequestRes(
        const CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES_T* res
        );


/****************************************************************************
NAME	
    handleSendKeypressNotificationReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SEND_KEYPRESS_NOTIFICATION_REQ message.

RETURNS
	
*/
void handleSendKeypressNotificationReq(
        const CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ_T* req
        );


/****************************************************************************
NAME	
    handleSetTrustLevelReq

DESCRIPTION
    This function is called on receipt of an CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ
     message.

RETURNS
	
*/
void handleSetTrustLevelReq(
            connectionSmState *smState,
            const CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ_T* req
            );


/****************************************************************************
NAME	
    handleAuthoriseRes	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_AUTHORISE_RES message.

RETURNS
	
*/
void handleAuthoriseRes(const CL_INTERNAL_SM_AUTHORISE_RES_T* res);


/****************************************************************************
NAME	
    handleAddAuthDeviceReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ to add a device to the trusted device
	list.
	
RETURNS
	
*/
void handleAddAuthDeviceReq(
        connectionSmState *smState,
        const CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ_T *req
        );


/****************************************************************************
NAME	
    handleGetAuthDeviceReq	

DESCRIPTION
    This function is called on receipt of an 
    CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ to get a device from the trusted device
	list.
	
RETURNS
	
*/
void handleGetAuthDeviceReq(
        connectionSmState *smState,
        const CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ_T *req
        );


/***************************************************************************
NAME	
    connectionHandleEncryptionChange

DESCRIPTION
    Hanlde notification from BlueStack of a change in encryption status on a
	particular ACL. This function needs to work out which tasks have a
	sink using this ACL and send a message to each task informing them of the
	change in encryption status.
*/
void connectionHandleEncryptionChange(const DM_SM_ENCRYPTION_CHANGE_IND_T *ind);


/***************************************************************************
NAME	
    connectionSmHandleGetAttributeReq

DESCRIPTION
    Called to request a read of attributes from persistent store
*/
void connectionSmHandleGetAttributeReq(
        Task appTask,
        const CL_INTERNAL_SM_GET_ATTRIBUTE_REQ_T * message
        );


/***************************************************************************
NAME	
    connectionSmHandleGetIndexedAttributeReq

DESCRIPTION
    Called to request a read of attributes from persistent store
*/
void connectionSmHandleGetIndexedAttributeReq(
        Task appTask,
        const CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ_T * message
        );

/***************************************************************************
NAME
    connectionDmHandleAclOpen

DESCRIPTION
    Called on receipt of CL_INTERNAL_DM_ACL_OPEN_REQ to ACL
    attach on device, identified by a specific BD_ADDR.
    See DM_ACL_OPEN_REQ in dm_prim.h.
*/
void connectionDmHandleAclOpen(
        const CL_INTERNAL_DM_ACL_OPEN_REQ_T *req
        );     

/***************************************************************************
NAME    
    connectionDmHandleAclClose

DESCRIPTION
    Called on receipt of CL_INTERNAL_DM_ACL_CLOSE_REQ to close ACL
    with device, identified by a specific BD_ADDR, or to close all
    ACL links depending on the flags set. See DM_ACL_CLOSE_REQ in dm_prim.h.
*/
void connectionDmHandleAclClose(
        const CL_INTERNAL_DM_ACL_CLOSE_REQ_T *req
        );


/***************************************************************************
NAME    
    connectionHandleAPTExpiredInd

DESCRIPTION
    Called on receipt of DM_HCI_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED_IND_T
    to inform the application that an authenticated payload has not been
    received on a link within the current timeout period.
*/
void connectionHandleAPTExpiredInd(
        Task appTask,
        const DM_HCI_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED_IND_T *ind );

/****************************************************************************
NAME    
    connectionHandleReadAPTReq

DESCRIPTION
    Read the current value of the authenticated payload timeout from the controller.
*/
void connectionHandleReadAPTReq(const CL_INTERNAL_DM_READ_APT_REQ_T *req);

/****************************************************************************
NAME	
    connectionHandleReadAPTCfm

DESCRIPTION
    This function handles a read authenticated payload timeout result.
*/
void connectionHandleReadAPTCfm(
        Task appTask, 
        const DM_HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_CFM_T *cfm);

/****************************************************************************
NAME    
    connectionHandleWriteAPTReq

DESCRIPTION
    Set the per-link value of the authenticated payload timeout.
*/
void connectionHandleWriteAPTReq(const CL_INTERNAL_DM_WRITE_APT_REQ_T *req);

/****************************************************************************
NAME	
    connectionHandleWriteAPTCfm

DESCRIPTION
    This function handles a write authenticated payload timeout result.
*/
void connectionHandleWriteAPTCfm(
        Task appTask, 
        const DM_SM_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_CFM_T *cfm);

#endif	/* CONNECTION_DM_SECURITY_HANDLER_H_ */

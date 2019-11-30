/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
*/

#ifndef _SINK_AUTH_PRIVATE_H_
#define _SINK_AUTH_PRIVATE_H_

#include <connection.h>
#ifdef ENABLE_PEER

#include <bdaddr_.h>

#include "sink_devicemanager.h"

/****************************************************************************
DESCRIPTION
    Add devices stored in PSKEY_PERMANENT_PAIRING to Connection library's PDL
*/
void AuthInitPermanentPairing (void);


/****************************************************************************
DESCRIPTION
    Removes permanent paired device from Connection library's PDL.
    Will also erase PSKEY_PERMANENT_PAIRING if erase_ps_key set TRUE.
*/
void AuthRemovePermanentPairing (bool erase_ps_key);


/****************************************************************************
DESCRIPTION
    Use supplied BDADDR to obtain linkkey from Connection library and update
    PSKEY_PERMANENT_PAIRING to retain this as the permanently paired device
*/
void AuthUpdatePermanentPairing (const bdaddr *bd_addr, const sink_attributes *attributes);


/****************************************************************************
DESCRIPTION
    Called in response to CL_SM_GET_AUTH_DEVICE_CFM message, which is generated
    due to calling updatePermanentPairing.
    Both the BDADDR and linkkey contained in CL_SM_GET_AUTH_DEVICE_CFM are used to
    update PSKEY_PERMANENT_PAIRING to retain this as the permanently paired device
*/
void handleGetAuthDeviceCfm (const CL_SM_GET_AUTH_DEVICE_CFM_T *cfm);


/****************************************************************************
DESCRIPTION
    Called in response to CL_SM_ADD_AUTH_DEVICE_CFM message, which is generated
    due to calling ConnectionSmAddAuthDevice.
*/
void handleAddAuthDeviceCfm (const CL_SM_ADD_AUTH_DEVICE_CFM_T *cfm);
#endif

/*************************************************************************
DESCRIPTION
     This function is called on receipt on an CL_PIN_CODE_IND message
     being recieved.  The sink device default pin code is sent back.
*/
void sinkHandlePinCodeInd(const CL_SM_PIN_CODE_IND_T* ind);


/*************************************************************************
DESCRIPTION
     This function is called on receipt on an CL_SM_USER_CONFIRMATION_IND
*/
void sinkHandleUserConfirmationInd(const CL_SM_USER_CONFIRMATION_REQ_IND_T* ind);

/*************************************************************************
DESCRIPTION
     This function is called on receipt on an CL_SM_USER_CONFIRMATION_IND
*/
void sinkHandleUserPasskeyInd(const CL_SM_USER_PASSKEY_REQ_IND_T* ind);


/*************************************************************************
DESCRIPTION
     This function is called on receipt on an CL_SM_USER_PASSKEY_NOTIFICATION_IND
*/
void sinkHandleUserPasskeyNotificationInd(const CL_SM_USER_PASSKEY_NOTIFICATION_IND_T* ind);

/*************************************************************************
DESCRIPTION
     This function is called on receipt on an CL_SM_IO_CAPABILITY_REQ_IND
*/
void sinkHandleIoCapabilityInd(const CL_SM_IO_CAPABILITY_REQ_IND_T* ind);

/*************************************************************************
DESCRIPTION
     This function is called on receipt on an CL_SM_REMOTE_IO_CAPABILITY_IND
*/
void sinkHandleRemoteIoCapabilityInd(const CL_SM_REMOTE_IO_CAPABILITY_IND_T* ind);


/****************************************************************************
DESCRIPTION
    Request to authorise access to a particular service.
*/
void sinkHandleAuthoriseInd(const CL_SM_AUTHORISE_IND_T *ind);


/****************************************************************************
DESCRIPTION
    Indicates whether the authentication succeeded or not.
*/
void sinkHandleAuthenticateCfm(const CL_SM_AUTHENTICATE_CFM_T *cfm);

/****************************************************************************
DESCRIPTION
    Respond correctly to a pairing info request ind (pin code, passkey, etc...)
*/
void sinkPairingAcceptRes( void );

/****************************************************************************
DESCRIPTION
    Respond reject to a pairing info request ind (pin code, passkey, etc...)
*/
void sinkPairingRejectRes( void );

/****************************************************************************
DESCRIPTION
    Helper function to reset the confirmation flags and associated BT address
*/
void AuthResetConfirmationFlags ( void ) ;

#endif /* _SINK_AUTH_PRIVATE_H_ */

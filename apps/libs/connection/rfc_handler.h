/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    rfc_handler.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_RFC_HANDLER_H_
#define	CONNECTION_RFC_HANDLER_H_

#ifndef CL_EXCLUDE_RFCOMM

/* If we don't get an rfc_REGISTER_CFM by the time this expires, assume it failed */
#define RFCOMM_REGISTER_TIMEOUT	    D_SEC(2)


/****************************************************************************
NAME	
	connectionHandleRfcommRegisterReq

DESCRIPTION
    This function handler is called in receipt of a 
    CL_INTERNAL_RFCOMM_REGISTER_REQ from the connection library.

RETURNS
	
*/
void connectionHandleRfcommRegisterReq(const CL_INTERNAL_RFCOMM_REGISTER_REQ_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommRegisterCfm

DESCRIPTION
    This function handler is called in receipt of an RFC_REGISTER_CFM from
    Bluestack

RETURNS
	
*/
void connectionHandleRfcommRegisterCfm(const RFC_REGISTER_CFM_T* cfm);

/****************************************************************************
NAME	
	connectionHandleRfcommUnregisterReq

DESCRIPTION
    This function handler is called in receipt of a 
    CL_INTERNAL_RFCOMM_UNREGISTER_REQ from the connection library.

RETURNS
	
*/
void connectionHandleRfcommUnregisterReq(const CL_INTERNAL_RFCOMM_UNREGISTER_REQ_T* req);

/****************************************************************************
NAME	
	connectionHandleRfcommRegisterCfm

DESCRIPTION
    This function handler is called in receipt of an RFC_REGISTER_CFM from
    Bluestack

RETURNS
	
*/
void connectionHandleRfcommUnregisterCfm(const RFC_UNREGISTER_CFM_T* cfm);


/****************************************************************************
NAME	
	connectionHandleRfcommConnectReq

DESCRIPTION
    This function handler is called in response to a 
    CL_INTERNAL_RFCOMM_CLIENT_CONNECT_REQ message as a result of a client 
    application requesting an RFCOMM connection.

RETURNS
	
*/
void connectionHandleRfcommConnectReq(const CL_INTERNAL_RFCOMM_CONNECT_REQ_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommConnectCfm

DESCRIPTION
    This function handler is called in response to a 
    RFC_CLIENT_CONNECT_CFM or RFC_SERVER_CONNECT_CFM message from BlueStack.

RETURNS
	
*/
void connectionHandleRfcommClientConnectCfm(const RFC_CLIENT_CONNECT_CFM_T* cfm);

/****************************************************************************
NAME	
	connectionHandleRfcommServerConnectCfm

DESCRIPTION
    This function handler is called in response to a 
    RFC_SERVER_CONNECT_CFM message from BlueStack.

RETURNS
	
*/
void connectionHandleRfcommServerConnectCfm(const RFC_SERVER_CONNECT_CFM_T* cfm);


/****************************************************************************
NAME	
	connectionHandleRfcommConnectInd

DESCRIPTION
    This function handler is called in response to a RFC_SERVER_CONNECT_IND
    message from BlueStack.

RETURNS
	
*/

void connectionHandleRfcommConnectInd(const RFC_SERVER_CONNECT_IND_T *ind);

/****************************************************************************
NAME	
	connectionHandleRfcommConnectRes

DESCRIPTION
    This function handler is called in response to a CL_INTERNAL_RFCOMM_CONNECT_RES
    message as a result of a remote device requesting an RFCOMM connection.

RETURNS
	
*/
void connectionHandleRfcommConnectRes(const CL_INTERNAL_RFCOMM_CONNECT_RES_T* res);


/****************************************************************************
NAME	
	connectionHandleRfcommDisconnectReq

DESCRIPTION
    This function handler is called in response to a 
    CL_INTERNAL_RFCOMM_DISCONNECT_REQ  message as a result of the local 
    device requesting the disconnection of the connection identified by the 
    connection sink.

RETURNS
	
*/
void connectionHandleRfcommDisconnectReq(const CL_INTERNAL_RFCOMM_DISCONNECT_REQ_T* req);

/****************************************************************************
NAME	
	connectionHandleRfcommDisconnectCfm

DESCRIPTION
    This function handler is called in response to a RFC_DISCONNECT_CFM.

RETURNS
	
*/
void connectionHandleRfcommDisconnectCfm(const RFC_DISCONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	connectionHandleRfcommDisconnectInd

DESCRIPTION
    This function handler is called in response to a RFC_DISCONNECT_IND, which
    indicates that the remote device has disconnected the RFCOMM DLCI indicated.
    
RETURNS
	
*/
void connectionHandleRfcommDisconnectInd(const RFC_DISCONNECT_IND_T *ind);

/****************************************************************************
NAME	
	connectionHandleRfcommDisconnectRsp

DESCRIPTION
    This function handler is called in response to a 
    CL_INTERNAL_RFCOMM_DISCONNECT_REQ.

RETURNS
	
*/
void connectionHandleRfcommDisconnectRsp(const CL_INTERNAL_RFCOMM_DISCONNECT_RSP_T *rsp);


/****************************************************************************
NAME	
	connectionHandleRfcommPortNegReq

DESCRIPTION
    This function handler is called in response to a 
    CL_INTERNAL_RFCOMM_PORTNEG_REQ message as a result of the client device 
    requesting to send Remote Port Negotiation configuration parameteres to 
    to the peer device.
  
RETURNS
	
*/
void connectionHandleRfcommPortNegReq(const CL_INTERNAL_RFCOMM_PORTNEG_REQ_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommPortNegCfm

DESCRIPTION
    This function handler is called in response to a 
    RFC_PORTNEG_CFM message as a result of the remote device 
    responding to a RFC_PORTNEG_REQ.
  
RETURNS
	
*/
void connectionHandleRfcommPortNegCfm(const RFC_PORTNEG_CFM_T* cfm);

/****************************************************************************
NAME	
	connectionHandleRfcommPortNegInd

DESCRIPTION
    This function handler is called in response to a RFC_PORTNEG_IND
    message as a result of the remote device requesting Port Parmeters or port
    parameter negotiation.
  
RETURNS
	
*/
void connectionHandleRfcommPortNegInd(const RFC_PORTNEG_IND_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommPortNegRsp

DESCRIPTION
    This function handler is called in response to a 
    CL_INTERNAL_RFCOMM_PORTNEG_RSP, as a result of the application receiving
    a Port Negotiation indication from BlueStack.
  
RETURNS
	
*/
void connectionHandleRfcommPortNegRsp(const CL_INTERNAL_RFCOMM_PORTNEG_RSP_T* rsp);


/****************************************************************************
NAME	
	connectionHandleRfcommControlReq

DESCRIPTION
    This function handler is called in response to a CL_INTERNAL_RFCOMM_CONTROL_REQ
    message as a result of the client device requesting to send it's control
    status signals to the peer device
  
RETURNS
	
*/
void connectionHandleRfcommControlReq(const CL_INTERNAL_RFCOMM_CONTROL_REQ_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommControlCfm

DESCRIPTION
    This function handler is called in response to an RFC_MODEM_STATUS_CFM 
    message being sent by Bluestack. 
  
RETURNS
	
*/
void connectionHandleRfcommControlCfm(const RFC_MODEM_STATUS_CFM_T* cfm);

/****************************************************************************
NAME	
	connectionHandleRfcommControlInd

DESCRIPTION
    This function handler is called in response to an RFC_MODEM_STATUS_IND 
    message being sent by Bluestack. 
  
RETURNS
	
*/
void connectionHandleRfcommControlInd(const RFC_MODEM_STATUS_IND_T* ind);



/****************************************************************************
NAME	
	connectionHandleRfcommLineStatusReq

DESCRIPTION
    This function handler is called in response to a
    CL_INTERNAL_RFCOMM_LINES_STATUS_REQ message from a VM task or application.
    This message is sent to indicate a Line Status error to the remote device.
  
RETURNS
	
*/
void connectionHandleRfcommLineStatusReq(const CL_INTERNAL_RFCOMM_LINE_STATUS_REQ_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommLineStatusCfm

DESCRIPTION
    This function handler is called in response to a RFC_LINESTATUS_CFM
    message being sent by Bluestack.  
  
RETURNS
	
*/
void connectionHandleRfcommLineStatusCfm(const RFC_LINESTATUS_CFM_T* cfm);


/****************************************************************************
NAME	
	connectionHandleRfcommLineStatusInd

DESCRIPTION
    This function handler is called in response to an RFC_LINESTATUS_IND 
    message being sent by Bluestack. 
  
RETURNS
	
*/
void connectionHandleRfcommLineStatusInd(const RFC_LINESTATUS_IND_T* ind);

#endif
#endif	/* CONNECTION_RFC_HANDLER_H_ */


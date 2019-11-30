/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    l2cap_handler.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_L2CAP_HANDLER_H_
#define	CONNECTION_L2CAP_HANDLER_H_

#include <app/bluestack/l2cap_prim.h>

#ifndef CL_EXCLUDE_L2CAP

/* If we don't get an L2CAP_REGISTER_CFM by the time this expires, assume it failed */
#define L2CAP_REGISTER_TIMEOUT	(1000)



/****************************************************************************
NAME	
	connectionHandleL2capRegisterReq

DESCRIPTION
	Used by client tasks to register a PSM so remote devices can connect to 
	it. 

RETURNS
	void
*/
void connectionHandleL2capRegisterReq(const CL_INTERNAL_L2CAP_REGISTER_REQ_T *req);


/****************************************************************************
NAME	
	connectionHandleL2capRegisterCfm

DESCRIPTION
	Confirmation that a PSM has been registered with the L2CAP layer of
	BlueStack.

RETURNS
	void
*/
void connectionHandleL2capRegisterCfm(const L2CA_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	connectionHandleL2capUnregisterReq

DESCRIPTION
	Request to unregister a particular psm.

RETURNS
	void
*/
void connectionHandleL2capUnregisterReq(const CL_INTERNAL_L2CAP_UNREGISTER_REQ_T *req);

/****************************************************************************
NAME	
    connectionHandleL2capUnregisterCfm

DESCRIPTION
    Confirmation from Bluestack that the PSM has been un-registered. This is
    passed on to the application.

RETURNS
	void
*/
void connectionHandleL2capUnregisterCfm(const L2CA_UNREGISTER_CFM_T *cfm);

/****************************************************************************
NAME	
	connectionHandleL2capConnectReq

DESCRIPTION
	Request to initiate an L2CAP connection.

RETURNS
	void
*/
void connectionHandleL2capConnectReq(const CL_INTERNAL_L2CAP_CONNECT_REQ_T *req);

/****************************************************************************
NAME	
	connectionHandleL2capTpConnectReq

DESCRIPTION
	Request to initiate an L2CAP LE COC connection.

RETURNS
	void
*/
void connectionHandleL2capTpConnectReq(const CL_INTERNAL_L2CAP_TP_CONNECT_REQ_T *req);

/****************************************************************************
NAME	
	connectionHandleL2capAddCreditReq

DESCRIPTION
	Request to extend credit to remote device.

RETURNS
	void
*/
void connectionHandleL2capAddCreditReq(const CL_INTERNAL_L2CAP_ADD_CREDIT_REQ_T *req);

/****************************************************************************
NAME	
	connectionHandleL2capConnectCfm

DESCRIPTION
	Response to an L2CAP connect request

RETURNS
	void
*/
void connectionHandleL2capConnectCfm(const L2CA_AUTO_CONNECT_CFM_T *cfm);

/****************************************************************************
NAME	
	connectionHandleL2capTpConnectCfm

DESCRIPTION
	Response to an L2CAP LE COC connect request

RETURNS
	void
*/
void connectionHandleL2capTpConnectCfm(const L2CA_AUTO_TP_CONNECT_CFM_T *cfm);

/****************************************************************************
NAME	
	connectionHandleL2capAddCreditCfm

DESCRIPTION
	Response to an L2CAP LE COC add credit request

RETURNS
	void
*/
void connectionHandleL2capAddCreditCfm(const L2CA_ADD_CREDIT_CFM_T *cfm);

/****************************************************************************
NAME	
	connectionHandleL2capConnectInd

DESCRIPTION
	Indication that the remove device is trying to connect to this device.

RETURNS
	void
*/
void connectionHandleL2capConnectInd(const L2CA_AUTO_CONNECT_IND_T *ind);

/****************************************************************************
NAME	
	connectionHandleL2capTpConnectInd

DESCRIPTION
	Indication that the remove device is trying to connect to this device.

RETURNS
	void
*/
void connectionHandleL2capTpConnectInd(const L2CA_AUTO_TP_CONNECT_IND_T *ind);

/****************************************************************************
NAME	
	connectionHandleL2capConnectRes

DESCRIPTION
	Handle a response from the client task telling us whether to proceed
	with establishing the L2CAP connection. 

RETURNS
	void
*/
void connectionHandleL2capConnectRes(const CL_INTERNAL_L2CAP_CONNECT_RES_T *res);

/****************************************************************************
NAME	
	connectionHandleL2capTpConnectRes

DESCRIPTION
	Handle a response from the client task telling us whether to proceed
	with establishing the L2CAP LE COC connection. 

RETURNS
	void
*/
void connectionHandleL2capTpConnectRes(const CL_INTERNAL_L2CAP_TP_CONNECT_RES_T *res);

/****************************************************************************
NAME	
	connectionHandleL2capDisconnectCfm

DESCRIPTION
	L2CAP connection has been disconnected by the local end.

RETURNS
	void
*/
void connectionHandleL2capDisconnectCfm(const L2CA_DISCONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	connectionHandleL2capDisconnectInd

DESCRIPTION
	L2CAP connection has been disconnected by the remote end.

RETURNS
	void
*/
void connectionHandleL2capDisconnectInd(const L2CA_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME	
	connectionHandleL2capDisconnectReq

DESCRIPTION
	Request by the local device to disconnect the L2CAP connection

RETURNS
	void
*/
void connectionHandleL2capDisconnectReq(const CL_INTERNAL_L2CAP_DISCONNECT_REQ_T *req);


/****************************************************************************
NAME	
	connectionHandleQosSetupCfm

DESCRIPTION
	Confirmation of QOS parameters

RETURNS
	void
*/
void connectionHandleQosSetupCfm(const DM_HCI_QOS_SETUP_CFM_T* cfm);


/****************************************************************************
NAME	
	connectionHandleL2capDisconnectRes

DESCRIPTION
	The client has read the CL_L2CAP_DISCONNECT_IND message so it is
    now safe to inform the remote device.

RETURNS
	void
*/
void connectionHandleL2capDisconnectRes(const CL_INTERNAL_L2CAP_DISCONNECT_RSP_T* msg);


/****************************************************************************
NAME    
    connectionHandleL2capTimeoutInd

DESCRIPTION
    Time out can occur during connection or disconnection transactions. 

RETURNS
    void
*/
void connectionHandleL2capTimeout(const L2CA_TIMEOUT_IND_T *ind);


/****************************************************************************
NAME    
    connectionHandleL2capMapConnectionlessReq

DESCRIPTION
    Map connectionless data to a remote device; Broadcast or Unicast. For 
    Unicast, nail up the ACL, but don't for Broadcast.

RETURNS
    void
*/
void connectionHandleL2capMapConnectionlessReq(connectionL2capState *state, const CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ_T* req);

/****************************************************************************
NAME    
    connectionHandleL2capGetInfoCfm

DESCRIPTION
    Part of the Map Connectionless Data message scenario. Response indicating
    if the remote device supports Connectionless Data.

RETURNS
    void
*/
void connectionHandleL2capGetInfoCfm(connectionL2capState *state, const L2CA_GETINFO_CFM_T* cfm);

/****************************************************************************
NAME    
    connectionHandleL2capMapFixedCidCfm

DESCRIPTION
    Handle the reception of the L2CA_MAP_FIXED_CID_CFM primitive.

RETURNS
    void
*/
void connectionHandleL2capMapFixedCidCfm(connectionL2capState *state, const L2CA_MAP_FIXED_CID_CFM_T* cfm);

/****************************************************************************
NAME    
    connectionHandleL2capMapFixedCidInd

DESCRIPTION
    Handle the reception of the L2CA_MAP_FIXED_CID_IND primitive. T

RETURNS
    void
*/
void connectionHandleL2capMapFixedCidInd(const L2CA_MAP_FIXED_CID_IND_T* ind);

/****************************************************************************
NAME    
    connectionHandleL2capMapConnectionlessRsp

DESCRIPTION
    Send the L2CA_MAP_FIXED_CID_RES, setting the app task for the scenario.

RETURNS
    void
*/
void connectionHandleL2capMapConnectionlessRsp(const CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_RES_T *res);

/****************************************************************************
NAME    
    connectionHandleL2capUnmapConnectionlessReq

DESCRIPTION
    Send the L2CA_UNMAP_FIXED_CID_REQ, for the Connectionless data.

RETURNS
    void
*/
void connectionHandleL2capUnmapConnectionlessReq(const CL_INTERNAL_L2CAP_UNMAP_CONNECTIONLESS_REQ_T *req);

/****************************************************************************
NAME    
    connectionHandleL2capUnmapFixedCidInd

DESCRIPTION
    Handle the reception of the L2CA_MAP_FIXED_CID_IND primitive. T

RETURNS
    void
*/
void connectionHandleL2capUnmapFixedCidInd(const L2CA_UNMAP_FIXED_CID_IND_T* ind);

#endif

#endif	/* CONNECTION_L2CAP_HANDLER_H_ */

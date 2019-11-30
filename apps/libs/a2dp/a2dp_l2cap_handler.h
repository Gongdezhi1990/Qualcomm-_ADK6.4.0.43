/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_l2cap_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_L2CAP_HANDLER_H_
#define A2DP_L2CAP_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME
	

DESCRIPTION
    Register AVDTP with L2CAP. 

*/
void a2dpRegisterL2cap(void);


/****************************************************************************
NAME
	a2dpHandleL2capRegisterCfm

DESCRIPTION
    Handle the CL_L2CAP_REGISTER_CFM message sent from the connection library
    as a result of calling the ConnectionL2capRegisterRequestLazy API.
*/
void a2dpHandleL2capRegisterCfm(const CL_L2CAP_REGISTER_CFM_T *cfm);
void a2dpHandleL2capLinklossTimeout(MessageId msg_id);

remote_device * a2dpFindDeviceFromSink (Sink sink);
bool a2dpL2capIsSignallingConnected(signalling_channel *signalling);

void a2dpHandleSdpServiceSearchAttributeCfm (const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm);
void a2dpHandleL2capConnectReq(const A2DP_INTERNAL_L2CAP_CONNECT_REQ_T *req);
void a2dpHandleSignallingConnectReq(const A2DP_INTERNAL_SIGNALLING_CONNECT_REQ_T *req);
void a2dpHandleSignallingConnectRes(const A2DP_INTERNAL_SIGNALLING_CONNECT_RES_T *res);
void a2dpMediaConnectReq(remote_device *device);
void a2dpHandleSignallingDisconnectReq(const A2DP_INTERNAL_SIGNALLING_DISCONNECT_REQ_T *req);
void a2dpMediaDisconnectReq(Sink sink);
bool a2dpSignallingDisconnectPending (remote_device *device);
bool a2dpDisconnectAllMedia(remote_device *device);

/****************************************************************************
NAME	
	a2dpHandleL2capConnectInd

DESCRIPTION
	This function is called on receipt of an CL_L2CAP_CONNECT_IND
	indicating that an L2CAP connection has been requested by a remote device.

*/
void a2dpHandleL2capConnectInd(const CL_L2CAP_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	a2dpHandleL2capConnectCfm

DESCRIPTION
	This function is called on receipt of an CL_L2CAP_CONNECT_CFM
	indicating that an L2CAP connection attempt has been completed.

*/
void a2dpHandleL2capConnectCfm(const CL_L2CAP_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	a2dpHandleL2capDisconnectInd
	

DESCRIPTION
	This function is called on receipt of an CL_L2CAP_DISCONNECT_IND
	indicating that a previously connected L2CAP connection has been
	disconnected.

*/
void a2dpHandleL2capDisconnect(Sink sink, l2cap_disconnect_status status);


#endif /* A2DP_L2CAP_HANDLER_H_ */

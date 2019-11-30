/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#ifndef AGHFP_RFC_H_
#define AGHFP_RFC_H_


/****************************************************************************
	Request an RFCOMM channel to be allocated.
*/
void aghfpHandleRfcommAllocateChannel(AGHFP *aghfp);


/****************************************************************************
	Rfcomm channel has been allocated.
*/
void aghfpHandleRfcommRegisterCfm(AGHFP *aghfp, const CL_RFCOMM_REGISTER_CFM_T *cfm);


/****************************************************************************
	Issue a request to the connection lib to create an RFCOMM connection.
*/
void aghfpHandleRfcommConnectRequest(AGHFP *aghfp, const AGHFP_INTERNAL_RFCOMM_CONNECT_REQ_T *req);


/****************************************************************************
	Response to an incoming RFCOMM connect request.
*/
void aghfpHandleRfcommConnectResponse(AGHFP *aghfp, bool response, Sink sink, uint8 server_channel, const rfcomm_config_params *config);


/****************************************************************************
 Outcome of the RFCOMM connect request.
*/
void aghfpHandleRfcommClientConnectCfm(AGHFP *aghfp, CL_RFCOMM_CLIENT_CONNECT_CFM_T* cfm);


/****************************************************************************
 Outcome of the RFCOMM connect response.
*/
void aghfpHandleRfcommServerConnectCfm(AGHFP *aghfp, CL_RFCOMM_SERVER_CONNECT_CFM_T* cfm);


/****************************************************************************
	Notification of an incoming rfcomm connection, pass this up to the app
	to decide whather to accept this or not.
*/
void aghfpHandleRfcommConnectInd(AGHFP *aghfp, const CL_RFCOMM_CONNECT_IND_T *ind);


/****************************************************************************
	Issue an RFCOMM disconnect to the connection lib.
*/
void aghfpHandleRfcommDisconnectRequest(AGHFP *aghfp);


/****************************************************************************
    Confirmation that the RFCOMM connection has been disconnected.
*/
void aghfpHandleRfcommDisconnectCfm(AGHFP *aghfp, const CL_RFCOMM_DISCONNECT_CFM_T *cfm);


/****************************************************************************
	Indication that the RFCOMM connection has been disconnected.
*/
void aghfpHandleRfcommDisconnectInd(AGHFP *aghfp, const CL_RFCOMM_DISCONNECT_IND_T *ind);


/****************************************************************************
	Indication of port negotiation request.
*/
void aghfpHandleRfcommPortnegInd(AGHFP *aghfp, CL_RFCOMM_PORTNEG_IND_T* ind);

#endif /* AGHFP_RFC_H_ */

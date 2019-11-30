/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#ifndef AGHFP_SLC_HANDLER_H_
#define AGHFP_SLC_HANDLER_H_


#include "aghfp_parse.h"
#include "aghfp_private.h"


/****************************************************************************
	Initiate the creation of a profile service level connection.
*/
void aghfpHandleSlcConnectRequest(AGHFP *aghfp, const AGHFP_INTERNAL_SLC_CONNECT_REQ_T *req);


/****************************************************************************
	Respond to a request to create an SLC from the remote device.
*/
void aghfpHandleSlcConnectResponse(AGHFP *aghfp, const AGHFP_INTERNAL_SLC_CONNECT_RES_T *res);


/****************************************************************************
	Reject the connect request outright because this profile instance is not 
	in the correct state.
*/
void aghfpHandleSlcConnectIndReject(AGHFP *aghfp, const CL_RFCOMM_CONNECT_IND_T *ind);


/****************************************************************************
	Send a HFP_SLC_CONNECT_CFM message to the app telling it the outcome
	of the connect attempt.
*/
void aghfpSendSlcConnectCfmToApp(aghfp_connect_status status, AGHFP *aghfp);


/****************************************************************************
	Record the supported features that the HF has just reported and send our
    supported features in response.
*/
void aghfpHandleBrsfRequest(AGHFP *aghfp, uint16 supported_features);


/****************************************************************************
	Respond to AT+CIND=? command.
*/
void aghfpHandleCindSupportedRequest(AGHFP *aghfp);


/****************************************************************************
	Respond to AT+CIND? command.
*/
void aghfpHandleCindStatusRequest(AGHFP *aghfp);


/****************************************************************************
	Respond to AT+CMER command.
*/
void aghfpHandleCmerRequest(AGHFP *aghfp, uint16 mode, uint16 ind);


/****************************************************************************
	Respond to AT+CHLD=? command.
*/
void aghfpHandleCallHoldSupportRequest(AGHFP *aghfp);


/****************************************************************************
	Called in response to AghfpSlcDisconnect if we currently have an SLC
    connection.
*/
void aghfpHandleSlcDisconnect(AGHFP *aghfp);


/****************************************************************************
	Send an AGHFP_SLC_DISCONNECT_IND message to the app notifying it that
	the SLC has been disconnected.
*/
void aghfpSendSlcDisconnectIndToApp(AGHFP *aghfp, aghfp_disconnect_status status);


/****************************************************************************
	Received a disconnect request from the app when we haven't got a 
	connection and are not attempting to connect. Return error message.
*/
void aghfpHandleDisconnectRequestFail(AGHFP *aghfp);

/****************************************************************************
	Respond to AT+BIND=? command.
*/
void aghfpHandleBindSupportedRequest(AGHFP *aghfp);

/****************************************************************************
	Respond to AT+BIND? command.
*/
void aghfpHandleBindIndicatorStatusRequest(AGHFP *aghfp);

#endif /* AGHFP_SLC_HANDLER_H_ */

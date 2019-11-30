/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    aghfp_audio_handler.h
    
DESCRIPTION
	
*/

#ifndef AGHFP_AUDIO_HANDLER_H_
#define AGHFP_AUDIO_HANDLER_H_


#include "aghfp.h"
#include "aghfp_private.h"


/****************************************************************************
NAME	
	aghfpGetDefaultAudioParams

DESCRIPTION
	Returns the most appropriate default audio parameters dependant on the
	specified packet type.
	
RETURNS
	Pointer to audio parameters.
*/
const aghfp_audio_params * aghfpGetDefaultAudioParams (sync_pkt_type packet_type);


/****************************************************************************
NAME	
	aghfpStoreAudioParams

DESCRIPTION
	Checks that the specified parameters are valid for the packet type being requested and stores
	the information for later use.
	
RETURNS
	TRUE on success, FALSE otherwise.
*/
bool aghfpStoreAudioParams(AGHFP *aghfp, sync_pkt_type packet_type, const aghfp_audio_params *audio_params);


/****************************************************************************
NAME	
	aghfpAudioConnectRequest

DESCRIPTION
	Kicks off an audio (synchronous) connection attempt with previously stored parameters,
	via aghfpStoreAudioParams.
	
RETURNS
	TRUE if connection request is issued, FALSE otherwise.
*/
bool aghfpAudioConnectRequest(AGHFP *aghfp);


/****************************************************************************
NAME	
	aghfpAudioDisconnectRequest

DESCRIPTION
	Kicks off an audio (synchronous) disconnection attempt.
	
RETURNS
	TRUE if disconnection request is issued, FALSE otherwise.
*/
bool aghfpAudioDisconnectRequest(AGHFP *aghfp);


/****************************************************************************
NAME	
	aghfpHandleSyncConnectInd

DESCRIPTION
	Incoming Synchronous connect notification, accept if we recognise the sink 
	reject otherwise.

RETURNS
	void
*/
void aghfpHandleSyncConnectInd(AGHFP *aghfp, const CL_DM_SYNC_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	aghfpHandleSyncConnectIndReject

DESCRIPTION
	Incoming Synchronous connect notification, reject outright, profile is in 
	the wrong state.  This is probably a synchronous connect indication for a
	different task.

RETURNS
	void
*/
void aghfpHandleSyncConnectIndReject(AGHFP *aghfp, const CL_DM_SYNC_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	aghfpHandleSyncConnectCfm

DESCRIPTION
	Confirmation in response to a Synchronous connect request indicating the 
	outcome of the connect attempt.

RETURNS
	void
*/
void aghfpHandleSyncConnectCfm(AGHFP *aghfp, const CL_DM_SYNC_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	aghfpHandleAudioDisconnectReq

DESCRIPTION
	Attempt to disconnect the Synchronous connection.

RETURNS
	void
*/
void aghfpHandleAudioDisconnectReq(AGHFP *aghfp);


/****************************************************************************
NAME	
	aghfpHandleAudioDisconnectReqError

DESCRIPTION
    Attempt has been made to disconnect an audio connection request.  However,
    AGHFP library is not in the correct state to process the request.

RETURNS
	void
*/
void aghfpHandleAudioDisconnectReqError(AGHFP *aghfp);


/****************************************************************************
NAME	
	aghfpHandleSyncDisconnectInd

DESCRIPTION
	The Synchronous connection has been disconnected 

RETURNS
	void
*/
void aghfpHandleSyncDisconnectInd(AGHFP *aghfp, const CL_DM_SYNC_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME	
	aghfpHandleAudioConnectReq

DESCRIPTION
	Transfer the audio from the AG to the HF or vice versa depending on which
	device currently has it.

RETURNS
	void
*/
void aghfpHandleAudioConnectReq(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_CONNECT_REQ_T *req);


/****************************************************************************
NAME	
	aghfpHandleAudioConnectReqError

DESCRIPTION
	The app has requested that the audio be transferred (either to or from the AG).
	If the profile instance was in the wrong state for this operation to be 
	performed we need to send an immediate response to the app indicating an
	error has ocurred. We send the a SCO status message with the status code 
	set to hfp_fail. We determine which message to send by looking at the action
	the app has requested and the current state of the AGHFP instance.

RETURNS
	void
*/
void aghfpHandleAudioConnectReqError(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_CONNECT_REQ_T *req);


/****************************************************************************
NAME	
	aghfpHandleAudioConnectRes

DESCRIPTION
    Accept or reject to an incoming audio connection request from remote device.

RETURNS
	void
*/
void aghfpHandleAudioConnectRes(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_CONNECT_RES_T *res);


/****************************************************************************
NAME	
	aghfpHandleAudioConnectResError

DESCRIPTION
    Attempt has been made to accept/reject an incoming audio connection request.  However,
    AGHFP library is not in the correct state to process the response.

RETURNS
	void
*/
void aghfpHandleAudioConnectResError(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_CONNECT_RES_T *res);


/****************************************************************************
NAME	
	aghfpHandleAudioTransferReq

DESCRIPTION
	Transfer the audio from the AG to the HF or vice versa depending on which
	device currently has it.

RETURNS
	void
*/
void aghfpHandleAudioTransferReq(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req);


/****************************************************************************
NAME	
	aghfpHandleAudioTransferReqError

DESCRIPTION
	The app has requested that the audio be transferred (either to or from the AG).
	If the profile instance was in the wrong state for this operation to be 
	performed we need to send an immediate response to the app indicating an
	error has ocurred. We send the a audio status message with the status code 
	set to hfp_fail. We determine which message to send by looking at the action
	the app has requested and the current state of the AGHFP instance.

RETURNS
	void
*/
void aghfpHandleAudioTransferReqError(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req);

/* Inform the app of the status of the audio (Synchronous) connection */
void sendAudioConnectCfmToApp(AGHFP *aghfp, aghfp_audio_connect_status status, hci_status cl_status);

#endif /* AGHFP_AUDIO_HANDLER_H_ */

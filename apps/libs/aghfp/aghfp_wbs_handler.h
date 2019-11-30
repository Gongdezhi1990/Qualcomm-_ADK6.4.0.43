/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    aghfp_wbs_handler.h
    
DESCRIPTION
	
*/

#ifndef AGHFP_WBS_HANDLER_H_
#define AGHFP_WBS_HANDLER_H_

#include "aghfp_private.h"
#include "aghfp_wbs.h"

/* Including each EDR eSCO bit disables the options.
   This bitmap therefore enables EV3 and 2EV3 */
#define WBS_PACKET_TYPE (sync_ev3 | sync_3ev5 | sync_2ev5 | sync_3ev3 /* | sync_2ev3 */)

/*
	Start Codec negotiation with HF
*/
bool aghfpWbsStartCodecNegotiation(AGHFP *aghfp, aghfp_wbs_negotiate_action wbs_negotiate_action);

/*
	Handle Codec Connection request from the HF (AT+BCC).
	Ask the app for its configured audio parameters.
*/
void aghfpHandleWbsCodecConReq(AGHFP *aghfp);

/*
	Handle Codec Negotiation response from HF
*/
void aghfpHandleWbsCodecNegReq(AGHFP *aghfp, aghfp_wbs_codec codec_id);

/*
	Get pointer to WB-Speech eSCO Parameters
*/
aghfp_audio_params *aghfpGetWbsParameters(AGHFP *aghfp);

/*
	Get WB-Speech packet type
*/
sync_pkt_type aghfpGetWbsPacketType(AGHFP *aghfp);

/*
	Send WB-Speech SCO Handle
*/
void aghfpWbsStartScoHandleSend(AGHFP *aghfp, uint8 handle);

/*
	Handle Codec Negotiation
*/
void aghfpHandleCodecNegotiationReq(AGHFP *aghfp, AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ_T *codecs);

/*
	Handle set audio parameters request from the app.
*/
void aghfpHandleSetAudioParamsReq(AGHFP *aghfp, const AGHFP_INTERNAL_SET_AUDIO_PARAMS_REQ_T *req);

/*
	Switch on WB-Speech in the AGHFP
*/
void aghfpEnableWbs(AGHFP *aghfp);

/*
	Switch off WB-Speech in the AGHFP
*/
void aghfpDisableWbs(AGHFP *aghfp);

#endif /* AGHFP_WBS_HANDLER_H_ */


/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#include "aghfp.h"
#include "aghfp_audio_handler.h"
#include "aghfp_call.h"
#include "aghfp_common.h"
#include "aghfp_indicators.h"
#include "aghfp_hf_indicators.h"
#include "aghfp_init.h"
#include "aghfp_misc_handler.h"
#include "aghfp_ok.h"
#include "aghfp_private.h"
#include "aghfp_profile_handler.h"
#include "aghfp_receive_data.h"
#include "aghfp_rfc.h"
#include "aghfp_sdp.h"
#include "aghfp_slc_handler.h"
#include "aghfp_user_data_handler.h"
#include "aghfp_call_manager.h"

#include "aghfp_wbs_handler.h"
#include "aghfp_qce_handler.h"

#include <stdio.h>
#include <panic.h>
#include <print.h>


typedef enum
{
	aghfpUnexpectedClPrim,
	aghfpUnexpectedAghfpPrim,
	aghfpUnexpectedMessage,
	aghfpUnexpectedWbsPrim
} aghfpUnexpectedReasonCode;


/****************************************************************************
 This function is called as a result of a message arriving when this
 library was not expecting it.
*/
static void handleUnexpected(aghfpUnexpectedReasonCode code, aghfp_state state, uint16 type)
{
	state = state;
	type = type;
	code = code;

    AGHFP_DEBUG_PANIC(("aghfp handleUnexpected - Code 0x%x State 0x%x MsgId 0x%x\n", code, state, type));
}


static void handleAghfpMessages(Task task, MessageId id, Message message)
{
	AGHFP *aghfp = (AGHFP *) task;
	aghfp_state profileState = aghfp->state;

	switch(id)
	{
		/*************************************************************************/
		/* Init messages                                                         */
		/*************************************************************************/



		case AGHFP_INTERNAL_INIT_REQ:
			PRINT(("AGHFP_INTERNAL_INIT_REQ\n"));

			switch(profileState)
			{
				case aghfp_initialising:
					aghfpHandleInternalInitReq(aghfp);
					break;
				default:
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;

		case AGHFP_INTERNAL_INIT_CFM:
			PRINT(("AGHFP_INTERNAL_INIT_CFM\n"));

			switch(profileState)
			{
				case aghfp_initialising:
					aghfpHandleInternalInitCfm(aghfp, (AGHFP_INTERNAL_INIT_CFM_T *) message);
					break;
				default:
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;

		/*************************************************************************/
		/* SDP Messages                                                          */
		/*************************************************************************/

		case AGHFP_INTERNAL_SDP_REGISTER_CFM:
			PRINT(("AGHFP_INTERNAL_SDP_REGISTER_CFM\n"));

			switch(profileState)
			{
				case aghfp_initialising:
					aghfpHandleSdpInternalRegisterInit(aghfp, (AGHFP_INTERNAL_SDP_REGISTER_CFM_T *) message);
					break;
				case aghfp_ready:
					aghfpHandleSdpInternalRegisterCfm((AGHFP_INTERNAL_SDP_REGISTER_CFM_T *) message);
					break;
				default:
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;

		/*************************************************************************/
		/* SLC Messages                                                          */
		/*************************************************************************/

		case AGHFP_INTERNAL_SLC_CONNECT_REQ:
			PRINT(("AGHFP_INTERNAL_SLC_CONNECT_REQ\n"));

			switch(profileState)
			{
				case aghfp_ready:
					aghfpHandleSlcConnectRequest(aghfp, (AGHFP_INTERNAL_SLC_CONNECT_REQ_T *) message);
					break;

				case aghfp_slc_connecting:
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                case aghfp_held_call:
					/*  If this profile instance is already connecting/connected then
					reject the SLC connect request and inform the app. */
					aghfpSendSlcConnectCfmToApp(aghfp_connect_failed_busy, aghfp);
					break;

				default:
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;

		case AGHFP_INTERNAL_SLC_CONNECT_RES:
			PRINT(("AGHFP_INTERNAL_SLC_CONNECT_RES\n"));

			switch(profileState)
			{
				case aghfp_slc_connecting:
					aghfpHandleSlcConnectResponse(aghfp, (AGHFP_INTERNAL_SLC_CONNECT_RES_T *) message);
					break;

				case aghfp_ready:
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                case aghfp_held_call:
					/*  If the app is sending us an SLC connect response when we're not
					connecting then send it an error message since we currently don't
					have an SLC being established. */
					aghfpSendSlcConnectCfmToApp(aghfp_connect_failed_busy, aghfp);
					break;

				default:
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;

		case AGHFP_INTERNAL_RFCOMM_CONNECT_REQ:
			PRINT(("AGHFP_INTERNAL_RFCOMM_CONNECT_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connecting:
					aghfpHandleRfcommConnectRequest(aghfp, (AGHFP_INTERNAL_RFCOMM_CONNECT_REQ_T *) message);
					break;
				default:
					/* Something wrong, this message is sent internally from the AGHFP */
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;

		case AGHFP_INTERNAL_SLC_DISCONNECT_REQ:
			PRINT(("AGHFP_INTERNAL_SLC_DISCONNECT_REQ"));

			switch(profileState)
			{
				case aghfp_slc_connecting:
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                case aghfp_active_call:	
                case aghfp_held_call:
					aghfpHandleSlcDisconnect(aghfp);
					break;
				case aghfp_ready:
					/* Send disconnect message with error - nothing to disconnect */
					aghfpHandleDisconnectRequestFail(aghfp);
					break;
                case aghfp_initialising:
				default:
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;


		/*************************************************************************/
		/* AT Messages                                                           */
		/*************************************************************************/

		case AGHFP_INTERNAL_AT_CKPD_CMD:
			PRINT(("AGHFP_INTERNAL_AT_CKPD_CMD\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleKeypadCommand(aghfp, ((AGHFP_INTERNAL_AT_CKPD_CMD_T*)message)->keycode);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;
			
		case AGHFP_INTERNAL_AT_BRSF_REQ:
			PRINT(("AGHFP_INTERNAL_AT_BRSF_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connecting:
				{
					uint16 hf_supported_features = ((AGHFP_INTERNAL_AT_BRSF_REQ_T *)message)->hf_supported_features;

					/* Only allowed if we are an HFP device */
					if (!supportedProfileIsHfp(aghfp->supported_profile)) Panic();

					aghfpHandleBrsfRequest(aghfp, hf_supported_features);
					break;
				}
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_AT_CIND_SUPPORTED_REQ:
			PRINT(("AGHFP_INTERNAL_AT_CIND_SUPPORTED_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connecting:
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					/* Only allowed if we are an HFP device */
					if (!supportedProfileIsHfp(aghfp->supported_profile)) Panic();
					aghfpHandleCindSupportedRequest(aghfp);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_AT_CIND_STATUS_REQ:
			PRINT(("AGHFP_INTERNAL_AT_CIND_STATUS_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connecting:
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					/* Only allowed if we are an HFP device */
					if (!supportedProfileIsHfp(aghfp->supported_profile)) Panic();
					aghfpHandleCindStatusRequest(aghfp);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_AT_CMER_REQ:
			PRINT(("AGHFP_INTERNAL_AT_CMER_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connecting:
                case aghfp_slc_connected:
				{
					AGHFP_INTERNAL_AT_CMER_REQ_T *data = (AGHFP_INTERNAL_AT_CMER_REQ_T *)message;

					/* Only allowed if we are an HFP device */
					if (!supportedProfileIsHfp(aghfp->supported_profile)) Panic();

					aghfpHandleCmerRequest(aghfp, data->mode, data->ind);
					break;
				}
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_CALL_HOLD_SUPPORT_REQ: /* HF sent AT+CHLD=? query */
			PRINT(("AGHFP_INTERNAL_CALL_HOLD_SUPPORT_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connecting:
					/* Only allowed if we are an HFP device */
					if (!supportedProfileIsHfp(aghfp->supported_profile)) Panic();
					aghfpHandleCallHoldSupportRequest(aghfp);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_CALL_HOLD_REQ: /* HF sent AT+CHLD=<action>[<idx>] instruction */
		{
    		AGHFP_INTERNAL_CALL_HOLD_REQ_T *msg = (AGHFP_INTERNAL_CALL_HOLD_REQ_T *)message;
			PRINT(("AGHFP_INTERNAL_CALL_HOLD_REQ\n"));

			switch(profileState)
			{
				case aghfp_active_call:
					aghfpHandleCallHoldReq(aghfp, msg->action, msg->index);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}

			break;
		}

		case AGHFP_INTERNAL_ANSWER_REQ: /* HF wants to answer the call */
			PRINT(("AGHFP_INTERNAL_ANSWER_REQ\n"));

            if ( aghfpCallManagerActive(aghfp) )
			{
                switch(profileState)
                {
                    case aghfp_incoming_call_establish:
                    case aghfp_held_call:
                        aghfpSendOk(aghfp);
						aghfpManageCall(aghfp, CallEventAnswer, CallFlagOpenAudio);
                        break;
                    default:
                        aghfpSendError(aghfp);
                        break;
				}
			}
            else
			{
                switch(profileState)
                {
                    case aghfp_slc_connecting:
                    case aghfp_slc_connected:
                    case aghfp_incoming_call_establish:
                    case aghfp_outgoing_call_establish:
                    case aghfp_active_call:
                    case aghfp_held_call:
                        aghfpHandleAnswer(aghfp);
                        break;
                    default:
                        aghfpSendError(aghfp);
                        break;
                }
			}
			break;

		case AGHFP_INTERNAL_CALLER_ID_SETUP_REQ: /* HF wants to enable/disable caller ID notification */
			PRINT(("AGHFP_INTERNAL_CALLER_ID_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleCallerIdSetupReq(aghfp, ((AGHFP_INTERNAL_CALLER_ID_SETUP_REQ_T*)message)->state);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_CALL_WAITING_SETUP_REQ: /* HF wants to enable/disable call waiting notification */
			PRINT(("AGHFP_INTERNAL_CALL_WAITING_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleCallWaitingSetupReq(aghfp, ((AGHFP_INTERNAL_CALL_WAITING_SETUP_REQ_T*)message)->state);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_CALL_HANG_UP_REQ: /* HF wants to reject incoming call */
			PRINT(("AGHFP_INTERNAL_CALL_HANG_UP_REQ\n"));

			if ( aghfpCallManagerActive(aghfp) )
			{
                switch(profileState)
                {
                    case aghfp_incoming_call_establish:
                    case aghfp_outgoing_call_establish:
                    case aghfp_active_call:
                    case aghfp_held_call:
                        aghfpSendOk(aghfp);
                        aghfpManageCall(aghfp, CallEventTerminate, CallFlagCloseAudio);
                        break;
                    default:
                        aghfpSendError(aghfp);
                        break;
                }
			}
			else
			{
                switch(profileState)
                {
                    case aghfp_slc_connecting:
                    case aghfp_slc_connected:
                    case aghfp_incoming_call_establish:
                    case aghfp_outgoing_call_establish:
                    case aghfp_active_call:
                    case aghfp_held_call:
                        aghfpHandleCallHangUpReq(aghfp);
                        break;
                    default:
                        aghfpSendError(aghfp);
                        break;
                }
			}
			break;

		case AGHFP_INTERNAL_DIAL_REQ: /* HF wants to dial a number */
			PRINT (("AGHFP_INTERNAL_DIAL_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_active_call:
				{
					AGHFP_INTERNAL_DIAL_REQ_T *data = (AGHFP_INTERNAL_DIAL_REQ_T*)message;
					aghfpHandleDialReq(aghfp, data->number, data->size_number);
					break;
				}
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_MEMORY_DIAL_REQ: /* HF wants to dial a number from the AG's phonebook */
			PRINT (("AGHFP_INTERNAL_MEMORY_DIAL_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				{
					AGHFP_INTERNAL_MEMORY_DIAL_REQ_T *data = (AGHFP_INTERNAL_MEMORY_DIAL_REQ_T*)message;
					aghfpHandleMemoryDialReq(aghfp, data->number, data->size_number);
					break;
				}
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_LAST_NUMBER_REDIAL_REQ: /* HF wants to redial last number dialed by AG */
			PRINT (("AGHFP_INTERNAL_LAST_NUMBER_REDIAL_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
					aghfpHandleLastNumberRedialReq(aghfp);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_NREC_SETUP_REQ:
			PRINT (("AGHFP_INTERNAL_NREC_SETUP_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleNrecSetupReq(aghfp, ((AGHFP_INTERNAL_NREC_SETUP_REQ_T*)message)->state);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;

		case AGHFP_INTERNAL_VOICE_RECOGNITION_SETUP_REQ:
			PRINT (("AGHFP_INTERNAL_VOICE_RECOGNITION_SETUP_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleVoiceRecognitionSetupReq(aghfp, ((AGHFP_INTERNAL_VOICE_RECOGNITION_SETUP_REQ_T*)message)->state);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;


		case AGHFP_INTERNAL_PHONE_NUMBER_REQ:
			PRINT (("AGHFP_INTERNAL_PHONE_NUMBER_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandlePhoneNumberForVoiceTagReq(aghfp);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;


		case AGHFP_INTERNAL_TRANSMIT_DTMF_CODE:
		{
			AGHFP_INTERNAL_TRANSMIT_DTMF_CODE_T *data = (AGHFP_INTERNAL_TRANSMIT_DTMF_CODE_T*)message;
			PRINT (("AGHFP_INTERNAL_TRANSMIT_DTMF_CODE\n"));

			switch(profileState)
			{
                case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleTransmitDtmfCode(aghfp, data->code);
					break;
				default:
					aghfpSendError(aghfp);
					break;
			}
			break;
		}

        case AGHFP_INTERNAL_RESPONSE_HOLD_STATUS_REQUEST_REQ:    /* AT+BTRH?         */
            PRINT (("AGHFP_INTERNAL_RESPONSE_HOLD_STATUS_REQUEST_REQ\n"));
        	switch(profileState)
        	{
            	case aghfp_slc_connected:
            	case aghfp_incoming_call_establish:
            	case aghfp_outgoing_call_establish:
            	case aghfp_active_call:
                case aghfp_held_call:
            	    aghfpHandleResponseHoldStatusRequest(aghfp);
            	    break;
        		default:
        			aghfpSendError(aghfp);
        			break;
        	}
            break;

        case AGHFP_INTERNAL_SET_RESPONSE_HOLD_STATUS_REQ:           /* AT+BTRH=c        */
        {
            AGHFP_INTERNAL_SET_RESPONSE_HOLD_STATUS_REQ_T *msg = (AGHFP_INTERNAL_SET_RESPONSE_HOLD_STATUS_REQ_T *)message;
            PRINT (("AGHFP_INTERNAL_SET_RESPONSE_HOLD_STATUS_REQ\n"));
        	switch(profileState)
        	{
            	case aghfp_slc_connected:
            	case aghfp_incoming_call_establish:
            	case aghfp_outgoing_call_establish:
            	case aghfp_active_call:
                case aghfp_held_call:
            	    aghfpHandleSetResponseHoldStatus(aghfp,msg->cmd);
            	    break;
        		default:
        			aghfpSendError(aghfp);
        			break;
        	}
            break;
        }

        case AGHFP_INTERNAL_SUBSCRIBER_NUMBER_REQ:       /* AT+CNUM          */
            PRINT (("AGHFP_INTERNAL_SUBSCRIBER_NUMBER_REQ\n"));
        	switch(profileState)
        	{
            	case aghfp_slc_connected:
            	case aghfp_incoming_call_establish:
            	case aghfp_outgoing_call_establish:
            	case aghfp_active_call:
                case aghfp_held_call:
            	    aghfpHandleSubscriberNumbersRequest(aghfp);
            	    break;
        		default:
        			aghfpSendError(aghfp);
        			break;
        	}
            break;

        case AGHFP_INTERNAL_CURRENT_CALLS_REQ:           /* AT+CLCC          */
            PRINT (("AGHFP_INTERNAL_CURRENT_CALLS_REQ\n"));
        	switch(profileState)
        	{
            	case aghfp_slc_connected:
            	case aghfp_incoming_call_establish:
            	case aghfp_outgoing_call_establish:
            	case aghfp_active_call:
                case aghfp_held_call:
            	    aghfpHandleCurrentCallsRequest(aghfp);
            	    break;
        		default:
        			aghfpSendError(aghfp);
        			break;
        	}
            break;

        case AGHFP_INTERNAL_NETWORK_OPERATOR_FORMAT_REQ: /* AT+COPS=m,f      */
        {
            AGHFP_INTERNAL_NETWORK_OPERATOR_FORMAT_REQ_T *msg = (AGHFP_INTERNAL_NETWORK_OPERATOR_FORMAT_REQ_T *)message;
            PRINT (("AGHFP_INTERNAL_NETWORK_OPERATOR_FORMAT_REQ\n"));
        	switch(profileState)
        	{
            	case aghfp_slc_connected:
            	case aghfp_incoming_call_establish:
            	case aghfp_outgoing_call_establish:
            	case aghfp_active_call:
                case aghfp_held_call:
            	    aghfpHandleNetworkOperatorFormatRequest(aghfp, msg->mode, msg->format);
            	    break;
        		default:
        			aghfpSendError(aghfp);
        			break;
        	}
            break;
        }

        case AGHFP_INTERNAL_NETWORK_OPERATOR_REQ:        /* AT+COPS?         */
            PRINT (("AGHFP_INTERNAL_NETWORK_OPERATOR_REQ\n"));
        	switch(profileState)
        	{
            	case aghfp_slc_connected:
            	case aghfp_incoming_call_establish:
            	case aghfp_outgoing_call_establish:
            	case aghfp_active_call:
                case aghfp_held_call:
            	    aghfpHandleNetworkOperatorRequest(aghfp);
            	    break;
        		default:
        			aghfpSendError(aghfp);
        			break;
        	}
            break;
            
        case AGHFP_INTERNAL_INDICATORS_ACTIVATION_REQ: /* AT+BIA=s,c,cs,ch,s,r,b*/
        {
            AGHFP_INTERNAL_INDICATORS_ACTIVATION_REQ_T *msg = (AGHFP_INTERNAL_INDICATORS_ACTIVATION_REQ_T *)message;
            PRINT (("AGHFP_INTERNAL_INDICATORS_ACTIVATION_REQ\n"));
        	switch(profileState)
        	{
            	case aghfp_slc_connected:
            	case aghfp_incoming_call_establish:
            	case aghfp_outgoing_call_establish:
            	case aghfp_active_call:
                case aghfp_held_call:
            	    aghfpHandleIndicatorsActivationRequest(aghfp, msg);
            	    break;
        		default:
        			aghfpSendError(aghfp);
        			break;
        	}
            break;
        }


		/*************************************************************************/
		/* Audio Connection Messages                                             */
		/*************************************************************************/

        case AGHFP_INTERNAL_AUDIO_TRANSFER_REQ:
            switch(profileState)
            {
                case aghfp_slc_connected:
                case aghfp_incoming_call_establish:
                case aghfp_outgoing_call_establish:
                case aghfp_active_call:
                case aghfp_held_call:
                    aghfpHandleAudioTransferReq(aghfp, (AGHFP_INTERNAL_AUDIO_TRANSFER_REQ_T*)message);
                    break;
                default:
                    handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
                    break;
            }
            break;

		case AGHFP_INTERNAL_AUDIO_CONNECT_REQ:
			PRINT(("AGHFP_INTERNAL_AUDIO_CONNECTION_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleAudioConnectReq(aghfp, (AGHFP_INTERNAL_AUDIO_CONNECT_REQ_T*)message);
					break;
				default:
					/* Panic in debug and ignore in release lib variants */
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;

		case AGHFP_INTERNAL_AUDIO_CONNECT_RES:
			PRINT(("AGHFP_INTERNAL_AUDIO_CONNECT_RES\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
				{
					aghfpHandleAudioConnectRes(aghfp, (AGHFP_INTERNAL_AUDIO_CONNECT_RES_T*)message);
					break;
				}
				default:
					/* Panic in debug and ignore in release lib variants */
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;

		case AGHFP_INTERNAL_AUDIO_DISCONNECT_REQ:
			PRINT(("AGHFP_INTERNAL_AUDIO_DISCONNECT_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleAudioDisconnectReq(aghfp);
					break;
				default:
					/* Panic in debug and ignore in release lib variants */
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;

		case AGHFP_INTERNAL_SET_AUDIO_PARAMS_REQ:
			PRINT(("AGHFP_INTERNAL_SET_AUDIO_PARAMS_REQ\n"));
			
			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSetAudioParamsReq(aghfp, (AGHFP_INTERNAL_SET_AUDIO_PARAMS_REQ_T*)message);
					break;
				default:
					/* Panic in debug and ignore in release lib variants */
					handleUnexpected(aghfpUnexpectedAghfpPrim, profileState, id);
					break;
			}
			break;
			

		/*************************************************************************/
		/* Indicator Messages                                                    */
		/*************************************************************************/

		case AGHFP_INTERNAL_SEND_SERVICE_INDICATOR:
			PRINT(("AGHFP_INTERNAL_SEND_SERVICE_INDICATOR\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                case aghfp_held_call:
					aghfpHandleSendServiceIndicator(aghfp,
					((AGHFP_INTERNAL_SEND_SERVICE_INDICATOR_T*)message)->availability);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SEND_SERVICE_INDICATOR_CFM, aghfp, aghfp_fail);
					break;
			}
			break;

        case AGHFP_INTERNAL_SEND_CALL_INDICATOR:
        {
            aghfp_call_status status = ((AGHFP_INTERNAL_SEND_CALL_INDICATOR_T*)message)->status;
            PRINT(("AGHFP_INTERNAL_SEND_CALL_INDICATOR\n"));
            switch(profileState)
            {
                case aghfp_slc_connected:
                case aghfp_incoming_call_establish:
                case aghfp_outgoing_call_establish:
                case aghfp_active_call:
                case aghfp_held_call:
                    aghfpHandleSendCallIndicator(aghfp, status);
                    break;
                default:
                    aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALL_INDICATOR_CFM, aghfp, aghfp_fail);
                    break;
            }
            break;
        }

        case AGHFP_INTERNAL_SEND_CALL_SETUP_INDICATOR:
        {
            aghfp_call_setup_status status;
            status = ((AGHFP_INTERNAL_SEND_CALL_SETUP_INDICATOR_T*)message)->type;

            PRINT(("AGHFP_INTERNAL_SEND_CALL_SETUP_INDICATOR\n"));
            switch(profileState)
            {
                case aghfp_slc_connected:
                case aghfp_incoming_call_establish:
                case aghfp_outgoing_call_establish:
                case aghfp_active_call:
                case aghfp_held_call:
                    aghfpHandleSendCallSetupIndicator(aghfp, status);
                    break;
                default:
                    aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALL_SETUP_INDICATOR_CFM, aghfp, aghfp_fail);
                    break;
            }
            break;
        }

        case AGHFP_INTERNAL_SEND_CALL_HELD_INDICATOR:
        {
            aghfp_call_held_status status = ((AGHFP_INTERNAL_SEND_CALL_HELD_INDICATOR_T*)message)->status;
            switch(profileState)
            {
                case aghfp_slc_connected:
                case aghfp_incoming_call_establish:
                case aghfp_outgoing_call_establish:
                case aghfp_active_call:
                case aghfp_held_call:
                    aghfpHandleSendCallHeldIndicator(aghfp, status);
                    break;
                default:
                    aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALL_HELD_INDICATOR_CFM, aghfp, aghfp_fail);
                    break;
            }
            break;
        }
        
		case AGHFP_INTERNAL_SEND_SIGNAL_INDICATOR:
			PRINT(("AGHFP_INTERNAL_SEND_SIGNAL_INDICATOR\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendSignalIndicator(aghfp,
					((AGHFP_INTERNAL_SEND_SIGNAL_INDICATOR_T*)message)->level);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SEND_SIGNAL_INDICATOR_CFM, aghfp, aghfp_fail);
					break;
			}			
			break;
		
		case AGHFP_INTERNAL_SEND_ROAM_INDICATOR:
			PRINT(("AGHFP_INTERNAL_SEND_ROAM_INDICATOR\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendRoamIndicator(aghfp,
					((AGHFP_INTERNAL_SEND_ROAM_INDICATOR_T*)message)->status);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SEND_ROAM_INDICATOR_CFM, aghfp, aghfp_fail);
					break;
			}			
			break;
			
		case AGHFP_INTERNAL_SEND_BATT_CHG_INDICATOR:
			PRINT(("AGHFP_INTERNAL_SEND_BATT_CHG_INDICATOR\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendBattChgIndicator(aghfp,
					((AGHFP_INTERNAL_SEND_BATT_CHG_INDICATOR_T*)message)->level);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SEND_BATT_CHG_INDICATOR_CFM, aghfp, aghfp_fail);
					break;
			}			
			break;

		case AGHFP_INTERNAL_SEND_CALL_WAITING_NOTIFICATION:
		{
			AGHFP_INTERNAL_SEND_CALL_WAITING_NOTIFICATION_T *scwn =	(AGHFP_INTERNAL_SEND_CALL_WAITING_NOTIFICATION_T *)message;
			PRINT(("AGHFP_INTERNAL_SEND_CALL_WAITING_NOTIFICATION\n"));
            aghfpHandleSendCallWaitingNotification(aghfp, scwn->type, scwn->size_number, scwn->number, scwn->size_string, scwn->string);
			break;
		}

		case AGHFP_INTERNAL_SET_SERVICE_STATE:
		{
			PRINT(("AGHFP_INTERNAL_SET_SERVICE_STATE\n"));

			switch (profileState)
			{
				case aghfp_initialising:
					break;	/* Silently ignore */
				case aghfp_ready:
				case aghfp_slc_connecting:
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSetServiceState(aghfp, ((AGHFP_INTERNAL_SET_SERVICE_STATE_T*)message)->service_state);
					break;
			}

			break;
		}


		/*************************************************************************/
		/* Call Related Messages                                                 */
		/*************************************************************************/

        case AGHFP_INTERNAL_SEND_RING_ALERT:
            PRINT(("AGHFP_INTERNAL_SEND_RING_ALERT\n"));
            switch (profileState)
            {
                case aghfp_slc_connected:
                case aghfp_incoming_call_establish:
                case aghfp_outgoing_call_establish:
                case aghfp_active_call:
                case aghfp_held_call:
                    aghfpHandleSendRingAlert(aghfp);
                    break;
                default:
                    aghfpSendCommonCfmMessageToApp(AGHFP_SEND_RING_ALERT_CFM, aghfp, aghfp_fail);
                    break;
            }
            break;

		case AGHFP_INTERNAL_SEND_CALLER_ID:
		{
			AGHFP_INTERNAL_SEND_CALLER_ID_T *msg = (AGHFP_INTERNAL_SEND_CALLER_ID_T*)message;
			switch(profileState)
			{
                case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
                case aghfp_held_call:
					aghfpHandleSendCallerId(aghfp, msg->type, msg->size_number, msg->size_string, msg->data);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALLER_ID_CFM, aghfp, aghfp_fail);
					break;
			}
			break;
		}
		case AGHFP_INTERNAL_INBAND_RING_TONE_ENABLE:
			PRINT(("AGHFP_INTERNAL_INBAND_RING_TONE_ENABLE\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                case aghfp_held_call:
					aghfpHandleInBandRingToneEnable(aghfp, ((AGHFP_INTERNAL_INBAND_RING_TONE_ENABLE_T*)message)->enable);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_INBAND_RING_ENABLE_CFM, aghfp, aghfp_fail);
					break;
			}
			break;


		/*************************************************************************/
		/* Call Management Messages                                              */
		/*************************************************************************/

		case AGHFP_INTERNAL_CALL_MGR_CREATE_REQ:
			PRINT(("AGHFP_INTERNAL_CALL_MGR_CREATE_REQ\n"));
			
			switch(profileState)
			{
				case aghfp_slc_connected:
					aghfpHandleCallCreate(aghfp, (AGHFP_INTERNAL_CALL_MGR_CREATE_REQ_T *)message);
					break;
				default:
					aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_error);
					break;
			}
			break;
			
			
		case AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ:
			PRINT(("AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ\n"));
			
			switch(profileState)
			{
				case aghfp_slc_connected:
					aghfpHandleCallCreateAudio(aghfp, (AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ_T *)message);
					break;
				default:
					aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_error);
					break;
			}
			break;
			
			
		case AGHFP_INTERNAL_CALL_MGR_ANSWER_REQ:
			PRINT(("AGHFP_INTERNAL_CALL_MGR_ANSWER_REQ\n"));
			
			switch(profileState)
			{
                /* Response and hold, sending agfhp_call_active
                   indicator will change the state to aghfp_active_call */
				case aghfp_incoming_call_establish:
                case aghfp_held_call:
                case aghfp_active_call:
					aghfpHandleCallAnswer(aghfp);
					break;
				default:
					/* Silently ignore message */
					break;
			}
			break;
			
			
		case AGHFP_INTERNAL_CALL_MGR_REMOTE_ANSWERED_REQ:
			PRINT(("AGHFP_INTERNAL_CALL_MGR_REMOTE_ANSWERED_REQ\n"));
			
			switch(profileState)
			{
				case aghfp_outgoing_call_establish:
					aghfpHandleCallRemoteAnswered(aghfp);
					break;
				default:
					/* Silently ignore message */
					break;
			}
			break;
			
			
		case AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ:
			PRINT(("AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ\n"));
			
			switch(profileState)
			{
                /* added aghfp_slc_connected for held incoming call */
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
                case aghfp_slc_connected:
					aghfpHandleCallTerminate(aghfp, (AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ_T *)message);
					break;
				default:
                    PRINT(("profileState = %d\n", profileState));
					aghfpSendCommonCfmMessageToApp(AGHFP_CALL_MGR_TERMINATE_IND, aghfp, aghfp_fail);
					break;
			}
			break;
            
        case AGHFP_INTERNAL_CALL_MGR_TERMINATE_HELD_REQ:
            PRINT(("AGHFP_INTERNAL_CALL_MGR_TERMINATE_HELD_REQ\n"));
            
            switch(profileState)
            {
                case aghfp_active_call:
                case aghfp_held_call:
                    aghfpHandleHeldCallTerminate(aghfp);
                    break;
                default:
                    PRINT(("profileState = %d\n", profileState));
                    break;
            }
            break;
            
        case AGHFP_INTERNAL_CALL_MGR_ACCEPT_HELD_REQ:
            PRINT(("AGHFP_INTERNAL_CALL_MGR_ACCEPT_HELD_REQ\n"));
            
            switch(profileState)
            {
                case aghfp_held_call:
                case aghfp_active_call:
                    aghfpHandleHeldCallAccept(aghfp);
                    break;
                default:
                    PRINT(("profileState = %d\n", profileState));
                    break;
            }
            break;
            			
			
		case AGHFP_INTERNAL_RING_REPEAT_REQ:
			PRINT(("AGHFP_INTERNAL_RING_REPEAT_REQ\n"));
			
			switch(profileState)
			{
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                    aghfpManageCall(aghfp, CallEventRing, CallFlagsNone);
					break;
				default:
					/* Silently ignore message */
					break;
			}
			break;
			
		
			
		/*************************************************************************/
		/* Misc Other Messages                                                   */
		/*************************************************************************/

		case AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ:
			PRINT(("AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ\n"));
			
			switch(profileState)
			{
				case aghfp_initialising:
				case aghfp_ready:
				case aghfp_slc_connecting:
				case aghfp_slc_connected:
					aghfpHandleSetCallerIdDetails(aghfp, (AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ_T *)message);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SET_CALLER_ID_DETAILS_CFM, aghfp, aghfp_fail);
					break;
			}
			break;
			
		case AGHFP_INTERNAL_VOICE_RECOGNITION_ENABLE:
			PRINT(("AGHFP_INTERNAL_VOICE_RECOGNITION_ENABLE\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleVoiceRecognitionEnable(aghfp, ((AGHFP_INTERNAL_VOICE_RECOGNITION_ENABLE_T*)message)->enable);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_VOICE_RECOGNITION_ENABLE_CFM, aghfp, aghfp_fail);
			}
			break;

		case AGHFP_INTERNAL_SEND_PHONE_NUMBER_FOR_VOICE_TAG:
		{
			AGHFP_INTERNAL_SEND_PHONE_NUMBER_FOR_VOICE_TAG_T *data;
			data = (AGHFP_INTERNAL_SEND_PHONE_NUMBER_FOR_VOICE_TAG_T *)message;
			PRINT(("AGHFP_INTERNAL_SEND_PHONE_NUMBER_FOR_VOICE_TAG\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendPhoneNumberForVoiceTag(aghfp, data->size_number, data->number);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SEND_PHONE_NUMBER_FOR_VOICE_TAG_CFM, aghfp, aghfp_fail);
			}
			break;
		}

		case AGHFP_INTERNAL_SET_REMOTE_MICROPHONE_GAIN:
			PRINT(("AGHFP_INTERNAL_SET_REMOTE_MICROPHONE_GAIN\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSetRemoteMicrophoneGain(aghfp, ((AGHFP_INTERNAL_SET_REMOTE_MICROPHONE_GAIN_T*)message)->gain);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SET_REMOTE_MICROPHONE_GAIN_CFM, aghfp, aghfp_fail);
			}
			break;

		case AGHFP_INTERNAL_SET_REMOTE_SPEAKER_VOLUME:
			PRINT(("AGHFP_INTERNAL_SET_REMOTE_SPEAKER_VOLUME\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSetRemoteSpeakerVolume(aghfp, ((AGHFP_INTERNAL_SET_REMOTE_SPEAKER_VOLUME_T*)message)->volume);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SET_REMOTE_SPEAKER_VOLUME_CFM, aghfp, aghfp_fail);
			}
			break;

		case AGHFP_INTERNAL_SYNC_MIC_GAIN:
			PRINT(("AGHFP_INTERNAL_SYNC_MIC_GAIN\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSyncMicrophoneGain(aghfp, ((AGHFP_INTERNAL_SYNC_MIC_GAIN_T*)message)->gain);
					break;
				default:
					/* We've received a mic gain command from the HF, but we haven't currently
					   got an SLC. That can't really happen. If it does, then just silently ignore. */
					break;
			}
			break;

		case AGHFP_INTERNAL_SYNC_SPEAKER_VOLUME:
			PRINT(("AGHFP_INTERNAL_SYNC_SPEAKER_VOLUME\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSyncSpeakerVolume(aghfp, ((AGHFP_INTERNAL_SYNC_SPEAKER_VOLUME_T*)message)->volume);
					break;
				default:
					/* We've received a speaker gain command from the HF, but we haven't currently
					   got an SLC. That can't really happen. If it does, then just silently ignore. */
					break;
			}
			break;

        case AGHFP_INTERNAL_SEND_RESPONSE_HOLD_STATE:
			PRINT(("AGHFP_INTERNAL_SEND_RESPONSE_HOLD_STATE\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendResponseHoldState(aghfp, ((AGHFP_INTERNAL_SEND_RESPONSE_HOLD_STATE_T*)message)->state);
					break;
				default:
					/* We've received a  command from the HF, but we haven't currently
					   got an SLC. That can't really happen. If it does, then just silently ignore. */
					break;
			}
			break;

        case AGHFP_INTERNAL_CONFIRM_RESPONSE_HOLD_STATE:
			PRINT(("AGHFP_INTERNAL_CONFIRM_RESPONSE_HOLD_STATE\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleConfirmResponseHoldState(aghfp, ((AGHFP_INTERNAL_CONFIRM_RESPONSE_HOLD_STATE_T*)message)->state);
					break;
				default:
					/* We've received a  command from the HF, but we haven't currently
					   got an SLC. That can't really happen. If it does, then just silently ignore. */
					break;
			}
			break;

        case AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBER:
			PRINT(("AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBER\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendSubscriberNumber(aghfp, (AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBER_T*)message);
					break;
				default:
					/* We've received a  command from the HF, but we haven't currently
					   got an SLC. That can't really happen. If it does, then just silently ignore. */
					break;
			}
			break;

        case AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBERS_COMPLETE:
			PRINT(("AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBERS_COMPLETE\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendSubscriberNumbersComplete(aghfp);
					break;
				default:
					/* We've received a  command from the HF, but we haven't currently
					   got an SLC. That can't really happen. If it does, then just silently ignore. */
					break;
			}
			break;

        case AGHFP_INTERNAL_SEND_CURRENT_CALL:
			PRINT(("AGHFP_INTERNAL_SEND_CURRENT_CALL\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendCurrentCall(aghfp, (AGHFP_INTERNAL_SEND_CURRENT_CALL_T*)message);
					break;
				default:
					/* We've received a  command from the HF, but we haven't currently
					   got an SLC. That can't really happen. If it does, then just silently ignore. */
					break;
			}
			break;

        case AGHFP_INTERNAL_SEND_CURRENT_CALLS_COMPLETE:
			PRINT(("AGHFP_INTERNAL_SEND_CURRENT_CALLS_COMPLETE\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendCurrentCallsComplete(aghfp);
					break;
				default:
					/* We've received a  command from the HF, but we haven't currently
					   got an SLC. That can't really happen. If it does, then just silently ignore. */
					break;
			}
			break;

        case AGHFP_INTERNAL_SEND_NETWORK_OPERATOR:
        {
			AGHFP_INTERNAL_SEND_NETWORK_OPERATOR_T *msg = (AGHFP_INTERNAL_SEND_NETWORK_OPERATOR_T*)message;
			PRINT(("AGHFP_INTERNAL_SEND_NETWORK_OPERATOR\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleSendNetworkOperator(aghfp, msg->mode, msg->size_operator, msg->operator);
					break;
				default:
					/* We've received a  command from the HF, but we haven't currently
					   got an SLC. That can't really happen. If it does, then just silently ignore. */
					break;
			}
			break;
		}

        case AGHFP_INTERNAL_USER_DATA_REQ:
            PRINT(("AGHFP_INTERNAL_USER_DATA_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_held_call:
					aghfpHandleUserDataReq(aghfp, (AGHFP_INTERNAL_USER_DATA_REQ_T *) message);
					break;

                case aghfp_ready:
                case aghfp_slc_connecting:
                    /* Cannot send the data, haven't got a valid SLC established */
                    aghfpHandleUserDataReqFail(aghfp, (AGHFP_INTERNAL_USER_DATA_REQ_T *) message);
                    break;

                case aghfp_initialising:
				default:
					/* We have received a request to send user data and we're in the wrong state */
					break;
			}
            break;

        case AGHFP_INTERNAL_SEND_ERROR_REQ:
            PRINT(("AGHFP_INTERNAL_SEND_ERROR_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_slc_connecting:
                case aghfp_held_call:
					aghfpSendError(aghfp);
					break;

                case aghfp_ready:
                    /* Cannot send error, haven't got a valid RFCOMM established */
                    aghfpSendCommonCfmMessageToApp(AGHFP_SEND_ERROR_CFM, aghfp, aghfp_fail);
                    break;

                case aghfp_initialising:
				default:
					/* We have received this request in the wrong state */
					break;
			}
            break;

        case AGHFP_INTERNAL_SEND_OK_REQ:
            PRINT(("AGHFP_INTERNAL_SEND_OK_REQ\n"));

			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
				case aghfp_active_call:
                case aghfp_slc_connecting:
                case aghfp_held_call:
					aghfpSendOk(aghfp);
					break;

                case aghfp_ready:
                    /* Cannot send error, haven't got a valid RFCOMM established */
                    aghfpSendCommonCfmMessageToApp(AGHFP_SEND_OK_CFM, aghfp, aghfp_fail);
                    break;

                case aghfp_initialising:
				default:
					/* We have received this request in the wrong state */
					break;
			}
            break;

		/* WB-Speech Messages */
		case AGHFP_INTERNAL_WBS_CODEC_NEGOTIATE_REQ:
			PRINT(("AGHFP_INTERNAL_WBS_CODEC_NEGOTIATE_REQ\n"));
			aghfpHandleWbsCodecNegReq(aghfp, ((AGHFP_INTERNAL_WBS_CODEC_NEGOTIATE_REQ_T*)message)->codec_id);
			break;
			
		case AGHFP_INTERNAL_WBS_CODEC_CONNECTION_REQ:
			PRINT(("AGHFP_INTERNAL_WBS_CODEC_CONNECTION_REQ\n"));
			aghfpHandleWbsCodecConReq(aghfp);
			break;
				
		case AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ:
			PRINT(("AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ\n"));
			aghfpHandleCodecNegotiationReq(aghfp, (AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ_T*)message);
			break;

        /* Qualcomm Codec Extensions Messages */
        case AGHFP_INTERNAL_AT_QAC_REQ:
                PRINT(("AGHFP_INTERNAL_AT_QAC_REQ\n"));
                aghfpHandleHfCodecModes(aghfp, (AGHFP_INTERNAL_AT_QAC_REQ_T *)message);
                break;

        case AGHFP_INTERNAL_AT_QCS_REQ:
                PRINT(("AGHFP_INTERNAL_AT_QCS_REQ\n"));
                aghfpHandleHfCodecSelect(aghfp, (AGHFP_INTERNAL_AT_QCS_REQ_T *)message);
                break;

        /* HF Indicator Mesages */
		case AGHFP_INTERNAL_AT_BIND_REQ:
			PRINT(("AGHFP_INTERNAL_AT_BIND_REQ\n"));
			aghfpHandleBindRequest(aghfp, (AGHFP_INTERNAL_AT_BIND_REQ_T*)message);			
			break;

		case AGHFP_INTERNAL_AT_BIND_SUPPORTED_REQ:
			PRINT(("AGHFP_INTERNAL_AT_BIND_SUPPORTED_REQ\n"));
			aghfpHandleBindSupportedRequest(aghfp);		
			break;

		case AGHFP_INTERNAL_AT_BIND_INDICATOR_STATUS_REQ:
			PRINT(("AGHFP_INTERNAL_AT_BIND_STATUS_REQ\n"));
			aghfpHandleBindIndicatorStatusRequest(aghfp);				
			break;

		case AGHFP_INTERNAL_AT_BIEV_STATUS_REQ:
			PRINT(("AGHFP_INTERNAL_AT_BIEV_STATUS_REQ\n"));
			aghfpHandleBievRequest(aghfp, (AGHFP_INTERNAL_AT_BIEV_STATUS_REQ_T*)message);
			break;

	       case AGHFP_INTERNAL_SEND_BIND_INDICATOR_STATE:
			switch(profileState)
			{
				case aghfp_slc_connected:
				case aghfp_incoming_call_establish:
				case aghfp_outgoing_call_establish:
                case aghfp_held_call:
					aghfpHfIndicatorsStatusResponse(aghfp,
					((AGHFP_INTERNAL_SEND_BIND_INDICATOR_STATE_T*)message)->assigned_num,
					((AGHFP_INTERNAL_SEND_BIND_INDICATOR_STATE_T*)message)->state);
					break;
				default:
					aghfpSendCommonCfmMessageToApp(AGHFP_SEND_BIND_INDICATOR_STATE_CFM, aghfp, aghfp_fail);
					break;
			}
			break;		   	

		default:
			PRINT(("Unhandled message\n"));
			/* Received an unhandled message */
			handleUnexpected(aghfpUnexpectedMessage, profileState, id);
			break;
    }
}


/****************************************************************************
 All messages for this profile lib are handled by this function
*/
void aghfpProfileHandler(Task task, MessageId id, Message message)
{
	AGHFP *aghfp = (AGHFP*)task;
	aghfp_state profileState = aghfp->state;

    /*printf("%s - ", stateNames[profileState]);*/
	if (id >= AGHFP_INTERNAL_MSG_BASE && id < AGHFP_INTERNAL_MSG_TOP)
	{
		handleAghfpMessages(task, id, message);
	}
	else
	{
		/* Check the message id */
		switch (id)
		{
	       case CL_DM_SYNC_RENEGOTIATE_IND:
				PRINT(("CL_DM_SYNC_RENEGOTIATE_IND \n" ));
	       break ;
    
			/*************************************************************************/
			/* SDP Messages                                                          */
			/*************************************************************************/
	
			case CL_SDP_REGISTER_CFM:
				PRINT(("CL_SDP_REGISTER_CFM\n"));
	
				switch(profileState)
				{
					case aghfp_initialising:
					case aghfp_ready:
					case aghfp_slc_connecting:
						/* Handle the register cfm */
						aghfpHandleSdpRegisterCfm(aghfp, (CL_SDP_REGISTER_CFM_T *) message);
						break;
	
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
	        case CL_SDP_UNREGISTER_CFM:
	            switch(profileState)
				{
					case aghfp_initialising:
					case aghfp_ready:
					case aghfp_slc_connecting:
	                case aghfp_slc_connected:
	                case aghfp_incoming_call_establish:
	                case aghfp_outgoing_call_establish:
	                case aghfp_active_call:
                    case aghfp_held_call:
						/* Handle the register cfm */
						aghfpHandleSdpUnregisterCfm(aghfp, (CL_SDP_UNREGISTER_CFM_T *) message);
						break;
	
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
			case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
				PRINT(("CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM\n"));
	
				switch(profileState)
				{
					case aghfp_slc_connecting:
						/* Currently we only look for attributes during SLC establishment */
						aghfpHandleServiceSearchAttributeCfm(aghfp, (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *) message);
						break;
	                case aghfp_slc_connected:
	                case aghfp_incoming_call_establish:
	                case aghfp_outgoing_call_establish:
	                case aghfp_active_call:
                    case aghfp_held_call:
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
            case CL_SDP_SERVICE_SEARCH_CFM:
				PRINT(("CL_SDP_SERVICE_SEARCH_CFM\n"));
	
				switch(profileState)
				{
					case aghfp_slc_connecting:
						/* Currently we only look for attributes during SLC establishment */
						aghfpHandleServiceSearchCfm(aghfp, (CL_SDP_SERVICE_SEARCH_CFM_T *) message);
						break;
	                case aghfp_slc_connected:
	                case aghfp_incoming_call_establish:
	                case aghfp_outgoing_call_establish:
	                case aghfp_active_call:
                    case aghfp_held_call:
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
			/*************************************************************************/
			/* RFCOMM Messages                                                       */
			/*************************************************************************/
	
			case CL_RFCOMM_REGISTER_CFM:
				PRINT(("CL_RFCOMM_REGISTER_CFM\n"));
	
				switch(profileState)
				{
					case aghfp_initialising:
						aghfpHandleRfcommRegisterCfm(aghfp, (CL_RFCOMM_REGISTER_CFM_T *) message);
						break;
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
			case CL_RFCOMM_CONNECT_IND:
				PRINT(("CL_RFCOMM_CONNECT_IND\n"));
	
				switch(profileState)
				{
					case aghfp_ready:
						/* Handle the connect indication */
						aghfpHandleRfcommConnectInd(aghfp, (CL_RFCOMM_CONNECT_IND_T *) message);
						break;
	
					case aghfp_slc_connecting:
					case aghfp_slc_connected:
					case aghfp_incoming_call_establish:
					case aghfp_outgoing_call_establish:
                    case aghfp_held_call:
						/*  Reject the connect request - this shouldn't happen too often as we
						unregister our service record after we establish an SLC */
						aghfpHandleSlcConnectIndReject(aghfp, (CL_RFCOMM_CONNECT_IND_T *) message);
						break;
	
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
			case CL_RFCOMM_CLIENT_CONNECT_CFM:
				PRINT(("CL_RFCOMM_CLIENT_CONNECT_CFM\n"));
	
				switch(profileState)
				{
					case aghfp_slc_connecting:
						/* Handle the connect cfm */
						aghfpHandleRfcommClientConnectCfm(aghfp, (CL_RFCOMM_CLIENT_CONNECT_CFM_T *) message);
						break;
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
			case CL_RFCOMM_SERVER_CONNECT_CFM:
				PRINT(("CL_RFCOMM_SERVER_CONNECT_CFM\n"));
	
				switch(profileState)
				{
					case aghfp_slc_connecting:
						/* Handle the connect cfm */
						aghfpHandleRfcommServerConnectCfm(aghfp, (CL_RFCOMM_SERVER_CONNECT_CFM_T *) message);
						break;
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
			case CL_RFCOMM_DISCONNECT_IND:
				PRINT(("CL_RFCOMM_DISCONNECT_IND\n"));
	
				switch(profileState)
				{
					case aghfp_ready:
					case aghfp_slc_connecting:
					case aghfp_slc_connected:
					case aghfp_incoming_call_establish:
					case aghfp_outgoing_call_establish:
	                case aghfp_active_call:
                    case aghfp_held_call:
						aghfpHandleRfcommDisconnectInd(aghfp, (CL_RFCOMM_DISCONNECT_IND_T*) message);
						break;
	
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
			case CL_RFCOMM_DISCONNECT_CFM:
				PRINT(("CL_RFCOMM_DISCONNECT_CFM\n"));
	
				switch(profileState)
				{
					case aghfp_ready:
					case aghfp_slc_connecting:
					case aghfp_slc_connected:
					case aghfp_incoming_call_establish:
					case aghfp_outgoing_call_establish:
	                case aghfp_active_call:
                    case aghfp_held_call:
						aghfpHandleRfcommDisconnectCfm(aghfp, (CL_RFCOMM_DISCONNECT_CFM_T*) message);
						break;
	
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
                
            case CL_RFCOMM_PORTNEG_IND:
                PRINT(("CL_RFCOMM_PORTNEG_IND\n"));
                
				switch(profileState)
				{
					case aghfp_ready:
					case aghfp_slc_connecting:
					case aghfp_slc_connected:
					case aghfp_incoming_call_establish:
					case aghfp_outgoing_call_establish:
	                case aghfp_active_call:
                    case aghfp_held_call:
						aghfpHandleRfcommPortnegInd(aghfp, (CL_RFCOMM_PORTNEG_IND_T*) message);
						break;
	
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
                
			/*************************************************************************/
			/* Audio Connection Messages                                             */
			/*************************************************************************/
	
			case CL_DM_SYNC_CONNECT_IND:
				PRINT(("CL_DM_SYNC_CONNECT_IND\n"));
	
				switch(profileState)
				{
					case aghfp_slc_connected:
					case aghfp_incoming_call_establish:
					case aghfp_outgoing_call_establish:
					case aghfp_active_call:
                    case aghfp_held_call:
						aghfpHandleSyncConnectInd(aghfp, (CL_DM_SYNC_CONNECT_IND_T *)message);
						break;
					default:
						/* Reject request outright because we are in the wrong state */
						aghfpHandleSyncConnectIndReject(aghfp, (CL_DM_SYNC_CONNECT_IND_T *)message);
						break;
				}
				break;
	
			case CL_DM_SYNC_CONNECT_CFM:
				PRINT(("CL_DM_SYNC_CONNECT_CFM\n"));
	
				switch(profileState)
				{
					case aghfp_slc_connected:
					case aghfp_incoming_call_establish:
					case aghfp_outgoing_call_establish:
					case aghfp_active_call:
                    case aghfp_held_call:
						aghfpHandleSyncConnectCfm(aghfp, (CL_DM_SYNC_CONNECT_CFM_T *)message);
						break;
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
			case CL_DM_SYNC_DISCONNECT_IND:
				PRINT(("CL_DM_SYNC_DISCONNECT_IND\n"));
	
				switch(profileState)
				{
					case aghfp_slc_connected:
					case aghfp_incoming_call_establish:
					case aghfp_outgoing_call_establish:
					case aghfp_active_call:
                    case aghfp_held_call:
						aghfpHandleSyncDisconnectInd(aghfp, (CL_DM_SYNC_DISCONNECT_IND_T *)message);
						break;
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
				
				
			/*************************************************************************/
			/* Misc Other Messages                                                   */
			/*************************************************************************/
	
			case MESSAGE_MORE_DATA:
				PRINT(("MESSAGE_MORE_DATA\n"));
	
				switch(profileState)
				{
					case aghfp_slc_connecting:
					case aghfp_slc_connected:
					case aghfp_incoming_call_establish:
					case aghfp_outgoing_call_establish:
					case aghfp_active_call:
                    case aghfp_held_call:
						/* We have received more data into the RFCOMM buffer */
						aghfpHandleReceivedData(aghfp, ((MessageMoreData *) message)->source);
						break;
	
					default:
						/* Panic in debug and ignore in release lib variants */
						handleUnexpected(aghfpUnexpectedClPrim, profileState, id);
						break;
				}
				break;
	
	        case CL_DM_SYNC_REGISTER_CFM:
	            PRINT(("CL_DM_SYNC_REGISTER_CFM\n"));
	
				switch(profileState)
				{
	            case aghfp_initialising:
	            case aghfp_ready:
	            case aghfp_slc_connecting:
	            case aghfp_slc_connected:
	            case aghfp_incoming_call_establish:
	            case aghfp_outgoing_call_establish:
	            case aghfp_active_call:
                case aghfp_held_call:
	                /* Registered for Synchronous connection indications */
	    			aghfpHandleSyncRegisterCfm(aghfp);
	                break;
	
	            default:
	                break;
	            }
	            break;
	
	
	        /* Unhandled messages */
            case CL_RFCOMM_PORTNEG_CFM:
            case CL_RFCOMM_CONTROL_IND:
            case CL_RFCOMM_CONTROL_CFM:
            case CL_RFCOMM_LINE_STATUS_IND:
            case CL_SM_ENCRYPTION_CHANGE_IND:
			case CL_SM_ENCRYPTION_KEY_REFRESH_IND:
	        case MESSAGE_MORE_SPACE:
	        case MESSAGE_SOURCE_EMPTY:
	            break;
	
	
			default:
				PRINT(("Unhandled message\n"));
				/* Received an unhandled message */
				handleUnexpected(aghfpUnexpectedMessage, profileState, id);
				break;
		}
	}
}

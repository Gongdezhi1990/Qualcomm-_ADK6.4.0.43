/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#include <stdio.h> 	/* For sprintf */
#include <string.h> /* For memcpy */

#include "aghfp.h"
#include "aghfp_common.h"
#include "aghfp_ok.h"
#include "aghfp_private.h"
#include "aghfp_misc_handler.h"
#include "aghfp_send_data.h"
#include "aghfp_call_manager.h"

#include <panic.h>


#define AUDIO_WITH_AG 0
#define AUDIO_WITH_HS 1

/****************************************************************************
 HS has sent us a keypad command. Profile handler has asked
 us to act on this request.
*/
/*                                                        Audio with AG               Audio with HS           */
static const uint16 aghfpSlcConnectedAppResponse[2] = { AGHFP_HS_AUDIO_REQUEST_IND, 0                         };
static const uint16 aghfpIncomingCallAppResponse[2] = { AGHFP_HS_CALL_ANSWER_IND  , AGHFP_HS_CALL_ANSWER_IND  };
static const uint16 aghfpOutgoingCallAppResponse[2] = { AGHFP_HS_AUDIO_REQUEST_IND, AGHFP_HS_CALL_HANG_UP_IND };
static const uint16 aghfpActiveCallAppResponse[2] =   { AGHFP_HS_AUDIO_REQUEST_IND, AGHFP_HS_CALL_HANG_UP_IND };
void aghfpHandleKeypadCommand(AGHFP *aghfp, uint16 keycode)
{
	if (keycode==200)
	{
		uint16 response;
		uint16 audioLoc = (aghfp->audio_sink==0)?AUDIO_WITH_AG:AUDIO_WITH_HS;
		
		/* Determine context of keypress command */
		switch (aghfp->state)
		{
		case aghfp_slc_connected:
			response = aghfpSlcConnectedAppResponse[audioLoc];
			break;
		case aghfp_incoming_call_establish:
			response = aghfpIncomingCallAppResponse[audioLoc];
			break;
		case aghfp_outgoing_call_establish:
			response = aghfpOutgoingCallAppResponse[audioLoc];
			break;
		case aghfp_active_call:
			response = aghfpActiveCallAppResponse[audioLoc];
			break;
		default:
			response = 0;
		}
		
        if ( aghfpCallManagerActive(aghfp) )
		{
            if (response)
            {
				switch (response)
				{
				case AGHFP_HS_CALL_ANSWER_IND:
					aghfpManageCall(aghfp, CallEventAnswer, CallFlagOpenAudio);
					break;
				case AGHFP_HS_CALL_HANG_UP_IND:
					aghfpManageCall(aghfp, CallEventTerminate, CallFlagCloseAudio);
					break;
				case AGHFP_HS_AUDIO_REQUEST_IND:
					aghfpManageCall(aghfp, CallEventAudioTransfer, CallFlagOpenAudio);
					break;
				default:
					Panic();
				}
                /* Send OK to HS */
                aghfpSendOk(aghfp);
			}
            else
            {
                /* Send ERROR to HS */
                aghfpSendError(aghfp);
            }
        }
        else
		{
			MAKE_AGHFP_MESSAGE(AGHFP_HS_BUTTON_PRESS_IND);  /* This is really acting as a template for all possible AGHFP_HS primitives */
            
            /* Let the app decide what to do with unexpected button presses */
            if(!response)
                response = AGHFP_HS_BUTTON_PRESS_IND;
                
            /* Inform app */
			message->aghfp = aghfp;
			MessageSend(aghfp->client_task, response, message);
		}
	}
	else
	{
		/* Send ERROR to HS */
		aghfpSendError(aghfp);
	}
}

/****************************************************************************
 HF wants to enable/disable NR and EC on the AG. Profile handler has asked
 us to act on this request.
*/
void aghfpHandleNrecSetupReq(AGHFP *aghfp, bool enable)
{
	MAKE_AGHFP_MESSAGE(AGHFP_NREC_SETUP_IND);
	message->aghfp = aghfp;
	message->enable = enable;
	MessageSend(aghfp->client_task, AGHFP_NREC_SETUP_IND, message);
}


/****************************************************************************
 HF wants to enable/disable voice recognition in the AG. Profile handler has
 asked us to act on this request.
*/
void aghfpHandleVoiceRecognitionSetupReq(AGHFP *aghfp, bool enable)
{
	if (aghfp->supported_features & aghfp_voice_recognition)
	{
		/* Send OK to HF */
		aghfpSendOk(aghfp);

		/* Send indicator to app */
		{
			MAKE_AGHFP_MESSAGE(AGHFP_VOICE_RECOGNITION_SETUP_IND);
			message->aghfp = aghfp;
			message->enable = enable;
			MessageSend(aghfp->client_task, AGHFP_VOICE_RECOGNITION_SETUP_IND, message);
		}
	}
	else
	{
		/* Feature not supported, so send error to HF */
		aghfpSendError(aghfp);
	}
}


/****************************************************************************
 AG wants to tell the HF to enable/disable its voice activation. Pass this
 request on to the profile handler.
*/
void AghfpVoiceRecognitionEnable(AGHFP *aghfp, bool enable)
{
	/* Is voice recognition supported in this AG? */
	if (aghfp->supported_features & aghfp_voice_recognition)
	{
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_VOICE_RECOGNITION_ENABLE);
		message->enable = enable;
		MessageSend(&aghfp->task, AGHFP_INTERNAL_VOICE_RECOGNITION_ENABLE, message);
	}
	else
	{
		aghfpSendCommonCfmMessageToApp(AGHFP_VOICE_RECOGNITION_ENABLE_CFM, aghfp, aghfp_fail);
	}
}


/****************************************************************************
 AG wants to tell the HF to enable/disable its voice activation. Profile
 handler has asked us to act on this request.
*/
void aghfpHandleVoiceRecognitionEnable(AGHFP *aghfp, bool enable)
{
	/* Send AT cmd to HF */
	const char disabledMessage[] = "+BVRA: 0";
	const char enabledMessage[] = "+BVRA: 1";
	if (enable)
	{
		aghfpSendAtCmd(aghfp, enabledMessage);
	}
	else
	{
		aghfpSendAtCmd(aghfp, disabledMessage);
	}

	/* Send confirm to app */
	aghfpSendCommonCfmMessageToApp(AGHFP_VOICE_RECOGNITION_ENABLE_CFM, aghfp, aghfp_success);
}


/****************************************************************************
 HF wants a phone number. Profile handler has asked us to act on this request.
*/
void aghfpHandlePhoneNumberForVoiceTagReq(AGHFP *aghfp)
{
	if (aghfp->supported_features & aghfp_attach_phone_number)
	{
		/* Send indicator to app */
		MAKE_AGHFP_MESSAGE(AGHFP_PHONE_NUMBER_REQUEST_IND);
		message->aghfp = aghfp;
		MessageSend(aghfp->client_task, AGHFP_PHONE_NUMBER_REQUEST_IND, message);

		/* Send OK to HF */
		aghfpSendOk(aghfp);
	}
	else
	{
		/* Feature not supported, so send error to HF */
		aghfpSendError(aghfp);
	}
}


/****************************************************************************
 AG wants to send a phone number to the HF in response to an AT+BINP request.
 Pass this on to the profile handler.
*/
void AghfpSendPhoneNumberForVoiceTag(AGHFP *aghfp, uint16 size_number, const uint8 *number)
{
	if (aghfp->supported_features & aghfp_attach_phone_number)
	{
		MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_SEND_PHONE_NUMBER_FOR_VOICE_TAG, size_number);
		message->size_number = size_number;
		memmove(message->number, number, size_number);
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_PHONE_NUMBER_FOR_VOICE_TAG, message);
	}
	else
	{
		/* Feature not supported, for send error to app */
		aghfpSendCommonCfmMessageToApp(AGHFP_SEND_PHONE_NUMBER_FOR_VOICE_TAG_CFM, aghfp, aghfp_fail);
	}
}


/****************************************************************************
 AG wants to send a phone number to the HF in response to an AT+BINP request.
 Profile handler has asked us to handle this.
*/
void aghfpHandleSendPhoneNumberForVoiceTag(AGHFP *aghfp, uint16 size_number, uint8 *number)
{
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+BINP: ");
	aghfpAtCmdData(aghfp, number, size_number);
	aghfpAtCmdEnd(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_PHONE_NUMBER_FOR_VOICE_TAG_CFM, aghfp, aghfp_success);
}


/******************************************************************************
 AG wants to set microphone gain on the HF. Pass this request on to the
 profile handler.
*/
void AghfpSetRemoteMicrophoneGain(AGHFP *aghfp, uint8 gain)
{
	/* Is remote volume control supported in this AG, and is the specified
       gain in the range 0-15? */
	if (gain <= 15)
	{
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SET_REMOTE_MICROPHONE_GAIN);
		message->gain = gain;
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SET_REMOTE_MICROPHONE_GAIN, message);
	}
	else
	{
		aghfpSendCommonCfmMessageToApp(AGHFP_SET_REMOTE_MICROPHONE_GAIN_CFM, aghfp, aghfp_fail);
	}
}


/******************************************************************************
 AG wants to set microphone gain on the HF. Profile handler has asked us to
 handle this request.
*/
void aghfpHandleSetRemoteMicrophoneGain(AGHFP *aghfp, uint8 gain)
{
	char buf[3];
	/* We can be certain that "gain" won't use more than 2 chars when we
	   sprintf it, because we checked that it was in the range 0-15 in
	   AghfpSetRemoteMicrophoneGain above. */
	sprintf(buf, "%d", gain);

	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+VGM: ");
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdEnd(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_SET_REMOTE_MICROPHONE_GAIN_CFM, aghfp, aghfp_success);
}


/******************************************************************************
 AG wants to set speaker volume on the HF. Pass this request on to the
 profile handler.
*/
void AghfpSetRemoteSpeakerVolume(AGHFP *aghfp, uint8 volume)
{
	/* Is remote volume control supported in this AG, and is the specified
       volume in the range 0-15? */
	if (volume <= 15)
	{
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SET_REMOTE_SPEAKER_VOLUME);
		message->volume = volume;
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SET_REMOTE_SPEAKER_VOLUME, message);
	}
	else
	{
		aghfpSendCommonCfmMessageToApp(AGHFP_SET_REMOTE_SPEAKER_VOLUME_CFM, aghfp, aghfp_fail);
	}
}


/******************************************************************************
 AG wants to set speaker volume on the HF. Profile handler has asked us to
 handle this request.
*/
void aghfpHandleSetRemoteSpeakerVolume(AGHFP *aghfp, uint8 volume)
{
	char buf[3];
	/* We can be certain that "volume" won't use more than 2 chars when we
	   sprintf it, because we checked that it was in the range 0-15 in
	   AghfpSetRemoteSpeakerVolume above. */
	sprintf(buf, "%d", volume);

	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+VGS: ");
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdEnd(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_SET_REMOTE_SPEAKER_VOLUME_CFM, aghfp, aghfp_success);
}


void aghfpHandleSyncMicrophoneGain(AGHFP *aghfp, uint8 gain)
{
	MAKE_AGHFP_MESSAGE(AGHFP_SYNC_MICROPHONE_GAIN_IND);
	message->gain = gain;
    message->aghfp = aghfp;
	MessageSend(aghfp->client_task, AGHFP_SYNC_MICROPHONE_GAIN_IND, message);

	aghfpSendOk(aghfp);
}


void aghfpHandleSyncSpeakerVolume(AGHFP *aghfp, uint8 volume)
{
	MAKE_AGHFP_MESSAGE(AGHFP_SYNC_SPEAKER_VOLUME_IND);
	message->volume = volume;
    message->aghfp = aghfp;
	MessageSend(aghfp->client_task, AGHFP_SYNC_SPEAKER_VOLUME_IND, message);

	aghfpSendOk(aghfp);
}


void aghfpHandleTransmitDtmfCode(AGHFP *aghfp, uint8 code)
{
	MAKE_AGHFP_MESSAGE(AGHFP_TRANSMIT_DTMF_CODE_IND);
	message->aghfp = aghfp;
	message->code = code;
	MessageSend(aghfp->client_task, AGHFP_TRANSMIT_DTMF_CODE_IND, message);

	aghfpSendOk(aghfp);
}


void aghfpHandleResponseHoldStatusRequest(AGHFP *aghfp)
{
	if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
	{
	    MAKE_AGHFP_MESSAGE(AGHFP_RESPONSE_HOLD_STATUS_REQUEST_IND);
	    message->aghfp = aghfp;
	    MessageSend(aghfp->client_task, AGHFP_RESPONSE_HOLD_STATUS_REQUEST_IND, message);
    }
    else
	{   /* Command not supported by this profile version */
    	aghfpSendError(aghfp);
	}
}

void AghfpSendResponseHoldState(AGHFP *aghfp, aghfp_response_hold_state state)
{
	if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
	{
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SEND_RESPONSE_HOLD_STATE);
		message->state = state;
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_RESPONSE_HOLD_STATE, message);
	}
	else
	{	/* Feature not supported by this profile version */
		aghfpSendCommonCfmMessageToApp(AGHFP_SEND_RESPONSE_HOLD_STATE_CFM, aghfp, aghfp_fail);
	}
}

void aghfpHandleSendResponseHoldState(AGHFP *aghfp, aghfp_response_hold_state state)
{
	uint8 stateAsChar = (uint8)state + (uint8)'0';
	
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+BTRH:");
	aghfpAtCmdData(aghfp, &stateAsChar, 1);
	aghfpAtCmdEnd(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_RESPONSE_HOLD_STATE_CFM, aghfp, aghfp_success);
}

void aghfpHandleSetResponseHoldStatus(AGHFP *aghfp, uint8 cmd)
{
    if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
    {
    	MAKE_AGHFP_MESSAGE(AGHFP_SET_RESPONSE_HOLD_STATUS_IND);
    	message->aghfp = aghfp;
    	message->cmd = cmd;
    	MessageSend(aghfp->client_task, AGHFP_SET_RESPONSE_HOLD_STATUS_IND, message);
	}
	else
	{   /* Command not supported by this profile version */
    	aghfpSendError(aghfp);
	}
}

void AghfpConfirmResponseHoldState(AGHFP *aghfp, aghfp_response_hold_state state)
{
	if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
	{
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_CONFIRM_RESPONSE_HOLD_STATE);
		message->state = state;
		MessageSend(&aghfp->task, AGHFP_INTERNAL_CONFIRM_RESPONSE_HOLD_STATE, message);
	}
	else
	{	/* Feature not supported by this profile version */
		aghfpSendCommonCfmMessageToApp(AGHFP_CONFIRM_RESPONSE_HOLD_STATE_CFM, aghfp, aghfp_fail);
	}
}

void aghfpHandleConfirmResponseHoldState(AGHFP *aghfp, aghfp_response_hold_state state)
{
	uint8 stateAsChar = (uint8)state + (uint8)'0';
	
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+BTRH:");
	aghfpAtCmdData(aghfp, &stateAsChar, 1);
	aghfpAtCmdEnd(aghfp);
	
	aghfpSendOk(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_CONFIRM_RESPONSE_HOLD_STATE_CFM, aghfp, aghfp_success);
}

void aghfpHandleSubscriberNumbersRequest(AGHFP *aghfp)
{
    if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
    {
    	MAKE_AGHFP_MESSAGE(AGHFP_SUBSCRIBER_NUMBER_IND);
	    message->aghfp = aghfp;
	    message->last_id = 0;
	    MessageSend(aghfp->client_task, AGHFP_SUBSCRIBER_NUMBER_IND, message);
    }
    else
	{   /* Command not supported by this profile version */
    	aghfpSendError(aghfp);
	}
}

void AghfpSendSubscriberNumber(AGHFP *aghfp, aghfp_subscriber_info *sub)
{
    if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
    {
		MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBER, sub->size_number);
    	message->type = sub->type;
    	message->service = sub->service;
    	message->size_number = sub->size_number;
		if ( sub->size_number )
		{
    	    memmove(message->number, sub->number, sub->size_number);
	    }
	    else
	    {
    	    *message->number = 0;
	    }
		
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBER, message);
    }
    else
	{	/* Feature not supported by this profile version */
		aghfpSendCommonCfmMessageToApp(AGHFP_SUBSCRIBER_NUMBER_CFM, aghfp, aghfp_fail);
	}
}

void AghfpSendSubscriberNumbersComplete(AGHFP *aghfp)
{
    if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
    {
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBERS_COMPLETE, 0);
    }
    else
	{	/* Feature not supported by this profile version */
		aghfpSendCommonCfmMessageToApp(AGHFP_SUBSCRIBER_NUMBER_CFM, aghfp, aghfp_fail);
	}
}

void aghfpHandleSendSubscriberNumber(AGHFP *aghfp, AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBER_T *res)
{
    char buf[16];
   	MAKE_AGHFP_MESSAGE(AGHFP_SUBSCRIBER_NUMBER_IND);
   	message->aghfp = aghfp;
   	message->last_id = res->id;
   	
	sprintf( buf, ", %d, , %d", res->type, res->service);
    
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CNUM: , ");
    aghfpAtCmdData(aghfp, res->number, res->size_number);
	aghfpAtCmdString(aghfp, buf);
    aghfpAtCmdEnd(aghfp);

   	MessageSend(aghfp->client_task, AGHFP_SUBSCRIBER_NUMBER_IND, message);
}

void aghfpHandleSendSubscriberNumbersComplete(AGHFP *aghfp)
{
   	aghfpSendOk(aghfp);
	aghfpSendCommonCfmMessageToApp(AGHFP_SUBSCRIBER_NUMBER_CFM, aghfp, aghfp_success);
}

void aghfpHandleCurrentCallsRequest(AGHFP *aghfp)
{
    if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
    {
    	MAKE_AGHFP_MESSAGE(AGHFP_CURRENT_CALLS_IND);
    	message->aghfp = aghfp;
    	message->last_idx = 0;
    	MessageSend(aghfp->client_task, AGHFP_CURRENT_CALLS_IND, message);
    }
    else
	{   /* Command not supported by this profile version */
    	aghfpSendError(aghfp);
	}
}

void AghfpSendCurrentCall(AGHFP *aghfp, aghfp_call_info *call)
{
    if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
    {
		MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_SEND_CURRENT_CALL, call->size_number);
    	message->idx = call->idx;
    	message->dir = call->dir;
    	message->status = call->status;
    	message->mode = call->mode;
    	message->mpty = call->mpty;
    	message->type = call->type;
    	message->size_number = call->size_number;
		if ( call->size_number )
		{
    	    memmove(message->number, call->number, call->size_number);
	    }
	    else
	    {
    	    *message->number = 0;
	    }
		
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_CURRENT_CALL, message);
    }
    else
	{	/* Feature not supported by this profile version */
		aghfpSendCommonCfmMessageToApp(AGHFP_CURRENT_CALLS_CFM, aghfp, aghfp_fail);
	}
}

void AghfpSendCurrentCallsComplete(AGHFP *aghfp)
{
    if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
    {
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_CURRENT_CALLS_COMPLETE, 0);
    }
    else
	{	/* Feature not supported by this profile version */
		aghfpSendCommonCfmMessageToApp(AGHFP_CURRENT_CALLS_CFM, aghfp, aghfp_fail);
	}
}

void aghfpHandleSendCurrentCall(AGHFP *aghfp, AGHFP_INTERNAL_SEND_CURRENT_CALL_T *res)
{
    char buf[16];
   	MAKE_AGHFP_MESSAGE(AGHFP_CURRENT_CALLS_IND);
   	message->aghfp = aghfp;
   	message->last_idx = res->idx;
   	
	sprintf( buf, "%d,%d,%d,%d,%d", res->idx, res->dir, res->status, res->mode, res->mpty);
    
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CLCC: ");
	aghfpAtCmdString(aghfp, buf);
	if ( res->size_number )
	{
    	aghfpAtCmdString(aghfp, ", ");
	    aghfpAtCmdData(aghfp, res->number, res->size_number);
    	sprintf( buf, ", %d", res->type);
		aghfpAtCmdString(aghfp, buf);
	}
    aghfpAtCmdEnd(aghfp);

   	MessageSend(aghfp->client_task, AGHFP_CURRENT_CALLS_IND, message);
}

void aghfpHandleSendCurrentCallsComplete(AGHFP *aghfp)
{
   	aghfpSendOk(aghfp);
	aghfpSendCommonCfmMessageToApp(AGHFP_CURRENT_CALLS_CFM, aghfp, aghfp_success);
}

void aghfpHandleNetworkOperatorFormatRequest(AGHFP *aghfp, uint8 mode, uint8 format)
{
    if ( supportedProfileHasHfp15Features(aghfp->supported_profile) && mode==3 && format==0 )
    {
    	aghfpSendOk(aghfp);
    }
    else
	{   /* Command not supported by this profile version */
    	aghfpSendError(aghfp);
	}
}

void aghfpHandleNetworkOperatorRequest(AGHFP *aghfp)
{
    if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
    {
	    MAKE_AGHFP_MESSAGE(AGHFP_NETWORK_OPERATOR_IND);
	    message->aghfp = aghfp;
	    MessageSend(aghfp->client_task, AGHFP_NETWORK_OPERATOR_IND, message);
    }
    else
	{   /* Command not supported by this profile version */
    	aghfpSendError(aghfp);
	}
}

void AghfpSendNetworkOperator(AGHFP *aghfp, uint8 mode, uint16 size_operator, uint8 *operator)
{
	if ( supportedProfileHasHfp15Features(aghfp->supported_profile) )
	{
		MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_SEND_NETWORK_OPERATOR, size_operator);
		message->mode = mode;
		message->size_operator = size_operator;
		memmove(message->operator, operator, size_operator);
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_NETWORK_OPERATOR, message);
	}
	else
	{	/* Feature not supported by this profile version */
		aghfpSendCommonCfmMessageToApp(AGHFP_NETWORK_OPERATOR_CFM, aghfp, aghfp_fail);
	}
}

void aghfpHandleSendNetworkOperator(AGHFP *aghfp, uint8 mode, uint16 size_operator, uint8* operator)
{
    char buf[2];
	sprintf( buf, "%d", mode);
    
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+COPS: ");
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ", 0, ");   /* HFP v1.5 only supports long alphanumeric format */
	aghfpAtCmdData(aghfp, operator, size_operator);
	aghfpAtCmdEnd(aghfp);
   	aghfpSendOk(aghfp);

	aghfpSendCommonCfmMessageToApp(AGHFP_NETWORK_OPERATOR_CFM, aghfp, aghfp_success);
}


/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#include "aghfp.h"
#include "aghfp_audio_handler.h"
#include "aghfp_call.h"
#include "aghfp_common.h"
#include "aghfp_indicators.h"
#include "aghfp_ok.h"
#include "aghfp_private.h"
#include "aghfp_send_data.h"

#include <bdaddr.h>
#include <panic.h>
#include <string.h>
#include <stdio.h>


/****************************************************************************
*/
void AghfpSetRingRepeatInterval(AGHFP *aghfp, uint16 interval)
{
	aghfp->ring_repeat_interval = interval;
}


/****************************************************************************
 The app has asked us to send a ring alert to the HF. Pass this request on
 to the profile handler
*/
void AghfpSendRingAlert(AGHFP *aghfp)
{
	MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_RING_ALERT, NULL);
}


/****************************************************************************
*/
void aghfpSendRingAlert(AGHFP *aghfp)
{
	/* Send ring to HF */
	aghfpSendAtCmd(aghfp, "RING");
}


/****************************************************************************
*/
void aghfpHandleSendRingAlert(AGHFP *aghfp)
{
	/* Send ring to HF */
	aghfpSendAtCmd(aghfp, "RING");

	/* Send CFM to app */
	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_RING_ALERT_CFM, aghfp, aghfp_success);
}


/****************************************************************************
 The app has asked us to send caller ID information to the HF. Pass this
 request on to the profile hadler
*/
void AghfpSendCallerId(AGHFP *aghfp, uint8 type_number, uint16 size_number, const uint8 *number, uint16 size_string, const uint8 *string)
{
	MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_SEND_CALLER_ID, size_number+size_string);

	if (message)
	{
    	message->type = type_number;
		message->size_number = size_number;
		message->size_string = size_string;
		memmove(message->data, number, size_number);
		memmove(message->data+size_number, string, size_string);
		
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_CALLER_ID, message);
	}
	else
	{
		free(message);
		aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALLER_ID_CFM, aghfp, aghfp_fail);
	}
}


/****************************************************************************
 The app has asked us to send caller ID information to the HF. The profile
 handler has asked us to act on this request.
*/
void aghfpSendCallerId(AGHFP *aghfp, uint8 type, uint16 size_number, uint16 size_string, uint8 *data)
{
	char buf[4];
	
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CLIP: ");
	
	if (size_number && data)
	{
		aghfpAtCmdData(aghfp, data, size_number);
	}
	
    aghfpAtCmdString(aghfp, ",");
	sprintf(buf, "%d", type);
	aghfpAtCmdString(aghfp, buf);
	
	if (size_string && data)
	{
		aghfpAtCmdString(aghfp, ",,,");
		aghfpAtCmdData(aghfp, data+size_number, size_string);
	}
	
	aghfpAtCmdEnd(aghfp);
}


/****************************************************************************
 The app has asked us to send caller ID information to the HF. The profile
 handler has asked us to act on this request.
*/
void aghfpHandleSendCallerId(AGHFP *aghfp, uint8 type, uint16 size_number, uint16 size_string, uint8 *data)
{
	aghfpSendCallerId(aghfp, type, size_number, size_string, data);
	
  	/* Tell the app about the new setting */
	aghfpSendCommonCfmMessageToApp(AGHFP_SEND_CALLER_ID_CFM, aghfp, aghfp_success);
}


/****************************************************************************
 The HF wants to answer the incoming call. The profile handler has asked us
 to act on this request.
*/
void aghfpHandleAnswer(AGHFP *aghfp)
{
    MAKE_AGHFP_MESSAGE(AGHFP_ANSWER_IND);
    message->aghfp = aghfp;

	/* Tell the app */
	MessageSend(aghfp->client_task, AGHFP_ANSWER_IND, message);
}


/****************************************************************************
 The app has asked us to enable/disable in band ringing. Send a message to
 the profile handler.
*/
void AghfpInBandRingToneEnable(AGHFP *aghfp, bool enable)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_INBAND_RING_TONE_ENABLE);
	message->enable = enable;
	MessageSend(&aghfp->task, AGHFP_INTERNAL_INBAND_RING_TONE_ENABLE, message);
}


/****************************************************************************
*/
void aghfpSendInBandRingToneEnable(AGHFP *aghfp, bool enable)
{
	const char *buf = "+BSIR: 0";

	if (enable)
	{
		buf = "+BSIR: 1";
	}

	/* Tell HF the new setting */
	aghfpSendAtCmd(aghfp, buf);
}


/****************************************************************************
 The app has asked us to tell the HF that we have enabled/disabled in band
 ringing. The profile handler has requested us to act on this instruction.
*/
void aghfpHandleInBandRingToneEnable(AGHFP *aghfp, bool enable)
{
	if (aghfp->supported_features & aghfp_inband_ring)
	{
		aghfpSendInBandRingToneEnable(aghfp, enable);
		
		/* Tell the app about the new setting */
		aghfpSendCommonCfmMessageToApp(AGHFP_INBAND_RING_ENABLE_CFM, aghfp, aghfp_success);

		/* Tell ourselves about the new setting */
		if (enable)
		{
			aghfp->features_status |= aghfp_feature_inband_ring;
		}
		else
		{
			aghfp->features_status &= ~aghfp_feature_inband_ring;
		}
	}
	else
	{
		/* This device doesn't support in band ringing, so enable/disable is an error */
		aghfpSendCommonCfmMessageToApp(AGHFP_INBAND_RING_ENABLE_CFM, aghfp, aghfp_fail);
	}
}


/****************************************************************************
 HF has rejected an incoming call. Profile handler has asked us to handle
 this instruction.
*/
void aghfpHandleCallHangUpReq(AGHFP *aghfp)
{
	MAKE_AGHFP_MESSAGE(AGHFP_CALL_HANG_UP_IND);
	message->aghfp = aghfp;

	/* Don't end the call here, wait for the app to call SendSetupIndicator
       in response to the AGHFP_CHUP_IND we send. */

	/* Tell the app */
	MessageSend(aghfp->client_task, AGHFP_CALL_HANG_UP_IND, message);
}


/****************************************************************************
 HF has request that a number be dialed. Profile handler has asked us to handle
 this request.
*/
void aghfpHandleDialReq(AGHFP *aghfp, uint8 *number, uint16 number_len)
{
	MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_DIAL_IND, number_len);

	if (!message)
    {
    	/* Tell the HF that we couldn't handle its request */
        aghfpSendError(aghfp);
    }
    else
    {
        message->aghfp = aghfp;
	    message->size_number = number_len;
		memmove(message->number, number, number_len * sizeof(uint8));

		/* Acknowledge the HF's request */
		aghfpSendOk(aghfp);

		/* Tell the app */
		MessageSend(aghfp->client_task, AGHFP_DIAL_IND, message);
	}
}


/****************************************************************************
 HF has request that a number be dialed from AG memory. Profile handler has
 asked us to handle this request.
*/
void aghfpHandleMemoryDialReq(AGHFP *aghfp, uint8 *number, uint16 number_len)
{
	MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_MEMORY_DIAL_IND, number_len);

	if (!message)
    {
    	/* Tell the HF that we couldn't handle its request */
        aghfpSendError(aghfp);
    }
    else
    {
        message->aghfp = aghfp;
	    message->size_number = number_len;
		memmove(message->number, number, number_len * sizeof(uint8));

		/* Tell the app */
		MessageSend(aghfp->client_task, AGHFP_MEMORY_DIAL_IND, message);
	}
}


/******************************************************************************
 HF has requested a "last number redial". Profile handler has asked us to handle
 this request.
*/
void aghfpHandleLastNumberRedialReq(AGHFP *aghfp)
{
	MAKE_AGHFP_MESSAGE(AGHFP_LAST_NUMBER_REDIAL_IND);
	message->aghfp = aghfp;

	/* Tell the app */
	MessageSend(aghfp->client_task, AGHFP_LAST_NUMBER_REDIAL_IND, message);
}


/******************************************************************************
 HF has requested that we perform some action to do with a waiting call.
 Profile handler has asked us to handle this request.
*/
void aghfpHandleCallHoldReq(AGHFP *aghfp, uint16 action, uint16 index)
{
	if (aghfp->features_status & aghfp_feature_call_waiting_notification)
    {
		if ( action<=4 )  /* Only handle AT+CHLD=(0,1,1x,2,2x,3,4) */
		{
			MAKE_AGHFP_MESSAGE(AGHFP_CALL_HOLD_IND);
			message->aghfp = aghfp;
            message->action = action;
            
            if ( supportedProfileHasHfp15Features(aghfp->supported_profile) && (action==1 || action==2) && index )
            {   /* Handling AT+CHLD=(1x,2x) - index will already be set appropriately */
                message->index = index;
            }
            else
            {   /* Handling AT+CHLD=(0,1,2,3,4) - message index is not used */
                message->index = 0;
            }
            
		    /* Acknowledge the HF's request */
		    aghfpSendOk(aghfp);

		    /* Tell the app */
		    MessageSend(aghfp->client_task, AGHFP_CALL_HOLD_IND, message);
	    }
	    else
        {
        	/* Command contained incorrect request - reject it */
        	aghfpSendError(aghfp);
        }
	}
    else
    {
    	/* Call waiting notification isn't enabled, so the HF shouldn't have sent this command */
    	aghfpSendError(aghfp);
    }
}


/******************************************************************************
	Eventually will cause AG to send an in-band ring tone to the connected HS/HF device.
*/
void aghfpSendInBandRingTone(AGHFP *aghfp)
{
	aghfp = aghfp;
}


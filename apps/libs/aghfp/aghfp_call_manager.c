/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#include "aghfp.h"
#include "aghfp_audio_handler.h"
#include "aghfp_call.h"
#include "aghfp_common.h"
#include "aghfp_indicators.h"
#include "aghfp_private.h"
#include "aghfp_slc_handler.h"
#include "aghfp_call_manager.h"
#include "aghfp_wbs.h"

/*#define DEBUG_PRINT_ENABLED 1*/

#include <stdio.h>
#include <panic.h>
#include <print.h>
#include <bdaddr.h>
#include <string.h>


static bool usingHsp (AGHFP *aghfp)
{
	if ( supportedProfileIsHsp(aghfp->supported_profile) )
	{
		return TRUE;
	}

	return FALSE;
}


static bool haveAudio (AGHFP *aghfp)
{
	if (aghfp->audio_sink!=NULL)
	{
		return TRUE;
	}
	
	return FALSE;
}


static void clearCallFlags (AGHFP *aghfp, aghfp_call_flags call_flag)
{
	aghfp->call_flags &= ~call_flag;
}


static void setCallFlags (AGHFP *aghfp, aghfp_call_flags call_flag)
{
	aghfp->call_flags |= call_flag;
}


static bool checkCallFlag (AGHFP *aghfp, aghfp_call_flags call_flag)
{
	return aghfp->call_flags & call_flag?TRUE:FALSE;
}


static void setCallProgress (AGHFP *aghfp, aghfp_call_progress progress)
{
	PRINT(("\nsetCallProgress %u", (uint16)progress));
	aghfp->call_progress = progress;
	
	if ( progress==CallProgressIdle )
	{
		clearCallFlags(aghfp, CallFlagsAll);
		aghfpResetCallerIdDetails(aghfp);
	}
}


static bool inbandSupported (AGHFP *aghfp)
{
	if ( (aghfp->supported_features & aghfp_inband_ring) && aghfp->call_params.in_band )
	{
		return TRUE;
	}
	
	return FALSE;
}


static aghfp_disconnect_status convertCallFlagToDisconnectStatus (aghfp_call_flags call_flag)
{
	aghfp_disconnect_status status;
	
	switch ( call_flag )
	{
	case CallFlagSuccess:
		status = aghfp_disconnect_success;
		break;
	case CallFlagLinkLoss:
		status = aghfp_disconnect_link_loss;
		break;
	case CallFlagTimeOut:
		status = aghfp_disconnect_timeout;
		break;
	case CallFlagFail:
	default:
		status = aghfp_disconnect_error;
		break;
	}
	
	return status;
}


static void sendCallCreateWaitingIndToApp (AGHFP *aghfp)
{
	if (aghfp->call_params.call_type == aghfp_call_type_incoming )
	{
		MAKE_AGHFP_MESSAGE(AGHFP_CALL_MGR_CREATE_WAITING_RESPONSE_IND);
		message->aghfp = aghfp;
		message->audio_sink = aghfp->audio_sink;
		MessageSend(aghfp->client_task, AGHFP_CALL_MGR_CREATE_WAITING_RESPONSE_IND, message);
	}
	else if (aghfp->call_params.call_type == aghfp_call_type_outgoing )
	{
		MAKE_AGHFP_MESSAGE(AGHFP_CALL_MGR_CREATE_ALERTING_REMOTE_IND);
		message->aghfp = aghfp;
		message->audio_sink = aghfp->audio_sink;
		MessageSend(aghfp->client_task, AGHFP_CALL_MGR_CREATE_ALERTING_REMOTE_IND, message);
	}
}


void aghfpSendCallCreateCfmToApp (AGHFP *aghfp, aghfp_call_create_status status)
{
	MAKE_AGHFP_MESSAGE(AGHFP_CALL_MGR_CREATE_CFM);
	message->aghfp = aghfp;
	message->status = status;
    message->link_type = aghfp->link_type;
	message->audio_sink = aghfp->audio_sink;
    message->rx_bandwidth = aghfp->rx_bandwidth;
    message->tx_bandwidth = aghfp->tx_bandwidth;

	/* Let the application know if a WBS codec is required. */
    if ((aghfp->use_wbs) && (aghfp->use_codec > 0))
    {
	    message->using_wbs = TRUE;
	    message ->wbs_codec = aghfp->use_codec;
    }
    else
	    message->using_wbs = FALSE;
	
	MessageSend(aghfp->client_task, AGHFP_CALL_MGR_CREATE_CFM, message);
}


static void sendCallTerminateIndToApp (AGHFP *aghfp, aghfp_call_terminate_status status)
{
	MAKE_AGHFP_MESSAGE(AGHFP_CALL_MGR_TERMINATE_IND);
	message->aghfp = aghfp;
	message->status = status;
	message->audio_sink = aghfp->audio_sink;
	MessageSend(aghfp->client_task, AGHFP_CALL_MGR_TERMINATE_IND, message);
}


static void sendRingNotifications (AGHFP *aghfp)
{
	if (aghfp->ring_repeat_interval != 0)
	{
		aghfpSendRingAlert(aghfp);
		MessageSendLater(&aghfp->task, AGHFP_INTERNAL_RING_REPEAT_REQ, 0, aghfp->ring_repeat_interval);

		if ( !usingHsp(aghfp) && (aghfp->hf_supported_features & aghfp_hf_cli_presentation) )
		{
			aghfp_call_details *call_details = &aghfp->call_params.call_details;
			aghfpSendCallerId(aghfp, call_details->type_number, 
			                  call_details->size_number, call_details->size_alpha, 
		    	              call_details->data);
		}
		
		if ( haveAudio(aghfp) && inbandSupported(aghfp) )
		{
			aghfpSendInBandRingTone(aghfp);
		}
	}
}


static void manageCallIdle (AGHFP *aghfp, aghfp_call_event call_event, aghfp_call_flags call_flags)
{
	switch ( call_event )
	{
	case CallEventCreate:
		clearCallFlags(aghfp, CallFlagsAll);
		setCallFlags(aghfp, call_flags & (CallFlagsAll));
		
		switch (aghfp->call_params.call_type)
		{
		case aghfp_call_type_incoming:
			/* Always require a response and ring indications for this type of call */
			setCallFlags(aghfp, CallFlagResponseReqd | CallFlagRingAlertsReqd);
			
			if ( usingHsp(aghfp) )
			{	/* Working in HSP mode */
				/* HSP mandates that audio must be present */
				setCallFlags(aghfp, CallFlagOpenAudio);  /* B-11840: what if audio params are unspecified? */
				if ( inbandSupported(aghfp) )
				{	/* Ensure audio exists before an answer call response is actioned */
					setCallFlags(aghfp, CallFlagResponseAfterAudio);
				}
			}
			else
			{	/* Working in HFP mode */
				aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_incoming);
				
				if ( inbandSupported(aghfp) && (haveAudio(aghfp) || checkCallFlag(aghfp, CallFlagOpenAudio)) )
				{
					setCallFlags(aghfp, CallFlagResponseAfterAudio);
					aghfpSendInBandRingToneEnable(aghfp, TRUE);
				}
				else
				{
					aghfpSendInBandRingToneEnable(aghfp, FALSE);
				}
			}
	        aghfpSetState(aghfp, aghfp_incoming_call_establish);
			break;
			
		case aghfp_call_type_outgoing:
			/* Always require a response, but not ring alerts, for this type of call */
			setCallFlags(aghfp, CallFlagResponseReqd | CallFlagResponseAfterAudio);
			clearCallFlags(aghfp, CallFlagRingAlertsReqd);
			
			aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_outgoing);
			if ( haveAudio(aghfp) || !checkCallFlag(aghfp, CallFlagOpenAudio) )
			{
				aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_remote_alert);
			}
	        aghfpSetState(aghfp, aghfp_outgoing_call_establish);
			break;
			
		case aghfp_call_type_transfer:
			/* Never require a response or ring alerts for this type of call */
			clearCallFlags(aghfp, CallFlagResponseReqd | CallFlagResponseAfterAudio | CallFlagRingAlertsReqd);
			
			if ( usingHsp(aghfp) )
			{	/* Working in HSP mode */
				/* HSP mandates that audio must be present */
				setCallFlags(aghfp, CallFlagOpenAudio);  /* B-11840: what if audio params are unspecified? */
			}
			else
			{	/* Working in HFP mode */
				/* Call is now setup even though we might not have an audio channel */
			}
			break;
		}
		
		if ( !haveAudio(aghfp) && checkCallFlag(aghfp, CallFlagOpenAudio) && !(checkCallFlag(aghfp, CallFlagResponseReqd) && !checkCallFlag(aghfp, CallFlagResponseAfterAudio)) )
		{	/* Open an audio channel */
			clearCallFlags(aghfp, CallFlagOpenAudio);
			
			if ( usingHsp(aghfp) && checkCallFlag(aghfp, CallFlagRingAlertsReqd) )
			{
				sendRingNotifications(aghfp);
			}
			
			aghfpAudioConnectRequest(aghfp);

			/* Check to see if aghfpAudioConnectRequest resulted in the commencement of a WBS negotiation */
			if(aghfp->audio_connection_state != aghfp_audio_codec_connect)
			{
				/* Only move to the CallProgressWaitAudioOpen state if a WBS negotiation has not been started. */
				setCallProgress(aghfp, CallProgressWaitAudioOpen);
			}
		}
		else if ( checkCallFlag(aghfp, CallFlagResponseReqd) && !(checkCallFlag(aghfp, CallFlagOpenAudio) && checkCallFlag(aghfp, CallFlagResponseAfterAudio)) )
		{	/* Wait for a repsonse from remote end */
			clearCallFlags(aghfp, CallFlagResponseReqd);
			
			if ( checkCallFlag(aghfp, CallFlagRingAlertsReqd) )
			{
				sendRingNotifications(aghfp);
			}
			
			sendCallCreateWaitingIndToApp(aghfp);
			setCallProgress(aghfp, CallProgressWaitResponse);
		}
		else
		{	/* Call setup is complete */
			aghfpSendCallIndicator(aghfp, aghfp_call_active);
			aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_none);
			setCallProgress(aghfp, CallProgressComplete);
            aghfpSetState(aghfp, aghfp_active_call);
            aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_success);
		}
		break;
		
	case CallEventAnswer:
	case CallEventRemoteAnswered:
	case CallEventTerminate:
    case CallEventTerminateHeld:
    case CallEventAcceptHeld:
	case CallEventAudioConnected:
	case CallEventAudioDisconnected:
	case CallEventAudioTransfer:
	case CallEventRing:
		break;
		
	case CallEventSlcRemoved:
		/* SLC has been removed */
		setCallProgress(aghfp, CallProgressIdle);
		aghfpSetState(aghfp, aghfp_ready);
		aghfpSendSlcDisconnectIndToApp(aghfp, convertCallFlagToDisconnectStatus(call_flags));
		break;
		
	default:
		Panic();
	}
}


static void manageCallWaitAudioOpen (AGHFP *aghfp, aghfp_call_event call_event, aghfp_call_flags call_flags)
{
	switch ( call_event )
	{
	case CallEventCreate:
	case CallEventAnswer:
	case CallEventRemoteAnswered:
		break;
		
	case CallEventTerminate:
		/* Queue up call terminate request until audio connect completes */
		setCallFlags(aghfp, call_flags | CallFlagTerminate);
		break;
		
	case CallEventAudioConnected:
		setCallFlags(aghfp, call_flags & (CallFlagsAll));
		
		if ( !checkCallFlag(aghfp, CallFlagSuccess) )
		{	/* Audio connect failed */
			if ( !usingHsp(aghfp) )
			{
				aghfpSendCallIndicator(aghfp, aghfp_call_none);
				aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_none);
			}
			
			setCallProgress(aghfp, CallProgressIdle);
            aghfpSetState(aghfp, aghfp_slc_connected);
            aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_failure);
		}
		else if ( checkCallFlag(aghfp, CallFlagTerminate) )
		{	/* Call terminate request has been made */
			clearCallFlags(aghfp, CallFlagTerminate);
			
			if ( haveAudio(aghfp) && checkCallFlag(aghfp, CallFlagCloseAudio) )
			{	/* Shut down audio connection */
				clearCallFlags(aghfp, CallFlagCloseAudio);
				aghfpAudioDisconnectRequest(aghfp);
				setCallProgress(aghfp, CallProgressWaitAudioCancel);
			}
			else
			{	/* Either audio connection does not exist or it is to remain open */
				if ( !usingHsp(aghfp) )
				{
					aghfpSendCallIndicator(aghfp, aghfp_call_none);
					aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_none);
				}
				
				setCallProgress(aghfp, CallProgressIdle);
	            aghfpSetState(aghfp, aghfp_slc_connected);
	            aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_aborted);
			}
		}
		else if ( checkCallFlag(aghfp, CallFlagResponseReqd) )
		{
			if ( checkCallFlag(aghfp, CallFlagRingAlertsReqd) )
			{
				sendRingNotifications(aghfp);
			}
			
			clearCallFlags(aghfp, CallFlagResponseReqd);
			sendCallCreateWaitingIndToApp(aghfp);
			setCallProgress(aghfp, CallProgressWaitResponse);
		}
		else
		{	/* Call setup is now complete */
			if ( !checkCallFlag(aghfp, CallFlagResponded) && !usingHsp(aghfp) )
			{
				aghfpSendCallIndicator(aghfp, aghfp_call_active);
				aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_none);
			}
		
			clearCallFlags(aghfp, CallFlagsAll);
			setCallProgress(aghfp, CallProgressComplete);
        	aghfpSetState(aghfp, aghfp_active_call);
            aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_success);
		}
		break;
		
	case CallEventAudioDisconnected:
	case CallEventAudioTransfer:
	case CallEventRing:
    case CallEventTerminateHeld:
    case CallEventAcceptHeld:
		break;
		
	case CallEventSlcRemoved:
		/* SLC has been removed */
		setCallProgress(aghfp, CallProgressIdle);
		aghfpSetState(aghfp, aghfp_ready);
        aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_slc_removed);
		aghfpSendSlcDisconnectIndToApp(aghfp, convertCallFlagToDisconnectStatus(call_flags));
		break;
		
	default:
		Panic();
	}
}


static void manageCallWaitResponse (AGHFP *aghfp, aghfp_call_event call_event, aghfp_call_flags call_flags)
{
	switch ( call_event )
	{
	case CallEventCreate:
		break;
		
	case CallEventAnswer:
	case CallEventRemoteAnswered:
		setCallFlags(aghfp, call_flags | CallFlagResponded);
		
		if ( !usingHsp(aghfp) &&
		     aghfp->audio_connection_state != aghfp_audio_codec_connect) /* Do not resend the call indicator info
		     															  if we got here from a WBS codec
		     															  negotiation */
		{	/* Working in HFP mode */
			/* Call is now setup even though we might not have an audio channel */
			aghfpSendCallIndicator(aghfp, aghfp_call_active);
			aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_none);
		}
		
		if ( checkCallFlag(aghfp, CallFlagCloseAudio) )
		{	/* Use CloseAudio flag to shutdown any existing audio channel or prevent one being created */
			clearCallFlags(aghfp, CallFlagOpenAudio | CallFlagCloseAudio);
			
			if ( haveAudio(aghfp) )
			{	/* Audio disconnected event will be picked up in CallConnected state */
				aghfpAudioDisconnectRequest(aghfp);
			}
		}
		
		if ( checkCallFlag(aghfp, CallFlagOpenAudio) && !haveAudio(aghfp))
		{
			clearCallFlags(aghfp, CallFlagOpenAudio);
			aghfpAudioConnectRequest(aghfp);

			/* Check to see if aghfpAudioConnectRequest resulted in the commencement of a WBS negotiation */
			if(aghfp->audio_connection_state != aghfp_audio_codec_connect)
			{
				/* Only move to the CallProgressWaitAudioOpen state if a WBS negotiation has not been started. */
				setCallProgress(aghfp, CallProgressWaitAudioOpen);
			}
		}
		else
		{
			setCallProgress(aghfp, CallProgressComplete);
            aghfpSetState(aghfp, aghfp_active_call);
            aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_success);
		}
		break;
		
	case CallEventTerminate:
		setCallFlags(aghfp, call_flags & (CallFlagsAll));
		
		if ( checkCallFlag(aghfp, CallFlagCloseAudio) && haveAudio(aghfp) )
		{
			aghfpAudioDisconnectRequest(aghfp);
			setCallProgress(aghfp, CallProgressWaitAudioCancel);
		}
		else
		{
            if ( !usingHsp(aghfp) )
			{	/* Working in HFP mode */
				aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_none);
			}
			setCallProgress(aghfp, CallProgressIdle);
            aghfpSetState(aghfp, aghfp_slc_connected);
            aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_aborted);
		}
		break;
        
    case CallEventTerminateHeld:
        /* duplicate of CallEventTerminate but without the aghfpSendCallSetupIndicator */
        setCallFlags(aghfp, CallFlagCloseAudio);
        setCallProgress(aghfp, CallProgressIdle);
        aghfpSetState(aghfp, aghfp_slc_connected);
        aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_aborted);
        
        break;
        
    case CallEventAcceptHeld:
        /*duplicate of CallEventAnswer but without the aghfpSendCallIndicator and aghfpSendCallSetupIndicator */
        /* still need to change aghfp state */
        setCallFlags(aghfp, call_flags | CallFlagResponded);
		aghfpSetState(aghfp, aghfp_active_call);
        /* Use CloseAudio flag to shutdown any existing audio channel or prevent one being created */
        clearCallFlags(aghfp, CallFlagOpenAudio | CallFlagCloseAudio);
			
        if ( haveAudio(aghfp) )
        {	/* Audio disconnected event will be picked up in CallConnected state */
            aghfpAudioDisconnectRequest(aghfp);
        }
        
        setCallProgress(aghfp, CallProgressComplete);
        aghfpSetState(aghfp, aghfp_active_call);
        aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_success);
		break;
        
	case CallEventAudioConnected:
	case CallEventAudioDisconnected:
	case CallEventAudioTransfer:
		break;
		
	case CallEventRing:
		sendRingNotifications(aghfp);
		break;
		
	case CallEventSlcRemoved:
		/* SLC has been removed */
		setCallProgress(aghfp, CallProgressIdle);
		aghfpSetState(aghfp, aghfp_ready);
        aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_slc_removed);
		aghfpSendSlcDisconnectIndToApp(aghfp, convertCallFlagToDisconnectStatus(call_flags));
		break;
		
	default:
		Panic();
	}
}


static void manageCallConnected (AGHFP *aghfp, aghfp_call_event call_event, aghfp_call_flags call_flags)
{
	switch ( call_event )
	{
	case CallEventCreate:
	case CallEventAnswer:
	case CallEventRemoteAnswered:
    case CallEventTerminateHeld:
    case CallEventAcceptHeld:
		break;
		
	case CallEventTerminate:
		setCallFlags(aghfp, call_flags & (CallFlagsAll));
		
		if ( haveAudio(aghfp) && checkCallFlag(aghfp, CallFlagCloseAudio) )
		{
			clearCallFlags(aghfp, CallFlagCloseAudio);
			aghfpAudioDisconnectRequest(aghfp);
			setCallProgress(aghfp, CallProgressWaitAudioClose);
		}
		else
		{
			if ( !usingHsp(aghfp) )
			{
				aghfpSendCallIndicator(aghfp, aghfp_call_none);
			}
			setCallProgress(aghfp, CallProgressIdle);
            aghfpSetState(aghfp, aghfp_slc_connected);
			sendCallTerminateIndToApp(aghfp, aghfp_call_terminate_success);
		}
		break;
		
	case CallEventAudioConnected:
	case CallEventAudioDisconnected:
		break;
		
	case CallEventAudioTransfer:
		setCallFlags(aghfp, call_flags & (CallFlagsAll));
		
		if ( checkCallFlag(aghfp, CallFlagCloseAudio) )
		{
			if ( haveAudio(aghfp) )
			{	/* Audio with HS - disconnect audio channel */
				aghfpAudioDisconnectRequest(aghfp);
			}
		}
		else if ( checkCallFlag(aghfp, CallFlagOpenAudio) )
		{
			if ( !haveAudio(aghfp) )
			{  /* Audio with AG - create an audio channel */
				aghfpAudioConnectRequest(aghfp);
			}
		}
		break;
		
	case CallEventRing:
		break;
		
	case CallEventSlcRemoved:
		/* SLC has been removed */
		setCallProgress(aghfp, CallProgressIdle);
		aghfpSetState(aghfp, aghfp_ready);
		sendCallTerminateIndToApp(aghfp, aghfp_call_terminate_success);
		aghfpSendSlcDisconnectIndToApp(aghfp, convertCallFlagToDisconnectStatus(call_flags));
		break;
	default:
		Panic();
	}
}


static void manageCallWaitAudioCancel (AGHFP *aghfp, aghfp_call_event call_event, aghfp_call_flags call_flags)
{
	switch ( call_event )
	{
	case CallEventCreate:
	case CallEventAnswer:
	case CallEventRemoteAnswered:
	case CallEventTerminate:
	case CallEventAudioConnected:
		break;
		
	case CallEventAudioDisconnected:
		setCallFlags(aghfp, call_flags & (CallFlagsAll));
		
		if ( checkCallFlag(aghfp, CallFlagSuccess) )
		{	/* Audio disconnect succeeded */
			clearCallFlags(aghfp, CallFlagSuccess);
			
			if ( !usingHsp(aghfp) )
			{
				aghfpSendCallIndicator(aghfp, aghfp_call_none);
				aghfpSendCallSetupIndicator(aghfp, aghfp_call_setup_none);
			}
			setCallProgress(aghfp, CallProgressIdle);
            aghfpSetState(aghfp, aghfp_slc_connected);
            aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_aborted);
		}
		else
		{	/* Audio disconnect failed */
			Panic();  /* B-11841: Should do something more meaningful here */
		}
		break;
		
	case CallEventAudioTransfer:
	case CallEventRing:
    case CallEventTerminateHeld:
    case CallEventAcceptHeld:
		break;
		
	case CallEventSlcRemoved:
		/* SLC has been removed */
		setCallProgress(aghfp, CallProgressIdle);
		aghfpSetState(aghfp, aghfp_ready);
        aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_slc_removed);
		aghfpSendSlcDisconnectIndToApp(aghfp, convertCallFlagToDisconnectStatus(call_flags));
		break;
		
	default:
		Panic();
	}
}


static void manageCallWaitAudioClose (AGHFP *aghfp, aghfp_call_event call_event, aghfp_call_flags call_flags)
{
	switch ( call_event )
	{
	case CallEventCreate:
	case CallEventAnswer:
	case CallEventRemoteAnswered:
	case CallEventTerminate:
	case CallEventAudioConnected:
    case CallEventTerminateHeld:
    case CallEventAcceptHeld:
		break;
		
	case CallEventAudioDisconnected:
		setCallFlags(aghfp, call_flags & (CallFlagsAll));
		
		if ( checkCallFlag(aghfp, CallFlagSuccess) )
		{	/* Audio disconnect succeeded */
			clearCallFlags(aghfp, CallFlagSuccess);
			
			if ( !usingHsp(aghfp) )
			{
				aghfpSendCallIndicator(aghfp, aghfp_call_none);
			}
			setCallProgress(aghfp, CallProgressIdle);
            aghfpSetState(aghfp, aghfp_slc_connected);
			sendCallTerminateIndToApp(aghfp, aghfp_call_terminate_success);
		}
		else
		{	/* Audio disconnect failed */
			Panic();  /* B-11841: Should do something more meaningful here */
		}
		break;
		
	case CallEventAudioTransfer:
	case CallEventRing:
		break;
		
	case CallEventSlcRemoved:
		/* SLC has been removed */
		setCallProgress(aghfp, CallProgressIdle);
		aghfpSetState(aghfp, aghfp_ready);
		sendCallTerminateIndToApp(aghfp, aghfp_call_terminate_success);
		aghfpSendSlcDisconnectIndToApp(aghfp, convertCallFlagToDisconnectStatus(call_flags));
		break;
		
	default:
		Panic();
	}
}


aghfp_call_flags aghfpConvertDisconnectStatusToCallFlag (aghfp_disconnect_status status)
{
	aghfp_call_flags call_flag;
	
	switch ( status )
	{
	case aghfp_disconnect_success:
		call_flag = CallFlagSuccess;
		break;
	case aghfp_disconnect_link_loss:
		call_flag = CallFlagLinkLoss;
		break;
	case aghfp_disconnect_timeout:
		call_flag = CallFlagTimeOut;
		break;
	default:
		call_flag = CallFlagFail;
		break;
	}
	
	return call_flag;
}


/****************************************************************************
	Returns true if Call Manager is processing a call, false otherwise
*/
bool aghfpCallManagerActive (AGHFP *aghfp)
{
	return (aghfp->call_progress!=CallProgressIdle);
}


/****************************************************************************
	Returns true if Call Manager is setting up or shutting down a call, false otherwise
*/
bool aghfpCallManagerActiveNotComplete (AGHFP *aghfp)
{
	return ((aghfp->call_progress!=CallProgressIdle) && (aghfp->call_progress!=CallProgressComplete));
}


/****************************************************************************
	Main entry to Call Manager sate machine.
*/
void aghfpManageCall (AGHFP *aghfp, aghfp_call_event call_event, uint16 event_value)
{
	PRINT(("\naghfpManageCall 0x%X - %u[%u,%u]\n", (uint16)aghfp, (uint16)aghfp->call_progress, (uint16)call_event, event_value));
	
	switch ( aghfp->call_progress )
	{
	case CallProgressIdle:
		manageCallIdle(aghfp, call_event, event_value);
		break;
	case CallProgressWaitAudioOpen:
		manageCallWaitAudioOpen(aghfp, call_event, event_value);
		break;
	case CallProgressWaitResponse:
		manageCallWaitResponse(aghfp, call_event, event_value);
		break;
	case CallProgressWaitAudioCancel:
		manageCallWaitAudioCancel(aghfp, call_event, event_value);
		break;
	case CallProgressComplete:
		manageCallConnected(aghfp, call_event, event_value);
		break;
	case CallProgressWaitAudioClose:
		manageCallWaitAudioClose(aghfp, call_event, event_value);
		break;
	default:
		Panic();
	}
}


/****************************************************************************
	Internal library function to reset the caller id details.
*/
void aghfpResetCallerIdDetails (AGHFP *aghfp)
{
	aghfp->call_params.call_details.type_number = 0;
	aghfp->call_params.call_details.size_number = 0;
	aghfp->call_params.call_details.size_alpha = 0;
	
	if ( aghfp->call_params.call_details.data )
	{
		free(aghfp->call_params.call_details.data);
		aghfp->call_params.call_details.data = 0;
	}
}


/****************************************************************************
	Sets the Caller Id details for +CLIP notifications.
*/
void AghfpSetCallerIdDetails (AGHFP *aghfp, uint8 type_number, uint16 size_number, const uint8 *number, uint16 size_string, const uint8 *string)
{
	uint16 size_data = 0;
	if ( number!=NULL )
	{
		size_data += size_number;
	}
	if ( string!=NULL )
	{
		size_data += size_string;
	}
	
	{
		MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ, size_data);
	
		message->type_number = type_number;
		message->size_number = size_number;
		message->size_alpha = size_string;
		memmove(message->data, number, size_number);
		memmove(message->data + size_number, string, size_string);
		
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ, message);
	}
}


/****************************************************************************
	Profile handler function called in response to issuing an AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ
	message.
*/
void aghfpHandleSetCallerIdDetails (AGHFP *aghfp, AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ_T *req)
{
	uint16 size_data = req->size_number + req->size_alpha;
	
	aghfp->call_params.call_details.type_number = req->type_number;
	aghfp->call_params.call_details.size_number = req->size_number;
	aghfp->call_params.call_details.size_alpha = req->size_alpha;
	
	if ( aghfp->call_params.call_details.data )
	{
		free(aghfp->call_params.call_details.data);
		aghfp->call_params.call_details.data = 0;
	}
	
	if ( size_data )
	{
		aghfp->call_params.call_details.data = malloc(size_data);
	}
	
	if (aghfp->call_params.call_details.data)
	{
		memmove(aghfp->call_params.call_details.data, req->data, size_data);
	}

	aghfpSendCommonCfmMessageToApp(AGHFP_SET_CALLER_ID_DETAILS_CFM, aghfp, aghfp_success);
}


/****************************************************************************
	Enables AG to setup a call to a HS/HF device.
*/
void AghfpCallCreate (AGHFP *aghfp, aghfp_call_type call_type, bool in_band)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_CALL_MGR_CREATE_REQ);
	message->call_type = call_type;
	message->in_band = in_band;
	
	MessageSend(&aghfp->task, AGHFP_INTERNAL_CALL_MGR_CREATE_REQ, message);
}


/****************************************************************************
	Profile handler function called in response to issuing an AGHFP_INTERNAL_CALL_MGR_CREATE_REQ
	message.  Will actually begin the call creation process.
*/
void aghfpHandleCallCreate (AGHFP *aghfp, AGHFP_INTERNAL_CALL_MGR_CREATE_REQ_T *req)
{
	/* B-11839: should validate parameters here? */
	if ( aghfp->audio_connection_state==aghfp_audio_disconnected || aghfp->audio_connection_state==aghfp_audio_connected )
	{
		aghfp->call_params.call_type = req->call_type;
		aghfp->call_params.in_band = req->in_band;
	
		aghfpManageCall(aghfp, CallEventCreate, CallFlagsNone);
	}
	else
	{
		aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_audio_handler_active);
	}
}


/****************************************************************************
	Enables AG to setup a call to a HS/HF device, creating an audio connection in the process.
*/
void AghfpCallCreateAudio (AGHFP *aghfp, aghfp_call_type call_type, bool in_band, sync_pkt_type packet_type, const aghfp_audio_params *audio_params)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ);
	message->call_type = call_type;
	message->in_band = in_band;
	message->packet_type = packet_type;
	
	if (audio_params)
	{
		message->audio_params = *audio_params;
	}
	else
	{
		message->audio_params = *aghfpGetDefaultAudioParams(packet_type);
	}
	
	MessageSend(&aghfp->task, AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ, message);
}


/****************************************************************************
	Profile handler function called in response to issuing an AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ
	message.  Will actually begin the call creation process.
*/
void aghfpHandleCallCreateAudio (AGHFP *aghfp, AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ_T *req)
{
	/* B-11839: should validate parameters here? */
	if ( aghfp->audio_connection_state!=aghfp_audio_connected )
	{
		if ( aghfp->audio_connection_state==aghfp_audio_disconnected ||
		     aghfp->audio_connection_state==aghfp_audio_codec_connect ) /* Carry on if we've just completed codec negotiation. */
		{
			if ( aghfpStoreAudioParams(aghfp, req->packet_type, &req->audio_params) )
			{
				aghfp->call_params.call_type = req->call_type;
				aghfp->call_params.in_band = req->in_band;
			
				aghfpManageCall(aghfp, CallEventCreate, CallFlagOpenAudio);
			}
			else
			{
				aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_invalid_params);
			}
		}
		else
		{
			aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_audio_handler_active);
		}
	}
	else
	{
		aghfpSendCallCreateCfmToApp(aghfp, aghfp_call_create_have_audio);
	}
}


/****************************************************************************
	Enables AG to answer a call.
*/
void AghfpCallAnswer (AGHFP *aghfp)
{
	MessageSend(&aghfp->task, AGHFP_INTERNAL_CALL_MGR_ANSWER_REQ, 0);
}


/****************************************************************************
	Profile handler function called in response to issuing an AGHFP_INTERNAL_CALL_MGR_ANSWER_REQ
	message.
*/
void aghfpHandleCallAnswer (AGHFP *aghfp)
{
	/* B-11839: should validate parameters here? */
	if ( aghfp->call_progress==CallProgressWaitResponse && aghfp->call_params.call_type==aghfp_call_type_incoming )
	{	/* Call being answered at AG, close any existing audio link to HS */
		aghfpManageCall(aghfp, CallEventAnswer, CallFlagCloseAudio);
	}
}


/****************************************************************************
	Informs Call Manager that the remote party has answered the outgoing call.
*/
void AghfpCallRemoteAnswered (AGHFP *aghfp)
{
	MessageSend(&aghfp->task, AGHFP_INTERNAL_CALL_MGR_REMOTE_ANSWERED_REQ, 0);
}


/****************************************************************************
	Profile handler function called in response to issuing an AGHFP_INTERNAL_CALL_MGR_REMOTE_ANSWERED_REQ
	message.
*/
void aghfpHandleCallRemoteAnswered (AGHFP *aghfp)
{
	if ( aghfp->call_progress==CallProgressWaitResponse && aghfp->call_params.call_type==aghfp_call_type_outgoing )
	{
		aghfpManageCall(aghfp, CallEventRemoteAnswered, CallFlagsNone);
	}
}


/****************************************************************************
	Enables AG to reject a new call/terminate an existing call.
*/
void AghfpCallTerminate (AGHFP *aghfp, bool keep_sink)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ);
	message->keep_sink = keep_sink;
	
	MessageSend(&aghfp->task, AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ, message);
}


/****************************************************************************
	Profile handler function called in response to issuing an AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ
	message.
*/
void aghfpHandleCallTerminate (AGHFP *aghfp, AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ_T *req)
{
	if ( aghfp->call_progress!=CallProgressIdle )
	{
		aghfpManageCall(aghfp, CallEventTerminate, (req->keep_sink)?CallFlagsNone:CallFlagCloseAudio);
	}
}

/****************************************************************************
	Enables AG to reject a held incoming call.
*/
void AghfpHeldCallTerminate (AGHFP *aghfp)
{
	MessageSend(&aghfp->task, AGHFP_INTERNAL_CALL_MGR_TERMINATE_HELD_REQ, 0);
}


/****************************************************************************
	Profile handler function called in response to issuing an AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ
	message.
*/
void aghfpHandleHeldCallTerminate (AGHFP *aghfp)
{
	if ( aghfp->call_progress!=CallProgressIdle )
	{
		aghfpManageCall(aghfp, CallEventTerminateHeld, CallFlagsNone);
	}
}

/****************************************************************************
	Enables AG to reject a held incoming call.
*/
void AghfpHeldCallAccept (AGHFP *aghfp)
{
	MessageSend(&aghfp->task, AGHFP_INTERNAL_CALL_MGR_ACCEPT_HELD_REQ, 0);
}


/****************************************************************************
	Profile handler function called in response to issuing an AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ
	message.
*/
void aghfpHandleHeldCallAccept (AGHFP *aghfp)
{
	if ( aghfp->call_progress!=CallProgressIdle )
	{
		aghfpManageCall(aghfp, CallEventAcceptHeld, CallFlagCloseAudio);
	}
}



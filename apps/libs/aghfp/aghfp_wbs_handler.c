/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
	aghfp_wbs_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_common.h"
#include "aghfp_wbs.h"
#include "aghfp_wbs_handler.h"
#include "aghfp_send_data.h"
#include "aghfp_ok.h"
#include "aghfp_call_manager.h"
#include "aghfp_audio_handler.h"

#include <panic.h>
#include <stdio.h>
#include <string.h>

/* WB-Speech parameters for an eSCO connection.  */
static const aghfp_audio_params wb_speech_esco_audio_params_sbc =
{
	8000,						/* Bandwidth for both Tx and Rx */
	0x000e, 					/* Max Latency					*/
	sync_air_coding_transparent,/* Voice Settings				*/
	sync_retx_link_quality,		/* Retransmission Effort		*/
	FALSE						/* Use WB-Speech if available	*/
};

static const aghfp_audio_params wb_speech_esco_audio_params_amr =
{
    3000,                    	/* Bandwidth for both Tx and Rx */
    0x000e,                  	/* Max Latency                  */
    sync_air_coding_transparent,/* Voice Settings               */
    sync_retx_link_quality,    	/* Retransmission Effort        */
    FALSE				     	/* Use WB-Speech if available   */
};


/*
	Handle Codec Connection request from the HF (AT+BCC).
	This function performs the the actual actions of handling the AT+BCC (once the audio params have been
	obtained from the app, i.e. after aghfpHandleWbsCodecConReq() and ).
*/
static void aghfpHandleWbsCodecConReqProcessing(AGHFP *aghfp)
{
	/* Are we supporting WBS */
	/* To test AT+BCC failure. */
#ifdef DEBUG_TEST_AT_BCC_REJECTION
	if(0) /* Reject the AT+BCC */
#else	
	if (aghfp->use_wbs)
#endif /* DEBUG_TEST_AT_BCC_REJECTION */
	{
		/* OK to initiate WBS Codec Negotiation. */
	
		/* Send OK */
		aghfpSendOk(aghfp);
	
		/* If we haven't negotiatied a codec we need to do so.
		   If we have, we're gonna go ahead and use it here. */
#ifdef DEBUG_TEST_AT_BCS_REJECTION
		/* To test AT+BCS failure (always negotiate a new codec so AT+BCS is hit). */
		if (1)
#else		
		if (aghfp->use_codec == 0)
#endif /* DEBUG_TEST_AT_BCS_REJECTION */
		{
			/* Start codec negotiation with HF. */
			if (aghfpWbsStartCodecNegotiation(aghfp, aghfp_negotiate_audio_at_hf))
			{
				aghfp->audio_connection_state = aghfp_audio_codec_connect;
			}
		}
		else
		{
			if (!aghfpCallManagerActiveNotComplete(aghfp))
			{
				MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AUDIO_CONNECT_REQ);
				AGHFP_DEBUG(("	  Normal Audio Connection\n"));
				
				message->audio_params = aghfp->audio_params;
				message->packet_type = aghfp->audio_packet_type;
				MessageSend(&aghfp->task, AGHFP_INTERNAL_AUDIO_CONNECT_REQ, message);
			}
			else
			{
				/* Save the audio packet type for Call Manager */
				aghfpStoreAudioParams(aghfp, aghfp->audio_packet_type, &aghfp->audio_params);
				
				/* Answer the call again now that the WBS negotiation is complete. */
				aghfpManageCall(aghfp, CallEventAnswer, CallFlagOpenAudio);
			}
		}
	}
	else
	{
		/* WBS not supported, send ERROR. */
		aghfpSendError(aghfp);
	}
}

/*
	Start Codec negotiation with HF
*/
bool aghfpWbsStartCodecNegotiation(AGHFP *aghfp, aghfp_wbs_negotiate_action wbs_negotiate_action)
{
	char bcs[15];
	uint16 codecs; 		/* Codec bitmap used internally */
	aghfp_wbs_codec codec_id; /* Used to store the actual codec ID */
	
	/* Find common supported codecs for the AG and HF */
	codecs = aghfp->codecs_info.ag_codecs & aghfp->hf_codecs;

	AGHFP_DEBUG(("aghfpWbsStartCodecNegotiation\n"));
	
	/* Is the chosen codec within the common supported codecs? */
	if ((codecs & aghfp->codec_to_negotiate) == 0)
		return FALSE;

	/* If we get here then the selected codec to negotiate is ok. */

	/* Reset the CODEC to use. */
	aghfp->use_codec = 0;
	
	/* Translate from the internal bitmap format (only one bit should be set in 'aghfp->codec_to_negotiate') into codec ID. */
	codec_id = aghfpWbsCodecMaskToCodecId(&aghfp->codecs_info, aghfp->codec_to_negotiate);

	/* Create the AT cmd we're sending */
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+BCS: ");

    /* may or may not need quotes here */
	/*sprintf(bcs, "\"%d\"", codec_id);*/
    sprintf(bcs, "%d", codec_id); 
    
	aghfpAtCmdString(aghfp, bcs);
	aghfpAtCmdEnd(aghfp);
	
	aghfp->wbs_negotiate_action = wbs_negotiate_action;
	
	return TRUE;
}

/*
	Handle set audio parameters request from the app.
*/
void aghfpHandleSetAudioParamsReq(AGHFP *aghfp, const AGHFP_INTERNAL_SET_AUDIO_PARAMS_REQ_T *req)
{
	/* Only act upon the apps request if in the correct state. */
	switch ( aghfp->audio_connection_state )
	{
	case aghfp_audio_disconnected:
	case aghfp_audio_codec_connect:
		aghfpStoreAudioParams(aghfp, req->packet_type, &req->audio_params);

		/* Continue the Codec Connection */
		aghfpHandleWbsCodecConReqProcessing(aghfp);
		break;
	case aghfp_audio_connecting_esco:
	case aghfp_audio_connecting_sco:
	case aghfp_audio_accepting:
	case aghfp_audio_disconnecting:
	case aghfp_audio_connected:
	default:
		AGHFP_DEBUG(("aghfpHandleAudioConnectReq invalid state %d\n",aghfp->audio_connection_state));
		break;
	}
}

/*
	Handle Codec Negotiation resopnse from the HF (AT+BCS)
*/
void aghfpHandleWbsCodecNegReq(AGHFP *aghfp, aghfp_wbs_codec codec_id)
{
    sync_pkt_type		audio_packet_type = aghfp->audio_packet_type;
    aghfp_audio_params	audio_params = aghfp->audio_params;
	uint16 codec; 		/* Codec bitmap used internally */
	
    AGHFP_DEBUG(("aghfpHandleWbsCodecNegReq : "));
    
	/* Translate from codec UUID16 format to internal bitmap. */
	codec = aghfpWbsCodecIdToCodecMask(&aghfp->codecs_info, codec_id);

	/* Note that one bit should be set in the bitmaps and it must be the same bit for success. */
#ifdef DEBUG_TEST_AT_BCS_REJECTION
	/* Reject the AT+BCS if the audio connection was initiated by the HF. */
	if(aghfp->use_wbs && (aghfp->codec_to_negotiate == codec) && (aghfp->wbs_negotiate_action != aghfp_negotiate_audio_at_hf))
#else
	if(aghfp->use_wbs && (aghfp->codec_to_negotiate == codec))
#endif /* DEBUG_TEST_AT_BCS_REJECTION */
	{
		AGHFP_DEBUG(("Starting WB-Speech eSCO\n"));
		
		/* We can now use the CODEC. */
		aghfp->use_codec = codec;
	
		/* Send OK */
		aghfpSendOk(aghfp);

		/* audio_packet_type and audio_params already set to WBS values. */
		
		/* Force connection create.
		   WBS SCO connection are always initiated by the AG. */
		if(aghfp->wbs_negotiate_action != aghfp_negotiate_no_audio)
		{
			aghfp->wbs_negotiate_action = aghfp_negotiate_audio_at_ag;
		}
	}
	else
	{
		AGHFP_DEBUG(("Falling back to request eSCO type\n"));
		/* send Error */
		aghfpSendError(aghfp);

		/* Reset any saved codecs; Codec Negotiation aborted. */
		aghfp->use_codec = 0;
		
		/* For normal SCO we only set up the SCO here if the AG initiated the connection. */
		if (aghfp->wbs_negotiate_action == aghfp_negotiate_audio_at_ag)
		{ /* Start eSCO Connection */
	        audio_packet_type = aghfp->audio_packet_type;
	        audio_params = aghfp->audio_params;
	        audio_params.override_wbs = TRUE;
        }
	}
	
	if (aghfp->wbs_negotiate_action == aghfp_negotiate_audio_at_ag)
	{ /* Start eSCO Connection. We get here if we are setting up a WBS link or an AG initiated normal SCO. */
		if (!aghfpCallManagerActiveNotComplete(aghfp))
		{
		    aghfp_audio_params  *ag_audio_params;
		    MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AUDIO_CONNECT_REQ);
		    AGHFP_DEBUG(("    Normal Audio Connection\n"));
            
            /* Get the audio params based on aghfp->use_codec negotiated */
            ag_audio_params  = aghfpGetWbsParameters(aghfp);
			message->audio_params = *ag_audio_params;
			message->packet_type = audio_packet_type;
	        MessageSend(&aghfp->task, AGHFP_INTERNAL_AUDIO_CONNECT_REQ, message);
		}
		else
		{
			/* Save the audio packet type for Call Manager */
			aghfpStoreAudioParams(aghfp, audio_packet_type, &audio_params);
			
			/* Answer the call again now that the WBS negotiation is complete. */
			aghfpManageCall(aghfp, CallEventAnswer, CallFlagOpenAudio);
        }
	}
	
	aghfp->wbs_negotiate_action = aghfp_negotiate_undefined; /* Reset flag to ensure it is correct next time */
}

/*
	Get pointer to WB-Speech eSCO Parameters
*/
aghfp_audio_params *aghfpGetWbsParameters(AGHFP *aghfp)
{
	switch(aghfp->use_codec)
	{
		case(wbs_codec_msbc):
			return((aghfp_audio_params*)&wb_speech_esco_audio_params_sbc);
			break;
		case(wbs_codec_cvsd):
			/* This relies on audio parameters having been stored previously using aghfpStoreAudioParams() */
			return(&aghfp->audio_params);
		default:
			AGHFP_DEBUG(("Codec not supported.\n"));
			Panic();
			return(NULL);
			break;
	}
}

/*
	Get WB-Speech packet type
*/
sync_pkt_type aghfpGetWbsPacketType(AGHFP *aghfp)
{
	switch(aghfp->use_codec)
	{
		case(wbs_codec_msbc):
			return(WBS_PACKET_TYPE);
			break;
		case(wbs_codec_cvsd):
			/* This relies on audio parameters having been stored previously using aghfpStoreAudioParams() */
			return(aghfp->audio_packet_type);
		default:
			AGHFP_DEBUG(("Codec not supported.\n"));
			Panic();
			return(0);
			break;
	}
}

/****************************************************************************
NAME	
	aghfpHandleCodecNegotiationReq

DESCRIPTION
	Update the AGHFP's information regarding the HF's codecs and send an OK AT command.
	This is a response to the AT+BAC command.

RETURNS
	void
*/

void aghfpHandleCodecNegotiationReq(AGHFP *aghfp, AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ_T *codecs)
{
	uint16 counter_1;

	aghfp->hf_codecs = 0;

	/* Convert from a list of UUID16s to a codec mask. */
	for(counter_1 = 0; counter_1 < codecs->num_codecs; counter_1++)
	{
		aghfp->hf_codecs |= aghfpWbsCodecIdToCodecMask(&aghfp->codecs_info, codecs->codec_uuids[counter_1]);
	}

        /* Try to negotiate for mSBC if mSBC is reported by remote device,
        * If remote device reports mSBC is temporaraly not available;then negotiate over CVSD
        * If remote device reports any other new codec for negotiation, fallback to CVSD.
        * There are devices which may dynamically changes the codec to be used for a new synchronous connection setup. 
        * So re-negotiation of codec has to happen.This behaviour will be addressed here.
        */
        if((aghfp->hf_codecs & wbs_codec_msbc) != 0)
        {
            AghfpSetCodecType(aghfp,wbs_codec_msbc);
        }
        else
        {
            AghfpSetCodecType(aghfp,wbs_codec_cvsd);
        }

    /* Send an Ok response to AT+BAC */
	aghfpSendOk(aghfp);
    
	/* Check audio (and AGHFP state?)
	   Restart codec negotiation if in the middle of WBS connect. */
	switch(aghfp->audio_connection_state)
	{
		case(aghfp_audio_disconnected):
			break;
		case(aghfp_audio_codec_connect):
			/* Start codec negotiation with HF. */
			if (aghfpWbsStartCodecNegotiation(aghfp, aghfp_negotiate_audio_at_ag))
			{
				aghfp->audio_connection_state = aghfp_audio_codec_connect;
			} /* Ignore error case for now. */
			break;
		case(aghfp_audio_connecting_esco):
		case(aghfp_audio_connecting_sco):
		case(aghfp_audio_accepting):
		case(aghfp_audio_disconnecting):
		case(aghfp_audio_connected):
			break;
	}
}
/*
	Handle Codec Connection request from the HF (AT+BCC).
	Ask the app for its configured audio parameters.
*/
void aghfpHandleWbsCodecConReq(AGHFP *aghfp)
{
	MessageSend(aghfp->client_task, AGHFP_APP_AUDIO_PARAMS_REQUIRED_IND, 0);
}

void aghfpEnableWbs(AGHFP *aghfp)
{
	if (aghfp->codecs_info.ag_codecs)
	{ /* WB-Speech Supported */
		AGHFP_DEBUG(("WB-Speech Local Codecs [0x%x]\n", aghfp->codecs_info.ag_codecs));
		aghfp->use_wbs = TRUE;
	}
	else
	{
		AGHFP_DEBUG(("Wbs - Not Supported, Disabling WB-Speech\n"));
		aghfp->use_wbs = FALSE;
	}
}

void aghfpDisableWbs(AGHFP *aghfp)
{
	AGHFP_DEBUG(("Wbs - Disabling WB-Speech\n"));
	aghfp->use_wbs = FALSE;

	/* Clear any negotiated codec. */
	aghfp->use_codec = 0;
}

/*
	Sets the codec to use.
*/
void AghfpSetCodecType(AGHFP *aghfp, aghfp_wbs_codec codec)
{
	aghfp->app_pending_codec_negotiation = FALSE;

	/* Convert to internal codec mask format. */
	switch(codec)
	{
		case(wbs_codec_cvsd):
			/* CVSD is a special case. It may be negotiated by the app. A check of this flag
			   in combination with aghfp->negotiation_type will determine which negotiation method to use. */
			aghfp->app_pending_codec_negotiation = TRUE;
			break;
		case(wbs_codec_msbc):
			break;
		default:
			aghfp->app_pending_codec_negotiation = TRUE;
			codec = 0;
			break;
	}

	/* Always re-negotiate the codec even if it has not changed. Rely on callers of this function
	   to protect against needlessly re-negotiating a codec. This has been left in to cope with
	   CVSD which can be noegitiated either by AGHFP or the app. */
	aghfp->codec_to_negotiate = codec;

	/* Signify that we need to negotiate a new codec at the AGHFP level. */
	aghfp->use_codec = 0;
}

/*
	Sets the codec negotiation type to use.
*/
void AghfpSetNegotiationType(AGHFP *aghfp, aghfp_codec_negotiation_type negotiation_type)
{
	aghfp->negotiation_type = negotiation_type;
}

/*
	Dynamically enables WBS codec negotiation. This function can only has an affect
	if the AGHFP's state is 'aghfp_ready', i.e. no SLC connection.
*/
void AghfpEnableWbsCodecNegotiation(AGHFP *aghfp)
{
	if(aghfp->state == aghfp_ready)
	{
		aghfpEnableWbs(aghfp);

		/* Enable WBS codec negotiation in the AGHFP's supported features */
		aghfp->supported_features |= aghfp_codec_negotiation;
	}
}

/*
	Dynamically dsiables WBS codec negotiation. This function can only has an affect
	if the AGHFP's state is 'aghfp_ready', i.e. no SLC connection.
*/
void AghfpDisableWbsCodecNegotiation(AGHFP *aghfp)
{
	if(aghfp->state == aghfp_ready)
	{
		aghfpDisableWbs(aghfp);

		/* Disable WBS codec negotiation in the AGHFP's supported features */
		aghfp->supported_features &= ~(aghfp_codec_negotiation);
	}
}

/*
	Start WBS negotiation if appropriate.
*/
void AghfpStartWbsCodecNegotiation(AGHFP *aghfp)
{
	if(aghfp->state >= aghfp_slc_connected)
	{
		aghfpWbsStartCodecNegotiation(aghfp, aghfp_negotiate_no_audio);
	}
}

/*
	Gets the WBS codecs supported by the connected HF.
*/
uint8 AghfpGetHfWbsCodecsSupported(AGHFP * aghfp)
{
	if(aghfp->state >= aghfp_slc_connected)
	{
		return(aghfp->hf_codecs);
	}
	else
	{
		return(0);
	}
}

/*
	Indicates whether a Wbs has been negotiated.
		TRUE - WBS codec has been negotiated
		FALSE - WBS codec has NOT been negotiated
*/
bool AghfpCodecHasBeenNegotiated(AGHFP * aghfp, uint8 *wbs_codec)
{
	bool	rtnVal = FALSE;

	if(aghfp->use_codec != 0)
	{
		*wbs_codec = aghfp->use_codec;
		rtnVal = TRUE;
	}

	return(rtnVal);
}

/*
	Returns the codecs supported by this instantiation of the AGHFP.
	Return value is only valid AFTER a call to AghfpInit().
*/
uint16 AghfpSupportedWbsCodecs(AGHFP * aghfp)
{
	return(aghfp->codecs_info.ag_codecs);
}


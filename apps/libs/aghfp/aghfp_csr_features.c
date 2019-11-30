/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_send_data.h"

#include "aghfp_csr_features.h"

#include "aghfp_call_manager.h"

#include "aghfp_ok.h"

#include <panic.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <util.h>

/*
	Helper function to deal with processing the AT+CSRSF command.
*/
static void handleReponseCSRSupportedFeatures(AGHFP * aghfp, bool callerName, bool rawText, bool smsInd, bool battLevel, bool pwrSource, uint16 codecs, bool codec_bandwidths_present, uint16 codec_bandwidths)
{
    MAKE_AGHFP_MESSAGE(AGHFP_CSR_SUPPORTED_FEATURES_IND);
    
    message->aghfp      = aghfp ;
    message->callerName = callerName;
	message->rawText    = rawText;
	message->smsInd     = smsInd;
	message->battLevel  = battLevel;
	message->pwrSource  = pwrSource;
	message->codecs     = codecs;    
	message->codec_bandwidths_present 	= codec_bandwidths_present;    
	message->codec_bandwidths 			= codec_bandwidths;    
    
    MessageSend ( aghfp->client_task , AGHFP_CSR_SUPPORTED_FEATURES_IND , message );
}

void aghfpCsrSupportedFeaturesResponse (AGHFP *aghfp, bool callerName, bool rawText, bool smsInd, bool battLevel, bool pwrSource , uint16 codecs, bool codec_bandwidths_present, uint16 codec_bandwidths)
{
    /*send AT command to HF*/
    
    char buf[4];
	
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CSRSF: ");
	
	
    sprintf(buf, "%d", callerName);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", rawText);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", smsInd);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", battLevel);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", pwrSource);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", codecs);
	aghfpAtCmdString(aghfp, buf);

	if(codec_bandwidths_present)
	{
		aghfpAtCmdString(aghfp, ",");
		
	    sprintf(buf, "%d", codec_bandwidths);
		aghfpAtCmdString(aghfp, buf);
	}
	
	aghfpAtCmdEnd(aghfp);
    
	/* Need to also send an OK message to the HF. */
	aghfpSendOk(aghfp);
}


void aghfpFeatureNegotiate ( AGHFP * aghfp , uint16 num_csr_features, uint16 indicator0, uint16 value0, uint16 indicator1, uint16 value1, bool sendLeadingOK )
{
    char buf[4];

	/* We may need to send a leading "OK" command if we are
	   responding to a rejection of a parameters from the AG. */
	if(sendLeadingOK) aghfpSendOk(aghfp);

	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CSRFN: ");
	
	aghfpAtCmdString(aghfp, "(");
    sprintf(buf, "%d", indicator0);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
    sprintf(buf, "%d", value0);
	aghfpAtCmdString(aghfp, buf);

    if(num_csr_features > 1)
    {
		aghfpAtCmdString(aghfp, "),(");
		
	    sprintf(buf, "%d", indicator1);
		aghfpAtCmdString(aghfp, buf);
		aghfpAtCmdString(aghfp, ",");

	    sprintf(buf, "%d", value1);
		aghfpAtCmdString(aghfp, buf);
    }

	aghfpAtCmdString(aghfp, ")");

	aghfpAtCmdEnd(aghfp);
}


void aghfpHandleFeatureNegotiation(Task task, const struct aghfpHandleFeatureNegotiation * feature )
{
    AGHFP * aghfp = (AGHFP *)task ;
    
    MAKE_AGHFP_MESSAGE(AGHFP_CSR_FEATURE_NEGOTIATION_IND) ;
    
    message->aghfp = (AGHFP*)task;
    message->num_csr_features = 1;
    message->csr_features[0].indicator 	= feature->ind;
    message->csr_features[0].value 		= feature->val ;        
    
    MessageSend ( aghfp->client_task , AGHFP_CSR_FEATURE_NEGOTIATION_IND , message ) ;
}


void aghfpHandleFeatureNegotiationWithBandwidth(Task task, const struct aghfpHandleFeatureNegotiationWithBandwidth * feature )
{
    AGHFP * aghfp = (AGHFP *)task ;
    
    MAKE_AGHFP_MESSAGE(AGHFP_CSR_FEATURE_NEGOTIATION_IND) ;
    
    message->aghfp = (AGHFP*)task;
    message->num_csr_features = 2;
    message->csr_features[0].indicator 	= feature->ind0;
    message->csr_features[0].value 		= feature->val0;        
    message->csr_features[1].indicator 	= feature->ind1;
    message->csr_features[1].value 		= feature->val1;        
    
    MessageSend ( aghfp->client_task , AGHFP_CSR_FEATURE_NEGOTIATION_IND , message ) ;
}


void aghfpHandleReponseCSRSupportedFeatures(Task task , const struct aghfpHandleReponseCSRSupportedFeatures * features )
{
    AGHFP * aghfp = (AGHFP *)task ;


	handleReponseCSRSupportedFeatures(aghfp, features->callerName, features->rawText, features->smsInd, features->battLevel, features->pwrSource, features->codecs, FALSE, 0);
}

void aghfpHandleReponseCSRSupportedFeaturesCodecsBw(Task task , const struct aghfpHandleReponseCSRSupportedFeaturesCodecsBw * features )
{
    AGHFP * aghfp = (AGHFP *)task ;

	handleReponseCSRSupportedFeatures(aghfp, features->callerName, features->rawText, features->smsInd, features->battLevel, features->pwrSource, features->codecs, TRUE, features->codecBandwidths);
}

void aghfpSendCsrFeatureNegotiationReqInd(AGHFP * aghfp)
{
    MAKE_AGHFP_MESSAGE(AGHFP_CSR_FEATURE_NEGOTIATION_REQ_IND);
    
    message->aghfp = aghfp ;

	/* Prompt the app to negotiate the codec */
	MessageSend(aghfp->client_task, AGHFP_CSR_FEATURE_NEGOTIATION_REQ_IND, message);
}

void AghfpClearAppCodecNegotiationPending(AGHFP * aghfp)
{
	aghfp->app_pending_codec_negotiation = FALSE;
}


void AghfpStartAudioAfterAppCodecNegotiation(AGHFP *aghfp, sync_pkt_type packet_type, const aghfp_audio_params *audio_params)
{
	if (!aghfpCallManagerActiveNotComplete(aghfp))
	{
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AUDIO_CONNECT_REQ);
		AGHFP_DEBUG(("	  Normal Audio Connection\n"));
		
		/* Assume that the audio parameters have been previously stored using aghfpStoreAudioParams() */
		message->audio_params = *audio_params;
		message->packet_type = packet_type;
		
		MessageSend(&aghfp->task, AGHFP_INTERNAL_AUDIO_CONNECT_REQ, message);
	}
	else
	{
		/* Answer the call again now that the WBS negotiation is complete. */
		aghfpManageCall(aghfp, CallEventAnswer, CallFlagOpenAudio);
	}
}


/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

*/


#include "aghfp.h"
#include "aghfp_common.h"
#include "aghfp_private.h"
#include "aghfp_init.h"
#include "aghfp_profile_handler.h"
#include "aghfp_rfc.h"
#include "aghfp_sdp.h"
#include "aghfp_wbs.h"
#include "aghfp_wbs_handler.h"

#include <panic.h>
#include <string.h>  /* For memset */


/* Send an init cfm message to the application */
static void sendInitCfmToApp(aghfp_init_status status, AGHFP *aghfp)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INIT_CFM);
	message->status = status;
	message->aghfp = aghfp;
	MessageSend(aghfp->client_task, AGHFP_INIT_CFM, message);

	/* If the initialization failed, free the allocated task */
	if (status != aghfp_init_success)
	{
		free(aghfp);
 	}
}


/****************************************************************************
 Reset all the connection related state in this function.
*/
void aghfpResetConnectionRelatedState(AGHFP *aghfp)
{
	/* Clear the SLC sink */
	aghfp->rfcomm_sink = 0;

	/* Clear the SCO sink */
	aghfp->audio_sink = 0;
	aghfp->audio_connection_state = aghfp_audio_disconnected;

    /* Reset any negotiated codec. */
	aghfp->codec_to_negotiate = wbs_codec_cvsd;
	aghfp->negotiation_type = aghfp_codec_negotiation_type_wbs;
	aghfp->use_codec = 0;
	aghfp->app_pending_codec_negotiation = FALSE;
        
    aghfp->hf_codecs = 0;
    aghfp->qce_codec_mode_id  = CODEC_MODE_ID_UNSUPPORTED;

    
    /* Set all indicators enabled */
    aghfp->active_indicators = aghfp_all_indicators;
    aghfp->indicators_enable = TRUE;

    /* Reset any negotiated HF indicators  */
    aghfp->hf_indicators = 0;
}

/****************************************************************************
This function initialises an instance of the Aghfp library by calling the
AghfpInitQCE() function with the codec_modes set to 0, effectively disabling
the Qualcomm Codec Extensions mode.

This is for backwards compatiblity.
*/

void AghfpInit(
        Task            theAppTask, 
        aghfp_profile   aghfp_supported_profile, 
        uint16          supported_features
        )
{
    AghfpInitQCE(theAppTask, aghfp_supported_profile, supported_features, 0);
}

/****************************************************************************
This function initialises an instance of the Aghfp library. The
application registers its own task, theAppTask, with the Aghfp library so
that return messages can be routed to the correct task.

The aghfp_supported_profile parameter is used to configure which profile this Aghfp
instance will support (HSP or HFP). If the HFP is being supported the supported
features should also be supplied. These are passed in as a bit mask, as
defined in the HFP specification. If the profile instance is being
configured as HSP the supported features should be set to zero as they
are not used.

MESSAGE RETURNED
 AGHFP_INIT_CFM
*/
void AghfpInitQCE(
        Task            theAppTask, 
        aghfp_profile   aghfp_supported_profile, 
        uint16          supported_features,
        uint16          codec_modes
        )
{
	AGHFP *aghfp = PanicUnlessNew(AGHFP);

	aghfpSetState(aghfp, aghfp_initialising);

	/* Set the handler function */
	aghfp->task.handler = aghfpProfileHandler;

	/* Init the start up state */
	if ( supportedProfileIsHfp(aghfp_supported_profile) )
	{
		/* HFP supported */
		aghfp->supported_profile = aghfp_supported_profile;
		aghfp->supported_features = supported_features;
                aghfp->ag_codec_modes = codec_modes;
		aghfp->hf_supported_features = 0;
		aghfp->features_status = 0;
	}
	else if ( supportedProfileIsHsp(aghfp_supported_profile) )
	{
		/* HSP supported */
		aghfp->supported_profile = aghfp_headset_profile;
		aghfp->supported_features = 0;
                aghfp->ag_codec_modes = 0;
		aghfp->hf_supported_features = 0;
		aghfp->features_status = 0;
	}
	else
	{
		/* If the app has not indicated support for any valid profile - proceed no further */
		AGHFP_DEBUG_PANIC(("Profile support not registered\n"));
	}

    aghfp->cind_poll_client = FALSE;

	/* Set initial call_setup_status */
	aghfp->call_setup_status = aghfp_call_setup_none;

	/* Connection related state updated in separate function */
	aghfpResetConnectionRelatedState(aghfp);

	/* Init the local server channel */
	aghfp->local_rfc_server_channel = 0;

	/* Init the service record handle */
	aghfp->sdp_record_handle = 0;

	/* Store the app task so we know where to return responses */
	aghfp->client_task = theAppTask;

	aghfp->mapped_rfcomm_sink = NULL;
	
	/* Clear call management related structures */
	aghfp->ring_repeat_interval = DEFAULT_RING_ALERT_REPEAT;
	aghfp->call_progress = CallProgressIdle;
	memset((void *)&aghfp->call_params, 0, sizeof(aghfp_call_params));

	/* Init the rfcomm lock */
	aghfp->rfcomm_lock = FALSE;
	
	aghfp->use_wbs = TRUE;

	aghfp->ag_attempt_settings = aghfp_audio_settings_none;

	aghfp->codecs_info.ag_codecs = wbs_codec_cvsd | wbs_codec_msbc;

	aghfp->codec_to_negotiate = wbs_codec_cvsd;
	aghfp->negotiation_type = aghfp_codec_negotiation_type_wbs;
	aghfp->use_codec = 0;
	aghfp->codecs_info.codec_ids[0] = wbs_codec_cvsd;
	aghfp->codecs_info.codec_ids[1] = wbs_codec_msbc;
	aghfp->app_pending_codec_negotiation = FALSE;

	aghfp->hf_indicators= 0;  /* Remote HF indicators */
	aghfp->ag_hf_indicators_info.active_hf_indicators =0; /* Local HF indicators */
	aghfp->ag_hf_indicators_info.hf_indicators_state = 0; /* Local HF indicators state mask*/

	aghfp->link_mode_secure = FALSE;

    aghfp->hf_codecs = 0;
    aghfp->qce_codec_mode_id  = CODEC_MODE_ID_UNSUPPORTED;

	/* Send an internal init message to kick off initialisation */
	MessageSend(&aghfp->task, AGHFP_INTERNAL_INIT_REQ, 0x00);
}


/****************************************************************************
 Send internal init req messages until we have completed the profile
 lib initialisation.
*/
void aghfpHandleInternalInitReq(AGHFP *aghfp)
{
	/* Get an rfcomm channel */
	aghfpHandleRfcommAllocateChannel(aghfp);
}


/****************************************************************************
 Send an internal init cfm message.
*/
void aghfpSendInternalInitCfm(Task task, aghfp_init_status s, uint8 c)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_INIT_CFM);
	message->status = s;
	message->rfcomm_channel = c;
	MessageSend(task, AGHFP_INTERNAL_INIT_CFM, message);
}


/****************************************************************************
 This message is sent once various parts of the library initialisation
 process have completed.
*/
void aghfpHandleInternalInitCfm(AGHFP *aghfp, const AGHFP_INTERNAL_INIT_CFM_T *cfm)
{
	/* The init stage may have failed check the result code */
	if (cfm->status != aghfp_init_success)
	{
		/* Something has gone wrong, tell the app */
		sendInitCfmToApp(cfm->status, aghfp);
	}
	else
	{
		if (!cfm->rfcomm_channel)
		{
			/* Update the local state to initialised */
			aghfpSetState(aghfp, aghfp_ready);

			/* Register our intention to use SCO connections */
			ConnectionSyncRegister(&aghfp->task);
		}
		else
		{
			/* Store the local server channel */
			aghfp->local_rfc_server_channel = cfm->rfcomm_channel;

			/* Rfcomm channel allocated. Register a service record for the profile */
			aghfpRegisterServiceRecord(aghfp, aghfp->supported_profile, cfm->rfcomm_channel);
		}
	}
}

/****************************************************************************
NAME
	aghfpHandleSyncRegisterCfm

DESCRIPTION
	Handles confirmation registering the AGHFP to receive Synchronous connection
	notifications from the Connection library.  This completes the AGHFP initialisation
	process - send message to app.

RETURNS
	void
*/
void aghfpHandleSyncRegisterCfm(AGHFP *aghfp)
{
    /* We have finished the profile init so send an init cfm to the app */
    sendInitCfmToApp(aghfp_init_success, aghfp);

    aghfpEnableWbs(aghfp);
}

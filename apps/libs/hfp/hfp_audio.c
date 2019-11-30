/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_audio.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"
#include "hfp_common.h"
#include "hfp_audio_handler.h"

/****************************************************************************
NAME    
    HfpAudioTransferRequest

DESCRIPTION
    Transfer audio to/from HF from/to the AG

MESSAGE RETURNED
    HFP_AUDIO_CONNECT_CFM/HFP_AUDIO_DISCONNECT_IND

RETURNS
    void
*/
void HfpAudioTransferRequest(hfp_link_priority priority, hfp_audio_transfer_direction direction, sync_pkt_type packet_type, const hfp_audio_params *audio_params)
{
    /* Send an audio transfer message */
    MAKE_HFP_MESSAGE(HFP_INTERNAL_AUDIO_TRANSFER_REQ);
    
#ifdef HFP_DEBUG_LIB
    if ( direction > hfp_audio_transfer )
    {
        HFP_DEBUG(("Invalid direction passed in.\n"));
    }
#endif
    
    message->link = hfpGetLinkFromPriority(priority);
    message->direction = direction;
    message->audio_params = *hfpAudioGetParams(message->link, &packet_type, audio_params, FALSE);
    message->packet_type  = packet_type;
    MessageSend(&theHfp->task, HFP_INTERNAL_AUDIO_TRANSFER_REQ, message);
}



/****************************************************************************
NAME    
    HfpAudioConnectResponse

DESCRIPTION
    Used by the application to respond to an incoming connection request from
    the AG.  The application will receive a HFP_AUDIO_CONNECT_IND to indicate
    that the AG is attempting to initiate a connection.
    
    The response is used to accept or reject the incoming connection request.

    The audio_params structure is used to specify the acceptable range of
    connection parameters when accepting an incoming request. It is the 
    application's responsibility to ensure that the remote device supports the 
    specified parameters.

MESSAGE RETURNED
    HFP_AUDIO_CONNECT_CFM.

RETURNS
    void
*/
void HfpAudioConnectResponse(hfp_link_priority priority, bool response, sync_pkt_type packet_type, const hfp_audio_params *audio_params, bool disable_override)
{
    /* Send audio connect response message */
    MAKE_HFP_MESSAGE(HFP_INTERNAL_AUDIO_CONNECT_RES);
    message->response = response;
    message->link = hfpGetLinkFromPriority(priority);
    message->audio_params = *hfpAudioGetParams(message->link, &packet_type, audio_params, disable_override);
    message->packet_type  = packet_type;
    MessageSend(&theHfp->task, HFP_INTERNAL_AUDIO_CONNECT_RES, message);
}


/****************************************************************************
NAME    
    HfpAudioDisconnectRequest

DESCRIPTION
    Disconnect the audio connection to the AG for the specified profile instance.
    
MESSAGE RETURNED
    HFP_AUDIO_DISCONNECT_IND.

RETURNS
    void
*/
void HfpAudioDisconnectRequest(hfp_link_priority priority)
{
    /* Send audio disconnect message */
    hfpSendCommonInternalMessagePriority(HFP_INTERNAL_AUDIO_DISCONNECT_REQ, priority);
}



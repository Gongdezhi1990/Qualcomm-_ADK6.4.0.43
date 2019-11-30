/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_link_manager.c

DESCRIPTION
    The link manager provides commonly used functionality for accessing links
    in hfp_task_data.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"
#include "hfp_common.h"
#include "hfp_sdp.h"
#include "hfp_rfc.h"
#include "bdaddr.h"

#include <sink.h>
#include <string.h>
#include <print.h>


/****************************************************************************
NAME
    hfpLinkSetup

DESCRIPTION
    Setup a link 

RETURNS
    void
*/
void hfpLinkSetup(hfp_link_data* link, hfp_service_data* service, const bdaddr* bd_addr, Sink sink, hfp_slc_state state)
{
    /* Don't change link priority if it already has one */
    if(hfpGetLinkPriority(link) == hfp_invalid_link)
    {
        /* Set the priority of this link */
        if(hfpGetLinkFromPriority(hfp_primary_link))
            hfpSetLinkPriority(link, hfp_secondary_link);
        else
            hfpSetLinkPriority(link, hfp_primary_link);
    }
    
    PRINT(("Link priority %X\n", link->bitfields.priority));
    
    /* Reserve the service for incoming connections */
    if(state == hfp_slc_incoming)
    {
        link->bitfields.owns_service = TRUE;
        service->bitfields.busy = TRUE;
    }
    
    /* Set up link params */
    link->service = service;
    hfpSetLinkBdaddr(link, bd_addr);
    hfpSetLinkSink(link, sink);
    hfpSetLinkSlcState(link, state);
}


/****************************************************************************
NAME
    hfpLinkReset

DESCRIPTION
    Reset all the connection related state in this function.

RETURNS
    void
*/
void hfpLinkReset(hfp_link_data* link, bool promote_secondary)
{
    bool manage_linkloss = link->bitfields.manage_linkloss;
    
    /* Free extra indicator indexes stored for this link */
    if(link->ag_supported_indicators.extra_indicator_idxs)
        free(link->ag_supported_indicators.extra_indicator_idxs);
    
    /* If link was using a service */
    if(link->service && link->bitfields.owns_service)
    {
        /* Release the service */
        link->service->bitfields.busy = FALSE;
        /* Register service with SDP if it was hidden and no other
           service is visible for the same profile */
        hfpRegisterServiceRecord(link->service);
    }
    
    /* Zero the hfp_link_data struct */
    memset(link, 0, sizeof(hfp_link_data));

    /*Linkloss management should be returned to its previous state. */
    link->bitfields.manage_linkloss = manage_linkloss;
    
    /* Reset the AG's supported features to default vals as specified in the HFP spec */
    link->ag_supported_features = (AG_THREE_WAY_CALLING | AG_IN_BAND_RING);
    
    /* Check if we need to promote a secondary link */
    if(promote_secondary)
        hfpSetLinkPriority(hfpGetLinkFromPriority(hfp_secondary_link), hfp_primary_link);

    /* Reset the QCE Mode IDS and Codec Selection */
    link->ag_codec_modes = 0;
    link->qce_codec_mode_id = CODEC_MODE_ID_UNSUPPORTED;
}


/****************************************************************************
NAME    
    hfpSetLinkSink

DESCRIPTION
    Set the Sink for a given link

RETURNS
    void
*/
void hfpSetLinkSink(hfp_link_data* link, Sink sink)
{
    link->identifier.sink = sink;
}

/****************************************************************************
NAME    
    hfpGetLinkSink

DESCRIPTION
    Get the Sink for a given link

RETURNS
    The link's Sink if successful, otherwise NULL.
*/
Sink hfpGetLinkSink(const hfp_link_data* link)
{
    /* If sink is not set it will be NULL, just need to check for NULL ptr */
    if(link)
        return link->identifier.sink;
    
    return (Sink)NULL;
}


/****************************************************************************
NAME    
    hfpSetLinkBdaddr

DESCRIPTION
    Set the bdaddr for a given link

RETURNS
    void
*/
void hfpSetLinkBdaddr(hfp_link_data* link, const bdaddr* bd_addr)
{
    link->identifier.bd_addr = *bd_addr;
}


/****************************************************************************
NAME
    hfpSetLinkSlcState

DESCRIPTION
    Set the SLC state for a given link

RETURNS
    TRUE if state transition was allowed, FALSE otherwise
*/
void hfpSetLinkSlcState(hfp_link_data* link, hfp_slc_state slc_state)
{
    link->bitfields.ag_slc_state = slc_state;
}


/****************************************************************************
NAME
    hfpSetLinkCallState

DESCRIPTION
    Set the call state for a given link

RETURNS
    TRUE if state transition was allowed, FALSE otherwise
*/
void hfpSetLinkCallState(hfp_link_data* link, hfp_call_state call_state)
{
    hfp_link_priority link_priority;
    if(link->bitfields.ag_call_state != call_state)
    {
        /* Call state of a link has changed, tell the app */

        link_priority = hfpGetLinkPriority(link);
        /*record the first incoming call link*/
        if(call_state == hfp_call_state_incoming)
        {
            if(theHfp->bitfields.first_incoming_call == hfp_invalid_link)
            {

                theHfp->bitfields.first_incoming_call = link_priority;
            }
        }
        else
        {
            if(theHfp->bitfields.first_incoming_call == link_priority)
            {
                theHfp->bitfields.first_incoming_call = hfp_invalid_link;
            }
        }

        if(link->bitfields.ag_call_state != call_state)
        {
            /* Call state of a link has changed, tell the app */
            hfpSendLinkCallStateNotification(link, call_state);

            /* Update the link's call state */
            link->bitfields.ag_call_state = call_state;
        }
    }
}

/****************************************************************************
NAME
    hfpSendLinkCallStateNotification

DESCRIPTION
    Send a call state notification to the VM application

RETURNS
    void
*/
void hfpSendLinkCallStateNotification(hfp_link_data* link, hfp_call_state call_state)
{
    MAKE_HFP_MESSAGE(HFP_CALL_STATE_IND);
    message->priority   = hfpGetLinkPriority(link);
    message->call_state = call_state;
    MessageSend(theHfp->clientTask, HFP_CALL_STATE_IND, message);
}




/****************************************************************************
NAME    
    hfpGetLinkBdaddr

DESCRIPTION
    Get the bdaddr for a given link

RETURNS
    If successful the bdaddr pointed to by bd_addr will be set to the link's
    bdaddr. If not successful this will be zero'ed.
*/
bool hfpGetLinkBdaddr(const hfp_link_data* link, bdaddr* bd_addr)
{
    if(link && link->bitfields.ag_slc_state > hfp_slc_disabled)
    {
        /* Before hfp_slc_connected link is identified by bdaddr */
        *bd_addr = link->identifier.bd_addr;
        return TRUE;
    }
    BdaddrSetZero(bd_addr);
    return FALSE;
}


/****************************************************************************
NAME    
    hfpGetLinkFromSink

DESCRIPTION
    Get the link data corresponding to a given Sink

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromSink(Sink sink)
{
    hfp_link_data*  link;
    
    /* Don't match a NULL sink */
    if(sink != (Sink)NULL)
    {
        for_all_links(link)
        {
            if(link->bitfields.ag_slc_state <= hfp_slc_disabled)
            {
                /* Ignore - can't possibly be a match */
            }
            else if(link->bitfields.ag_slc_state < hfp_slc_outgoing)
            {
                /* Before hfp_slc_outgoing link is identified by bdaddr */
                tp_bdaddr check_addr;
                /* If we can get a bdaddr from the sink and it matches then return the link */
                if(SinkGetBdAddr(sink, &check_addr) && BdaddrIsSame(&check_addr.taddr.addr, &link->identifier.bd_addr))
                    return link;
            }
            else
            {
                /* Once RFCOMM is established link is identified by sink */
                if(link->identifier.sink == sink)
                    return link;
            }
        }
    }
    return NULL;
}


/****************************************************************************
NAME    
    hfpGetLinkFromBdaddr

DESCRIPTION
    Get the link data corresponding to a given bdaddr

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromBdaddr(const bdaddr* bd_addr)
{
    hfp_link_data*  link;
    
    for_all_links(link)
    {
        bdaddr link_addr;
        
        if(hfpGetLinkBdaddr(link, &link_addr) && BdaddrIsSame(&link_addr, bd_addr))
            return link;
    }
    return NULL;
}


/****************************************************************************
NAME    
    hfpGetLinkFromAudioSink

DESCRIPTION
    Get the link data corresponding to a given audio Sink

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromAudioSink(Sink sink)
{
    hfp_link_data* link;
    
    for_all_links(link)
    {
        if(link->bitfields.audio_state < hfp_audio_connected)
        {
            tp_bdaddr bd_addr;
            bdaddr    link_addr;

            /* Get bdaddr from the audio sink and match with that */
            if(SinkGetBdAddr(sink, &bd_addr) && hfpGetLinkBdaddr(link, &link_addr) && BdaddrIsSame(&link_addr, &bd_addr.taddr.addr))
                return link;
        }
        else
        {
            /* We have an audio sink stored, match that instead */
            if(link->audio_sink == sink)
                return link;
        }
    }
    return NULL;
}


/****************************************************************************
NAME    
    hfpGetLinkFromSlcState

DESCRIPTION
    Get link data for the first link in the given SLC state

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
static hfp_link_data* hfpGetLinkFromSlcState(hfp_slc_state state)
{
    hfp_link_data*  link;
    
    for_all_links(link)
        if(link->bitfields.ag_slc_state == state)
            return link;
    
    return NULL;
}


/****************************************************************************
NAME    
    hfpGetIdleLink

DESCRIPTION
    Get link data for the first link in the idle SLC state

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetIdleLink(void)
{
    hfp_link_data*  link;
    
    for_all_links(link)
        if(link->bitfields.ag_slc_state == hfp_slc_idle && link->bitfields.ag_link_loss_state != hfp_link_loss_recovery)
            return link;
    
    return NULL;
}


/****************************************************************************
NAME    
    hfpGetLinkFromService

DESCRIPTION
    Get link data for the link using the given service

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromService(const hfp_service_data* service)
{
    hfp_link_data*  link;
    
    for_all_links(link)
        if(link->service == service && link->bitfields.owns_service)
            return link;
    
    return NULL;
}


/****************************************************************************
NAME    
    hfpSetLinkPriority

DESCRIPTION
    Set the priority of a link

RETURNS
    void
*/
void hfpSetLinkPriority(hfp_link_data* link, hfp_link_priority priority)
{
    if(link)
        link->bitfields.priority = priority;
}


/****************************************************************************
NAME    
    hfpGetLinkPriority

DESCRIPTION
    Get the priority of a link

RETURNS
    void
*/
hfp_link_priority hfpGetLinkPriority(const hfp_link_data* link)
{
    if(link)
        return link->bitfields.priority;
    else
        return hfp_invalid_link;
}


/****************************************************************************
NAME    
    hfpGetLinkFromPriority

DESCRIPTION
    Get a link from it's priority

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromPriority(hfp_link_priority priority)
{
    hfp_link_data*  link;
    
    if(theHfp && theHfp->bitfields.initialised && priority != hfp_invalid_link)
        for_all_links(link)
            if(link->bitfields.priority == priority)
                return link;
    
    return NULL;
}


/****************************************************************************
NAME    
    hfpGetLinkTimeoutMessage

DESCRIPTION
    Because we have two links that could have pending commands we need 
    separate timeout messages for each link. This function is used to get
    the timeout message corresponding to a given link.

RETURNS
    The MessageId for the timeout message corresponding to the link.
*/
MessageId hfpGetLinkTimeoutMessage(const hfp_link_data* link, MessageId timeout_base)
{
    bool link_is_zero_idx = (link == &theHfp->links[0]);
    
    switch(timeout_base)
    {
        case HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_0_IND:
            return link_is_zero_idx ? HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_0_IND : HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_1_IND;
            
        case HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_0_IND:
            return link_is_zero_idx ? HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_0_IND : HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_1_IND;
        
        case HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_0_IND:
            return link_is_zero_idx ? HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_0_IND : HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_1_IND;
        
        case HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_0_IND:
            return link_is_zero_idx ? HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_0_IND : HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_1_IND;
            
        default:
            HFP_ASSERT_FAIL(("Timeout requested for invalid base\n"));
            return 0;
    }
}


/****************************************************************************
NAME    
    hfpGetLinkFromTimeoutMessage

DESCRIPTION
    This function is used in handling the above timeouts, we need to know
    which link a command has timed out from.

RETURNS
    The link corresponding to the timeout MessageId
*/
hfp_link_data* hfpGetLinkFromTimeoutMessage(MessageId id)
{
    switch(id)
    {
        case HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_0_IND:
        case HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_0_IND:
        case HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_0_IND:
        case HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_0_IND:
            return &theHfp->links[0];
            
        case HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_1_IND:
        case HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_1_IND:
        case HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_1_IND:
        case HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_1_IND:
            return &theHfp->links[1];
            
        default:
            return NULL;
    }
}


/****************************************************************************
NAME    
    hfpLinkGetProfile

DESCRIPTION
    Get the profile being used by a link (if any)

RETURNS
    The profile if one is in use, otherwise hfp_no_profile
*/
hfp_profile hfpLinkGetProfile(const hfp_link_data* link)
{
    if(link && link->service)
        return link->service->bitfields.profile;
    else
        return hfp_no_profile;
}


/****************************************************************************
NAME    
    hfpLinkIsHsp

DESCRIPTION
    Check if a link is using HSP

RETURNS
    TRUE if it is, FALSE if not
*/
bool hfpLinkIsHsp(const hfp_link_data* link)
{
    return supportedProfileIsHsp(hfpLinkGetProfile(link));
}


/****************************************************************************
NAME    
    hfpLinkIsHfp

DESCRIPTION
    Check if a link is using HFP

RETURNS
    TRUE if it is, FALSE if not
*/
bool hfpLinkIsHfp(const hfp_link_data* link)
{
    return supportedProfileIsHfp(hfpLinkGetProfile(link));
}


/****************************************************************************
NAME
    hfpLinkIsHfp107

DESCRIPTION
    Check if a link is using HFP 1.7

RETURNS
    TRUE if it is, FALSE if not
*/
bool hfpLinkIsHfp107(const hfp_link_data* link)
{
    return supportedProfileIsHfp107(hfpLinkGetProfile(link));
}


/****************************************************************************
NAME    
    hfpLinkDisable

DESCRIPTION
    Disable or enable a link. This will fail if the link is invalid. It will
    also fail if attempting to disable a link that is not idle, or attempting
    to enable a link that is not disabled.

RETURNS
    TRUE if successful, FALSE otherwise
*/
static bool hfpLinkDisable(hfp_link_data* link, bool disable)
{
    if(!link)
        return FALSE;

    if(disable)
    {
        if(link->bitfields.ag_slc_state != hfp_slc_idle)
            return FALSE;
        PRINT(("0x%p disabled\n", (void*)link));
        link->bitfields.ag_slc_state = hfp_slc_disabled;
    }
    else
    {
        if(link->bitfields.ag_slc_state != hfp_slc_disabled)
            return FALSE;
        PRINT(("0x%p enabled\n", (void*)link));
        link->bitfields.ag_slc_state = hfp_slc_idle;
    }
    return TRUE;
}

/****************************************************************************
NAME    
    HfpLinkGetBdaddr

DESCRIPTION
    External function to get the bdaddr of a given link

RETURNS
    TRUE if successful, FALSE otherwise
*/
bool HfpLinkGetBdaddr(hfp_link_priority priority, bdaddr* bd_addr)
{
    const hfp_link_data* link = hfpGetLinkFromPriority(priority);
    return hfpGetLinkBdaddr(link, bd_addr);
}


/****************************************************************************
NAME    
    HfpLinkGetSlcSink

DESCRIPTION
    External function to get the slc Sink of a given link

RETURNS
    TRUE if successful, FALSE otherwise
*/
bool HfpLinkGetSlcSink(hfp_link_priority priority, Sink* sink)
{
    const hfp_link_data* link = hfpGetLinkFromPriority(priority);
    *sink = hfpGetLinkSink(link);
    return (*sink) ? TRUE : FALSE;
}


/****************************************************************************
NAME    
    HfpLinkGetAudioSink

DESCRIPTION
    External function to get the audio Sink of a given link

RETURNS
    TRUE if successful, FALSE otherwise
*/
bool HfpLinkGetAudioSink(hfp_link_priority priority, Sink* sink)
{
    const hfp_link_data* link = hfpGetLinkFromPriority(priority);
    if(link && link->audio_sink)
    {
        *sink = link->audio_sink;
        return TRUE;
    }
    *sink = (Sink)NULL;
    return FALSE;
}

/****************************************************************************
NAME    
    HfpLinkGetCallState

DESCRIPTION
    External function to get the call state of a given link

RETURNS
    TRUE if successful, FALSE otherwise
*/
bool HfpLinkGetCallState(hfp_link_priority priority, hfp_call_state* state)
{
    const hfp_link_data* link = hfpGetLinkFromPriority(priority);
    
    if(link)
    {
        *state = link->bitfields.ag_call_state;

        return TRUE;
    }
    *state = hfp_call_state_idle;
    return FALSE;
}


/****************************************************************************
NAME    
    HfpLinkSetMaxConnections

DESCRIPTION
    Set the maximum number of allowed connections. It is not possible
    to set max connections greater than the configured number, so if 
    multipoint support is not configured any setting other than one will fail.
    If reducing the number of connections this function will fail if an idle
    link is not available to be disabled.

RETURNS
    TRUE if successful, FALSE otherwise
*/
bool HfpLinkSetMaxConnections(uint8 max_connections)
{
    hfp_link_data* link;
    uint8 link_count = 0;
    
    /* Count number of active links */
    for_all_links(link)
        if(link->bitfields.ag_slc_state != hfp_slc_disabled)
            link_count++;
    
    PRINT(("Set max connections %d, currently %d\n", max_connections, link_count));
    
    if(max_connections == link_count)
    {
        PRINT(("Already done\n"));
        return TRUE;
    }
    else if(max_connections > link_count)
    {
        link = hfpGetLinkFromSlcState(hfp_slc_disabled);
        PRINT(("Enabling link 0x%p\n", (void*)link));
        return hfpLinkDisable(link, FALSE);
    }
    else
    {
        link = hfpGetIdleLink();
        PRINT(("Disabling link 0x%p\n", (void*)link));
        return hfpLinkDisable(link, TRUE);
    }
}


/****************************************************************************
NAME    
    HfpLinkPriorityFromCallState
    
DESCRIPTION
    determine if any of the connected AG's are in the passed in state, if
    a state match is found return the link priority

RETURNS
    link priority if match is found, invalid link if none found
*/   
hfp_link_priority HfpLinkPriorityFromCallState(hfp_call_state call_state)
{
    hfp_call_state state;
    hfp_link_priority priority;
    
    for_all_priorities(priority)
        if((HfpLinkGetCallState(priority, &state)) && (call_state == state))
            return priority;
    
    return hfp_invalid_link;
}


/****************************************************************************
NAME    
    HfpLinkPriorityWithAudio
    
DESCRIPTION
    Get the highest priority link with audio.

RETURNS
    link priority if match is found, invalid link if none found
*/
hfp_link_priority HfpLinkPriorityWithActiveCall(bool link_with_audio_wanted)
{
    hfp_link_priority priority;
    Sink sink;
    
    /* Find a link with audio and a call */
    for_all_priorities(priority)
    {
        /* If link has audio or we don't care about audio*/
        if(!link_with_audio_wanted || HfpLinkGetAudioSink(priority, &sink))
        {
            /* Return if it has a call ongoing */
            hfp_call_state call_state;
            HfpLinkGetCallState(priority, &call_state);
            switch(call_state)
            {
                case hfp_call_state_active:
                case hfp_call_state_twc_incoming:
                case hfp_call_state_twc_outgoing:
                case hfp_call_state_held_active:
                case hfp_call_state_multiparty:
                    return priority;

                default:
                    break;
            }
        }
    }
    
    /* Failed so just return the highest priority link with audio */
    if(link_with_audio_wanted)
        for_all_priorities(priority)
            if(HfpLinkGetAudioSink(priority, &sink))
                return priority;
    
    /* No links with audio, we failed completely */
    return hfp_invalid_link;
}


/****************************************************************************
NAME    
    HfpLinkPriorityFromAudioSink
    
DESCRIPTION
    try to find a match for current headset audio sink value

RETURNS
    link priority if match is found, invalid link if none found
*/
hfp_link_priority HfpLinkPriorityFromAudioSink(Sink sco_sink)
{
    /* Return if sco_sink matches one of the links */
    const hfp_link_data* link = hfpGetLinkFromAudioSink(sco_sink);
    return hfpGetLinkPriority(link);
}


/****************************************************************************
NAME    
    HfpLinkPriorityFromBdaddr
    
DESCRIPTION
    Try to find a match for a Bluetooth address

RETURNS
    link priority if match is found, invalid link if none 
    found
*/
hfp_link_priority HfpLinkPriorityFromBdaddr(const bdaddr* bd_addr)
{
    const hfp_link_data* link = hfpGetLinkFromBdaddr(bd_addr);
    if(link) 
        return link->bitfields.priority;

    return hfp_invalid_link;
}


/****************************************************************************
NAME    
    HfpLinkLoss
    
DESCRIPTION
    Query if HFP is currently attempting to reconnect any lost links

RETURNS
    TRUE if HFP is recovering from link loss, FALSE 
    otherwise
*/
bool HfpLinkLoss(void)
{
    const hfp_link_data*  link;
    
    for_all_links(link)
        if(link->bitfields.ag_link_loss_state == hfp_link_loss_recovery)
            return TRUE;
    
    return FALSE;
}

/****************************************************************************
NAME    
    HfpLinkLossLinkPriority
    
DESCRIPTION
    Query if HFP is currently attempting to reconnect any lost links and return
    the link priority if a match is found, invalid link if none found.

RETURNS
    Link priority of the lost link, invalid link if none found.
*/
hfp_link_priority HfpLinkLossLinkPriority(void)
{
    const hfp_link_data*  link;
    
    for_all_links(link)
    {
        if(link->bitfields.ag_link_loss_state == hfp_link_loss_recovery)
        {
            return link->bitfields.priority;
        }
    }
    
    return hfp_invalid_link;
}


/****************************************************************************
NAME    
    HfpLinkSetLinkMode
    
DESCRIPTION
    The link whether SC or non-SC is decided as a result of link encrypted after pairing.
    This function will set the link mode accordingly in HFP library.
    
*/
void HfpLinkSetLinkMode(hfp_link_priority priority , bool secure_link)
{
    hfp_link_data* link = hfpGetLinkFromPriority(priority);

    /* Set the link secure connection */
    if(link)
    {
        link->bitfields.link_mode_secure = secure_link;
    }
}

/****************************************************************************
NAME    
    HfpIsLinkSecure
    
DESCRIPTION
    This function will return the link mode, whether SC or non-SC,  set in HFP library.
    
*/
bool HfpIsLinkSecure(hfp_link_priority priority)
{
    hfp_link_data* link = hfpGetLinkFromPriority(priority);

    return (link->bitfields.link_mode_secure);
}



/****************************************************************************
NAME    
    HfpLinkPriorityManageLinkLoss
    
DESCRIPTION
    Enable/disable HFP library managed link loss recovery mechanism for the provided link priority.

*/
void HfpManageLinkLoss(hfp_link_priority link_priority_to_manage , bool enable)
{
    if(theHfp && (link_priority_to_manage != hfp_invalid_link))
    {
        hfp_link_priority lost_link_priority = HfpLinkLossLinkPriority();
        hfp_link_data *link_data = hfpGetLinkFromPriority(link_priority_to_manage);
        if(link_data)
        {
            link_data->bitfields.manage_linkloss = enable;
        }

        /*If the linkloss recovery has already started then abort it */
        if((link_priority_to_manage == lost_link_priority) && (enable == FALSE))
        {
            hfpHandleRfcommLinkLossAbort(&theHfp->links[lost_link_priority] , TRUE);                       
        }
    }
}


/****************************************************************************
NAME    
    HfpPriorityIsHsp
    
DESCRIPTION
    Query if the connection for the profile passed in is HSP profile

RETURNS
    TRUE if HSP profile otherwise FALSE
*/
bool HfpPriorityIsHsp(hfp_link_priority priority)
{
    const hfp_link_data* link = hfpGetLinkFromPriority(priority);
    
    return(hfpLinkIsHsp(link));
}
/*****************************************************************************
NAME    
    HfpGetFirstIncomingCallPriority
    
DESCRIPTION
    Function that provides information on priority that received the
    first incoming call	    

RETURNS
    The priority on which the first call came.
*/

hfp_link_priority HfpGetFirstIncomingCallPriority(void)
{
    return (hfp_link_priority)(theHfp->bitfields.first_incoming_call);
}


/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_link_manager.h
    
DESCRIPTION
    The link manager provides commonly used functionality for accessing links
    in hfp_task_data.
*/

#ifndef HFP_LINK_MANAGER_H_
#define HFP_LINK_MANAGER_H_

/* Iterate over all links */
#define for_all_links(link)             for(link = theHfp->links; link < &theHfp->links[theHfp->num_links]; link++)

/* Iterate over all priorities */
#define for_all_priorities(priority)    for(priority = hfp_primary_link; priority <= hfp_secondary_link; priority++)

/* Check if a link is connected  (FALSE if link is NULL) */
#define link_is_connected(link)         (bool)((link) && (link)->bitfields.ag_slc_state == hfp_slc_complete)

/* Check if link is disconnected (TRUE if link is NULL) */
#define link_is_disconnected(link)      (bool)(!(link) || (link)->bitfields.ag_slc_state < hfp_slc_complete)


/****************************************************************************
NAME
    hfpLinkSetup

DESCRIPTION
    Setup a link

RETURNS
    void
*/
void hfpLinkSetup(hfp_link_data* link, hfp_service_data* service, const bdaddr* bd_addr, Sink sink, hfp_slc_state state);


/****************************************************************************
NAME
    hfpLinkReset

DESCRIPTION
    Reset all the connection related state in this function.

RETURNS
    void
*/
void hfpLinkReset(hfp_link_data* link, bool promote_secondary);


/****************************************************************************
NAME    
    hfpSetLinkSink

DESCRIPTION
    Set the Sink for a given link

RETURNS
    void
*/
void hfpSetLinkSink(hfp_link_data* link, Sink sink);

/****************************************************************************
NAME    
    hfpGetLinkSink

DESCRIPTION
    Get the Sink for a given link

RETURNS
    The link's Sink if successful, otherwise NULL.
*/
Sink hfpGetLinkSink(const hfp_link_data* link);


/****************************************************************************
NAME    
    hfpSetLinkBdaddr

DESCRIPTION
    Set the bdaddr for a given link

RETURNS
    void
*/
void hfpSetLinkBdaddr(hfp_link_data* link, const bdaddr* bd_addr);


/****************************************************************************
NAME
    hfpSetLinkSlcState

DESCRIPTION
    Set the SLC state for a given link

RETURNS
    TRUE if state transition was allowed, FALSE otherwise
*/
void hfpSetLinkSlcState(hfp_link_data* link, hfp_slc_state slc_state);


/****************************************************************************
NAME
    hfpSetLinkCallState

DESCRIPTION
    Set the call state for a given link

RETURNS
    TRUE if state transition was allowed, FALSE otherwise
*/
void hfpSetLinkCallState(hfp_link_data* link, hfp_call_state call_state);

/****************************************************************************
NAME
    hfpSendLinkCallStateNotification

DESCRIPTION
    Send a call state notification to the VM application

RETURNS
    void
*/
void hfpSendLinkCallStateNotification(hfp_link_data* link, hfp_call_state call_state);


/****************************************************************************
NAME    
    hfpGetLinkBdaddr

DESCRIPTION
    Get the bdaddr for a given link

RETURNS
    If successful the bdaddr pointed to by bd_addr will be set to the link's
    bdaddr and the function will return TRUE. If not successful the function
    will return FALSE
*/
bool hfpGetLinkBdaddr(const hfp_link_data* link, bdaddr* bd_addr);

/****************************************************************************
NAME    
    hfpGetLinkFromSink

DESCRIPTION
    Get the link data corresponding to a given Sink

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromSink(Sink sink);


/****************************************************************************
NAME    
    hfpGetLinkFromBdaddr

DESCRIPTION
    Get the link data corresponding to a given bdaddr

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromBdaddr(const bdaddr* bd_addr);


/****************************************************************************
NAME    
    hfpGetLinkFromAudioSink

DESCRIPTION
    Get the link data corresponding to a given audio Sink

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromAudioSink(Sink sink);


/****************************************************************************
NAME    
    hfpGetIdleLink

DESCRIPTION
    Get link data for the first link in the idle SLC state

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetIdleLink(void);


/****************************************************************************
NAME    
    hfpGetLinkFromService

DESCRIPTION
    Get link data for the link using the given service

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromService(const hfp_service_data* service);


/****************************************************************************
NAME    
    hfpSetLinkPriority

DESCRIPTION
    Set the priority of a link

RETURNS
    void
*/
void hfpSetLinkPriority(hfp_link_data* link, hfp_link_priority priority);


/****************************************************************************
NAME    
    hfpGetLinkPriority

DESCRIPTION
    Get the priority of a link

RETURNS
    void
*/
hfp_link_priority hfpGetLinkPriority(const hfp_link_data* link);


/****************************************************************************
NAME    
    hfpGetLinkFromPriority

DESCRIPTION
    Get a link from it's priority

RETURNS
    Pointer to the corresponding hfp_link_data if successful. Otherwise NULL.
*/
hfp_link_data* hfpGetLinkFromPriority(hfp_link_priority priority);


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
MessageId hfpGetLinkTimeoutMessage(const hfp_link_data* link, MessageId timeout_base);


/****************************************************************************
NAME    
    hfpGetLinkFromTimeoutMessage

DESCRIPTION
    This function is used in handling the above timeouts, we need to know
    which link a command has timed out from.

RETURNS
    The link corresponding to the timeout MessageId
*/
hfp_link_data* hfpGetLinkFromTimeoutMessage(MessageId id);


/****************************************************************************
NAME    
    hfpLinkGetProfile

DESCRIPTION
    Get the profile being used by a link (if any)

RETURNS
    The profile if one is in use, otherwise hfp_profile_none
*/
hfp_profile hfpLinkGetProfile(const hfp_link_data* link);


/****************************************************************************
NAME    
    hfpLinkIsHsp

DESCRIPTION
    Check if a link is using HSP

RETURNS
    TRUE if it is, FALSE if not
*/
bool hfpLinkIsHsp(const hfp_link_data* link);


/****************************************************************************
NAME    
    hfpLinkIsHfp

DESCRIPTION
    Check if a link is using HFP

RETURNS
    TRUE if it is, FALSE if not
*/
bool hfpLinkIsHfp(const hfp_link_data* link);


/****************************************************************************
NAME
    hfpLinkIsHfp7

DESCRIPTION
    Check if a link is using HFP 1.7

RETURNS
    TRUE if it is, FALSE if not
*/
bool hfpLinkIsHfp107(const hfp_link_data* link);

#endif /* HFP_LINK_MANAGER_H_ */

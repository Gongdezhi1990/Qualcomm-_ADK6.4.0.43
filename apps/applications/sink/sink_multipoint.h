/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    This is the multipoint manager 
*/

#ifndef _HS_MULTIPOINT_H_
#define _HS_MULTIPOINT_H_

#include <hfp.h>

/****************************************************************************
NAME    
    sinkHandleCallInd
    
DESCRIPTION
   handle a call indication from the AG

RETURNS
    void
*/
void sinkHandleCallInd(const HFP_CALL_STATE_IND_T *pInd);

/****************************************************************************
NAME    
    MPCheckRingInd
    
DESCRIPTION
   check whether multipoint is enabled and there is more than one call,
   if so indicate a multipoint call waiting event, if not return false

RETURNS
   true or false as to whether the ring ind has been handled
*/
bool MPCheckRingInd(const HFP_RING_IND_T * pInd);

/****************************************************************************
NAME    
    MpReleaseAllHeld
    
DESCRIPTION
   terminate call found in the on hold audio state

RETURNS

*/
void MpReleaseAllHeld(void);

/****************************************************************************
NAME    
    MpAcceptWaitingReleaseActive
    
DESCRIPTION
   terminate call found in the active audio state and answer incoming call
   audio con/discon will take care of audio routing

RETURNS

*/
void MpAcceptWaitingReleaseActive(void);

/****************************************************************************
NAME    
    MpAcceptWaitingHoldActive
    
DESCRIPTION
   change state of active call to on hold state and answer incoming call, audio
   conn/discon will take care of routing audio
   
RETURNS

*/
void MpAcceptWaitingHoldActive(void);

/****************************************************************************
NAME    
    mpHandleCallWaitingInd
    
DESCRIPTION
   determine whether a call waiting indication notification needs to be played
   if the audio of the AG is not currently being routed
   
RETURNS

*/
void mpHandleCallWaitingInd(HFP_CALL_WAITING_IND_T * pInd);

/****************************************************************************
NAME    
    MpTwcAnswerCall
    
DESCRIPTION
   Called when the device is in a twc operating state to answer a call on a 
   second AG. If the device is currently routing audio, put this on hold ready to
   route the call once answered.
   
RETURNS

*/
void MpTwcAnswerCall(void);

/****************************************************************************
NAME    
    MpDropCall
    
DESCRIPTION
   performs a call drop on passed in AG depending upon call status, if the AG
   has a single call a CHUP command will be sent, if the AG has multiple calls
   the CHLD = 1, release active accept waiting will be sent 
   
RETURNS

*/
void MpDropCall(hfp_link_priority priority);

/****************************************************************************
NAME    
    MpHandleConferenceCall
    
DESCRIPTION
   can be passed true or false, when true a conference call will be created if
   an AG is found with 2 calls on it, one active and one held, when false an
   explicit call transfer will be performed if a conference call can be found, i.e.
   the AG is disconnected from the conference call 
   
RETURNS

*/
void MpHandleConferenceCall(bool create);

/****************************************************************************
NAME    
    mpSwitchToAG
    
DESCRIPTION
   helper function to switch between AGs where a call exists on both AGs
   
RETURNS

*/
void mpSwitchToAG(hfp_link_priority priority);

#endif

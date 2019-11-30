/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    This is the multipoint manager 
*/

#include "sink_multipoint.h"
#include "sink_callmanager.h"
#include "sink_statemanager.h"
#include "sink_tones.h"
#include "sink_audio.h"
#include "sink_configmanager.h"
#include "sink_slc.h"
#include "sink_audio_prompts.h"
#include "sink_audio_routing.h"
#include "sink_hfp_data.h"
#include "sink_volume.h"
#include "sink_main_task.h"
#include "sink_multipoint.h"
#include "sink_private_data.h"

#include <ps.h>
#include <connection.h>
#include <hfp.h>
#include <stdlib.h>
#include <audio.h>

#ifdef DEBUG_MULTI_MAN
    #define MP_DEBUG(x) DEBUG(x)
#else
    #define MP_DEBUG(x) 
#endif


/****************************************************************************
NAME    
    sinkHandleCallEnd
    
DESCRIPTION
    Change audio priority of call that has ended and set new priority for 
    call on other AG

RETURNS
    void
*/
static void sinkHandleCallEnd(hfp_link_priority priority, audio_priority audio_priority_new)
{
    hfp_link_priority link_priority_other = OTHER_PROFILE(priority);
    audio_priority audio_priority_other   = getScoPriorityFromHfpPriority(link_priority_other);
    
    /* Allow audio handler to immediately route audio from other call */
    setScoPriorityFromHfpPriority(priority, sco_about_to_disconnect);

    /* Update audio priority of other AG */
    if(audio_priority_other == sco_held_call)
        setScoPriorityFromHfpPriority(link_priority_other, audio_priority_new);
}

static bool callStateIndicatesDisconnect(hfp_call_state call_state)
{
    return (call_state == hfp_call_state_idle);
}

/****************************************************************************
NAME    
    sinkHandleCallInd
    
DESCRIPTION
   handle a call setup indication from the AG or AG's in the case of multipoint

RETURNS
    void
*/
void sinkHandleCallInd(const HFP_CALL_STATE_IND_T *pInd)
{
    /* initialise call state of other AG to idle, should a second AG not exist the 
       get call state function will fail */
    hfp_link_priority link_priority_other = OTHER_PROFILE(pInd->priority);
    hfp_call_state CallStateOtherAG = hfp_call_state_idle;
    hfp_call_state CallState = hfp_call_state_idle;
    bool OtherAGPresent;
    sinkState lSinkState = stateManagerGetState ();

    /* Check if call action was local and reset the flag */
    bool local_action = sinkHfpDataGetProfileStatusLocalCallAction(PROFILE_INDEX(pInd->priority));
    sinkHfpDataSetProfileLocalCallActionStatus(FALSE,PROFILE_INDEX(pInd->priority));

    /* it is possible to get an incorrect indication if AG1 is disconnected from the device mid call with a 
       call still on AG2, check for this and update as necessry */
    if(!((HfpLinkGetCallState(pInd->priority, &CallState))&&(CallState != pInd->call_state)))
        CallState = pInd->call_state;

    /* get call state of other AG */
    OtherAGPresent = HfpLinkGetCallState(link_priority_other, &CallStateOtherAG);
    
    MP_DEBUG(("MP: CallInd [%d] on AG%d, Other AG%d state = [%d]\n",pInd->call_state, pInd->priority, link_priority_other, CallStateOtherAG)) ;		

    /* determine call state */
    switch(CallState)
    {
       /* no longer in call */ 
       case hfp_call_state_idle:
       {
            /* if state was incoming call terminate ringtone */
            if(lSinkState == deviceIncomingCallEstablish ||
               lSinkState == deviceThreeWayCallWaiting )
            {
               if(!local_action) 
               {
                    MP_DEBUG(("MP: Call Rejected on AG\n"));
                    ToneTerminate();
               }
            }

            /* No longer in call, un mute the audio if main output is muted and 
                generate mute off event to notify the user. */
            if (volumeGetMuteState(audio_mute_group_main) == AUDIO_MUTE_ENABLE)
            {
               MessageSend(&theSink.task, EventUsrMainOutMuteOff, NULL);
            }

            /* theSink.RepeatCallerIDFlag needs to reset for the following states,
               while, ToneTerminate() needs to be done only for deviceIncomingCallEstablish
               state only. See B-170797 for details */
            if(lSinkState == deviceIncomingCallEstablish ||
               lSinkState == deviceThreeWayCallWaiting ||
               lSinkState == deviceThreeWayMulticall ||
               lSinkState == deviceThreeWayCallOnHold )
            {
               sinkHfpDataSetRepeatCallerIDFlag(TRUE);
            }
#ifdef ENABLE_PEER
            /* When the call has ended/inactive/idle: update the TWS peer state */
            peerUpdateLocalStatusOnCallEnd();
#endif            
      		/* determine whether there is another AG and it has an active call 
               of some description on it */
            if(OtherAGPresent)            
            {
                switch(CallStateOtherAG)
                {
                    case hfp_call_state_multiparty:
                        sinkHandleCallEnd(pInd->priority, sco_active_call);
                        stateManagerEnterThreeWayMulticallState();
                    break;

                    case hfp_call_state_held_remaining:
                    break;
    
                    case hfp_call_state_twc_outgoing:
                    case hfp_call_state_held_active:
                        sinkHandleCallEnd(pInd->priority, sco_active_call);
                        stateManagerEnterThreeWayCallOnHoldState();
                    break;

                    case hfp_call_state_twc_incoming:
                        sinkHandleCallEnd(pInd->priority, sco_active_call);
                        stateManagerEnterThreeWayCallWaitingState();
                    break;
    
                    case hfp_call_state_active:
                        sinkHandleCallEnd(pInd->priority, sco_active_call);
                        /* down to one call now so change to active call state */        
                        stateManagerEnterActiveCallState();
                    break;

                    case hfp_call_state_incoming_held:
                        stateManagerEnterThreeWayCallOnHoldState();  
                    break;
    
                    case hfp_call_state_incoming:
                        sinkHandleCallEnd(pInd->priority, sco_inband_ring);
                        /* indication of an incoming call */     
                        stateManagerEnterIncomingCallEstablishState();
                    break;
    
                    case hfp_call_state_outgoing:
                        /* indication of an outgoing call setup */
                        sinkHandleCallEnd(pInd->priority, sco_active_call);
                        stateManagerEnterOutgoingCallEstablishState();
                    break;
    
                    case hfp_call_state_idle:
                    default:
                        MP_DEBUG(("MP: CallInd idle on AG%d = connected\n",pInd->priority)) ;		
                        /* call now finished, update device state */
                        stateManagerEnterConnectedState();
                    break;
                }
            }
            /* only 1 AG now connected, no calls, go to connected state */
            else
            {
                stateManagerEnterConnectedState();
            }
            
            /*if we are muted - then un mute when no further calls are available on this AG*/
            if (sinkHfpDataGetAudioGMuted(PROFILE_INDEX(pInd->priority)))
            {
                MP_DEBUG(("MP: UnMute AG%d\n",pInd->priority)) ;		
                VolumeSetHfpMicrophoneGainCheckMute(pInd->priority, MICROPHONE_MUTE_OFF);
          	}
       }
       break;
       
       /* incoming call indication */
       case hfp_call_state_incoming:
          MP_DEBUG(("MP: CallInd incoming on AG%d = IncCallEst\n",pInd->priority)) ;	

           /* if we got the sco connection before this call indication then ensure sco state is correct */
           setScoPriorityFromHfpPriority(pInd->priority, sco_inband_ring);

           
#ifdef ENABLE_PEER
            /* When the call is active or incoming call is being established, mark the relay stream as unavailable */
	    	peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_CALL_ACTIVE);
#endif

           /* determine the number of AG's & active or held calls */
           if(OtherAGPresent && (CallStateOtherAG >= hfp_call_state_incoming))
           {
               /* change state to TWC call waiting and indicate call via mp call waiting event */
               stateManagerEnterThreeWayCallWaitingState();      
           }
           /* non multipoint or only 1 call */
           else
           {
               /* indication of an incoming call */     
               stateManagerEnterIncomingCallEstablishState();
           }
 
       break;

       /* indication of an incoming call being put on hold */
       case hfp_call_state_incoming_held:
           /* determine the number of AG's & active or held calls */
           if(OtherAGPresent && (CallStateOtherAG >= hfp_call_state_outgoing))
           {
               /* change state to TWC call waiting and indicate call via mp call waiting event */
               stateManagerEnterThreeWayCallOnHoldState();                                                 
           }
           /* non multipoint or only 1 call */
           else
           {
               /* change to incoming call on hold state */
               stateManagerEnterIncomingCallOnHoldState();
           }
       break;   		
       
       /* indication of an outgoing call */
       case hfp_call_state_outgoing:
    	   MP_DEBUG(("MP: CallInd outgoing on AG%d = outgoingEstablish\n",pInd->priority)) ;

           /* update the last used AG when making an outgoing call */
           sinkHfpDataSetLastOutgoingAg(pInd->priority);
           
           /* outgoing call establish, now in active call state */
           setScoPriorityFromHfpPriority(pInd->priority, sco_active_call); 

#ifdef ENABLE_PEER
           /* When the call is active or incoming call/outgoing is being established, mark the relay stream as unavailable */
           peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_CALL_ACTIVE);
#endif
           
           /* determine the number of AG's & active or held calls */
           if(OtherAGPresent && (CallStateOtherAG >= hfp_call_state_outgoing))
           {
               /* set the sco state of the current active call to on hold */
               setScoPriorityFromHfpPriority(link_priority_other, sco_held_call);                    
               /* enter on hold state */
               stateManagerEnterThreeWayCallOnHoldState();
           }
           /* for the case where one AG is ringing and the other AG makes an outgoing call
              go to call waiting state */
           else if(OtherAGPresent && (CallStateOtherAG == hfp_call_state_incoming))
           {
               /* change state to TWC call waiting and indicate call via mp call waiting event */
               stateManagerEnterThreeWayCallWaitingState();                     
           }
           /* non multipoint or only call */
           else
           {
                /* indication of an outgoing call */     
                stateManagerEnterOutgoingCallEstablishState();
           }
       break;

       /* call is now in active call state, answered incoming or outgoing call */
       case hfp_call_state_active:
     	   MP_DEBUG(("MP: CallInd active on AG%d = ActiveCall\n",pInd->priority)) ;		
           /* if state was incoming call terminate ringtone */
            if(lSinkState == deviceIncomingCallEstablish ||
               lSinkState == deviceThreeWayCallWaiting )
           {
               if(!local_action) 
               {
                    MP_DEBUG(("MP: Call Accepted on AG\n"));
                    ToneTerminate();
               }
                sinkHfpDataSetRepeatCallerIDFlag(TRUE);
           }

#ifdef ENABLE_PEER
           {
               Sink sco_sink;
               
               /* Check if there is an active SCO connection */               
               HfpLinkGetAudioSink( pInd->priority, &sco_sink );
               
               if( sco_sink )
               {
                    /* Mark the relay stream as unavailable */
                    peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_CALL_ACTIVE);
               }
           }
#endif           
           
           /* if this is the indication of a call with in band ring having been answered,
              update the audio priority as this is now an active call */
           if(getScoPriorityFromHfpPriority(pInd->priority) == sco_inband_ring)
           {
               /* now in active call audio state */
               setScoPriorityFromHfpPriority(pInd->priority, sco_active_call);                    
           }
           
           /* determine the number of AG's & active or held calls */
           if(OtherAGPresent && (CallStateOtherAG >= hfp_call_state_outgoing))
           {
               /* set the sco state of the current active call to on hold */
               setScoPriorityFromHfpPriority(link_priority_other, sco_held_call);                    
               /* enter on hold state */
               stateManagerEnterThreeWayCallOnHoldState();
           }
           /* non multipoint or only active call */
           else
           {
               /* check whether the other AG has an incoming call on it, use case of power on
                  to two incoming calls, if so go to twc waiting state such that second incoming
                  call can be handled by use of twc events */
               if(OtherAGPresent && (CallStateOtherAG == hfp_call_state_incoming))
                   stateManagerEnterThreeWayCallWaitingState();   
               /* no second incoming call on other AG */
               else
                   stateManagerEnterActiveCallState();
           }
       break;

       /* indication of a second incoming call */
       case hfp_call_state_twc_incoming:
      	   MP_DEBUG(("MP: CallInd twcIncoming on AG%d = callWaiting\n",pInd->priority)) ;		
           /* indication of a three way call situation with an incoming 
              second call */
           stateManagerEnterThreeWayCallWaitingState();   
       break;

       /* indication of a second call which is an outgoing call */
       case hfp_call_state_twc_outgoing:
      	   MP_DEBUG(("MP: CallInd twcOutgoing on AG%d = CallOnHold\n",pInd->priority)) ;		
           /* update the last used AG when making an outgoing call */
           sinkHfpDataSetLastOutgoingAg(pInd->priority);
           /* twc outgoing call establish, now in call on hold call state */
           setScoPriorityFromHfpPriority(pInd->priority, sco_active_call);                    
           /* determine the number of AG's & active or held calls */
           if(OtherAGPresent && (CallStateOtherAG >= hfp_call_state_outgoing))
           {
               /* set the sco state of the current active call to on hold */
               setScoPriorityFromHfpPriority(link_priority_other, sco_held_call);                    
           }      
           /* indication of a twc on hold state change due to
              second outgoing call */
           stateManagerEnterThreeWayCallOnHoldState();
       break;

       case hfp_call_state_held_active:
      	   MP_DEBUG(("MP: CallInd heldActive on AG%d = CallOnHold\n",pInd->priority)) ;	
           stateManagerEnterThreeWayCallOnHoldState();
       break;

       case hfp_call_state_held_remaining:
      	   MP_DEBUG(("MP: CallInd heldRemaining on AG%d = >\n",pInd->priority)) ;		
       break;

       case hfp_call_state_multiparty:
       	   MP_DEBUG(("MP: CallInd multiparty on AG%d = twcMultiCall>\n",pInd->priority)) ;		
           /* indication of conference call */
           stateManagerEnterThreeWayMulticallState();
       break;      
    }
    
    if(callStateIndicatesDisconnect(CallState))
    {
        audioUpdateAudioRoutingAfterDisconnect();
    }
    else
    {
    	audioUpdateAudioRouting();
    }
}

/****************************************************************************
NAME    
    MPCheckRingInd
    
DESCRIPTION
   check whether multipoint is enabled and there is more than one call,
   if so indicate a multipoint call waiting event, if not return false

RETURNS
   true or false as to whether the ring ind has been handled
*/
bool MPCheckRingInd(const HFP_RING_IND_T * pInd)
{  
    /* determine the number of AG's & active or held calls */
    if((sinkDataIsMultipointEnabled()) && 
       (sinkAudioIsVoiceRouted()) &&
       (HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink()) != pInd->priority))
    {
        hfp_call_state CallStateOtherAG;
        HfpLinkGetCallState(OTHER_PROFILE(pInd->priority), &CallStateOtherAG); 
        
        /* check whether the SCO on the other phone has a call associated with it,
           might just be SCO caused by pressing buttons on other phone */
        if(CallStateOtherAG == hfp_call_state_idle)
        {
            /* no call on other phone so indicate as normal */
            return FALSE;
        }
        /* other AG has a call on it, indicate this call by call waiting tone */
        else
        {
            /* already have another call so indicate a call waiting event */
            MessageSend(&theSink.task,EventSysMultipointCallWaiting,0);
            /* ring ind handled so return true */
            return TRUE;
        }
    }  
    /* not a multipoint twc ring ind situation */
    return FALSE;    
}

/****************************************************************************
NAME    
    MpReleaseAllHeld
    
DESCRIPTION
   terminate call found in the on hold audio state

RETURNS

*/
void MpReleaseAllHeld(void)
{
    /* there may be up three lots of calls to release, determine which AG's
       have held calls and whether multipoint is in use also */
    hfp_link_priority priorityActive;
    hfp_call_state stateActive = hfp_call_state_idle;  
    hfp_call_state stateOther = hfp_call_state_idle;  
    
    MP_DEBUG(("MP: ReleaseAllHeld \n")) ;		

    /* are there multiple AG's connected? */
    if(deviceManagerNumConnectedDevs() > 1)
    {
        /* determine active AG by finding out which AG has its audio routed, if any */
        priorityActive = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());
        /* if audio routed check call state */
        if(priorityActive)
        {
            /* get call state of active and other AGs */
            HfpLinkGetCallState(priorityActive, &stateActive);  
            HfpLinkGetCallState(OTHER_PROFILE(priorityActive), &stateOther);  
            /* deteremine whether active AG has more than one call that needs ending */
            if((stateActive) && ((stateActive == hfp_call_state_held_active)||
                                 (stateActive == hfp_call_state_twc_incoming)||
                                 (stateActive == hfp_call_state_twc_outgoing)||
                                 (stateActive == hfp_call_state_held_remaining)||
                                 (stateActive == hfp_call_state_incoming_held)))
            {
                MP_DEBUG(("MP: RAH Act state %d release held/reject on AG%d \n",stateActive, priorityActive)) ;		
                /* release held and waiting calls */
                HfpCallHoldActionRequest(priorityActive, hfp_chld_release_held_reject_waiting, 0);
            }
            /* determine whether other AG has calls, active or waiting */           
            if(stateOther)
            {
                /* are there held or call waiting calls? */
                if((stateOther == hfp_call_state_held_active)||
                   (stateOther == hfp_call_state_twc_incoming)||                  
                   (stateOther == hfp_call_state_twc_outgoing)||
                   (stateOther == hfp_call_state_multiparty)||
                   (stateOther == hfp_call_state_held_remaining)||
                   (stateOther == hfp_call_state_incoming_held))
                {
                    MP_DEBUG(("MP: RAH Hld state %d release held/reject on AG%d \n",stateOther, OTHER_PROFILE(priorityActive))) ;		
                    /* release held and waiting calls */
                    HfpCallHoldActionRequest(OTHER_PROFILE(priorityActive), hfp_chld_release_held_reject_waiting, 0);
                    /* if another call on AG terinate it */                    
                    if((stateOther != hfp_call_state_held_remaining)&&(stateOther != hfp_call_state_incoming_held))
                    {
                        MP_DEBUG(("MP: RAH Hld state %d release held/reject + terminate on AG%d \n",stateOther, OTHER_PROFILE(priorityActive))) ;		
                        /* end active or outgoing calls */
                        HfpCallTerminateRequest(OTHER_PROFILE(priorityActive));   
                    }
                }
                /* also check if other AG has an active call */
                if((stateOther == hfp_call_state_active)||
                   (stateOther == hfp_call_state_outgoing))
                {
                    MP_DEBUG(("MP: RAH Hld state %d terminate on AG%d \n",stateOther, OTHER_PROFILE(priorityActive))) ;		
                    /* end active or outgoing calls */
                    HfpCallTerminateRequest(OTHER_PROFILE(priorityActive));   
                }
                /* for incoming call */
                if(stateOther == hfp_call_state_incoming)
                {
                    MP_DEBUG(("MP: RAH Hld state %d reject on AG%d \n",stateOther, OTHER_PROFILE(priorityActive))) ;		
                    /* reject incoming call */
                    HfpCallAnswerRequest(OTHER_PROFILE(priorityActive), FALSE);
                }
            }
        }
    }
    /* only one AG connected so issue release all held cmd */
    else
    {
        /* perform release all held on appropriate AG */
        HfpCallHoldActionRequest(hfp_primary_link, hfp_chld_release_held_reject_waiting, 0);
    }        
}

/****************************************************************************
NAME    
    MpAcceptWaitingReleaseActive
    
DESCRIPTION
   terminate call found in the active audio state and answer incoming call
   audio con/discon will take care of audio routing

RETURNS

*/
void MpAcceptWaitingReleaseActive(void)
{
    hfp_link_priority priorityActive;
    hfp_call_state CallStateAG1 = hfp_call_state_idle;
    hfp_call_state CallStateAG2 = hfp_call_state_idle;

    /* determine which is the currently active call, check for an outgoing call first 
       which may not have sco open */
    priorityActive = HfpLinkPriorityFromCallState(hfp_call_state_outgoing);
    /* if no outgoing call check for outgoing call as part of a three way call */    
    if(!priorityActive)
    {
        /* outgoing as part of twc? */        
        priorityActive = HfpLinkPriorityFromCallState(hfp_call_state_twc_outgoing);
        /* if no outgoing call then choose the active call based on the current sco being routed */
        if(!priorityActive)
            priorityActive = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());
    }    
    /* determine the call state of AG1 */
    HfpLinkGetCallState(hfp_primary_link, &CallStateAG1);
    /* determine the call state of AG2 */
    HfpLinkGetCallState(hfp_secondary_link, &CallStateAG2);
    /* if no active link is found check for the prescence of held calls with no sco */
    if(priorityActive == hfp_invalid_link)
    {
        /* does AG1 have a held call ? */
        if((CallStateAG1 == hfp_call_state_held_remaining)||(CallStateAG1 == hfp_call_state_held_active))
            priorityActive = hfp_primary_link;
        /* does AG2 havea  held call ? */
        else if((CallStateAG2 == hfp_call_state_held_remaining)||(CallStateAG2 == hfp_call_state_held_active))
            priorityActive = hfp_secondary_link;            
    }
    

    /* If both State are incoming, pick the call that came first */
    if( (CallStateAG1 == hfp_call_state_incoming) && (CallStateAG2 == hfp_call_state_incoming) )
    {    
        priorityActive = OTHER_PROFILE(HfpGetFirstIncomingCallPriority());
    }        
    
    
    MP_DEBUG(("MP: MpAcceptWaitingReleaseActive Priority = %d, State AG1 = %d, AG2 = %d\n",priorityActive,CallStateAG1,CallStateAG2)) ;		

    /*  AG1 has an active call, check for call(s) on AG2 */
    if((priorityActive == hfp_primary_link)&&((CallStateAG2 == hfp_call_state_incoming)||
                                              (CallStateAG2 == hfp_call_state_twc_incoming)||
                                              (CallStateAG2 == hfp_call_state_held_remaining)||
                                              (CallStateAG2 == hfp_call_state_held_active)))
    {
        /* drop the call on AG1 which is the active call */        
        MpDropCall(hfp_primary_link);
        /* is the call on AG2 a single incoming call?, if so need to send ATA, if there is more than one call
           on AG2 then issue the CHLD=2 */
        if(CallStateAG2 == hfp_call_state_incoming)        
        {
            /* accept incoming call */
            HfpCallAnswerRequest(hfp_secondary_link, TRUE);
            sinkHfpDataSetProfileLocalCallActionStatus(TRUE, PROFILE_INDEX(hfp_secondary_link));
            ToneTerminate();
        }
        /* call waiting is second call on AG2 */
        else
        {
            /* accept call on AG2 */
            HfpCallHoldActionRequest(hfp_secondary_link, hfp_chld_hold_active_accept_other, 0);
        }            
        /* audio call swap required as accepting call on other AG */
        setScoPriorityFromHfpPriority(hfp_primary_link, sco_held_call);                    
        setScoPriorityFromHfpPriority(hfp_secondary_link, sco_active_call); 
        /* if AG2 is muted, unmute it if answering another incoming call */
        if(sinkHfpDataGetAudioGMuted(PROFILE_INDEX(hfp_secondary_link))&&
           ((CallStateAG2 == hfp_call_state_incoming)||(CallStateAG2 == hfp_call_state_twc_incoming)))
        {
            /* unmute new active call */            
            VolumeSetHfpMicrophoneGainCheckMute(hfp_secondary_link, MICROPHONE_MUTE_OFF);
        }                                             
    }
    /*  or AG2 active call but call waiting is on AG1 and AG1 has only one call */
    else if((priorityActive == hfp_secondary_link)&&((CallStateAG1 == hfp_call_state_incoming)||
                                                     (CallStateAG1 == hfp_call_state_twc_incoming)||
                                                     (CallStateAG1 == hfp_call_state_held_remaining)||
                                                     (CallStateAG1 == hfp_call_state_held_active)))
    {
        /* drop the call on AG2 which is the active call */        
        MpDropCall(hfp_secondary_link);
        /* is the call on AG1 a single incoming call?, if so need to send ATA, if there is more than one call
           on AG1 then issue the CHLD=2 */       
        if(CallStateAG1 == hfp_call_state_incoming)        
        {
            /* accept waiting call */
            HfpCallAnswerRequest(hfp_primary_link, TRUE);
            sinkHfpDataSetProfileLocalCallActionStatus(TRUE, PROFILE_INDEX(hfp_primary_link));
            ToneTerminate();
        }
        else
        {
            /* accept call on AG1 */
            HfpCallHoldActionRequest(hfp_primary_link, hfp_chld_hold_active_accept_other, 0);           
        }
        /* audio call swap required as accepting call on other AG */
        setScoPriorityFromHfpPriority(hfp_primary_link, sco_active_call);                    
        setScoPriorityFromHfpPriority(hfp_secondary_link, sco_held_call);                    
        /* if AG1 is muted, unmute it if answering another incoming call */
        if((sinkHfpDataGetAudioGMuted(PROFILE_INDEX(hfp_primary_link))) &&
           ((CallStateAG1 == hfp_call_state_incoming)||(CallStateAG1 == hfp_call_state_twc_incoming)))
        {
            /* unmute new active call */            
            VolumeSetHfpMicrophoneGainCheckMute(hfp_primary_link, MICROPHONE_MUTE_OFF);
        }                                             
    }
    /* or AG1 is active call with second incoming call on AG1, cover both call waiting and call held cases
       whereby CHLD = 1 is required to drop active call */
    else if((priorityActive == hfp_primary_link)&&((CallStateAG1 == hfp_call_state_twc_incoming)||
                                                   (CallStateAG1 == hfp_call_state_held_active)||
                                                   (CallStateAG1 == hfp_call_state_multiparty)))
    {
        /* active call is on AG1 with call waiting on AG1 so send CHLD = 1, rel and accept */   
        HfpCallHoldActionRequest(hfp_primary_link, hfp_chld_release_active_accept_other, 0);
        /* no call swap required as already on AG1 with incoming call */
        /* if answering a new call and AG is muted, unmute */        
        if((sinkHfpDataGetAudioGMuted(PROFILE_INDEX(hfp_primary_link)))&&
           (CallStateAG1 == hfp_call_state_twc_incoming))
        {
            /* unmute new active call */            
            VolumeSetHfpMicrophoneGainCheckMute(hfp_primary_link, MICROPHONE_MUTE_OFF);
        }                                             
    }
    /* or AG2 is active call with incoming call on AG2, cover both call waiting and call held cases
       whereby CHLD = 1 is required to drop active call */
    else if((priorityActive == hfp_secondary_link)&&((CallStateAG2 == hfp_call_state_twc_incoming)||
                                                     (CallStateAG2 == hfp_call_state_held_active)||
                                                     (CallStateAG2 == hfp_call_state_multiparty)))
    {
        /* active call is on AG2 with call waiting on AG2 so send CHLD = 1, rel and accept */   
        HfpCallHoldActionRequest(hfp_secondary_link, hfp_chld_release_active_accept_other, 0);        
        /* no call swap required as already on AG2 with incoming call */
        /* if answering a new call and AG is muted, unmute */        
        if(sinkHfpDataGetAudioGMuted(PROFILE_INDEX(hfp_secondary_link)) &&
           (CallStateAG2 == hfp_call_state_twc_incoming))
        {
            /* unmute new active call */            
            VolumeSetHfpMicrophoneGainCheckMute(hfp_secondary_link, MICROPHONE_MUTE_OFF);
        }                                             
    }
    /* one call on each AG, drop the active call */
    else 
    {
        /* if there is an active call with sco */
        if(priorityActive)
            MpDropCall(priorityActive);
        /* if there is no active call with sco, check for active call without sco as being the only
           other possibilty */
        else
        {
            /* does AG1 have an active call without sco ? */
            if((CallStateAG1 == hfp_call_state_active)&&
               (getScoPriorityFromHfpPriority(hfp_primary_link) == sco_none))
            {
                MpDropCall(hfp_primary_link);
            }
            /* does AG2 have an active call without sco ? */
            else if((CallStateAG2 == hfp_call_state_active)&&
                    (getScoPriorityFromHfpPriority(hfp_secondary_link) == sco_none))
            {
                MpDropCall(hfp_secondary_link);
            }
        }
    }
    
    /* redo audio routing */
    audioUpdateAudioRouting();

}

/****************************************************************************
NAME    
    MpAcceptWaitingHoldActive
    
DESCRIPTION
   change state of active call to on hold state and answer incoming call, audio
   conn/discon will take care of routing audio
   
RETURNS

*/
void MpAcceptWaitingHoldActive(void)
{
    hfp_link_priority priority = hfp_invalid_link;
    uint8 NoOfCallsAG1 = 0;
    uint8 NoOfCallsAG2 = 0;
    hfp_call_state CallStateAG1 = hfp_call_state_idle;
    hfp_call_state CallStateAG2 = hfp_call_state_idle;
    hfp_link_priority priority_audio_sink = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());
    
    /* determine the number of calls on this device */
    /* find number of calls on AG 1 */
    if(HfpLinkGetCallState(hfp_primary_link, &CallStateAG1))
    {
        /* if no TWC state then only 1 call */
        if((CallStateAG1 == hfp_call_state_active)||
           (CallStateAG1 == hfp_call_state_twc_incoming)||
           (CallStateAG1 == hfp_call_state_held_remaining)||
           (CallStateAG1 == hfp_call_state_multiparty))
            NoOfCallsAG1 = 1;
        else if(CallStateAG1 == hfp_call_state_held_active)
            NoOfCallsAG1 = 2;
    }    
    /* find number of calls on AG 2 */
    if(HfpLinkGetCallState(hfp_secondary_link, &CallStateAG2))
    {
        /* if no TWC then only 1 call */
        if((CallStateAG2 == hfp_call_state_active)||
           (CallStateAG2 == hfp_call_state_twc_incoming)||
           (CallStateAG2 == hfp_call_state_held_remaining)||
           (CallStateAG2 == hfp_call_state_multiparty))
            NoOfCallsAG2 = 1;
        else if(CallStateAG2 == hfp_call_state_held_active)
            NoOfCallsAG2 = 2;
    }

    MP_DEBUG(("MP: HoldSwap State AG1 = %d,%d State AG2 = %d,%d \n",CallStateAG1,NoOfCallsAG1,CallStateAG2,NoOfCallsAG2)) ;		
    
    /* look for twc call waiting AG state */
    priority = HfpLinkPriorityFromCallState(hfp_call_state_twc_incoming);  
    /* if not found look for outgoing call state to swap between active and twc outgoing */    
    if(!priority)    
    {
        /* look for held call */
        priority = HfpLinkPriorityFromCallState(hfp_call_state_held_active);
        /* if not found look for outgoing call state to swap between active and twc outgoing */    
        if(!priority)    
        {
            /* look for outgoing call */
            priority = HfpLinkPriorityFromCallState(hfp_call_state_twc_outgoing);
            /* if not found get active call instead */
            if(!priority)    
            {
                priority = HfpLinkPriorityFromCallState(hfp_call_state_active);
                /* finally look for held remaining in the case of a single call that has been put on hold */
                if(!priority)    
                {
                    /* look for active call */
                    priority = HfpLinkPriorityFromCallState(hfp_call_state_held_remaining);
                }                
            }
        }        
    }
    
    /* if there is not an active call on each AG but one active call on one AG and
       an incoming call on the other AG */
    if(!(NoOfCallsAG1 && NoOfCallsAG2)&&
       ((NoOfCallsAG1 && (CallStateAG2 == hfp_call_state_incoming))||
        (NoOfCallsAG2 && (CallStateAG1 == hfp_call_state_incoming)))
      )       
    {
        /* answer call on incoming AG and hold call audio on active AG */
        MpTwcAnswerCall();
        /* set the held call index such that it will correctly switch to the next
           call next time */
        sinkHfpDataSetHeldCallIndex(NoOfCallsAG1 + 1);
    }
    /* if there are calls on both AG's and an incoming then answer it and hold other or
       if there are no incoming calls then cycle around the existing calls in order */
    else if(NoOfCallsAG1 && NoOfCallsAG2)
    {
        /* is there another incoming call to answer? */
        if((NoOfCallsAG1 && (CallStateAG2 == hfp_call_state_incoming))||
           (NoOfCallsAG2 && (CallStateAG1 == hfp_call_state_incoming))
          )
        {
            /* answer incoming call and hold others */            
            MpTwcAnswerCall();
        }
        /* or a call waiting on one of the AGs */        
        else if(CallStateAG1 == hfp_call_state_twc_incoming)
        {
            HfpCallHoldActionRequest(hfp_primary_link, hfp_chld_hold_active_accept_other, 0);           
            /* ensure the audio of answered call is being routed */
            if(getScoPriorityFromHfpPriority(hfp_primary_link) == sco_held_call)        
            {
                setScoPriorityFromHfpPriority(hfp_primary_link, sco_active_call);                     
                setScoPriorityFromHfpPriority(hfp_secondary_link, sco_held_call);                     
            }
            /* if AG1 is muted, unmute it if answering another incoming call */
            if(sinkHfpDataGetAudioGMuted(PROFILE_INDEX(hfp_primary_link)))
            {
                /* unmute new active call */            
                VolumeSetHfpMicrophoneGainCheckMute(hfp_primary_link, MICROPHONE_MUTE_OFF);
            }                                             
        }
        /* or a call waiting on other AG */        
        else if(CallStateAG2 == hfp_call_state_twc_incoming)
        {
            HfpCallHoldActionRequest(hfp_secondary_link, hfp_chld_hold_active_accept_other, 0);
            /* ensure the audio of answered call is being routed */
            if(getScoPriorityFromHfpPriority(hfp_secondary_link) == sco_held_call)        
            {
                setScoPriorityFromHfpPriority(hfp_secondary_link, sco_active_call);                     
                setScoPriorityFromHfpPriority(hfp_primary_link, sco_held_call);                     
            }
            /* if AG2 is muted, unmute it if answering another incoming call */
            if(sinkHfpDataGetAudioGMuted(PROFILE_INDEX(hfp_secondary_link)))
            {
                /* unmute new active call */            
                VolumeSetHfpMicrophoneGainCheckMute(hfp_secondary_link, MICROPHONE_MUTE_OFF);
            }                                             
        }
        /* no incoming call so cycle around existing calls */
        else
        {
            /* there are calls on both AG's, therefore cycle round the available calls via
               the call index pointer in the order of AG1 then AG2 and back to AG1 calls */
            /* swap to next call */
            sinkHfpDataSetHeldCallIndex(sinkHfpDataGetHeldCallIndex() + 1);
            
            MP_DEBUG(("MP: HoldSwap multiple calls show %d\n",sinkHfpDataGetHeldCallIndex())) ;

            /* is this on AG1 or AG2 or back to AG1 ?*/
            if(sinkHfpDataGetHeldCallIndex() > NoOfCallsAG1)
            {
                /* is index still a call on AG2 ? */
                if(sinkHfpDataGetHeldCallIndex() <= (NoOfCallsAG1 + NoOfCallsAG2))
                {                        
                    MP_DEBUG(("MP: HoldSwap call on AG2 \n")) ;		
                    /* if AG2 is not the routed audio then switch to it */
                    if(priority_audio_sink != hfp_secondary_link)
                    {
                        MP_DEBUG(("MP: HoldSwap swap audio routing to AG2 \n")) ;		
                        /* the device is not currently routing the audio from this AG so swap over */
                        /* switch to AG2 */
                        mpSwitchToAG(hfp_secondary_link);
                    }
                    /* audio already routed from this AG, switch calls if applicable */
                    else if(NoOfCallsAG2>1)
                    {
                        /* if there are multiple calls on AG2 then swap between then */
                        MP_DEBUG(("MP: HoldSwap Send CHLD to AG2 \n")) ;		
                        HfpCallHoldActionRequest(hfp_secondary_link, hfp_chld_hold_active_accept_other, 0);
                    }
                    /* in error condition, audio already routed to this AG, AG only has one call so swap to
                       other AG if available */
                    else if(NoOfCallsAG1)
                    {
                        MP_DEBUG(("MP: HoldSwap Error: swap audio to AG1 \n")) ;		
                        /* switch to AG1 */
                        mpSwitchToAG(hfp_primary_link);
                    }
                }
                /* index now higher than all calls so back to next suitable index */
                else
                {
                    MP_DEBUG(("MP: HoldSwap call on AG1 - rollover - show 1\n")) ;		
                    /* can index be set to AG1 or is that already having its audio routed? */
                    if(priority_audio_sink != hfp_primary_link)                  
                    {
                        sinkHfpDataSetHeldCallIndex(1);
                        /* if there were two calls on the secondary AG swap them again to maintain
                           the same scrolling order next time around */
                        if(NoOfCallsAG2 > 1)
                           HfpCallHoldActionRequest(hfp_secondary_link, hfp_chld_hold_active_accept_other, 0);                           
                        /* if there were two calls on the primary AG also swap them again to maintain
                           the same scrolling order next time around */
                        if(NoOfCallsAG1 > 1)
                           HfpCallHoldActionRequest(hfp_primary_link, hfp_chld_hold_active_accept_other, 0);                           
                        /* back to AG1, is this routed */
                        if(priority_audio_sink != hfp_primary_link)
                        {
                            MP_DEBUG(("MP: HoldSwap swap audio on AG 1 \n")) ;		
                            /* the device is not currently routing the audio from this AG so swap over */
                            /* switch to AG1 */
                            mpSwitchToAG(hfp_primary_link);
                        }
                    }
                    /* error condition as AG1 is already routed, set as AG2 if AG2 has available calls */
                    else if(NoOfCallsAG2)
                    {
                        /* set to AG2 */                        
                        sinkHfpDataSetHeldCallIndex(NoOfCallsAG1+1);
                        MP_DEBUG(("MP: HoldSwap swap audio on AG 2 \n")) ;		
                        /* the device is not currently routing the audio from this AG so swap over */
                        /* switch to AG2 */
                        mpSwitchToAG(hfp_secondary_link);
                    }
                }
            }
            /* index is a call on AG1 */
            else
            {
                MP_DEBUG(("MP: HoldSwap call on AG1 \n")) ;		
                /* if AG1 is not the routed audio then switch to it */
                if(priority_audio_sink != hfp_primary_link)
                {
                    MP_DEBUG(("MP: HoldSwap swap audio to AG1 \n")) ;		
                    /* the device is not currently routing the audio from this AG so swap over */
                    /* switch to AG1 */
                    mpSwitchToAG(hfp_primary_link);
               }
                /* audio already routed from this AG, switch calls if applicable */
                else if(NoOfCallsAG1>1)
                {
                    /* if there are multiple calls on AG2 then swap between then */
                    MP_DEBUG(("MP: HoldSwap send CHLD to AG1 \n")) ;		
                    HfpCallHoldActionRequest(hfp_primary_link, hfp_chld_hold_active_accept_other, 0);
                }
                /* in error condition, audio already routed to this AG, AG only has one call so swap to
                   other AG if available */
                else if(NoOfCallsAG2)
                {
                    MP_DEBUG(("MP: HoldSwap Error: swap audio to AG2 \n")) ;		
                    /* switch to AG2 */
                    mpSwitchToAG(hfp_secondary_link);
                }
            }
        }
    }
    /* no other calls so this is request to hold incoming call */
    else if(priority)
    {
        MP_DEBUG(("MP: AccWaitHoldAct - no calls chld=2 = %d \n",priority)) ;		
        /* perform CHLD = 2 command to put incoming call on hold */
        HfpCallHoldActionRequest(priority, hfp_chld_hold_active_accept_other, 0);                               
    }  
    
    /* redo audio routing */
    audioUpdateAudioRouting();
}



/****************************************************************************
NAME    
    mpSwitchToAG
    
DESCRIPTION
   helper function to switch between AGs where a call exists on both AGs
   
RETURNS

*/
void mpSwitchToAG(hfp_link_priority priority)
{
    hfp_link_priority priority_other = (priority == hfp_primary_link) ? hfp_secondary_link : hfp_primary_link;   
    hfp_call_state CallState = hfp_call_state_idle;
    
    /* change sco priority to selected required AG */
    setScoPriorityFromHfpPriority(priority, sco_active_call);                    
    setScoPriorityFromHfpPriority(priority_other, sco_held_call);  
    /* check to see if the call is actually on hold and un-hold if applicable */
    if(HfpLinkGetCallState(priority, &CallState) == hfp_call_state_held_remaining)
        HfpCallHoldActionRequest(priority, hfp_chld_hold_active_accept_other, 0);                                          
}

/****************************************************************************
NAME    
    mpHandleCallWaitingInd
    
DESCRIPTION
   determine whether a call waiting indication notification needs to be played
   if the audio of the AG is not currently being routed
   
RETURNS

*/
void mpHandleCallWaitingInd(HFP_CALL_WAITING_IND_T * pInd)
{
    MP_DEBUG(("MP: mpHandleCallWaitingInd from AG%d, sco is from AG%d\n",pInd->priority,HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink()))) ;
    /* determine if the indication of a call waiting has been sent from the AG which
       has its audio currently routed, if not it will not be heard and will need to be
       generated locally */
    if(pInd->priority != HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink()))
    {
        /* generate event to play call waiting indication */
        MessageSend(&theSink.task,EventSysMultipointCallWaiting,0);        
    }    
}



/****************************************************************************
NAME    
    MpTwcAnswerCall
    
DESCRIPTION
   Called when the device is in a twc operating state to answer a call on a 
   second AG. If the device is currently routing audio, put this on hold ready to
   route the call once answered.
   
RETURNS

*/
void MpTwcAnswerCall(void)
{
    hfp_link_priority priority = hfp_invalid_link;

    
    /* get profile if AG with incoming call */
    priority = HfpLinkPriorityFromCallState(hfp_call_state_incoming);
    /* if no incoming calls check for presence of two active calls and
       swap between those two calls */
    if(priority)
    {
        /* determine whether device has audio routed, if so hold this audio */
        if(sinkAudioIsVoiceRouted())
        {
            /* change audio sco priority of current active call, when incoming call is 
               answered audio will get routed automatically as it will have a higher priority */
            setScoPriorityFromHfpPriority(HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink()), sco_held_call);
        }
        /* accept waiting call */
        HfpCallAnswerRequest(priority, TRUE);   
        sinkHfpDataSetProfileLocalCallActionStatus(TRUE, PROFILE_INDEX(priority));
        ToneTerminate();

        /* if AG is muted, unmute it to be able to speak to caller */
        if(sinkHfpDataGetAudioGMuted(PROFILE_INDEX(priority)))           
        {
            /* unmute new active call */            
            VolumeSetHfpMicrophoneGainCheckMute(priority, MICROPHONE_MUTE_OFF);
        }                                             

    }
}

/****************************************************************************
NAME    
    MpDropCall
    
DESCRIPTION
   performs a call drop on passed in AG depending upon call status, if the AG
   has a single call a CHUP command will be sent, if the AG has multiple calls
   the CHLD = 1, release active accept waiting will be sent 
   
RETURNS

*/
void MpDropCall(hfp_link_priority priority)
{
    hfp_call_state CallState;
    
    MP_DEBUG(("MP: Drop Call - "));

    /* get call state AG to end call on */
    if(HfpLinkGetCallState(priority, &CallState))
    {
        switch(CallState)
        {
            case hfp_call_state_active:
            case hfp_call_state_outgoing:
            case hfp_call_state_twc_outgoing:
                MP_DEBUG(("Terminate\n"));
                HfpCallTerminateRequest(priority);
            break;

            case hfp_call_state_held_active:
            case hfp_call_state_multiparty:
                MP_DEBUG(("Release Active Accept Other\n"));
                HfpCallHoldActionRequest(priority, hfp_chld_release_active_accept_other, 0);
            break;

            case hfp_call_state_held_remaining:
                MP_DEBUG(("Release Held\n"));
                HfpCallHoldActionRequest(priority, hfp_chld_release_held_reject_waiting, 0);
            break;

            default:
                MP_DEBUG(("Ignored\n"));
            break;
        }
    }  
}

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
void MpHandleConferenceCall(bool create)
{
    hfp_link_priority priority = hfp_invalid_link;
    hfp_call_state CallState;
                
    /* determine if the AG which the user is currently listening to is capable of
       performing the conference call function */
    if(sinkAudioIsVoiceRouted())
    {
        /* obtain AG currently listening to */
        priority = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());
        
        /* get call state AG to end call on */
        if(HfpLinkGetCallState(priority, &CallState))
        {
            /* if trying to create a conference call and AG has two calls */
            if((create)&&(CallState == hfp_call_state_held_active))
                HfpCallHoldActionRequest(priority, hfp_chld_add_held_to_multiparty, 0);
            /* if trying to disconnect from a conference call and AG has a conference call */
            else if((!create)&&((CallState == hfp_call_state_multiparty) ||
                                           (CallState == hfp_call_state_held_active)))
                HfpCallHoldActionRequest(priority, hfp_chld_join_calls_and_hang_up, 0);
        }
    }
}

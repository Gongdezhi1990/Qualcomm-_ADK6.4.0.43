/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    state machine helper functions used for state changes etc - provide single 
    state change points etc for the sink device application
*/
#include "sink_statemanager.h"
#include "sink_led_manager.h"
#include "sink_buttonmanager.h"
#include "sink_pio.h"
#include "sink_audio.h"
#include "sink_scan.h"
#include "sink_slc.h"
#include "sink_inquiry.h"
#include "sink_devicemanager.h"
#include "sink_powermanager.h"
#include "sink_speech_recognition.h" 
#include "psu.h"
#include "sink_audio_routing.h"
#include "sink_auth.h"
#include "sink_ba.h"
#include "sink_dormant.h"
#include "sink_main_task.h"
#include "sink_wired.h"
#include "sink_bredr.h"
#include "sink_private_data.h"
#include "sink_callmanager.h"
#include "sink_ble.h"
#include "sink_fm.h"
#ifdef ENABLE_FAST_PAIR
#include "sink_fast_pair.h"
#endif
#include "sink_hfp_data.h"

#include "sink_display.h"

#ifdef ENABLE_AVRCP
#include "sink_avrcp.h"    
#endif

#ifdef ENABLE_SUBWOOFER
#include "sink_swat.h"
#include <bdaddr.h>
#endif

#include <stdlib.h>
#include <boot.h>
#include <ahi.h>

#ifdef DEBUG_STATES
#define SM_DEBUG(x) DEBUG(x)


/*
    NB: When adding a new extended state, it must be put at
        the end of the array
        + Should follow the same order as in sink_extended_state_t
        (see sink_extendedstates.h)
        + needs consistency with "values" in global XML config file
*/
const char* const gHSExtendedStateStrings [ SINK_NUM_EXTENDED_STATES ] = 
{ 
                                                "Limbo",
                                                "Connectable",
                                                "ConnDiscoverableToSource",  /* Connectable/Discoverable Ext states */
                                                "ConnDiscoverableToPeer",
                                                "ConnectedToSource",  /* Connected Ext states */
                                                "ConnectedToPeer",
                                                "ConnectedToSourceAndPeer",
                                                "OutgoingCallEstablish",
                                                "IncomingCallEstablish",
                                                "ActiveCallSCO",
                                                "TestMode",
                                                "ThreeWayCallWaiting",
                                                "ThreeWayCallOnHold",
                                                "ThreeWayMulticall",
                                                "IncomingCallOnHold",
                                                "ActiveCallNoSCO",   
                                                "deviceA2DPStreaming",
                                                "LowBattery",
                                                "Config Mode",
                                                "Default Pattern / No Pattern found"
                                                /* ADD EXTENDED STATE HERE */
} ;
                              
#else
#define SM_DEBUG(x) 
#endif

typedef enum
{
    GOODBYE_DORMANT,
    GOODBYE_POWER_OFF
} sinkGoodbyeState;

/****************************************************************************
VARIABLES
*/

    /*the device state variable - accessed only from below fns*/
static sinkState gTheSinkState ;
    /* The device extended state variable - accessed only from below fns */
static sink_extended_state_t pTheSinkExtendedState ; /* Keeps track of the current extended state */
static sinkLimboReason gTheSinkLimboReason ;


/****************************************************************************
FUNCTIONS
*/

static void stateManagerSetState ( sinkState pNewState ) ;
static void stateManagerSetExtendedState ( sink_extended_state_t pNewExtendedState ) ;
static void stateManagerResetPIOs ( void ) ;
static sink_extended_state_t stateManagerFindExtendedStateFromState( sinkState pState );
static bool stateManagerIsStateSameExtendedState( sinkState pState1, sink_extended_state_t* pNewExtendedState );
static sink_extended_state_t getConnDiscovExtendedStates(void);
static sink_extended_state_t getConnectedExtendedStates(void);
static bool checkSecondSlc(void);

#define isPowerDownPioConfigured()  PioIsPioConfigured(PIO_POWER_ON)
#define isMutePioConfigured()       PioIsPioConfigured(PIO_AMP_MUTE)

/****************************************************************************
NAME
    stateManagerSetState

DESCRIPTION
    helper function to Set the current device state
    provides a single state change point and passes the information
    on to the managers requiring state based responses
    
RETURNS
    void

*/
static void stateManagerSetState ( sinkState pNewState )
{
    SM_DEBUG(("SM:[%s]->[%s][%d]\n",gHSExtendedStateStrings[stateManagerGetState()] , gHSExtendedStateStrings[pNewState] , pNewState ));

    if ( pNewState < SINK_NUM_STATES )
    {
        /* Needed to set the new extended state if it has changed */    
        sink_extended_state_t newExtendedState;

        /* Indicate the machine state immediately to the AHI 
            and display whenever the state is updated*/
        if (pNewState != gTheSinkState)
        {
            AhiTestReportStateMachineState(1, pNewState);
            
            /* this should be called before the state is updated */
            displayUpdateAppState(pNewState);
        }
        else
        {
            /*State already indicated, need not resend the same*/
        }

        if ( !stateManagerIsStateSameExtendedState(pNewState, &newExtendedState) )
        {
            /* Indicate the LED pattern based on extended state */
            stateManagerSetExtendedState(newExtendedState);
            /* send message to reduce stack usage */          
            MessageSend(&theSink.task, EventSysLEDIndicateState, 0);
        }

        gTheSinkState = pNewState ;

#ifdef ENABLE_FAST_PAIR
        if(gTheSinkState != deviceConnDiscoverable)
            sinkFastPairSetDiscoverabilityStatus(no_discoverability);
#endif
    }
    else
    {
        SM_DEBUG(("SM: ? [%s] [%x]\n",gHSExtendedStateStrings[ pNewState] , pNewState)) ;
    }

}


/****************************************************************************
NAME	
	stateManagerGetState

DESCRIPTION
	helper function to get the current device state

RETURNS
	the Devie State information
    
*/
sinkState stateManagerGetState ( void )
{
    return gTheSinkState ;
}

/****************************************************************************
NAME
    stateManagerSetExtendedState

DESCRIPTION
    helper function to Set the current device extended state

RETURNS

*/
static void stateManagerSetExtendedState (sink_extended_state_t pNewExtendedState)
{
    pTheSinkExtendedState = pNewExtendedState;
}

/****************************************************************************
NAME
    stateManagerGetExtendedState

DESCRIPTION
    helper function to get the current device extended state

RETURNS
    the Devie Extended State information

*/
sink_extended_state_t stateManagerGetExtendedState ( void )
{
    return pTheSinkExtendedState;
}

/****************************************************************************
NAME
    updateExtendedState

DESCRIPTION
    Updating the extended state and indicate the LED state.

RETURNS
    void
*/
void updateExtendedState ( sinkState pNewState )
{
    if ((pNewState == deviceConnected) || (pNewState == deviceConnDiscoverable))
    {
        /* Needed to set the new extended state if it has changed */    
        sink_extended_state_t newExtendedState;
        /* Extended states are verified instead of application states because although this can be the same 
         * Application state, the extended state might be different and then needs to be displayed 
         */
        if ( !stateManagerIsStateSameExtendedState(pNewState, &newExtendedState) )
        {
            /* LED state to indicate */
            stateManagerSetExtendedState(newExtendedState);
            /* send message to reduce stack usage */          
            MessageSend(&theSink.task, EventSysLEDIndicateState, 0);
        }
    }
}

/****************************************************************************
NAME
    checkSecondSlc

DESCRIPTION
    This function to be called to check the second incoming connection (SLC) 
    comes from either AG/Peer to update the extended state as both AG and Peer 
    connected(i.e. sink_ext_state_connectedToSourceAndPeer)

RETURNS
    TRUE if any one of the AG/Peer already connected. Else returns FALSE. 
*/
static bool checkSecondSlc(void)
{
    return ((deviceManagerIsOnlyAgSourceConnected() && (stateManagerGetExtendedState() 
                                                        == sink_ext_state_connectedToAG)) 
        || (deviceManagerIsPeerConnected() && (stateManagerGetExtendedState() 
                                               == sink_ext_state_connectedToPeer)));
}

/****************************************************************************
NAME
    getConnDicovExtendedStates

DESCRIPTION
    Get the extended states of connectable/discoverable device state (sinkState)

RETURNS
    The corresponding extended state of connectable/discoverable device state
    
*/
static sink_extended_state_t getConnDiscovExtendedStates(void)
{

    sink_extended_state_t conndisc_ext_state = sink_ext_state_Default;

    if (sinkInquiryIsInqSessionNormal())
    {
        conndisc_ext_state = sink_ext_state_connDiscoverableToSource;
    }
    else if (sinkInquiryIsInqSessionPeer())
    { /* For clarity, though no need to check */
        conndisc_ext_state = sink_ext_state_connDiscoverableToPeer;
    }

    return conndisc_ext_state;

}

/****************************************************************************
NAME
    getConnectedExtendedStates

DESCRIPTION
    Get the extended states of connected device state (sinkState)

RETURNS
    The corresponding extended state of connected device state
    
*/
static sink_extended_state_t getConnectedExtendedStates(void)
{

    sink_extended_state_t connected_ext_state = sink_ext_state_Default;

    if (deviceManagerIsBothAgSourceAndPeerConnected())
    {
        connected_ext_state = sink_ext_state_connectedToSourceAndPeer;
    }
    else if (deviceManagerIsOnlyAgSourceConnected())
    {
        connected_ext_state = sink_ext_state_connectedToAG;
    }
    else if (deviceManagerIsPeerConnected())
    {
        connected_ext_state = sink_ext_state_connectedToPeer;
    }

    return connected_ext_state;

}

/****************************************************************************
NAME
    stateManagerFindExtendedStateFromState

DESCRIPTION
    Find the extended state corresponding to the sinkState pState input

RETURNS
    The corresponding extended state
    
*/
static sink_extended_state_t stateManagerFindExtendedStateFromState( sinkState pState )
{
#ifdef DEBUG_STATES
    const char* debug_state = gHSExtendedStateStrings[pState];
#endif /* DEBUG_STATES */

    sink_extended_state_t extended_state;

    SM_DEBUG(("SM: Getting extended state from state %s IS[%d]\n",debug_state,pState));

    switch(pState)
    {
        case(deviceLimbo):
            return sink_ext_state_limbo;
        case(deviceConnectable):
            return sink_ext_state_connectable;
        case(deviceConnDiscoverable):
            extended_state = getConnDiscovExtendedStates();
            break;
        case(deviceConnected):
            extended_state = getConnectedExtendedStates();
            break;
        case(deviceOutgoingCallEstablish):
            return sink_ext_state_outgoingCallEstablish;
        case(deviceIncomingCallEstablish):
            return sink_ext_state_incomingCallEstablish;
        case(deviceActiveCallSCO):
            return sink_ext_state_activeCallSCO;
        case(deviceTestMode):
            return sink_ext_state_testMode;
        case(deviceThreeWayCallWaiting):
            return sink_ext_state_threeWayCallWaiting;
        case(deviceThreeWayCallOnHold):
            return sink_ext_state_threeWayCallOnHold;
        case(deviceThreeWayMulticall):
            return sink_ext_state_threeWayMulticall;
        case(deviceIncomingCallOnHold): 
            return sink_ext_state_incomingCallOnHold; 
        case(deviceActiveCallNoSCO):
            return sink_ext_state_activeCallNoSCO;
        case(deviceA2DPStreaming): 
            return sink_ext_state_a2dpStreaming; 
        case(deviceLowBattery):
            return sink_ext_state_lowBattery;
        case(deviceInConfigMode):
            return sink_ext_state_inConfigMode;
        default:
            return sink_ext_state_Default;
    }
    
    SM_DEBUG(("SM: No Extended State found for %s\n",debug_state));

    return extended_state;
}

/****************************************************************************
NAME
    stateManagerIsStateSameExtendedState

DESCRIPTION
    Check if the input state is a new extended state to display,
    i.e. different from the one currently displayed

RETURNS
    TRUE if the corresponding extended state is already displayed
    FALSE otherwise
    The potentially new state is stored in the pNewExtendedState variable
*/
static bool stateManagerIsStateSameExtendedState( sinkState pState1, sink_extended_state_t* pNewExtendedState ) 
{
    if( (*pNewExtendedState = stateManagerFindExtendedStateFromState(pState1)) == pTheSinkExtendedState)
    {
        SM_DEBUG(("SM : Already displaying the extended state %s, do nothing IS[%d]\n",gHSExtendedStateStrings[pTheSinkExtendedState],pTheSinkExtendedState));
        return TRUE;
    }
    SM_DEBUG(("SM : Extended State %s needs to be displayed IS[%d]\n",gHSExtendedStateStrings[*pNewExtendedState],*pNewExtendedState));
    return FALSE;
}

/****************************************************************************
NAME
    stateManagerEnterConnectableState

DESCRIPTION
    single point of entry for the connectable state - enters discoverable state 
    if configured to do so

RETURNS
    void
    
*/
void stateManagerEnterConnectableState ( bool req_disc )
{
    sinkState lOldState = stateManagerGetState() ;
    
    /* Don't go connectable if we're still pairing */
    if(!req_disc && sinkInquiryCheckInqActionRssi())
        return;

    if ( stateManagerIsConnected() && req_disc )
    {       /*then we have an SLC active*/
       sinkDisconnectAllSlc();
    }
        /*disable the ring PIOs if enabled*/
    stateManagerResetPIOs();
        /* Make the device connectable */
    sinkEnableConnectable();
    stateManagerSetState ( deviceConnectable ) ;

        /* if remain discoverable at all times feature is enabled then make the device 
           discoverable in the first place */
    if (sinkDataIsDiscoverableAtAllTimes())
    {
        /* Make the device discoverable */  
        sinkEnableDiscoverable();    
    }
    
        /*determine if we have got here after a DiscoverableTimeoutEvent*/
    if ( lOldState == deviceConnDiscoverable )
    {
        /*disable the discoverable mode*/
        if (!sinkDataIsDiscoverableAtAllTimes())
        {
            sinkDisableDiscoverable();
#ifdef ENABLE_FAST_PAIR
            SM_DEBUG(("SM : ConnectableDiscoverable to Connectable\n"));
            MessageSend( &theSink.task, EventSysExitDiscoverableMode, 0);
            sinkFastPairStopDiscoverabilityTimer();
#endif
        }
        MessageCancelAll ( &theSink.task , EventSysPairingFail ) ;        
    }
    else
    {
        uint16 lNumDevices = ConnectionTrustedDeviceListSize();
        
#ifdef ENABLE_SUBWOOFER
        /* deteremine if there is a subwoofer present in the PDL, this will need to be 
           discounted from the number of paired AG's */
        if (!BdaddrIsZero(sinkSwatGetBdaddr()))
        {
            /* remove subwoofer pairing from number of devices */
            if(lNumDevices)
                lNumDevices --;
        }
#endif        
        
        /*if we want to auto enter pairing mode*/
        if ( sinkDataAutoPairModeEnabled() )
        {
            sinkInquirySetInquirySession(inquiry_session_normal);
            stateManagerEnterConnDiscoverableState( req_disc );
        }  
        SM_DEBUG(("SM: Disco %X RSSI %X\n", sinkDataGetPDLLimitForDiscovery(), sinkDataGetPDLLimitforRSSIPairing()));

        
        /* check whether the RSSI pairing if PDL < feature is set and there are less paired devices than the
           level configured, if so start rssi pairing */
        if((sinkDataGetPDLLimitforRSSIPairing())&&( lNumDevices < sinkDataGetPDLLimitforRSSIPairing() ))
        {
            SM_DEBUG(("SM: NumD [%d] <= PairD [%d]\n" , lNumDevices , sinkDataGetPDLLimitforRSSIPairing()))

            /* send event to enter pairing mode, that event can be used to play a tone if required */
            MessageSend(&theSink.task, EventSysEnterPairingEmptyPDL, 0);
            MessageSend(&theSink.task, EventUsrRssiPair, 0);
        }           
        /* otherwise if any of the other action on power on features are enabled... */
        else if (sinkDataGetPDLLimitForDiscovery())
        {
            SM_DEBUG(("SM: Num Devs %d\n",lNumDevices));
            /* Check if we want to go discoverable */
            if ( lNumDevices < sinkDataGetPDLLimitForDiscovery() )
            {
                SM_DEBUG(("SM: NumD [%d] <= DiscoD [%d]\n" , lNumDevices , sinkDataGetPDLLimitForDiscovery()))
                /* send event to enter pairing mode, that event can be used to play a tone if required */
                MessageSend(&theSink.task, EventSysEnterPairingEmptyPDL, 0);
            }
#ifdef ENABLE_SUBWOOFER
            /* If there is only one paired device and it is the subwoofer, start pairing mode */
            else if ( (ConnectionTrustedDeviceListSize() == 1) && (!BdaddrIsZero(sinkSwatGetBdaddr())) )
            {
                /* Check the subwoofer exists in the paired device list */
                sinkSwatSetCheckPairing(TRUE);
                ConnectionSmGetAuthDevice(&theSink.task, sinkSwatGetBdaddr());
            }
#endif
        }
    }
}

/****************************************************************************
NAME
    stateManagerEnterConnDiscoverableState

DESCRIPTION
    single point of entry for the connectable / discoverable state 
    uses timeout if configured
RETURNS
    void

*/
void stateManagerEnterConnDiscoverableState ( bool req_disc )
{
    /* cancel any pending connection attempts when entering pairing mode */
    MessageCancelAll(&theSink.task, EventSysContinueSlcConnectRequest);

    if(sinkDataCheckDoNotDisconDuringLinkLoss() && HfpLinkLoss())
    {
        /*if we are in link loss and do not want to go discoverable during link loss then ignore*/                    
    }
    else
    {    
        /* if in connected state disconnect any still attached devices */
        if ( stateManagerIsConnected() && req_disc )
        {
            /* do we have an SLC active? */
            sinkDisconnectAllSlc();
            
            /* or an a2dp connection active? */
            if (sinkInquiryIsInqSessionPeer())
            {   /* Entering Peer session initiation, ensure any Peer is also disconnected */
                disconnectAllA2dpAvrcp(TRUE);
            }
            else
            {   /* Non Peer discoverable mode - leave any Peer connected */
                disconnectAllA2dpAvrcp(FALSE);
            }
        }

        /* Make the device connectable */
        sinkEnableConnectable();

        /* Make the device discoverable */
        sinkEnableDiscoverable();

        /* If there is a timeout - send a user message*/
        MessageCancelAll(&theSink.task, EventSysPairingFail);
        if ( sinkDataGetPairModePDLTimeout() != 0 )
        {
            /* if there are no entries in the PDL, then use the second
               pairing timer */
            uint16 lNumDevices = ConnectionTrustedDeviceListSize();
            if( lNumDevices != 0)
            {   /* paired devices in list, use original timer if it exists */
                if( sinkDataGetPairModeTimeout() != 0 )
                {
                    SM_DEBUG(("SM : Pair [%x]\n" ,  sinkDataGetPairModeTimeout() )) ;
                    MessageSendLater ( &theSink.task , EventSysPairingFail , 0 , D_SEC(sinkDataGetPairModeTimeout()) ) ;
                }
            }
            else
            {   /* no paired devices in list, use secondary timer */
                SM_DEBUG(("SM : Pair (no PDL) [%x]\n" ,sinkDataGetPairModePDLTimeout() )) ;
                MessageSendLater ( &theSink.task , EventSysPairingFail , 0 , D_SEC(sinkDataGetPairModePDLTimeout()) ) ;
            }
        }
        else if ( sinkDataGetPairModeTimeout() != 0 )
        {
            SM_DEBUG(("SM : Pair [%x]\n" , sinkDataGetPairModeTimeout() )) ;
            
            MessageSendLater ( &theSink.task , EventSysPairingFail , 0 , D_SEC(sinkDataGetPairModeTimeout()) ) ;
        }
        else
        {
            SM_DEBUG(("SM : Pair Indefinetely\n")) ;
        }
        /* Disable the ring PIOs if enabled*/
        stateManagerResetPIOs();

        /* The device is now in the connectable/discoverable state */
        stateManagerSetState(deviceConnDiscoverable);
        sinkCancelAndIfDelaySendLater(EventSysPairingReminder, D_SEC(sinkDataGetPairingReminderInterval()));
    }
}

/****************************************************************************
NAME
    stateManagerEnterConnectedState

DESCRIPTION
    single point of entry for the connected state - disables disco / connectable modes
RETURNS
    void

*/
void stateManagerEnterConnectedState ( void )
{
#ifdef ENABLE_FAST_PAIR
    sinkState lOldState = stateManagerGetState();
#endif

    if((stateManagerGetState () != deviceConnected) && (!sinkInquiryCheckInqActionRssi()))
    {
            /*make sure we are now neither connectable or discoverable*/
        SM_DEBUG(("SM:Remain in Disco Mode [%c]\n", (sinkDataIsDiscoverableAtAllTimes()?'T':'F') )) ;
        /* If we are in "broadcast audio" mode, then once entering connected state we shouldn't be 
            discoverable, as we don't support more than one connection */
        if (!sinkDataIsDiscoverableAtAllTimes() || sinkBroadcastAudioIsActive())
        {
#ifdef ENABLE_SUBWOOFER
            if(SwatGetSignallingSink(sinkSwatGetDevId()) || sinkBroadcastAudioIsActive())
            {
               sinkDisableDiscoverable();
            }
#else
            sinkDisableDiscoverable();
            /* Before entering connected state, State manager will always be in connectable state, so
                old state will never be connectable discoverable, That's the reason we will never hit this condition.
                We have already informed BLE GAP SM while entering from ConnDiscoverable to Connectable state.
                But as we are calling sinkDisableDiscoverable, that's why good to have this condition.*/
#ifdef ENABLE_FAST_PAIR                 
            if(lOldState == deviceConnDiscoverable)
            {
                SM_DEBUG(("SM : Will never hit this condition.\n"));
                MessageSend( &theSink.task, EventSysExitDiscoverableMode, 0);
            }
#endif
#endif
        }

        /* For multipoint connections need to remain connectable after 1 device is connected.
            However in "broadcast audio" mode, when once we are connected, then we shall not allow any other
            paired device to connect, basically over-ride multipoint configuration */
        if(!sinkDataIsMultipointEnabled() || sinkBroadcastAudioIsActive())
        {
#ifdef ENABLE_SUBWOOFER     
            if(SwatGetSignallingSink(sinkSwatGetDevId()) || sinkBroadcastAudioIsActive())
            {
               sinkDisableConnectable();
            }
#else
            sinkDisableConnectable();
#endif
        }
        else
        {
            /* if both profiles are now connected disable connectable mode */
            if(deviceManagerNumConnectedDevs() == MAX_MULTIPOINT_CONNECTIONS)
            {
                /* two devices connected */
                sinkDisableConnectable();
            }
            else
            {
                /* still able to connect another devices */
                sinkEnableConnectable();
                /* remain connectable for a further 'x' seconds to allow a second 
                   AG to be connected if non-zero, otherwise stay connecatable forever */

                /* cancel any currently running timers that would disable connectable mode */
                sinkCancelAndIfDelaySendLater(EventSysConnectableTimeout, D_SEC(sinkDataGetConnectableTimeout()));
            }
        }

        switch ( stateManagerGetState() )
        {    
            case deviceIncomingCallEstablish:
                if (sinkHfpDataGetMissedCallIndicateTime() != 0 && 
                    sinkHfpDataGetMissedCallIndicateAttempsTime() != 0)
                {
                    sinkHfpDataSetMissedCallIndicated(sinkHfpDataGetMissedCallIndicateAttempsTime());
                    
                    MessageSend   (&theSink.task , EventSysMissedCall , 0 ) ; 
                    MessageSend ( &theSink.task , EventSysSpeechRecognitionStop , 0 ) ;
                }
            case deviceActiveCallSCO:
            case deviceActiveCallNoSCO:
            case deviceThreeWayCallWaiting:
            case deviceThreeWayCallOnHold:
            case deviceThreeWayMulticall:
            case deviceOutgoingCallEstablish:
                    /*then we have just ended a call*/
                MessageSend ( &theSink.task , EventSysEndOfCall , 0 ) ;
            break ;
            default:
            break ;
        }

        MessageCancelAll ( &theSink.task , EventSysPairingFail ) ;

            /*disable the ring PIOs if enabled*/
        stateManagerResetPIOs();

        /* when returning to connected state, check for the prescence of any A2DP instances
           if found enter the appropriate state */
        if((getA2dpStatusFlag(CONNECTED, a2dp_primary))||(getA2dpStatusFlag(CONNECTED, a2dp_secondary)))
        {
            SM_DEBUG(("SM:A2dp connected\n")) ;
            /* are there any A2DP instances streaming audio? */
            if((A2dpMediaGetState(getA2dpLinkDataDeviceId(a2dp_primary), getA2dpLinkDataStreamId(a2dp_primary)) == a2dp_stream_streaming)||
               (A2dpMediaGetState(getA2dpLinkDataDeviceId(a2dp_secondary), getA2dpLinkDataStreamId(a2dp_secondary)) == a2dp_stream_streaming))
            {
                SM_DEBUG(("SM:A2dp connected - still streaming\n")) ;
                stateManagerSetState( deviceA2DPStreaming );
            }
			/* no A2DP streaming, check for the presence of any call  */
			 else  if ((hfp_call_state_active == sinkCallManagerGetHfpCallState(hfp_primary_link))
					 || (hfp_call_state_active == sinkCallManagerGetHfpCallState(hfp_secondary_link)))
			{
				SM_DEBUG(("SM:Resume Active call state\n")) ;
				stateManagerSetState( deviceActiveCallSCO );
			}
			/* Go to connected state */
            else
			{
                stateManagerSetState( deviceConnected );
			}
		}
		/*HFP profile is only supported  */
		else  if ((hfp_call_state_active == sinkCallManagerGetHfpCallState(hfp_primary_link))
			|| (hfp_call_state_active == sinkCallManagerGetHfpCallState(hfp_secondary_link)))
		{
			SM_DEBUG(("SM:A2DP profile not connected ,resume Active call state\n")) ;
			stateManagerSetState( deviceActiveCallSCO );
		}		
        /* Go to connected state */
        else
		{
            stateManagerSetState( deviceConnected );
		}
    }
    else if (checkSecondSlc())
    {
        /* Updating the both AG and Peer connected extended state when 
           the device state is in deviceConnected state */
        updateExtendedState( deviceConnected );
    }
	/*Checks to see if an inquiry is still in progress, if yes set to discoverable state*/
	else if (inquiry_searching == sinkInquiryGetInquiryState())
	{
		SM_DEBUG(("SM:Resume discoverable state\n")) ;
		stateManagerSetState( deviceConnDiscoverable );
	}
}
/****************************************************************************
NAME
    stateManagerEnterIncomingCallEstablishState

DESCRIPTION
    single point of entry for the incoming call establish state
RETURNS
    void

*/
void stateManagerEnterIncomingCallEstablishState ( void )
{
   
    stateManagerSetState( deviceIncomingCallEstablish );

    /*if we enter this state directly*/
    MessageCancelAll ( &theSink.task , EventSysPairingFail );
    PioDrivePio(sinkHfpDataGetIncomingRingPIO(), TRUE);
#ifdef ENABLE_SPEECH_RECOGNITION
    /* speech recognition not support on HSP profile AG's */
    if ((speechRecognitionIsEnabled()) && (!HfpPriorityIsHsp(HfpLinkPriorityFromCallState(hfp_call_state_incoming))))
        MessageSend ( &theSink.task , EventSysSpeechRecognitionStart , 0 );
#endif    
}

/****************************************************************************
NAME
    stateManagerEnterOutgoingCallEstablishState

DESCRIPTION
    single point of entry for the outgoing call establish state
RETURNS
    void

*/
void stateManagerEnterOutgoingCallEstablishState ( void )
{
    stateManagerSetState( deviceOutgoingCallEstablish );
    
    /*if we enter this state directly*/
    MessageCancelAll ( &theSink.task , EventSysPairingFail ) ;

    PioDrivePio(sinkHfpDataGetOutgoingRingPIO(), TRUE) ;
}
/****************************************************************************
NAME
    stateManagerEnterActiveCallState

DESCRIPTION
    single point of entry for the active call state
RETURNS
    void

*/
void stateManagerEnterActiveCallState ( void )   
{
    Sink sinkAG1,sinkAG2;

    if((stateManagerGetState() == deviceOutgoingCallEstablish) ||
    (stateManagerGetState() == deviceIncomingCallEstablish))
    {
        /* If a call is being answered then send call answered event */
        MessageSend ( &theSink.task , EventSysCallAnswered , 0 ) ;
        MessageSend ( &theSink.task , EventSysSpeechRecognitionStop , 0 ) ;
    }

    /* get any SCO sinks */
    HfpLinkGetAudioSink(hfp_primary_link, &sinkAG1);
    HfpLinkGetAudioSink(hfp_secondary_link, &sinkAG2);
    
    /* Check current SCO audio status */
    if(sinkAG1 || sinkAG2)
    {
        stateManagerSetState( deviceActiveCallSCO );
    }
    else
    {
        stateManagerSetState( deviceActiveCallNoSCO );
    }

    /*disable the ring PIOs if enabled*/
    PioDrivePio(sinkHfpDataGetIncomingRingPIO(), FALSE) ;
    PioDrivePio(sinkHfpDataGetOutgoingRingPIO(), FALSE) ;
    PioDrivePio(sinkHfpDataGetCallActivePIO(), TRUE) ;

    /*if we enter this state directly*/
    MessageCancelAll ( &theSink.task , EventSysPairingFail ) ;

}


/****************************************************************************
NAME
    stateManagerEnterA2dpStreamingState

DESCRIPTION
    enter A2DP streaming state if not showing any active call states
RETURNS
    void

*/
void stateManagerEnterA2dpStreamingState(void)
{
    /* only allow change to A2DP connected state if not currently showing 
       any active call states */
    if(stateManagerGetState() == deviceConnected) 
    {
        stateManagerSetState(  deviceA2DPStreaming );
    }
}


/****************************************************************************
NAME
    stateManagerPowerOn

DESCRIPTION
    Power on the deviece by latching on the power regs
RETURNS
    void

*/
void stateManagerPowerOn( void ) 
{
    SM_DEBUG(("--hello--\nSM : PowerOn\n"));

    /* Check for DUT mode enable */
    {
        /* Reset caller ID flag */
        sinkHfpDataSetRepeatCallerIDFlag(TRUE);

        /*cancel the event power on message if there was one*/
        MessageCancelAll ( &theSink.task , EventSysLimboTimeout ) ;
        MessageCancelAll(&theSink.task, EventSysContinueSlcConnectRequest);

        PioSetPowerPin ( TRUE ) ;

        /* Turn on AMP pio drive */
        stateManagerAmpPowerControl(POWER_UP);

        /* An NFC wakeup could have put the application into connectable\discoverable mode */
        if (stateManagerGetState() != deviceConnDiscoverable)
        {
            stateManagerEnterConnectableState( TRUE );
        
            if(sinkDataGetPDLLimitforRSSIPairing() || sinkDataCheckAutoReconnectPowerOn() || sinkDataGetPanicReconnect())
            {
                uint16 lNumDevices = ConnectionTrustedDeviceListSize();
        
                /* Check if we want to start RSSI pairing */
                if( lNumDevices < sinkDataGetPDLLimitforRSSIPairing() )
                {
                    SM_DEBUG(("SM: NumD [%d] <= PairD [%d]\n" , lNumDevices , sinkDataGetPDLLimitforRSSIPairing()))

                    /* send event to enter pairing mode, that event can be used to play a tone if required */
                    MessageSend(&theSink.task, EventSysEnterPairingEmptyPDL, 0);
                    MessageSend(&theSink.task, EventUsrRssiPair, 0);
                }
                else if ((sinkDataCheckAutoReconnectPowerOn())||sinkDataGetPanicReconnect())
                {
                    sinkEvents_t event = EventUsrEstablishSLC;
#ifdef ENABLE_AVRCP
                    if(avrcpAvrcpIsEnabled())
                    {
                        sinkAvrcpCheckManualConnectReset(NULL);
                    }
#endif
                    SM_DEBUG(("SM: Auto Reconnect\n")) ;
                    if(sinkDataGetPanicReconnect())
                    {
                        event = EventSysEstablishSLCOnPanic;
                    }

                    /*  Queue the event with a delay for the A2DP library to be ready  */
                    sinkCancelAndSendLater(event, 100);
                }
            }
        }
    }
}

/****************************************************************************
NAME
    stateManagerIsShuttingDown

DESCRIPTION
    Helper method to see if we are shutting down or not
*/
bool stateManagerIsShuttingDown(void)
{
    return ((gTheSinkState == deviceLimbo) &&
            (gTheSinkLimboReason == limboShutDown));
}

/****************************************************************************
NAME
    stateManagerIsWakingUpFromDormant

DESCRIPTION
    Helper method to see if we are waking up from dormant or not
*/
bool stateManagerIsWakingUp(void)
{
    return ((gTheSinkState == deviceLimbo) &&
            (gTheSinkLimboReason == limboWakeUp));
}

/****************************************************************************
NAME	
	stateManagerIsConnected

DESCRIPTION
    Helper method to see if we are connected or not   
*/
bool stateManagerIsConnected ( void )
{
    bool lIsConnected = FALSE ;
    
    switch (stateManagerGetState() )
    {
        case deviceLimbo:
        case deviceConnectable:
        case deviceConnDiscoverable:
        case deviceTestMode:
            lIsConnected = FALSE ;    
        break ;
        
        default:
            lIsConnected = TRUE ;
        break ;
    }
    return lIsConnected ;
}

/****************************************************************************
NAME
    stateManagerEnterLimboState

DESCRIPTION
    Method to provide a single point of entry to the limbo /poweringOn state

RETURNS
    void
*/
void stateManagerEnterLimboState ( sinkLimboReason reason )
{
#ifdef ENABLE_FAST_PAIR
    sinkState lOldState = stateManagerGetState();
#endif
    SM_DEBUG(("SM: Enter Limbo State[%d]\n" , sinkDataGetLimboTimeout())); 
    
    /*set a timeout so that we will turn off eventually anyway*/
    sinkCancelAndSendLater(EventSysLimboTimeout, D_SEC(sinkDataGetLimboTimeout())) ;

    stateManagerSetState( deviceLimbo );
    gTheSinkLimboReason = reason;

    /*Make sure panic reconnect flag is reset*/
    sinkDataSetPanicReconnect(FALSE);

    if((isPowerDownPioConfigured() || isMutePioConfigured()) && !SinkWiredPlayUsbAndWiredInLimbo())
    {
        /* Invoke the system event based on a timeout value to turn off AMP eventually once the timer expires,
        if timeout interval is too short, audio prompts and tones may not be audible after this event is invoked */
        sinkCancelAndSendLater(EventSysAmpPowerDown, D_SEC(sinkDataAmpPowerDownTimeout()));
    }

    /*Cancel inquiry if in progress*/
    if (!sinkInquiryCheckInqActionRssiNone())
    {
        inquiryStop();
    }

    /* Disconnect any slc's and cancel any further connection attempts including link loss */
    MessageCancelAll(&theSink.task, EventSysStreamEstablish);
    MessageCancelAll(&theSink.task, EventSysContinueSlcConnectRequest);
    MessageCancelAll(&theSink.task, EventSysLinkLoss);
    sinkDisconnectAllSlc();     
    
    /* disconnect any a2dp signalling channels */
    disconnectAllA2dpAvrcp(TRUE);

    /* reset the pdl list indexes in preparation for next boot */
    slcReset();
    
    /*in limbo, the internal regs must be latched on (smps and LDO)*/
    PioSetPowerPin ( TRUE ) ;
    
    /*make sure we are neither connectable or discoverable*/
    sinkDisableDiscoverable();
#ifdef ENABLE_FAST_PAIR
    /* On Power off, If we are in connected or connectable state, then we are already exited from discoverable mode,
        If not, then check the old state and if it is connectable discoverable then inform BLE GAP SM of exiting
        discoverable mode.*/
    if(lOldState == deviceConnDiscoverable)
    {
        SM_DEBUG(("SM : ConnectableDiscoverable to Limbo state\n"));
        MessageSend( &theSink.task, EventSysExitDiscoverableMode, 0);
    }
#endif    
#ifdef ENABLE_SUBWOOFER
    /* allow subwoofer to connect */
    sinkEnableConnectable();    
#else 
    sinkDisableAllConnectable();
#endif

#if defined ENABLE_PEER
    /* Ensure Peer device is at top of PDL when we power back on */
    AuthInitPermanentPairing();
#endif

    /* reconnect usb if applicable */
    audioUpdateAudioRouting();
    
#ifdef ENABLE_SUBWOOFER   
    MessageSend(&theSink.task, EventSysSubwooferCloseMedia, 0);                               
#endif    
}

bool stateManagerIsReadyForAudio(void)
{
    if(isPowerDownPioConfigured() || isMutePioConfigured())
    {
        if(stateManagerGetState() == deviceLimbo)
        {
            return sinkAudioIsAmpReady();
        }
    }
    return TRUE;
}

/****************************************************************************
NAME
    stateManagerAmpPowerControl

DESCRIPTION
    Control the power supply for audio amplifier stage
    
RETURNS
    void

*/
void stateManagerAmpPowerControl(power_control_dir control)
{
    MessageCancelAll(&theSink.task, EventSysAmpPowerDown);
    MessageCancelAll(&theSink.task, EventSysAmpPowerUp);
    
    SM_DEBUG(("SM: amp: audioAmpReady=%u control=%u\n",
              sinkAudioIsAmpReady(),
              control));
    
    if(isMutePioConfigured())
    {
        if (control == POWER_DOWN)
        {
            if (sinkAudioIsAmpReady())
            {
            /*  Mute the amplifier and queue an event to power it down  */
                PioDrivePio(PIO_AMP_MUTE, TRUE);
                sinkCancelAndSendLater(EventSysAmpPowerDown, sinkAudioAmpPowerDownTime());
                sinkAudioSetAmpReady(FALSE);
            }
            else
            {
            /*  Power down the amplifier  */
                PioDrivePio(sinkAudioGetPowerOnPio(), FALSE);            
            }
        }
        else
        {
            if (sinkAudioIsAmpReady())
            {
            /*  Unmute the amplifier  */
                PioDrivePio(PIO_AMP_MUTE, FALSE);
            }
            else
            {
            /*  Power up the amplifier and queue an event to unmute it  */
                PioDrivePio(sinkAudioGetPowerOnPio(), TRUE);            
                sinkCancelAndSendLater(EventSysAmpPowerUp, sinkAudioAmpPowerUpTime());
                sinkAudioSetAmpReady(TRUE);
            }
        }
    }
    else
    {
    /*  No muting required; drive power PIO if configured and set amp ready flag  */
        if (control == POWER_DOWN)
        {
            PioDrivePio(sinkAudioGetPowerOnPio(), FALSE);
            sinkAudioSetAmpReady(FALSE);
        }
        else
        {
            PioDrivePio(sinkAudioGetPowerOnPio(), TRUE);
            sinkAudioSetAmpReady(TRUE);
        }
    }
}

/****************************************************************************
NAME
    stateManagerShutDownAudioTest

DESCRIPTION
   Test for audio features that may prevent power off.

RETURNS
   bool
*/
static bool stateManagerShutDownAudioTest(void)
{
    return (!sinkFmIsFmRxOn()
            && (   ((!analogAudioConnected()) && (!spdifAudioConnected()) && (!i2sAudioConnected()))
                || ( SinkWiredIsPowerOffOnWiredAudioConnected()
                    && (analogAudioConnected() || i2sAudioConnected() || spdifAudioConnected()))
#ifdef ENABLE_BATTERY_OPERATION
                || (!SinkWiredPlayUsbAndWiredInLimbo())
#endif
           ));
}

/****************************************************************************
NAME
    stateManagerShutDownVregEnableTest

DESCRIPTION
   Test for Vreg Enable features that may prevent power off.

RETURNS
   bool
*/
static bool stateManagerShutDownVregEnableTest(void)
{
    return ((!sinkDataIsPowerOffOnlyIfVRegEnlow())||(sinkDataIsPowerOffOnlyIfVRegEnlow() && !PsuGetVregEn()));
}

/****************************************************************************
NAME
    stateManagerOkayToShutDown

DESCRIPTION
   Tests all features that may prevent power off.

RETURNS
   bool
*/
static bool stateManagerOkayToShutDown(void)
{
    return ((gTheSinkLimboReason == limboShutDown) &&
            (!sinkDataIsDisableCompletePowerOff()) &&
            (stateManagerShutDownAudioTest()) &&
            (stateManagerShutDownVregEnableTest()));
}

/****************************************************************************
NAME
    stateManagerGoodbye

DESCRIPTION
    power off or go dormant
RETURNS

*/
static void stateManagerGoodbye(sinkGoodbyeState goodbye)
{
    SM_DEBUG(("SM : --goodbye--\n")) ;

    configManagerDefragIfRequired();
    /* Used as a power on indication if required */
    PioDrivePio(sinkAudioGetPowerOnPio(), FALSE) ;

    if (goodbye == GOODBYE_DORMANT)
    {
        SM_DEBUG(("SM : Dormant\n")) ;
        sinkDormantEnable(powerManagerIsChargerConnected(), powerManagerIsVthmCritical());
    }
    else
    {
        SM_DEBUG(("SM : Power Off\n")) ;
        PioSetPowerPin(FALSE);
    }
}

/****************************************************************************
NAME
    stateManagerAttemptToShutdown

DESCRIPTION
    If the Sink application is ready to shutdown then take appropriate shutdown
    actions.
    - if the device can be woken up by NFC then attempt to go dormant.
    - otherwise, power off if the charger is not connected.

    This function will not return if the device powers off or goes dormant.

RETURNS
    void
*/
static void stateManagerAttemptToShutdown(void)
{
    if (stateManagerOkayToShutDown())
    {
        if (sinkDormantIsWakeUpFromNfcConfigured())
        {
            if (powerManagerIsReadyForDormant())
            {
                SM_DEBUG(("SM: Charger is disabled, go dormant\n"));
                stateManagerGoodbye(GOODBYE_DORMANT);
            }
            else
            {
                SM_DEBUG(("SM: Not ready for dormant mode, stay awake\n"));
            }
        }
        else
        {
            if (powerManagerIsChargerConnected())
            {
                SM_DEBUG(("SM: Can't power off with charger connected, stay awake\n"));
            }
            else
            {
                SM_DEBUG(("SM: Charger not connected, so power off\n"));
                stateManagerGoodbye(GOODBYE_POWER_OFF);
            }
        }
    }
    else
    {
        SM_DEBUG(("SM: Not okay to shutdown yet, stay awake\n"));
    }
}

/****************************************************************************
NAME
    stateManagerHandleCriticalBatteryTemperature

DESCRIPTION
    If the battery temperature is critical then take safety actions to
    go into the lowest possible power state.
    - if the charger is not connected the device can be powered off.
    - if the charger is connected then the device cannot power off so attempt
      to go dormant instead.

    This function will not return if the device powers off or goes dormant.

RETURNS
    void
*/
static void stateManagerHandleCriticalBatteryTemperature(void)
{
    if (powerManagerIsVthmCritical())
    {
        SM_DEBUG(("SM: Battery temperature is critical, take safety actions\n"));
        if (powerManagerIsChargerConnected())
        {
            SM_DEBUG(("SM: Charger is connected, device cannot power off so prepare to go dormant\n"));
            if (powerManagerIsReadyForDormant())
            {
                SM_DEBUG(("SM: Charger is disabled, go dormant\n"));
                stateManagerGoodbye(GOODBYE_DORMANT);
            }
            else
            {
                SM_DEBUG(("SM: Dormant mode not ready yet or not supported, device cannot power off so disable charger instead\n"));
                powerManagerDisableCharger();
            }
        }
        else
        {
            SM_DEBUG(("SM: Charger is not connected, power off\n"));
            stateManagerGoodbye(GOODBYE_POWER_OFF);
        }
    }
    else
    {
        SM_DEBUG(("SM: Battery temperature is not critical, stay awake\n"));
    }
}

/****************************************************************************
NAME
    stateManagerHandleCriticalBatteryVoltage

DESCRIPTION
    If the battery voltage is critical and the charger is not connected
    then power the device off immediately to prevent further battery discharge.

    If the battery voltage is critical but the charger is connected then do not
    power the device off so that battery charging can continue.

    This function is called every time the limbo timeout expires. The battery
    voltage or charger connected state may change between each limbo timeout.

    This function will not return if the device powers off.

RETURNS
    void
*/
static void stateManagerHandleCriticalBatteryVoltage(void)
{
    if (powerManagerIsVbatCritical())
    {
        if (powerManagerIsChargerConnected())
        {
            SM_DEBUG(("SM: Battery voltage is critical, continue charging\n"));
        }
        else
        {
            SM_DEBUG(("SM: Battery voltage is critical, prevent further discharging\n"));
            stateManagerGoodbye(GOODBYE_POWER_OFF);
        }
    }
    else
    {
        SM_DEBUG(("SM: Battery voltage is not critical, stay awake\n"));
    }
}


/****************************************************************************
NAME
    stateManagerUpdateLimboState

DESCRIPTION
    Method to update the limbo state and power off when necessary
RETURNS 
    
*/
void stateManagerUpdateLimboState(void)
{
    SM_DEBUG(("SM: Limbo Update - %s\n",
              gTheSinkLimboReason == limboShutDown ? "Shut Down" : "Wake Up"));

    stateManagerHandleCriticalBatteryTemperature();
    stateManagerHandleCriticalBatteryVoltage();
    stateManagerAttemptToShutdown();

    /* Device was not able to shutdown at this point, schedule another check.
       For soundbar application (where battery is not used) the shutdown request
       will never succeed and the following timer will always restart.
       For battery powered devices the device may not always be able to enter
       the shutdown state. */
    sinkCancelAndSendLater(EventSysLimboTimeout, D_SEC(sinkDataGetLimboTimeout()));
}

/****************************************************************************
NAME
    stateManagerEnterTestModeState

DESCRIPTION
    method to provide a single point of entry to the test mode state
RETURNS
    
*/
void stateManagerEnterTestModeState ( void )
{
    stateManagerSetState( deviceTestMode );
}

/****************************************************************************
NAME
    stateManagerEnterCallWaitingState

DESCRIPTION
    method to provide a single point of entry to the 3 way call waiting state
RETURNS
    
*/
void stateManagerEnterThreeWayCallWaitingState ( void ) 
{
    stateManagerSetState( deviceThreeWayCallWaiting );

    /*if we enter this state directly*/
    MessageCancelAll ( &theSink.task , EventSysPairingFail ) ;
}


void stateManagerEnterThreeWayCallOnHoldState ( void ) 
{   
    stateManagerSetState( deviceThreeWayCallOnHold );

    /*if we enter this state directly*/
    MessageCancelAll ( &theSink.task , EventSysPairingFail ) ;
}

void stateManagerEnterThreeWayMulticallState ( void ) 
{
    stateManagerSetState( deviceThreeWayMulticall );

    /*if we enter this state directly*/
    MessageCancelAll ( &theSink.task , EventSysPairingFail ) ;
}


void stateManagerEnterIncomingCallOnHoldState ( void )
{
    switch ( stateManagerGetState() )
    {    
        case deviceIncomingCallEstablish:
            MessageSend ( &theSink.task , EventSysSpeechRecognitionStop , 0 ) ;
        break ;
        default:
        break ;
    }    
     
    stateManagerSetState( deviceIncomingCallOnHold );

    /*if we enter this state directly*/
    MessageCancelAll ( &theSink.task , EventSysPairingFail ) ;
}

static void stateManagerResetPIOs ( void )
{
    /*disable the ring PIOs if enabled*/
    PioDrivePio(sinkHfpDataGetIncomingRingPIO(), FALSE) ;
    PioDrivePio(sinkHfpDataGetOutgoingRingPIO(), FALSE) ;
    /*disable the active call PIO if there is one*/
    PioDrivePio(sinkHfpDataGetCallActivePIO(), FALSE) ;
     
}

/****************************************************************************
NAME
    stateManagerEnterConfigMode

DESCRIPTION
    Function that sets state to config mode,
    and indicates that with related LED pattern.
*/
void stateManagerEnterConfigMode(void)
{
    stateManagerSetState( deviceInConfigMode );
    /* sink_ext_state_inConfigMode is set in stateManagerSetState */
    LedManagerIndicateExtendedState();
}


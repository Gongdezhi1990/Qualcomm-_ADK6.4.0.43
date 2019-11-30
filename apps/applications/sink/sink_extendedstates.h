/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_extendedstates
    
DESCRIPTION
    Definition of the extended states for LED indication
*/

#ifndef SINK_EXTENDEDSTATES_H_
#define SINK_EXTENDEDSTATES_H_

/*!
@file   sink_extendedstates.h
@brief  Header file for the extended states for LED indication

    This defines the extended states used by the LEDs to indicate more states

*/

/*!
	@brief The supported extended states
*/
/*
    NB: When adding a new extended state, it must be put at
        the end of the enum but before SINK_NUM_EXTENDED_STATES
        + Should follow the same order in gHSExtendedStateStrings
        (see sink_statemanager.c)
        + needs consistency with "values" in global XML config file
*/
typedef enum
{
        /*! The device is logically off but physically on - limbo */	
    sink_ext_state_limbo=0,
        /*! The device is connectable - page scanning */
    sink_ext_state_connectable,
        /*! The device is connectable and discoverable - page and inquiry scanning - to a source*/
    sink_ext_state_connDiscoverableToSource,
        /*! The device is connectable and discoverable - page and inquiry scanning - to a peer*/
    sink_ext_state_connDiscoverableToPeer,  
        /*! The device is connected to AG*/
    sink_ext_state_connectedToAG,
        /*! The device is connected to peer*/
    sink_ext_state_connectedToPeer,    
            /*! The device is connected to both source and peer*/
    sink_ext_state_connectedToSourceAndPeer,
        /*! The connected AG has an outgoing call in progress*/
    sink_ext_state_outgoingCallEstablish,
        /*! The connected AG has an incoming call in progress*/
    sink_ext_state_incomingCallEstablish,
        /*! The connected AG has an active call in progress and the audio is in the device */
    sink_ext_state_activeCallSCO ,
        /*! The device is in test mode*/
    sink_ext_state_testMode ,
        /*! The connected AG has an active call and a second incoming call*/
    sink_ext_state_threeWayCallWaiting,
        /*! The connected AG has an active call and a second call on hold*/
    sink_ext_state_threeWayCallOnHold,
        /*! The connected AG has more than one active call*/
    sink_ext_state_threeWayMulticall,
        /*! The connected AG has an incoming call on hold*/
    sink_ext_state_incomingCallOnHold, 
        /*! The connected AG has an active call and the audio is in the handset*/
    sink_ext_state_activeCallNoSCO,
        /*! The device is streaming A2DP audio*/
    sink_ext_state_a2dpStreaming,
        /*! low battery state, won't actually change to this state but will be used for independant 
           low battery led warning */
    sink_ext_state_lowBattery,
        /*! Application runs in config mode */
    sink_ext_state_inConfigMode,
            /*! When no extended state found, default pattern */
    sink_ext_state_Default,
/*    
* ADD EXTENDED STATE RIGHT HERE        
*/
        /*! The maximum number of device extended states - must always be the last enum item */
    SINK_NUM_EXTENDED_STATES

} sink_extended_state_t;

#endif


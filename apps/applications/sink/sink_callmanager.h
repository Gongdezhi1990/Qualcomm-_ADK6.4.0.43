/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   This is the call manager for Sink device application
*/
#ifndef _HS_CALL_MANAGER_H_
#define _HS_CALL_MANAGER_H_

#include <hfp.h>
#include "sink_events.h"

/****************************************************************************
NAME    
    sinkHandleRingInd
DESCRIPTION
   handle a ring indication from the AG

RETURNS
    void
*/
void sinkHandleRingInd( const HFP_RING_IND_T * pInd );

/****************************************************************************
NAME    
    sinkAnswerOrRejectCall
    
DESCRIPTION
    Answer or Reject an incoming call from the device

RETURNS
    void
*/
void sinkAnswerOrRejectCall( bool Action );

/****************************************************************************
NAME    
    sinkHangUpCall
    
DESCRIPTION
    Hang up the call from the device.

RETURNS
    void
*/
void sinkHangUpCall( void );
        
/****************************************************************************
NAME    
    sinkInitiateLNR
    
DESCRIPTION
    If HFP and connected - issues command
    If HFP and not connected - connects and issues if not in call
    If HSP sends buton press

RETURNS
    void
*/
void sinkInitiateLNR ( hfp_link_priority priority );

/****************************************************************************
NAME    
    sinkInitiateVoiceDial
    
DESCRIPTION
    If HFP and connected - issues command
    If HFP and not connected - connects and issues if not in call
    If HSP sends buton press

RETURNS
    void
*/
void sinkInitiateVoiceDial ( hfp_link_priority priority );

/****************************************************************************
NAME    
    sinkCancelVoiceDial
    
DESCRIPTION
    cancels a voice dial request
   
RETURNS
    void
*/
void sinkCancelVoiceDial ( hfp_link_priority priority );

/****************************************************************************
NAME    
    sinkRecallQueuedEvent
    
DESCRIPTION
    Checks to see if an event was Queued and issues it - used on connection

RETURNS
    void
*/
void sinkRecallQueuedEvent ( void ) ;

/****************************************************************************
NAME    
    sinkClearQueueudEvent
    
DESCRIPTION
    Clears the QUEUE - used on failure to connect / power on / off etc

RETURNS
    void
*/
void sinkClearQueueudEvent ( void ) ;

/****************************************************************************
NAME    
    sinkUpdateStoredNumber
    
DESCRIPTION
	Request a number to store from the primary AG
    
RETURNS
    void
*/
void sinkUpdateStoredNumber (void);

/****************************************************************************
NAME    
    sinkWriteStoredNumber
    
DESCRIPTION
	Store number obtained via HfpRequestNumberForVoiceTag in 
    HFP data Phone Number config Block
    
RETURNS
    void
*/
void sinkWriteStoredNumber ( const HFP_VOICE_TAG_NUMBER_IND_T* ind );

/****************************************************************************
NAME
    sinkDialStoredNumber
    
DESCRIPTION
    checks on connection for an audio connction and dials a stored number from 
	the config

RETURNS
    void
*/
void sinkDialStoredNumber ( void ) ;

/****************************************************************************
NAME    
    sinkQueueEvent
    
DESCRIPTION
    Queues an event to be sent once the device is connected

RETURNS
    void
*/
void sinkQueueEvent ( sinkEvents_t pEvent );

/****************************************************************************
NAME    
    sinkPlaceIncomingCallOnHold
    
DESCRIPTION
	looks for an incoming call and performs the twc hold incoming call function

RETURNS
    void
*/
void sinkPlaceIncomingCallOnHold(void);

/****************************************************************************
NAME    
    sinkAcceptHeldIncomingCall
    
DESCRIPTION
	looks for a held incoming call and performs the twc accept held incoming
    call function

RETURNS
    void
*/
void sinkAcceptHeldIncomingCall(void);

/****************************************************************************
NAME    
    sinkRejectHeldIncomingCall
    
DESCRIPTION
	looks for a held incoming call and performs the twc reject held incoming
    call function

RETURNS
    void
*/
void sinkRejectHeldIncomingCall(void);

/****************************************************************************
NAME
    sinkCallManagerGetHfpCallState

DESCRIPTION
    returns the hfp_call_state of the hfp link at the requested index
*/
hfp_call_state sinkCallManagerGetHfpCallState(hfp_link_priority priority);

/****************************************************************************
NAME
    sinkCallManagerGetHfpSink

DESCRIPTION
    returns the audio sink of the hfp link at the requested index
*/
Sink sinkCallManagerGetHfpSink(hfp_link_priority priority);


/****************************************************************************
NAME
    sinkDialGivenNumber
    
DESCRIPTION
    checks on connection for an audio connction and dials a given number

RETURNS
    bool: TRUE if the given number is valid, else FALSE
*/
bool sinkDialGivenNumber ( uint16 phone_number_length, const uint8* phone_number, const hfp_link_priority priority );


/****************************************************************************
NAME
    sinkDialSavedGivenNumber
    
DESCRIPTION
    checks on connection for an audio connction and dials a number given by a
    prior call to sinkDialGivenNumber

RETURNS
    void
*/
void sinkDialSavedGivenNumber ( void );


#endif


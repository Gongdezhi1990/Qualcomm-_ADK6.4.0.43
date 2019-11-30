/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief Header file for the state handling of the sink application.

    Provides single point of entry and exit for all of the device states
*/

#ifndef _SINK_STATE_MANAGER_H
#define _SINK_STATE_MANAGER_H

#include <csrtypes.h>

#include "sink_extendedstates.h"
#include "sink_states.h"

/* We need a type to be used as part of audio amplifier power control for PowerOnPIO signal given above*/
typedef enum {
 POWER_DOWN = 0,
 POWER_UP
} power_control_dir ;

/* Enum to record sink discoverability status */
typedef enum
{
    no_discoverability, /* Not discoverable */
    user_triggered_discoverability, /* Discoverable due to user event*/
    fp_seeker_triggered_discoverability /* FP Provider going discoverable due to Seeker's Kbp write request with discoverability bit set to 1 */
}fp_discoverability_status_t;

/*!
	@brief Instruct the device to enter the connectable state
	
	@param req_disc If the device is currently connected then request a HfpDisconnect
	
	After this funciton is called, the device will be in page scanning mode
*/
void stateManagerEnterConnectableState( bool req_disc ) ;

/*!
	@brief Instruct the device to enter the connected state
	
	If the device was previously discoverable or connectable, then disables these modes
	
	If the device was previously in an active call the EventSysEndOfCall is sent
*/
void stateManagerEnterConnectedState( void ) ;

/*!
	@brief Instruct the device to enter the connectable/Discoveranble state
	
	After this function is callerd, then the device will be both page scanning and 
    inquiry scanning
*/
void stateManagerEnterConnDiscoverableState( bool req_disc ) ;

/*!
	@brief Instruct the device to enter the Incoming call state
	
	This occurs after the HFP call setup indicator has changed
	or in HSP mode, the RING has been detected
	
	If the device was previously in a non-conencted state and the feature bit to 
    auto answer is enabled, then the device will attempt to answer the call
    
    Any Incoming RING PIO is set high
*/
void stateManagerEnterIncomingCallEstablishState( void ) ;

/*!
	@brief Instruct the device to enter the outgoing call state
	
	This occurs after the HFP call setup indicator has changed
	Any outgoing RING is set high	
*/
void stateManagerEnterOutgoingCallEstablishState( void ) ;

/*!
	@brief Instruct the device to enter the active call state
	
	This occurs after the HFP call indicator has changed
	
	Incoming / outgoing PIOs are cleared 
	Any active call PIO is set 
	
*/
void stateManagerEnterActiveCallState( void ) ;

/*!
	@brief Instruct the device to enter the Limbo state
	
	The device is now logically off but physically still on.
	
	This occurs on power on before a power on event has been received 
    OR after a power off event has ben received and before the device is 
    physically shut down.
    
    The device remains in this state for the Limbo Timout which can 
    be configured in  sink_private data timers 
*/
void stateManagerEnterLimboState( sinkLimboReason reason ) ;

/*!
	@brief Instruct the device to check the Limbo state
	
	This occurs periodically to check if the device can physically power
	down (after the limbo timeout)
	
	If the charger is not connected then the device will physically power down 
	
	If the charger is connected, then the device remains in the limbo state in order to 
	service any charger LED indications
*/
void stateManagerUpdateLimboState( void ) ;

/*!
	@brief Instruct the device to physically power on
	
	This occurs only after receiveing a power on event
	After this call, the device will be in the conenctable state
*/
void stateManagerPowerOn( void ) ;

/*!
	@brief Instruct the device to enter the three way call waiting state	
*/
void stateManagerEnterThreeWayCallWaitingState( void ) ;

/*!
	@brief Instruct the device to enter the three way call on hold state	
*/
void stateManagerEnterThreeWayCallOnHoldState( void ) ;

/*!
	@brief Instruct the device to enter the three way multi party call state	
*/
void stateManagerEnterThreeWayMulticallState( void ) ;

/*!
	@brief Instruct the device to enter the single incoming on hold state	
*/
void stateManagerEnterIncomingCallOnHoldState( void ) ;

/*!
	@brief request the current state of the device 

    @return The current state of the device	
*/
sinkState stateManagerGetState ( void ) ;

/*!
    @brief request the current extended state of the device 

    @return The current extended state of the device
*/
sink_extended_state_t stateManagerGetExtendedState ( void );

/*!
	@brief request whether or not the device is connected 

    @return TRUE if connected
*/
bool stateManagerIsConnected ( void ) ;

/*!
	@brief Instruct the device to enter the test mode state
*/
void stateManagerEnterTestModeState ( void ) ;

/*!
	@brief enter A2DP streaming state if not showing any active call states	
*/
void stateManagerEnterA2dpStreamingState(void);

/*!
	@brief method to control logic state of mute and power PIOs
	
    @param control Power state to transistion to
*/
void stateManagerAmpPowerControl(power_control_dir control);

/*!
	@brief check to see if external hardware is ready for audio playback 

    @return TRUE if hardware is ready for audio playback, else FALSE
*/
bool stateManagerIsReadyForAudio(void);

/*!
    @brief check to see if device is shutting down

    @return TRUE if is shutting down, else FALSE
*/
bool stateManagerIsShuttingDown(void);

/*!
    @brief check to see if device is waking up 

    @return TRUE if is waking up, else FALSE
*/
bool stateManagerIsWakingUp(void);

/*!
    @brief Function that sets config mode state, and displays LED pattern.
*/
void stateManagerEnterConfigMode(void);

/*!
    @brief Function that updates the extended states, and indicates LED pattern.
*/
void updateExtendedState ( sinkState pNewState );
        
#endif


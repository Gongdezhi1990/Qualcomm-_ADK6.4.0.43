/*
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    Module responsible for managing the PIO outputs including LEDs
*/

#ifndef SINK_LEDS_H
#define SINK_LEDS_H

#include "sink_leddata.h"
#include "sink_states.h"


/****************************************************************************
NAME	
	PioSetLedPin

DESCRIPTION
    Fn to change an LED attached to a PIO, a special LED pin , or a tricolour LED
    
RETURNS
	void
*/
void PioSetLedPin ( uint16 pPIO , bool pOnOrOff ) ;

/****************************************************************************
NAME	
	PioSetDimState  
	
DESCRIPTION
    Update funtion for a led that is currently dimming
    
RETURNS
	void
*/
void PioSetDimState ( uint16 pPIO );



/****************************************************************************
NAME 
    LedsTaskInit

DESCRIPTION
 	Initialise the Leds data
RETURNS
    void
    
*/
void LedsTaskInit ( void ) ;


/****************************************************************************
NAME 
    ledsIndicateLedsPattern

DESCRIPTION
 	Given the indication type and leds pattern, Play the LED Pattern
RETURNS
    void
*/
void ledsIndicateLedsPattern(LEDPattern_t *lPattern, uint8 lIndex, IndicationType_t Ind_type, uint16 state_or_event);
        

/****************************************************************************
NAME 
    LedsIndicateNoState

DESCRIPTION
    used to cancel any outstanding state indications

RETURNS
    void
    
*/
void LedsIndicateNoState ( void ) ;

/****************************************************************************
NAME 
 LedActiveFiltersCanOverideDisable

DESCRIPTION
    Check if active filters disable the global LED disable flag.
RETURNS
 	bool
*/
bool LedActiveFiltersCanOverideDisable( void ) ;

/****************************************************************************
NAME 
    LedsCheckForFilter

DESCRIPTION
    checks and sets a filter if one is defined for the given event
    
RETURNS
    void    
*/
void LedsCheckForFilter ( sinkEvents_t pEvent ) ;

/****************************************************************************
NAME 
    LedsEnableFilterOverrides

DESCRIPTION
    Enable or disable filters overriding LEDs. This will not change which 
    filters are active, it will just turn off any LEDs the filters are 
    forcing on.
    
RETURNS
    void    
*/
void LedsEnableFilterOverrides(bool enable);

/****************************************************************************
NAME 
    LedsSetLedActivity

DESCRIPTION
    Sets a Led Activity to a known state
RETURNS
    void
*/
void LedsSetLedActivity ( LEDActivity_t * pLed , IndicationType_t pType , uint16 pIndex , uint16 pDimTime) ;

/****************************************************************************
NAME 
    LedsResetAllLeds

DESCRIPTION
    resets all led pins to off and cancels all led and state indications
RETURNS
    void
*/
void LedsResetAllLeds ( void ) ;

/****************************************************************************
NAME 
 isOverideFilterActive

DESCRIPTION
    determine if an overide filter is currently active and driving one of the
    leds in which case return TRUE to prevent it being turned off to display 
    another pattern, allows solid red with flashing blue with no interuption in
    red for example.
RETURNS
    true or false
*/
bool isOverideFilterActive ( uint8 Led );

/****************************************************************************
NAME 
    LedsIndicateError

DESCRIPTION
    Indicates a fatal application error by flashing each LED in turn.

    After this has been called only the error LED pattern will be displayed.
    All state, event and filter patterns will be ignored.
    Error values should all be defined in sink_led_err.h, as this helps make them unique.

RETURNS
    void
*/
void LedsIndicateError ( uint16 errId ) ;

/****************************************************************************
NAME 
    LedsInErrorMode

DESCRIPTION
    Find out if the LED module is in error mode.

    This function will only return TRUE after LedsIndicateError
    has been called.

RETURNS
    TRUE if LED manager is in error mode, FALSE otherwise.
*/
uint16 LedsInErrorMode(void);

/****************************************************************************
NAME 
    LedsActiveLed

DESCRIPTION
    Map a value covering the full range of possible "LEDs" to an active LED

    This function returns a pointer to the active LED matching that requested.
    If there is no matching LED, it will be added - if possible, otherwise
    the address of the last entry in the table will be returned (or the
    function will Panic in debug mode.

PARAMETERS
    led Identifier of the output, whether an LED identifier or a PIO. Tri-state identifiers shoud not be used.

RETURNS
    Pointer to the LEDActivity_t entry matching the requested LED.
*/

LEDActivity_t *LedsActiveLed(uint16 led);

/****************************************************************************
NAME 
    LedsSetLedCurrentIndicateEvent

DESCRIPTION
    Sets a Led CurrentlyIndicatingEvent flag.
RETURNS
    void
*/
void LedsSetLedCurrentIndicateEvent(bool event_status);


#endif


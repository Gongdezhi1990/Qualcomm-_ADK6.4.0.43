/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/
#ifndef SINK_LED_MANAGER_H
#define SINK_LED_MANAGER_H

#include "sink_extendedstates.h"
#include "sink_states.h"
#include "sink_events.h"
#include "sink_leddata.h"


#define LedManagerQueuedEvent() (LedGetGlobalData()->gLedEventQueue[0])

#define MAKE_LEDS_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);

void LedManagerMemoryInit(void);
void LedManagerInit(bool full_init);
void LedManagerIndicateEvent (MessageId pEvent);
void LedManagerIndicateQueuedEvent(void);
void LedManagerIndicateExtendedState (void);
void LedManagerDisableLEDS(void);
void LedManagerEnableLEDS(void);
void LedManagerToggleLEDS(void);
void LedManagerResetLEDIndications(void);
void LedManagerResetStateIndNumRepeatsComplete(void);
void LedManagerCheckTimeoutState(void);
void LedManagerForceDisable(bool disable);

#ifdef DEBUG_LM
void LMPrintPattern(LEDPattern_t * pLED);
#endif

/****************************************************************************
NAME
    LedManagerIsEnabled

DESCRIPTION
    Get if LED is enabled or not

PARAMS
    void 

RETURNS
    Bool TRUE for enabled otherwise FALSE
*/
bool LedManagerIsEnabled(void);

/****************************************************************************
NAME
    LedManagerGetStateTimeout

DESCRIPTION
    Get the LED state timeout
    
PARAMS
    void 

RETURNS
    Bool
*/
bool LedManagerGetStateTimeout(void);

/****************************************************************************
NAME
    LedManagerSetStateTimeout

DESCRIPTION
    Set the LED state timeout
    
PARAMS
    Bool 

RETURNS
    void
*/
void LedManagerSetStateTimeout(bool value);

/****************************************************************************
NAME
    LedManagerGetExtendedStatePattern

DESCRIPTION
    Get extended state pattern for the given sink extended state.

PARAMS
    state [in] Sink extended state for which pattern is requested
    pattern [out] Pointer to the memory to store the led pattern in.

RETURNS
    TRUE if pattern was found, FALSE otherwise.    
*/
bool LedManagerGetExtendedStatePattern(sink_extended_state_t state, LEDPattern_t *pattern);

/****************************************************************************
NAME
    LedManagerSetExtendedPattern

DESCRIPTION
    Get LED pattern for the given extended state. Only the existing patterns will get
    set; so if the state does not have a pattern it will not be added to
    the config.

PARAMS
    state [in] Sink state to update the pattern for.
    pattern [in] Pointer to the led pattern data.

RETURNS
    TRUE if pattern was updated, FALSE otherwise.
*/
bool LedManagerSetExtendedStatePattern(sink_extended_state_t state, const LEDPattern_t *pattern);

/****************************************************************************
NAME
    LedManagerGetEventPattern

DESCRIPTION
    Get pattern based on events.
    
PARAMS
    event          Event for which pattern is requested
    pEventPattern  Pointer to pattern is retrieved by caller

RETURNS
    Bool 
    
*/
bool LedManagerGetEventPattern(sinkEvents_t event, LEDPattern_t *pEventPattern);

/****************************************************************************
NAME
    LedManagerSetEventPattern

DESCRIPTION
    Set pattern for the given event. Only the exisiting patterns will get
    set; so if the event does not have a pattern it will not be added to
    the config.

PARAMS
    event          Event for which pattern is to be set.
    pEventPattern  Pointer to pattern to set.

RETURNS
    Bool 
    
*/
bool LedManagerSetEventPattern(sinkEvents_t event, const LEDPattern_t *pEventPattern);

/****************************************************************************
NAME
    LedManagerGetEventFilter

DESCRIPTION
    Get LED filter for the given sink event.

PARAMS
    index [in] Index for which filter is requested
    filter [out] Pointer to memory to write filter data to.

RETURNS
    TRUE if filter found, FALSE otherwise.
*/
bool LedManagerGetEventFilter(uint16 index, LEDFilter_t *filter);

/****************************************************************************
NAME
    LedManagerSetEventFilter

DESCRIPTION
    Set pattern for the given event. Only the exisiting patterns will get
    set; so if the event does not have a pattern it will not be added to
    the config.

PARAMS
    index [in] Index for which filter is to be set.
    filter [in] Filter to set.

RETURNS
    TRUE if filter was updated, FALSE otherwise.
*/
bool LedManagerSetEventFilter(uint16 index, LEDFilter_t *filter);

/****************************************************************************
NAME
    LedManagerQueueLedEvents

DESCRIPTION
    Whether LED Events need to be queued
    Note: Allocation/Deallocation of Read config structure need to removed once the config library is available
    
PARAMS
    void

RETURNS
    Bool TRUE for Queuing otherwise FALSE
*/
bool LedManagerQueueLedEvents(void);

/****************************************************************************
NAME
    LedManagerOverideFilterOn

DESCRIPTION
    Whether Overide Filter is Permanently on
    Note: Allocation/Deallocation of Read config structure need to removed once the config library is available
    
PARAMS
    void

RETURNS
    Bool TRUE for ON otherwise FALSE
*/
bool LedManagerOverideFilterOn(void);

/****************************************************************************
NAME
    LedManagerLedEnablePioValue

DESCRIPTION
    Returns the configured PIO for Led Enabling.

RETURNS
    Led Enable pio value
*/
uint8 LedManagerLedEnablePioValue(void);

/****************************************************************************
NAME
    LedManagerChargerTermLedOveride

DESCRIPTION
    Whether LED Charger termination LED overide is ON
    Note: Allocation/Deallocation of Read config structure need to removed once the config library is available
    
PARAMS
    void

RETURNS
    Bool TRUE for ON otherwise FALSE
*/
bool LedManagerChargerTermLedOveride(void);

/****************************************************************************
NAME
    LedManagerResetLedState

DESCRIPTION
    If set the LED disable state is reset after boot 
    Note: Allocation/Deallocation of Read config structure need to removed once the config library is available
    
PARAMS
    void

RETURNS
    Bool TRUE for Reset otherwise FALSE
*/
bool LedManagerResetLedState(void);

/****************************************************************************
NAME
    LedManagerGetTimeMultiplier

DESCRIPTION
    Get the time multiplier value for LED
    
PARAMS
    void

RETURNS
    uint16
*/
uint16 LedManagerGetTimeMultiplier(void);

/****************************************************************************
NAME
    LedManagerGetSessionLedData

DESCRIPTION
    Set Led data from Session Data
    
PARAMS
    session_data

RETURNS
    void 
    
*/
bool LedManagerGetSessionLedData(void);


/****************************************************************************
NAME
    void LedManagerSetSessionLedData(void)


DESCRIPTION
    Set session data from Led data
    
PARAMS
    session_data

RETURNS
    void 
    
*/
void LedManagerSetSessionLedData(void);


/*************************************************************************
NAME    
    ledManagerGetNumFilters
    
DESCRIPTION
    Function to get Number of event filters allocated

PARAMS
    void

RETURNS
    uin16: Number of event filters allocated
    
**************************************************************************/
uint16 ledManagerGetNumFilters(void);

/*************************************************************************
NAME    
    ledManagerGetTriColorLeds
    
DESCRIPTION
    Function to get Tri Color LEDs set

PARAMS
    pTriColorLed  Pointer to PioTriColLeds_t

RETURNS
    void    
**************************************************************************/
void ledManagerGetTriColorLeds(PioTriColLeds_t *pTriColorLed);


/****************************************************************************
NAME
    LedManagerSetLedEnabled

DESCRIPTION
    Set LED enabled or not

PARAMS
    Bool 

RETURNS
    void
*/
void LedManagerSetLedEnabled(bool value);


/****************************************************************************
NAME
    LedManagerGetLedStatePattern

DESCRIPTION
    Get the pattern data for the Sink app state at index idx.

PARAMS
    pattern Pointer to pattern struct to copy the values to.
    idx Index into the state pattern array to copy the values from.

RETURNS
    void
*/
void LedManagerGetLedStatePattern(uint16 idx, LEDPattern_t *pattern);


/****************************************************************************
NAME
    LedManagerGetLedEventPattern

DESCRIPTION
    Get the pattern data for the Sink event at index idx.

PARAMS
    pattern Pointer to pattern struct to copy the values to.
    idx Index into the event pattern array to copy the values from.

RETURNS
    void
*/
void LedManagerGetLedEventPattern(uint16 idx, LEDPattern_t *pattern, uint16 *event);

#endif

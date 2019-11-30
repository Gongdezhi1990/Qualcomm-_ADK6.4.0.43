/*
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    Module responsible for managing the PIO outputs including LEDs
*/

/****************************************************************************
INCLUDES
*/
#include "sink_debug.h"
#include "sink_led_manager.h"
#include "sink_configmanager.h"
#include "sink_statemanager.h"
#include "sink_led_err.h"
#include "sink_leds.h"
#include "sink_leddata.h"
#include "sink_powermanager.h"
#include "sink_config.h"
#include "sink_main_task.h"
#include "sink_malloc_debug.h"
#include "sink_led_config_def.h"

#include <config_store.h>

#include <panic.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>


/****************************************************************************
DEFINITIONS
*/

#ifdef DEBUG_LM
#define LM_DEBUG(x) DEBUG(x)
#else
#define LM_DEBUG(x) 
#endif


/* Global config data */
static sink_led_global_data_t gLedData;
#define LED_GDATA   gLedData


/*************************************************************************
NAME    
    ledManagerInitStatePatterns
    
DESCRIPTION
    Copy the LED state patterns config to the working copy.

    The working copy is required because in config mode this module needs
    it to lookup the "config state" pattern, but at the same time a host,
    e.g. the config tool, also needs to access it to change it.

RETURNS
    uin16: size, if 0 then there is an error in read, else success
    
**************************************************************************/
static uint16 ledManagerInitStatePatterns(void)
{
    sink_led_state_pattern_config_def_t *state_pattern;
    uint16 size = 0;

    LM_DEBUG(("LED:ledManagerInitStatePatterns()\n"));

    size = configManagerGetReadOnlyConfig(SINK_LED_STATE_PATTERN_CONFIG_BLK_ID, (const void **)&state_pattern);
    if (size)
    {
        LED_GDATA.gStatePatternsAllocated = (uint16)((size * sizeof(uint16)) / sizeof(state_pattern->pStatePatterns[0]));
        if (LED_GDATA.gStatePatternsAllocated > SINK_NUM_EXTENDED_STATES)
            Panic();

        /*Copy all words from config state pattern and each word is of 2 bytes*/
        if (!LED_GDATA.pStatePatterns) /* Not allocated */
        {
            LED_GDATA.pStatePatterns =PanicUnlessMalloc(size*sizeof(uint16)) ;
        }
        else /* Already allocated */
        {
            LED_GDATA.pStatePatterns = PanicUnlessRealloc(LED_GDATA.pStatePatterns, size*sizeof(uint16)) ;
        }
        memcpy(LED_GDATA.pStatePatterns, state_pattern->pStatePatterns, (size*sizeof(uint16)));

        LM_DEBUG(("LED:ledManagerInitStatePatterns(): Init Success, Size %d\n", size));
    }
    configManagerReleaseConfig(SINK_LED_STATE_PATTERN_CONFIG_BLK_ID);
    return size;
}

/*************************************************************************
NAME    
    ledManagerGetExtendedStatePatternIndex
    
DESCRIPTION
    Get the pattern data for a particular Sink state.

PARAMS
    state Sink state to get pattern for.
    pattern Pointer to the struct to copy the pattern data to.
    pattern_index Pointer to save index into state pattern array.

RETURNS
    bool TRUE if state was found, FALSE otherwise
    
**************************************************************************/
static bool ledManagerGetExtendedStatePatternIndex(sink_extended_state_t state, LEDPattern_t *pattern, uint8 *pattern_index)
{
    uint16 i;

    *pattern_index = NO_STATE_OR_EVENT;

    for (i = 0; i < LED_GDATA.gStatePatternsAllocated; i++)
    {
        if (LED_GDATA.pStatePatterns[i].state == state)
        {
            *pattern_index = i;
            *pattern = LED_GDATA.pStatePatterns[i].pattern;
            break;
        }
    }

    if (NO_STATE_OR_EVENT == *pattern_index)
        return FALSE;
    else
        return TRUE;
}


/*************************************************************************
NAME    
    ledManagerGetEventPatternIndex
    
DESCRIPTION
    Get the pattern data for a particular Sink event.

PARAMS
    event         Sink event to get pattern for.
    pattern       Pointer to the struct to copy the pattern data to.
    pattern_index Pointer to save index into state pattern array.

RETURNS
    bool TRUE if state was found, FALSE otherwise
    
**************************************************************************/
static bool ledManagerGetEventPatternIndex(sinkEvents_t event, LEDPattern_t *pattern, uint8 *pattern_index)
{
    uint16 i;
    uint16 size;
    uint16 num_patterns;
    sink_led_event_pattern_config_def_t *event_pattern;

    *pattern_index = NO_STATE_OR_EVENT;

    size = configManagerGetReadOnlyConfig(SINK_LED_EVENT_PATTERN_CONFIG_BLK_ID, (const void **)&event_pattern);
    if (size)
    {
        num_patterns = ((size * sizeof(uint16)) / sizeof(event_pattern->pEventPatterns[0]));

        for (i = 0; i < num_patterns; i++)
        {
            if (event_pattern->pEventPatterns[i].event == event)
            {
                *pattern_index = i;
                *pattern = event_pattern->pEventPatterns[i].pattern;
                break;
            }
        }
    }
    configManagerReleaseConfig(SINK_LED_EVENT_PATTERN_CONFIG_BLK_ID);

    if (NO_STATE_OR_EVENT == *pattern_index)
        return FALSE;
    else
        return TRUE;
}


/****************************************************************************
NAME 
 ledManagerInitActiveLEDS

DESCRIPTION
 Creates the active LED space for the number of leds the system supports

RETURNS
 void
    
*/
static void ledManagerInitActiveLEDS(void) 
{
    uint16 lIndex = 0; 

    memset(LED_GDATA.gLedMapping,NO_STATE_OR_EVENT,sizeof(LED_GDATA.gLedMapping));

    for ( lIndex= 0;lIndex < SINK_NUM_LEDS; lIndex ++)
    {
        LedsSetLedActivity (&LED_GDATA.gLedActivity[lIndex], IT_Undefined, 0, 0);
    }
}


/****************************************************************************
NAME 
 ledManagerInitFilterPatterns

DESCRIPTION
    Function to get LED event filter patterns config data.

RETURNS
    uin16: size, if 0 then there is an error in read, else success
    
**************************************************************************/
static uint16 ledManagerInitFilterPatterns(void)
{
    sink_led_event_filter_config_def_t *event_filter;
    uint16 size=0;

    LM_DEBUG(("LED:ledManagerInitFilterPatterns()\n"));

    size = configManagerGetReadOnlyConfig(SINK_LED_EVENT_FILTER_CONFIG_BLK_ID, (const void **)&event_filter);
    if (size)
    {
        LED_GDATA.gLMNumFiltersUsed = ((size * sizeof(uint16)) / sizeof(LEDFilter_t));
        LED_GDATA.pEventFilters = event_filter->pEventFilters;

        LM_DEBUG(("LED:ledManagerInitFilterPatterns(): Init Success, Size %d\n", size));

        /* Never release the config on purpose because it will be read at runtime. */
    }
    else
    {
        configManagerReleaseConfig(SINK_LED_EVENT_FILTER_CONFIG_BLK_ID);
    }
    return size;
}


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
void LedManagerSetLedEnabled(bool value)
{    
    LED_GDATA.gLEDSEnabled = value;
}


#ifdef DEBUG_LM
/****************************************************************************
NAME 
 LMPrintPattern

DESCRIPTION
    debug fn to output a LED pattern
    
RETURNS
 void
*/

void LMPrintPattern ( LEDPattern_t * pLED ) 
{
#ifdef DEBUG_PRINT_ENABLED
    const char * const lColStrings [ 5 ] =   {"ERROR ","LED_A","LED_B","ALT","Both"} ;
    if(pLED)
    {
        LM_DEBUG(("[%d][%d] [%d][%d][%d] ", pLED->LED_A , pLED->LED_B, pLED->OnTime ,pLED->OffTime ,pLED->RepeatTime)) ;  
        LM_DEBUG(("[%d] [%d] [%s]\n",       pLED->NumFlashes, pLED->TimeOut, lColStrings[pLED->Colour])) ;    
        LM_DEBUG(("[%d]\n",       pLED->OverideDisable)) ;    
    }
    else
    {
        LM_DEBUG(("LMPrintPattern = NULL \n")) ;  
    }
#endif  /* DEBUG_PRINT_ENABLED */

}
#endif /* DEBUG_LM */

static bool ledManagerCanPlayPattern(LEDPattern_t* pattern)
{
    if(LED_GDATA.gLEDSSuspend)
        return FALSE;

    return (LedManagerIsEnabled() || pattern->OverideDisable || LedActiveFiltersCanOverideDisable());
}

/***********************GLOBAL FUNCTIONS**************************************/
/****************************************************************************
NAME
    LedGetGlobalData

DESCRIPTION
    Get the LED manager task data

RETURNS
    LED task data
    
*/
sink_led_global_data_t *LedGetGlobalData(void)
{
    return &LED_GDATA;
}

/****************************************************************************
NAME
    LedManagerMemoryInit

DESCRIPTION
    Initialise memory for led manager, this has moved from theSink as ran 
    out of globals space.

RETURNS
    void
    
*/
void LedManagerMemoryInit(void) 
{
    memset(&LED_GDATA, 0, sizeof(sink_led_global_data_t));
}

/****************************************************************************
NAME
    LedManagerInit

DESCRIPTION
    Initialise led manager task and global data.

RETURNS
    void
*/
void LedManagerInit(bool full_init)
{
    LM_DEBUG(("Led Manager Init...\n"));

    /* Initialise memory for the led manager */
    LedManagerMemoryInit();

    /*Initialise globals*/
    LED_GDATA.gConfigMode = FALSE;
    LED_GDATA.gLEDSSuspend = FALSE;
    LED_GDATA.gCurrentlyIndicatingEvent = FALSE ;
    LED_GDATA.gFollowing = FALSE ;
    LED_GDATA.gErrorCode = led_err_id_enum_no_error ;
    memset(LED_GDATA.gLedEventQueue, 0, (sizeof(sinkEvents_t)*MAX_LED_EVENT_QUEUE_SIZE));
    ledManagerInitActiveLEDS();
    LED_SETACTIVEFILTERS(0x0);

    ledManagerInitStatePatterns();
    if(full_init)
    {
        ledManagerInitFilterPatterns();
    }

    /*Init Task Message Handler*/
    LedsTaskInit();
}


/****************************************************************************
NAME 
 LedManagerIndicateEvent

DESCRIPTION
 displays event notification
    This function also enables / disables the event filter actions - if a normal event indication is not
    associated with the event, it checks to see if a filer is set up for the event 

RETURNS
 void
    
*/
void LedManagerIndicateEvent ( MessageId event ) 
{
    uint8 i, lPatternIndex;
    LEDPattern_t lPattern;

    LM_DEBUG(("LM: IndicateEvent [%x]\n", event)) ;   

    if (LedsInErrorMode())
    {
        LM_DEBUG(("LM: Ignoring event: LM in config error mode\n"));
        return;
    }

    /*if there is an event configured*/
    if (ledManagerGetEventPatternIndex(event, &lPattern, &lPatternIndex))
    {
        /*only indicate if LEDs are enabled*/
        if (ledManagerCanPlayPattern(&lPattern))
        {
            LM_DEBUG(("LM: IE[%x]\n", event));
 
            /*only update if wer are not currently indicating an event*/
            if (!LED_GDATA.gCurrentlyIndicatingEvent)
            {
                ledsIndicateLedsPattern(&lPattern, lPatternIndex, IT_EventIndication, event);
            }
            else
            {
                if (LedManagerQueueLedEvents())
                {
                    /*try and add it to the queue*/
                    LM_DEBUG(("LM: Queue LED Event [%x]\n", event));

                    for(i = 0; i < MAX_LED_EVENT_QUEUE_SIZE; i++)
                    {
                        if(LED_GDATA.gLedEventQueue[i] == 0)
                        {
                            LED_GDATA.gLedEventQueue[i] = event;
                            break;
                        }
                    }

#ifdef DEBUG_LM
                    if (i == MAX_LED_EVENT_QUEUE_SIZE)
                    {
                        LM_DEBUG(("LM: Err Queue Full!!\n"));
                    }
#endif
                }
            }
        }
        else
        {
            LM_DEBUG(("LM: No IE disabled\n"));
        }
    }
    else
    {
        LM_DEBUG(("LM: NoEvPatCfg %x\n", event));
    }

    /* Indicate a filter if there is one present */
    LedsCheckForFilter(event);
}

/*************************************************************************
NAME    
    ledManagerGetNumFilters
    
DESCRIPTION
    Function to get Number of event filters allocated

RETURNS
    uin16: Number of event filters allocated
    
**************************************************************************/
uint16 ledManagerGetNumFilters(void)
{
    /*LM_DEBUG(("LED:: ledManagerGetNumFilters(): %d Filter Patterns\n", LED_GDATA.gLMNumFiltersUsed));*/
    return LED_GDATA.gLMNumFiltersUsed;
}

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
void ledManagerGetTriColorLeds(PioTriColLeds_t *pTriColorLed)
{
    sink_led_readonly_config_def_t *read_config = NULL;
    
    if(configManagerGetReadOnlyConfig(SINK_LED_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
        pTriColorLed->TriCol_a = read_config->TriColLeds.TriCol_a;
        pTriColorLed->TriCol_b = read_config->TriColLeds.TriCol_b;
        pTriColorLed->TriCol_c = read_config->TriColLeds.TriCol_c;
        configManagerReleaseConfig(SINK_LED_READONLY_CONFIG_BLK_ID);
    }
}


/****************************************************************************
NAME
    LEDManagerIndicateQueuedEvent

DESCRIPTION
    Display the event at the top of the event queue

RETURNS
    void
    
*/
void LedManagerIndicateQueuedEvent(void)
{
    uint8 i;
    /*LM_DEBUG(("LM : Queue [%x]", LED_GDATA.Queue[0]));*/
    
    LedManagerIndicateEvent(LED_GDATA.gLedEventQueue[0]) ;
    
    /* Shuffle the queue */
    for(i = 1; i < MAX_LED_EVENT_QUEUE_SIZE; i++)
    {
        /*LM_DEBUG(("[%x]", LED_GDATA.Queue[i]));*/
        LED_GDATA.gLedEventQueue[i - 1] = LED_GDATA.gLedEventQueue[i];
    }
    
    /*LM_DEBUG(("\n"));*/
    LED_GDATA.gLedEventQueue[MAX_LED_EVENT_QUEUE_SIZE - 1] = 0;
}

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
void LedManagerGetLedStatePattern(uint16 idx, LEDPattern_t *pattern)
{
    *pattern = LED_GDATA.pStatePatterns[idx].pattern;
}

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
void LedManagerGetLedEventPattern(uint16 idx, LEDPattern_t *pattern, uint16 *event)
{
    uint16 size;
    sink_led_event_pattern_config_def_t *event_pattern;

    size = configManagerGetReadOnlyConfig(SINK_LED_EVENT_PATTERN_CONFIG_BLK_ID, (const void **)&event_pattern);
    if (size)
    {
        *event = event_pattern->pEventPatterns[idx].event;
        *pattern = event_pattern->pEventPatterns[idx].pattern;
    }
    configManagerReleaseConfig(SINK_LED_EVENT_PATTERN_CONFIG_BLK_ID);
}

/****************************************************************************
NAME
    LedManagerIndicateExtendedState

DESCRIPTION
	displays the current state indication information

RETURNS
	void
    
*/

void LedManagerIndicateExtendedState(  void)
{
    /* Flags controlling which led indications are to be displayed are set to false by default */
    bool displayLedIndication = FALSE;

    /* Local variables and led pattern pointers are required for ledsIndicateLedsPattern(...) function
    to display the Led pattern */
    uint8 lPatternIndex = 0;
    LEDPattern_t lPattern = {0};

    /* Get the current extended state to display */
    sink_extended_state_t state = stateManagerGetExtendedState();
    LM_DEBUG(("LM: IndicateState [%x]\n", state));

    if(state == sink_ext_state_inConfigMode)
    {
        LED_GDATA.gConfigMode = TRUE;
        LedManagerSetLedEnabled(TRUE);
    }

    /* Determine what the LED indication status should be and set the associated flags to be
    processed in the next step for a proper LED indication handling mechanism */

    if(powerManagerIsVbatLow())
    {
        /* In low battery warning state */
        LM_DEBUG(("LM: low battery\n"));
        if (ledManagerGetExtendedStatePatternIndex(sink_ext_state_lowBattery, &lPattern, &lPatternIndex))
        {
            LM_DEBUG(("LM : Low Battery LED Indication Being Processed\n IS[%x]\n", state));
            displayLedIndication = TRUE;
        }
    }

    if (!displayLedIndication)
    {
        /* Either not in low battery warning state, or there is no pattern set for it. */
        if (ledManagerGetExtendedStatePatternIndex(state, &lPattern, &lPatternIndex))
        {
            LM_DEBUG(("LM : State LED Indication Being Processed\n IS[%x]\n", state));
            displayLedIndication = TRUE;
        }
        /* Limbo hardcoded as no LED to display, display default pattern otherwise */
        else if(state != sink_ext_state_limbo && ledManagerGetExtendedStatePatternIndex(sink_ext_state_Default, &lPattern, &lPatternIndex))
        {
            LM_DEBUG(("LM : No pattern found for state\n IS[%x]\nDisplaying default pattern\n IS[%x]\n", state, lPatternIndex));
            displayLedIndication = TRUE;
        }
    }

    /* Now process the flags previously set and display the associated LED pattern */
    if(displayLedIndication)
    {
        /*if there is a pattern associated with the state and not disabled, indicate it*/
        LED_GDATA.gStateCanOverideDisable = lPattern.OverideDisable;

        /* only indicate if LEDs are enabled*/
        if (ledManagerCanPlayPattern(&lPattern))
        {
            LM_DEBUG(("LM : State LED Indication can play pattern\n IS[%x]\n", state));

            if (   (LedsActiveLed(lPattern.LED_A)->Type != IT_EventIndication)
                && (LedsActiveLed(lPattern.LED_B)->Type != IT_EventIndication))
            {
               /*Indicate the LED Pattern of Event/State*/
               ledsIndicateLedsPattern(&lPattern, lPatternIndex, IT_StateIndication, state);
            }
        }
        else /* No Led Indication to be processed due to ledManagerCanPlayPattern(lPattern) returning false */
        {
            LM_DEBUG(("LM: NoStCfg[%x]\n", state));
            LedsIndicateNoState();
        }
    }
    else /* No Led Indication to be processed due to no associated pattern having been found to display */
    {
        LM_DEBUG(("LM: DIS NoStCfg[%x]\n", state));
        LedsIndicateNoState();
    }
}

/****************************************************************************
NAME	
	LedManagerDisableLEDS

DESCRIPTION
    Disable LED indications
RETURNS
	void
    
*/
void LedManagerDisableLEDS ( void )
{
    LM_DEBUG(("LM Disable LEDS\n")) ;

    /*turn off all current LED Indications if not overidden by state or filter */
    if (!LED_GDATA.gStateCanOverideDisable && !LedActiveFiltersCanOverideDisable())
    {
        LedsIndicateNoState();
    }    
    
    LedManagerSetLedEnabled(FALSE);
}

/****************************************************************************
NAME	
	LedManagerEnableLEDS

DESCRIPTION
    Enable LED indications
RETURNS
	void
    
*/
void LedManagerEnableLEDS ( void )
{
    LM_DEBUG(("LM Enable LEDS\n")) ;
    
    LedManagerSetLedEnabled(TRUE);
         
    LedManagerIndicateExtendedState();
}


/****************************************************************************
NAME	
	LedManagerToggleLEDS

DESCRIPTION
    Toggle Enable / Disable LED indications
RETURNS
	void
    
*/
void LedManagerToggleLEDS ( void ) 
{
    if ( LedManagerIsEnabled() )
    {
        MessageSend (&theSink.task , EventUsrLedsOff , 0) ;
    }
    else
    {
        MessageSend (&theSink.task , EventUsrLedsOn , 0) ;
    }
}

/****************************************************************************
NAME	
	LedManagerResetLEDIndications

DESCRIPTION
    Resets the LED Indications and reverts to state indications
	Sets the Flag to allow the Next Event to interrupt the current LED Indication
    Used if you have a permanent LED event indication that you now want to interrupt
RETURNS
	void
    
*/
void LedManagerResetLEDIndications ( void )
{    
    LedsResetAllLeds();
    
    LED_GDATA.gCurrentlyIndicatingEvent = FALSE;
    
    LedManagerIndicateExtendedState();
}

/****************************************************************************
NAME	
	LedManagerResetStateIndNumRepeatsComplete

DESCRIPTION
    Resets the LED Number of Repeats complete for the current state indication
       This allows the time of the led indication to be reset every time an event 
       occurs.
RETURNS
	void
    
*/
void LedManagerResetStateIndNumRepeatsComplete( void ) 
{
    uint8 lPatternIndex;
    LEDPattern_t lPattern;

    /* does pattern exist for this state */
    if (ledManagerGetExtendedStatePatternIndex(stateManagerGetExtendedState(), &lPattern, &lPatternIndex))
    {
        LEDActivity_t * lLED = LedsActiveLed(lPattern.LED_A);
        if (lLED)
        {
            /*reset num repeats complete to 0*/
            lLED->NumRepeatsComplete = 0 ;
        }    
    }
}

/****************************************************************************
NAME	
	LedManagerCheckTimeoutState

DESCRIPTION
    checks the led timeout state and resets it if required, this function is called from
    an event or volume button press to re-enable led indications as and when required
    to do so 
RETURNS
	void
    
*/
void LedManagerCheckTimeoutState( void )
{
    /*handles the LED event timeouts - restarts state indications if we have had a user generated event only*/
    if (LED_GDATA.gLEDSStateTimeout)
    {   
        /* send message that can be used to show an led pattern when led's are re-enabled following a timeout */
        MessageSend( &theSink.task, EventSysResetLEDTimeout, 0);
    }
    else
    {
        /*reset the current number of repeats complete - i.e restart the timer so that the leds will disable after
          the correct time*/
        LedManagerResetStateIndNumRepeatsComplete  ( ) ;
    }
}


/****************************************************************************
NAME	
	LedManagerForceDisable

DESCRIPTION
    Set disable TRUE to force all LEDs off, call again with disable FALSE
    to restore LEDs to the correct state (including filters).

RETURNS
	void
*/
void LedManagerForceDisable( bool disable )
{
    LM_DEBUG(("LM: LedManagerForceDisable disable %u\n", disable));

    if((disable != (bool)LED_GDATA.gLEDSSuspend) && (LED_GDATA.gConfigMode != TRUE))
    {
        if(disable)
        {
            /* Suspend LED indications */
            LedsResetAllLeds() ;
            LED_GDATA.gCurrentlyIndicatingEvent = FALSE ;
            LED_GDATA.gLEDSSuspend = TRUE;
            LedsEnableFilterOverrides(FALSE);
        }
        else
        {
            /* Resume LED indications */
            LED_GDATA.gLEDSSuspend = FALSE;
            LedManagerIndicateExtendedState();
            LedsEnableFilterOverrides(TRUE);
        }
    }
}

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
bool LedManagerIsEnabled(void)
{
    return LED_GDATA.gLEDSEnabled;
}


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

bool LedManagerGetStateTimeout(void)
{
     return LED_GDATA.gLEDSStateTimeout;
}

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
void LedManagerSetStateTimeout(bool value)
{
     LED_GDATA.gLEDSStateTimeout = value;
}


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
bool LedManagerQueueLedEvents(void)
{
    sink_led_readonly_config_def_t *read_config = NULL;
    bool value = FALSE;
    
    if(configManagerGetReadOnlyConfig(SINK_LED_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
        value =  read_config->QueueLEDEvents;
        configManagerReleaseConfig(SINK_LED_READONLY_CONFIG_BLK_ID);
    }

    LM_DEBUG(("LM: LedManagerQueueLedEvents %d \n", value));

    return value;
}


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
bool LedManagerOverideFilterOn(void)
{
    sink_led_readonly_config_def_t *read_config = NULL;
    bool value = FALSE;
    
    if(configManagerGetReadOnlyConfig(SINK_LED_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
        value =  read_config->OverideFilterPermanentlyOn;
        configManagerReleaseConfig(SINK_LED_READONLY_CONFIG_BLK_ID);
    }

    LM_DEBUG(("LM: LedManagerOverideFilterOn %d \n", value));
    return value;
}

/****************************************************************************
NAME
    LedManagerLedEnablePioValue

DESCRIPTION
    Returns the configured PIO for Led Enabling.

RETURNS
    Led Enable pio value
*/
uint8 LedManagerLedEnablePioValue(void)
{
    sink_led_readonly_config_def_t *read_config = NULL;
    uint8 led_enable_pio = 0x0;
    
    if(configManagerGetReadOnlyConfig(SINK_LED_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
        led_enable_pio =  read_config->LedEnablePIO;
        configManagerReleaseConfig(SINK_LED_READONLY_CONFIG_BLK_ID);
    }

    LM_DEBUG(("LM: LedManagerLedEnablePioValue %d \n", led_enable_pio));
    return led_enable_pio;
}


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
bool LedManagerChargerTermLedOveride(void)
{
    sink_led_readonly_config_def_t *read_config = NULL;
    bool value = FALSE;
    
    if(configManagerGetReadOnlyConfig(SINK_LED_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
        value =  read_config->ChargerTerminationLEDOveride;
        configManagerReleaseConfig(SINK_LED_READONLY_CONFIG_BLK_ID);
    }

    LM_DEBUG(("LM: LedManagerChargerTermLedOveride %d \n", value));
    return value;
}

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
bool LedManagerResetLedState(void)
{
    sink_led_readonly_config_def_t *read_config = NULL;
    bool value = FALSE;
    
    if(configManagerGetReadOnlyConfig(SINK_LED_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
        value =  read_config->ResetLEDEnableStateAfterReset;
        configManagerReleaseConfig(SINK_LED_READONLY_CONFIG_BLK_ID);
    }

    LM_DEBUG(("LM: LedManagerResetLedState %d \n", value));
    return value;
}

/****************************************************************************
NAME
    LedManagerGetTimeMultiplier

DESCRIPTION
    Get the time multiplier value for LED

RETURNS
    uint16
*/
uint16 LedManagerGetTimeMultiplier(void)
{    
    sink_led_readonly_config_def_t *read_config = NULL;
    uint16 value = 0x0;
  
    if(configManagerGetReadOnlyConfig(SINK_LED_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
        value =  read_config->LedTimeMultiplier;
        configManagerReleaseConfig(SINK_LED_READONLY_CONFIG_BLK_ID);
    }
 
    LM_DEBUG(("LM: LedManagerGetTimeMultiplier %d \n", value));
    return value;
}

/****************************************************************************
NAME
    LedManagerGetEventPattern

DESCRIPTION
    Get pattern based on events.
    
PARAMS
    event          Event for which pattern is requested
    pEventPattern  Event Pattern is retrieved by caller

RETURNS
    Bool 
    
*/
bool LedManagerGetEventPattern(sinkEvents_t event, LEDPattern_t *pEventPattern)
{
    uint8 idx;

    if (!ledManagerGetEventPatternIndex(event, pEventPattern, &idx))
        return FALSE;

    return TRUE;
}

/****************************************************************************
NAME
    LedManagerSetEventPattern

DESCRIPTION
    Set pattern for the given event. Only the exisiting patterns will get
    set; so if the event does not have a pattern it will not be added to
    the config.

PARAMS
    event          Event for which pattern is to be set.
    pEventPattern  Event Pattern to set.

RETURNS
    Bool 
    
*/
bool LedManagerSetEventPattern(sinkEvents_t event, const LEDPattern_t *pEventPattern)
{
    uint16 i;
    uint16 size;
    uint16 num_patterns;
    sink_led_event_pattern_config_def_t *config;

    size = configManagerGetWriteableConfig(SINK_LED_EVENT_PATTERN_CONFIG_BLK_ID, (void **)&config, 0);
    if (size)
    {
        num_patterns = ((size * sizeof(uint16)) / sizeof(config->pEventPatterns[0]));

        for (i = 0; i < num_patterns; i++)
        {
            if (config->pEventPatterns[i].event == event)
            {
                break;
            }
        }
    }
    else
        return FALSE;
    
    /* If a new pattern needs to be added, re-open the config block with a larger size. */
    if (i == num_patterns)
    {
        num_patterns++;

        configManagerReleaseConfig(SINK_LED_EVENT_PATTERN_CONFIG_BLK_ID);
        size = configManagerGetWriteableConfig(SINK_LED_EVENT_PATTERN_CONFIG_BLK_ID, 
                                               (void **)&config, 
                                               PS_SIZE_ADJ(num_patterns * sizeof(config->pEventPatterns[0])));
        if (!size)
            return FALSE;
    }

    config->pEventPatterns[i].event = event;
    config->pEventPatterns[i].pattern = *pEventPattern;
    configManagerUpdateWriteableConfig(SINK_LED_EVENT_PATTERN_CONFIG_BLK_ID);

    return TRUE;
}

/****************************************************************************
NAME
    LedManagerGetSessionLedData

DESCRIPTION
    Get Led global data from Session Data
    
PARAMS
    void
    
RETURNS
    void 

*/
bool LedManagerGetSessionLedData(void)
{
    bool led_enabled = FALSE;
    sink_led_session_config_def_t *data;
    LM_DEBUG(("LED:LedManagerGetSessionLedData()\n"));

    if(configManagerGetReadOnlyConfig(SINK_LED_SESSION_CONFIG_BLK_ID, (const void **)&data))
    {
        led_enabled = data->gLEDSEnabled;
        configManagerReleaseConfig(SINK_LED_SESSION_CONFIG_BLK_ID);
    }
    LM_DEBUG(("LED: led_enabled = %d\n", led_enabled));
    return (led_enabled) ? TRUE : FALSE;
}


/****************************************************************************
NAME
    LedManagerSetSessionLedData

DESCRIPTION
    Set session data from Led global data
    
PARAMS
    void

RETURNS
    void 
*/
void LedManagerSetSessionLedData(void)
{
    sink_led_session_config_def_t *data;
    LM_DEBUG(("LED:LedManagerSetSessionLedData()\n"));

    if (configManagerGetWriteableConfig(SINK_LED_SESSION_CONFIG_BLK_ID, (void **)&data, 0))
    {
        data->gLEDSEnabled = LED_GDATA.gLEDSEnabled;
        configManagerUpdateWriteableConfig(SINK_LED_SESSION_CONFIG_BLK_ID);
    }
}

bool LedManagerGetExtendedStatePattern(sink_extended_state_t state, LEDPattern_t *pattern)
{
    uint8 idx;

    if (!ledManagerGetExtendedStatePatternIndex(state, pattern, &idx))
        return FALSE;

    return TRUE;
}

bool LedManagerSetExtendedStatePattern(sink_extended_state_t state, const LEDPattern_t *pattern)
{
    uint16 size;
    uint8 i;
    uint8 num_patterns;
    sink_led_state_pattern_config_def_t *config_data;

    if (state >= SINK_NUM_EXTENDED_STATES)
        return FALSE;

    /* Attempt to find the state if it already exists */
    num_patterns = LED_GDATA.gStatePatternsAllocated;
    for (i = 0; i < num_patterns; i++)
    {
        if (LED_GDATA.pStatePatterns[i].state == state)
        {
            break;
        }
    }

    /* If the state wasn't found, attempt to add it to the array. */
    if (i == num_patterns)
        num_patterns++;

    if (num_patterns >= SINK_NUM_EXTENDED_STATES)
        return FALSE;

    /* Get writeable config block containing Led event patterns. */
    size = configManagerGetWriteableConfig(SINK_LED_STATE_PATTERN_CONFIG_BLK_ID, 
                                           (void **)&config_data, 
                                           PS_SIZE_ADJ(num_patterns * sizeof(config_data->pStatePatterns[0])));
    if (size)
    {
        /* Update the writeable config data. */
        config_data->pStatePatterns[i].state = state;
        config_data->pStatePatterns[i].pattern = *pattern;
        configManagerUpdateWriteableConfig(SINK_LED_STATE_PATTERN_CONFIG_BLK_ID);

        /* Update the working copy of the config data. */
        LED_GDATA.pStatePatterns[i].state = state;
        LED_GDATA.pStatePatterns[i].pattern = *pattern;
        LED_GDATA.gStatePatternsAllocated = num_patterns;

        return TRUE;
    }

    return FALSE;
}

bool LedManagerGetEventFilter(uint16 index, LEDFilter_t *filter)
{
    if (index >= LED_GDATA.gLMNumFiltersUsed)
        return FALSE;

    *filter = LED_GDATA.pEventFilters[index];

    return TRUE;
}

bool LedManagerSetEventFilter(uint16 index, LEDFilter_t *filter)
{
    if (index > LED_GDATA.gLMNumFiltersUsed)
    {
        /* Index is more than 1 larger than current array size, so it is invalid. */
        return FALSE;
    }
    else if (index == LED_GDATA.gLMNumFiltersUsed)
    {
        /* Allow for a new filter to be added at the end of the array. */
        LED_GDATA.gLMNumFiltersUsed++;
    }

    /* Close the open handle to the filter array in case it is read-only.
       Re-open as writeable and update the values of the matching filter.
       Store the new values in config_store. 
       Leave the handle to the config data open as it used directly by
         the LED code. */
    configManagerReleaseConfig(SINK_LED_EVENT_FILTER_CONFIG_BLK_ID);
    configManagerGetWriteableConfig(SINK_LED_EVENT_FILTER_CONFIG_BLK_ID, 
                                    (void **)&LED_GDATA.pEventFilters, 
                                    PS_SIZE_ADJ(LED_GDATA.gLMNumFiltersUsed * sizeof(LED_GDATA.pEventFilters[0])));
    LED_GDATA.pEventFilters[index] = *filter;
    ConfigStoreWriteConfig(SINK_LED_EVENT_FILTER_CONFIG_BLK_ID);

    return TRUE;
}

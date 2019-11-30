/****************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_leddata.h
    
DESCRIPTION
    data structures  /defines for use with the LED module
    
*/
#ifndef SINK_LED_DATA_H
#define SINK_LED_DATA_H

#include "sink_events.h"
#include "sink_extendedstates.h"
#include "sink_states.h"
#include "sink_led_config_def.h"


/****************************************************************************
Types
*/
    /* before increasing these note they effect memory usage and 
       are carefully optimised for VM slot availability
       LED state masks will also need enlarging */

/*the number of LEDS (including pin outputs) that we support*/
#define SINK_NUM_LEDS (16)

/* Max Queue size for Led events*/
#define MAX_LED_EVENT_QUEUE_SIZE (4)

/* the number of HW LED pins we support */
/* The define differs by chip. If we don't see LED_ID_N, assume 3 LEDs */
#ifndef LED_ID_N
# define SINK_NUM_HW_LEDS (3)
#else
# define SINK_NUM_HW_LEDS (LED_ID_N)
#endif

#define NO_STATE_OR_EVENT 0xff

typedef enum LEDSpeedActionTag
{
    SPEED_MULTIPLY = 0,
    SPEED_DIVIDE  
}LEDSpeedActionTag ;

typedef enum LEDColTag
{
    LED_COL_EITHER ,
    LED_COL_LED_A ,
    LED_COL_LED_B ,
    LED_COL_LED_ALT ,    /*Alternate the colours*/
    LED_COL_LED_BOTH    /*use Both LEDS*/
}LEDColour_t ;

/* The order of values in the LEDFilterType_t shouldn't be changed */
typedef enum LEDFilterTypeTag
{
    DISABLED = 0,
    CANCEL ,
    SPEED ,
    OVERRIDE, 
    COLOUR ,
    FOLLOW
}LEDFilterType_t ;

typedef enum LEDPatternForTag
{
    LED_STATE = 1,
    LED_EVENT ,
    LED_FILTER
}LEDPatternFor_t ;

typedef enum IndicationTypeTag
{
    IT_Undefined = 0 ,
    IT_StateIndication,
    IT_EventIndication    
    
}IndicationType_t ;


    /*the information required for a LED to be updated*/
typedef struct LEDActivityTag
{  
    unsigned         Index:7; /*what this led is displaying*/
    unsigned         NumFlashesComplete:8 ; /*how far through the pattern we currently are*/        
    unsigned         OnOrOff:1 ;
    
    unsigned         FilterIndex:4 ;/*the filter curently attached to this LED (0-15)*/    
    unsigned         Type:2 ; /*what this LED is displaying*/
    unsigned         NumRepeatsComplete:10;
        /*dimming*/
    unsigned         DimState:7  ; /*how far through the dim pattern we are*/
    unsigned         DimDir:1    ; /*which direction we are going*/
    unsigned         DimTime:8   ;
    
}LEDActivity_t ;


    /*the event message sent on completion of an event */
typedef struct 
{
    uint16 Event;  
    bool PatternCompleted;
    
} LMEndMessage_t;

/* LED information structure */
typedef struct LEDInfo
{
    unsigned led:8; /* LED and PIOs are uint16, though we aren't going to have more than 256 for each */
    unsigned OnOff:1; /* On Off state */
    unsigned is_led:1; /* 1:LED, 0:PIO */
    unsigned unused:6;
    
    unsigned current_dim:12; /* current dim value of the LED if it is an LED */
    unsigned unused2:4;
}LEDInfo_t;

/* pPIO information structure */
typedef struct LEDpPIOInfo
{
    LEDInfo_t led1;
    LEDInfo_t led2;
    bool is_ledpair; /* TRUE if pPIO is a pair of pPIOs */
}LEDpPIOInfo_t;

/* LED State information structure */
typedef struct LEDState
{
    unsigned  current_dim:12; /* current dim value to compare with a new LED pattern or ramp direction */
    unsigned  OnOff:1; /* State of the LED, whether it is on off (or ramping up or down) */
    unsigned  unused:3;
}LEDState_t;

typedef struct
{
    TaskData                task;
    LEDpPIOInfo_t           gpPIOInfo; /* structure to be used by functions that require translation of pPIO to HW */

    uint8                   gLedMapping[SINK_NUM_LEDS]; /* Which LED, or PIO each active LED matches */
    LEDActivity_t           gLedActivity[SINK_NUM_LEDS+1]; /* the array of LED Activities*/
    LEDState_t              gLEDState[SINK_NUM_HW_LEDS]; /* array for LED hardware state information */
    sinkEvents_t            gLedEventQueue[MAX_LED_EVENT_QUEUE_SIZE];

    LEDStatePattern_t*      pStatePatterns; /* working copy of LED state patterns */
    LEDFilter_t*            pEventFilters; /* pointer to array of LED Filter patterns */
    PioTriColLeds_t         gTriColLeds;

    unsigned                gStatePatternsAllocated:5;
    unsigned                unused:5; 
    unsigned                gLMNumFiltersUsed:5;
    unsigned                gLEDSEnabled:1;

    unsigned                gLEDSStateTimeout:1; /*this is set to true if a state pattern has completed - reset if new event occurs*/
    unsigned                gLEDSSuspend:1;       /*LED indications suspended until this bit is cleared*/
    unsigned                gCurrentlyIndicatingEvent:1; /*if we are currently indicating an event*/
    unsigned                gFollowing:1; /* do we currently have a follower active*/
    unsigned                gStateCanOverideDisable:1;    
    unsigned                gFollowPin:4;    
    unsigned                gTheActiveFiltersH :4; /*Mask of Filters Active - High nibble */
    unsigned                gConfigMode :1; /*This is set to TRUE when config mode LED pattern is displayed*/
    unsigned                unused1:2;

    unsigned                gErrorCode:16; /* 0 if not error otherwise the code of the error that occured */

    unsigned                gTheActiveFiltersL:16; /*Mask of Filters Active - Low word */
}sink_led_global_data_t;
sink_led_global_data_t *LedGetGlobalData(void);

/* Macros for the max number of LED events and filter patterns, capped at 20 each */
#define LM_NUM_FILTER_EVENTS ((uint16)(ledManagerGetNumFilters()>20 ? 20 : ledManagerGetNumFilters()))  

#define LED_GETACTIVEFILTERS() (uint32)(((uint32)LedGetGlobalData()->gTheActiveFiltersH<<16) | LedGetGlobalData()->gTheActiveFiltersL)
#define LED_SETACTIVEFILTERS(x) {LedGetGlobalData()->gTheActiveFiltersH = ((uint32)x>>16) & 0xF ;\
                                 LedGetGlobalData()->gTheActiveFiltersL = x & 0xffff;}

#endif


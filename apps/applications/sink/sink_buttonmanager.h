/*
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/
#ifndef SINK_BUTTON_MANAGER_H
#define SINK_BUTTON_MANAGER_H

#include "sink_events.h"
#include "config_definition.h"
#include "sink_button_config_def.h"
#include "pio_common.h"


typedef enum ButtonsTimeTag
{
    B_INVALID,
    B_SHORT,
    B_LONG,
    B_VERY_LONG,
    B_DOUBLE,
    B_REPEAT,
    B_LOW_TO_HIGH,
    B_HIGH_TO_LOW,
    B_SHORT_SINGLE,
    B_LONG_RELEASE,
    B_VERY_LONG_RELEASE,
    B_VERY_VERY_LONG,
    B_VERY_VERY_LONG_RELEASE,
    B_TRIPLE,
    B_HOLD
} ButtonsTime_t;

typedef enum ButtonEventState
{
    B_EVENT_SENT,       /* A button event has been sent since the last down, or no buttons are down */
    B_NEWLY_PRESSED     /* One or more buttons has just been pressed, no event has been generated yet */
}ButtonEventState_t;

#define ARRAY_COUNT(array) (sizeof((array)) / sizeof((array)[0]))

#define BM_NUM_BUTTON_MATCH_PATTERNS (ARRAY_COUNT(((sink_button_pattern_config_def_t *)0)->button_pattern))
#define BM_NUM_BUTTONS_PER_MATCH_PATTERN (ARRAY_COUNT(((button_pattern_config_type_t *)0)->pattern))

/* Number of event config blocks for Sink App Mode*/
#define BM_NUM_BLOCKS (3)

/* Number of events stored per event config block */
#define BM_EVENTS_PER_CONF_BLOCK (ARRAY_COUNT(((sink_button_eventset_a_config_def_t *)0)->user_event_set_a))

/* Number of button translations per config block */
#define BM_NUM_BUTTON_TRANSLATIONS (ARRAY_COUNT(((sink_button_translation_config_def_t *)0)->button_translation))

#define BM_CAP_SENSORS 6
#define BM_CAP_SENSOR_LOW_SENSITIVITY 500
#define BM_CAP_SENSOR_HIGH_SENSITIVITY 100


#ifdef ENABLE_CAPACITIVE_SENSOR
#define OLD_CAPACITIVE_SENSOR_STATE(btd_ptr) (btd_ptr)->gOldCapState
#else
#define OLD_CAPACITIVE_SENSOR_STATE(btd_ptr) 0
#endif

/* Global data that is shared between sink_buttons and sink_button_manager. */
typedef struct
{
    TaskData    task;
    Task        client;

    pio_common_allbits  pio_debounce_mask;      /* mask used set PIO debounce state, not required for capacitive sensor input assignments */
    pio_common_allbits  gOldPIOState;           /* last PIO state used to merge in with the capacitive sensor data */
    uint32              gPerformInputEdgeCheck; /* bit mask of translated inputs that are configured for edge detect */
    uint32              gPerformInputLevelCheck;/* bit mask of translated inputs that are configured for level detect */
    uint32              gBOldInputState;        /* bit mask of translated pio and capacitive sensor inputs last seen as pressed */
    uint32              gBMultipleState;

    button_debounce_config_type_t *button_debounce_config;        /* The button durations etc*/
    button_config_type_t *button_config;        /* The button durations etc*/

    unsigned    gBTapCount:8;                   /* multiple press counter */
    unsigned    gBTime:7;                       /* ButtonsTime_t (enum ButtonsTimeTag) - The type of press single, double etc. */
    unsigned    gBButtonEventState:1;           /* ButtonEventState_t (enum ButtonEventState) - Records if an event has been send for this press */

    /* The mapping of input values to sink events. */
    event_config_type_t *pButtonEvents[BM_NUM_BLOCKS];

    /* Pattern matching config for buttons with complex patterns. */
    button_pattern_config_type_t *pButtonPatterns;

    /* Counters used for matching input against button patterns. */
    uint16 gButtonMatchProgress[BM_NUM_BUTTON_MATCH_PATTERNS];

#ifdef ENABLE_CAPACITIVE_SENSOR
    uint16      gOldCapState;                   /* last stored state of the capacitive sensor switches */
#endif

    uint16 gNumBAEvents;

    /* Translation between raw input to logical input. */
    button_translation_type_t *pTranslations;

    unsigned                 buttons_locked:1;
    unsigned                 unused:15;
} ButtonsTaskData;


/****************************************************************************
NAME
 buttonManagerInit

DESCRIPTION
 Initialises the button event

RETURNS

*/
void buttonManagerInit(bool useButtonDebounceConfiguration);

/****************************************************************************
NAME
    BMButtonDetected

DESCRIPTION
    Check if an input mask matches any of the button to event mappings.
    If so, send the correspending event to the registered client Task.

RETURNS
    void
*/
void BMButtonDetected(uint32 pButtonMask, ButtonsTime_t pTime);

/****************************************************************************
DESCRIPTION
    Get the button manager task data
*/
ButtonsTaskData *BMGetSinkButtonTask(void);

/****************************************************************************
DESCRIPTION
    Updates the status of buttons locked flag
*/
void BMSetButtonsLocked(bool enable);

/****************************************************************************
DESCRIPTION
    Returns the status of buttons locked flag
*/
bool BMGetButtonsLocked(void);

/****************************************************************************
DESCRIPTION
    Returns the status of go connectable upon button press feature flag
*/
bool BMGetGoConnectableButtonPress(void);

/****************************************************************************
DESCRIPTION
    Returns status of Separate Voice Dail button feature flag
*/
bool BMGetSeparateVDButtons(void);

/****************************************************************************
DESCRIPTION
    Returns status of Separate LNR button feature flag
*/
bool BMGetSeparateLNRButtons(void);

/****************************************************************************
DESCRIPTION
    Calculate the full pio mask for the given event config, including
    VREG and VCHG.
*/
uint32 bmGetButtonEventConfigPioMask(const event_config_type_t *event_config);

#endif

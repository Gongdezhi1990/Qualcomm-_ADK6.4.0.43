/*
Copyright (c) 2005 - 2019 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    This is the button manager for Sink device
    This file provides a configurable wrapper for the button messages and
    converts them into the standard system messages which are passed to the
    main message handler - main.c
*/
#include "sink_main_task.h"
#include "sink_buttonmanager.h"
#include "sink_statemanager.h"
#include "sink_configmanager.h"
#include "sink_buttons.h"
#include "sink_volume.h"
#include "sink_scan.h"
#include "sink_led_manager.h"
#include "sink_config.h"
#include "sink_ba.h"
#include "sink_malloc_debug.h"
#include "sink_debug.h"

#include <stddef.h>
#include <csrtypes.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <config_store.h>


#ifdef DEBUG_BUT_MAN
    #define BM_DEBUG(x) DEBUG(x)

    const char * const gDebugTimeStrings[] = {"Inv" ,
                                              "Short",
                                              "Long" ,
                                              "VLong" ,
                                              "Double" ,
                                              "Rpt" ,
                                              "LToH" ,
                                              "HToL" ,
                                              "ShSingle",
                                              "Long Release",
                                              "VLong Release",
                                              "V V Long" ,
                                              "VV Long Release" ,
                                              "Triple",
                                              "Hold" } ;

#else
    #define BM_DEBUG(x)
#endif

static ButtonsTaskData *gButtonData = NULL;

#define BUTTON_GDATA gButtonData

/****************************************************************************
VARIABLES
*/

#define BM_NUM_CONFIGURABLE_EVENTS (BM_EVENTS_PER_BLOCK * bmGetNumBlocks())

/*
 LOCAL FUNCTION PROTOTYPES
 */
static void bmCheckForButtonMatch ( uint32 pButtonMask , ButtonsTime_t  pDuration  )  ;

static void bmCheckForButtonPatternMatch ( uint32 pButtonMask ) ;

static void bmInitPatternMapping(void);

static void bmInitEventMapping(void);

static void bmInitButtonConfig(void);

static void bmCheckButtonsAfterReadingConfig(bool useButtonDebounceConfiguration);
static bool BMGetIgnoreButtonPressAfterLedEnable(void);

static bool bmIsEventMaskInButtonMask(uint32 event_mask, uint32 button_mask);
static bool bmIsEventMaskEqualButtonMask(uint32 event_mask, uint32 button_mask);
static bool bmIsEdge(ButtonsTime_t button_duration);
static bool bmIsButtonInEvent(event_config_type_t *event_config, uint32 button_mask, ButtonsTime_t button_duration);

/*************************************************************************
NAME
    bmGetNumBlocks

DESCRIPTION
    Function to get Number of User Events Blocks configured.

RETURNS
    uin16: Number of Blocks configured

**************************************************************************/
static uint16 bmGetNumBlocks(void)
{
    return BM_NUM_BLOCKS;
}

/*************************************************************************
NAME
    bmGetNumEventsPerConfBlock

DESCRIPTION
    Function to get Number of User Events configured per Block.

RETURNS
    uin16: Number of Blocks configured

**************************************************************************/
static uint16 bmGetNumEventsPerConfBlock(void)
{
    return BM_EVENTS_PER_CONF_BLOCK;
}

/*************************************************************************
NAME
    bmInitButtonTranslation

DESCRIPTION
    Function to get Button translation config data.

RETURNS
    uin16: size, if 0 then there is an error in read, else success

**************************************************************************/
static void bmInitButtonTranslation(void)
{
    if (configManagerGetReadOnlyConfig(SINK_BUTTON_TRANSLATION_CONFIG_BLK_ID, (const void **)&BUTTON_GDATA->pTranslations))
    {
        /* Do not release the config block on purpose. The poitner is used at runtime. */
    }
    else
    {
        BM_DEBUG(("BM: bmGetButtonTranslation() failed to retrieve button translation\n")) ;
    }
}

/*************************************************************************
NAME
    bmInitButtonConfig

DESCRIPTION
    Function to get Button config data.

RETURNS
    uin16: size, if 0 then there is an error in read, else success

**************************************************************************/
static void bmSetupButtonConfig(void)
{
    sink_button_config_def_t *sink_button_config = NULL;

    if(configManagerGetReadOnlyConfig(SINK_BUTTON_CONFIG_BLK_ID, (const void **)&sink_button_config))
    {
        memcpy(BUTTON_GDATA->button_debounce_config, &sink_button_config->button_debounce_config, (sizeof(button_debounce_config_type_t)));
        memcpy(BUTTON_GDATA->button_config, &sink_button_config->button_config, (sizeof(button_config_type_t)));
        configManagerReleaseConfig(SINK_BUTTON_CONFIG_BLK_ID);
    }
    else
    {
        BM_DEBUG(("BM: bmGetButtonTranslation() failed to retrieve button translation\n")) ;
    }
}

/****************************************************************************
NAME
    bmInitButtonConfig
*/
static void bmInitButtonConfig(void)
{
    /* Read button translation configuration */
    bmInitButtonTranslation();

    /* Read button configuration and populate global data*/
    bmSetupButtonConfig();

    /* Set up Event mapping from config */
    bmInitEventMapping();

    /* Set up Pattern mapping from config */
    bmInitPatternMapping();
}

/****************************************************************************
NAME
 bmCheckButtonLock

DESCRIPTION
 function to chyeck if a button press should be ignored because of
 the button lock feature.  Returns TRUE if the button event should be blocked.

RETURNS

    void
*/
static bool bmCheckButtonLock ( MessageId id )
{
    /* ignore button lock in a call state */
    uint16 statemask = ( (1 << deviceOutgoingCallEstablish) |
                         (1 << deviceIncomingCallEstablish) |
                         (1 << deviceActiveCallSCO) |
                         (1 << deviceThreeWayCallWaiting) |
                         (1 << deviceThreeWayCallOnHold) |
                         (1 << deviceThreeWayMulticall) |
                         (1 << deviceIncomingCallOnHold) );

    sinkState state = stateManagerGetState ();

    /* check if button lock is enabled or in a call state where the lock should be ignored */
    if((!BMGetButtonsLocked()) || ( statemask & (1 << state) ))
    {
        BM_DEBUG(("BM : Button Lock status[%x][%x][%x]\n" , BMGetButtonsLocked(), statemask, state)) ;
        return FALSE;
    }

    /* Buttons are locked and not in a call state, check this isn't a charger event or an attempt to
       unlock the buttons */
    switch(id)
    {
        case EventUsrButtonLockingToggle:
        case EventUsrButtonLockingOff:
        case EventUsrChargerConnected:
        case EventUsrChargerDisconnected:
        case EventUsrQuickChargeStatInterrupt:
            BM_DEBUG(("BM : Event bypasses button lock [%x]\n" , id)) ;
            return FALSE;
        default:
            BM_DEBUG(("BM : Button Locked [%x]\n" , id)) ;
            return TRUE;
    }

}

static bool bmIsEventMaskInButtonMask(uint32 event_mask, uint32 button_mask)
{
    return ((event_mask & button_mask) == event_mask);
}

static bool bmIsEventMaskEqualButtonMask(uint32 event_mask, uint32 button_mask)
{
    return (event_mask == button_mask);
}

static bool bmIsEdge(ButtonsTime_t button_duration)
{
    return ((button_duration == B_LOW_TO_HIGH) || (button_duration == B_HIGH_TO_LOW));
}

/****************************************************************************
NAME
 bmIsButtonInEvent

DESCRIPTION
 function to check if a button is an event based on mask and duration

RETURNS
    TRUE if a duration and mask match appropriately, FALSE otherwise
*/
static bool bmIsButtonInEvent(event_config_type_t *event_config, uint32 button_mask, ButtonsTime_t button_duration)
{
    uint32 event_mask;
    ButtonsTime_t event_duration = event_config->type;

    if (event_duration != button_duration)
        return FALSE;

    event_mask = bmGetButtonEventConfigPioMask(event_config);

    if (bmIsEdge(button_duration))
        return bmIsEventMaskInButtonMask(event_mask, button_mask);

    return bmIsEventMaskEqualButtonMask(event_mask, button_mask);
}

/****************************************************************************
NAME
 bmCheckForButtonMatch

DESCRIPTION
 function to check a button for a match in the button events map - sends a message
    to a connected task with the corresponding event

RETURNS

    void
*/
static void bmCheckForButtonMatch ( uint32 pButtonMask , ButtonsTime_t  pDuration )
{
    uint16 lBlockIndex = 0 ;
    uint16 lNumBlocks = 0;
    uint16 lEvIndex = 0 ;
    uint16 lNumEvents = 0;

    lNumBlocks = bmGetNumBlocks();
    lNumEvents = bmGetNumEventsPerConfBlock();

        /*each block*/
    for (lBlockIndex = 0 ; lBlockIndex <  lNumBlocks; lBlockIndex++)
    {       /*Each Entry*/
        for (lEvIndex = 0 ; lEvIndex < lNumEvents ; lEvIndex ++)
        {
            event_config_type_t * event_config = &BUTTON_GDATA->pButtonEvents[lBlockIndex][lEvIndex];

            /*if the event is valid*/
            if (event_config)
            {
                if (bmIsButtonInEvent(event_config, pButtonMask, pDuration))
                {
                    if ((event_config->state_mask) & (1 << stateManagerGetState()))
                    {
                        BM_DEBUG(("BM : State Match [%lx][%x][%x]\n" , pButtonMask, event_config->user_event, event_config->state_mask));

                        /* if the led's are disabled and the feature bit to ignore a button press when the
                           led's are enabled is true then ignore this button press and just re-enable the leds */
                        if(LedManagerGetStateTimeout() && BMGetIgnoreButtonPressAfterLedEnable())
                        {
                            LedManagerCheckTimeoutState();
                        }
                        /* all other cases the button generated event is processed as normal */
                        else
                        {
                            /* Only indicate the event if the buttons are unlocked or this is actually the user
                               trying to unlock them */
                            if(!bmCheckButtonLock(event_config->user_event + EVENTS_MESSAGE_BASE))
                            {
                                /*we have fully matched an event....so tell the main task about it*/
                                MessageSend(BUTTON_GDATA->client, (event_config->user_event + EVENTS_MESSAGE_BASE), 0);
                            }
                            else
                            {
                                /* Button has been blocked because the buttons are locked, event is sent so the block event can be indicated */
                                MessageSend(BUTTON_GDATA->client, EventSysButtonBlockedByLock, 0);
                            }
                        }
                    }
                }
            }
        }
    }
}

/****************************************************************************
DESCRIPTION
    check to see if a button pattern has been matched
*/
static void bmCheckForButtonPatternMatch(uint32 button_mask)
{
    uint16 i = 0;
    uint32 pattern_mask;
    button_pattern_config_type_t *button_pattern;

    BM_DEBUG(("BM: Pat[%lx]\n", button_mask));

    for (i = 0; i < BM_NUM_BUTTON_MATCH_PATTERNS ; i++)
    {
        BM_DEBUG(("BM: Check Match - progress= [%d]\n", BUTTON_GDATA->gButtonMatchProgress[i]));

        button_pattern = &BUTTON_GDATA->pButtonPatterns[i];
        pattern_mask = (((uint32)1) << (button_pattern->pattern[BUTTON_GDATA->gButtonMatchProgress[i]] - 1));

        if (pattern_mask == button_mask)
        {
            BUTTON_GDATA->gButtonMatchProgress[i]++;

            BM_DEBUG(("BM: Pat Prog[%d][%x]\n", i, BUTTON_GDATA->gButtonMatchProgress[i]));

            if (button_pattern->pattern[BUTTON_GDATA->gButtonMatchProgress[i]] == 0)
            {
                /* We reached the end of the list of button patterns to match,
                   so we have matched the full pattern. */
                BM_DEBUG(("BM: Pat Match[%d] Ev[%x]\n", i, button_pattern->event));

                BUTTON_GDATA->gButtonMatchProgress[i] = 0;
                MessageSend(BUTTON_GDATA->client, button_pattern->event, 0);
                break;
            }
        }
        else
        {
            BUTTON_GDATA->gButtonMatchProgress[i] = 0;

            /* Special case = if the last button pressed was the same as the first button. */
            pattern_mask = (((uint32)1) << (button_pattern->pattern[0] - 1));
            if (pattern_mask == button_mask)
            {
                BUTTON_GDATA->gButtonMatchProgress[i] = 1;
            }
        }
    }
}

/****************************************************************************
DESCRIPTION
 Adds a button pattern to match against

RETURNS
 void
*/
static void bmInitPatternMapping(void)
{
    configManagerGetReadOnlyConfig(SINK_BUTTON_PATTERN_CONFIG_BLK_ID, (const void **)&BUTTON_GDATA->pButtonPatterns);

    /* Do not release the config block. It is used directly when checking for button presses. */
}

/****************************************************************************
DESCRIPTION
    Build up the bitmasks for which inputs need edge or level checking and
    which pio bits need to be debounced by parsing all the button event
    config data.

PARAMS
    None

RETURNS
    void
*/
static void bmInitInputCheckMasks(void)
{
    uint16 block;
    uint16 event;
    event_config_type_t *button_events;
    event_config_type_t *event_config;
    uint32 mask;
    uint16 num_blocks = 0;
    uint16 lNumEvents = 0;

    num_blocks = bmGetNumBlocks();
    lNumEvents = bmGetNumEventsPerConfBlock();

    for (block = 0; block < num_blocks; block++)
    {
        button_events = BUTTON_GDATA->pButtonEvents[block];

        /* pButtonEvents should be valid by the time this is called. */
        if (!button_events)
            Panic();

        for(event = 0; event < lNumEvents; event++)
        {
            event_config = &button_events[event];

            /* check to see if a valid pio mask is present, this includes the upper 2 bits of the state
               info as these are being used for bc5 as vreg enable and charger detect */
            mask = bmGetButtonEventConfigPioMask(event_config);
            if (mask)
            {
                pio_common_allbits temp_mask;
                /* look for edge detect configuration of input and add the input used to the check for edge detect */
                if((event_config->type == B_LOW_TO_HIGH) || (event_config->type == B_HIGH_TO_LOW))
                {
                    /* An edge check */
                    BUTTON_GDATA->gPerformInputEdgeCheck |= mask;
                }
                else
                {
                    /* Otherwise must be a level check */
                    BUTTON_GDATA->gPerformInputLevelCheck |= mask;
                }

                BM_DEBUG(("BM: Add Mapping: Event[%x] Duration[%x] Input Mask[0x%lx] Level Check[0x%lx]\n", event_config->user_event
                                                                                                          , event_config->type
                                                                                                          , mask
                                                                                                          , BUTTON_GDATA->gPerformInputLevelCheck));

                /* translate input into PIO only mask for registering PIO debounce assignments, excluding capacitive touch bits */
                /* create mask of pios being used to set debounce state */
                PioCommonBitsOR(&(BUTTON_GDATA->pio_debounce_mask),
                                   &(BUTTON_GDATA->pio_debounce_mask),
                                   ButtonsTranslateInput(&temp_mask, event_config, FALSE));
            }
        }
    }
}

/****************************************************************************
DESCRIPTION
    Initialise the config for mapping button presses to sink events.

RETURNS
    void
*/
static void bmInitEventMapping(void)
{
    configManagerGetReadOnlyConfig(SINK_BUTTON_EVENTSET_A_CONFIG_BLK_ID, (const void **)&BUTTON_GDATA->pButtonEvents[0]);
    configManagerGetReadOnlyConfig(SINK_BUTTON_EVENTSET_B_CONFIG_BLK_ID, (const void **)&BUTTON_GDATA->pButtonEvents[1]);
    configManagerGetReadOnlyConfig(SINK_BUTTON_EVENTSET_C_CONFIG_BLK_ID, (const void **)&BUTTON_GDATA->pButtonEvents[2]);

    /* Create bitmasks for input checking based on the button event config data. */
    bmInitInputCheckMasks();
}

/****************************************************************************
DESCRIPTION
    Perform an initial read of pios following configuration reading as it is
    possible that pio states may have changed whilst the config was being read
    and now needs checking to see if any relevant events need to be generated.

 RETURNS
    void
*/
static void bmCheckButtonsAfterReadingConfig(bool useButtonDebounceConfiguration)
{
    ButtonsCheckForChangeAfterInit(useButtonDebounceConfiguration);
}

/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************
NAME
    buttonManagerInit
*/
void buttonManagerInit (bool useButtonDebounceConfiguration)
{
    /* Put the buttons task and the button patterns in a single memory block*/
    BUTTON_GDATA = mallocPanic(sizeof(*BUTTON_GDATA));
    memset(BUTTON_GDATA, 0, sizeof(*BUTTON_GDATA));

    BUTTON_GDATA->client = &theSink.task;
    BUTTON_GDATA->button_debounce_config = mallocPanic(sizeof(button_debounce_config_type_t));
    BUTTON_GDATA->button_config = mallocPanic(sizeof(button_config_type_t));
    BUTTON_GDATA->gNumBAEvents = 0;

    /* init the PIO button routines with the Button manager Task data */
    ButtonsInit();
    bmInitButtonConfig();

    ButtonsInitHardware();

    /* perform an initial pio check to see if any pio changes need processing following the completion
    of the configuration ps key reading */
    bmCheckButtonsAfterReadingConfig(useButtonDebounceConfiguration);
}

/****************************************************************************
DESCRIPTION
    get the button manager task data
*/
ButtonsTaskData *BMGetSinkButtonTask(void)
{
    return BUTTON_GDATA;
}

/****************************************************************************
DESCRIPTION
    Updates the status of buttons locked flag
*/
void BMSetButtonsLocked(bool enable)
{
    BUTTON_GDATA->buttons_locked = enable;
}

/****************************************************************************
DESCRIPTION
    Returns the status of buttons locked flag
*/
bool BMGetButtonsLocked(void)
{
    return BUTTON_GDATA->buttons_locked;
}

/****************************************************************************
DESCRIPTION
    Returns the status of go connectable upon button press feature status
*/
bool BMGetGoConnectableButtonPress(void)
{
    sink_button_config_def_t *button_config_data = NULL;
    bool enabled = FALSE;

    if(configManagerGetReadOnlyConfig(SINK_BUTTON_CONFIG_BLK_ID, (const void **)&button_config_data))
    {
        enabled = button_config_data->go_connectable_on_button_press;
        configManagerReleaseConfig(SINK_BUTTON_CONFIG_BLK_ID);
    }
    return enabled;
}

/****************************************************************************
DESCRIPTION
    Returns status of Separate Voice Dail button feature status
*/
bool BMGetSeparateVDButtons(void)
{
    sink_button_config_def_t *button_config_data = NULL;
    bool enabled = FALSE;

    if(configManagerGetReadOnlyConfig(SINK_BUTTON_CONFIG_BLK_ID, (const void **)&button_config_data))
    {
        enabled = button_config_data->separate_vd_buttons;
        configManagerReleaseConfig(SINK_BUTTON_CONFIG_BLK_ID);
    }
    return enabled;
}

/****************************************************************************
DESCRIPTION
    Returns status of Separate LNR button feature status
*/
bool BMGetSeparateLNRButtons(void)
{
    sink_button_config_def_t *button_config_data = NULL;
    bool enabled = FALSE;

    if(configManagerGetReadOnlyConfig(SINK_BUTTON_CONFIG_BLK_ID, (const void **)&button_config_data))
    {
        enabled = button_config_data->separate_lnr_buttons;
        configManagerReleaseConfig(SINK_BUTTON_CONFIG_BLK_ID);
    }
    return enabled;
}

/****************************************************************************
DESCRIPTION
    Returns IgnoreButtonPressAfterLedEnable features status.
*/
static bool BMGetIgnoreButtonPressAfterLedEnable(void)
{
    sink_button_config_def_t *button_config_data = NULL;
    bool enabled = FALSE;

    if(configManagerGetReadOnlyConfig(SINK_BUTTON_CONFIG_BLK_ID, (const void **)&button_config_data))
    {
        enabled = button_config_data->IgnoreButtonPressAfterLedEnable;
        configManagerReleaseConfig(SINK_BUTTON_CONFIG_BLK_ID);
    }
    return enabled;
}

/****************************************************************************
NAME
    BMButtonDetected

DESCRIPTION
    Check if an input mask matches any of the button to event mappings.
    If so, send the correspending event to the registered client Task.

RETURNS
    void

*/
void BMButtonDetected(uint32 pButtonMask, ButtonsTime_t pTime)
{
    BM_DEBUG(("BM : But [%lx] [%s]\n", pButtonMask, gDebugTimeStrings[pTime]));

    bmCheckForButtonMatch(pButtonMask, pTime);

    /* Only use regular button presses for the pattern matching to make life simpler. */
    if ((pTime == B_SHORT) || (pTime == B_LONG))
    {
        bmCheckForButtonPatternMatch(pButtonMask);
    }
}

/****************************************************************************
DESCRIPTION
    Calculate the overall pio mask for the given button event config.
*/
uint32 bmGetButtonEventConfigPioMask(const event_config_type_t *event_config)
{
    uint32 mask = 0;

    if (event_config)
        mask = (uint32)(event_config->pio_mask | (uint32)((uint32)((event_config->chg_enabled << 1) | event_config->vreg_enabled) << VREG_LOGICAL_INPUT_ID));

    return mask;
}

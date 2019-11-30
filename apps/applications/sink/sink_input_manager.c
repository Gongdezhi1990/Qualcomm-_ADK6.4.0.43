/*
Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.

*/

/*!
@file
@ingroup sink_app
@brief   
    The Input Manager that is responsible for monitoring messages sent from
    the various "input monitoring" tasks
*/

/* Firmware includes */
#include <message.h>
#include <stdio.h>

#if defined(ENABLE_IR_REMOTE) || (defined(GATT_ENABLED) && defined(GATT_HID_CLIENT))

/* Lib includes */
#include <config_store.h>
/* Application includes */
#include "sink_input_manager.h"
#include "sink_main_task.h"
#include "sink_states.h"
#include "sink_statemanager.h"
#include "sink_configmanager.h"
#include "sink_events.h"
#include "sink_config.h"
#ifdef ENABLE_IR_REMOTE
#include "sink_ir_remote_control.h"
#endif


#if defined(GATT_ENABLED) && defined(GATT_HID_CLIENT)
#include "sink_gatt_hid_remote_control.h"
#endif

/* Macro for Input Manager Debug */
#ifdef DEBUG_INPUT_MANAGER
#define INPUT_MANAGER_DEBUG(x) DEBUG(x)
#else
#define INPUT_MANAGER_DEBUG(x) 
#endif

/* 
    Input manager global data
*/
typedef struct
{
    uint16  down_mask; /* If a remote control has generated a button press, store the mask of the down (to remember the held buttons down, will also block out new button presses from other input sources) */
}inputManager_global_data_t;

static inputManager_global_data_t gInputManager;
#define INPUT_MANAGER gInputManager

/****************************************************************************
NAME 
      inputManagerGetDownMask

DESCRIPTION
    To get the stored down mask 
 
RETURNS
      uint16 down_mask
*/ 
static uint16 inputManagerGetDownMask(void)
{
    return INPUT_MANAGER.down_mask;
}

/****************************************************************************
NAME 
      inputManagerSetDownMask

DESCRIPTION
    To set the stored down mask 
 
PARAMS
      uint16 down_mask
      
RETURNS
      void 
*/ 
static void inputManagerSetDownMask(uint16 down_mask)
{
    INPUT_MANAGER.down_mask = down_mask;
}

/**********************************************************************
NAME
    inputManagerGetLookupSize
 
DESCRIPTION
    This function used to get the look up table size from config data size
 
PARAMS
    uint16 config_size Size of config data
RETURNS
    uint16 return size of look up table
*/
static uint16 inputManagerGetLookupSize(uint16 config_size)
{
    uint16 size_withoutlookup = (sizeof(sink_inputmanager_readonly_config_def_t)-sizeof(eventLookupTable_t ));
    
    return config_size ? (((config_size * sizeof(uint16)) - size_withoutlookup) /sizeof(eventLookupTable_t )) : 0;
}

/*******************************************************************************
    Helper function to process an input event, requires the input event that
    has occured and the current input mask when the event was generated.
    
    Searches through the "input manager" lookup table to see if the event that
    has been generated should be converted to a user event and sent to the
    application handler to be processed.
*/
static void processInputEvent(inputEvent_t event, uint16 mask)
{
    sink_inputmanager_readonly_config_def_t *r_config_data;
    /* Read the config data and get the look up table size */
    uint16 size_lookuptable = inputManagerGetLookupSize(configManagerGetReadOnlyConfig(SINK_INPUTMANAGER_READONLY_CONFIG_BLK_ID, (const void **)&r_config_data));
    
    if (size_lookuptable)
    {    
        /* Find the "input event" in the lookup table */
        uint16 i;
        for (i = 0; i < size_lookuptable; i++)
        {
            /* "Input event ID", "Input mask" and state must match for the user event to be generated */
            if ((r_config_data->lookuptable[i].input_event == event)
                && (r_config_data->lookuptable[i].mask == mask)
                && ((1<<stateManagerGetState()) & (r_config_data->lookuptable[i].state_mask)))
            {
                /* Use the offset in the lookup table to create the user event that needs to be sent to the app_handler task */
                sinkEvents_t x = EVENTS_USR_MESSAGE_BASE + r_config_data->lookuptable[i].user_event;

                INPUT_MANAGER_DEBUG(("Generate Event [%x]\n", x));
                MessageSend(&theSink.task, x, 0);

                break;
            }
        }

        configManagerReleaseConfig(SINK_INPUTMANAGER_READONLY_CONFIG_BLK_ID);
    }

    /* No lookup entry exists for the input event */
    INPUT_MANAGER_DEBUG(("Ignore\n"));
}


/****************************************************************************/
void notifyInputManager(inputEvent_t event, uint16 mask, uint16 id)
{
    UNUSED(id);
#ifdef ENABLE_IR_REMOTE
    /* Check for IR learning mode */
    if (irRemoteControlIsLearningMode() && (irRemoteControlGetInputIdToLearn() == 0) )
    {
        /* Do no process the input event, just learn it */
        if (event == inputEventDown)
        {
            /* We have received a code from the remote controller, send a message to blink the led*/            
            MessageSend(&theSink.task, EventSysRemoteControlCodeReceived, 0);
            
            /* Store the input mask to learn */
            irRemoteControlSetInputIdToLearn(mask);
            
            /* Restart the learning mode timeout as it's being used */
            MessageCancelAll(&theSink.task, EventSysIRLearningModeTimeout);
            MessageSendLater(&theSink.task, EventSysIRLearningModeTimeout, 0, irRemoteControlGetLearningModeTimeout());
            
            /* Nothing more to do here, wait for an "unknown" button on the IR remote to be pressed so it can be learnt */
            return;
        }
    }
#endif
    /* Which event has occured? */
    switch(event)
    {
        /* Has an input been pressed? */
        case inputEventDown:
        {
            /* Ignore the press if there is an ongoing input process */
            if (!inputManagerGetDownMask())
            {
                INPUT_MANAGER_DEBUG(("IN: Event[%x] mask[%x] : ", event, mask));
                processInputEvent(event, mask);
                inputManagerSetDownMask(mask);
                
                /* We have received a code from the remote controller, send a message to blink the led*/            
                MessageSend(&theSink.task, EventSysRemoteControlCodeReceived, 0);
            }
        }
        break;
        
        /* Has a button been released? */
        case inputEventVShortRelease:
        case inputEventShortRelease:
        case inputEventLongRelease:
        case inputEventVVLongRelease:
        case inputEventVLongRelease:
        {
            INPUT_MANAGER_DEBUG(("IN: Event[%x] mask[%x] : ", event, inputManagerGetDownMask()));
            processInputEvent(event, inputManagerGetDownMask());
            inputManagerSetDownMask(0);
        }
        break;
        
        /* Has a timer fired? */
        case inputEventShortTimer:
        case inputEventLongTimer:
        case inputEventVLongTimer:
        case inputEventVVLongTimer:
        case inputEventRepeatTimer:
        {
            INPUT_MANAGER_DEBUG(("IN: Event[%x] mask[%x] : ", event, mask));
            processInputEvent(event, mask);
        }
        break;
    }
}


/****************************************************************************/
bool inputManagerBusy(void)
{
    /* Is the input manager task busy processing an input event? */
    if (inputManagerGetDownMask())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/****************************************************************************
NAME 
      inputManagerGetInputTimers

DESCRIPTION
    Get the Input Timer configuration from the Input Manager config
 
RETURNS
      Bool TRUE, if successfully retrieved timer data, otherwise FALSE
*/ 
bool inputManagerGetInputTimers(timerConfig_t *pInputTimers)
{
    bool ret_val = FALSE;
    sink_inputmanager_readonly_config_def_t *r_config_data;

    if ((pInputTimers) && (configManagerGetReadOnlyConfig(SINK_INPUTMANAGER_READONLY_CONFIG_BLK_ID, (const void **)&r_config_data)))
    {
        memcpy(pInputTimers, &r_config_data->input_timers, sizeof(timerConfig_t));
        configManagerReleaseConfig(SINK_INPUTMANAGER_READONLY_CONFIG_BLK_ID);
        ret_val = TRUE;
    }

    return ret_val;
}

/****************************************************************************
NAME 
      InputManagerConfigInit

DESCRIPTION
    Reads the PSKEY containing the Input Manager configuration
 
RETURNS
      void
*/ 
static bool InputManagerConfigInit(void)
{
    uint16  size_lookuptable = 0;
    uint16 size_config = 0;
    bool ret_val = FALSE;
    sink_inputmanager_readonly_config_def_t *r_config_data;

    size_config = configManagerGetReadOnlyConfig(SINK_INPUTMANAGER_READONLY_CONFIG_BLK_ID, (const void **)&r_config_data);

    /* Timer config should be the first bit of data in the config data, at a minimum, ensure that the timers have been read */
    if ((size_config * sizeof(uint16)) >= sizeof(timerConfig_t))
    {
        /* Config data for input manager matches the expected size */
        uint16 i;
        INPUT_MANAGER_DEBUG(("InputManager Timers: MD[%x] S[%x] L[%x] VL[%x] VVL[%x] R[%x]\n",
                    r_config_data->input_timers.multipleDetectTimer,
                    r_config_data->input_timers.shortTimer,
                    r_config_data->input_timers.longTimer,
                    r_config_data->input_timers.vLongTimer,
                    r_config_data->input_timers.vvLongTimer,
                    r_config_data->input_timers.repeatTimer));
        
        /* Get the look up table size */
        size_lookuptable = inputManagerGetLookupSize(size_config);
        INPUT_MANAGER_DEBUG(("InputManager Lookup[%d]:\n", size_lookuptable));
        for (i=0; i<size_lookuptable; i++)
        {
            INPUT_MANAGER_DEBUG(("[%02d]=[%04x][%04x][%02x][%02x]\n",
                        i,
                        r_config_data->lookuptable[i].mask,
                        r_config_data->lookuptable[i].state_mask,
                        r_config_data->lookuptable[i].input_event,
                        r_config_data->lookuptable[i].user_event));
        }
        
        configManagerReleaseConfig(SINK_INPUTMANAGER_READONLY_CONFIG_BLK_ID);
        ret_val = TRUE;
    }

    return ret_val;
}

/****************************************************************************
NAME 
      InputManagerInit

DESCRIPTION
    Initialise Input Manager module
 
RETURNS
      void
*/ 
void InputManagerInit(void)
{
    InputManagerConfigInit();
}


#endif

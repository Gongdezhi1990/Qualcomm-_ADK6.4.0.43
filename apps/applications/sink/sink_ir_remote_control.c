/****************************************************************************
Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_ir_remote_control.c

DESCRIPTION
    Interface with Infrared Remote controller(s)
*/

#ifdef ENABLE_IR_REMOTE

/* Application includes */
#include "sink_ir_remote_control.h"
#include "sink_main_task.h"
#include "sink_events.h"
#include "sink_malloc_debug.h"
#include "sink_debug.h"
#include "sink_configmanager.h"
#include "sink_config.h"
#include "sink_ir_remote_control_config_def.h"
#include <config_store.h>

#include <pio_common.h>

/* Firmware includes */
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <infrared.h>

/* Macro for BLE IR RC Debug */
#ifdef DEBUG_IR_RC
#define IR_RC_DEBUG(x) DEBUG(x)
#else
#define IR_RC_DEBUG(x)
#endif

/*
    Global data required by the Infrared input monitor
*/
typedef struct
{
    TaskData task;                /* The task that is responsible for communicating with the Infra Red RC */
    Task input_manager;       /* The "input manager" task that the "IR Input Monitor" will notify of all valid input events sent from the Infra Red RC */

    /* IR learning mode */
    uint16 learn_this_input_id; /* When in learning mode, this is the input ID to learn */

    /* Runtime data */
    uint16 button_mask;         /* Used to keep track of which button is held down (if any) */
    unsigned button_state:4;      /* Used to keep track of the button state */

    /* Control flags */
    unsigned learning_mode:1;      /* Flag to indicate whether or not "learning mode" is in progess */
    unsigned reserved:11;
} irRc_global_data_t;


/* Global data structure element for IR RC module */
static irRc_global_data_t *gIrInputMonitor = NULL;
#define IR_RC_GDATA  gIrInputMonitor


/****************************************************************************
NAME
    irRcGetNumCodes

DESCRIPTION
    Get the number of entries in the app-defined lookup table.

RETURNS
    uint16 The number of elements in the table.
*/
static uint16 irRcGetNumCodes(uint16 readonly_size)
{
    uint16 num_lookup = 0;

    uint16 size_lookup_table = (readonly_size  * sizeof(uint16)) - (sizeof(sink_ir_rc_readonly_config_def_t) - sizeof(irLookupTableConfig_t));

    num_lookup = size_lookup_table/sizeof(irLookupTableConfig_t);
    return num_lookup;
}

/****************************************************************************
NAME
    irRcGetNumLearntCodes

DESCRIPTION
    Get the number of entries in the learnt ir codes lookup table.

RETURNS
    uint16 The number of elements in the table.
*/
static uint16 irRcGetNumLearntCodes(void)
{
    sink_ir_rc_writeable_config_def_t *data;
    uint16 size;
    uint16 num_learnt_codes = 0;

    size = configManagerGetReadOnlyConfig(SINK_IR_RC_WRITEABLE_CONFIG_BLK_ID, (const void **)&data);
    if (size)
    {
        num_learnt_codes = ((size * sizeof(uint16)) / sizeof(data->learnt_codes[0]));
    }

    configManagerReleaseConfig(SINK_IR_RC_WRITEABLE_CONFIG_BLK_ID);

    return num_learnt_codes;
}

/*******************************************************************************
NAME
    tableIndexToBitmask

DESCRIPTION
    Helper function to convert a given uint16 table index value into a uint16 bitmask that is
    to be utilised by input manager module

RETURNS
    uint16 bitmask corresponding to the given table index value.

*********************************************************************************/
static uint16 tableIndexToBitmask(const uint16 index)
{
    uint16 bitmask = 0;
    bitmask = 0x1 << index;

    return bitmask;
}

/*******************************************************************************
NAME
    bitmaskToTableIndex

DESCRIPTION
    Helper function to convert a given uint16 bitmask value into a uint16 table index value that is
    to be utilised by remote oontrol module while creating a new record for recently learnt RC codes

RETURNS
    uint16 table index corresponding to the given bitmask value.

*********************************************************************************/
static uint16 bitmaskToTableIndex(const uint16 bitmask){

    uint16 index = 0;
    uint16 tempBitmask = bitmask;

    /* initial case (bitmask == 0) could result in an infinite loop, take care! */
    if(tempBitmask == 0)
        return 0;

    while(tempBitmask != 0x1)
    {
        tempBitmask = tempBitmask >> 0x1;
        index++;
    }

    return index;

}

/*******************************************************************************
NAME
    irCodeLookup

DESCRIPTION
    Helper function to search through the IR lookup tables to see if the
    recieved code matched.

RETURNS
    uint16 input mask indicating which input (button) was pressed, if no match
    in the lookup tables was found, will return zero.

*********************************************************************************/
static uint16 irCodeLookup( uint16 address, uint16 code )
{
    uint16 i;
    uint16 input_mask = 0;
    sink_ir_rc_readonly_config_def_t *r_config_data = NULL;

    uint16 size;
    uint16 num_entries;

    /*Retrieve read only data*/
    size = configManagerGetReadOnlyConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID, (const void **)&r_config_data);
    if (size)
    {
        /* Does the recieved IR code match an entry in the IR lookup table? */
        num_entries = irRcGetNumCodes(size);
        for (i = 0; i < num_entries; i++)
        {
            if ((r_config_data->lookupTable[i].ir_code == code)
                && (r_config_data->lookupTable[i].remote_address == address))
            {
                /* Found a match; translate the IR code to an input mask */
                input_mask = tableIndexToBitmask(r_config_data->lookupTable[i].input_id);
                break;
            }
        }

        configManagerReleaseConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID);
    }

    if (!input_mask)
    {
        /* No match was found in the default lookup table; Does the recieved
           IR code match an entry in the learnt codes lookup table? */
        sink_ir_rc_writeable_config_def_t *w_config_data = NULL;

        size = configManagerGetReadOnlyConfig(SINK_IR_RC_WRITEABLE_CONFIG_BLK_ID, (const void **)&w_config_data);
        if (size)
        {
            num_entries = ((size * sizeof(uint16))/ sizeof(w_config_data->learnt_codes[0]));
            for (i = 0; i < num_entries; i++)
            {
                if ( (w_config_data->learnt_codes[i].ir_code == code)
                    && (w_config_data->learnt_codes[i].remote_address == address))
                {
                    /* Found a match; translate the IR code to an input mask and return */
                    input_mask = tableIndexToBitmask(w_config_data->learnt_codes[i].input_id);
                    break;
                }
            }
        }
       
        configManagerReleaseConfig(SINK_IR_RC_WRITEABLE_CONFIG_BLK_ID);
    }

    return input_mask;
}


/*******************************************************************************
    Helper function to create and send a message to the irInputMonitorTask

    PARAMETERS:
    mid         - Message ID
    timer       - Timer ID to identify which timer this message is going to be
    mask        - input mask
    addr        - Address of the IR RC whose button this message is associated
    delay_time  - delay sending the message by *timer* ms
*/
static void createIrRcButtonEventMessage(irRcMessageID_t mid, irRcTimerID_t timer, uint16 mask, uint16 addr, uint32 delay_time)
{
    MAKE_IR_RC_MESSAGE(IR_RC_BUTTON_EVENT_MSG);
    message->timer  = timer;
    message->mask   = mask;
    message->addr   = addr;

    /* Dispatch the message */
    MessageSendLater(&IR_RC_GDATA->task, mid, message, delay_time);
}

/*******************************************************************************
NAME
    handleIrEventMsg

DESCRIPTION
    Helper function to handle when firmware sends an Infra Red Event message
*/
static void handleIrEventMsg( MessageInfraRedEvent * msg )
{
    /* If not in learning mode, process the IR event and notify the Input Manager (if IR event is valid) */
    if (!IR_RC_GDATA->learning_mode )
    {
        /* Is this a button press or a button release? */
        if (msg->event == EVENT_PRESSED)
        {
            /* Find out if the recieved IR code is one that is understood */
            uint16 input_mask = irCodeLookup(msg->address, msg->data[0]);

            /* Was the IR code valid (understood)? */
            if (input_mask)
            {
                timerConfig_t InputTimers;

                inputManagerGetInputTimers(&InputTimers);

                /* Inform the input manager which button has been pressed */
                notifyInputManager(inputEventDown, input_mask, msg->address);

                /* Store the input mask of the button that is now held down */
                IR_RC_GDATA->button_mask = input_mask;

                /* Update the button state */
                IR_RC_GDATA->button_state = inputDownVShort;

                /* Start the duration and repeat timers */
                createIrRcButtonEventMessage(IR_RC_BUTTON_TIMER_MSG, IR_RC_TIMER_SHORT, input_mask, msg->address, InputTimers.shortTimer);
                createIrRcButtonEventMessage(IR_RC_BUTTON_REPEAT_MSG, 0, input_mask, msg->address, InputTimers.repeatTimer);
            }
            else
            {
                IR_RC_DEBUG(("IR: Unrecognised IR MSG [%x] [%x]\n", msg->address, msg->data[0]));
            }
        }
        else
        {
            /* Cancel the (MULTIPLE,SHORT,LONG,VLONG or VVLONG) TIMER & REPEAT timers*/
            MessageCancelAll(&IR_RC_GDATA->task, IR_RC_BUTTON_TIMER_MSG);
            MessageCancelAll(&IR_RC_GDATA->task, IR_RC_BUTTON_REPEAT_MSG);

            /* Inform the input manager that the button has been released */
            switch (IR_RC_GDATA->button_state)
            {
                case inputNotDown:
                    /* State not valid for a release event */
                case inputMultipleDetect:
                    /* State not supported by the IR remote (mutliple IR buttons are not supported) */
                break;

                case inputDownVShort:
                {
                    notifyInputManager(inputEventVShortRelease, IR_RC_GDATA->button_mask, msg->address);
                }
                break;
                case inputDownShort:
                {
                    notifyInputManager(inputEventShortRelease, IR_RC_GDATA->button_mask, msg->address);
                }
                break;
                case inputDownLong:
                {
                    notifyInputManager(inputEventLongRelease, IR_RC_GDATA->button_mask, msg->address);
                }
                break;
                case inputDownVLong:
                {
                    notifyInputManager(inputEventVLongRelease, IR_RC_GDATA->button_mask, msg->address);
                }
                break;
                case inputDownVVLong:
                {
                    notifyInputManager(inputEventVVLongRelease, IR_RC_GDATA->button_mask, msg->address);
                }
                break;
            }

            /* Update the button state and clear the mask as a button has just been released */
            IR_RC_GDATA->button_state = inputNotDown;
            IR_RC_GDATA->button_mask = 0;
        }
    }


    /* Learning mode is active */
    else
    {
        if ((msg->event == EVENT_PRESSED) && irCanLearnNewCode())
        {
            /* Is there an input ID (or mask) to learn? */
            if (IR_RC_GDATA->learn_this_input_id)
            {
                /* Check that the code to learn is not already one that is known; there would be no point learning a known code */
                if (irCodeLookup(msg->address, msg->data[0]) != IR_RC_GDATA->learn_this_input_id)
                {
                    uint16 num_learnt_codes;
                    sink_ir_rc_writeable_config_def_t *write_config_data=NULL;

                    /* Open the learnt ir codes config as writeable, and
                       allocate one element more for the new learnt code. */
                    num_learnt_codes = irRcGetNumLearntCodes();
                    if (configManagerGetWriteableConfig(SINK_IR_RC_WRITEABLE_CONFIG_BLK_ID,
                                               (void **)&write_config_data,
                                               ((num_learnt_codes + 1) * sizeof (irLookupTableConfig_t))))
                    {
                        irLookupTableConfig_t *new_code = &write_config_data->learnt_codes[num_learnt_codes];
                        new_code->input_id = bitmaskToTableIndex(IR_RC_GDATA->learn_this_input_id);
                        new_code->remote_address = msg->address;
                        new_code->ir_code = msg->data[0];

                        configManagerUpdateWriteableConfig(SINK_IR_RC_WRITEABLE_CONFIG_BLK_ID);

                        /* Clear the "input to learn" as may want to learn other codes */
                        IR_RC_GDATA->learn_this_input_id = 0;

                        /* Let the application decide what to do now IR code has been learnt (could play tone or exit learning mode) */
                        MessageSend(&theSink.task, EventSysIRCodeLearnSuccess, 0);
                    }
#ifdef DEBUG_MALLOC
                    else
                    {
                        Panic();
                    }
#endif
                }
                else
                {
                    IR_RC_DEBUG(("IR: Cannot learn a known IR code addr[%x] data[%x] id[%x]\n", msg->address, msg->data[0], IR_RC_GDATA->learn_this_input_id));

                    /* Let app decide how to handle a failed code learn */
                    MessageSend(&theSink.task, EventSysIRCodeLearnFail, 0);
                }
            }
            else
            {
                /* Notify the input manager of the button press to learn */
                notifyInputManager(inputEventDown, irCodeLookup(msg->address, msg->data[0]), msg->address );
            }
        }
    }
}


/*******************************************************************************
NAME
    ir_rc_message_handler

DESCRIPTION
    message handler for the IR recieve messages provided by the firmware
*/
static void ir_rc_message_handler( Task task, MessageId id, Message message )
{
    timerConfig_t InputTimers;

    inputManagerGetInputTimers(&InputTimers);

    if (id == MESSAGE_INFRARED_EVENT)
    {
        handleIrEventMsg( (MessageInfraRedEvent*)message );
    }

    /* Has a duration timer fired? */
    else if (id == IR_RC_BUTTON_TIMER_MSG)
    {
        IR_RC_BUTTON_EVENT_MSG_T *msg = (IR_RC_BUTTON_EVENT_MSG_T *)message;

        /* Which timer has just fired? */
        switch(msg->timer)
        {
            case IR_RC_TIMER_SHORT:
            {
                /* Update the button state */
                IR_RC_GDATA->button_state = inputDownShort;

                /* Notify the input manager of the timer event */
                notifyInputManager(inputEventShortTimer, msg->mask, msg->addr);

                /* Start the LONG timer */
                createIrRcButtonEventMessage( IR_RC_BUTTON_TIMER_MSG, IR_RC_TIMER_LONG, msg->mask, msg->addr, (InputTimers.longTimer - InputTimers.shortTimer) );
            }
            break;
            case IR_RC_TIMER_LONG:
            {
                /* Update the button state */
                IR_RC_GDATA->button_state = inputDownLong;

                /* Notify the input manager of the timer event */
                notifyInputManager(inputEventLongTimer, msg->mask, msg->addr);

                /* Start the VLONG timer */
                createIrRcButtonEventMessage( IR_RC_BUTTON_TIMER_MSG, IR_RC_TIMER_VLONG, msg->mask, msg->addr, (InputTimers.vLongTimer - InputTimers.longTimer) );
            }
            break;
            case IR_RC_TIMER_VLONG:
            {
                /* Update the button state */
                IR_RC_GDATA->button_state = inputDownVLong;

                /* Notify the input manager of the timer event */
                notifyInputManager(inputEventVLongTimer, msg->mask, msg->addr);

                /* Start the VVLONG timer */
                createIrRcButtonEventMessage( IR_RC_BUTTON_TIMER_MSG, IR_RC_TIMER_VVLONG, msg->mask, msg->addr, (InputTimers.vvLongTimer - InputTimers.vLongTimer) );
            }
            break;
            case IR_RC_TIMER_VVLONG:
            {
                /* Update the button state */
                IR_RC_GDATA->button_state = inputDownVVLong;

                /* Notify the input manager of the timer event */
                notifyInputManager(inputEventVVLongTimer, msg->mask, msg->addr);
            }
            break;
        }
    }

    /* Has the repeat timer fired? */
    else if (id == IR_RC_BUTTON_REPEAT_MSG)
    {
        IR_RC_BUTTON_EVENT_MSG_T *msg = (IR_RC_BUTTON_EVENT_MSG_T *)message;

        /* Notify the input manager of the timer event */
        notifyInputManager(inputEventRepeatTimer, msg->mask, msg->addr);

        /* Keep sending REPEAT messages until the button(s) is/are released */
        createIrRcButtonEventMessage(  IR_RC_BUTTON_REPEAT_MSG, 0, msg->mask, msg->addr, InputTimers.repeatTimer );
    }
}


/****************************************************************************/
bool irCanLearnNewCode(void)
{
    return (irRcGetNumLearntCodes() < irRemoteControlGetMaxLearningCodes());
}


/****************************************************************************/
bool irStartLearningMode(void)
{
    /* Is it a good state for the IR monitor to start learning a new code? */
    if ( ( irCanLearnNewCode() ) && ( !inputManagerBusy() ) )
    {
        /* Start the failsafe timer to automatically stop IR learning mode after timeout */
        MessageSendLater(&theSink.task, EventSysIRLearningModeTimeout, 0, irRemoteControlGetLearningModeTimeout());

        /* Start the learning mode reminder message (can be used to trigger reminder tone indicating learning mode is active) */
        MessageSendLater(&theSink.task, EventSysIRLearningModeReminder, 0, irRemoteControlGetLearningModeReminder());

        /* Start learning mode */
        IR_RC_GDATA->learning_mode = 1;

        return TRUE;
    }
    else
    {
        /* Can't start learning mode */
        IR_RC_DEBUG(("IR: Cannot start IR Learning mode (bad state)\n"));

        return FALSE;
    }
}


/****************************************************************************/
void handleIrLearningModeReminder(void)
{
    /* A new reminder needs to be generated */
    MessageSendLater(&theSink.task, EventSysIRLearningModeReminder, 0, irRemoteControlGetLearningModeReminder());

    /* TODO : Play a tone or something */
}


/****************************************************************************/
void irStopLearningMode(void)
{
    if (IR_RC_GDATA->learning_mode)
    {
        /* Terminate learning mode by clearing the learning mode data and cancelling the timeout and reminder */
        IR_RC_GDATA->learning_mode = 0;
        IR_RC_GDATA->learn_this_input_id = 0;
        MessageCancelAll(&theSink.task, EventSysIRLearningModeTimeout);
        MessageCancelAll(&theSink.task, EventSysIRLearningModeReminder);
    }
}


/****************************************************************************/
void irClearLearntCodes(void)
{
    ConfigStoreRemoveConfig(SINK_IR_RC_WRITEABLE_CONFIG_BLK_ID);
}


/****************************************************************************
NAME
    irRemoteControlIsLearningMode

DESCRIPTION
    Indicate whether or not "learning mode" is in progess

RETURNS
      bool TRUE if learning mode is in progress, else FALSE
*/
bool irRemoteControlIsLearningMode(void)
{
    return IR_RC_GDATA->learning_mode;
}

/****************************************************************************
NAME
      irRemoteControlGetLearningModeTimeout

DESCRIPTION
    Get the learning mode timeout

RETURNS
      uint16
*/
uint16 irRemoteControlGetLearningModeTimeout(void)
{
    uint16 timeout=0;
    sink_ir_rc_readonly_config_def_t *r_config_data=NULL;

   /*Read config data*/
    if (configManagerGetReadOnlyConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID, (const void **)&r_config_data))
    {
       timeout = r_config_data->learning_mode_timeout;
       configManagerReleaseConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID);
    }

    return timeout;
}

/****************************************************************************
NAME
    irRemoteControlGetMaxLearningCodes

DESCRIPTION
    Get the max learning codes

RETURNS
    uint16
*/
uint16 irRemoteControlGetMaxLearningCodes(void)
{
    uint16 max_codes = 0;
    sink_ir_rc_readonly_config_def_t *r_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID, (const void **)&r_config_data))
    {
        max_codes = r_config_data->max_learning_codes;
        configManagerReleaseConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID);
    }

    return max_codes;
}

/****************************************************************************
NAME
    irRemoteControlGetLearningModeReminder

DESCRIPTION
    Get the learning mode reminder

RETURNS
    uint16
*/
uint16 irRemoteControlGetLearningModeReminder(void)
{
    uint16 learning_mode_reminder=0;
    sink_ir_rc_readonly_config_def_t *r_config_data=NULL;

    /*Read config data*/
    if (configManagerGetReadOnlyConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID, (const void **)&r_config_data))
    {
        learning_mode_reminder = r_config_data->learning_mode_reminder;
        configManagerReleaseConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID);
    }

    return learning_mode_reminder;
}


/****************************************************************************
NAME
      irRemoteControlGetInputIdToLearn

DESCRIPTION
    When in learning mode, this is the input ID to learn

RETURNS
      uint16
*/
uint16 irRemoteControlGetInputIdToLearn(void)
{
    return IR_RC_GDATA->learn_this_input_id;
}

/****************************************************************************
NAME
      irRemoteControlSetInputIdToLearn

DESCRIPTION
    When in learning mode, set this as the input ID to learn

RETURNS
      void
*/
void irRemoteControlSetInputIdToLearn(uint16 inputId)
{
    IR_RC_GDATA->learn_this_input_id = inputId;
}


/****************************************************************************/
static void initIrInputMonitor(void)
{
    uint16 size=0;
    uint16 num_lookup_table=0;
    sink_ir_rc_readonly_config_def_t *r_config_data=NULL;

    IR_RC_DEBUG(("IR: initIrInputMonitor\n"));

    /*Retrieve read only data*/
    size = configManagerGetReadOnlyConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID, (const void **)&r_config_data);

    /* The input monitor is only required if there's a lookup table to convert incoming Infrared events */
    if (size)
    {
        num_lookup_table = irRcGetNumCodes(size);
        if (num_lookup_table && pioCommonCheckValid(r_config_data->ir_pio))
        {
            /* Setup the message handler for the Infrared RC Monitor */
            IR_RC_GDATA->task.handler = ir_rc_message_handler;

            /* register for IR messages from the firmware */
            MessageInfraredTask(&IR_RC_GDATA->task);

            /* set the required protocol, NEC or RC5 */
            InfraredConfigure(INFRARED_PROTOCOL, r_config_data->protocol);

            /* set the pio to use for the IR receiver */
            InfraredConfigure(INFRARED_PIO, r_config_data->ir_pio);

            /* set FW IR interface parameters */
            InfraredConfigure(INFRARED_JITTER_ALLOWANCE, 300);
            InfraredConfigure(INFRARED_START_PULSE_STABLE_PERIOD, 200);
            InfraredConfigure(INFRARED_KEY_RELEASE_PERIOD, 120);
            InfraredConfigure(INFRARED_KEEP_AWAKE_PERIOD, 110);

            /* enable FW IR receive scanning */
            InfraredConfigure(INFRARED_ENABLE, 1);

#ifdef DEBUG_IR_RC
            {
                uint16 i;

                /* Print the lookup table in a readable format for debugging purposes */
                IR_RC_DEBUG(("CONF: IR Protocol[%x], MaxLearningCodes[%x], LearnTimeout[%d], LearnReminder[%d], IR_PIO[%d], num_lookup_table[%d]:\n",
                r_config_data->protocol, r_config_data->max_learning_codes, r_config_data->learning_mode_timeout,
                r_config_data->learning_mode_reminder, r_config_data->ir_pio, num_lookup_table));

                for (i=0; i<num_lookup_table; i++)
                {
                    IR_RC_DEBUG(("CONF: ADDR[%x] , INPUT ID[0x%x] , IR CODE[0x%02x]\n", r_config_data->lookupTable[i].remote_address,
                        r_config_data->lookupTable[i].input_id, r_config_data->lookupTable[i].ir_code));
                }
            }

            {
                sink_ir_rc_writeable_config_def_t *w_config_data;
                uint16 size = configManagerGetReadOnlyConfig(SINK_IR_RC_WRITEABLE_CONFIG_BLK_ID, (const void **)&w_config_data);
                if (size)
                {
                    uint16 i;
                    uint16 num_learnt_codes = ((size * sizeof(uint16)) / sizeof(w_config_data->learnt_codes[0]));

                    IR_RC_DEBUG(("IR: Num learnt codes = [%d]:\n", num_learnt_codes));
                    for (i = 0; i < num_learnt_codes; i++)
                    {
                        IR_RC_DEBUG(("IR: ADDR[%x] : [0x%x]->[0x%02x]\n", w_config_data->learnt_codes[i].remote_address,
                            w_config_data->learnt_codes[i].input_id, w_config_data->learnt_codes[i].ir_code));
                    }
                }
                   
                configManagerReleaseConfig(SINK_IR_RC_WRITEABLE_CONFIG_BLK_ID);
            }

#endif /*DEBUG_IR_RC*/

        }
            
        configManagerReleaseConfig(SINK_IR_RC_READONLY_CONFIG_BLK_ID);
    }
    else
    {
        IR_RC_DEBUG(("IR: NO IR Config\n"));
    }
}


/****************************************************************************
NAME
      irRemoteControlInit

DESCRIPTION
    Initialise the IR RC module

RETURNS
      void
*/
void irRemoteControlInit(void)
{
    /*Allocate for global*/
    IR_RC_GDATA = mallocPanic(sizeof(irRc_global_data_t));
    memset(IR_RC_GDATA,0,sizeof(irRc_global_data_t));
    /* Now initialise the Infra Red Remote Controller Task */
    initIrInputMonitor();
}

#else /* ENABLE_IR_REMOTE */

static const int dummy_ir_rc;  /* ISO C forbids an empty source file */

#endif /* ENABLE_IR_REMOTE */

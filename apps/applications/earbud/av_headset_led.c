/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_led.c
\brief      LED Manager
*/

#include <pio.h>
#include <vm.h>
#include <led.h>
#include <sink.h>
#include <panic.h>

#include "av_headset.h"
#include "av_headset_config.h"
#include "av_headset_led.h"
#include "av_headset_log.h"

/*! Define to prefer use of PIOs for LED control, rather than the use of the LED
    hardware blocks.
    PIOs were found to use lower power for the use cases here */
#define LED_USE_PIOS

#ifdef LED_USE_PIOS
/*! \cond Undocumented */
#define LED_0_BANK      (appConfigLed0Pio() >> 5)
#define LED_0_PIO_MASK  (1UL << (appConfigLed0Pio() & 31))
#define LED_1_BANK      (appConfigLed1Pio() >> 5)
#define LED_1_PIO_MASK  (1UL << (appConfigLed1Pio() & 31))
#define LED_2_BANK      (appConfigLed2Pio() >> 5)
#define LED_2_PIO_MASK  (1UL << (appConfigLed2Pio() & 31))
/*! \endcond Undocumented */
#endif

/*! Macro to make LED manager messages. */
#define MAKE_LED_MESSAGE(TYPE) \
    TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/*!  Internal messages */
enum
{
    LED_INTERNAL_UPDATE,            /*!< Update LEDs */
    LED_INTERNAL_SET_PATTERN,
    LED_INTERNAL_STOP_PATTERN,
    LED_INTERNAL_ENABLE,
};

typedef struct
{
    int8 priority;
} LED_INTERNAL_SET_PRIORITY_T;

typedef struct
{
    int8 priority;
    const ledPattern *pattern;
} LED_INTERNAL_SET_PATTERN_T;

typedef struct
{
    int8 priority;
} LED_INTERNAL_STOP_PATTERN_T;

typedef struct
{
    bool enable;
} LED_INTERNAL_ENABLE_T;

/*! \brief Update LEDs

    This function is called to update the LEDs, it checks the current LED state and
    runs through the active filters modifying the LEDs state as specified by
    the filters.
*/    
static void appLedUpdate(ledTaskData *theLed)
{
    int filter;
    uint16 led_state = theLed->led_state;
    const unsigned nleds = appConfigNumberOfLeds();

    /* Run LEDs through filters */
    for (filter = 0; filter < LED_NUM_FILTERS; filter++)
    {
        ledFilter filter_func = theLed->filter[filter];
        if (filter_func)
            led_state = filter_func(led_state);
    }

    /* Update LEDs */
    switch (nleds)
    {
        /* Jump then fall-through to set the correct number of LEDS */
#ifdef LED_USE_PIOS
        case 3: PioSet32Bank(LED_2_BANK, LED_2_PIO_MASK, led_state & 0x04 ? 0 : LED_2_PIO_MASK);
        case 2: PioSet32Bank(LED_1_BANK, LED_1_PIO_MASK, led_state & 0x02 ? 0 : LED_1_PIO_MASK);
        case 1: PioSet32Bank(LED_0_BANK, LED_0_PIO_MASK, led_state & 0x01 ? 0 : LED_0_PIO_MASK);
#else
        case 3: LedConfigure(2, LED_ENABLE, led_state & 0x04 ? 1 : 0);
        case 2: LedConfigure(1, LED_ENABLE, led_state & 0x02 ? 1 : 0);
        case 1: LedConfigure(0, LED_ENABLE, led_state & 0x01 ? 1 : 0);
#endif
        default: break;
    }
}

/*! \brief Update LED pattern

    This function is called to update the LED pattern, it is called
    on reception of the internal LED_INTERNAL_UPDATE message.
    
    This function walks through the LED pattern definition running
    the specified actions and only exits when it encounters a delay
    action or the pattern is completed.
    
    When a delay is encountered, a delayed LED_INTERNAL_UPDATE message
    is sent to the LED task with the appropriate delay.
*/    
static bool appLedHandleInternalUpdate(ledTaskData *theLed)
{
    bool update_leds = FALSE;
    MessageCancelAll(&theLed->task, LED_INTERNAL_UPDATE);
    for (;;)
    {
        ledPriorityState *state = &theLed->priority_state[theLed->priority];
        ledStack *stack = &state->stack[state->stack_ptr];
        const ledPattern *pattern = state->pattern + stack->position;

        switch (pattern->code)
        {
            case LED_PATTERN_END:
            {
                /* Stop this pattern */
                appLedStopPattern(theLed->priority);
                
                /* Exit loop */
                return FALSE;
            }
            
            case LED_PATTERN_ON:
            {
                /* Update LEDs on exit */
                update_leds = TRUE;

                /* Turn on LED */
                theLed->led_state |= pattern->data;

                /* Move to next instruction */
                stack->position++;
            }
            break;

            case LED_PATTERN_OFF:
            {
                /* Update LEDs on exit */
                update_leds = TRUE;

                /* Turn off LED */
                theLed->led_state &= ~pattern->data;

                /* Move to next instruction */
                stack->position++;
            }
            break;

            case LED_PATTERN_TOGGLE:
            {
                /* Update LEDs on exit */
                update_leds = TRUE;

                /* Toggle LED */
                theLed->led_state ^= pattern->data;

                /* Move to next instruction */
                stack->position++;
            }
            break;

            case LED_PATTERN_DELAY:
            {
                /* Send timed message if not infinite delay */
                if (pattern->data)
                    MessageSendLater(&theLed->task, LED_INTERNAL_UPDATE, 0, pattern->data);

                /* Move to next instruction */
                stack->position++;

                /* Exit loop */
                return update_leds;
            }

            case LED_PATTERN_SYNC:
            {
                /* Attempt to get wallclock from sink */
                wallclock_state_t wc_state;
                rtime_t wallclock;
                if (RtimeWallClockGetStateForSink(&wc_state, theLed->wallclock_sink) &&
                    RtimeLocalToWallClock(&wc_state, VmGetTimerTime(), &wallclock))
                {
                    uint32_t offset = wallclock % (pattern->data * 1000);
                    rtime_t local;

                    rtime_t sync_time = rtime_sub(wallclock, offset);
                    sync_time = rtime_add(sync_time, pattern->data * 1000);

                    if (RtimeWallClockToLocal(&wc_state, sync_time, &local))
                    {
                        /* Convert to milliseconds in the future */
                        int32_t delay = rtime_sub(local, VmGetTimerTime()) / 1000;

                        /* If value is negative, adjust by period to make in future again */
                        while (delay < 0)
                            delay += pattern->data;

                        /* Wait for specified delay */
                        MessageSendLater(&theLed->task, LED_INTERNAL_UPDATE, 0, delay);
                    }
                    else
                        Panic();
                }
                else
                {
                    uint32 sync_delay = pattern->data - (VmGetClock() % pattern->data);
                    MessageSendLater(&theLed->task, LED_INTERNAL_UPDATE, 0, sync_delay);
                }

                /* Move to next instruction */
                stack->position++;

                /* Exit loop */
                return update_leds;
            }
            
            case LED_PATTERN_REPEAT:
            {
                /* Check if not at end off loop */
                if (stack->position != stack->loop_end)
                {
                    /* Push new loop onto stack */
                    ledStack *stack_new   = &state->stack[++state->stack_ptr];
                    stack_new->loop_count = pattern->data & 0x3F;
                    stack_new->loop_start = pattern->data >> 6;
                    stack_new->loop_end   = stack->position;
                    stack_new->position   = stack_new->loop_start;
    
                    /* Move to next instruction */
                    stack->position++;
                }
                else
                {
                    /* Check if loop is not infinite */
                    if (stack->loop_count != 0)
                    {
                        /* Decrement loop counter */
                        stack->loop_count--;
                        
                        /* Pop top of stack if we have finished loop */
                        if (stack->loop_count == 0)
                            state->stack_ptr--;
                    }

                    /* Jump to start of loop */
                    stack->position = stack->loop_start;
                }
            }
            break;

            case LED_PATTERN_LOCK:
            {
                /* Update lock */
                theLed->lock = pattern->data;

                /* Move to next instruction */
                stack->position++;

                /* Check if we're not locked anymore */
                if (!theLed->lock)
                {
                    /* Unlocked, send update message to ourselves to allow any blocked messages to be delivered */
                    MessageSend(&theLed->task, LED_INTERNAL_UPDATE, 0);

                    /* Exit loop */
                    return update_leds;
                }
            }
            break;
        }
    }
}

/*! \brief Set priority level of active pattern

    This function is called internally to set the priority level of the active
    LED pattern.
*/
static void appLedSetPriority(int8 priority)
{
    ledTaskData *theLed = appGetLed();
    DEBUG_LOGF("appLedSetPriority %d", priority);

    /* Store new priority */
    theLed->priority = priority;

    /* Clear LEDs */
    theLed->led_state = 0;

    /* Cancel LED update message */
    MessageCancelFirst(&theLed->task, LED_INTERNAL_UPDATE);

    /* Post update message to LED task if active */
    if (theLed->priority >= 0)
        MessageSend(&theLed->task, LED_INTERNAL_UPDATE, 0);
    else
        appLedUpdate(theLed);
}


/*! \brief Message Handler

    This function is the message handler for the LED task, it only handles
    one message, LED_INTERNAL_UPDATE.
*/    
static void appLedHandler(Task task, MessageId id, Message message)
{
    ledTaskData *theLed = (ledTaskData *)task;
    switch (id)
    {
        case LED_INTERNAL_UPDATE:
        {
            if (appLedHandleInternalUpdate(theLed))
            {
                /* We need to update LEDs */
                appLedUpdate(theLed);
            }
        }
        break;

        case LED_INTERNAL_SET_PATTERN:
        {
            LED_INTERNAL_SET_PATTERN_T *req = (LED_INTERNAL_SET_PATTERN_T *)message;
            ledPriorityState *state = &theLed->priority_state[req->priority];
            state->pattern = req->pattern;
            state->stack_ptr = 0;
            state->stack[0].position = 0;
            state->stack[0].loop_start = 0;
            state->stack[0].loop_end = 0;
            state->stack[0].loop_count = 0;

            /* Check if LEDs are enabled */
            if (theLed->enable)
            {
                if (req->priority >= theLed->priority)
                    appLedSetPriority(req->priority);
            }
        }
        break;

        case LED_INTERNAL_STOP_PATTERN:
        {
            LED_INTERNAL_STOP_PATTERN_T *req = (LED_INTERNAL_STOP_PATTERN_T *)message;
            int8 priority;

            /* Clear pattern */
            theLed->priority_state[req->priority].pattern = NULL;

            /* Check if LEDs are enabled */
            if (theLed->enable)
            {
                /* Find highest priority */
                priority = LED_NUM_PRIORITIES;
                while (priority--)
                    if (theLed->priority_state[priority].pattern)
                        break;
            }
            else
            {
                /* LEDs are disabled */
                priority = -1;
            }

            /* Set highest priority pattern */
            appLedSetPriority(priority);
        }
        break;

        case LED_INTERNAL_ENABLE:
        {
            LED_INTERNAL_ENABLE_T *req = (LED_INTERNAL_ENABLE_T *)message;

            /* Check if we have enabled LEDs */
            if (req->enable & !theLed->enable)
            {
                /* Set enable flag */
                theLed->enable = req->enable;

                /* Find highest priority */
                int8 priority = LED_NUM_PRIORITIES;
                while (priority--)
                    if (theLed->priority_state[priority].pattern)
                        break;

                /* Set highest priority pattern */
                appLedSetPriority(priority);
            }
            else if (!req->enable & theLed->enable)
            {
                /* Clear enable flag */
                theLed->enable = req->enable;

                /* Set priority to -1 to disable updates and turn off LEDs */
                appLedSetPriority(-1);
            }
        }
        break;

        default:
            Panic();
    }
}


/*! \brief Initialise LED module

    This function is called at startup to initialise the LED module.
*/    
void appLedInit(void)
{
    ledTaskData *theLed = appGetLed();
    int8 priority;
    uint8 filter;
    const unsigned nleds = appConfigNumberOfLeds();

    theLed->task.handler = appLedHandler;
    theLed->priority = -1;
    theLed->led_state = 0;
    theLed->enable = TRUE;
    theLed->lock = 0;

    /* Clear patterns */
    for (priority = 0; priority < LED_NUM_PRIORITIES; priority++)
        theLed->priority_state[priority].pattern = NULL;
        
    /* Clear filters */
    for (filter = 0; filter < LED_NUM_FILTERS; filter++)
        theLed->filter[filter] = NULL;

    /* Initialise LEDs */
#ifdef LED_USE_PIOS
    switch (nleds)
    {
        // Jump then fall-through to setup the correct number of LEDS
        case 3:
            PioSetMapPins32Bank(LED_2_BANK, LED_2_PIO_MASK, LED_2_PIO_MASK);
            PioSetDir32Bank(LED_2_BANK, LED_2_PIO_MASK, LED_2_PIO_MASK);
        case 2:
            PioSetMapPins32Bank(LED_1_BANK, LED_1_PIO_MASK, LED_1_PIO_MASK);
            PioSetDir32Bank(LED_1_BANK, LED_1_PIO_MASK, LED_1_PIO_MASK);
        case 1:
            PioSetMapPins32Bank(LED_0_BANK, LED_0_PIO_MASK, LED_0_PIO_MASK);
            PioSetDir32Bank(LED_0_BANK, LED_0_PIO_MASK, LED_0_PIO_MASK);
        default:
            break;
    }
#endif
}

/*! \brief Enable/Disable LEDs

    This function is called to enable or disable all LED indications.

    \param enable Whether to enable or disable. 
*/
void appLedEnable(bool enable)
{
    ledTaskData *theLed = appGetLed();
    MAKE_LED_MESSAGE(LED_INTERNAL_ENABLE);

    DEBUG_LOGF("appLedEnable, enable %d", enable);

    message->enable = enable;
    MessageSendConditionally(&theLed->task, LED_INTERNAL_ENABLE, message, &theLed->lock);
}

/*! \brief Set LED pattern

    This function is called to set the LED pattern with the specified
    priority.  If the new priority is higher than the current priority
    then the new pattern is show immediately, otherwise the pattern will
    only be shown when the higher priority pattern has completed.

    \param[in]  pattern  Pointer to the LED pattern to be applied.
    \param      priority The priority of the LED pattern to be set. The range is 0
            to \ref LED_NUM_PRIORITIES -1.

*/
void appLedSetPattern(const ledPattern *pattern, int8 priority)
{
    ledTaskData *theLed = appGetLed();
    MAKE_LED_MESSAGE(LED_INTERNAL_SET_PATTERN);

    DEBUG_LOGF("appLedSetPattern pattern %p, priority %d", pattern, priority);

    PanicFalse(priority < LED_NUM_PRIORITIES);

    message->pattern = pattern;
    message->priority = priority;
    MessageSendConditionally(&theLed->task, LED_INTERNAL_SET_PATTERN, message, &theLed->lock);
}

/*! \brief Stop the LED pattern running at the specified priority.

    \param priority The priority of the LED pattern to be cancelled. The range is 0
                to \ref LED_NUM_PRIORITIES -1.
*/    
void appLedStopPattern(int8 priority)
{
    ledTaskData *theLed = appGetLed();
    MAKE_LED_MESSAGE(LED_INTERNAL_STOP_PATTERN);

    DEBUG_LOGF("appLedStopPattern, priority %d", priority);

    PanicFalse(priority < LED_NUM_PRIORITIES);

    message->priority = priority;
    MessageSendConditionally(&theLed->task, LED_INTERNAL_STOP_PATTERN, message, &theLed->lock);
}

/*! \brief Set LED filter

    This function is called to set a LED filter, a priority can
    be specified to control the order in which filters are applied.

    \param filter_func  The filter function to be used
    \param filter_pri   The filter to be set. Range is 0 to \ref LED_NUM_FILTERS -1 
*/
void appLedSetFilter(ledFilter filter_func, uint8 filter_pri)
{
    ledTaskData *theLed = appGetLed();

    PanicFalse(filter_pri < LED_NUM_FILTERS);

    /* Store new filter and update LEDs if filter changed */
    if (theLed->filter[filter_pri] != filter_func)
    {
        theLed->filter[filter_pri] = filter_func;
        appLedUpdate(theLed);
    }
}

/*! \brief Cancel LED filter

    Cancel previously applied filter at the specified filter priority.

    \param filter_pri The filter to be cancelled. Range is 0 to \ref LED_NUM_FILTERS -1 
*/
void appLedCancelFilter(uint8 filter_pri)
{
    ledTaskData *theLed = appGetLed();

    PanicFalse(filter_pri < LED_NUM_FILTERS);

    /* Cancel filter and update LEDS if filter active */
    if (theLed->filter[filter_pri] != NULL)
    {
        theLed->filter[filter_pri] = NULL;
        appLedUpdate(theLed);
    }
}

/*! \brief Set the time base for the LEDs to match a sink

    Stores the supplied sink for later use by the LED manager.

    \param sink The sink to be used as a time base.
*/
void appLedSetWallclock(Sink sink)
{
    ledTaskData *theLed = appGetLed();
    theLed->wallclock_sink = sink;
}


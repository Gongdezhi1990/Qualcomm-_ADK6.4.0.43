/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_led.h
\brief      Header file for the LED Manager

This file defines the macros and data structures used to configure
the LED flashing patterns in the av_headset application.

The LED flashing code supports the concept of priority levels where a 
given priority can be assigne to each pattern so that a higher priority
LED indication can interrupt a lower priority one. By default the code 
supports up to four LED priority levels.

This code also allows LED filters to be defined. When using these an
application author can define that on a certain condition (for example 
low battery state) all LED indications are displayed using the red LED
rather than the blue one.
*/

#ifndef __AV_HEADSET_LED_H
#define __AV_HEADSET_LED_H

#include <message.h>
#include <rtime.h>

/*! Number of LED priorities */
#define LED_NUM_PRIORITIES  (4)

/*! Number of LED filters */
#define LED_NUM_FILTERS     (2)


/*!@{ \name LED priority 

    \brief Macros that can be used to define priority for an LED.

    Higher numbers are higher in priority.
*/
#define LED_PRI_LOW     (0)
#define LED_PRI_MED     (1)
#define LED_PRI_HIGH    (2)
#define LED_PRI_EVENT   (3)
/*!@} */

/*!
	\brief Structure for defining each LED pattern. 

	Each LED pattern is built up using a sequence of primitives that define 
	when each LED pin is turned on or off. These basic building blocks are used 
	to build up the sequence of actions that make up a given LED pattern.
*/
typedef struct
{
    unsigned int code:3;	/*!< Code to control the next sequence in the pattern */
    unsigned int data:13;	/*!< LED pins controlled by the code */
} ledPattern;

/*! Type of filter function used for LEDs. See \ref appLedSetFilter */
typedef uint16 (*ledFilter)(uint16);


/*!@{
    \name LED Pattern macros

    \brief Macros used to control the LED pattern displayed 

    The macros below provide a top level interface for 
    specifying the individual components that can be used to 
    build up an LED pattern. They can also be used to control 
    how the patterns are displayed.
    
    See av_headset_ui.c for examples
*/
#define LED_PATTERN_END     (0x0000)            /*!< Marker for the end of an LED pattern */
#define LED_PATTERN_ON      (0x0001)            /*!< Marker for an led on instruction */
#define LED_PATTERN_OFF     (0x0002)            /*!< Marker for an led off instruction */
#define LED_PATTERN_TOGGLE  (0x0003)            /*!< Marker for an led toggle instruction */
#define LED_PATTERN_REPEAT  (0x0004)            /*!< Marker for a repeat instruction */
#define LED_PATTERN_DELAY   (0x0005)            /*!< Marker for a delay instruction */
#define LED_PATTERN_SYNC    (0x0006)            /*!< Marker for a synchronisation pause instruction */
#define LED_PATTERN_LOCK    (0x0007)            /*!< Marker for a lock/unlock instruction */

#define LED_ON(pio)             {LED_PATTERN_ON,  (pio)}                        /*!< Turn on LEDs */
#define LED_OFF(pio)            {LED_PATTERN_OFF, (pio)}                        /*!< Turn off LEDs */
#define LED_TOGGLE(pio)         {LED_PATTERN_TOGGLE, (pio)}                     /*!< Toggle the LEDs */
#define LED_REPEAT(loop, count) {LED_PATTERN_REPEAT, (loop) << 6 | (count)}     /*!< Defines how many times the primitives above this statement should be repeated */
#define LED_WAIT(delay)         {LED_PATTERN_DELAY, (delay)}                    /*!< Defines a duration for which the pattern is not updated */
#define LED_SYNC(sync)          {LED_PATTERN_SYNC, (sync)}                      /*!< Wait until clock reaches sychronisation interval */
#define LED_END                 {LED_PATTERN_END, 0}                            /*!< Used to specify the end of the LED pattern */
#define LED_LOCK                {LED_PATTERN_LOCK, 1}                           /*!< Lock pattern, prevents any else from interrupting pattern */
#define LED_UNLOCK              {LED_PATTERN_LOCK, 0}                           /*!< Unlock pattern, allows something else to pattern */
/*!@} */

/*! Stack element */
typedef struct
{
    unsigned int loop_start:5;     /*!< Index into pattern for start of loop */
    unsigned int loop_end:5;       /*!< Index into pattern for end of loop */
    unsigned int position:5;       /*!< Current position index in pattern */
    unsigned int loop_count:6;     /*!< Number of loops remaining */
} ledStack;

/*! \brief LED priority structure

    This structure hold the state for LED patterns at a particular priority.
    It contains a pointer to the current LED pattern and a 'stack' to allow for
    nesting of loops in the pattern definition.    
*/
typedef struct
{
    const ledPattern  *pattern;         /*!< Pointer to LED pattern */
    unsigned int       stack_ptr:2;     /*!< Index into stack array */
    ledStack           stack[3];        /*!< Array of stack elements */
} ledPriorityState;

/*! LED Task Structure */
typedef struct
{
    TaskData           task;                                /*!< LED task */
    uint16             led_state;                           /*!< Current LED PIO state */
    signed             priority:3;                          /*!< Current priority level */
    unsigned           enable:1;                            /*!< Flag, set if LEDs are enabled */
    ledPriorityState   priority_state[LED_NUM_PRIORITIES];  /*!< Array of LED priority structures */
    ledFilter          filter[LED_NUM_FILTERS];             /*!< Array of LED filters */
    Sink               wallclock_sink;                      /*!< Sink to get wallclock used for common timebase */
    uint16             lock;                                /*!< If the current pattern cannot be interrupted */
} ledTaskData;

extern void appLedInit(void);
extern void appLedEnable(bool enable);

extern void appLedSetPattern(const ledPattern *pattern, int8 priority);
extern void appLedStopPattern(int8 priority);

extern void appLedSetFilter(ledFilter filter_func, uint8 priority);
extern void appLedCancelFilter(uint8 filter_pri);

extern void appLedSetWallclock(Sink sink);

#endif

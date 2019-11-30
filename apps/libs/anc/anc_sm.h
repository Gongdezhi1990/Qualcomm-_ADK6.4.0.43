/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_sm.h

DESCRIPTION
    Entry point to the ANC VM Library State Machine.
*/

#ifndef ANC_SM_H_
#define ANC_SM_H_

#include "anc.h"

#include <csrtypes.h>

/* ANC VM Library States. The order is important due to the way the API
   functions are implemented */
typedef enum
{
    anc_state_uninitialised,
    anc_state_disabled,
    anc_state_enabled
} anc_state;

/* ANC VM Library State Events */
typedef enum
{
    anc_state_event_initialise,
    anc_state_event_enable,
    anc_state_event_disable,
    anc_state_event_set_mode,
    anc_state_event_set_sidetone_gain,
    anc_state_event_cycle_fine_tune_gain
} anc_state_event_id;

/* Event structure that is used to inject an event and any corresponding
   arguments into the state machine. */
typedef struct
{
    anc_state_event_id id;
    void *args;
} anc_state_event_t;

/* Event arguments for the initialise event */
typedef struct
{
    anc_mic_params_t *mic_params;
    anc_mode_t mode;
    unsigned gain;
} anc_state_event_initialise_args_t;

/* Event arguments for the set_mode event */
typedef struct
{
    anc_mode_t mode;
} anc_state_event_set_mode_args_t;

/* Event arguments for the set_sidetone_gain event */
typedef struct
{
    uint16 gain;
} anc_state_event_set_sidetone_gain_args_t;

/******************************************************************************
NAME
    ancStateMachineHandleEvent

DESCRIPTION
    Main entry point to the ANC VM library State Machine. All events will be
    injected using this function that will then determine which state specific
    handler should process the event.

RETURNS
    Bool indicating if the event was successfully processed.
*/
bool ancStateMachineHandleEvent(anc_state_event_t event);

#endif

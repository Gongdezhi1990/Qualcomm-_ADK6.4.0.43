/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_uninitialised_state.h

DESCRIPTION
    Event handling functions for the ANC Uninitialised State.
*/

#ifndef ANC_UNINITIALISED_STATE_H_
#define ANC_UNINITIALISED_STATE_H_

#include "anc_sm.h"

/******************************************************************************
NAME
    ancStateUninitialisedHandleEvent

DESCRIPTION
    Event handler for the ANC Uninitialised State.

RETURNS
    Bool indicating if the event was successfully processed.
*/
bool ancStateUninitialisedHandleEvent(anc_state_event_t event);

#endif

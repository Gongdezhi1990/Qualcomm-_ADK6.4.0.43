/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_enabled_state.h

DESCRIPTION
    Event handling functions for the ANC Enabled State.
*/

#ifndef ANC_ENABLED_STATE_H_
#define ANC_ENABLED_STATE_H_

#include "anc_sm.h"

/******************************************************************************
NAME
    ancStateEnabledHandleEvent

DESCRIPTION
    Event handler for the ANC Enabled State.

RETURNS
    Bool indicating if the event was successfully processed.
*/
bool ancStateEnabledHandleEvent(anc_state_event_t event);

#endif

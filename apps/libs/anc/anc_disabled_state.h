/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_disabled_state.h

DESCRIPTION
    Event handling functions for the ANC Disabled State.
*/

#ifndef ANC_DISABLED_STATE_H_
#define ANC_DISABLED_STATE_H_

#include "anc_sm.h"

/******************************************************************************
NAME
    ancStateDisabledHandleEvent

DESCRIPTION
    Event handler for the ANC Disabled State.

RETURNS
    Bool indicating if the event was successfully processed.
*/
bool ancStateDisabledHandleEvent(anc_state_event_t event);
        
#endif

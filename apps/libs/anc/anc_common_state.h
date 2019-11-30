/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_common_state.h

DESCRIPTION
    Event handling functions that are common to more than one state.
*/
#ifndef ANC_COMMON_STATE_H_
#define ANC_COMMON_STATE_H_

#include "anc_sm.h"

/******************************************************************************
NAME
    ancCommonStateHandleSetSidetoneGain

DESCRIPTION
    Common event handler to process the set_gain event.

RETURNS
    Bool indicating if the event was successfully processed.
*/
bool ancCommonStateHandleSetSidetoneGain(anc_state_event_t event);


/******************************************************************************
NAME
    ancCommonStateHandleSetMode

DESCRIPTION
    Common event handler to process the set_mode event.

RETURNS
    Bool indicating if the event was successfully processed.
*/
bool ancCommonStateHandleSetMode(anc_state_event_t event);
        
#endif

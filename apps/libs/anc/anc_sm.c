/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_sm.c

DESCRIPTION
    Entry point to the ANC VM Library State Machine.
*/

#include "anc_uninitialised_state.h"
#include "anc_disabled_state.h"
#include "anc_enabled_state.h"
#include "anc_common_state.h"

#include "anc_data.h"
#include "anc_debug.h"

/******************************************************************************/
bool ancStateMachineHandleEvent(anc_state_event_t event)
{
    bool ret_val = FALSE;

    switch(ancDataGetState())
    {
        case anc_state_uninitialised:
            ret_val = ancStateUninitialisedHandleEvent(event);
        break;
        
        case anc_state_disabled:
            ret_val = ancStateDisabledHandleEvent(event);
        break;

        case anc_state_enabled:
            ret_val = ancStateEnabledHandleEvent(event);
        break;

        default:
            ANC_DEBUG_INFO(("Unhandled state [%d]\n", ancDataGetState()));
            ANC_PANIC();
        break;
    }
    return ret_val;
}

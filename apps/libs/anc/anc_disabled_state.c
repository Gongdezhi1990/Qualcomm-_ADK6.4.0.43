/*******************************************************************************
Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_disabled_state.c

DESCRIPTION
    Event handling functions for the ANC Disabled State.
*/

#include "anc_disabled_state.h"
#include "anc_common_state.h"
#include "anc_data.h"
#include "anc_debug.h"
#include "anc_configure.h"

static bool enableAncEventHandler(void)
{
    bool enabled = FALSE;
    if(ancConfigure(TRUE))
    {
        ancDataSetState(anc_state_enabled);
        enabled = TRUE;
    }
    return enabled;;
}

/******************************************************************************/
bool ancStateDisabledHandleEvent(anc_state_event_t event)
{
    /* Assume failure until proven otherwise */
    bool success = FALSE;

    switch (event.id)
    {
        case anc_state_event_enable:
        {
            success = enableAncEventHandler();
        }
        break;

        case anc_state_event_set_mode:
        {
            /* Common processing for setting the mode */
            success = ancCommonStateHandleSetMode(event);
        }
        break;

        case anc_state_event_set_sidetone_gain:
        {
            /* Common processing for setting the gain */
            success = ancCommonStateHandleSetSidetoneGain(event);
        }
        break;

        case anc_state_event_cycle_fine_tune_gain:
        {
            /* Cycle through the fine tune gain only when ANC is in enabled state */
        }
        break;

        default:
        {
            ANC_DEBUG_INFO(("Unhandled event [%d]\n", event.id));
            ANC_PANIC();
        }
        break;
    }
    return success;
}

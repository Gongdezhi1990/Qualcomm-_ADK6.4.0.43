/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sport_health_hub_sm.c

DESCRIPTION
    Entry point to the HUB VM Library State Machine.
*/

#include "sport_health_hub_sm.h"
#include "sport_health_hub_private.h"
#include "macros.h"

/**
 * @brief Main entry point to the ANC VM library State Machine. All events will be
 *        injected using this function that will then determine which state specific
 *        handler should process the event.
 *
 * @param event
 *
 * @return TRUE/FALSE
 */
bool sh_hub_sm_handle_event(hub_state_event_t event)
{
    bool ret_val = FALSE;

    switch(sh_hub_get_state())
    {
        case HUB_STATE_UNINITIALISE:
            ret_val = sh_hub_state_uninitialised_handle_event(event);
        break;

        case HUB_STATE_READY:
            ret_val = sh_hub_state_ready_handle_event(event);
        break;

        case HUB_STATE_ACTIVE:
            ret_val = sh_hub_state_active_handle_event(event);
        break;

        default:
            SH_HUB_DEBUG_INFO(("Unhandled state [%d]\n", sh_hub_get_state()));
            SH_HUB_PANIC();
        break;
    }
    return ret_val;
}

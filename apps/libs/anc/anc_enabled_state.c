/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_enabled_state.c

DESCRIPTION
    Event handling functions for the ANC Enabled State.
*/

#include "anc_enabled_state.h"
#include "anc_common_state.h"
#include "anc_data.h"
#include "anc_debug.h"
#include "anc_configure.h"
#include "anc_gain.h"

static bool disableAncEventHandler(void)
{
    bool disabled = FALSE;
    if(ancConfigure(FALSE))
    {
        ancDataSetState(anc_state_disabled);
        disabled = TRUE;
    }
    return disabled;
}

static bool setSidetoneGainEventHandler(anc_state_event_t event)
{
    uint16 old_gain = ancDataGetSidetoneGain();
    bool gain_set = FALSE;

    if(ancCommonStateHandleSetSidetoneGain(event))
    {
        gain_set = TRUE;
        if(old_gain != ancDataGetSidetoneGain())
        {
            ancConfigureSidetoneGains();
        }
    }
    return gain_set;
}

/******************************************************************************/
bool ancStateEnabledHandleEvent(anc_state_event_t event)
{
    bool success = FALSE;

    switch (event.id)
    {
        case anc_state_event_disable:
        {
            success = disableAncEventHandler();
        }
        break;

        case anc_state_event_set_mode:
        {
            if (ancCommonStateHandleSetMode(event))
            {
                success = ancConfigureAfterModeChange();
            }
        }
        break;

        case anc_state_event_set_sidetone_gain:
        {
            success = setSidetoneGainEventHandler(event);
        }
        break;

        case anc_state_event_cycle_fine_tune_gain:
        {
            success = ancGainIncrementFineTuneGain();
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

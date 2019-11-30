/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_uninitialised_state.c

DESCRIPTION
    Event handling functions for the ANC Uninitialised State.
*/

#include "anc_uninitialised_state.h"
#include "anc_common_state.h"
#include "anc_config_data.h"
#include "anc_data.h"
#include "anc_debug.h"
#include <stdlib.h>
#include "anc_licence_check.h"

static bool initialiseAncEventHandler(anc_state_event_initialise_args_t * args)
{
    bool initialised = FALSE;

    if(args && ancLicenceCheckIsAncLicenced() && ancDataInitialise())
    {
        ancDataRetrieveAndPopulateTuningData(args->mode);

        ancDataSetMicParams(args->mic_params);
        ancDataSetSidetoneGain(args->gain);
        ancDataSetMode(args->mode);

        ancDataSetState(anc_state_disabled);
        initialised = TRUE;
    }
    return initialised;
}

/******************************************************************************/
bool ancStateUninitialisedHandleEvent(anc_state_event_t event)
{
    bool success = FALSE;

    switch (event.id)
    {
        case anc_state_event_initialise:
        {
            ANC_ASSERT(event.args != NULL);
            if (event.args == NULL) return FALSE;

            success = initialiseAncEventHandler((anc_state_event_initialise_args_t *)event.args);
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

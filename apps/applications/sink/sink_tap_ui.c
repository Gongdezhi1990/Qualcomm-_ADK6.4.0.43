/*! \brief Message Handler

    This function is the main message handler for the TAP UI module, all user tap
    types are handled by this function.

*/

#ifdef ACTIVITY_MONITORING
#include "sink_tap_ui.h"
#include "sink_configmanager.h"
#include "sink_statemanager.h"
#include "sink_main_task.h"
#include "sink_events.h"
#include "sink_malloc_debug.h"

static tapUiTaskData *gTapUiData = NULL;
#define TAP_UI_GDATA gTapUiData

/**
 * @brief Handler for the Tap UI task. Function to handle the single/double
 *        tap notifications and map them to different user events as per that
 *        configured in configuration tool.
 *
 * @param task Task ID
 * @param id Message ID
 * @param message Message content
 */
static void tapUiHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(message);
    UNUSED(task);
    uint8 i = 0;

    for(i = 0; i < NUM_TAP_UI_PATTERNS; i++)
    {
        if((TAP_UI_GDATA->tapUiData[i].tap_type) == id)
        {
            if ((TAP_UI_GDATA->tapUiData[i].state_mask) & (1 << stateManagerGetState()))
            {
                /*we have fully matched an event....so tell the main task about it*/
                MessageSend(TAP_UI_GDATA->client, (TAP_UI_GDATA->tapUiData[i].user_event + EVENTS_MESSAGE_BASE), 0);
            }
         }
    }
}

/*! brief Initialise tap UI module */
void tapUiInit(void)
{
    activity_monitoring_config_def_t *p_activity_monitoring_config = NULL;

    /* Put the tap UI task in a single memory block*/
    TAP_UI_GDATA = mallocPanic(sizeof(*TAP_UI_GDATA));
    memset(TAP_UI_GDATA, 0, sizeof(*TAP_UI_GDATA));
    /* Set up task handler */
    TAP_UI_GDATA->task.handler = tapUiHandleMessage;
    TAP_UI_GDATA->client = &theSink.task;
    /* Read the tap user event set configured by user in configuration tool */
    if(configManagerGetReadOnlyConfig(ACTIVITY_MONITORING_CONFIG_BLK_ID,
                         (const void **)&p_activity_monitoring_config) > 0)
    {
        memcpy(TAP_UI_GDATA->tapUiData, p_activity_monitoring_config->tap_user_event_set, (sizeof(tap_ui_config_type_t) * NUM_TAP_UI_PATTERNS));
        configManagerReleaseConfig(ACTIVITY_MONITORING_CONFIG_BLK_ID);
    }
}

/*! brief Get the button manager task data */
tapUiTaskData *sinkGetTapUiTask(void)
{
    return TAP_UI_GDATA;
}

#endif

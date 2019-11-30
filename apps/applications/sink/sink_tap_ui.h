#ifndef SINK_TAP_UI_H
#define SINK_TAP_UI_H

#ifdef ACTIVITY_MONITORING
#include "sink_activity_monitoring_config_def.h"

#ifdef DEBUG_TAP_UI
#define TAP_UI_DEBUG(x) DEBUG(x)
#else
#define TAP_UI_DEBUG(x)
#endif

#define ARRAY_COUNT(array) (sizeof((array)) / sizeof((array)[0]))
#define NUM_TAP_UI_PATTERNS (ARRAY_COUNT(((activity_monitoring_config_def_t *)0)->tap_user_event_set))

/*! \brief Tap UI task structure */
typedef struct
{
    /*! The UI task. */
    TaskData    task;
    Task        client;
    tap_ui_config_type_t tapUiData[NUM_TAP_UI_PATTERNS];
} tapUiTaskData;

tapUiTaskData *sinkGetTapUiTask(void);

/*! brief Initialise tap UI module */
void tapUiInit(void);

#endif  /* ACTIVITY_MONITORING */
#endif  /* SINK_TAP_UI_H */

/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_ba_broadcaster_scm.c
 
DESCRIPTION
    Handler for SCM messages.
*/

#include "sink_ba_broadcaster_scm.h"
#include "sink_debug.h"

#ifdef ENABLE_BROADCAST_AUDIO

#include "sink_ba.h"

#include <broadcast_cmd.h>

#include <stdlib.h>

/* Debugging defines */
#ifdef DEBUG_BA_BROADCASTER
#define DEBUG_BROADCASTER(x) DEBUG(x)
#else
#define DEBUG_BROADCASTER(x)
#endif

static bool afh_update_scm_in_progress;
static uint8*      afh_update_scm_message;

/******************************************************************************/
static void broadcasterHandleBcmdInitConfirm(Task task, BROADCAST_CMD_INIT_CFM_T *cfm)
{
    UNUSED(task);

    DEBUG_BROADCASTER(("Broadcaster:broadcasterHandleBcmdInitConfirm(), Status %d\n",cfm->status));

    /* Check if SCM Library initialised successfully */
    if (cfm->status == broadcast_cmd_success)
    {
        afh_update_scm_in_progress = FALSE;
        sinkBroadcastAudioHandleInitCfm();
    }
    else
    {
        DEBUG_BROADCASTER(("Broadcaster:broadcasterHandleScmInitConfirm() Failed\n"));
    }
}

/******************************************************************************/
static void broadcasterHandleAfhUpdateMessageCfm(void)
{
    /* if this was an AFH Update message cfm, clear the flag to prevent further messages
     * and free the memory for the message just completed. */
    if (afh_update_scm_in_progress)
    {
        free(afh_update_scm_message);
        afh_update_scm_in_progress = FALSE;
    }
}

/*******************************************************************************/
static void broadcasterHandleShutdownCfm(void)
{
    /* we have some memory waiting to be freed on completion of an
     * AFH SCM transmission, free it if we were still waiting for it */
    if ((afh_update_scm_in_progress) && (afh_update_scm_message))
    {
        free(afh_update_scm_message);
        afh_update_scm_in_progress = FALSE;
    }
}


/******************************************************************************/
void sinkBroadcasterHandleBcmdMessage(Task task, MessageId id, Message message)
{
    switch (id)
    {
        case BROADCAST_CMD_INIT_CFM:
            DEBUG_BROADCASTER(("BA: BCMD: INIT CFM\n"));
            broadcasterHandleBcmdInitConfirm(task,(BROADCAST_CMD_INIT_CFM_T *)message);
            break;

        case BROADCAST_CMD_AFH_UPDATE_CFM:
            DEBUG_BROADCASTER(("BA: BCMD: AFH Update CFM\n"));
            broadcasterHandleAfhUpdateMessageCfm();
            break;

        case BROADCAST_CMD_SHUTDOWN_CFM:
            DEBUG_BROADCASTER(("BA: BCMD: SHUTDOWN CFM\n"));
            broadcasterHandleShutdownCfm();
            break;

        case BROADCAST_CMD_ENABLE_CFM:
            DEBUG_BROADCASTER(("BA: BCMD: ENABLE_CFM\n"));
            break;

        case BROADCAST_CMD_DISABLE_CFM:
            DEBUG_BROADCASTER(("BA:BCMD: DISABLE_CFM\n"));
            break;

        default:
            DEBUG_BROADCASTER(("BA:Unhandled SCM message 0x%x\n", id));
            break;
    }
}

#endif

/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_ba_broadcaster_scm.h
 
DESCRIPTION
    Handler for SCM messages.
*/

#ifndef SINK_BA_BROADCASTER_SCM_H_
#define SINK_BA_BROADCASTER_SCM_H_

#include <message.h>

/***************************************************************************
DESCRIPTION
    Handler for Broadcast Cmd Lib Messages
*/
#ifdef ENABLE_BROADCAST_AUDIO
void sinkBroadcasterHandleBcmdMessage(Task task, MessageId id, Message message);
#else
#define sinkBroadcasterHandleBcmdMessage(task, id, msg) ((void)(0))
#endif

#endif /* SINK_BA_BROADCASTER_SCM_H_ */

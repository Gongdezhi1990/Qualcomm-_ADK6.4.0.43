/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_task.c

DESCRIPTION
    The internal AHI library Task.

    Mainly processes commands from the Host.

*/

#ifndef AHI_TASK_H_
#define AHI_TASK_H_

#include "message.h"


/******************************************************************************
NAME
    ahiHandleMessage

DESCRIPTION
    Main AHI library message handler.

    It has a couple of main responsibilities:
      Process & respond to AHI messages sent from the host.

      Manage the state when sending or receiving AHI messages
        that consist of multiple data packets.

RETURNS
    n/a
*/
void ahiHandleMessage(Task task, MessageId id, Message message);


#endif /* AHI_TASK_H_ */

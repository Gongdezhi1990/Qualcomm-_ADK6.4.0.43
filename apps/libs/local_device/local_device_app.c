/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device_app.c

DESCRIPTION
    Interface from local_device to application
*/

#include "local_device_app.h"
#include "local_device.h"

#include <stdlib.h>

static Task app_task;

/******************************************************************************/
void localDeviceAppSetTask(Task task)
{
    app_task = task;
}

/******************************************************************************/
void localDeviceAppSendInitCfm(void)
{
    MessageSend(app_task, LOCAL_DEVICE_INIT_CFM, NULL); 
}

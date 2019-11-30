/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    local_device_app.h

DESCRIPTION
    Interface from local_device to application
*/

#ifndef LOCAL_DEVICE_APP_
#define LOCAL_DEVICE_APP_

#include <message.h>

/******************************************************************************/
void localDeviceAppSetTask(Task task);

/******************************************************************************/
void localDeviceAppSendInitCfm(void);

#endif /* LOCAL_DEVICE_APP_ */

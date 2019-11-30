/*
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   Interface to the PTS qualification for HID over GATT qualification, which includes:
    -Enabling notification for all external report reference characteristics.
*/

#ifndef _SINK_GATT_HID_QUALIFICATION_H_
#define _SINK_GATT_HID_QUALIFICATION_H_

#include "sink_gatt_client_hid.h"

#include <message.h>
#include <app/message/system_message.h>


/****************************************************************************

 @brief sinkGattHIDExtraConfig
    Extra configurations to read External Report Reference Characteristic Descriptors value and
    register for input report notifications for qualification purpose.

 @param  none
*/
#ifdef GATT_HID_CLIENT
void sinkGattHIDExtraConfig(void);
#else
#define sinkGattHIDExtraConfig() ((void)(0))
#endif

/*******************************************************************************
 @brief sinkGattHIDQualificationMsgHandler
    Handle messages from the GATT Manager Client Task library related to HID qualification.

 @param  task The task the message is delivered
 @param  id The ID for the GATT message
 @param  message The message payload
*/
#ifdef GATT_HID_CLIENT
void sinkGattHIDQualificationMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattHIDQualificationMsgHandler(task, id, message) ((void)(0))
#endif


#endif /* _SINK_GATT_HID_QUALIFICATION_H_ */


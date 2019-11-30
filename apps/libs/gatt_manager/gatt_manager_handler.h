/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

*/

#ifndef GATTMANAGER_HANDLER_H_
#define GATTMANAGER_HANDLER_H_

#include <library.h>
#include <gatt.h>

/* Send GATT_MANAGER_REGISTER_WITH_GATT_CFM message. */
void registerWithGattCfm(gatt_manager_status_t status);

/* GATT Manager message handler. */
void gattManagerMessageHandler(Task task, MessageId id, Message payload);

#endif /*GATTMANAGER_HANDLER_H_*/

/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

#ifndef GATT_APPLE_NOTIFICATION_CLIENT_WRITE_H_
#define GATT_APPLE_NOTIFICATION_CLIENT_WRITE_H_

#include "gatt_apple_notification_client.h"
#include "gatt_apple_notification_client_private.h"
#include <gatt_manager.h>

/***************************************************************************
NAME
    writeClientConfigNotifyValue

DESCRIPTION
    Write Client Configuration descriptor value by handle.
*/
void writeClientConfigNotifyValue(GANCS *ancs, bool notifications_enable, uint16 handle);

/***************************************************************************
NAME
    ancsWriteCharValue

DESCRIPTION
    Write Control Point Characteristic value by handle.
*/
void ancsWriteCharValue(GANCS *ancs, const ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC_T* req, uint16 handle);

/***************************************************************************
NAME
    handleAncsWriteValueResp

DESCRIPTION
    Handle response as a result of writing a characteristic value.
*/
void handleAncsWriteValueResp(GANCS *ancs, const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *write_cfm);

#endif /* GATT_APPLE_NOTIFICATION_CLIENT_WRITE_H_ */

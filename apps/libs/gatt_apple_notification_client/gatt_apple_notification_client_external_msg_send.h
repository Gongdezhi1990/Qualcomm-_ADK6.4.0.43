/* Copyright (c) 2019 Qualcomm Technologies International, Ltd. */

#ifndef GATT_APPLE_NOTIFICATION_CLIENT_EXTERNAL_MSG_SEND_H_
#define GATT_APPLE_NOTIFICATION_CLIENT_EXTERNAL_MSG_SEND_H_

#include "gatt_apple_notification_client.h"

void gattAncsSendSetNotificationSourceNotificationResponse(GANCS *ancs, gatt_status_t gatt_status);
void gattAncsSendSetDataSourceNotificationResponse(GANCS *ancs, gatt_status_t gatt_status);
void gattAncsSendWriteControlPointResponse(GANCS *ancs, gatt_status_t gatt_status);
void gattAncsSendNotificationSourceInd(GANCS *ancs, uint8 event_id, uint8 event_flag, uint8 category_id, uint8 category_count, uint32 notification_uid);
void gattAncsSendDataSourceAttributesInd(GANCS *ancs, uint32 notification_uid, const uint8 *attributes_data, uint16 attributes_data_size);
void gattAncsSendDataSourceAppAttributesInd(GANCS *ancs, uint16 value_size, const uint8 *value);
void gattAncsSendInitResponse(GANCS *ancs, gatt_status_t gatt_status);

#endif /* GATT_APPLE_NOTIFICATION_CLIENT_EXTERNAL_MSG_SEND_H_ */

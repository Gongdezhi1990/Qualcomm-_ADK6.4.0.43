/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_link_setup.h
DESCRIPTION
    Header file for link setup with remote device.
*/

#ifndef __IAP2_LINK_SETUP_H__
#define __IAP2_LINK_SETUP_H__

#include "iap2_private.h"

#define IAP2_SYNC_PAYLOAD_BASE_LEN          (13)

#define IAP2_SERIAL_TRANSPORT_ID            (30)
#define IAP2_USBDEV_TRANSPORT_ID            (31)
#define IAP2_USBHOST_TRANSPORT_ID           (32)
#define IAP2_BLUETOOTH_TRANSPORT_ID         (33)

#define IAP2_IDENT_INFO_PARAM_NAME          (0)
#define IAP2_IDENT_INFO_PARAM_MODEL_ID      (1)
#define IAP2_IDENT_INFO_PARAM_MANUFACTURER  (2)
#define IAP2_IDENT_INFO_PARAM_SERIAL_NUMBER (3)
#define IAP2_IDENT_INFO_PARAM_FIRMWARE_VER  (4)
#define IAP2_IDENT_INFO_PARAM_HARDWARE_VER  (5)
#define IAP2_IDENT_INFO_PARAM_MESSAGES_TX   (6)
#define IAP2_IDENT_INFO_PARAM_MESSAGES_RX   (7)
#define IAP2_IDENT_INFO_PARAM_POWER_CAP     (8)
#define IAP2_IDENT_INFO_PARAM_MAX_CURRENT   (9)
#define IAP2_IDENT_INFO_PARAM_EAP           (10)
#define IAP2_IDENT_INFO_PARAM_TEAM_ID       (11)
#define IAP2_IDENT_INFO_PARAM_CURRENT_LANG  (12)
#define IAP2_IDENT_INFO_PARAM_SUPPORT_LANG  (13)
#define IAP2_IDENT_INFO_PARAM_SERIAL_TRANS  (14)
#define IAP2_IDENT_INFO_PARAM_USBDEV_TRANS  (15)
#define IAP2_IDENT_INFO_PARAM_USBHOST_TRANS (16)
#define IAP2_IDENT_INFO_PARAM_BT_TRANS      (17)
#define IAP2_IDENT_INFO_PARAM_IAP2_HID      (18)
#define IAP2_IDENT_INFO_PARAM_VEHICLE_INFO  (20)
#define IAP2_IDENT_INFO_PARAM_VEHICLE_STAT  (21)
#define IAP2_IDENT_INFO_PARAM_LOCATION_INFO (22)
#define IAP2_IDENT_INFO_PARAM_USBHOST_HID   (23)
#define IAP2_IDENT_INFO_PARAM_WIRELESS_CAR  (24)
#define IAP2_IDENT_INFO_PARAM_BT_HID        (29)

#define IAP2_EAP_PARAM_ID                   (0)
#define IAP2_EAP_PARAM_NAME                 (1)
#define IAP2_EAP_PARAM_MATCH_ACTION         (2)
#define IAP2_EAP_PARAM_NATIVE_TRANS_ID      (3)
#define IAP2_EAP_PARAM_CAR_PLAY             (4)

#define IAP2_USBHOST_PARAM_ID               (0)
#define IAP2_USBHOST_PARAM_NAME             (1)
#define IAP2_USBHOST_PARAM_SUPPORT_IAP2     (2)
#define IAP2_USBHOST_PARAM_CARPLAY_IF       (3)
#define IAP2_USBHOST_PARAM_SUPPORT_CARPLAY  (4)

#define IAP2_SERIAL_PARAM_ID                (0)
#define IAP2_SERIAL_PARAM_NAME              (1)
#define IAP2_SERIAL_PARAM_SUPPORT_IAP2      (2)

#define IAP2_BT_PARAM_ID                    (0)
#define IAP2_BT_PARAM_NAME                  (1)
#define IAP2_BT_PARAM_SUPPORT_IAP2          (2)
#define IAP2_BT_PARAM_MAC_ADDR              (3)

#define IAP2_HID_PARAM_ID                   (0)
#define IAP2_HID_PARAM_NAME                 (1)
#define IAP2_HID_PARAM_FUNCTION             (2)

#define IAP2_USBHOST_HID_PARAM_ID           (0)
#define IAP2_USBHOST_HID_PARAM_NAME         (1)
#define IAP2_USBHOST_HID_PARAM_FUNCTION     (2)
#define IAP2_USBHOST_HID_PARAM_TRANS_ID     (3)
#define IAP2_USBHOST_HID_PARAM_TRANS_IF     (4)

#define IAP2_BT_HID_PARAM_ID                (0)
#define IAP2_BT_HID_PARAM_NAME              (1)
#define IAP2_BT_HID_PARAM_FUNCTION          (2)
#define IAP2_BT_HID_PARAM_TRANS_ID          (3)

void iap2HandleInternalLinkInitReq(iap2_link *link);
uint16 iap2HandleDataLinkInit(iap2_link *link, uint8 *data, uint16 size_data);
void iap2HandleInternalLinkSyncReq(iap2_link *link);
void iap2HandleInternalLinkSyncInd(iap2_link *link, const IAP2_INTERNAL_LINK_SYNC_IND_T *ind);
void iap2HandleInternalLinkAuthInd(iap2_link *link, const IAP2_INTERNAL_LINK_AUTH_IND_T *ind);
void iap2HandleCpReadAuthCertLenCfm(iap2_link *link, const IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM_T *cfm);
void iap2HandleCpReadAuthCertCfm(iap2_link *link, const IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM_T *cfm);
void iap2HandleCpWriteChallengeDataCfm(iap2_link *link, const IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM_T *cfm);
void iap2HandleCpReadChallengeRspCfm(iap2_link *link, const IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM_T *cfm);
void iap2HandleInternalLinkIdentReq(iap2_link *link);
void iap2IdentInfoPayloadGenerate(uint8 *payload);

#endif /* __IAP2_LINK_SETUP_H__ */

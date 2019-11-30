/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_BATTERY_SERVER_ACCESS_H_
#define GATT_BATTERY_SERVER_ACCESS_H_


/***************************************************************************
NAME
    handleBatteryAccess

DESCRIPTION
    Handles the GATT_MANAGER_SERVER_ACCESS_IND message that was sent to the battery library.
*/
void handleBatteryAccess(GBASS *battery_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind);


/***************************************************************************
NAME
    sendBatteryLevelAccessRsp

DESCRIPTION
    Sends a battery level response back to the GATT Manager.
*/
void sendBatteryLevelAccessRsp(const GBASS *battery_server, uint16 cid, uint8 battery_level, uint16 result);


/***************************************************************************
NAME
    sendBatteryConfigAccessRsp

DESCRIPTION
    Sends an client configuration access response back to the GATT Manager library.
*/
void sendBatteryConfigAccessRsp(const GBASS *battery_server, uint16 cid, uint16 client_config);


/***************************************************************************
NAME
    sendBatteryPresentationAccessRsp

DESCRIPTION
    Sends an presentation access response back to the GATT Manager library.
*/
void sendBatteryPresentationAccessRsp(const GBASS *battery_server, uint16 cid, uint8 name_space, uint16 description);


#endif

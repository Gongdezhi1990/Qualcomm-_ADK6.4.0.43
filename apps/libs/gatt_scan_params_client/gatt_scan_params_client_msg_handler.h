/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_SCAN_PARAMS_CLIENT_MSG_HANDLER_H_
#define GATT_SCAN_PARAMS_CLIENT_MSG_HANDLER_H_

#include <csrtypes.h>
#include <message.h>

#include "gatt_scan_params_client.h"

/***************************************************************************
NAME
    scanParamsClientMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the client role.
*/
void scanParamsClientMsgHandler(Task task, MessageId id, Message payload);

/***************************************************************************
NAME
    scanParamsSendInitSuccessCfm
    
DESCRIPTION
    Send a success GATT_CLIENT_INIT_CFM message to the registered client task.
*/
void scanParamsSendInitSuccessCfm(GSPC_T *scan_params_client, uint16 scan_interval_window_handle, uint16 scan_refresh_handle);

/***************************************************************************
NAME
    scanParamsSendInitFailureCfm
    
DESCRIPTION
    Send a failure GATT_CLIENT_INIT_CFM message to the registered client task.
*/
void scanParamsSendInitFailureCfm(GSPC_T *scan_params_client);

/***************************************************************************
NAME
    scanRefreshEnableNotification
    
DESCRIPTION
    Enables Scan refresh notifications from the remote Scan server.
*/
void scanRefreshEnableNotification(GSPC_T *scan_params_client, uint16 handle);

#endif /* GATT_CAN_PARAMS_CLIENT_MSG_HANDLER_H_ */

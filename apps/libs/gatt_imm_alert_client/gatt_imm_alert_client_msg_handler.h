/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_IMM_ALERT_CLIENT_MSG_HANDLER_H_
#define GATT_IMM_ALERT_CLIENT_MSG_HANDLER_H_


#include <csrtypes.h>
#include <message.h>

#include "gatt_imm_alert_client.h"
#include "gatt_imm_alert_client_private.h"

/***************************************************************************
NAME
    imm_alert_client_ext_msg_handler

DESCRIPTION
    Handler for messages sent to the library and internal messages. Expects notifications and indications
*/
void imm_alert_client_msg_handler(Task task, MessageId id, Message msg);

/***************************************************************************
NAME
    send_imm_alert_client_cfm

DESCRIPTION
   Utility function to send init confirmation to application  
*/
void send_imm_alert_client_cfm(GIASC_T *const imm_alert_client,
              const uint16 cid,
              const uint16 handle,
              const gatt_imm_alert_client_status status,
              const imm_alert_cfm_mesage_type cfmType);

#endif /* GATT_IMM_ALERT_CLIENT_MSG_HANDLER_H_ */


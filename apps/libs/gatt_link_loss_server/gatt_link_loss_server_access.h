/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_LINK_LOSS_SERVER_ACCESS_H_
#define GATT_LINK_LOSS_SERVER_ACCESS_H_


#include <gatt_manager.h>

#include "gatt_link_loss_server.h"

/***************************************************************************
NAME
    handle_link_loss_access

DESCRIPTION
    Handles the GATT_MANAGER_SERVER_ACCESS_IND message that was sent to the link loss library.
*/

void handle_link_loss_access(GLLSS_T *const link_loss_server,
              const GATT_MANAGER_SERVER_ACCESS_IND_T *const access_ind);

/***************************************************************************
NAME
    send_link_loss_alert_level_access_rsp

DESCRIPTION
    Sends a alert level response back to the GATT Manager.
*/
void send_link_loss_alert_level_access_rsp(const GLLSS_T *const link_loss_server, uint16 cid, uint8 alert_level);

#endif /* GATT_LINK_LOSS_SERVER_ACCESS_H_ */

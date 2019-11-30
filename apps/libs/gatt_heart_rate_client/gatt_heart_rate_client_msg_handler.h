/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_HEART_RATE_CLIENT_MSG_HANDLER_H
#define GATT_HEART_RATE_CLIENT_MSG_HANDLER_H

/***************************************************************************
NAME
    heart_rate_client_msg_handler

DESCRIPTION
    Handler for messages sent to the library and internal messages. 
    Expects notifications and indications
*/
void heart_rate_client_msg_handler(Task task, MessageId id, Message msg);

/***************************************************************************
NAME
    send_heart_rate_client_init_cfm

DESCRIPTION
   Utility function to send init confirmation to application  
*/
void send_heart_rate_client_init_cfm(GHRSC_T *const heart_rate_client,
                                                const gatt_status_t status,
                                                uint8 sensor_location,
                                                bool control_point_support);

#endif /* GATT_HEART_RATE_CLIENT_MSG_HANDLER_H */


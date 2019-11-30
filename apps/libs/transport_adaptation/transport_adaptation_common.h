/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 
FILE NAME
    transport_adaptation_common.h
 
DESCRIPTION
    This file provides necessary utility functions and macros for transport 
    adaptation module.
*/

#ifndef TRANSPORT_ADAPTATION_COMMON_H
#define TRANSPORT_ADAPTATION_COMMON_H

#include<stdlib.h>
#include<bdaddr.h>
#include <message_.h>

typedef struct transport_adaptation_data
{
    Task appTask;
    TaskData transportTask;
    uint16 cid;                   /* Remote device GATT connection identifier. */
}transport_adaptation_data_t;


#define SUCCESS (TRUE)
#define FAIL    (FALSE)

#define MAKE_TA_CLIENT_MESSAGE(TYPE,NAME) TYPE##_T *NAME = (TYPE##_T *) PanicNull(calloc(1,sizeof(TYPE##_T)))
#define MAKE_TA_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);

#define TA_SEND_REGISTER_CFM(message)   MessageSend(transportGetApptask(), TRANSPORT_REGISTER_CFM, message);
#define TA_SEND_DEREGISTER_CFM(message) MessageSend(transportGetApptask(), TRANSPORT_DEREGISTER_CFM, message);
#define TA_SEND_CONNECT_CFM(message)    MessageSend(transportGetApptask(), TRANSPORT_CONNECT_CFM, message);
#define TA_SEND_DISCONNECT_CFM(message) MessageSend(transportGetApptask(), TRANSPORT_DISCONNECT_CFM, message);
#define TA_SEND_MESSAGE_MORE_DATA(message) MessageSend(transportGetApptask(), TRANSPORT_MESSAGE_MORE_DATA, message);

/*!
    @brief Get the transport adaptation Application task

    @param None
    @return Task Returns the transport adaptation Application task
*/
Task transportGetApptask(void);

/*!
    @brief Intialise transport adaptation database.

    @param Task Transport adaptation Application task

    @return None
*/
void transportInitialise(Task app_task);

/*!
    @brief Get the transport adaptation library task

    @param None
    @return Task Returns the transport adaptation library task
*/
Task transportGetLibTask(void);

/*!
    @brief Checks whether transport adaptation library tasks are initialised or not.

    @param None

    @return bool Returns TRUE if the tasks are intialised else FALSE.
*/
bool transportIsInitialised(void);

/*!
    @brief Get the Connection identifier of remote device.

    @param None

    @return uint16 return Connection identifier of remote device.
*/
uint16 transportGetGattCid(void);

/*!
    @brief Store the Connection identifier of remote device.

    @param cid  GATT connection identifier

    @return uint16 return Connection identifier of remote device.
*/
void transportSetGattCid(uint16 cid);

#endif /* TRANSPORT_ADAPTATION_COMMON_H */


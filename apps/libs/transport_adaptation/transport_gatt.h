/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 
FILE NAME
    transport_gatt.h
 
DESCRIPTION
    This file provides necessary routines for creating and managing a transport
    link using GATT.
*/

#ifndef TRANSPORT_GATT_H
#define TRANSPORT_GATT_H

#include<message.h>

/* GATT Invalid start handle */
#define TA_GATT_INVALID_START_HANDLE (0xffff)

/* GATT Invalid End handle */
#define TA_GATT_INVALID_END_HANDLE (0)

/* GATT invalid connection identifier. */
#define TA_GATT_INVALID_CID     0

/* Validate GATT start and end handles. */
#define IS_GATT_HANDLES_NOT_VALID(start_handle, end_handle) \
((start_handle == TA_GATT_INVALID_START_HANDLE) || (end_handle == TA_GATT_INVALID_END_HANDLE))

/*!
    @brief        Transport adaptation module uses this wrapper API for register
                     with the GATT Manager using start and end handles.

    @param lib_task     The Task that will receive the messages sent from the
                               GATT Manager library.
    @param start_handle Server characteristics start handle
    @param end_handle  Server characteristics end handle

    @return TRUE if successful, FALSE otherwise
*/

bool transportGattRegister(Task lib_task,
                           uint16 start_handle,
                           uint16 end_handle);
/*!
    @brief      Transport adaptation module uses this wrapper API for sending
                   GATT remote client notification to the remote device using
                   GATT Manager library.

    Sends a notification value to the remote client on a given handle. There
    is no response to the application task if this action fails.

    @param handle       Handle of the characteristic
    @param size_value  Length of the characteristic value to be notified.
    @param value         The characteristic value to be notified. This is copied.

    @return None
*/
bool transportGattSendNotify(uint16 handle, uint16 size_value, const uint8 *value);

/******************************************************************************
DESCRIPTION

    @brief This function is called to Respond for GATT Server Access Indication.

    Used in response to GATT_MANAGER_SERVER_ACCESS_IND message, indicating that
    the remote device wants to access a value in the server database and that the
    application must provide permission for the value to return, depending on the
    permission flags set.

    @param handle         Handle of the value
    @param result          Result of Access Indication.
    @param size_value    The size of the value (octets)
    @param value           The value as an octet array, which is copied.

    @return None.
*/
void transportSendGattServerAccessRsp(uint16 handle,
                                     uint16 result,
                                     uint16 size_value,
                                     const uint8 *value);

/*!
    @brief  This function handles all GATT messages received in the transport
              adaptation module

    @param id            Message type identifier.
    @param message   Message contents.

    @return None
*/
void handleGattMessage(MessageId id, Message message);

/*!
    @brief This function is called to inform about GATT connection confirmation from application.

    @param status GATT Connection status.
    @param cid GATT Connection identifier.

    @return None
*/
void transportHandleGattConnected(gatt_status_t status, uint16 cid);

/*!
    @brief This function is called to inform about GATT disconnection confirmation from application.

    @param status GATT Connection status.
    @param cid GATT connection identifier.

    @return None
*/
void transportHandleGattDisconnected(gatt_status_t status, uint16 cid);

#endif /* TRANSPORT_GATT_H */

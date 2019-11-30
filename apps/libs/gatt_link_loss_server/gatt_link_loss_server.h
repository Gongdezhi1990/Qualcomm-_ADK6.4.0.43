/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file    gatt_link_loss_server.h
@brief   Header file for the GATT link loss server library.

        This file provides documentation for the GATT Link loss server library
        API (library name: gatt_link_loss_server).
*/

#ifndef GATT_LINK_LOSS_SERVER_H
#define GATT_LINK_LOSS_SERVER_H


#include <csrtypes.h>
#include <message.h>

#include <library.h>

#include "gatt_manager.h"

/*! @brief The Link Loss server internal structure for the server role.

    This structure is visible to the application as it is responsible for managing resource to pass to the Link Loss library.
    The elements of this structure are only modified by the Link Loss library.
    The application SHOULD NOT modify the values at any time as it could lead to undefined behaviour.
    
 */
typedef struct
{
    TaskData lib_task;
    Task app_task;
}GLLSS_T;

/*!@brief Alert level write indication type
*/
typedef struct __GATT_LLS_ALERT_LEVEL_CHANGE_IND
{
    const GLLSS_T *link_loss_server;   /*! Reference structure for the instance */
    uint16 cid;                        /*! Connection ID */
    uint16 alert_level;
} GATT_LLS_ALERT_LEVEL_CHANGE_IND_T;

/*!@brief Alert level read request type
*/
typedef struct __GATT_LLS_ALERT_LEVEL_READ_REQ
{
    const GLLSS_T *link_loss_server;   /*! Reference structure for the instance */
    uint16 cid;                        /*! Connection ID */
} GATT_LLS_ALERT_LEVEL_READ_REQ_T;

/*! @brief Enumeration of messages a client task may receive from the Link Loss service library.
 */
typedef enum
{
    GATT_LLS_ALERT_LEVEL_CHANGE_IND = GATT_LINK_LOSS_SERVER_MESSAGE_BASE,
    GATT_LLS_ALERT_LEVEL_READ_REQ,
    GATT_LLS_ALERT_SERVER_MESSAGE_TOP
} GattLinkLossServiceMessageId;


/*!
    @brief initialises the Link Loss Service Library.

    @param appTask The Task that will receive the messages sent from this link loss service library.
    @param link_loss_server A valid area of memory that the service library can use.Must be of at least the size of GLLSS_T
    @param start_handle This indicates the start handle of the LLS service
    @param end_handle This indicates the end handle of the LLS service
    
    @return TRUE if success, FALSE otherwise.

*/
bool GattLinkLossServerInit(
                                Task appTask , 
                                GLLSS_T *const link_loss_server,
                                uint16 start_handle,
                                uint16 end_handle);

/*!
    @brief This API is used to return a lls alert level to the library when a GATT_LLS_ALERT_LEVEL_READ_REQ message is received.

    @param link_loss_server The instance pointer that was in the payload of the GATT_LLS_ALERT_LEVEL_READ_REQ message.
    @param cid The connection identifier from the GATT_LLS_ALERT_LEVEL_READ_REQ message.
    @param alert_level The alert level to return to the library.

    @return The status result of calling the API.

*/
void GattLinkLossServerReadLevelResponse(
                                const GLLSS_T *link_loss_server, 
                                uint16 cid, 
                                uint8 alert_level);

#endif /* GATT_LINK_LOSS_SERVER_H */


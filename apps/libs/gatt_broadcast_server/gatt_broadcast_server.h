/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */
/*
FILE NAME
    gatt_broadcast_server.h

DESCRIPTION
    Interface to the Broadcast GATT service server.
*/

#ifndef GATT_BROADCAST_SERVER_H_
#define GATT_BROADCAST_SERVER_H_

#include "broadcast.h"

#include <message.h>
#include <bdaddr.h>

/* control debug generation */
#ifdef BROADCAST_SERVER_DEBUG_LIB
#include <stdio.h>
#define BROADCAST_SERVER_DEBUG(x)  printf x
#else
#define BROADCAST_SERVER_DEBUG(x)
#endif

/*! Major version of the Broadcast service.
 
    Changes to the broadcaster which break compatibility for
    existing receivers require update to a higher major version
    number.
 */
#define BROADCAST_SERVICE_VERSION_MAJOR   (0x03)

/*! Minor version of the Broadcast service.
 
    Changes to the broadcaster which do not break compatibility for
    existing receivers, such as adding new features, require update
    to a higher minor version number.
 */
#define BROADCAST_SERVICE_VERSION_MINOR   (0x00)

/*! @brief Definition of the Broadcast Server.
 
    There is a task and message handler for the Broadcast server,
    and a reference to the client task, which in this case is the
    broadcast library.
 */
typedef struct
{
    /*! Task for this broadcast service server instance. */
    TaskData service_task;

    /*! Task of the client library or app. */
    Task client_task;

    /*! Flag indicating if association is active, i.e. service characteristics
     * can be read. */
    bool association_active;

    /*! Configured information to be provided in service characteristics. */
    broadcast_assoc_data assoc_data;
} gatt_broadcast_server;

/*! Opaque definition of the Broadacst Server. */
typedef gatt_broadcast_server GBSS;

/*! @brief Enumeration of return codes used by the Broadcast server.
 */
typedef enum
{
    /*! Operation succeeded. */
    gatt_broadcast_server_status_success,

    /*! Registration with GATT Manager failed. */
    gatt_broadcast_server_status_registration_failed,

    /*! Invalid parameter during initialisation. */
    gatt_broadcast_server_status_invalid_parameter,

    /*! Failed to set the characteristic values. */
    gatt_broadcast_server_status_setup_char_failed,

    /*! Passed Server instance is invalid */
    gatt_broadcast_server_invalid_instance
} gatt_broadcast_server_status;

/*! @brief Initialisation function for the Broadcast server.
 
    @param client_task [IN] Client task, to which messages will be sent.
    @param broadcast_server [IN] Pointer to a gatt_broadcast_server to initialise.
    @param cid [IN] Connection ID.
    @param start_handle [IN] Start of the handle range of the server characteristics.
    @param end_handle [IN] End of the handle range of the server characteristics.

    @return gatt_broadcast_server_status Result of requested operation.
 */
gatt_broadcast_server_status GattBroadcastServerInit(Task clientTask,
                                                     GBSS* const broadcast_server,
                                                     uint16 start_handle,
                                                     uint16 end_handle);

/*! @brief Permit clients to read contents of broadcast server database

    @param GBSS [IN] Pointer to a GATT Broadcast Service Server instance.
    @param enable [IN] TRUE to enable, FALSE to disable client access.

    @return gatt_broadcast_server_status Result of requested operation.
 */
gatt_broadcast_server_status GattBroadcastServerEnable(GBSS* const broadcast_server,
                                                       bool enable);

/*! @brief Setup the contents of the Broadcast server characteristics.

    @param GBSS [IN] Pointer to a GATT Broadcast Service Server instance.
    @param assoc_data [IN] Pointer to association data, to be provided by the service.

    @return gatt_broadcast_server_status Result of requested operation.
 */
gatt_broadcast_server_status GattBroadcastServerSetupCharacteristics(GBSS* const broadcast_server,
                                                                     broadcast_assoc_data* assoc_data);

#endif /* GATT_BROADCAST_SERVER_H_ */

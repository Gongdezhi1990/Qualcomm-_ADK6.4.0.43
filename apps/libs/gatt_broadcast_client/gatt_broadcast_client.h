/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */
/*
FILE NAME
    gatt_broadcast_client.h

DESCRIPTION
    Interface to the Broadcast GATT service client.
*/

#ifndef GATT_BROADCAST_CLIENT_H_
#define GATT_BROADCAST_CLIENT_H_

#include <broadcast.h>

#include <message.h>
#include <bdaddr.h>
#include <library.h>

/* control debug generation */
#ifdef BROADCAST_CLIENT_DEBUG_LIB
#include <stdio.h>
#define BROADCAST_CLIENT_DEBUG(x)  printf x
#else
#define BROADCAST_CLIENT_DEBUG(x)
#endif

/*! @brief Enumeration of Broadcast Characteristics.
 */
typedef enum
{
    broadcast_version_characteristic,
    broadcast_address_characteristic,
    broadcast_status_characteristic,
    broadcast_sec_key_characteristic,
    broadcast_ssr_characteristic,
    broadcast_identifier_characteristic
} gatt_broadcast_characteristics;

/*! @brief Definition of the Broadcast Client.
 
    There is a task and message handler for the Broadcast client,
    and a reference to the client task, which in this case is the
    broadcast library.
 */
typedef struct
{
    /*! Task for the GATT Broadcast Service Client instance. */
    TaskData lib_task;

    /*! Task of user of the client instance. */
    Task app_task;

    /* Flag to track the read characteristics values */
    gatt_broadcast_characteristics current_characteristic_read;
    /*! Association data retrieved from server */
    broadcast_assoc_data assoc_data;
} gatt_broadcast_client;

/*! @brief Opaque definition of the Broadacst Client.
 */
typedef gatt_broadcast_client GBSC;

/*! @brief Enumeration of return codes used by the Broadcast client.
 */
typedef enum
{
    /*! Requested operation successful. */
    gatt_broadcast_client_status_success,

    /*! Client failed to register with GATT Manager. */
    gatt_broadcast_client_status_registration_failed,

    /*! Failed to destroy client instance. */
    gatt_broadcast_client_status_destruction_failed,

    /*! GBSC instance pointer invalid. */
    gatt_broadcast_client_status_invalid_parameter,

    /*! Failed to read the characteristics from the server. */
    gatt_broadcast_client_status_read_failed
} gatt_broadcast_client_status;

/*! @brief Messages sent by a GATT broadcast client to it's client.
 */
typedef enum
{
    /*! Message sent to client_task with info read from Broadcast Server.
        Defined in GATT_BROADCAST_CLIENT_INIT_CFM_T */
    GATT_BROADCAST_CLIENT_INIT_CFM = GATT_BROADCAST_CLIENT_MESSAGE_BASE
} GattBroadcastClientMessageId;

/*! @brief Message sent to broadcast_client client task with server info. */
typedef struct
{
    const GBSC *ba_client;      /*! Reference structure for the instance */
    gatt_broadcast_client_status status;  /*! Status as per gatt_broadcast_client_status */
    /*! Contents of the broadcast service characteristics from server. */
    broadcast_assoc_data* assoc_data;
} GATT_BROADCAST_CLIENT_INIT_CFM_T;

/*!
    @brief Parameters used by the Initialisation API, valid value of these  parameters are must for library initialisation  
*/
typedef struct
{
    uint16 cid;                 /*!Connection ID of the GATT connection on which the server side BA service need to be accessed*/
    uint16 start_handle;        /*! The first handle of BA service need to be accessed*/
    uint16 end_handle;          /*!The last handle of BA service need to be accessed */
} GATT_BROADCAST_CLIENT_INIT_PARAMS_T;

/*! @brief Initialisation function for the Broadcast client.
 
    @param client_task [IN] Client task, to which messages will be sent.
    @param broadcast_client [IN] Pointer to a gatt_broadcast_client.
    @param params as defined in GATT_BROADCAST_CLIENT_INIT_PARAMS_T, it is must all the parameters are valid
                The memory allocated for GATT_BROADCAST_CLIENT_INIT_PARAMS_T can be freed once the API returns.

    @return gatt_broadcast_client_status Result of the requested operation.
 */
gatt_broadcast_client_status GattBroadcastClientInit(Task app_task,
                                                     GBSC* const broadcast_client,
                                                     const GATT_BROADCAST_CLIENT_INIT_PARAMS_T* params);

/*! @brief Destruction function for the Broadcast client.
 
    Used when no longer required by the client_task or when the
    connection has gone.
    
    @param broadcast_client [IN] Pointer to the gatt_broadcast_client to destroy.

    @return gatt_broadcast_client_status Result of the requested operation.
 */
gatt_broadcast_client_status GattBroadcastClientDestroy(GBSC* const broadcast_client);

#endif /* GATT_BROADCAST_CLIENT_H_ */

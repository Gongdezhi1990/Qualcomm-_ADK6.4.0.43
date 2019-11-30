/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_client.h
    
DESCRIPTION
    Header file for the GATT Client library.
*/


/*!
@file    gatt_client.h
@brief   Header file for the GATT Client library.

        This file provides documentation for the GATT Client library
        API (library name: gatt_client).
*/

#ifndef GATT_CLIENT_H
#define GATT_CLIENT_H

#include <csrtypes.h>
#include <message.h>

#include "library.h"
#include "gatt.h"

/* Invalid start handle */
#define INVALID_GATT_START_HANDLE   (0xffff)
/*  Invalid end handle */
#define INVALID_GATT_END_HANDLE   (0x0)
/*!
    @brief Macro to check init params are valid or not
*/
#define CLIENT_INIT_PARAM_INVALID(param) ((param != NULL) && (param->cid ==INVALID_CID ||param->start_handle == INVALID_GATT_START_HANDLE || param->end_handle == INVALID_GATT_END_HANDLE ))

/*!
    @brief persistent data for each known Gatt server device.

    Each Gatt server device that is bonded can have data associated against
    it so that re-connections are much faster in that no GATT discovery is 
    required.
*/
typedef struct
{
    uint16 service_changed_handle;
} gatt_client_device_data_t;

/*!
    @brief Client library private data.
    
    This structure is made public to application as application is responsible for managing resources 
    for the elements of this structure. The data elements are intended for use by the Client lib only. 
    Application SHOULD NOT access (read/write) any elements of this library structure at any point of time and doing so  
    may cause undesired behaviour of this library
*/
typedef struct __GGATTC
{
    TaskData lib_task;
    Task app_task;
    uint16 service_changed_handle;
    bool discovery_error;
} GGATTC;

/*! @brief Enumeration of messages a client task may receive from the client library.
 */
typedef enum __gatt_client_message_id
{
    GATT_CLIENT_INIT_CFM = GATT_CLIENT_MESSAGE_BASE,    /* 00 */
    GATT_CLIENT_SERVICE_CHANGED_IND,                    /* 01 */

    /* Library message limit */
    GATT_CLIENT_MESSAGE_TOP
} gatt_client_message_id_t;

/*!
    @brief Enumeration for client status code  
*/
typedef enum __gatt_client_status
{
    gatt_client_status_success,             /*! Request was a success*/
    gatt_client_status_invalid_parameter,   /*! Invalid parameter was supplied*/
    gatt_client_status_discovery_error,     /*! Error in discovery of Characteristics*/
    gatt_client_status_failed               /*! Request has failed*/
} gatt_client_status_t;

/*!
    @brief Parameters used by the Initialisation API
*/
typedef struct __gatt_client_init_params
{
     uint16 cid;                /*! Connection ID of the GATT connection on which the server side service needs to be accessed*/
     uint16 start_handle;       /*! The first handle of the service that needs to be accessed*/
     uint16 end_handle;         /*! The last handle of the service that needs to be accessed */
} gatt_client_init_params_t;

/*!
    @brief Gatt client library message sent as a result of calling the GattClientInit API.
*/
typedef struct PACK_STRUCT
{
    const GGATTC *gatt_client;             /*! Reference structure for the instance */
    uint16 service_changed_handle;         /*! Handle of the Service Changed characteristic */
    gatt_client_status_t status;           /*! Status of the initialisation attempt */
} GATT_CLIENT_INIT_CFM_T;

/*!
    @brief Gatt client library message sent when the server sends a Server Changed indication.
*/
typedef struct PACK_STRUCT
{
    const GGATTC *gatt_client;              /*! Reference structure for the instance */
    uint16 cid;                             /*! The connection ID */
    uint16 start_handle;                    /*! Start handle of the changed services */
    uint16 end_handle;                      /*! End handle of the changed services */
} GATT_CLIENT_SERVICE_CHANGED_IND_T;


/*!
    @brief Initialises the Gatt Client Library.

     NOTE: This interface need to be invoked for every new gatt connection that wishes to use 
     the Gatt client library.

    @param app_task The Task that will receive the messages sent from this immediate alert client library
    @param gatt_client A valid area of memory that the service library can use. Must be of at least the size of GGATTC
    @param client_init_params Mandatory initialisation parameters
    @param device_data Persistent data for the device
    
    @return The status result of calling the API defined in gatt_client_status_t

    NOTE: A gatt_client_status_t code will be received as indication that the client library initialisation started. Once completed
     GATT_CLIENT_INIT_CFM will be received with a gatt_client_status_t status code.

*/
gatt_client_status_t GattClientInit(Task app_task,
                                    GGATTC *const gatt_client, 
                                    const gatt_client_init_params_t *client_init_params,
                                    const gatt_client_device_data_t *device_data);


/*!
    @brief When a GATT connection is removed, the application must remove all client service instances that were
    associated with the connection (using the CID value).
    This is the clean up routine as a result of calling the GattClientInit API. That is,
    the GattClientInit API is called when a connection is made, and the GattClientDestroy is called 
    when the connection is removed.

    @param gatt_client The client instance that was passed into the GattClientInit API.
    @param cid The connection ID.

    @return The status result of calling the API.

*/
gatt_client_status_t GattClientDestroy(GGATTC *const gatt_client, uint16 cid);


#endif /* GATT_CLIENT_H */


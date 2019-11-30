/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/* Part of 6.2 */

/*!
@file    gatt_ama_server.h
@brief   Header file for the GATT ama server library.

        This file provides documentation for the GATT ama server library
        API (library name: gatt_ama_server).
*/

#ifndef GATT_AMA_SERVER_H_
#define GATT_AMA_SERVER_H_

#include <csrtypes.h>
#include <message.h>

#include <library.h>

#include "gatt_manager.h"

/*! @brief The ama server internal structure for the server role.

    This structure is visible to the application as it is responsible for managing resource to pass to the ama library.
    The elements of this structure are only modified by the ama library.
    The application SHOULD NOT modify the values at any time as it could lead to undefined behaviour.

 */
typedef struct __GAMASS
{
    TaskData lib_task;
    Task app_task;
    bool notifications_enabled;


    typed_bdaddr            taddr;
    /*! Connection configuration flags. */
    uint16                  flags;
    /*! Connection identifier of remote device. */
    uint16                  cid;
    /*! Connection MTU. */
    uint16                  mtu;

	tp_bdaddr  				peer_public_addr;

}GAMASS ;


/*!
    @brief Status code returned from the GATT ama server library

    This status code indicates the outcome of the request.
*/
typedef enum
{
    gatt_ama_server_status_success,
    gatt_ama_server_status_registration_failed,
    gatt_ama_server_status_invalid_parameter,
    gatt_ama_server_status_not_allowed,
    gatt_ama_server_status_failed
} gatt_ama_server_status_t;


typedef struct __GATT_AMA_SERVER_NOTIFICATION_CFM
{
    const GAMASS *ama;    /*! Reference structure for the instance  */
	gatt_status_t	status;
	uint16			cid;
	uint16			handle;
} GATT_AMA_SERVER_NOTIFICATION_CFM_T;


typedef struct __GATT_AMA_SERVER_WRITE_IND
{
    const GAMASS *ama;    /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
    uint16 handle;
    uint16 length;        /*! Client Configuration value to be written */
    uint8* data;
    uint16 value;
}GATT_AMA_SERVER_WRITE_IND_T;


typedef struct __GATT_AMA_SERVER_CLIENT_C_CFG
{
    const GAMASS *ama;    /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
    uint16 handle;
    uint16 client_config;        /*! Client Configuration value to be written */
}GATT_AMA_SERVER_CLIENT_C_CFG_T;




/*! @brief Enumeration of messages an application task can receive from the ama server library.
 */
typedef enum
{
    /* Server messages */
    GATT_AMA_SERVER_WRITE_IND = GATT_AMA_SERVER_MESSAGE_BASE,             /* 00 */
    GATT_AMA_SERVER_NOTIFICATION_CFM,	                                  /* 01 */
    GATT_AMA_SERVER_CLIENT_C_CFG,									  /* 01 */
    /* Library message limit */

    GATT_AMA_SERVER_TICK,

    GATT_AMA_SERVER_MESSAGE_TOP
} gatt_ama_server_message_id_t;

/*!
    @brief Optional parameters used by the Initialisation API

    Parameters that can define how the ama server library is initialised.
 */
typedef struct
{
   bool enable_notifications;     /*! Flag that can be used to enable or disable notifications */
} gatt_ama_server_init_params_t;


/*!
    @brief Initialises the Ama Service Library in the Server role.

    @param ama_server pointer to server role data
    @param app_task The Task that will receive the messages sent from this ama server library.
    @param init_params pointer to ama server initialization parameters
    @param start_handle start handle
    @param end_handle end handle

    @return TRUE if successful, FALSE otherwise

*/
bool GattAmaServerInit(GAMASS *ama_server,
                           Task app_task,
                           const gatt_ama_server_init_params_t *init_params,
                           uint16 start_handle,
                           uint16 end_handle);



bool GattAmaServerSendNotification(const GAMASS *ama, uint16 cid, uint16 handle,
                                   uint16 length, uint8 *data);



bool SendAmaNotification(uint8 *data, uint16 length);

#endif

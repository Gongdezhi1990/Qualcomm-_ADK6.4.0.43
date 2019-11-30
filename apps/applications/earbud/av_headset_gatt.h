/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_gatt.h
\brief      Header file for GATT, GATT Server and GAP Server
*/

#ifndef _AV_HEADSET_GATT_H_
#define _AV_HEADSET_GATT_H_

#include "av_headset.h"

#include <gatt_server.h>
#include <gatt_gap_server.h>
#include <gatt_battery_server.h>


/*! Messages sent by the av_headset_gatt module */
typedef enum {
    APP_GATT_INIT_CFM = APP_GATT_MESSAGE_BASE,  /*!< Application GATT module has initialised */
    APP_GATT_CONNECTABLE,                       /*!< This device is advertising for a connection */
    APP_GATT_CONNECTION_MADE,                   /*!< A BLE connection has been made */
    APP_GATT_CONNECTION_DROPPED,                /*!< A BLE connection has been dropped */
} av_headet_gatt_messages;


/*! GATT Client Configuration attributes that need to be stored per remote client 

    @note This is a server only structure
*/
typedef struct __gatt_ccd_attributes_t
{
    uint16 gatt;            /*!< Attribute data for the gatt server */
#ifdef INCLUDE_GATT_BATTERY_SERVER
    uint16 battery_left;    /*!< Attribute data for the battery server for the left battery */
    uint16 battery_right;   /*!< Attribute data for the battery server for the right battery */
#endif
} gatt_config_values_t;


/*! Structure used per server instance. */
typedef struct
{
    /*! GATT connection ID

        This is used in many messages to distinguish the GATT instance */
    uint16      conn_id;

    /*! Client Service Configuration set by by the client */
    gatt_config_values_t   config;

    /*! Gatt Server information. Required as we're using BLE */
    GGATTS      gatt_server;

    /*! GAP server information. Required for using BLE */
    GGAPS       gap_server;

#ifdef INCLUDE_GATT_BATTERY_SERVER
    GBASS       battery_server_left;
    GBASS       battery_server_right;
#endif
} gattGattServerInfo;


/*! Structure holding information for the gatt task */
typedef struct
{
    /*! Task for handling messaging from GATT Manager */
    TaskData    gatt_task;

    /*! Task for handling battery related messages */
    TaskData    gatt_battery_task;

    /*! Advertising rate to use when connecting */
    appConfigBleAdvertisingMode     advertising_mode;

    /*! Settings for the current advertisement being used.
        The gatt module only uses one advert */
    advManagerAdvert *advert_settings;

    /*! Data for each of the simultaneously supported BLE connections */
    gattGattServerInfo  instance[appConfigBleGetGattServerInstances()];
} gattTaskData;

COMPILE_TIME_ASSERT(1==appConfigBleGetGattServerInstances(),double_check_multiple_gatt_instances);

/*! Start a BLE connection

    Starts advertising to make a new connection as a peripheral. The message
    APP_GATT_CONNECTABLE will be sent when advertising is started. And 
    APP_GATT_CONNECTION_MADE if a connection is made.

    \returns FALSE if there is an existing connection, TRUE otherwise
 */
bool appGattGoConnectable(void);

/*! Change whether gatt connections are now allowed and advertise/connect as appropriate

    Update the permission for BLE connections. If BLE connections are allowed
    and the application logic currently allows a connection then advertising will
    be started.

    If there is an existin BLE connection, or advertising is in progress, and 
    connections are no longer allowed - then the BLE activity will be stopped.

    \param allow Enable or disable BLE connection

    \return FALSE if no change was made.
*/
bool appGattAllowBleConnections(bool allow);

/*! Does the GATT module have a BLE connection 

    \return TRUE if there is a BLE connection */
#define appGattHasBleConnection()           (0 != appGetGatt()->instance[0].conn_id)

/*! Is the GATT module trying to make a BLE connection

    \return TRUE if a connection attempt is in progress */
#define appGattAttemptingBleConnection()    (NULL != appGetGatt()->advert_settings)


/*! Set the advertising mode/rate for the next connection

    This value does not take effect if there is currently advertising
    in process.

    \param mode The advertising mode requested
 */
void appGattSetAdvertisingMode(appConfigBleAdvertisingMode mode);


/*! Handler for connection library messages not sent directly

    This function is called to handle any connection library messages sent to
    the application that the GATT module is interested in. If a message 
    is processed then the function returns TRUE.

    \note Some connection library messages can be sent directly as the 
        request is able to specify a destination for the response.

    \param  id              Identifier of the connection library message 
    \param  message         The message content (if any)
    \param  already_handled Indication whether this message has been processed by
                            another module. The handler may choose to ignore certain
                            messages if they have already been handled.

    \returns TRUE if the message has been processed, otherwise returns the
        value in already_handled
 */
bool appGattHandleConnectionLibraryMessages(MessageId id, Message message, bool already_handled);


/*! @brief Initialise the GATT component. 
*/
void appGattInit(void);


#endif /* _AV_HEADSET_GATT_H_ */

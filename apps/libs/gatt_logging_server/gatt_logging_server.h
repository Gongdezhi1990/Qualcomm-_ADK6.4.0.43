#ifndef GATT_LOGGING_SERVER_H
#define GATT_LOGGING_SERVER_H

#include <csrtypes.h>
#include <message.h>

#include <library.h>

#include "gatt_manager.h"

/*! @brief Logging Service library  data structure type .
 */

#define LOGGING_DEBUG_CONFIGURATION_DATA_SIZE   4

/* This structure is made public to the application as it is responsible for
 * managing resources for the elements of this structure. The data elements are
 * intended for use by the Logging Service lib only.
 * The Application SHOULD NOT access (read/write) any elements of this library
 * structure at any point in time and doing so may cause undesired behavior.
 */
typedef struct _gatt_logging_server_t
{
    TaskData lib_task;
    Task app_task;
    uint8  logging_control;
}_gatt_logging_server_t;

/*! @brief GATT Logging Service[GLOG]Library Instance.
 */
typedef struct  _gatt_logging_server_t GLOG_T;

/*! @brief Contents of the GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND message that
    is sent by the library, due to a read of the LOGGING client configuration characteristic.
 */
typedef struct __GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND
{
    const GLOG_T *logging;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
} GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND message that
    is sent by the library, due to a write of the LOGGING client configuration characteristic.
 */
typedef struct __GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND
{
    const GLOG_T *logging;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
    uint16 config_value;        /*! Client Configuration value to be written */
} GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND message that is
    sent by the library, due to a read of the Logging Control
    characteristic value.
 */
typedef struct __GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND
{
    const GLOG_T *logging;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
} GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND_T;

/*! @brief Contents of the GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND message that is
    sent by the library, due to a read of the Logging Control
    characteristic value.
 */
typedef struct __GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND
{
    const GLOG_T *logging;        /*! Reference structure for the instance  */
    uint16 cid;                  /*! Connection ID */
    uint8  logging_control;        /*! Logging Control value to be written */
} GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND_T;


/*! @brief Contents of the GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND message
 *  that is sent by the library, due to a read of the logging debug configuration
    characteristic value.
 */
typedef struct __GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND
{
    const GLOG_T *logging;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
} GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND_T;

/*! @brief Contents of the GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND message
 *  that is sent by the library, due to a write to the characteristic value.
 */
typedef struct __GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND
{
    const GLOG_T *logging;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
    uint8 p_debug_configuration[LOGGING_DEBUG_CONFIGURATION_DATA_SIZE];
    /*! Personal measurements */
} GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND_T;

/*! @brief Enumeration of messages an application task can receive from the
    Logging Service library.
 */
 typedef enum
{
    /* Server messages */
    GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND = GATT_LOGGING_SERVER_MESSAGE_BASE,  /* 00 */
    GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND,                                    /* 01 */
    GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND,                                   /* 02 */
    GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND,                                  /* 03 */
    GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND,                               /* 04 */
    GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND,                              /* 05 */

    /* Library message limit */
    GATT_LOGGING_SERVER_MESSAGE_TOP
} gatt_logging_server_message_id_t;

/*!
    @brief  Initializes the Logging Service Library.

    @param  appTask     The Task that will receive the messages sent from this
                        Logging Service library.

    @param  logging     A valid area of memory that the Logging Service library can use.
                        Must be of at least the size of GLOG_T

    @param  start_handle This indicates the start handle of the Logging Service

    @param  end_handle   This indicates the end handle of the Logging Service

    @return TRUE if success, FALSE otherwise.

*/
bool GattLoggingServerInit(Task appTask, GLOG_T *const logging, uint16 start_handle, uint16 end_handle);

/*!
    @brief  This API is used to return a Logging client configuration value to the library when a
            GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND message is received.

    @param  logging         The pointer that was in the payload of the
                            GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND message.

    @param  cid             The connection identifier from the
                            GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND message.

    @param  client_config   The client configuration to return to the library

    @return TRUE if success, FALSE otherwise

*/
bool GattLoggingServerReadClientConfigResponse(const GLOG_T *logging, uint16 cid, uint16 client_config);

/*!
    @brief  This API is used to return a Logging control value to the library when a
            GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND message is received.

    @param  logging         The pointer that was in the payload of the
                            GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND message.

    @param  cid             The connection identifier from the
                            GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND message.

    @param  logging_control  The logging control to return to the library

    @return TRUE if success, FALSE otherwise

*/
bool GattLoggingServerReadLoggingControlResponse(const GLOG_T *logging, uint16 cid, uint8 logging_control);

/*!
    @brief  This API is used to notify a remote Logging client
            of the Logging measurement value. This will only
            be allowed if notifications have been enabled by the remote device.

    @param  logging     The instance pointer that was passed into the
                        GattLoggingServerInit API.

    @param  cid         The connection identifier from the
                        GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND message.

    @param  log_length The length of the logging

    @param  log_data   The Logging measurement value to send
                        in the notification. The logging
                        data is as follows

     *  --------------------------------------------------------------------------------------------------------------------
     *  | Flags | Instantaneous Speed | Instantaneous Cadence | Instantaneous Stride Length | Total Distance |-----------
     *  --------------------------------------------------------------------------------------------------------------------
     *      Flags field:
     *              Bit 0:  Instantaneous Stride Length Field Present,
                            0 - Not Present, 1 - Present
     *              Bit 1:  Total Distance Field Present,
                            0 - Not Present, 1 - Present
     *              Bit 2:  Walking or Running Feature, 0 - Not Supported or
                            user is walking, 1 - User is running
     *              Bit 3 - 7: RFU (Reserved for future use must be set to 0)
     *
     *      Instantaneous Speed Field:
     *              The instantaneous speed of the user, as measured by the RSC sensor. This is a mandatory field.
     *
     *      Instantaneous Cadence Field:
     *              This field represents the number of times per minute a foot fall occurs. This is a mandatory field.
     *
     *      Instantaneous Stride Length:
     *              This field represents the distance between two successive
     *              contacts of the same foot to the ground. This field is only
     *              present if Bit 0 of the Flags field is 1.
     *

    @return TRUE if success, FALSE otherwise

*/
bool GattLoggingServerSendNotification(const GLOG_T *logging, uint16 cid, uint16 log_length, uint8 *log_data);

/*!
    @brief  This API is used to return a Logging Debug Configuration data to the library when a
            GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND message is received.

    @param  logging     The instance pointer that was passed into the
                        GattLoggingServerInit API.

    @param  cid         The connection identifier from the
                        GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND message.

    @param  debug_config_length   The length of the Logging Debug Configuration data.

    @param  debug_config_data     The Logging Debug Configuration bitmask that is used by the algorithms

    @return TRUE if success, FALSE otherwise
*/
bool GattLoggingServerReadLoggingDebugConfigurationResponse(const GLOG_T *logging, uint16 cid, uint16 debug_config_length, uint8 *debug_config_data);

#endif /* GATT_LOGGING_SERVER_H */

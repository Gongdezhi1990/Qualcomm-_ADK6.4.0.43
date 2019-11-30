/*****************************************************************************
Copyright (c) 2011 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt.h

DESCRIPTION
    Header file for the GATT library
*/

/*!
\defgroup gatt gatt
\ingroup vm_libs

\brief      Header file for the GATT library

\section gatt_intro INTRODUCTION
    This file provides documentation for the GATT library API.

@{

*/


#ifndef GATT_H_
#define GATT_H_

#include <bdaddr_.h>
#include <library.h>
#include <message_.h>


/*****************************************************************************
GATT Service UUID's see (https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx)
*****************************************************************************/
#define GATT_SERVICE_UUID_ALERT_NOTIFICATION_SERVICE                        (0x1811)
#define GATT_SERVICE_UUID_BATTERY_SERVICE                                   (0x180F)
#define GATT_SERVICE_UUID_BLOOD_PRESSURE                                    (0x1810)
#define GATT_SERVICE_UUID_CURRENT_TIME_SERVICE                              (0x1805)
#define GATT_SERVICE_UUID_CYCLING_POWER                                     (0x1818)
#define GATT_SERVICE_UUID_CYCLING_SPEED_AND_CADENCE                         (0x1816)
#define GATT_SERVICE_UUID_DEVICE_INFORMATION                                (0x180A)
#define GATT_SERVICE_UUID_GENERIC_ACCESS                                    (0x1800)
#define GATT_SERVICE_UUID_GENERIC_ATTRIBUTE                                 (0x1801)
#define GATT_SERVICE_UUID_GLUCOSE                                           (0x1808)
#define GATT_SERVICE_UUID_HEALTH_THERMOMETE                                 (0x1809)
#define GATT_SERVICE_UUID_HEART_RATE                                        (0x180D)
#define GATT_SERVICE_UUID_HUMAN_INTERFACE_DEVICE                            (0x1812)
#define GATT_SERVICE_UUID_IMMEDIATE_ALERT                                   (0x1802)
#define GATT_SERVICE_UUID_LINK_LOSS                                         (0x1803)
#define GATT_SERVICE_UUID_LOCATION_AND_NAVIGATION                           (0x1819)
#define GATT_SERVICE_UUID_NEXT_DSG_CHANGE_SERVICE                           (0x1807)
#define GATT_SERVICE_UUID_PHONE_ALERT_STATUS_SERVICE                        (0x180E)
#define GATT_SERVICE_UUID_REFERENCE_TIME_UPDATE_SERVICE                     (0x1806)
#define GATT_SERVICE_UUID_RUNNING_SPEED_AND_CADENCE                         (0x1814)
#define GATT_SERVICE_UUID_SCAN_PARAMETERS                                   (0x1813)
#define GATT_SERVICE_UUID_TX_POWER                                          (0x1804)


/*****************************************************************************
GATT Characteristic UUID's see (https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicsHome.aspx)
*****************************************************************************/
#define GATT_CHARACTERISTIC_UUID_ALERT_CATEGORY_ID                          (0x2A43)
#define GATT_CHARACTERISTIC_UUID_ALERT_CATEGORY_ID_BIT_MASK                 (0x2A42)
#define GATT_CHARACTERISTIC_UUID_ALERT_LEVEL                                (0x2A06)
#define GATT_CHARACTERISTIC_UUID_ALERT_NOTIFICATION_CONTROL_POINT           (0x2A44)
#define GATT_CHARACTERISTIC_UUID_ALERT_STATUS                               (0x2A3F)
#define GATT_CHARACTERISTIC_UUID_APPERANCE                                  (0x2A01)
#define GATT_CHARACTERISTIC_UUID_BATTERY_LEVEL                              (0x2A19)
#define GATT_CHARACTERISTIC_UUID_BLOOD_PRESSURE_FEATURE                     (0x2A49)
#define GATT_CHARACTERISTIC_UUID_BLOOD_PRESSURE_MEASUREMENT                 (0x2A35)
#define GATT_CHARACTERISTIC_UUID_BODY_SENSOR_LOCATION                       (0x2A38)
#define GATT_CHARACTERISTIC_UUID_BOOT_KEYBOARD_INPUT_REPORT                 (0x2A22)
#define GATT_CHARACTERISTIC_UUID_BOOT_KEYBOARD_OUTPUT_REPORT                (0x2A32)
#define GATT_CHARACTERISTIC_UUID_BOOT_MOUSE_INPUT_REPORT                    (0x2A33)
#define GATT_CHARACTERISTIC_UUID_CLIENT_SUPPORTED_FEATURES                  (0x2B29)
#define GATT_CHARACTERISTIC_UUID_CSC_FEATURE                                (0x2A5C)
#define GATT_CHARACTERISTIC_UUID_CSC_MEASUREMENT                            (0x2A5B)
#define GATT_CHARACTERISTIC_UUID_CURRENT_TIME                               (0x2A2B)
#define GATT_CHARACTERISTIC_UUID_CYCLING_POWER                              (0x2A66)
#define GATT_CHARACTERISTIC_UUID_CYCLING_POWER_FEATURE                      (0x2A65)
#define GATT_CHARACTERISTIC_UUID_CYCLING                                    (0x2A63)
#define GATT_CHARACTERISTIC_UUID_CYCLING_POWER_VECTOR                       (0x2A64)
#define GATT_CHARACTERISTIC_UUID_DATABASE_HASH                              (0x2B2A)
#define GATT_CHARACTERISTIC_UUID_DATE_TIME                                  (0x2A08)
#define GATT_CHARACTERISTIC_UUID_DAY_DATE_TIME                              (0x2A0A)
#define GATT_CHARACTERISTIC_UUID_DAY_OF_WEEK                                (0x2A09)
#define GATT_CHARACTERISTIC_UUID_DEVICE_NAME                                (0x2A00)
#define GATT_CHARACTERISTIC_UUID_DST_OFFSET                                 (0x2A0D)
#define GATT_CHARACTERISTIC_UUID_EXACT_TIME_256                             (0x2A0C)
#define GATT_CHARACTERISTIC_UUID_FIRMWARE_REVISION_STRING                   (0x2A26)
#define GATT_CHARACTERISTIC_UUID_GLUCOSE_FEATURE                            (0x2A51)
#define GATT_CHARACTERISTIC_UUID_GLUCOSE_MEASUREMENT                        (0x2A18)
#define GATT_CHARACTERISTIC_UUID_GLUCOSE_MEASUREMENT_CONTEXT                (0x2A34)
#define GATT_CHARACTERISTIC_UUID_HARDWARE_REVISION_STRING                   (0x2A27)
#define GATT_CHARACTERISTIC_UUID_HEART_RATE_CONTROL_POINT                   (0x2A39)
#define GATT_CHARACTERISTIC_UUID_HEART_RATE_MEASUREMENT                     (0x2A37)
#define GATT_CHARACTERISTIC_UUID_HID_CONTROL_POINT                          (0x2A4C)
#define GATT_CHARACTERISTIC_UUID_HID_INFORMATION                            (0x2A4A)
#define GATT_CHARACTERISTIC_UUID_IEEE_11073                                 (0x2A2A)
#define GATT_CHARACTERISTIC_UUID_INTERMEDIATE_CUFF_PRESSURE                 (0x2A36)
#define GATT_CHARACTERISTIC_UUID_INTERMEDIATE_TEMPERATURE                   (0x2A1E)
#define GATT_CHARACTERISTIC_UUID_LN                                         (0x2A6B)
#define GATT_CHARACTERISTIC_UUID_LN_FEATURE                                 (0x2A6A)
#define GATT_CHARACTERISTIC_UUID_LOCAL_TIME_INFORMATION                     (0x2A0F)
#define GATT_CHARACTERISTIC_UUID_LOCATION                                   (0x2A67)
#define GATT_CHARACTERISTIC_UUID_MANUFACTURER_NAME_STRING                   (0x2A29)
#define GATT_CHARACTERISTIC_UUID_MEASUREMENT_INTERVAL                       (0x2A21)
#define GATT_CHARACTERISTIC_UUID_MODEL_NUMBER_STRING                        (0x2A24)
#define GATT_CHARACTERISTIC_UUID_NAVIGATION                                 (0x2A68)
#define GATT_CHARACTERISTIC_UUID_NEW_ALERT                                  (0x2A46)
#define GATT_CHARACTERISTIC_UUID_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS (0x2A04)
#define GATT_CHARACTERISTIC_UUID_PERIPHERAL_PRIVACY_FLAG                    (0x2A02)
#define GATT_CHARACTERISTIC_UUID_PNP_ID                                     (0x2A50)
#define GATT_CHARACTERISTIC_UUID_POSITION_QUALITY                           (0x2A69)
#define GATT_CHARACTERISTIC_UUID_PROTOCOL_MODE                              (0x2A4E)
#define GATT_CHARACTERISTIC_UUID_RECONNECTION_ADDRESS                       (0x2A03)
#define GATT_CHARACTERISTIC_UUID_RECORD_ACCESS_CONTROL_POINT                (0x2A52)
#define GATT_CHARACTERISTIC_UUID_REFERENCE_TIME_INFORMATION                 (0x2A14)
#define GATT_CHARACTERISTIC_UUID_REPORT                                     (0x2A4D)
#define GATT_CHARACTERISTIC_UUID_REPORT_MAP                                 (0x2A4B)
#define GATT_CHARACTERISTIC_UUID_RINGER_CONTROL_POINT                       (0x2A40)
#define GATT_CHARACTERISTIC_UUID_RINGER_SETTING                             (0x2A41)
#define GATT_CHARACTERISTIC_UUID_RSC_FEATURE                                (0x2A54)
#define GATT_CHARACTERISTIC_UUID_RSC_MEASUREMENT                            (0x2A53)
#define GATT_CHARACTERISTIC_UUID_SC_CONTROL_POINT                           (0x2A55)
#define GATT_CHARACTERISTIC_UUID_SCAN_INTERVAL_WINDOW                       (0x2A4F)
#define GATT_CHARACTERISTIC_UUID_SCAN_REFRESH                               (0x2A31)
#define GATT_CHARACTERISTIC_UUID_SENSOR_LOCATION                            (0x2A5D)
#define GATT_CHARACTERISTIC_UUID_SERIAL_NUMBER_STRING                       (0x2A25)
#define GATT_CHARACTERISTIC_UUID_SERVICE_CHANGED                            (0x2A05)
#define GATT_CHARACTERISTIC_UUID_SOFTWARE_REVISION_STRING                   (0x2A28)
#define GATT_CHARACTERISTIC_UUID_SUPPORTED_NEW_ALERT_CATEGORY               (0x2A47)
#define GATT_CHARACTERISTIC_UUID_SUPPORTED_UNREAD_ALERT_CATEGORY            (0x2A48)
#define GATT_CHARACTERISTIC_UUID_SYSTEM_ID                                  (0x2A23)
#define GATT_CHARACTERISTIC_UUID_TEMPERATURE_MEASUREMENT                    (0x2A1C)
#define GATT_CHARACTERISTIC_UUID_TEMPERATURE_TYPE                           (0x2A1D)
#define GATT_CHARACTERISTIC_UUID_TIME_ACCURACY                              (0x2A12)
#define GATT_CHARACTERISTIC_UUID_TIME_SOURCE                                (0x2A13)
#define GATT_CHARACTERISTIC_UUID_TIME_UPDATE_CONTROL_POINT                  (0x2A16)
#define GATT_CHARACTERISTIC_UUID_TIME_UPDATE_STATE                          (0x2A17)
#define GATT_CHARACTERISTIC_UUID_TIME_WITH_DST                              (0x2A11)
#define GATT_CHARACTERISTIC_UUID_TIME_ZONE                                  (0x2A0E)
#define GATT_CHARACTERISTIC_UUID_TX_POWER_LEVEL                             (0x2A07)
#define GATT_CHARACTERISTIC_UUID_UNREAD_ALERT_STATUS                        (0x2A45)


/*****************************************************************************
Characteristic Declaration UUIDs see (https://developer.bluetooth.org/gatt/declarations/Pages/DeclarationsHome.aspx)
*****************************************************************************/
#define GATT_CHARACTERISTIC_DECLARATION_UUID                                (0x2803)
#define GATT_INCLUDE_DECLARATION_UUID                                       (0x2802)
#define GATT_PRIMARY_SERVICE_DECLARATION_UUID                               (0x2800)
#define GATT_SECONDARY_SERVICE_DECLARATION_UUID                             (0x2801)


/*****************************************************************************
Characteristic Descriptor UUIDs see (https://developer.bluetooth.org/gatt/descriptors/Pages/DescriptorsHomePage.aspx)
*****************************************************************************/
#define GATT_CHARACTERISTIC_EXTENDED_PROPERTIES_UUID                        (0x2900)
#define GATT_CHARACTERISTIC_USER_DESCRIPTION_UUID                           (0x2901)
#define GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_UUID                       (0x2902)
#define GATT_CHARACTERISTIC_PRESENTATION_FORMAT_UUID                        (0x2904)
#define GATT_CHARACTERISTIC_AGGREGATE_FORMAT_UUID                           (0x2905)
#define GATT_VALID_RANGE_UUID                                               (0x2906)
#define GATT_EXTERNAL_REPORT_REFERENCE_UUID                                 (0x2907)
#define GATT_REPORT_REFERENCE_UUID                                          (0x2908)


/*****************************************************************************
Characteristic properties (as defined in the Bluetooth Core Specification)
*****************************************************************************/
typedef enum
{
    gatt_char_prop_broadcast          = 0x1,      /* If set, permits broadcast of the characteristic value using characteristic configuration descriptor */
    gatt_char_prop_read               = 0x2,      /* If set, permits reads of the characteristic value */
    gatt_char_prop_write_no_response  = 0x4,      /* If set, permits writes of the characteristic value without response */
    gatt_char_prop_write              = 0x8,      /* If set, permits writes of the characteristic value with response */
    gatt_char_prop_notify             = 0x10,     /* If set, permits notifications of a characteristic value without acknowledgement*/
    gatt_char_prop_indicate           = 0x20,     /* If set, permits indications of a characteristic value with acknowledgement */
    gatt_char_prop_auth_signed_writes = 0x40,     /* If set, permits signed writes to the characteristic value */
    gatt_char_prop_extended_props     = 0x80      /* If set, additional characteristic properties are defined in the characteristic extended properties descriptor */
} gatt_characteristic_properties_t;


/*****************************************************************************
Report Reference - report characteristic types (as defined in the Bluetooth Core Specification)
*****************************************************************************/
typedef enum
{
    gatt_report_reference_reserved       = 0,     /* Key value '0' is reserved */
    gatt_report_reference_input_report   = 1,     /* If set, the associated report characteristic is an input report */
    gatt_report_reference_output_report  = 2,     /* If set, the associated report characteristic is an output report */
    gatt_report_reference_feature_report = 3      /* If set, the associated report characteristic is a feature report */
    /* Key values: 4-255 are also reserved */
} gatt_report_reference_report_types_t;


/*****************************************************************************
Client Characteristic Configuration properties (16bit format, as defined in the Bluetooth Core Specification)
*****************************************************************************/
#define GATT_CLIENT_CHAR_CONFIG_SIZE (2)

typedef enum
{
    gatt_client_char_config_notifications_enabled = (1 << 0), /* If set, notifications for client characteristic are enabled */
    gatt_client_char_config_indications_enabled   = (1 << 1)  /* If set, indications for client characteristic are enabled */
} gatt_client_characteristic_configuration_properties_t;


/*****************************************************************************
Constants, enumerations, types
*****************************************************************************/
typedef enum
{
    csf_robust_caching_enable           = 0x01   /* Enables robust caching. 0b00000001 */
} gatt_client_supported_features;

/*!
    \brief GATT library return status 
*/

/*****************************************************************************
These status codes are defined in the Bluetooth Core Specification (Vol 3, Part F, Section 3.4.1.1), and the
Core Specification Supplement (Part B, Section 1.2).
*****************************************************************************/

typedef enum
{
    /*! The operation was successful. */
    gatt_status_success                         = 0x00,
    /*! The attribute handle given was not valid */
    gatt_status_invalid_handle,
    /*! The attribute cannot be read */
    gatt_status_read_not_permitted,
    /*! The attribute cannot be written */
    gatt_status_write_not_permitted,
    /*! The attribute PDU was invalid */
    gatt_status_invalid_pdu,
    /*! The attribute requires an authentication before it can be read or
        written */
    gatt_status_insufficient_authentication,
    /*! Target device doesn't support request */
    gatt_status_request_not_supported,
    /*! Offset specified was past the end of the long attribute */
    gatt_status_invalid_offset,
    /*! The attribute requires authorization before it can be read or
        written */
    gatt_status_insufficient_authorization,
    /*! Too many prepare writes have been queued */
    gatt_status_prepare_queue_full,
    /*! No attribute found within the given attribute handle range. */
    gatt_status_attr_not_found,
    /*! This attribute cannot be read or written using the Read Blob Request
        or Write Blob Requests. */
    gatt_status_not_long,
    /*! The Encryption Key Size used for encrypting this link is
        insufficient. */
    gatt_status_insufficient_encr_key_size,
    /*! The attribute value length is invalid for the operation. */
    gatt_status_invalid_length,
    /*! The attribute request that was requested has encountered an error
        that was very unlikely, and therefore could not be completed as
        requested. */
    gatt_status_unlikely_error,
    /*! The attribute requires encryption before it can be read or written */
    gatt_status_insufficient_encryption,
    /*! The attribute type is not a supported grouping attribute as defined
        by a higher layer specification. */
    gatt_status_unsupported_group_type,
    /*! Insufficient Resources to complete the request. */
    gatt_status_insufficient_resources,
    /*! The server requests the client to rediscover the database. */
    gatt_status_db_out_of_sync,
    /*! The attribute parameter value was not allowed. */
    gatt_status_value_not_allowed,
    
    /*! Error Codes 0x14 - 0x7F are reserved for future use. */
    
    /*! Application error to indicate a attribute request not valid for the
        current radio type */
    gatt_status_application_error = 0x0080,
    
    /*! Error Codes 0x80 - 0x9F are Application error codes defined by a higher layer specification. */
    
    /*! Error Codes 0xA0 - 0xDF are reserved for future use. */
    
    /*! Error Codes 0xE0 - 0xFF are common profile and service error codes defined in the Core Specification Supplement, Part B, Section 1.2. */
    /*! Error Codes 0xE0 - 0xFB are reserved for future use. */
    
    /*! The write request was rejected. */
    gatt_status_write_request_rejected = 0x00FC,
    /*! The Client Characteristic Configuration Descriptor is improperly configured. */
    gatt_status_cccd_improper_config,
    /*! The attempted procedure is already in progress. */
    gatt_status_procedure_already_in_progress,
    /*! The connected device is out of range. */
    gatt_status_out_of_range,
    
    
    /*! Connection is initialising */
    gatt_status_initialising,

    /*! Generic failure status. */
    gatt_status_failure,
    /*! Failed to register with the ATT protocol (initialisation). */
    gatt_status_att_reg_failure,
    /*! ATT Database registration failed (initialisation). */
    gatt_status_att_db_failure,
    /*! Max Number of ATT connections have already been made. */
    gatt_status_max_connections,
    /*! ATT disconnected abnormally (L2CAP Disconnection). */
    gatt_status_abnormal_disconnection,
    /*! ATT disconnected because of Link Loss. */
    gatt_status_link_loss,
    /*! MTU can only be exchanged once per connection. */
    gatt_status_mtu_already_exchanged,
    /*! Characteristic Value returned by the server did not match the
      requested one. */
    gatt_status_value_mismatch,

    /*! Connection was rejected because of PSM */
    gatt_status_rej_psm,
    /*! Connection was rejected because of security */
    gatt_status_rej_security,
    /*! Connection was rejected because of missing link key */    
    gatt_status_key_missing,
    /*! Connection timed out */
    gatt_status_connection_timeout,
    /*! Connection retrying */
    gatt_status_retrying,
    /*! Peer aborted the connection */
    gatt_status_peer_aborted,
    
    /*! Error to indicate that request to DM can not be completed because
        device ACL entity is not found */
    gatt_status_device_not_found = 0x7f73,
    /*! Attribute signing failed. */
    gatt_status_sign_failed,
    /*! Operation can't be done now. */
    gatt_status_busy,
    /*! Current operation timed out. */
    gatt_status_timeout,
    /*! Invalid MTU */
    gatt_status_invalid_mtu,
    /*! Invalid UUID type */
    gatt_status_invalid_uuid,
    /*! Operation was successful, and more responses will follow */
    gatt_status_success_more,
    /*! Indication sent, awaiting confirmation from the client */
    gatt_status_success_sent,
    /*! Invalid connection identifier */
    gatt_status_invalid_cid,
    /*! Attribute database is invalid */
    gatt_status_invalid_db,
    /*! Attribute server database is full */
    gatt_status_db_full,
    /*! Requested server instance is not valid */
    gatt_status_invalid_phandle,
    /*! Attribute permissions are not valid */
    gatt_status_invalid_permissions,    
    /*! Signed write done on an encrypted link */
    gatt_result_signed_disallowed,
#ifdef BUILD_FOR_HOST_FOR_ENCRYPTION_ATT_RACE
    /*! It indicates that encryption is in process */
    gatt_result_encryption_pending,
#endif /* BUILD_FOR_HOST_FOR_ENCRYPTION_ATT_RACE */
    /*! The invalid parameters */
    gatt_result_invalid_params,
    /*! Connection to remote device already exists */
    gatt_status_already_connected
} gatt_status_t;


/*!
    \brief Transport of ATT link
*/
typedef enum
{
    /*! BR/EDR Master. */
    gatt_connection_bredr_master,
    /*! BLE Master Directed. */
    gatt_connection_ble_master_directed,
    /*! BLE Master Whitelist. */
    gatt_connection_ble_master_whitelist,
    /*! BLE Slave Directed High Duty Cycle. */
    gatt_connection_ble_slave_directed_high_duty,
    /*! BLE Slave Directed - for legacy support - same as Directed High Duty. */
    gatt_connection_ble_slave_directed = 
        gatt_connection_ble_slave_directed_high_duty,
    /*! BLE Slave Whitelist. */
    gatt_connection_ble_slave_whitelist,
    /*! BLE Slave Undirected.*/
    gatt_connection_ble_slave_undirected,
    /*! BLE Slave Directed Low Duty Cycle. */
    gatt_connection_ble_slave_directed_low_duty
} gatt_connection_type;

/*! 
    \name GATT Access Permission Flags 

    \{
*/
/*! Request to read attribute value */
#define ATT_ACCESS_READ                         0x0001
/*! Request to write attribute value. */
#define ATT_ACCESS_WRITE                        0x0002
/*! Request for sufficient permission for r/w of attribute value. */
#define ATT_ACCESS_PERMISSION                   0x8000
/*! Indication of completed write, rsp mandatory */
#define ATT_ACCESS_WRITE_COMPLETE               0x4000
/*! Indication that GATT will request APP for enc key len*/
#define ATT_ACCESS_ENCRYPTION_KEY_LEN           0x2000
/*! \} */

/*!
    \brief UUID type definitions for GATT.
*/
typedef enum
{
    /*! UUID is not present */
    gatt_uuid_none,
    /*! UUID is a 16-bit Attribute UUID */
    gatt_uuid16,
    /*! UUID is a 128-bit UUID */
    gatt_uuid128,
    /*! UUID is a 32-bit Attribute UUID */
    gatt_uuid32
} gatt_uuid_type_t;

/*!
    \brief GATT UUID type

    All GATT UUID are stored gatt_uuid_t array of 4 elements and are Big Endian. 
    A 16-bit Attribute UUID and 32-bit UUID are stored in only the first 
    element of the array [0]. 

    For example, the 16-bit UUID 0x2093 is stored as...

        gatt_uuid_t [0] = 0x00002093 
        gatt_uuid_t [1] - ignored
        gatt_uuid_t [2] - ignored
        gatt_uuid_t [3] - ignored
    
    Note: The top 16-bits of the first element must be 0.

    The 32-bit UUID 0xA5054492 is stored as...

        gatt_uuid_t [0] = 0xA5054492
        gatt_uuid_t [1] - ignored
        gatt_uuid_t [2] - ignored
        gatt_uuid_t [3] - ignored
    
    A 128-bit UUID is stored in all four elementents [0..3].

    For example 128-bit UUID bit UUID 00112233-4455-6677-8899-aabbccddeeff is 
    stored as...

        gatt_uuid_t [0] = 0x00112233
        gatt_uuid_t [1] = 0x44556677
        gatt_uuid_t [2] = 0x8899aabb
        gatt_uuid_t [3] = 0xccddeeff

    Wherever the 4 element gatt_uuid_t parameter is used, there is also a
    parameter of the #gatt_uuid_type_t enum type which is used to explicitly 
    declare which UUID type is stored in the array.
*/
typedef uint32 gatt_uuid_t;

#define GATT_UUID_IS_16BIT(uuid) (((uuid) & 0xFFFF0000lu) == 0)


/*! \brief Minimum GATT handle number. */
#define GATT_HANDLE_MIN         0x0001
/*! \brief Maximum GATT handle number. */
#define GATT_HANDLE_MAX         0xffff


/*
    Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT

typedef enum
{
    GATT_INIT_CFM = GATT_MESSAGE_BASE,
    GATT_CONNECT_CFM,
    GATT_CONNECT_IND,
    GATT_DISCONNECT_IND,
    GATT_ACCESS_IND,
    GATT_EXCHANGE_MTU_CFM,
    GATT_EXCHANGE_MTU_IND,
    GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM,
    GATT_DISCOVER_PRIMARY_SERVICE_CFM,
    GATT_DISCOVER_ALL_BREDR_SERVICES_CFM,
    GATT_DISCOVER_BREDR_SERVICE_CFM,
    GATT_FIND_INCLUDED_SERVICES_CFM,
    GATT_DISCOVER_CHARACTERISTIC_CFM,
    GATT_DISCOVER_ALL_CHARACTERISTICS_CFM,    
    GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM,
    GATT_READ_CHARACTERISTIC_VALUE_CFM,
    GATT_READ_USING_CHARACTERISTIC_UUID_CFM,
    GATT_READ_LONG_CHARACTERISTIC_VALUE_CFM,
    GATT_READ_MULTIPLE_CHARACTERISTIC_VALUES_CFM,
    GATT_WRITE_WITHOUT_RESPONSE_CFM,
    GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM,
    GATT_WRITE_CHARACTERISTIC_VALUE_CFM,
    GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM,
    GATT_RELIABLE_WRITE_PREPARE_CFM,
    GATT_RELIABLE_WRITE_EXECUTE_CFM,
    GATT_NOTIFICATION_IND,
    GATT_NOTIFICATION_CFM,
    GATT_INDICATION_IND,
    GATT_INDICATION_CFM,

    /* Library message limit */
    GATT_MESSAGE_TOP
} GattMessageId;

#endif /* end of DO_NOT_DOCUMENT */


/*****************************************************************************
Public API Definition
*****************************************************************************/
#define GATT_FLAGS_DEFAULT  (0)
#define GATT_FLAGS_CONST_DB (1 << 0)

/*!
    \brief This constant depicts the Channel Identifier for accessing data 
    from the local device's GATT database.
*/
#define GATT_CID_LOCAL (0)

/*!
    \name GATT Invalid CID
*/
#define INVALID_CID (0xFFFF)

/*!
    \brief This function is called to initialise the GATT library. The
    connection library must be initialised before the GATT library (this
    function) is called.

    This function is a thin wrapper for GattInitEx and passes
    GATT_FLAGS_DEFAULT as the flags parameter.

    A GATT_INIT_CFM message will be sent indicating if initialising the library
    is successful.

    \param theAppTask The client task.
    \param size_database Size of the GATT database.
    \param database The GATT database to register.

    \return GATT_INIT_CFM_T
*/
void GattInit(Task theAppTask, uint16 size_database, uint16* database);

/*!
    \brief This function is called to initialise the GATT library. The
    connection library must be initialised before the GATT library (this
    function) is called.

    If the GATT database is stored in VM constant memory then this function
    should be used instead of GattInit and GATT_FLAGS_CONST_DB should be
    passed as the flags parameter.

    A GATT_INIT_CFM message will be sent indicating if initialising the library
    is successful.

    \param theAppTask The client task.
    \param size_database Size of the GATT database.
    \param database The GATT database to register.
    \param flags Configuration flags for the GATT service.

    \return GATT_INIT_CFM_T
*/
void GattInitEx(Task theAppTask, uint16 size_database, uint16* database, uint16 flags);

/*!
    \brief GATT library initialisation confirmation
*/
typedef struct 
{
    /*! The GATT library initialisation status.*/
    gatt_status_t     status;
} GATT_INIT_CFM_T;

/*!
    \brief Create an ATT connections to the device indicated. 

    A GATT_CONNECT_CFM message will be received with status 
    gatt_status_initiating. The 'cid' will be valid and can be used to 
    disconnect before fully established. When established a GATT_CONNECT_CFM
    will be received with gatt_status_success. 

    NOTE: A 'gatt_connect_bredr_slave' connection is the exception to the above
    and will not receive the 'gatt_status_initiating' GATT_CONNECT_CFM message.

    \param theAppTask The Task initiating the connection.
    \param taddr The address of the device to make a connection to. This can be
           NULL when the conn_type (see below) is for a slave connection type. 
    \param conn_type The type of ATT connection. 
    \param conn_timeout If TRUE, the attempt will timeout (default 30-seconds).

    The connection attempt time out can be configured using the 
    ConnectionDmSetConnectionParametersUpdateReq() function.

    \return GATT_CONNECT_CFM_T
*/

void GattConnectRequest(
        Task                    theAppTask,
        const typed_bdaddr      *taddr,
        gatt_connection_type    conn_type,
        bool                    conn_timeout
        );

/*!
    \brief GATT Connection confirmation
*/
typedef struct
{
    /*! Connection status. */
    gatt_status_t           status;
    /*! Typed BD_ADDR of remote device. */
    typed_bdaddr            taddr;
    /*! Connection configuration flags. */
    uint16                  flags;
    /*! Connection identifier of remote device. */
    uint16                  cid;
    /*! Connection MTU. */
    uint16                  mtu;
} GATT_CONNECT_CFM_T;

/*!
    \brief GATT Connection Indication

    This message is received when a remote device tries to make a GATT 
    connection. This will be sent to the task which initiated the GATT library.

     The GattConnectResponse() function must be called to accept or decline the 
     connection. Only after this is done, can the connection be used. 
*/
typedef struct
{
    /*! Typed BD_ADDR of remote device. */
    typed_bdaddr            taddr;
    /*! Connection configuration flags. */
    uint16                  flags;
    /*! The identifier for this connection. */
    uint16                  cid;
    /*! Connection MTU. */
    uint16                  mtu;
} GATT_CONNECT_IND_T;

/*!
    \brief GATT Connection Response

    Respond to a #GATT_CONNECT_IND_T from a remote device trying to create
    a GATT connection. F

    \param theAppTask The task to be associated with this connection.
    \param cid The connection identifier, as indicated in the 
           #GATT_CONNECT_IND_T message.
    \param flags The connection flags, as indicated in the 
           #GATT_CONNECT_IND_T message.
    \param accept TRUE to accept the connection, FALSE to reject it.

    \return a #GATT_CONNECT_CFM_T message will be returned to indicate if the
    connection was successful.
*/
void GattConnectResponse(
        Task theAppTask, 
        uint16 cid, 
        uint16 flags, 
        bool accept
        );

/*!
    \brief Disconnect an ATT connection. 

    A GATT_DISCONNECT_IND message will be sent indicating the result of the 
    Disconnection attempt to the Application task that initialised the GATT 
    library. 

    For BLE; The device issuing the Disconnect should get an IND with status
    'gatt_status_success' and for the other device 'gatt_status_link_loss'.

    For BR/EDR; Both devices should receive and IND with status 
    'gatt_status_success'.
    
    \param cid The connection identifier of the connection to disconnect.

    \return GATT_DISCONNECT_IND
*/
void GattDisconnectRequest(uint16 cid);

/*! 
    \brief GATT Disconnection Indication. This message will be sent to the task
    that initiallised the GATT library.
*/
typedef struct
{
    /*! Disconnection status. */
    gatt_status_t           status;
    /*! Connection identifier of remote device. */
    uint16                  cid;
} GATT_DISCONNECT_IND_T;


/*!
    \brief GATT Exchange MTU Indication, sent from device associated with the 
    cid. The application should respond with GattExchangeMtuRsp().
*/
typedef struct
{
    /*! Connection identifier of remote device. */
    uint16                  cid;
    /*! MTU indicated by the other device.*/
    uint16                  mtu;
} GATT_EXCHANGE_MTU_IND_T;

/*! 
    \brief Respond to GATT_EXCHANGE_MTU_IND_T from connected device.

    Used to respond to the GATT_EXCHANGE_MTU_IND_T from a connected device. 
    The lowest MTU between requester and responder will be selected. 

    \param cid The connection identifier of the other device.
    \param mtu The MTU value to send in response. Set to 0 to use the default 
    GATT MTU value (23).

*/
void GattExchangeMtuResponse(uint16 cid, uint16 mtu);

/*!
    \brief GATT Server Configuration: Exchange MTU sub-procedure

    This sub-procedure is used by the client to set the ATT_MTU to the
    maximum possible value that can be supported by both devices when the
    client supports a value greater than the default ATT_MTU for the
    Attribute Protocol. This sub-procedure shall only be initiated once
    during a connection.
    
    This sub-procedure shall not be used on a BR/EDR physical link since
    the MTU size is negotiated using L2CAP channel configuration procedures.

    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param mtu          Maximum GATT_MTU supported by the client

    \return GATT_EXCHANGE_MTU_CFM_T
*/
void GattExchangeMtuRequest(Task theAppTask, uint16 cid, uint16 mtu);

/*!
    \brief Response message for GattExchangeMtuRequest().
*/
typedef struct
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! GATT_MTU for this connection */
    uint16 mtu;
    /*! Status of the request */
    gatt_status_t status;
} GATT_EXCHANGE_MTU_CFM_T;


/*! 
    \brief GATT Access Indication 

    Indicates that the remote device is accessing a value in the database.
    The application must respond with approval (security) and the value
    to be used; GattAccessResponse.
*/
typedef struct
{
    /*! Connection identifier of remote device. */
    uint16                  cid;
    /*! Handle being accessed. */
    uint16                  handle;
    /*! Flags - uses ATT_ACCESS range. */
    uint16                  flags;
    /*! The offset of the first octet to be accessed. */
    uint16                  offset;
    /*! Length of the value. */
    uint16                  size_value;
    /*! Value data. */
    uint8                   value[1];
} GATT_ACCESS_IND_T;

/*! 
    \brief Respond to GATT Access Indication.

    Used in response to GATT_ACCESS_IND_T message, indicating that the remote 
    device wants to access a value in the local database and that the 
    application must provide permission or the value to return, depending the
    permission flags set.

    \param cid Connection identifier of the remote device
    \param handle Handle of the value
    \param result Result of Access Indication.
    \param size_value the size of the value (octets)
    \param value The value as an octet array, which is copied.

*/
void GattAccessResponse(
        uint16 cid,
        uint16 handle,
        uint16 result,
        uint16 size_value,
        const uint8 *value
        );

/*!
    \brief GATT Primary Service Discovery: Discover All Primary Services
    sub-procedure

    This sub-procedure is used by a client to discover all the primary
    services on a server.

    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device

    \return GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM_T
*/
void GattDiscoverAllPrimaryServicesRequest(Task theAppTask, uint16 cid);

/*!
    \brief Response message for GattDiscoverAllPrimaryServicesRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the service */
    uint16 handle;
    /*! End handle of the service */
    uint16 end;
    /*! UUID type of the service */
    gatt_uuid_type_t uuid_type;
    /*! UUID of the service */
    gatt_uuid_t uuid[4];
    /*! Flag indicating if more services will follow (TRUE) or not (FALSE) */
    bool more_to_come;
    /*! Status of the request */
    gatt_status_t status;
} GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM_T;

/*!
    \brief GATT Primary Service Discovery: Discover Primary Services by
    Service UUID sub-procedure

    This sub-procedure is used by a client to discover a specific primary
    service on a server when only the Service UUID is known. The specific
    primary service may exist multiple times on a server.

    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param uuid_type    UUID type of the service
    \param uuid         UUID of the service

    \return GATT_DISCOVER_PRIMARY_SERVICE_CFM_T
*/
void GattDiscoverPrimaryServiceRequest(Task theAppTask,
                                       uint16 cid,
                                       gatt_uuid_type_t uuid_type,
                                       const gatt_uuid_t *uuid);

/*!
    \brief Response message for GattDiscoverPrimaryServiceRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the service */
    uint16 handle;
    /*! End handle of the service */
    uint16 end;
    /*! UUID type of the service */
    gatt_uuid_type_t uuid_type;
    /*! UUID of the service */
    gatt_uuid_t uuid[4];
    /*! Flag indicating if more services will follow (TRUE) or not (FALSE) */
    bool more_to_come;
    /*! Status of the request */
    gatt_status_t status;
} GATT_DISCOVER_PRIMARY_SERVICE_CFM_T;

/*!
    \brief GATT Primary Service Discovery: Discover All Primary Services
    on a BR/EDR capable device sub-procedure

    This sub-procedure is used by a client to discover all the primary
    services on a server which are available over BR/EDR transport.

    \param theAppTask   Application task
    \param bd_addr      Address of the remote device

    \return GATT_DISCOVER_ALL_BREDR_SERVICES_CFM_T
*/
void GattDiscoverAllBrEdrServicesRequest(Task theAppTask, bdaddr *bd_addr);

/*!
    \brief Response message for GattDiscoverAllBrEdrServicesRequest().
*/
typedef struct 
{
    /*! Address of the remote device */
    bdaddr bd_addr;
    /*! Handle of the service */
    uint16 handle;
    /*! End handle of the service */
    uint16 end;
    /*! UUID type of the service */
    gatt_uuid_type_t uuid_type;
    /*! UUID of the service */
    gatt_uuid_t uuid[4];
    /*! Flag indicating if more services will follow (TRUE) or not (FALSE) */
    bool more_to_come;
    /*! Status of the request */
    gatt_status_t status;
} GATT_DISCOVER_ALL_BREDR_SERVICES_CFM_T;

/*!
    \brief GATT Primary Service Discovery: Discover Primary Services by
    Service UUID on a BR/EDR capable device sub-procedure

    This sub-procedure is used by a client to discover a specific primary
    service on a server when only the Service UUID is known. The specific
    primary service may exist multiple times on a server.

    \param theAppTask   Application task
    \param bd_addr      Address of the remote device
    \param uuid_type    UUID type of the service
    \param uuid         UUID of the service

    \return GATT_DISCOVER_BREDR_SERVICE_CFM_T
*/
void GattDiscoverBrEdrServiceRequest(Task theAppTask,
                                     bdaddr *bd_addr,
                                     gatt_uuid_type_t uuid_type,
                                     const gatt_uuid_t *uuid);

/*!
    \brief Response message for GattDiscoverPrimaryServiceRequest().
*/
typedef struct 
{
    /*! Address of the remote device */
    bdaddr bd_addr;
    /*! Handle of the service */
    uint16 handle;
    /*! End handle of the service */
    uint16 end;
    /*! UUID type of the service */
    gatt_uuid_type_t uuid_type;
    /*! UUID of the service */
    gatt_uuid_t uuid[4];
    /*! Flag indicating if more services will follow (TRUE) or not (FALSE) */
    bool more_to_come;
    /*! Status of the request */
    gatt_status_t status;
} GATT_DISCOVER_BREDR_SERVICE_CFM_T;

/*!
    \brief GATT Relationship Discovery: Find Included Services sub-procedure

    This sub-procedure is used by a client to find include service
    declarations within a service definition on a server. The service
    specified is identified by the service handle range.

    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param start        Starting handle of the service
    \param end          Ending handle of the service

    \return GATT_FIND_INCLUDED_SERVICES_CFM_T
*/
void GattFindIncludedServicesRequest(Task theAppTask,
                                     uint16 cid,
                                     uint16 start,
                                     uint16 end);

/*!
    \brief Response message for GattFindIncludedServicesRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the service */
    uint16 handle;
    /*! End handle of the service */
    uint16 end;
    /*! UUID type of the service */
    gatt_uuid_type_t uuid_type;
    /*! UUID of the service */
    gatt_uuid_t uuid[4];
    /*! Flag indicating if more services will follow (TRUE) or not (FALSE) */
    bool more_to_come;
    /*! Status of the request */
    gatt_status_t status;
} GATT_FIND_INCLUDED_SERVICES_CFM_T;

/*!
    \brief GATT Characteristic Discovery: Discover All Characteristics of a
    Service sub-procedure

    This sub-procedure is used by a client to find all the characteristic
    declarations within a service definition on a server when only the
    service handle range is known. The service specified is identified by
    the service handle range.

    \param theAppTask   task
    \param cid          Connection identifier of the remote device
    \param start        Starting handle of the service
    \param end          Ending handle of the service

    \return GATT_DISCOVER_ALL_CHARACTERISTICS_CFM_T
*/
void GattDiscoverAllCharacteristicsRequest(Task theAppTask,
                                           uint16 cid,
                                           uint16 start,
                                           uint16 end);

/*!
    \brief Response message for GattDiscoverAllCharacteristicsRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic declaration */
    uint16 declaration;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Characteristic properties */
    uint8 properties;
    /*! UUID type of the characteristic */
    gatt_uuid_type_t uuid_type;
    /*! UUID of the characteristic */
    gatt_uuid_t uuid[4];
    /*! Flag indicating if more services will follow (TRUE) or not (FALSE) */
    bool more_to_come;
    /*! Status of the request */
    gatt_status_t status;
} GATT_DISCOVER_ALL_CHARACTERISTICS_CFM_T;

/*!
    \brief GATT Characteristic Descriptor Discovery: Discover All
    Characteristic Descriptors sub-procedure

    This sub-procedure is used by a client to find all the characteristic
    descriptors' Attribute Handles and Attribute Types within a
    characteristic definition when only the characteristic handle range is
    known. The characteristic specified is identified by the characteristic
    handle range.

    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param start        Starting handle of the characteristic
    \param end          Ending handle of the characteristic

    \return GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T
*/
void GattDiscoverAllCharacteristicDescriptorsRequest(Task theAppTask,
                                                     uint16 cid,
                                                     uint16 start,
                                                     uint16 end);

/*!
    \brief Response message for
    GattDiscoverAllCharacteristicDescriptorsRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic descriptor */
    uint16 handle;
    /*! UUID type of the characteristic descriptor */
    gatt_uuid_type_t uuid_type;
    /*! UUID of the characteristic descriptor */
    gatt_uuid_t uuid[4];
    /*! Flag indicating if more services will follow (TRUE) or not (FALSE) */
    bool more_to_come;
    /*! Status of the request */
    gatt_status_t status;
} GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T;

/*!
    \brief GATT Characteristic Value Read: Read Characteristic Value
    sub-procedure

    This sub-procedure is used to read a Characteristic Value from a server
    when the client knows the Characteristic Value Handle.
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param handle       Handle of the characteristic 

    \note This request can also be used to perform GATT Read Characteristic
    Descriptor sub-procedure.
    
    \return GATT_READ_CHARACTERISTIC_VALUE_CFM_T
*/
void GattReadCharacteristicValueRequest(Task  theAppTask,
                                        uint16 cid,
                                        uint16 handle);

/*!
    \brief Response message for GattReadCharacteristicValueRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Status of the request */
    gatt_status_t status;
    /*! Length of the characteristic value */
    uint16 size_value;
    /*! The characteristic value */
    uint8 value[1];
} GATT_READ_CHARACTERISTIC_VALUE_CFM_T;

/*!
    \brief GATT Characteristic Value Read: Read Using Characteristic UUID
    sub-procedure

    This sub-procedure is used to read a Characteristic Value from a server
    when the client only knows the characteristic UUID and does not know the
    handle of the characteristic.

    The specific characteristic may exist multiple times in the search range
    on a server, and it is up to the client to perform another Read Using
    Characteristic UUID sub-procedure should it wish to read another value.
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param start        Starting handle of the range (typically a service)
    \param end          Ending handle of the range (typically a service)
    \param uuid_type    UUID type of the characteristic
    \param uuid         UUID of the characteristic
    
    \return GATT_READ_USING_CHARACTERISTIC_UUID_CFM_T
*/
void GattReadUsingCharacteristicUuidRequest(Task theAppTask,
                                            uint16 cid,
                                            uint16 start,
                                            uint16 end,
                                            gatt_uuid_type_t uuid_type,
                                            const gatt_uuid_t *uuid);

/*!
    \brief Response message for GattReadUsingCharacteristicUuidRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Flag indicating if more characteristic data will follow (TRUE) or
      not (FALSE) */
    bool more_to_come;
    /*! Status of the request */
    gatt_status_t status;
    /*! Length of the characteristic value */
    uint16 size_value;
    /*! The characteristic value */
    uint8 value[1];
} GATT_READ_USING_CHARACTERISTIC_UUID_CFM_T;

/*!
    \brief GATT Characteristic Value Read: Read Long Characteristic Value
    sub-procedure

    This sub-procedure is used to read a Characteristic Value from a server
    when the client knows the Characteristic Value Handle and the length of
    the Characteristic Value is longer than can be sent in a single Read
    Response message.
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param handle       Handle of the characteristic 

    \note This request can also be used to perform GATT Read Long
    Characteristic Descriptor sub-procedure.
    
    \return GATT_READ_LONG_CHARACTERISTIC_VALUE_CFM_T
*/
void GattReadLongCharacteristicValueRequest(Task theAppTask,
                                            uint16 cid,
                                            uint16 handle);

/*!
    \brief Response message for GattReadLongCharacteristicValueRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Flag indicating if more characteristic data will follow (TRUE) or
      not (FALSE) */
    bool more_to_come;
    /*! Status of the request */
    gatt_status_t status;
    /*! Offset to the characteristic value in this message */
    uint16 offset;
    /*! Length of the characteristic value */
    uint16 size_value;
    /*! The characteristic value */
    uint8 value[1];
} GATT_READ_LONG_CHARACTERISTIC_VALUE_CFM_T;

/*!
    \brief GATT Characteristic Value Read: Read Multiple Characteristic Values
    sub-procedure

    This sub-procedure is used to read multiple Characteristic Values from a
    server when the client knows the Characteristic Value Handles. The Read
    Multiple Response returns the Characteristic Values in the Set Of Values
    parameter.
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param size_handles Number of handles to be read
    \param handles      Handles of the characteristics

    \return GATT_READ_MULTIPLE_CHARACTERISTIC_VALUES_CFM_T
*/
void GattReadMultipleCharacteristicValuesRequest(Task theAppTask,
                                                 uint16 cid,
                                                 uint16 size_handles,
                                                 uint16 *handles);

/*!
    \brief Response message for GattReadMultipleCharacteristicValuesRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Status of the request */
    gatt_status_t status;
    /*! Length of the set of characteristic values */
    uint16 size_value;
    /*! The set of characteristic values */
    uint8 value[1];
} GATT_READ_MULTIPLE_CHARACTERISTIC_VALUES_CFM_T;

/*!
    \brief GATT Characteristic Value Write: Write Without Response
    sub-procedure

    This sub-procedure is used to write a Characteristic Value to a server
    when the client knows the Characteristic Value Handle and the client does
    not need an acknowledgment that the write was successfully performed.
    This sub-procedure only writes the first (ATT_MTU - 3) octets of a
    Characteristic Value. This sub-procedure cannot be used to write a long
    characteristic; instead the Write Long Characteristic Values sub-procedure
    should be used.
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param handle       Handle of the characteristic
    \param size_value   Length of the characteristic value to be written
    \param value        The characteristic value to be written

    \return GATT_WRITE_WITHOUT_RESPONSE_CFM_T
*/
void GattWriteWithoutResponseRequest(Task theAppTask,
                                     uint16 cid,
                                     uint16 handle,
                                     uint16 size_value,
                                     const uint8 *value);

/*!
    \brief Response message for GattWriteWithoutResponseRequest().

    This confirmation message only indicates whether the command was
    successfully sent to the server or not. Server does not send an
    acknowledgment to the client.
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Status of the request */
    gatt_status_t status;
} GATT_WRITE_WITHOUT_RESPONSE_CFM_T;

/*!
    \brief GATT Characteristic Value Write: Signed Write Without Response
    sub-procedure

    This sub-procedure is used to write a Characteristic Value to a server
    when the client knows the Characteristic Value Handle and the ATT Bearer
    is not encrypted. This sub-procedure shall only be used if the
    Characteristic Properties authenticated bit is enabled and the client and
    server device share a bond. This sub-procedure only writes the first
    (ATT_MTU - 15) octets of an Attribute Value. This sub-procedure cannot be
    used to write a long Attribute; instead the Write Long Characteristic
    Values sub-procedure should be used.
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param handle       Handle of the characteristic
    \param size_value   Length of the characteristic value to be written
    \param value        The characteristic value to be written

    \return GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM_T
*/
void GattSignedWriteWithoutResponseRequest(Task theAppTask,
                                           uint16 cid,
                                           uint16 handle,
                                           uint16 size_value,
                                           const uint8 *value);

/*!
    \brief Response message for GattSignedWriteWithoutResponseRequest().

    This confirmation message only indicates whether the command was
    successfully sent to the server or not. Server does not send an
    acknowledgment to the client.
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Status of the request */
    gatt_status_t status;
} GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM_T;

/*!
    \brief GATT Characteristic Value Write: Write Characteristic Value
    sub-procedure

    This sub-procedure is used to write a Characteristic Value to a server
    when the client knows the Characteristic Value Handle. This sub-procedure
    only writes the first (ATT_MTU - 3) octets of a Characteristic Value.
    This sub-procedure cannot be used to write a long Attribute; instead the
    Write Long Characteristic Values sub-procedure should be used.
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param handle       Handle of the characteristic
    \param size_value   Length of the characteristic value to be written
    \param value        The characteristic value to be written

    \note This request can also be used to perform GATT Write Characteristic
    Descriptor sub-procedure.
    
    \return GATT_WRITE_CHARACTERISTIC_VALUE_CFM_T
*/
void GattWriteCharacteristicValueRequest(Task theAppTask,
                                         uint16 cid,
                                         uint16 handle,
                                         uint16 size_value,
                                         const uint8 *value);

/*!
    \brief Response message for GattWriteCharacteristicValueRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Status of the request */
    gatt_status_t status;
} GATT_WRITE_CHARACTERISTIC_VALUE_CFM_T;

/*!
    \brief GATT Characteristic Value Write: Write Long Characteristic Value
    sub-procedure

    This sub-procedure is used to write a Characteristic Value to a server
    when the client knows the Characteristic Value Handle but the length of
    the Characteristic Value is longer than can be sent in a single Write
    Characteristic Value message.
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param handle       Handle of the characteristic
    \param size_value   Length of the characteristic value to be written
    \param value        The characteristic value to be written

    \note This request can also be used to perform GATT Write Long
    Characteristic Descriptor sub-procedure.
    
    \return GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM_T
*/
void GattWriteLongCharacteristicValueRequest(Task theAppTask,
                                             uint16 cid,
                                             uint16 handle,
                                             uint16 size_value,
                                             const uint8 *value);

/*!
    \brief Response message for GattWriteLongCharacteristicValueRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Status of the request */
    gatt_status_t status;
} GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM_T;

/*!
    \brief GATT Characteristic Value Write: Reliable Write sub-procedure

    This sub-procedure is used to write a Characteristic Value to a server
    when the client knows the Characteristic Value Handle, and assurance is
    required that the correct Characteristic Value is going to be written by
    transferring the Characteristic Value to be written in both directions
    before the write is performed.
    
    This sub-procedure can also be used when multiple values must be written,
    in order, in a single operation.

    When this sub-procedure is used the Characteristic Values are first
    transferred to the server and then back to the client using
    GattReliableWritePrepareRequest(). At this point the GATT library checks
    that the value is correct and returns the confirmation message to the
    application. After the confirmation the client can request another
    Reliable Write, or request the server to store or cancel all queued
    Reliable Writes using GattReliableWriteExecuteRequest().
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param handle       Handle of the characteristic
    \param offset       Offset to the characteristic value
    \param size_value   Length of the characteristic value to be written
    \param value        The characteristic value to be written

    \return GATT_RELIABLE_WRITE_PREPARE_CFM_T
*/
void GattReliableWritePrepareRequest(Task theAppTask,
                                     uint16 cid,
                                     uint16 handle,
                                     uint16 offset,
                                     uint16 size_value,
                                     const uint8 *value);

/*!
    \brief Response message for GattReliableWritePrepareRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Status of the request */
    gatt_status_t status;
} GATT_RELIABLE_WRITE_PREPARE_CFM_T;

/*!
    \brief GATT Characteristic Value Write: Reliable Write sub-procedure

    This sub-procedure is used to write a Characteristic Value to a server
    when the client knows the Characteristic Value Handle, and assurance is
    required that the correct Characteristic Value is going to be written by
    transferring the Characteristic Value to be written in both directions
    before the write is performed.
    
    This sub-procedure can also be used when multiple values must be written,
    in order, in a single operation.

    When this sub-procedure is used the Characteristic Values are first
    transferred to the server and then back to the client using
    GattReliableWritePrepareRequest(). At this point the GATT library checks
    that the value is correct and returns the confirmation message to the
    application. After the confirmation the client can request another
    Reliable Write, or request the server to store or cancel all queued
    Reliable Writes using GattReliableWriteExecuteRequest().
    
    \param theAppTask   Application task
    \param cid          Connection identifier of the remote device
    \param execute      A flag choosing whether to store (TRUE) or cancel
                        (FALSE) prepared write queue on the server.

    \return GATT_RELIABLE_WRITE_EXECUTE_CFM_T
*/
void GattReliableWriteExecuteRequest(Task theAppTask,
                                     uint16 cid,
                                     bool execute);

/*!
    \brief Response message for GattReliableWriteExecuteRequest().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the failed characteristic in case of error, otherwise 0 */
    uint16 handle;
    /*! Status of the request */
    gatt_status_t status;
} GATT_RELIABLE_WRITE_EXECUTE_CFM_T;

/*!
    \brief GATT Notification: Notification sub-procedure - server side.

    Sends a notification to the client for the handle and value. There is 
    no response to the application task if this action fails. 

    The notification does not update the characteristic value in the server
    ATT database itself. 

    \param theAppTask  Application task
    \param cid         Connection identifier of the remote device
    \param handle      Handle of the characteristic
    \param size_value  Length of the characteristic value to be notified.
    \param value       The characteristic value to be notified. This is copied.

    \return GATT_NOTIFICATION_CFM_T
*/
void GattNotificationRequest(
        Task theAppTask, 
        uint16 cid, 
        uint16 handle, 
        uint16 size_value,
        const uint8 *value
        );

/*! 
    \brief GATT Notification Confirmation: Notification sub-procedure - server.

    Sent in response to a GattNotificationRequest().
*/
typedef struct
{
    gatt_status_t   status;
    uint16          cid;
    uint16          handle;
    uint16          dummy[2];   /* Added to increase size to above 4 bytes in order to ensure proper memory allocation */
} GATT_NOTIFICATION_CFM_T;


/*!
    \brief GATT Characteristic Value Notification: Notification sub-procedure

    This message is received when a server is configured to notify a
    Characteristic Value to a client without expecting any Attribute Protocol
    layer acknowledgment that the notification was successfully received.

    This is an unsolicited message which does not require response.
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Length of the characteristic value */
    uint16 size_value;
    /*! The characteristic value */
    uint8 value[1];
} GATT_NOTIFICATION_IND_T;

/*!
    \brief GATT Indication Request: Indication sub-procedure - server.

    Sends a indication to the client for the handle and value. The client 
    should respond to the indication. 

    The notification does not update the characteristic value in the server
    ATT database itself. 

    \param theAppTask  Application task
    \param cid         Connection identifier of the remote device
    \param handle      Handle of the characteristic
    \param size_value  Length of the characteristic value to be indicated.
    \param value       The characteristic value to be indicated. This is copied.

    \return GATT_INDICATION_CFM_T;

*/
void GattIndicationRequest(
        Task theAppTask, 
        uint16 cid, 
        uint16 handle, 
        uint16 size_value,
        const uint8 *value
        );

/*!
    \brief GATT Indication Confirmation: Indication sub-procedure - server.

    This message is received in response to GattIndicationRequest().
    When an Indication is successfully sent, this message is returned with 
    status 'gatt_status_success_sent'. Then, when the remote device responds to 
    the notification, this message should be returned with status 
    'gatt_status_success'.
*/
typedef struct
{
    gatt_status_t   status;
    uint16          cid;
    uint16          handle;
    uint16          dummy[2];   /* Added to increase size to above 4 bytes in order to ensure proper memory allocation */
} GATT_INDICATION_CFM_T;
    

/*!
    \brief GATT Characteristic Value Notification: Indication sub-procedure

    This sub-procedure is used when a server is configured to indicate a
    Characteristic Value to a client and expects an Attribute Protocol layer
    acknowledgment that the indication was successfully received.

    This is an unsolicited message which the application must acknowledge
    using GattIndicationResponse().
*/
typedef struct 
{
    /*! Connection identifier of the remote device */
    uint16 cid;
    /*! Handle of the characteristic value */
    uint16 handle;
    /*! Length of the characteristic value */
    uint16 size_value;
    /*! The characteristic value */
    uint8 value[1];
} GATT_INDICATION_IND_T;

/*!
    \brief GATT Characteristic Value Notification: Indication acknowledgment

    This function is used to acknowledge the sever that client has received
    Handle Value Indication, and server may send another indication.

    Only one indication may be sent by the server at a time.

    This function is used in response to GATT_INDICATION_IND_T message.
    
    \param cid          Connection identifier of the remote device

    \return Nothing
*/
void GattIndicationResponse(uint16 cid);

/*!
    \brief Get the connection id associated with a remote device address,
    for an existing connection.

    \param taddr The remote device address used to make the connection.
    \return uint16 value of the connection id or 0, if the address is not
    found.
*/
uint16 GattGetCidForBdaddr(const typed_bdaddr *taddr);

/*!
    \brief Get the maximum available length for transmission for a given connection id.

    \param cid Connection identifier of the remote device
    \return uint16 The maximum length available for transmission.
*/
uint16 GattGetMaxTxDataLength(uint16 cid);

#ifdef GATT_PTS_TEST

/*!
    \brief Set the first ATT_READ_BLOB_REQ offset explicitly for following
    GattReadLongCharacteristicValueRequest() function calls.

    This GATT profile implementation hides the complexity of ATT_READ_BLOB_REQ
    operations and takes care of setting the correct offset when performing
    Read Long Characterisitic Value functions. However, the Profile Tuning
    Suite SIG tool has test cases that explicitly require setting an invalid
    offset in order to elicite an error response:
    * GATT/CL/GAR/BV-13-C [Read Long Characteristic Value - Invalid Offset]
    * GATT/CL/GAR/BI-29-C [Read Long Characteristic Descriptor – Invalid Offset]
    
    Therefore, to satisfy the PTS qualification, the following function will
    allow the initial offset value (which is normally, and defaults to 0) to
    be explicitly set. This function must be called before running the 
    relevant PTS test case AND then set back to 0 for following test cases or
    they could fail!

    \param offset The (invalid) offset value to set.

    \return None
*/
void GattSetPtsReadLongCharacteristicOffset(uint16 offset);
#endif /* GATT_PTS_TEST */

/*!
    \brief      Handles a write by a client to the Client Supported Features
                characteristic.

    \param feat The feature being enabled.
    \param cid  The cid of the client who wrote to the characteristic. 
    
    \return True if the value is valid, False otherwise
*/
bool GattHandleCsfWrite(uint8 feat, uint16 cid);

/*!
    \brief      Handles a read request from a client to the Client Supported
                Features characteristic.

    \param cid  The cid of the client who wrote to the characteristic.
    
    \return None
*/
void GattHandleCsfRead(uint16 cid, uint8 *val);

/*!
    \brief      Handles the request to read the Service Changed Client Characteristic 
                Configuration Descriptor.
                
    \param cid  The cid of the client who wants to read the characteristic. 
    
    \return The value of the CCCD; either 2 or 0.
*/
uint8 GattHandleServChangedIndRead(uint16 cid);

/*!
    \brief      This function handles a write by a client to the Service Changed
                Client Characteristic Configuration Descriptor.

    \param feat The value to be written. This can only be 2 or 0.
    \param cid  The cid of the client who wrote to the characteristic. 
    
    \return None
*/
void GattHandleServChangedIndWrite(uint8 value, uint16 cid);

#endif  /* GATT_H_ */

/** @} */

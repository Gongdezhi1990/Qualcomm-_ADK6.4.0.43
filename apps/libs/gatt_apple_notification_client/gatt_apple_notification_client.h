/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

/*!
@file    gatt_apple_notification_client.h
@brief   Header file for the GATT Apple notification client library.

        This file provides documentation for the GATT Apple notification client library
        API (library name: gatt_apple_notification_client).
*/

#ifndef GATT_APPLE_NOTIFICATION_CLIENT_H_
#define GATT_APPLE_NOTIFICATION_CLIENT_H_

#include <csrtypes.h>
#include <message.h>
#include <library.h>
#include <gatt.h>

 /*!
* Invalid Handle Value
*/
#define GATT_ANCS_INVALID_HANDLE (0xFFFF)

/*!
* macro definitions for the Apple notification categories
*/
#define ANCS_OTHER_CATEGORY_ID                  0
#define ANCS_INCOMING_CALL_CATEGORY_ID          1
#define ANCS_MISSED_CALL_CATEGORY_ID            2
#define ANCS_VOICE_MAIL_CATEGORY_ID             3
#define ANCS_SOCIAL_CATEGORY_ID                 4
#define ANCS_SCHEDULE_CATEGORY_ID               5
#define ANCS_EMAIL_CATEGORY_ID                  6
#define ANCS_NEWS_CATEGORY_ID                   7
#define ANCS_HEALTH_N_FITNESS_CATEGORY_ID       8
#define ANCS_BUSINESS_N_FINANCE_CATEGORY_ID     9
#define ANCS_LOCATION_CATEGORY_ID               10
#define ANCS_ENTERTAINMENT_CATEGORY_ID          11

/*!
* bit-masks for the Apple notification categories
*/
#define ANCS_NO_CATEGORY                        (0U)
#define ANCS_OTHER_CATEGORY                     (1U << ANCS_OTHER_CATEGORY_ID)
#define ANCS_INCOMING_CALL_CATEGORY             (1U << ANCS_INCOMING_CALL_CATEGORY_ID)
#define ANCS_MISSED_CALL_CATEGORY               (1U << ANCS_MISSED_CALL_CATEGORY_ID)
#define ANCS_VOICE_MAIL_CATEGORY                (1U << ANCS_VOICE_MAIL_CATEGORY_ID)
#define ANCS_SOCIAL_CATEGORY                    (1U << ANCS_SOCIAL_CATEGORY_ID)
#define ANCS_SCHEDULE_CATEGORY                  (1U << ANCS_SCHEDULE_CATEGORY_ID)
#define ANCS_EMAIL_CATEGORY                     (1U << ANCS_EMAIL_CATEGORY_ID)
#define ANCS_NEWS_CATEGORY                      (1U << ANCS_NEWS_CATEGORY_ID)
#define ANCS_HEALTH_N_FITNESS_CATEGORY          (1U << ANCS_HEALTH_N_FITNESS_CATEGORY_ID)
#define ANCS_BUSINESS_N_FINANCE_CATEGORY        (1U << ANCS_BUSINESS_N_FINANCE_CATEGORY_ID)
#define ANCS_LOCATION_CATEGORY                  (1U << ANCS_LOCATION_CATEGORY_ID)
#define ANCS_ENTERTAINMENT_CATEGORY             (1U << ANCS_ENTERTAINMENT_CATEGORY_ID)


/*!
    @brief Status code returned from the GATT Apple notification client library

    This status code indicates the outcome of the request.
*/
typedef enum
{
    gatt_ancs_status_success,
    gatt_ancs_status_initiating,
    gatt_ancs_status_failed,
    gatt_ancs_status_not_supported,
    /* ancs specific error codes */
    gatt_ancs_status_unknown_command,
    gatt_ancs_status_invalid_command,
    gatt_ancs_status_invalid_parameter,
    gatt_ancs_status_action_failed
} gatt_ancs_status_t;

/*!
    @brief Enumeration for Apple notification command ids
*/
typedef enum
{
    gatt_ancs_notification_attr = 0,            /* Command Id for getting Notification Attributes */
    gatt_ancs_notification_app_attr,            /* Command Id for getting Application Attributes */
    gatt_ancs_notification_action,              /* Command Id for performing Notification Action */
    gatt_ancs_reserved
}gatt_ancs_command_id;

/*!
    @brief Enumeration for Apple notification attribute ids
*/
typedef enum
{
    gatt_ancs_attr_app_id = 0,       /* Attribute Id for getting Application Identifier */
    gatt_ancs_attr_title,            /* Attribute Id for getting Title */
    gatt_ancs_attr_subtitle,         /* Attribute Id for getting SubTitle */
    gatt_ancs_attr_message,          /* Attribute Id for getting Messages */
    gatt_ancs_attr_message_size,     /* Attribute Id for getting Message Size */
    gatt_ancs_attr_date,             /* Attribute Id for getting Date */
    gatt_ancs_attr_pos_action,       /* Attribute Id for getting positive action label */
    gatt_ancs_attr_neg_action,       /* Attribute Id for getting negative action label */
    gatt_ancs_attr_reserved
}gatt_ancs_attr_id;

/*!
    @brief Enumeration for Apple notification application attribute ids
*/
typedef enum
{
    gatt_ancs_app_attr_name = 0,              /* Application Attribute Id for getting Application Name */
    gatt_ancs_app_attr_reserved
}gatt_ancs_app_attr_id;

/*!
    @brief Enumeration for Apple notification action ids
*/
typedef enum
{
    gatt_ancs_action_positive = 0,              /* Positive Action Id */
    gatt_ancs_action_negative,                  /* Negative Action Id */
    gatt_ancs_action_reserved
}gatt_ancs_action_id;

/*!
    @brief Enumeration for Apple notification event ids
*/
typedef enum
{
    gatt_ancs_notification_added,
    gatt_ancs_notification_modified,
    gatt_ancs_notification_removed
}gatt_ancs_event_id;

/*! @brief Enumeration of messages a client task may receive from the Apple notification client library.
 */
typedef enum
{
    /* Client messages */
    GATT_ANCS_NS_IND = GATT_ANCS_MESSAGE_BASE,
    GATT_ANCS_INIT_CFM,
    GATT_ANCS_SET_NS_NOTIFICATION_CFM,
    GATT_ANCS_SET_DS_NOTIFICATION_CFM,
    GATT_ANCS_DS_ATTR_IND,
    GATT_ANCS_DS_APP_ATTR_IND,
    GATT_ANCS_WRITE_CP_CFM,
    
    /* Library message limit */
    GATT_ANCS_MESSAGE_TOP
} gatt_ancs_message_id_t;

/*! @brief The Apple notification internal structure for the client role.

    This structure is visible to the application as it is responsible for managing resource to pass to the
    Apple notification library.
    The elements of this structure are ONLY modified by the Apple notification library.
    The application SHOULD NOT modify the values at any time as it could lead to undefined behaviour.

 */
typedef struct __GANCS
{
    TaskData   lib_task;
    Task       app_task;
    uint16     cid;
    uint16     notification_source;
    uint16     ns_ccd;                    /*!< Handle to store the client characteristic descriptor for NS */
    uint16     control_point;
    uint16     data_source;
    uint16     ds_ccd;                    /*!< Handle to store the client characteristic descriptor for DS */
    uint16     pending_cmd;
    Task       task_pending_cfm;
    uint16     ns_notification_mask;      /*!< This mask holds the notification which the application is interested in */
    uint8      char_report_mask;          /*!< Mask to report the order of characteristic received */
    uint8      char_report_mask_index;    /*!< Index to extract the order of characteristic received */
} GANCS;

/*! @brief Contents of the GATT_ANCS_NS_IND_T message that is sent by the library,
    as a result of a notification from Notification Source of remote Apple Notification Server.
 */
typedef struct __GATT_ANCS_NS_IND
{
    const GANCS *ancs;
    uint8        event_id;                   /*! This field informs the accessory whether the given iOS notification was added, modified, or removed. */
    uint8        event_flag;                 /*! Flag to determine the specificities with the iOS notification */
    uint8        category_id;                /*! Field to identify the category of iOS notification */
    uint8        category_count;             /*! The current number of active iOS notifications in the given category */
    uint32       notification_uid;           /*! Uniquely identifies the iOS notification */
} GATT_ANCS_NS_IND_T;

/*! @brief Contents of the GATT_ANCS_INIT_CFM message that is sent by the library,
    as characteristic being found out.
 */
typedef struct __GATT_ANCS_INIT_CFM
{
    const GANCS        *ancs;
    gatt_ancs_status_t status;
} GATT_ANCS_INIT_CFM_T;

/*! @brief Contents of the GATT_ANCS_SET_NS_NOTIFICATION_CFM message that is sent by the library,
    as a result of setting notifications on the server.
 */
typedef struct __GATT_ANCS_SET_NS_NOTIFICATION_CFM
{
    const GANCS       *ancs;
    uint16             cid;
    gatt_ancs_status_t status;
    gatt_status_t      gatt_status;
} GATT_ANCS_SET_NS_NOTIFICATION_CFM_T;

/*! @brief Contents of the GATT_ANCS_SET_DS_NOTIFICATION_CFM message that is sent by the library,
    as a result of setting notifications on the server.
 */
typedef struct __GATT_ANCS_SET_DS_NOTIFICATION_CFM
{
    const GANCS       *ancs;
    uint16             cid;
    gatt_ancs_status_t status;
    gatt_status_t      gatt_status;
} GATT_ANCS_SET_DS_NOTIFICATION_CFM_T;

typedef struct __GATT_ANCS_DS_ATTR_IND
{
    const GANCS *ancs;
    uint32      notification_uid;      /*! Uniquely identifies the iOS notification */
    uint16      size_value;
    uint8       value[1];/* This will contain Attribute Id followed by Attr Value length followed by actual value
                            Exp: Byte1: Attr ID | Byte 2-3: Attr Value Length | Byte 4 -n: Attr Value and so on*/
} GATT_ANCS_DS_ATTR_IND_T;

typedef struct __GATT_ANCS_DS_APP_ATTR_IND
{
    const GANCS *ancs;
    uint16      size_value;
    uint8       value[1]; /* This will contain App ID (NULL terminated string), followed attr values
                             Exp: Byte1-n: App ID (till NULL) | Byte n+1: App Id | Byte n+2 -n+3: App value length |
                             Byte n+4 - m: App value  and so on*/
} GATT_ANCS_DS_APP_ATTR_IND_T;

/*! @brief Contents of the GATT_ANCS_WRITE_CP_ATTR_CFM message that is sent by the library,
    as a result of writing to CP.
 */
typedef struct __GATT_ANCS_WRITE_CP_CFM
{
    const GANCS          *ancs;
    uint16                cid;
    gatt_ancs_command_id  command_id;
    gatt_ancs_status_t    status;
    gatt_status_t         gatt_status;
} GATT_ANCS_WRITE_CP_CFM_T;

typedef void (*gatt_ancs_ready_state_observer_t)(const GANCS *ancs, bool is_ready);

/*!
    @brief After the VM application has used the GATT manager library to establish a connection to a discovered BLE device in the Client role,
    it can discover any supported services in which it has an interest. It should then register with the relevant client service library
    (passing the relevant CID and handles to the service). For the Apple notification client it will use this API. The GATT manager
    will then route notifications and indications to the correct instance of the client service library for the CID.

    @param ancs A valid area of memory that the service library can use.
    @param app_task The Task that will receive the messages sent from this Apple notification client library.
    @param cid The connection ID.
    @param start_handle The start handle of the Apple notification client instance.
    @param end_handle The end handle of the Apple notification client instance.

    @return The status result of calling the API.

*/
gatt_ancs_status_t GattAncsInit(GANCS *ancs, Task app_task, uint16 cid, uint16 start_handle, uint16 end_handle);

/*!
    @brief When a GATT connection is removed, the application must remove all client service instances that were
    associated with the connection (using the CID value).
    This is the clean up routine as a result of calling the GattAncsInit API. That is,
    the GattAncsInit API is called when a connection is made, and the GattAncsDestroy is called
    when the connection is removed.

    @param ancs The client instance that was passed into the GattAncsInit API.

    @return The status result of calling the API.

*/
gatt_ancs_status_t GattAncsDestroy(GANCS *ancs);

/*!
    @brief This API is used to write the client characteristic configuration on a remote device,
    to enable notifications for Notification Source Characteristic with the server.
    An error will be returned if the server does not support notifications.

    @param ancs The client instance that was passed into the GattAncsInit API.
    @param notifications_enable Set to TRUE to enable notifications on the server, FALSE to disable them.
    @param notification_mask This mask is used to set the notification which the application is interested to either mask/unmask.
        Application can make use of the above defined category macros to set/clear each category.
        For example: notification_mask = ANCS_INCOMING_CALL_CATEGORY | ANCS_EMAIL_CATEGORY...

    @return A GATT_ANCS_SET_NS_NOTIFICATION_CFM message will be sent to the registered application Task.

*/
void GattAncsSetNSNotificationEnableRequest(const GANCS *ancs, bool notifications_enable, uint16 notification_mask);

/*!
    @brief Use to enable/disable notifications for Notification Source characteristic on the server.
    An error will be returned if the server does not support notifications.

    @param task The task to receive the GATT_ANCS_SET_NS_NOTIFICATION_CFM message.
    @param ancs The client instance.
    @param notifications_enable Set to TRUE to enable notifications, FALSE to disable them.
    @param notification_mask This mask is used to set the notification which the application is interested to either mask/unmask.
        For example: notification_mask = ANCS_INCOMING_CALL_CATEGORY | ANCS_EMAIL_CATEGORY...
*/
void GattAncsSetNotificationSourceNotificationEnableRequest(Task task, const GANCS *ancs, bool notifications_enable, uint16 notification_mask);

/*!
    @brief This API is used to write the client characteristic configuration on a remote device,
    to enable notifications for Data Source Characteristic with the server.
    An error will be returned if the server does not support notifications.

    @param ancs The client instance that was passed into the GattAncsInit API.
    @param notifications_enable Set to TRUE to enable notifications on the server, FALSE to disable them.

    @return A GATT_ANCS_SET_DS_NOTIFICATION_CFM message will be sent to the registered application Task.

*/
void GattAncsSetDSNotificationEnableRequest(const GANCS *ancs, bool notifications_enable);

/*!
    @brief Use to enable/disable notifications for Data Source characteristic on the server.
    An error will be returned if the server does not support notifications.

    @param task The task to receive the GATT_ANCS_SET_DS_NOTIFICATION_CFM message.
    @param ancs The client instance.
    @param notifications_enable Set to TRUE to enable notifications, FALSE to disable them.
*/
void GattAncsSetDataSourceNotificationEnableRequest(Task task, const GANCS *ancs, bool notifications_enable);

/*!
    @brief This API is used to write the Control Point on Apple Notification Service of server side for
              "Get Notification Attributes"

    @param ancs The client instance that was passed into the GattAncsInit API.
    @param notification_uid    Represents the UID of the iOS notification on
                               which the client wants to perform an action
    @param size_attribute_list The size of the attribute_list
    @param attribute_list      A list of attributes the NC wants to retrieve

    @return void

    NOTE: GATT_ANCS_DS_ATTR_IND message will be sent to the registered application Task.
        In case of error/success application can expect GATT_ANCS_WRITE_CP_CFM
*/
void GattAncsGetNotificationAttributes(const GANCS *const ancs, uint32 notification_uid, uint16 size_attribute_list, uint8 *attribute_list);

/*******************************************************************************
    @brief ANCS Get Notification Attributes
    
    The Get App Attributes command allows an NC to retrieve attributes of a
    specific app installed on the NP.
    
    @param ancs                The client instance that was passed to GattAncsInit
    @param size_app_id       The size of the app_id
    @param app_id              The string identifier of the app the client wants
                               information about. This string must be NULL terminated
    @param size_attribute_list The size of the attribute_list
    @param attribute_list      A list of attributes the NC wants to retrieve
    
    @return void
    
    NOTE: GATT_ANCS_DS_APP_ATTR_IND message will be sent to the registered application Task.
            In case of error/succes application can expect GATT_ANCS_WRITE_CP_CFM
*/
void GattAncsGetAppAttributes(const GANCS *const ancs, uint16 size_app_id, uint8 * app_id, uint16 size_attribute_list, uint8 *attribute_list);

/*******************************************************************************
    @brief ANCS Perform Notification Action

    Starting with iOS 8.0, the NP can inform the NC of potential actions that 
    are associated with iOS notifications. On the user's behalf, the NC can
    then request the NP to perform an action associated with a specific iOS 
    notification.

    @param ancs              The client instance that was passed to GattAncsInit
    @param notification_uid  represents the UID of the iOS notification on
                             which the client wants to perform an action
    @param action_id         The desired action the NC wants to be performed on
                             the iOS notification
    
    @return void
*/
void GattAncsPerformNotificationAction(const GANCS *const ancs, uint32 notification_uid, gatt_ancs_action_id action_id);

/*!
    @brief Use to write the Control Point characteristic.

    @param task The task to receive the GATT_ANCS_WRITE_CP_CFM message.
    @param ancs The client instance.
    @param value The value we want to write.
    @param value_size The size of the value.
*/
void GattAncsWriteControlPoint(Task task, const GANCS *ancs, const uint8 *value, uint16 value_size);

/*!
    @brief The function passed will be called when the ready state of a client instance changes.
    Ready means a client instance has just been fully initialised.
    Non-ready means a client instance is going to be destroyed immediately after this call.

    @param observer The function called when the ready state changes.

    @return TRUE when the observer has been successfully added, FALSE otherwise.
*/
bool GattAncsAddReadyStateObserver(gatt_ancs_ready_state_observer_t observer);

/*!
    @brief Get the connection id used by the client.

    @param ancs The client instance.

    @return The connection id (cid).
*/
uint16 GattAncsGetConnectionId(const GANCS *ancs);

/*!
    @brief Get the Notification Source characteristic handle.

    @param ancs The client instance.

    @return The handle value on success, GATT_ANCS_INVALID_HANDLE otherwise (Characteristic is not supported by the server, client initialisation is not completed).
*/
uint16 GattAncsGetNotificationSourceHandle(const GANCS *ancs);

/*!
    @brief Get the Data Source characteristic handle.

    @param ancs The client instance.

    @return The handle value on success, GATT_ANCS_INVALID_HANDLE otherwise (Characteristic is not supported by the server, client initialisation is not completed).
*/
uint16 GattAncsGetDataSourceHandle(const GANCS *ancs);

#endif /* GATT_APPLE_NOTIFICATION_CLIENT_H_ */

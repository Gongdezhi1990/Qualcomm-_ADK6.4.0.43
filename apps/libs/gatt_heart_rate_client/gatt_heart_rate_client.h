/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file     gatt_heart_rate_client.h
@brief   Header file for the GATT Heart Rate Client library.

        This file provides documentation for the GATT Heart Rate Client library
        API (library name: gatt_heart_rate_client).
*/

#ifndef GATT_HEART_RATE_CLIENT_H
#define GATT_HEART_RATE_CLIENT_H

#include <csrtypes.h>
#include <message.h>

#include "library.h"

/* This structure is made public to application as application is responsible for managing resources 
* for the elements of this structure. The data elements are indented to use by Heart Rate Client library only. 
* Application SHOULD NOT access (read/write) any elements of this library structure at any point of time and doing so  
* may cause undesired behaviour of this library functionalities
*/
typedef struct _gatt_heart_rate_client_t
{
    TaskData lib_task;                   /*! Lib Task */
    Task app_task;                       /*! Application Registered Task */
    
    uint16 hr_meas_handle;               /*! Heart Rate Measurement Handle: Used by lib, unused by app*/
    uint16 hr_meas_ccd_handle;           /*! Heart Rate Measurement CC Handle */
    uint16 hr_meas_end_handle;           /*! Heart Rate Measurement characteristics End Handle: Used by lib, unused by app*/
    uint16 hr_sensor_loc_handle;         /*! Heart Rate Sensor Location Handle: Used by lib, unused by app*/
    uint16 hr_control_point_handle;      /*! Heart Rate Control Point Handle */
    
    uint16 pending_cmd;                  /*! Any read/write command pending? */
    unsigned notification_enabled:1;     /*! Notification has been enabled/disabled */
    unsigned _SPARE1_:15;                /*! Spare bits available */

}_gatt_heart_rate_client_t;

/*! @brief GATT Heart Rate Service Client[GHRSC_T]Library Instance.
 */
typedef struct _gatt_heart_rate_client_t GHRSC_T;


/*!
    @brief persistent data for each known Heart rate sensor device.

    Each Heart rate sensor device that is bonded can have data associated against
    it so that re-connections are much faster in that case no GATT discovery is required.
*/
typedef struct
{
    uint16 hr_meas_ccd_handle;           /*! Heart Rate Measurement CC Handle */
    uint16 hr_control_point_handle;      /*! Heart Rate Control Point Handle */
    uint8  hr_sensor_location;           /*! Heart Rate Sensor Location, if supported by sensor */
} gatt_heart_rate_client_device_data_t;



/*! @brief Enumeration of messages a client task may receive from the Heart Rate Service Client library.
 */
typedef enum
{
    GATT_HEART_RATE_CLIENT_INIT_CFM = GATT_HEART_RATE_CLIENT_MESSAGE_BASE,  /* Confirmation for Init */
    GATT_HEART_RATE_CLIENT_NOTIFICATION_CFM,    /* Confirmation for Notification */
    GATT_HEART_RATE_CLIENT_RESET_EE_CFM,        /* Confirmation for reset of HR EE values */
    GATT_HEART_RATE_CLIENT_NOTIFICATION_IND,    /* Indication for HR measurement values */
    GATT_HEART_RATE_CLIENT_MESSAGE_TOP          /* Top of message enumeration */
} GattHeartRateServiceMessageId;


/*!
    @brief Enumeration for Heart rate client status code  
*/

typedef enum
{
    gatt_heart_rate_client_status_success,        /* Request was success */
    gatt_heart_rate_client_status_not_allowed,    /* Request is not allowed at the moment,something went wrong internally  */
    gatt_heart_rate_client_status_no_connection,  /* There is no GATT connection exists for given CID so that service library can issue a request to remote device */
    gatt_heart_rate_client_status_failed          /* Request has been failed */
}gatt_heart_rate_client_status;


/* HEART RATE MEASUREMENT FLAGS */
#define HR_MEASUREVAL_FORMAT_UINT8   (0x00)
#define HR_MEASUREVAL_FORMAT_UINT16  (0x01)

/* Sensor contact feature support (bit 2 of the flag) */
#define HR_SENSOR_CONTACT_FEATURE_SUPPORTED (0x04)

/* Sensor contact bits when feature is supported (bit 1 - sensor contact bit)*/
#define HR_SENSOR_NOT_IN_CONTACT \
                                       (HR_SENSOR_CONTACT_FEATURE_SUPPORTED | 0x00)
#define HR_SENSOR_IN_CONTACT\
                                       (HR_SENSOR_CONTACT_FEATURE_SUPPORTED | 0x02)

/* Energy expended status bit (Bit 3 of the flag) */
#define HR_ENERGY_EXP_PRESENT            (0x08)

/* RR-Interval status bit (Bit 4 of the flag) */
#define HR_RR_INTERVAL_PRESENT           (0x10)

/* Body Sensor Location unknown */
#define HR_BODY_SENSOR_LOCATION_UNKNOWN  (0x00)

/* Control  Sensor Location unknown */
#define HR_CONTROL_POINT_NOT_SUPPORTED   (0x00)

/*!
    @brief Parameters used by the Initialisation API, valid value of these  parameters are must for library initialisation  
*/
typedef struct
{
     uint16 cid;           /*!Connection ID of the GATT connection on which the server side Heart Rate Service need to be accessed*/
     uint16 start_handle;  /*! The first handle of Heart Rate Service need to be accessed*/
     uint16 end_handle;    /*!The last handle of Heart Rate Service need to be accessed */
} GATT_HEART_RATE_CLIENT_INIT_PARAMS_T;

/*!
    @brief Initialises the Heart Rate Client Library.
     Initialize heart rate client library handles, It starts finding out the characteristic handles of Heart Rate Service.
     Once the initialisation has been completed, GATT_HEART_RATE_CLIENT_INIT_CFM will be received with 
     status as enumerated as gatt_heart_rate_client_status.'gatt_heart_rate_client_status_success' has to 
     be considered initialisation of library is done successfully and all the required charecteristics has been found out

     NOTE:This interface need to be invoked for every new gatt connection when the client wish to use 
     heart rate client library  

    @param appTask The Task that will receive the messages sent from this heart rate client  library.
    @param heart_rate_client A valid area of memory that the service library can use.Must be of at least the size of GHRSC_T
    @param client_init_params as defined in GATT_HEART_RATE_CLIENT_INIT_PARAMS_T, it is must all the parameters are valid
                The memory allocated for GATT_HEART_RATE_CLIENT_INIT_PARAMS_T can be freed once the API returns.
    @param device_data Pointer to heart rate device data structure
    @return TRUE if success, FALSE otherwise

*/
bool GattHeartRateClientInit(Task appTask , 
                             GHRSC_T *const heart_rate_client,
                             const GATT_HEART_RATE_CLIENT_INIT_PARAMS_T *const client_init_params,
                             const gatt_heart_rate_client_device_data_t *device_data);


/*!@brief Heart Rate Client Library initialisation confirmation 
*/
typedef struct __GATT_HEART_RATE_CLIENT_INIT_CFM
{
    const GHRSC_T *heart_rate_client;      /*! Reference structure for the instance */
    uint8  hr_sensor_location;             /*! Heart Rate Sensor Location, if supported by sensor*/
    uint16 hr_control_point_support;       /*! Heart Rate control point to reset energy expended, if supported by sensor*/
    gatt_heart_rate_client_status status;  /*! Status as per gatt_heart_rate_client_status */

} GATT_HEART_RATE_CLIENT_INIT_CFM_T;

/*!
    @brief Heart Rate Client Library notification registration confirmation
             This confirmation will be received on Enabling/Disabling notification 
*/
typedef struct __GATT_HEART_RATE_CLIENT_NOTIFICATION_CFM
{
    const GHRSC_T *heart_rate_client;       /*! Reference structure for the instance */
    gatt_heart_rate_client_status status;   /*! status as per gatt_heart_rate_client_status */
} GATT_HEART_RATE_CLIENT_NOTIFICATION_CFM_T;

/*!
    @brief Heart Rate Client Library confirmation for reset of energy expended values
             This confirmation will be received on Reset of EE  request by application
*/
typedef struct __GATT_HEART_RATE_CLIENT_RESET_EE_CFM
{
    const GHRSC_T *heart_rate_client;     /*! Reference structure for the instance */
    gatt_heart_rate_client_status status; /*! status as per gatt_heart_rate_client_status */
} GATT_HEART_RATE_CLIENT_RESET_EE_CFM_T;

/*!
    @brief Heart Rate Client Library notification indication, This indication will be received on each notification from HR service   
*/
typedef struct __GATT_HEART_RATE_CLIENT_NOTIFICATION_IND
{
    const GHRSC_T *heart_rate_client;       /*! Reference structure for the instance */
    uint16 heart_rate_characteristic_flags; /*! Heart rate characteristic flags */
    uint16 heart_rate_value;                /*! Heart rate value, mandatory */
    uint16 energy_expended;                 /*! Energy expended value, if present */
    uint16 size_rr_interval;                /*! Size of RR interval, if present. Note RR interval is a uint16 entity 
                                                 and we are passing it as uint8, hence passing the size */
    uint8  rr_interval[1];                  /*! RR interval value, if present */
} GATT_HEART_RATE_CLIENT_NOTIFICATION_IND_T;

/*!
    @brief DeInitialises the Heart Rate Client Library.
    When a GATT connection is removed, the application must remove all client service instances that were
    associated with the connection (using the CID value).This is the clean up routine as a result of calling the 
    GattHeartRateClientInit API. That is, the GattHeartRateClientInit API is called when a connection is made, 
    and the GattHeartRateClientDestroy is called when the connection is removed.

    @param heart_rate_client The client instance that was passed into the GattHeartRateClientInit API.  
    @return TRUE if success, FALSE otherwise

*/
bool GattHeartRateClientDeInit(GHRSC_T *heart_rate_client);

/*!
    @brief Register for heart rate measurement notification from peer.

    @param heart_rate_client The client instance that was passed into the GattHeartRateClientInit API.
    @param enable Enable or Disable notification.   
    @return TRUE if success, FALSE indicates the heart rate client is not yet initialised to process the request
*/
bool GattHeartRateRegisterForNotification(const GHRSC_T *heart_rate_client,  bool enable);

/*!
    @brief Reset energy expended field.

    @param heart_rate_client The client instance that was passed into the GattHeartRateClientInit API.
    @return TRUE if success, FALSE otherwise
*/
bool GattHeartRateResetEnergyExpendedReq(const GHRSC_T *heart_rate_client);


#endif /* GATT_HEART_RATE_CLIENT_H */


/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. 

*/

/*!
@file    
@brief   Header file for the GATT Running Speed and Cadence Service library.

        This file provides documentation for the GATT Running Speed and Cadence 
        Service library API (library name: gatt_running_speed_cadence_server).
*/

#ifndef GATT_RUNNING_SPEED_CADENCE_SERVER_H
#define GATT_RUNNING_SPEED_CADENCE_SERVER_H


#include <csrtypes.h>
#include <message.h>

#include <library.h>

#include "gatt_manager.h"

/*! @brief Running Speed and Cadence Service library  data structure type .
 */

/* This structure is made public to the application as it is responsible for 
 * managing resources for the elements of this structure. The data elements are  
 * intended for use by the Running Speed and Cadence Service lib only.  
 * The Application SHOULD NOT access (read/write) any elements of this library 
 * structure at any point in time and doing so may cause undesired behavior.
 */
typedef struct _gatt_rsc_server_t
{
    TaskData lib_task;
    Task app_task;
}_gatt_rsc_server_t;

/*! @brief GATT Running Speed and Cadence Service[GRSCS]Library Instance.
 */
typedef struct  _gatt_rsc_server_t GRSCS_T;

/*! @brief Contents of the GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND message that 
    is sent by the library, due to a read of the Running Speed and Cadence 
    measurement client configuration characteristic.
 */
typedef struct __GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND
{
    const GRSCS_T *rscs;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
} GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND message that
    is sent by the library, due to a read of the measurement client configuration characteristic.
 */
typedef struct __GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND
{
    const GRSCS_T *rscs;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
} GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND message that 
    is sent by the library, due to a write of the  Running Speed and Cadence 
    measurement client configuration characteristic.
 */
typedef struct __GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND
{
    const GRSCS_T *rscs;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
    uint16 config_value;        /*! Client Configuration value to be written */
} GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND message that
    is sent by the library, due to a write of the measurement client configuration characteristic.
 */
typedef struct __GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND
{
    const GRSCS_T *rscs;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
    uint16 config_value;        /*! Client Configuration value to be written */
} GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_RSC_SERVER_READ_RSC_FEATURE_IND message that is 
    sent by the library, due to a read of the Running Speed and Cadence feature 
    characteristic value.
 */
typedef struct __GATT_RSC_SERVER_READ_RSC_FEATURE_IND
{
    const GRSCS_T *rscs;        /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
} GATT_RSC_SERVER_READ_RSC_FEATURE_IND_T;

/*! @brief Enumeration of messages an application task can receive from the 
    Running Speed and Cadence Service library.
 */
typedef enum
{
    /* Server messages */
    GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND = GATT_RSC_SERVER_MESSAGE_BASE,  /* 00 */
    GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND,                                /* 01 */
    GATT_RSC_SERVER_READ_RSC_FEATURE_IND,                                   /* 02 */
    GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND,                       /* 03 */
    GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND,                      /* 04 */
    
    /* Library message limit */
    GATT_RSC_SERVER_MESSAGE_TOP
} gatt_rsc_server_message_id_t;

/*!
    @brief  Initializes the Running Speed and Cadence Service Library.

    @param  appTask     The Task that will receive the messages sent from this 
                        Running Speed and Cadence Service library.
                        
    @param  rscs        A valid area of memory that the Running Speed and 
                        Cadence Service library can use.Must be of at least the 
                        size of GRSCS_T
                        
    @param  start_handle This indicates the start handle of the RSC Service
    
    @param  end_handle   This indicates the end handle of the RSC Service
    
    @return TRUE if success, FALSE otherwise.

*/
bool GattRSCServerInit(Task appTask, GRSCS_T *const rscs, uint16 start_handle, uint16 end_handle);

/*!
    @brief  This API is used to return a Running Speed and Cadence Measurement 
            client configuration value to the library when a 
            GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND message is received.

    @param  rscs            The pointer that was in the payload of the 
                            GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND message.
                            
    @param  cid             The connection identifier from the 
                            GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND message.
                            
    @param  client_config   The client configuration to return to the library
    
    @return TRUE if success, FALSE otherwise

*/
bool GattRSCServerReadClientConfigResponse(const GRSCS_T *rscs, uint16 cid, uint16 client_config);

/*!
    @brief  This API is used to return a Running Speed and Cadence Measurement
            client configuration value to the library when a
            GATT_MEASUREMENT_SERVER_READ_CLIENT_CONFIG_IND message is received.

    @param  rscs            The pointer that was in the payload of the
                            GATT_MEASUREMENT_SERVER_READ_CLIENT_CONFIG_IND message.

    @param  cid             The connection identifier from the
                            GATT_MEASUREMENT_SERVER_READ_CLIENT_CONFIG_IND message.

    @param  client_config   The client configuration to return to the library

    @return TRUE if success, FALSE otherwise

*/
bool GattRSCExtensionServerReadClientConfigResponse(const GRSCS_T *rscs, uint16 cid, uint16 client_config);

/*!
    @brief  This API is used to notify a remote Running Speed and Cadence client
            of the Running Speed and Cadence measurement value. This will only 
            be allowed if notifications have been enabled by the remote device.

    @param  rscs        The instance pointer that was passed into the 
                        GattRSCServerInit API.
                        
    @param  cid         The connection identifier from the 
                        GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND message.
                        
    @param  rscm_length The length of the Running Speed and Cadence measurement 
                        value.
    
    @param  rscm_data   The Running Speed and Cadence measurement value to send 
                        in the notification. The Running Speed and Cadence 
                        measurement data is as follows
     
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
bool GattRSCServerSendNotification(const GRSCS_T *rscs, uint16 cid, uint16 rscm_length, uint8 *rscm_data);

/*!
    @brief  This API is used to notify a remote Running Speed and Cadence client
            of the Running Speed and Cadence extension values. This will only 
            be allowed if notifications have been enabled by the remote device.

    @param  rscs        The instance pointer that was passed into the 
                        GattRSCServerInit API.
                        
    @param  cid         The connection identifier from the 
                        GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND message.
                        
    @param  rscm_length The length of the Running Speed and Cadence extension 
                        value.
    
    @param  rscm_data   The Running Speed and Cadence extension value to send 
                        in the notification. The Running Speed and Cadence 
                        extension data is as follows
     
    The  running speed and cadence step extension notification data
       --------------------------------------------------------------------------------------------------------------------
       | Flags | Step Count | Context | Total Distance | Norm Acceleration |  Energy Used |-----------
       --------------------------------------------------------------------------------------------------------------------
       Flags:                       1 byte
       Step count:                  2 byte (optional)
       Context:                     1 byte (optional)
       Total Distance:              2 byte (optional)
       Norm Acc:                    1 byte (optional)
       Energy Used:                 2 byte (optional)
    
    @return TRUE if success, FALSE otherwise

*/
bool GattRSCServerSendNotificationExtension(const GRSCS_T *rscs, uint16 cid, uint16 rscm_length, uint8 *rscm_data);

#endif /* GATT_RUNNING_SPEED_CADENCE_SERVER_H */


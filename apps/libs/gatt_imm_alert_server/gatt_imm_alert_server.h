/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file    gatt_imm_alert_server.h
@brief   Header file for the GATT immediate alert service server library.

        This file provides documentation for the GATT Imediate Alert Server library
        API (library name: gatt_imm_alert_server).
*/

#ifndef GATT_IMM_ALERT_SERVER_H
#define GATT_IMM_ALERT_SERVER_H


#include <csrtypes.h>
#include <message.h>

#include <library.h>

#include "gatt_manager.h"

/*! @brief Immediate Alert service server library  data structure type .
 */

/* This structure is made public to application as application is responsible for managing resources 
* for the elements of this structure. The data elements are indented to use by Immediate Alert Service Server lib only. 
* Application SHOULD NOT access (read/write) any elements of this library structure at any point of time and doing so  
* may cause undesired behaviour of this library functionalities
*/
typedef struct _gatt_imm_alert_server_t
{
    TaskData lib_task;
    Task app_task;
}_gatt_imm_alert_server_t;

/*! @brief GATT Immediate Alert Service Server[GIASS]Library Instance.
 */
typedef struct  _gatt_imm_alert_server_t GIASS_T;

/*!
    @brief Enumeration for Alert level Value of Alert Level characteristic for immediate alert service 
*/

/* For Alert Level characteristic value, refer http://developer.bluetooth.org/
 * gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.
 * characteristic.alert_level.xml
 */
typedef enum {

    alert_level_no          = 0,     /* No Alert request */
    alert_level_mild        = 1,    /* Mild Alert request */
    alert_level_high        = 2,    /* High Alert request  */
    alert_level_reserved
}gatt_imm_alert_level;

/*!@brief Alert level write indication type
*/
typedef struct
{
    const GIASS_T *imm_alert_service;   /*! Reference structure for the instance  */
    uint16 cid;                                       /*! Connection ID */
    gatt_imm_alert_level alert_level;        /*! Alert level requested */
} GATT_IMM_ALERT_SERVER_WRITE_LEVEL_IND_T;

/*! @brief Enumeration of messages a client task may receive from the immediate alert server library.
 */
typedef enum
{
    GATT_IMM_ALERT_SERVER_WRITE_LEVEL_IND = GATT_IMM_ALERT_SERVER_MESSAGE_BASE,

     /* Library message limit */
    GATT_IMM_ALERT_SERVER_MESSAGE_TOP
} gatt_imm_alert_sever_service_message_id_t;


/*!
    @brief initialises the Immediate Alert Service Server Library.

    @param appTask The Task that will receive the messages sent from this immediate alert service library.
    @param imm_alert_server A valid area of memory that the service library can use.Must be of at least the size of GIASS_T
    @param start_handle This indicates the start handle of the IAS service
    @param end_handle This indicates the end handle of the IAS service
    
    @return TRUE if success, FALSE otherwise.

*/
bool GattImmAlertServiceServerInit(Task appTask ,
                                GIASS_T *const imm_alert_server,
                                uint16 start_handle,
                                uint16 end_handle);


#endif /* GATT_IMM_ALERT_SERVER_H */      


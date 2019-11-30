/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file    
@brief   Header file for the GATT Transport Discovery Service library.

        This file provides documentation for the GATT Transport Discovery Service library
        API (library name: gatt_transport_discovery_server).
*/

#ifndef GATT_TRANSPORT_DISCOVERY_SERVER_H
#define GATT_TRANSPORT_DISCOVERY_SERVER_H


#include <csrtypes.h>
#include <message.h>

#include <library.h>

#include "gatt_manager.h"


/* There currently are no transport-specific data. Update this value if needed.*/
#define TRANSPORT_SPECIFIC_DATA_SIZE 0

/*
    The TDS indication is made up of an octet for the requested Op Code, an octet
    for the result code, and optionally an octet for the relevant Organization ID
    followed by transport-specific data up to the size of the MTU.
*/
#define TRANSPORT_DISCOVERY_INDICATION_SIZE    (3 + TRANSPORT_SPECIFIC_DATA_SIZE)

#define activate_transport 0x01
#define SIG_org_ID 0x01


/* All current result codes. */
#define op_code_success             0x00
#define op_code_not_supported       0x01
#define op_code_invalid_parameter   0x02
#define op_code_unsupported_org_id  0x03
#define op_code_operation_failed    0x04


/*! @brief Transport Discovery Server library  data structure type .
 */

/* This structure is made public to application as application is responsible for managing resources 
 * for the elements of this structure. The data elements are indented to use by Transport Discovery Server lib only. 
 * Application SHOULD NOT access (read/write) any elements of this library structure at any point of time and doing so  
 * may cause undesired behavior of this library's functionalities.
 */
typedef struct GTDS_T
{
    TaskData lib_task;
    Task app_task;
} GTDS_T;

/*! @brief Contents of the GATT_TRANSPORT_DISCOVERY_SERVER_READ_CLIENT_CONFIG_IND message that is sent by the library,
    due to a read of the TDS client configuration characteristic.
 */
typedef struct __GATT_TRANSPORT_DISCOVERY_SERVER_READ_CLIENT_CONFIG_IND
{
    const GTDS_T *tds;      /*! Reference structure for the instance  */
    uint16 cid;             /*! Connection ID */
} GATT_TRANSPORT_DISCOVERY_SERVER_READ_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CLIENT_CONFIG_IND message that is sent by the library,
    due to a write of the TDS client configuration characteristic.
 */
typedef struct __GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CLIENT_CONFIG_IND
{
    const GTDS_T *tds;      /*! Reference structure for the instance  */
    uint16 cid;             /*! Connection ID */
    uint16 config_value;    /*! Client Configuration value to be written */
} GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CONTROL_POINT_IND message that is sent by the library,
    due to a write of the TDS control point characteristic.
 */
typedef struct __GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CONTROL_POINT_IND
{
    const GTDS_T *tds;      /*! Reference structure for the instance  */
    uint16 cid;             /*! Connection ID */
    uint16 handle;
    uint16 size_value;      /*! Size of value to be written */
    uint16 value[1];        /*! Control Point value to be written */
} GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CONTROL_POINT_IND_T;

/*! @brief Enumeration of messages an application task can receive from the TDS library.
 */
typedef enum
{
    /* Server messages */
    GATT_TRANSPORT_DISCOVERY_SERVER_READ_CLIENT_CONFIG_IND = GATT_TRANSPORT_DISCOVERY_SERVER_MESSAGE_BASE,  /* 00 */
    GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CLIENT_CONFIG_IND,                                                /* 01 */
    GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CONTROL_POINT_IND,
    
    /* Library message limit */
    GATT_TRANSPORT_DISCOVERY_SERVER_MESSAGE_TOP
} gatt_tds_server_message_id_t;

/*!
    @brief Initializes the Transport Discovery Service Library.

    @param appTask The Task that will receive the messages sent from this TDS library.
    @param tds A valid area of memory that the TDS library can use.Must be of at least the size of GTDS_T
    @param start_handle This indicates the start handle of the service
    @param end_handle This indicates the end handle of the service
    
    @return TRUE if success, FALSE otherwise.

*/
bool GattTransportDiscoveryServerInit(Task appTask, GTDS_T *const tds, uint16 start_handle, uint16 end_handle);

/*!
    @brief This API is used to return a TDS client configuration value to the library when a 
    GATT_TRANSPORT_DISCOVERY_SERVER_READ_CLIENT_CONFIG_IND message is received.

    @param tds The pointer that was in the payload of the GATT_TRANSPORT_DISCOVERY_SERVER_READ_CLIENT_CONFIG_IND message.
    @param cid The connection identifier from the GATT_TRANSPORT_DISCOVERY_SERVER_READ_CLIENT_CONFIG_IND message.
    @param client_config The client configuration to return to the library
    
    @return TRUE if success, FALSE otherwise

*/
bool GattTdsServerReadClientConfigResponse(const GTDS_T *tds, uint16 cid, uint16 client_config);


/*!
    @brief  This API is used to send an indication to a remote Transport 
            Discovery client regarding the write to the Control Point. This will  
            only be allowed if notifications have been enabled by the remote device.

    @param tds              The instance pointer that was passed into the 
                            GattTransportDiscoveryServerInit API.
                        
    @param cid              The connection identifier from the 
                            GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CONTROL_POINT_IND message.
                        
    @param tds_ind_size     The length of the Transport Discovery indication
    
    @param tds_ind_data     The Transport Discovery indication itself
    
    @return TRUE if success, FALSE otherwise

*/
bool GattTdsServerSendNotification(const GTDS_T *tds, uint16 cid, uint16 tds_ind_size, uint8 *tds_ind_data);


/*!
    @brief Sends a response to the client through the GATT manager.

    @param ind   The message from the client that caused the error
    @param result        The response code to be sent
    
    @return TRUE if success, FALSE otherwise.

*/                                   
void GattTdsServerSendResponse(const GATT_TRANSPORT_DISCOVERY_SERVER_WRITE_CONTROL_POINT_IND_T *ind, uint16 result);


#endif /* GATT_TRANSPORT_DISCOVERY_SERVER_H */


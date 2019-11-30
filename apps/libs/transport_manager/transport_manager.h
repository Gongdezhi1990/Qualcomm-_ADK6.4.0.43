/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file     transport_manager.h
@brief   This module manages different transports that is required 
            to achieve the application use case..
*/
#ifndef TRANSPORT_MANAGER_H_
#define TRANSPORT_MANAGER_H_

#include <bdaddr.h>
#include <library.h>
#include <gatt.h>

/*!
    @brief types of transports managed by transport manager
*/
typedef enum
{
    transport_mgr_type_none = 0,
    transport_mgr_type_rfcomm,
    transport_mgr_type_gatt,
    transport_mgr_type_accessory,
    transport_mgr_type_max
}transport_mgr_type_t;

/*!
    @brief types of status codes that could be expected from transport manager
*/
typedef enum
{
    transport_mgr_status_fail = 0,
    transport_mgr_status_success,
    transport_mgr_status_not_registered,
    transport_mgr_status_invalid_params,
    transport_mgr_status_not_initialized
}transport_mgr_status_t;


/*!
    @brief upstream messages.
    Messages to be communicated to the respective task registered with
    transport manager.

*/
typedef enum
{
    TRANSPORT_MGR_LINK_CREATED_CFM = TRANSPORT_MGR_MESSAGE_BASE,
    TRANSPORT_MGR_LINK_DISCONNECTED_CFM,
    TRANSPORT_MGR_REGISTER_CFM,
    TRANSPORT_MGR_DEREGISTER_CFM,
    TRANSPORT_MGR_MORE_DATA,
    TRANSPORT_MGR_MORE_SPACE,
    TRANSPORT_MGR_TOP
}transport_mgr_message_id_t;

/*!
    @brief Transport config used for GATT
    Contains the first handle in the pre-built GATT database that the server task is interested in 
    and the last handle in the pre-built GATT database that the server task is interested in
    start_handle and end_handle represent the range of handles on which the GATT client could
    be notified from a respective server
Note: Only GATT Server role shall register with transport manager.*/
typedef struct
{
   uint16           start_handle;  
   uint16           end_handle;    
}transport_mgr_gatt_trans_config_t;

/*!
    @brief Non-GATT based Transport config
    Contains the link identifier used for non gatt specific transports
    for eg:RFCOMM
    */
typedef struct
{
 uint16 trans_link_id; 
}transport_mgr_nongatt_trans_config_t;

/*!
    @brief Transport Config
    The common transport config information which
    would be used to share the trasnport specific 
    information with transport manager
    Always either of 
    transport_mgr_gatt_trans_config_t
    or
    transport_mgr_nongatt_trans_config_t
    is expected be in use*/

typedef struct
{
    transport_mgr_type_t type;
    union
    {
    transport_mgr_gatt_trans_config_t gatt_trans;
    transport_mgr_nongatt_trans_config_t non_gatt_trans;
    }trans_info;
}transport_mgr_link_cfg_t;

/*!
    @brief Confirm the link id based on transport registration
    @param status of registration
    @param pointer to transport config info structure
*/
typedef struct
{
    bool    status; 
    transport_mgr_link_cfg_t link_cfg;
}TRANSPORT_MGR_REGISTER_CFM_T;

/*!
    @brief Confirmation for Deregistering the transport

    @param status of deregistering procedure
    @param transport type
    @param transport link identifier
Note : In case of GATT no de-registration happens as only 
        GATT Server role is supported.

*/
typedef struct
{
    bool    status; 
    transport_mgr_type_t type;
    uint16 trans_link_id;
} TRANSPORT_MGR_DEREGISTER_CFM_T;

/*!
    @brief confirm the successful connection of link. But in case of 
    GATT, we can expect TRANSPORT_MGR_LINK_CREATED_CFM
    without being triggered connection from transport manager.

    @param status of link creation procedure
    @param pointer to transport config info structure
    @param trans_sink: The associated sink with the connection.Shall
                be NULL when link_cfg->type is transport_mgr_type_gatt


Note: In case of GATT (non stream) trans_sink is not valid
*/
typedef struct
{
    bool    status; 
    transport_mgr_link_cfg_t link_cfg;
    Sink trans_sink;
    tp_bdaddr addr;
}TRANSPORT_MGR_LINK_CREATED_CFM_T;

/*!
    @brief Indicate the successful disconnection of link

    @param status of link disconnection
    @param pointer to transport config info structure
    @param trans_link_id : The Link id used for communication with
                                        underlying transport.
    @param trans_sink : The associated sink with the connection
Note: In case of GATT (non stream) trans_sink is not valid
*/
typedef struct
{
    bool    status; 
    transport_mgr_link_cfg_t link_cfg;
    uint16 trans_link_id;
    Sink trans_sink;
}TRANSPORT_MGR_LINK_DISCONNECTED_CFM_T;


/*!
    @brief Indicate space is available to write to a transport

    @param trans_link_id : The Link id used for communication with
                                        underlying transport.
    @param type: type of the transport for which the link is created.
*/
typedef struct
{
    uint16 trans_link_id;
    transport_mgr_type_t type;
}TRANSPORT_MGR_MORE_SPACE_T;

/*!
    @brief Indication to the application about the data that is recieved. 
     This message would just tell that the data is available and the 
     length of the data that could be read by application

    @param transport_mgr_type_t transport
    @param trans_link_info : The Link information used for communication with
    underlying transport.In case of GATT it would be handle of a particular
    characteristic

*/
typedef struct
{
    transport_mgr_type_t type;
    uint16 trans_link_info;
} TRANSPORT_MGR_MORE_DATA_T;


/*******************Common APIs (Used for both GATT as well as NON-GATT transport types)******************/


/*!
    @brief Initialize the transport manager module
 
    @param 
 
    @return 
*/

void TransportMgrInit(void);


/*!
    @brief Register the app_task with transport manager module 
              with respective link configuration.

    @param AppTask        : The task handler to which the transport 
                                      manager would direct all the messages to.
    @param transport_mgr_link_cfg_t : pointer to link configuration structure.

    @return transport_mgr_status_t
    A TRANSPORT_MGR_REGISTER_CFM is returned with the status 
    of the registration.
    
*/
transport_mgr_status_t TransportMgrRegisterTransport(Task app_task, transport_mgr_link_cfg_t *trans_link_cfg);


/*********************************DATA Read Transaction *****************************************************


    Client registered with TM                                   Transport Manager

                        TRANSPORT_MGR_MORE_DATA
                <--------------------------------------
                        TransportMgrGetAvailableDataSize
                --------------------------------------->
                        TransportMgrReadData
                --------------------------------------->
                        TransportMgrDataConsumed
                --------------------------------------->
*/


    
  /*!
    @brief To obtain the size of data available for application to read
 
    @param type: Transport type 
    @param trans_link_info : The Link information used for communication with
    underlying transport.

 
    @return 
    datasize_available: length of data that could be read.
*/

uint16 TransportMgrGetAvailableDataSize(transport_mgr_type_t type,uint16 trans_link_info);

 /*!
    @brief Application could read the data once after recieving TRANSPORT_MGR_MORE_DATA
 
    @param type: Transport type 
    @param trans_link_info : The Link information used for communication with
    underlying transport.In case of GATT it would be handle of a particular
    characteristic
 
    @return 
    pointer to data that is read
Note : Once the data read is processed by the application, the api "TransportMgrDataConsumed"
        should be called to properly close each read transaction. If a subsequent read is attempted
        without calling "TransportMgrDataConsumed", the buffer returned by the previous read will 
        be returned again.
*/

const uint8* TransportMgrReadData(transport_mgr_type_t type,uint16 trans_link_info);

 /*!
    @brief Application would let transport manager know about the data to be dropped
    from the recieve buffer.
 
    @param type: Transport type 
    @param trans_link_info : The Link information used for communication with
    underlying transport.In case of GATT it would be handle of a particular
    characteristic
    @param data_consumed: Length of data to be dropped.
 
    @return transport_mgr_status_t
Note:  Once the data read is consumed, the api "TransportMgrDataConsumed" should
       be called to properly close each read transaction. If a subsequent read
       is attempted without calling "TransportMgrDataConsumed", the buffer 
       returned by the previous invocation of "TransportMgrDataRead" will
       be returned again.In case of GATT (non stream) transport data_consumed is not valid
       as the complete data received as part of TRANSPORT_MORE_DATA  would be freed.
*/

transport_mgr_status_t TransportMgrDataConsumed(transport_mgr_type_t type,uint16 trans_link_info,uint16 data_consumed);


/*********************DATA Write Transaction *****************

    Client registered with TM                                   Transport Manager

                        TransportMgrGetAvailableSpace
                ---------------------------------->
                        TransportMgrWriteData
                ---------------------------------->
                        TransportMgrDataSend (This call shall
                        be executed only in case of Non Gatt 
                        base transport types)
                ---------------------------------->
*/


 /*!
    @brief To write the data onto the respective transport
 
    @param type: Transport type 
    @param trans_link_info : The Link information used for communication with
    underlying transport.In case of GATT it would be handle of a particular
    characteristic
    @param data_buffer : Pointer to the data that has to be sent to remote
    @param data_length: Length of data to be written.
 
    @return 
    TRUE when the write is success 
    else FALSE
Note : In case of Non Gatt based transports, the write action would be complete
       only when "TransportMgrDataSend" is called after calling TransportMgrWriteData.
*/

bool TransportMgrWriteData(transport_mgr_type_t type,uint16 trans_link_info,uint8 *data_buffer,uint16 data_length);

  /*!
    @brief To obtain the information about the space available before 
    performing a write operation.
 
    @param type: Transport type 
    @param trans_link_info : The Link information used for communication with
    underlying transport.In case of GATT regardless of trans_link_info the available space 
    would be returned

 
    @return 
    space_available: length of data that could be written.
*/

uint16 TransportMgrGetAvailableSpace(transport_mgr_type_t type,uint16 trans_link_info);


/******************* APIs for NON-GATT transport type***********************/

/*!
     @brief Send data over the respective transport. This shall be applicable only for 
     Non Gatt transport write operation.
  
     @param type: Transport type (RFCOMM) 
     @param trans_link_id : The Link id used for communication with
                            underlying transport

     @return API usage validation (error code)
 */
transport_mgr_status_t TransportMgrDataSend(transport_mgr_type_t type, uint16 trans_link_id, uint16 len);

 /*!
    @brief Unregister the requested transport..

    @param AppTask        : The task handler to which the transport 
                                      manager would direct all the messages to.
    @param type: non GATT Transport type (RFCOMM) 
    @param link_id : The Link id used for communication with
                            underlying transport
 
    @return transport_mgr_status_t
    A TRANSPORT_MGR_DEREGISTER_CFM is returned with the status 
    of the de-registration
*/
 transport_mgr_status_t TransportMgrDeRegisterTransport(Task app_task,transport_mgr_type_t type, uint16 link_id);

 /*!
    @brief Register a named Accessory protocol.

    @param name The name of the Accessory protocol.
    @param id the identifier of the named protocol.
 
    @return transport_mgr_status_t
*/
 transport_mgr_status_t TransportMgrRegisterAccessory(uint16 *id, char *name);
 
/*!
    @brief Create connection for the respective transport that is requested.
 
    @param type : non GATT Transport type (RFCOMM) 
    @param trans_link_id : The Link id used for communication with
                                        underlying transport.
    @param remote_trans_link_id : The remote devices link id
 
    @return transport_mgr_status_t
    A TRANSPORT_MGR_LINK_CREATED_CFM is returned with the status of 
    link creation procedure.
*/
transport_mgr_status_t TransportMgrConnect(transport_mgr_type_t type, uint16 trans_link_id,uint8 remote_trans_link_id, tp_bdaddr *remote_addr);

/*!
    @brief Disconnect the link running on the respective transport.
    @param type: non GATT Transport type (RFCOMM) 
    @param Sink   The associated sink with the connection
 
    @return transport_mgr_status_t
    A TRANSPORT_MGR_LINK_DISCONNECTED_CFM is returned with the 
    status of link disconnection procedure.
*/
transport_mgr_status_t TransportMgrDisconnect(transport_mgr_type_t type, Sink disconnect_sink);

/******************* APIs used for GATT transport type***********************/

  /*!
    @brief The status of the GATT client connection shall be notified to transport
    manager if GATT transport type is registered with transport manager
 
    @param status: gatt_status_t describing the status of the connection 
    @param cid : gatt connection identifier
.
 
    @return 

*/

void TransportMgrClientConnected(gatt_status_t status,uint16 cid);

 /*!
    @brief The status of the GATT client disconnection shall be notified to transport
    manager if GATT transport type is registered with transport manager
 
    @param status: gatt_status_t describing the status of the connection 
    @param cid : gatt connection identifier
.
 
    @return 

*/

void TransportMgrClientDisconnected(gatt_status_t status,uint16 cid); 

#endif

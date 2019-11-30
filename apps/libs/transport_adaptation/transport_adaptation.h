/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/
 
 
/*!
@file    transport_adaptation.h
@brief   This module provides necessary routines for the transport manager
          layer to access supported underlying transport.
 
         This file provides documentation for the transport_adaptation API.
*/


#ifndef TRANSPORT_ADAPTATION_H
#define TRANSPORT_ADAPTATION_H

#include<bdaddr.h>
#include<library.h>
#include<sink_.h>
#include<message.h>
#include<gatt.h>

/*!
@brief Transport type chosen.
*/
typedef enum
{
    TRANSPORT_RFCOMM,
    TRANSPORT_GATT,
    TRANSPORT_ACCESSORY,
    TRANSPORT_TOP
} transport_type_t;

/*!
@brief Defintions of Transport adaptation layer messages.
*/
typedef enum
{
    TRANSPORT_REGISTER_CFM = TA_MESSAGE_BASE,
    TRANSPORT_DEREGISTER_CFM,
    TRANSPORT_CONNECT_CFM,
    TRANSPORT_DISCONNECT_CFM,
    TRANSPORT_MESSAGE_MORE_DATA,
    TRANSPORT_MESSAGE_TOP
}transport_adaptation_message_id_t;

/*!
@brief The Transport Register confirm message.
*/
typedef struct
{
    /*! Transport adaptation layer status.*/
    bool                 status;

    /*! Tranport type chosen. */
    transport_type_t     transport;

    /*! In case of TRANSPORT_RFCOMM/TRANSPORT_LECOC transports this field gives
          the channel identifier value and in case of TRANSPORT_GATT transport it gives
          GATT service start handle value. */
    uint16               transport_id;
} TRANSPORT_REGISTER_CFM_T;

/*!
@brief The Transport Deregister confirm message.
*/
typedef struct
{
    /*! Transport adaptation layer status.*/
    bool                    status;

    /*! Tranport type chosen. */
    transport_type_t        transport;

    /*! channel identifier of the underlying transport.*/
    uint16                  transport_id;
} TRANSPORT_DEREGISTER_CFM_T;

/*!
@brief The Transport Connect confirm message.
*/
typedef struct
{
    /*! Transport adaptation layer status.*/
    bool                 status;

    /*! Transport type chosen. */
    transport_type_t     transport;

    /*! Device address.
    This field is valid only if transport_type_t is other than TRANSPORT_GATT.*/
    tp_bdaddr               addr;

    /*! The sink.
    This field is valid only if transport_type_t is other than TRANSPORT_GATT.*/
    Sink                 sink;

    /*! channel identifier of the underlying transport.
        This field is valid only if transport_type_t is other than TRANSPORT_GATT.*/
    uint16               transport_id;
} TRANSPORT_CONNECT_CFM_T;

/*!
@brief The Transport Disconnect confirm message.
*/
typedef struct
{
    /*! Transport adaptation layer status.*/
    bool                 status;

    /*! Tranport type chosen. */
    transport_type_t     transport;

    /*! Channel identifier of the underlying transport.
        This field is valid only if transport_type_t is other than TRANSPORT_GATT.*/
    Sink             sink;
    
    /*! instance identifier of the underlying transport.*/
    uint16               transport_id;
} TRANSPORT_DISCONNECT_CFM_T;

/*!
@brief The Transport GATT message more data message.
*/
typedef struct
{
    /*! Tranport type chosen. */
    transport_type_t     transport;

    /*! Handle being accessed. */
    uint16 handle;

    /*! Length of the data information. */
    uint16 data_len;

    /*! Data information. */
    uint8 data[1];
} TRANSPORT_MESSAGE_MORE_DATA_T;

/*!
    @brief Initialise the transport adaptation module and register the
    application task.

    @param app_task The client task.

    @return TRUE on success, FALSE otherwise.
*/
bool TransportInit(Task app_task);

/*! @brief Requests registration with an underlying transport channel.

    @param transport Type of underlying transport
    @param transport_id Unique channel id of the transport
    @return Returns TRUE if success, FALSE if failure.If success, 
    a #TRANSPORT_REGISTER_CFM_T message will be sent to the task
    registered with Transport adaptation layer to indicate the
    registration status and the channel allocated.
*/
bool TransportRegisterReq(transport_type_t transport, uint16 transport_id);


/*! @brief Requests Deregisteration from an underlying transport channel.

    @param transport Type of underlying transport
    @param transport_id Unique channel id of the transport
    @return Returns TRUE if success, FALSE if failure.If success, 
    a #TRANSPORT_DEREGISTER_CFM_T message will be sent to the task
    registered with Transport adaptation layer to indicate the 
    deregistration status.
*/
bool TransportDeregisterReq(transport_type_t transport,
                            uint16 transport_id);


/*! @brief Requests a transport layer connection with the remote device  with
           the specified bluetooth address and transport id.

    @param transport Type of underlying transport
    @param tp_bdaddr Typed Bluetooth address with the Transport type
    @param transport_id channel id of the transport
    @param remote_transport_id remote channel id of the transport
    @return Returns TRUE if success, FALSE if failure.If success, 
    a #TRANSPORT_CONNECT_CFM_T message will be sent to the task
    registered with Transport adaptation layer to indicate the 
    connection status and the sink created for the connection.
*/
bool TransportConnectReq(transport_type_t transport,
                         const tp_bdaddr *tpaddr,
                         uint16 transport_id,
                         uint16 remote_transport_id);

/*! @brief Requests a disconnection from an underlying transport channel
           specified by the sink.

    @param transport Type of underlying transport
    @param sink Sink allocated for the transport
    @return Returns TRUE if success, FALSE if failure.If success, 
    a #TRANSPORT_DISCONNECT_CFM_T message will be sent to the task
    registered with Transport adaptation layer to indicate the 
    disconnection status.
*/
bool TransportDisconnectReq(transport_type_t transport, Sink sink);


/*! @brief Register an Accessory transport protocol.

    @param name the name of the protocol to be registered.

    @return The protocol identifier or 0 on error.
*/
uint16 TransportAccessoryRegisterProtocol(char *name);

/*! @brief This API is used to send the payload information to the remote client
    for a given External Accessory transport.

    @param transport_id The transport identifier
    @param size_payload The length of the payload to be copied into the packet
    @param payload Pointer to the uint8[]

    @return TRUE on success else FALSE.
*/
bool TransportAccessorySendData(uint16 transport_id, uint16 size_payload, uint8 *payload);

/*! @brief Returns the available space for sending data over this transport.
    @param transport_id The transport identifier
    @return Number of bytes available.
*/
uint16 TransportAccessoryGetAvailableSpace(uint16 transport_id);

/******************************************************************************
DESCRIPTION

    @brief This function is called to register with GATT manager with start and end handles.

    @param start_handle Server characteristics start handle.
    @param end_handle Server characteristics end handle.

    @return bool Returns TRUE on success else FALSE.
*/
bool TransportGattRegisterReq(uint16 start_handle, uint16 end_handle);

/******************************************************************************
DESCRIPTION

    @brief This API is used to send the payload information to the remote client
    for a given handle. There is no response to the application task if this action fails.

    @param handle Value of the Handle
    @param size_payload The length of the payload to be copied into the packet
    @param payload Pointer to the uint8[]
    If the payload length is zero this argument is not accessed and may be NULL

    @return bool Returns TRUE on success else FALSE.
*/
bool TransportGattSendData(uint16 handle, uint16 size_payload, uint8 *payload);

/******************************************************************************
DESCRIPTION

    @brief This function is called to inform about GATT connection confirmation from application.

    @param status GATT Connection status.
    @param cid GATT Connection identifier.

    @return None
*/
void TransportGattConnected(gatt_status_t status, uint16 cid);

/******************************************************************************
DESCRIPTION

    @brief This function is called to inform about GATT disconnection confirmation from application.

    @param status GATT Connection status.
    @param cid GATT connection identifier.

    @return None
*/
void TransportGattDisconnected(gatt_status_t status, uint16 cid);

/******************************************************************************
DESCRIPTION

    @brief This function is called for responding TRANSPORT_MESSGAE_MORE_DATA message

    @param handle         Handle of the value
    @param result          Result of TRANSPORT_MESSGAE_MORE_DATA.
    @param size_value    The size of the value (octets)
    @param value           The value as an octet array, which is copied.

    @return None.
*/
void TransportGattSendServerAccessRsp(uint16 handle,
                                     uint16 result,
                                     uint16 size_value,
                                     const uint8 *value);

/*! @brief Main handler for Transport adaptation module. This handles all the 
           communication with underlying transports.

    @param task     Client Task(Currently unused)
    @param id       Id of the message to be handled
    @param message  Carries the message payload

    Note : This handler is currently exposed to the upper layer so that it 
    can handle any unknown transport specific messages/events.
*/
void TransportHandleMessage(Task task, MessageId id, Message message);

/******************************************************************************/
#endif /* TRANSPORT_ADAPTATION_H */

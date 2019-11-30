/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    spps.h
    
DESCRIPTION
	Header file for the SPP Server library.
*/

/*!
@file	spps.h
@brief	Header file for the Serial Port Profile Server

        This library implements the Serial Port Profile (SPP) specification for
        a server side connection (Device B, Acceptor). SPP can be used to emulate
        a serial port connection using RFCOMM between two peer devices.
		
        The library exposes a functional downstream API and an upstream message
        based API.
*/

#ifndef SPP_SERVER_H_
#define SPP_SERVER_H_

#include <spp_common.h>

/*!
	@brief The messages sent by the SPP library to the App.
*/

#define SPPS_MESSAGE_BASE SPP_MESSAGE_BASE
typedef enum
{
    SPPS_START_SERVICE_CFM  = SPP_START_SERVICE_CFM,
    SPPS_STOP_SERVICE_CFM   = SPP_STOP_SERVICE_CFM,
    SPPS_SERVER_CONNECT_CFM = SPP_SERVER_CONNECT_CFM,
    SPPS_CONNECT_IND        = SPP_CONNECT_IND,	
    SPPS_MESSAGE_MORE_DATA  = SPP_MESSAGE_MORE_DATA,
    SPPS_MESSAGE_MORE_SPACE = SPP_MESSAGE_MORE_SPACE,
    SPPS_DISCONNECT_IND     = SPP_DISCONNECT_IND,	
    SPPS_DISCONNECT_CFM     = SPP_DISCONNECT_CFM
} SppsMessageId;


/************************************ Typedefs *****************************/


/*!
	@brief Status codes for the SPP_START_SERVICE_CFM message.
*/
typedef enum
{
	/*! Successful initialisation of the service.*/
	spp_start_success,			
	/*! RFCOMM channel registration failed.*/
	spp_start_rfc_chan_fail,		
	/*! SPP default RFCOMM Channel could not be registered. */
    spp_start_rfc_default_fail,
	/*! Service record registration failed.*/	
	spp_start_sdp_reg_fail,		
	/*! An SPP Service is already registered and waiting for connection.*/
	spp_start_already_started				
} spp_start_status;


/*!
    @brief Status codes for the SPP_STOP_SERVICE_CFM.
*/
typedef enum
{
    /*! The service has been successfully stopped. */
    spp_stop_success,
    /*! There was no service to stop. */
    spp_stop_not_started,
    /*! The task trying to stop the service is not the task that started it.*/
    spp_stop_invalid_app_task,
    /*! Unregistering the service record failed.*/
    spp_stop_sdp_unreg_fail,
    /*! Un-allocating the RFCOMM channel failed. */
    spp_stop_rfc_chan_fail
} spp_stop_status;

/*!
	@brief This message indicates the result of starting the service for
	accepting incoming SPP connections.
    
    This message is returned by the SPP Server library in response to a call to
    SppStartService().
*/
typedef struct
{
	/*! The SPP initialisation status.*/
	spp_start_status	status;	
} SPP_START_SERVICE_CFM_T;


/*!
    @brief This message indicates the result of stopping the service for
    accepting incoming SPP connections.

    This message is returned byt the SPP Server library in response to a call
    to SppStartService().
*/
typedef struct
{
    /*! The status of trying to Stop the SPP service.*/
    spp_stop_status     status;
} SPP_STOP_SERVICE_CFM_T;


/*!
	@brief A message indicating an incoming connection.

    This message is sent by the SPP Server library to indicate that a remote 
    device is attempting to connect to our device. The client task
    must respond by calling SppConnectResponse().
*/

typedef struct
{
	/*! The Bluetooth address of the device attempting to connect. */	
	bdaddr	addr;	

    /*! The local server channel.*/
    uint8   server_channel;

    /*! The Sink associated with the link. */
    Sink    sink;
} SPP_CONNECT_IND_T;


/*!
    @brief Initialise the SPP server by registering a default SPP RFCOMM channel
    and SDP service record for the SPP service.

    @param theAppTask the task initialising the SPP Server library. This is the
    task where the SPP_INIT_CFM message will be sent.
*/
void SppStartService(Task theAppTask);


/*!
    @brief Unregister the SPP service record with SDP and unallocate the RFCOMM 
    channel for the SPP service. 

    @param theAppTask the task stopping the SPP service. This must be the same
    task that started the service. 
*/

void SppStopService(Task theAppTask);

/*!
    @brief Called by the application in response to an SPP_CONNECT_IND, this 
    function forwards the SPP RFCOM response.

    @param theAppTask the task that is responsible for receiving the SPP 
    messages once the connection is established.

    @param bd_addr The address of the remote device to respond to (indicated in 
    the SPP_CONNECT_IND message).

    @param response True for accepting and False for declining the SPP 
    connection.

    @param sink The sink as indicated in the SPP_CONNECT_IND message. 

    @param local_server_channel The server channel associated with this SPP 
    connection (as indicated in the SPP_CONNECT_IND message).

    @param max_payload_size The maximum payload size, to be negotiated for the 
    RFCOMM link.
*/


void SppConnectResponse(Task theAppTask, const bdaddr *bd_addr, const bool response, const Sink sink, const uint8 local_server_channel, const uint16 max_payload_size);


#endif /* SPP_SERVER_H_ */

/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sppc.h
    
DESCRIPTION
	Header file for the SPP Client library.
*/

/*!
@file	sppc.h
@brief	Header file for the Serial Port Profile Client

        This library implements the Serial Port Profile (SPP) specification for
        a client side connection (Device A, Initiator). SPP can be used to 
        emulate a serial port connection using RFCOMM between two peer devices.
		
        The library exposes a functional downstream API and an upstream message
        based API.
*/

#ifndef SPP_CLIENT_H_
#define SPP_CLIENT_H_

#include <spp_common.h>

#define SPPC_MESSAGE_BASE SPP_MESSAGE_BASE
typedef enum
{
    SPPC_DUMMY_MESSAGE_BASE = SPPC_MESSAGE_BASE,
    SPPC_CLIENT_CONNECT_CFM = SPP_CLIENT_CONNECT_CFM,
    SPPC_MESSAGE_MORE_DATA  = SPP_MESSAGE_MORE_DATA,
    SPPC_MESSAGE_MORE_SPACE = SPP_MESSAGE_MORE_SPACE,
    SPPC_DISCONNECT_IND     = SPP_DISCONNECT_IND,	
    SPPC_DISCONNECT_CFM     = SPP_DISCONNECT_CFM
} SppcMessageId;

/*!
	@brief Initiate a SPP connection to a remote device.

	@param theAppTask The application task to which SPP client messages should
	be returned to.

	@param bd_addr Bluetooth address of the device to connect to.
	
    @param security_channel An outgoing RFCOMM security channel (set up using 
    ConnectionSmRegisterOutgoingService() function). If this is set to 0, then
    the SPP client will create and use it's own security channel and set it to
    use the default security of 'sec4_out_ssp'.

    @param max_payload_size The requested message payload size. Setting this to 0 
    cause the default RFCOMM max_payload_size to be used. 
    
    An SPP_CLIENT_CONNECT_CFM message will be returned to the task that 
    initialised the SPP library to indicate the outcome of the connect attempt 
    and pass an instance pointer to the SPP Client task created, if the 
    connection was successful.
*/
void SppConnectRequest(Task theAppTask, const bdaddr *bd_addr, const uint16 security_channel, uint16 max_payload_size);

#endif /* SPP_CLIENT_H_ */


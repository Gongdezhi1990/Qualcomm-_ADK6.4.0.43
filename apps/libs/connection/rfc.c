/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    rfc.c        

DESCRIPTION
	File containing the rfcomm API function implementations.		

NOTES

*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"

#ifndef CL_EXCLUDE_RFCOMM

/* 
	RFCOMM Flag Defines
	These flags are used in the establishment of an RFCOMM connection.
*/

/* Bit mask with value of RFCOMM break signal. */
#define DEFAULT_RFCOMM_BREAK_SIGNAL         (0)

/* Bit mask with value of RFCOMM modem status signal. */
#define DEFAULT_RFCOMM_MODEM_STATUS_SIGNAL  (0)
/* Bit mask with value of RFCOMM flow control. */
#define DEFAULT_RFCOMM_FLOW_CTRL_MASK       (0)
/* Bit mask with value of RFCOMM parameter mask. */
#define DEFAULT_RFCOMM_PARAMETER_MASK       (RFC_PM_BIT_RATE | RFC_PM_DATA_BITS | RFC_PM_STOP_BITS | RFC_PM_PARITY | RFC_PM_PARITY_TYPE | RFC_PM_XON | RFC_PM_XOFF | RFC_PM_XONXOFF_INPUT | RFC_PM_XONXOFF_OUTPUT | RFC_PM_RTR_INPUT | RFC_PM_RTR_OUTPUT | RFC_PM_RTC_INPUT | RFC_PM_RTC_OUTPUT)



/****************************************************************************

DESCRIPTION
	This function is called to initialise the RFCOMM configuration parameters
*/
static void initPortParams(port_par *port_params)
{
    port_params->baud_rate      = RFCOMM_9600_BAUD;
    port_params->data_bits      = RFCOMM_DATA_BITS_8;
    port_params->stop_bits      = RFCOMM_STOP_BITS_ONE;
    port_params->parity         = RFCOMM_PARITY_ON;
    port_params->parity_type    = RFCOMM_PARITY_TYPE_ODD;
    port_params->flow_ctrl_mask = RFCOMM_FLC_XONXOFF_INPUT | RFCOMM_FLC_XONXOFF_OUTPUT;
    port_params->xon            = RFCOMM_XON_CHAR_DEFAULT;
    port_params->xoff           = RFCOMM_XOFF_CHAR_DEFAULT;
    port_params->parameter_mask = 0x00;
}

/*****************************************************************************/
static void initConfigParams(const rfcomm_config_params *from, rfcomm_config_params *to)
{
    if (!from)
    {
        to->max_payload_size    = RFCOMM_DEFAULT_PAYLOAD_SIZE;
        to->modem_signal        = RFCOMM_DEFAULT_MODEM_SIGNAL;
        to->break_signal        = RFCOMM_DEFAULT_BREAK_SIGNAL;
        to->msc_timeout         = RFCOMM_DEFAULT_MSC_TIMEOUT;
    }
    else
    {
        *to = *from;
    }
}


/*****************************************************************************/
void ConnectionRfcommAllocateChannel(Task theAppTask, uint8 suggested_server_channel)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_REGISTER_REQ);
    message->theAppTask = theAppTask;
    message->suggested_server_channel = suggested_server_channel;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_REGISTER_REQ, message);
}


/*****************************************************************************/
void ConnectionRfcommDeallocateChannel(Task theAppTask, uint8 local_server_channel)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_UNREGISTER_REQ);
    message->theAppTask = theAppTask;
    message->local_server_channel = local_server_channel;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_UNREGISTER_REQ, message);
}


/*****************************************************************************/
void ConnectionRfcommConnectRequest(Task theAppTask, const bdaddr* bd_addr, uint16 security_channel, uint8 remote_server_chan, const rfcomm_config_params *config)
{
#ifdef CONNECTION_DEBUG_LIB
    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr));
    }
#endif
    {
	    /* Send an internal message */
	    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_CONNECT_REQ);
	    message->theAppTask = theAppTask;
        message->bd_addr = *bd_addr;
        message->remote_server_channel = remote_server_chan;
        message->security_channel = security_channel;

        initConfigParams(config, &message->config);

        MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_REQ, message);
    }
}


/*****************************************************************************/
void ConnectionRfcommConnectResponse(Task theAppTask, bool response, const Sink sink, uint8 local_server_channel, const rfcomm_config_params *config)
{
    /* Send an internal message */
    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_CONNECT_RES);
    message->theAppTask = theAppTask;
    message->response = response;
    message->sink = sink;
    message->server_channel = local_server_channel;
    
    initConfigParams(config, &message->config);
    
    MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_RES, message);
}

/*****************************************************************************/
void ConnectionRfcommDisconnectRequest(Task appTask, Sink sink)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_DISCONNECT_REQ);
	message->theAppTask = appTask;
    message->sink = sink;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_DISCONNECT_REQ, message);
}

/*****************************************************************************/
void ConnectionRfcommDisconnectResponse(Sink sink)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_DISCONNECT_RSP);
    message->sink = sink;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_DISCONNECT_RSP, message);
}


/*****************************************************************************/
void ConnectionRfcommPortNegRequest(Task appTask, Sink sink, bool request, port_par* port_params)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_PORTNEG_REQ);
    message->theAppTask = appTask;
    message->sink = sink;
    message->request = request;

    if (port_params)
        message->port_params = *port_params;
    else
        initPortParams(&(message->port_params));
    
    MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_PORTNEG_REQ, message);
}

/*****************************************************************************/
void ConnectionRfcommPortNegResponse(Task appTask, Sink sink, const port_par* port_params)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_PORTNEG_RSP);
    message->theAppTask = appTask;
    message->sink = sink;

    if (port_params)
        message->port_params = *port_params;
    else
        initPortParams(&(message->port_params));

    MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_PORTNEG_RSP, message);
}

/*****************************************************************************/
void ConnectionRfcommControlSignalRequest(Task appTask, Sink sink, uint8 break_signal, uint8 modem_signal)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_CONTROL_REQ);

#ifdef CONNECTION_DEBUG_LIB
    if (break_signal > 0xF)
        CL_DEBUG_INFO(("break signal 0x%x is out of range!\n", break_signal));
#endif

	message->theAppTask = appTask;
    message->sink = sink;
    message->break_signal = break_signal;
    message->modem_signal = modem_signal;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONTROL_REQ, message);
}

/*****************************************************************************/
void ConnectionRfcommLineStatusRequest(Task appTask, Sink sink, bool error, rfcomm_line_status_error line_status)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_LINE_STATUS_REQ);
    message->theAppTask = appTask;
    message->sink = sink;
    message->error = error;
    message->lines_status = line_status;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_RFCOMM_LINE_STATUS_REQ, message);
}

#endif


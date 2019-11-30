/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    spp_common.h
    
DESCRIPTION
	Header file for the SPP profile library containing common data and 
	functions.
	
*/

#ifndef SPP_COMMON_H_
#define SPP_COMMON_H_

#include <library.h>
#include <panic.h>
#include <connection.h>


typedef enum
{
    SPP_START_SERVICE_CFM = SPP_MESSAGE_BASE,
    SPP_STOP_SERVICE_CFM,
    SPP_CLIENT_CONNECT_CFM,
    SPP_SERVER_CONNECT_CFM,
    SPP_CONNECT_IND,
    SPP_MESSAGE_MORE_DATA,
    SPP_MESSAGE_MORE_SPACE,
    SPP_DISCONNECT_IND,
    SPP_DISCONNECT_CFM,

    /* Library message limit */
    SPP_MESSAGE_TOP
} SppMessageId;


/*! 
    @brief SPP connection status.
*/

typedef enum
{
    /*! The connection was successful.*/
    spp_connect_success,     
    /*! The connection is pending.*/
    spp_connect_pending,
    /*! The connection failed.*/
    spp_connect_failed,                        
    /*! The server channel was not registered.*/
    spp_connect_channel_not_registered,   
    /*! The client security channel was not set */
    spp_connect_security_not_set,
    /*! The connection was declined by this application.*/
    spp_connect_declined,                        
    /*! The connection was rejected.*/
    spp_connect_rejected,                    
    /*! The client has attempted to connect to a server channel that has already been connected to. */
    spp_connect_channel_already_open,
    /*! The connection was rejected mismatched security settings.*/
    spp_connect_rejected_security,
    /*! The response to an RFCOMM connect request timed out.*/
    spp_connect_res_ack_timeout,
    /*! The connection failed because of an underlying l2cap error.*/
    spp_connect_l2cap_error,
    /*! The SDP search failed to find an SPP service.*/
    spp_connect_sdp_fail
} spp_connect_status;                        


/*! 
    @brief SPP disconnection status.
*/
typedef enum
{
    /*! The disconnection was successful.*/
    spp_disconnect_success,                                    
    /*! The disconnection attempt disconnected normally.*/
    spp_disconnect_normal_disconnect,                        
    /*! The disconnection attempt disconnected abnormally.*/
    spp_disconnect_abnormal_disconnect,                        
    /*! The disconnection attempt was refused remotely.*/
    spp_disconnect_remote_refusal,                            
    /*! Already exists.*/
    spp_disconnect_dlc_already_exists,
    /*! Rejected security.*/
    spp_disconnect_dlc_rej_security,                            
    /*! Invalid server channel.*/
    spp_disconnect_invalid_server_channel,                    
    /*! Unknown primitive encountered.*/
    spp_disconnect_unknown_primitive,                        
    /*! Maximum payload exceeded.*/
    spp_disconnect_max_payload_exceeded,                        
    /*! Inconsistent parameters.*/
    spp_disconnect_inconsistent_parameters,                    
    /*! Credit flow control protocol violation.*/
    spp_disconnect_credit_flow_control_protocol_violation,    
    /*! Sink is unknown. */
    spp_disconnect_unknown_sink,
    /*! Diconnection due to L2CAP layer Link Loss. */
    spp_disconnect_l2cap_link_loss,
    /*! Diconnection for any other L2CAP error reason.*/
    spp_disconnect_l2cap_error,
    /*! Disconnection for unknown reason.*/
    spp_disconnect_unknown                                    
} spp_disconnect_status;


struct __SPP;
/*!
	@brief The Serial Port Profile instance pointer.
*/
typedef struct __SPP SPP; 

/*!
    @brief This message indicates that a source associated with an SPP profile
    task has received data.
*/
typedef struct  
{
    /*! The SPP instance pointer. */
    SPP     *spp;

    /*! The source that has more data in it. */
    Source  source;    
} SPP_MESSAGE_MORE_DATA_T;


/*!
    @brief This message indicates that a sink associated with an SPP profile
     task has more space.
*/
typedef struct 
{
    /*! The SPP instance pointer. */
    SPP     *spp;

    /*! The sink that has more space in it. */
    Sink    sink;    
} SPP_MESSAGE_MORE_SPACE_T;


/*!
	@brief This message indicates disconnection from the result device.

    The application must respond with sppDisconnectRsp(), before the 
    associated sink will be closed.
*/
typedef struct 
{
    /*! The SPP instance pointer. */
    SPP						*spp;

	/*! The disconnect status.*/	
	spp_disconnect_status	status; 

    /*! The sink for this SPP connection. */
    Sink                    sink;
} SPP_DISCONNECT_IND_T;


/*!
	@brief This message is sent in response to SppDisconnectRequest() call
	from the application. It has the same structure as SPP_DISCONNECT_IN_T.
	Shortly after the App has received this message, the SPP task will be
	deleted.
*/
typedef SPP_DISCONNECT_IND_T SPP_DISCONNECT_CFM_T;


/*!
	@brief This message indicates the result of an SPP connection Request.

    This message is returned by the SPP Client library in response to a 
    connection  attempt initiated by calling SppConnectRequest().
*/
typedef struct 
{
    /*! The SPP instance pointer - this is only set if the connection was
    successful. */
    SPP				            *spp;

	/*! The SPP Client connection status.*/
	spp_connect_status          status;

	/*! The sink for the connection if the connection succeeded, otherwise
    set to null. */    
    Sink                        sink;
	
	/*! Negotiated RFCOMM payload size for the connection */
	uint16					    payload_size;
} SPP_CLIENT_CONNECT_CFM_T;


/*!
	@brief This message indicates the result of an SPP Server connection
	indication.

    This message is returned by the SPP Server library in response to a 
    connection response initiated by calling SppConnectResponse().
*/
typedef SPP_CLIENT_CONNECT_CFM_T SPP_SERVER_CONNECT_CFM_T;


/*!
	@brief SPP Disconnect Response called in response to a SPP_DISCONNECT_IND.

	@param spp The SPP task instance. 

    After this function call, the sink associated with the SPP profile will no
    longer be valid and the SPP task instance will shortly be deleted.
*/
void SppDisconnectResponse(SPP *spp);


/*!
	@brief The SPP Disconnect Request function can be called to disconnect an
	SPP connection. The Sink will remain valid for reading data until an
	SPP_DISCONNECT_CFM has been received by the App.

	@param spp The SPP task instance. 

*/
void SppDisconnectRequest(SPP *spp);

#endif







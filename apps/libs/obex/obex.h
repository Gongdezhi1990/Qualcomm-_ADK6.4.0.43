/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    obex.h
    
DESCRIPTION
    Interface definition for the OBEX library.

*/
/*!
@file    obex.h
@brief   Interface to the OBEX library.

This interface file is used by File Transfer Profile (FTP), Object Push
Profile (OPP), Phone Book Access Profile (PBAP) and Message Access 
Profile (MAP) Libraries. Current version does not support OBEX 
Authentication.

*/

#ifndef    OBEX_H_
#define    OBEX_H_

#include <library.h>
#include <message.h>
#include <bdaddr_.h>


/*!
    @brief Handle of the OBEX Session.
 */

struct __obex;
typedef struct __obex OBEX, *Obex;

/*! @brief Error Code for Object header utility APIs */
#define  OBEX_INVALID_LEN          0
#define  OBEX_INVALID_CHANNEL      0

/*!
	@brief Size of a MD5 Digest String
*/
#define OBEX_SIZE_DIGEST 16

/*!
    @brief The Obex local status and remote response
*/
typedef enum
{
    obex_success,               /**< General Success */ 
    obex_failure,               /**< General local failure */
    obex_local_unauth,          /**< Client Unauthorized */
    obex_invalid_command,       /**< Command is Invalid in this lib state. */
    obex_invalid_parameters,    /**< Command Parameters are invalid. */
    obex_invalid_response,      /**< An invalid response from remote */
    obex_host_abort,            /**< Remote abort of multi-packet command.*/
    obex_local_abort,           /**< Local abort of multi-packet command.*/
    obex_remote_disconnect,     /**< Remote device initiates disconnection.*/
    obex_transport_failure,     /**< Connection interruption   */
    obex_continue       = 0x90, /**< Continue. Intermediate success */
    obex_remote_success = 0xA0, /**< OK. Current remote operation is success*/
    obex_bad_request    = 0xC0, /**< Server couldn't understand request */
    obex_unauthorized   = 0xC1, /**< Server unauthorized */
    obex_forbidden      = 0xC3, /**< Operation is understood but refused */
    obex_not_found      = 0xC4, /**< Object not found */
    obex_not_acceptable = 0xC6, /**< Parameter value not accepted */
    obex_precon_fail    = 0xCC, /**< Problem with parameter value */
    obex_not_implemented= 0xD1, /**< Not supported */
    obex_srv_unavailable= 0xD3, /**< Service unavailable */
    obex_unknown_error  = 0xE2  /**< Unknown Error code */

} ObexStatus, ObexResponse; 


#ifndef DO_NOT_DOCUMENT

/*
   OBEX Message IDs. Do not document this enum.
*/

/* CL_RFCOMM_CONNECT_IND message for an OBEX channel must be responded
   through OBEX using ObexConnectResponse() for establishing a OBEX session
   along with the RFCOMM Connection. Hence it is mapped directly 
   to OBEX_CONNECT_IND. The application which is using OBEX connections
   as well as independent RFCOMM connections must able to differentiate it 
   using the allocated RFCOMM server channel. */
#define OBEX_CONNECT_IND CL_RFCOMM_CONNECT_IND

/* CL_L2CAP_CONNECT_IND message for an OBEX channel must be responded
   through OBEX using ObexL2capConnectResponse() for establishing a
   OBEX session along with the L2CAP Connection. Hence it is mapped directly 
   to OBEX_L2CAP_CONNECT_IND. The application which is using OBEX connections
   as well as independent L2CAP connections must able to differentiate it 
   using the allocated L2CAP PSM.  */
#define OBEX_L2CAP_CONNECT_IND CL_L2CAP_CONNECT_IND

typedef enum
{
    OBEX_CREATE_SESSION_CFM  = OBEX_MESSAGE_BASE,
    OBEX_CONNECT_CFM,
    OBEX_L2CAP_CONNECT_CFM,
    OBEX_DELETE_SESSION_IND,
    OBEX_PUT_CFM,
    OBEX_GET_CFM,
    OBEX_SET_PATH_CFM,
    OBEX_DELETE_CFM,
    OBEX_PUT_IND,
    OBEX_GET_IND,
    OBEX_SET_PATH_IND,
    OBEX_ABORT_IND,
    OBEX_AUTH_REQ_IND,
    OBEX_AUTH_CLG_IND,
    OBEX_AUTH_RSP_CFM,
    OBEX_MESSAGE_END 
}ObexMessageId;

#endif /* DO_NOT_DOCUMENT */

/*!
    @brief OBEX Byte Sequence and Unicode Headers for framing the OBEX Headers

    Headers like target, who, auth challenge,  auth response are handled 
    internally.
*/
typedef enum
{
    obex_name_hdr   = 0x01,     /**< The Name header */
    obex_desc_hdr   = 0x05,     /**< The description header */
    obex_type_hdr   = 0x42,     /**< The type header */
    obex_time_hdr   = 0x44,     /**< The time header */
    obex_http_hdr   = 0x47,     /**< The HTTP header */
    obex_body_hdr   = 0x48,     /**< The BODY Header */
    obex_ebody_hdr  = 0x49,     /**< The End of Body header */
    obex_app_params = 0x4C,     /**< The Application Params header */
    obex_obj_class  = 0x4F      /**< The Object header */  
} ObexSeqHeaderId;

/*!
    @brief options value used for OBEX Authetication 
*/
typedef enum
{
    obex_auth_no_option = 0x0, /**< Full Access with no user ID */
    obex_auth_with_id = 0x01,  /**< user ID required with full permissions */
    obex_auth_read_only = 0x02,/**< Read only permissions with no user ID */
    obex_auth_read_only_id = 0x03 /**< user ID required and Read only */ 
} ObexAuthOptions;
/*!
    @brief OBEX UINT32 or Byte Headers for framing the OBEX Headers

    Connection ID, SRM and SRMP headers are handled internally.
*/
typedef enum
{
    obex_count_hdr  = 0xC0,     /**< The Count header */
    obex_length_hdr = 0xC3      /**< The Length header */
} ObexByteHeaderId;


/*!
    @brief Flag for folder navigation using setpath
*/
typedef enum
{
    obex_new_folder = 0x00,     /**< Create new folder and Navigate inside */
    obex_folder_root,           /**< Set the folder to root */
    obex_folder_in,             /**< Navigate to the specified folder */
    obex_folder_out             /**< Navigate backwards "../" */
} ObexFolderPath;

/*!
    @brief The OBEX Connection parameters
*/ 
typedef struct
{
    /*! TaskData for the connection context task. The connection context
        task returned by the ObexConnectRequest() and ObexConnectResponse()
        APIs  will have the  same TaskData in it. */
    TaskData        connTaskData;

    /*! Size required for the connection context task. 
    The ObexConnectRequest() and ObexConnectResponse() API creates a 
    connection  context task of this size and returns  
    that task by setting the  connTaskData as its TaskData. */    
    uint16          sizeConnTask;

    /*! The Target header. The application must not free it  during the 
     entire OBEX session */
    const uint8*    target;

    /*!  Size of the target parameter string */
    uint16          sizeTarget; 

    /*! TRUE if OBEX auth is required as server. */
    bool            auth;  

} ObexConnParams;


/*! 
    @brief Indication message for an incoming OBEX connection over RFCOMM.

    The application receives this messages when a remote device is
    attempting to establish a RFCOMM connection with this device. 
    This message is directly mapped to CL_RFCOMM_CONNECT_IND.

    The application must respond to the OBEX_CONNECT_IND using 
    ObexConnectResponse() API.  If any application is using both 
    CL_RFCOMM_CONNECT_IND and OBEX_CONNECT_IND, it must be differentiated 
    using the associated  RFCOMM server channel allocated for the OBEX
    connection. 

    To receive this indication message, the application must register 
    its RFCOMM server channel using ConnectionRfcommAllocateChannel().
*/
typedef struct
{
    bdaddr  addr;           /**< The Bluetooth address of the remote device.*/
    uint8   channel;        /**< The local server channel.*/               
    Sink    sink;           /**< The sink associated with the connection. */

} OBEX_CONNECT_IND_T;



/*! 
    @brief Indication message for an incoming OBEX connection over L2CAP.

    The application receives this messages when a remote device is
    attempting to establish a L2CAP connection with this device. 
    This message is directly mapped to CL_L2CAP_CONNECT_IND.

    The application must respond to the OBEX_L2CAP_CONNECT_IND using 
    ObexL2capConnectResponse() API.  If any application is using both 
    CL_L2CAP_CONNECT_IND and OBEX_CONNECT_IND, it must be differentiated 
    using the associated  L2CAP psm value allocated for the OBEX
    connection. 

    To receive this indication message, the application must register 
    its L2CAP psm value using ConnectionL2capRegisterRequest().
*/
typedef struct
{
    bdaddr       addr;      /**< The Bluetooth address of the remote device.*/
    uint16       psm;       /**< Incoming connection on this L2CAP PSM */ 
    uint8        identifier;/**< channel Identifier to be used in the response */
    uint16       connId;    /**< Connection Identifier for the connection */    
} OBEX_L2CAP_CONNECT_IND_T;

/*!
    @brief Indication message to initiate the Authentication.

    The library uses this message to initiate the authentication for the 
    OBEX connection. As a OBEX client, the library initiates the 
    authentication on receiving an unauthorized response from the remote 
    device for a previous connnection attempt. Whereas OBEX server initiates
    this message if the application had requested to authenticate the 
    incoming connections.
   
    The application must respond with ObexAuthReqResponse() on 
    receiving this message.
*/
typedef struct
{
    Obex        session;   /**< Obex Session Handle */
} OBEX_AUTH_REQ_IND_T;

/*!
    @brief Indication message on receiving Authentication request 

    The library uses this message to notifies the application on receving
    an authentication request from the remote device. This message
    contains the Authentication Challenge header received from the remote
    device. 

    The application must respond with ObexAuthClgResponse() on receiving
    this message.
*/
typedef struct
{
    Obex        session;        /**< Obex Session Handle */
	uint8       nonce[OBEX_SIZE_DIGEST];/**< The Challenge nonce string */
	ObexAuthOptions options;    /**< The options field in the Challenge */
	uint8       sizeRealm;      /**< Length of realm string */
	const uint8* realm;         /**< The realm string */
} OBEX_AUTH_CLG_IND_T;

/*!
    @brief Confirmation message on receiving the Authentication response

    The library returns the remote Authentication Response for the 
    Authentication Challenge sent to the remote side. The application 
    can use the MD5 algorithm to validate the response with the challenge.

    If application failed to authenticate the received response, it must
    disconnect the connection using ObexDisconnectRequest().  
*/
typedef struct
{
    Obex    session;            /**< Obex Session Handle */
	uint8   request[OBEX_SIZE_DIGEST];/**< The request digest string */
	uint16  sizeUserId;               /**< The User ID string length */
	uint8   userId[1];                /**< The User ID string */
} OBEX_AUTH_RSP_CFM_T;

/*!
    @brief Indication message for an incoming OBEX SET_PATH message 

    The application must respond by calling ObexSetPathResponse(). The Source
    contains the folder name in Unicode format.
*/
typedef struct
{
    Obex        session;   /**< Obex Session Handle */
    ObexFolderPath flags;  /**< The direction flag */
    uint16      sourceLen; /**< Size of the source */
    Source      source;    /**< The source for Unicode folder name  */
} OBEX_SET_PATH_IND_T;  

/*!
    @brief Indication message on aborting the ongoing PUT or GET operation

    The application must not respond to this indication message. On receiving
    this indication the application must no longer wait for any continuation
    request for a multi-packet PUT or GET operation.
*/
typedef struct
{
    Obex session;   /**< The Obex Session Handle */
} OBEX_ABORT_IND_T;

/*!
    @brief Indication message for an incoming OBEX PUT request

    On receiving this indication message, the application must call 
    ObexPutResponse() with the response code. If the application 
    wants to add any Object headers, it must add it by  creating 
    new Object using ObexObJNew() and adding the headers using 
    ObexObjAddByteHdr() or ObexObjAddVariableHdr() API.

    The application may use ObexObjMapHdrValue() or ObexObjFilterHdrValue()
    or ObexObjExtractByteHdrValue() API to extract the headers
    from the Source buffer. The application must not access the source 
    buffer after calling ObexPutResponse(). The Source buffer will be
    deleted for receiving the next OBEX request packet after sending the 
    response.

    The application may treat a Source object without any 
    Body/End-Of-Body headers as a Delete request and a Source Object
    with empty End-of-Body header as a Create request.
*/
typedef struct
{
    Obex        session;   /**< Obex Session Handle */
    bool        final;     /**< TRUE if it is the last one is a multi-Put */
    uint16      sourceLen; /**< Size of the source */
    Source      source;    /**< The source Object containing the Headers */
} OBEX_PUT_IND_T;  

/*!
    @brief Indication message for an incoming OBEX GET request

    On receiving this indication message, the application must call 
    ObexGetResponse() with the response code. If the application 
    wants to add any Object headers, it must add it by  creating 
    new Object using ObexObJNew() and adding the headers using 
    ObexObjAddByteHdr() or ObexObjAddVariableHdr() API.

    The application may use ObexObjMapHdrValue() or ObexObjFilterHdrValue()
    or ObexObjExtractByteHdrValue() API to extract the headers
    from the Source buffer. The application must not access the source 
    buffer after calling ObexGetResponse(). The Source buffer will be
    deleted for receiving the next OBEX request packet after sending the 
    response.
*/
typedef struct
{
    Obex        session;   /**< Obex Session Handle */
    bool        final;     /**< TRUE if it is the last one is a multi-Get */
    uint16      sourceLen; /**< Size of the source */
    Source      source;    /**< The source Object containing the Headers */
} OBEX_GET_IND_T;  

/*!
    @brief Confirmation message on establishing the RFCOMM or L2CAP
    Connection.
*/
typedef struct
{
    Obex        session;          /**< Obex Session Handle */
    ObexStatus  status;           /**< Connection Status */
    bdaddr      remoteAddr;       /**< Remote device BDADDR */
    uint16      channel;          /**< Server Channel or PSM */

}OBEX_CREATE_SESSION_CFM_T;

/*!
    @brief Confirmation message on OBEX connection establishment.

    The application must not send any OBEX packets more than the 
    size of MaxPacketLen.
*/
typedef struct
{
    Obex        session;        /**< Obex Session Handle */
    ObexStatus  status;         /**< obex_remote_success on success */
    uint16      maxPacketLen;   /**< Maximum OBEX packet size */

} OBEX_CONNECT_CFM_T, OBEX_L2CAP_CONNECT_CFM_T;

/*!
    @brief  This message is generated as a result of a call to
    ObexPutRequest().

    The application may use ObexObjMapHdrValue() or ObexObjFilterHdrValue()
    or ObexObjExtractByteHdrValue() API to extract the headers
    from the Source buffer. 

    If the status is obex_continue or the last ObexPutrequest() request has 
    the final parameter as FALSE, the application must either call 
    ObexPutRequest() request again to send the rest of the Body header or
    call  ObexAbort() request to abort the PUT operation. In either 
    case, the application must process the source before calling the API.  
*/
typedef struct
{
    Obex        session;   /**< Obex Session Handle */
    ObexStatus  status;    /**< Obex_remote_success on final success */
    uint16      sourceLen; /**< Size of the source */
    Source      source;    /**< The source Object containing the Headers */
} OBEX_PUT_CFM_T;     

/*!
    @brief  This message is generated as a result of a call to
    ObexGetRequest().

    The application may use ObexObjMapHdrValue() or ObexObjFilterHdrValue()
    or ObexObjExtractByteHdrValue() API to extract the headers
    from the Source buffer. 

    If the status is obex_remote_success, the application should call 
    ObexSourceDrop() after processing the source. If the status is 
    obex_continue, the application must either call ObexGet() request to get
    the rest of the data or ObexAbort() to abort the operation. In both 
    cases, the application must process or copy the source before calling 
    the API.
*/
typedef struct
{
    Obex        session;   /**< Obex Session Handle */
    ObexStatus  status;    /**< obex_remote_success on final success */
    uint16      sourceLen; /**< Size of the source */
    Source      source;    /**< The source containing the Headers */
} OBEX_GET_CFM_T;     

/*!
    @brief  This message is generated as a result of a call to
            ObexSetPathRequest().

    The library returns obex_remote_success if the remote setpath operation
    has been successful. 
*/
typedef struct
{
    Obex        session;   /**< Obex Session Handle */ 
    ObexStatus  status;    /**< obex_remote_success on success */
} OBEX_SET_PATH_CFM_T;


/*!
    @brief  This message is generated as a result of a call to
            ObexDeleteRequest().

    The library returns obex_remote_success if the remote delete operation
    has been successful. 
*/
typedef struct
{
    Obex        session;   /**< Obex Session Handle */ 
    ObexStatus  status;    /**< obex_remote_success on success */
} OBEX_DELETE_CFM_T;

/*!
    @brief  Indication message on deleting the OBEX session

    The application must respond to this message by calling 
    ObexDeleteSessionResponse() after completing all operations 
    associated with this session. 
*/
typedef struct 
{
    Obex    session;          /**< The OBEX session handle to be deleted */
    Task    connTask;         /**< The connection context task to be deleted */

} OBEX_DELETE_SESSION_IND_T;


/*!
    @brief Open a OBEX Connection with a server.

    @param addr The Bluetooth address of the remote OBEX Server device.

    @param rfcChannel RFCOMM Server channel to use for the connection.
    The application must set the security requirements for this channel 
    using ConnectionSmSetSecurityLevel() or 
    ConnectionSmRegisterOutgoingService() before calling this API.

    @param connParams  Not NULL parameters required to establish OBEX session.

    @return The connection context task associated with this session.

    The application will receive a @link OBEX_CREATE_SESSION_CFM_T 
    OBEX_CREATE_SESSION_CFM @endlink message followed
    by a @link OBEX_CONNECT_CFM_T OBEX_CONNECT_CFM @endlink message. 
    The OBEX_CREATE_SESSION_CFM on success 
    contains a valid OBEX session handle. The OBEX_CONNECT_CFM contains the 
    final status of the Connection.  
*/
Task ObexConnectRequest( const bdaddr          *addr,
                        uint8                 rfcChannel,
                        const ObexConnParams  *connParams );


/*!
    @brief Open a OBEX Connection with a L2CAP OBEX server.

    @param addr The Bluetooth address of the remote OBEX Server device.

    @param psm  The L2CAP PSM to use for the connection.
    The application must register the PSM using 
    ConnectionL2capRegisterRequest() before calling this API.
    The application can also set the security requirements for this channel 
    using ConnectionSmSetSecurityLevel() or 
    ConnectionSmRegisterOutgoingService().

    @param connParams  parameters required to establish OBEX session. This must
    not be NULL.    

    @return The connection context task associated with this session.

    The application will receive a @link OBEX_CREATE_SESSION_CFM_T 
    OBEX_CREATE_SESSION_CFM @endlink message followed
    by a @link OBEX_L2CAP_CONNECT_CFM_T OBEX_L2CAP_CONNECT_CFM @endlink message. 
    The OBEX_CREATE_SESSION_CFM on success 
    contains a valid OBEX session handle. The OBEX_L2CAP_CONNECT_CFM 
    contains the  final status of the Connection. This API is available
    only with v2 variant of the obex library. 
*/
Task ObexL2capConnectRequest( const bdaddr          *addr,
                              uint16                psm,
                              const ObexConnParams  *connParams );

/*!
    @brief Disconnect a OBEX session

    @param session The OBEX session handle

    The library returns OBEX_DELETE_SESSION_IND message as a result of this
    API call. The application must respond to that indication message by 
    calling ObexDeleteSessionResponse().

    @return Message @link 
    OBEX_DELETE_SESSION_IND_T OBEX_DELETE_SESSION_IND
    @endlink is sent to the application. 



*/
void ObexDisconnectRequest( Obex session );

/*!
    @brief Delete a OBEX session Task

    @param session The OBEX session handle

    The application must call this API on receiving OBEX_DELETE_SESSION_IND.
    After calling this API, the application must not refer to this session
    and the associated  application task indicated in the 
    OBEX_DELETE_SESSION_IND message.
*/ 
void ObexDeleteSessionResponse( Obex session );

/*!
    @brief Send a OBEX PUT request

    @param session The OBEX session handle

    @param final   TRUE for a single PUT or final request in a multi-PUT.

    The application must create an Object using ObexObjNew() and add the 
    necessary headers using ObexObjAddVariableHdr() or ObexObjAddByteHdr()
    one or multiple times to frame the Object to put. Body/End of Body header
    and Name headers are mandatory to add as per the GOEP specification, but
    it may be overridden  by each individual profile (e.g: MAP does not 
    mandate Name header for the PUT request ). 

    In a multiple put operation, the final PUT request without any 
    headers can be send without creating any object.
  
    The library returns OBEX_PUT_CFM with the status. If the status is
    obex_continue, or the previous PUT request was not final, the 
    application must call this API again to send the rest of the PUT packet 
    or call ObexAbort() to abort this PUT operation. The library deletes the 
    associated Object after processing  the API request.

    @return Message @link 
    OBEX_PUT_CFM_T OBEX_PUT_CFM
    @endlink is sent to the application. 

*/
void ObexPutRequest( Obex session, bool final );

/*!
    @brief Send a OBEX SET PATH request

    @param session The OBEX session handle.
    @param flags Folder direction to navigate.
    @param folderLen The length of folder name to navigate.
    @param folderName Name of the folder to be navigated.

    If the flags is set to obex_new_folder or obex_folder_in,
    the application must provide a  valid folderName and folderLen.

    @return Message @link 
    OBEX_SET_PATH_CFM_T OBEX_SET_PATH_CFM
    @endlink is sent to the application. 

*/
void ObexSetPathRequest( Obex session, 
                         ObexFolderPath flags, 
                         uint16 folderLen, 
                         const uint8* folderName);

/*!
	@brief Delete a remote Object.

	@param session OBEX session handle 
    @param objLen Length of the object name.
    @param objName The object name.
	
    @return Message #OBEX_DELETE_CFM_T OBEX_DELETE_CFM  is sent to the
            application.
*/
void ObexDeleteRequest(Obex session, uint16 objLen , const uint8* objName);

/*!
    @brief Send a OBEX GET request

    @param session The OBEX session handle

    @param final   TRUE for a single GET or final request in a multi-GET.

    The application must create an Object using ObexObjNew() and add the 
    necessary headers using ObexObjAddVariableHdr() or ObexObjAddByteHdr()
    one or multiple times to frame the Object for the GET request.
    Adding Name/Type header is mandatory as per the GOEP specification,  
    but it may be overridden  by each individual profile (e.g: MAP does
    not mandate Name header for the GET request ).
    
    In a multiple get operation, once all headers are added to the first 
    GET request, the subsequent GET requests can be send without creating
    an Object.  

    The library returns OBEX_GET_CFM with the status. If the status is
    obex_continue, or the previous GET request was not final , the 
    application must call this API again to get the rest of the packet
    or call ObexAbort() to abort this operation. The library deletes the 
    associated Object after processing  the API request.

    @return Message #OBEX_GET_CFM_T OBEX_GET_CFM is sent to the application.
*/
void ObexGetRequest( Obex session, bool final );

/*!
	@brief Abort the current multi-packet transaction.

	@param session OBEX session handle.

    This application can call this API to abort a multi-packet GET or PUT
    transaction. On success, it returns OBEX_GET_CFM or OBEX_PUT_CFM depends 
    on the ongoing transaction with the status code as obex_local_abort.
    If there is no Put or Get operation in progress, this API will not have
    any effect and will be ignored by the library.
*/
void ObexAbort( Obex session ); 


/*!
    @brief Accept or reject an incoming OBEX Connection

    @param sink The sink received in the OBEX_CONNECT_IND message.
    @param rfcChannel The local RFCOMM server channel on which the 
    connection has been received.

    @param accept   TRUE to accept the connection and FALSE to reject. 

    @param connParams The parameters required to establish OBEX session.
                      NULL for rejecting the connection.

    The application must call this API to accept or reject the connection on 
    receiving @link OBEX_CONNECT_IND_T OBEX_CONNECT_IND@endlink message. 
    The application will receive a @link OBEX_CREATE_SESSION_CFM_T
    OBEX_CREATE_SESSION_CFM@endlink message followed by a @link 
    OBEX_CONNECT_CFM_T OBEX_CONNECT_CFM @endlink message
    on accepting the connection.

    The OBEX_CREATE_SESSION_CFM on success contains a valid OBEX session 
    handle. The OBEX_CONNECT_CFM contains the  final status of the 
    Connection.  

    This API returns a NULL task on rejecting the connection and the application
    will not receive any more messages related to the rejected connection.
*/
Task ObexConnectResponse( Sink					sink,
                          uint8                 rfcChannel,
                          bool                  accept,
                          const ObexConnParams* connParams);   


/*!
    @brief Accept or reject an incoming OBEX Connection over L2CAP.

    @param psm  The L2CAP PSM of the channel.

    @param identifier The channel identifier received in OBEX_L2CAP_CONNECT_IND

    @param connId   The connection identifier received in OBEX_L2CAP_CONNECT_IND

    @param accept   TRUE to accept the connection and FALSE to reject. 

    @param connParams The parameters required to establish OBEX session.
                      NULL for rejecting the connection.

    The application must call this API to accept or reject the connection on 
    receiving @link OBEX_L2CAP_CONNECT_IND_T OBEX_L2CAPCONNECT_IND@endlink
    message.  The application will receive a @link OBEX_CREATE_SESSION_CFM_T
    OBEX_CREATE_SESSION_CFM@endlink message followed by a @link 
    OBEX_L2CAP_CONNECT_CFM_T OBEX_L2CAP_CONNECT_CFM @endlink message
    on accepting the connection.

    The OBEX_CREATE_SESSION_CFM on success contains a valid OBEX session 
    handle. The OBEX_L2CAP_CONNECT_CFM contains the  final status of the 
    Connection.  

    This API returns a NULL task on rejecting the connection and 
    the application  will not receive any more messages related to
    the rejected connection. This API is available only with the v2 version 
    of the OBEX library.
   
*/
Task ObexL2capConnectResponse( uint16                psm,
                               uint8                 identifier,
                               uint16                connId,
                               bool                  accept,
                               const ObexConnParams* connParams ); 



/*
    @brief Send a OBEX PUT response

    @param session The OBEX session handle
    @param response OBEX response. obex_remote_success for success.

    If the application wants to send any Object with this response, it 
    must create an Object using ObexObjNew() and add the 
    necessary headers using ObexObjAdd*Hdr() one or multiple times to 
    frame the Object to put. 

    The application must call this API on receiving @link OBEX_PUT_IND_T
    OBEX_PUT_IND @endlink message.
    if the application expects more data on receiving the final flag on 
    OBEX_PUT_IND as false, the application must send a obex_continue response.
*/
void ObexPutResponse( Obex session, ObexResponse response );

/*!
    @brief Send a OBEX GET response

    @param session The OBEX session handle
    @param response obex_remote_success for final success response.

    If the application wants to send any Object with this response, it 
    must create an Object using ObexObjNew() and add the 
    necessary headers using ObexObjAdd*Hdr() one or multiple times to 
    frame the Object for the GET response. 

    The application must call this API on receiving @link OBEX_GET_IND_T
     OBEX_GET_IND @endlink message.
    The ObexObjNew() returns the size of headers  the application can add 
    in its response. If the application cannot send all data in one response,
    it must send obex_continue_response  and expects next OBEX_GET indication
    message to send the rest.
*/
void ObexGetResponse( Obex session, ObexResponse response );

/*!
    @brief  Send a OBEX SET_PATH response

    @param session The OBEX session handle
    @param response obex_remote_success for success response.
   
    The application must call this API on receiving @link OBEX_SET_PATH_IND_T
    OBEX_SET_PATH_IND @endlink  message.
*/
void ObexSetPathResponse( Obex session, ObexResponse response );

/*!
    @brief Initiate the local OBEX authentication.
     
    @param  session The OBEX session handle

    @param  nonce The 128 bit (16 byte) string produced by the MD5
            hashing algorithm  using time stamp (or any other non-repeating
            value) and a private-key known only to the sender. If the
            application does not want to authenticate the remote, this 
            value must be NULL.

    @param  options The options for access mode.

    @param  realmLen The length of the realm string. Maximum permitted
            value is 20.

    @param  realm A string indication which user id/or password to use. 
            The first byte of this is the character set to use. This is 
            optional and set to NULL if not used. 

    The application must call this API on receiving the @link
    OBEX_AUTH_REQ_IND_T OBEX_AUTH_REQ_IND @endlink
    message. This message is generated as part of OBEX connection 
    establishment procedure if OBEX authentication is requested by the 
    Server.

    The library returns @link OBEX_AUTH_RSP_CFM_T OBEX_AUTH_RSP_CFM 
    @endlink with the request-digest string
    from the remote device for the associated nonce string provided 
    in this API on success, followed by the OBEX_CONNECT_CFM. On failure,
    it just returns OBEX_CONNECT_CFM with failure status.
*/
void ObexAuthReqResponse( Obex session,
                          const uint8* nonce,
                          ObexAuthOptions options,
                          uint16 realmLen,
                          const uint8* realm );

/*!
    @brief Respond to the remote OBEX Authentication Challenge.
     
    @param  session The OBEX session handle

    @param  reqDigest The request digest string produced using the 
            nonce string received in the OBEX_AUTH_CLG_IND and the 
            password using MD5 algorithm. The length of this string 
            must be 128 bits (16 bytes).

    @param  userIdLen The length of userId string. This must not be
            greater than 20.

    @param  userId  The optional user identifier. The maximum allowed size
            is 20. set to NULL if not required. This field is  required 
            only if the options field in the received OBEX_AUTH_CLG_IND 
            was set to 1.   

    The application must call this API on receiving the @link 
    OBEX_AUTH_CLG_IND_T OBEX_AUTH_CLG_IND @endlink
    message. This message is generated as part of the OBEX connection 
    establishment procedure when OBEX authentication is requested by the 
    Server.

    At the end of the connection procedure the library returns 
    @link OBEX_CONNECT_CFM_T OBEX_CONNECT_CFM @endlink with status.
*/
void ObexAuthClgResponse( Obex session,
                          const uint8* reqDigest, 
                          uint8 userIdLen,
                          const uint8* userId ); 

/*! 
    @brief Create a new Object for PUT or GET 

    @param  session The OBEX session handle

    @return Data space for adding OBEX headers from the application

    The application must call this utility API to frame the OBEX Headers for 
    any OBEX PUT/GET request/response with any OBEX headers.
    If the application is calling any OBEX transaction APIs( ObexPutRequest(),
    ObexPutResponse, ObexGetRequest(), ObexGetResponse() )
    without framing the required headers, the library sends out the OBEX 
    packet without adding any headers. 
   
    This function returns 0 on failure or there is no free space for the
    headers. On success, it returns the available space for the headers.
    To send a PUT or GET packet, the application must use the APIs 
    ObexObjAddVarialeHdr() or ObexObjAddByteHdr() to add the required 
    headers followed by the PUT or GET APIs to  send the packet. 

    If the application wants to abort the framing of the packet, it must 
    delete the Object by calling ObexObjDelete( ). If the application 
    wants to use SRM mode, it must call the ObexSrmObjNew() API. 
*/
uint16 ObexObjNew( Obex session );

/*! 
    @brief Create a new Object for PUT or GET with SRM enabled.

    @param  session The OBEX session handle

    @param  obexPut TRUE if the object is being created for PUT operation.

    @param  srmpWait TRUE to set SRMP wait mode.

    @return Data space for adding OBEX headers from the application

    This is same as the ObexObjNew() except, this will set the SRM and 
    SRMP headers in the GET/PUT packet. The srmpWait is valid only
    for a PUT response or a GET request.
*/
uint16 ObexObjSrmNew( Obex session, bool obexPut, bool srmpWait );


/*!
    @brief Delete a OBEX Object
 
    @param session A valid Obex session

    Using the API, the application can drop the Object after creating it but 
    before sending the PUT or GET packet. This can be used if the 
    application wants to reclaim the Object space or the application wants
    to drop the command due to any error.
*/
void ObexObjDelete( Obex session );  


/*!
    @brief Add a Byte Sequence or Unicode text Header to the Object

    @param session A valid Obex session
    @param hdrId   Header ID of the Byte Sequence or Unicode string header
    @param hdrLen  Length of the Byte Sequence or Unicode string header value
    @param src     Header value in byte sequence

    For Unicode string headers ( Name, description), the OBEX library 
    converts the byte sequence value to Unicode null terminated string
    value. If the hdrVal is 0, the library fills filler-byte 0x30 of length 
    hdrLen as the hdrVal.
*/
bool ObexObjAddVariableHdr( Obex session,
                            ObexSeqHeaderId hdrId,
                            uint16  hdrLen,
                            Source  src );

/*!
    @brief Add an unsigned integer header to the OBEX object

    @param session   A valid Obex session
    @param hdrId     Unsigned Header ID
    @param hdrVal    Unsigned integer header value.

    Add a 4 byte or 1 byte unsigned header to the OBEX object.
*/
bool ObexObjAddByteHdr( Obex session, ObexByteHeaderId hdrId, uint32 hdrVal);

/*!
    @brief Map the OBEX header value in the Source buffer

    @param hdrId  Header Id of the Byte Sequence or Unicode string Header
    @param srcLen Length of the source
    @param src  The Source buffer
    @param hdrLen Pointer to RETURNED header length

    The application must copy the returned value for reuse if it is dropping 
    the source buffer or calling any other OBEX transaction APIs.
*/ 
const uint8* ObexObjMapHdrValue( ObexSeqHeaderId hdrId,
                                 uint16 srcLen,
                                 Source src,
                                 uint16* hdrLen );


/*!
    @brief  return the requested header from the Source and drop others.

    @param session OBEX session
    @param hdrId Header Id to be filtered in.
    @param hdrLen Length of the header in the source to be returned. 

    After calling this API, the application must not refer to any previously
    mapped header values from this source buffer.  This API drops all 
    preceding headers of the requested one from the source buffer. 
*/
Source ObexObjFilterHdrValue( Obex session,
                              ObexSeqHeaderId  hdrId,
                              uint16 *hdrLen );

/*!
    @brief Extract 4 byte header value 

    @param hdrId Header Id to be extracted.
    @param srcLen Length of the source
    @param src The Source buffer

    Extract a 4 byte or 1 byte header from the source. On failure it returns 
    0xFFFFFFFF.
*/
uint32 ObexObjExtractByteHdrValue( ObexByteHeaderId hdrId,
                                   uint16 srcLen,
                                   Source src );


/*!
    @brief Drop the received Source buffer

    @param session A valid Obex session

    The application can use this API to drop the source buffer received in
    the GET/PUT IND or CFM messages if the session is going to be idle. 
    Not using this API will not affect any of the OBEX functionality, but 
    it is a best practise to call this  API and free some buffer resources 
    if the session is not going to be active for some time.
*/
void ObexSourceDrop( Obex session );

/*!
    @brief  Get the RFCOMM Sink associated with a OBEX connection.

    @param session A valid Obex session 
*/
Sink ObexGetSink( Obex session );  

/*!
    @brief  Get the RFCOMM server channel associated with the OBEX connection.

    @param session A valid Obex session 

    On success, OBEX returns the remote server channel for the client 
    session and the local server channel for the Server session. On failure
    it returns OBEX_INVALID_CHANNEL.
*/
uint8 ObexGetChannel( Obex session );  

/*!
    @brief  Get the L2CAP PSM associated with the OBEX connection.

    @param session A valid Obex session 

    OBEX returns the L2CAP PSM associated with the Session on success, other
    wise it returns OBEX_INVALID_CHANNEL.
*/

uint16 ObexGetPsm( Obex session );  

#endif /* OBEX_H_ */


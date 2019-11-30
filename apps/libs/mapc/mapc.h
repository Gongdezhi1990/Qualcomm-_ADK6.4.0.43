/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    mapc.h
    
DESCRIPTION 

    Header file for the Message Access Profile (MAP) Client library. This
    profile library implements the protocol model of MAP Messaging Client
    using the APIs of the GOEP library. The application must use this library
    along with the message_parser library to implement the Messaging 
    client use cases.

    The library exposes a functional downstream API and an upstream message 
    based API.
    
*/
/*!
@file    mapc.h
@brief   Interface to the Messaging Client library.

        This profile library implements the protocol model for the Client
        role of Messaging Access Profile (MAP). 
        
        Note: This library does not parse or frame the bMessages and the 
        application must take care of encoding and decoding the bMessages. 
    
        The library exposes a functional downstream API and an upstream
        message based API.

        Supported Message Access Profile Client features are 
        1. Message Notification
        2. Message Browsing ( SMS )
        3. Message Delete
        4. Notification Registration
        5. Message Uploading
*/


#ifndef MAPC_H_
#define MAPC_H_

#include <library.h>
#include <message.h>
#include <bdaddr_.h>


struct __MAPC;
/*!
    @brief The Message Notification Session Handle.
*/
typedef struct __MAPC *Mns;

/*!
    @brief The Message Access Session Handle.
*/
typedef struct __MAPC *Mas;


/*! @brief The Map Status */

typedef enum mapc_status
{
    mapc_success,          /*!< The last operation was successful. */
    mapc_failure,          /*!< General failure */
    mapc_pending,          /*!< The operation is pending or in progress */
    mapc_connect_rejected, /*!< The Connection has been rejected locally */
    mapc_aborted,          /*!< The operation has been aborted locally */
    mapc_invalid_state,    /*!< Command not acceptable in this state */
    mapc_mns_started,      /*!< The MNS Service was already started. */
    mapc_object_not_found, /*!< Remote file or folder not found */
    mapc_object_protected, /*!< Access denied to the remote object */
    mapc_command_rejected, /*!< Remote rejected the command */
    mapc_invalid_parameter /*!< Remote send corrupt or invalid response */

} MapcStatus, MapcResponse;

/*! @brief Status of the message to set */
typedef enum 
{
    mapc_message_read,      /*!< MSE to change the message as old */
    mapc_message_unread,    /*!< MSE to change the message as new */
    mapc_message_deleted,   /*!< MSE to shift the message to delete folder */
    mapc_message_undeleted  /*!< MSE to shift the message to the inbox */

} MapcMessageStatus;

/*! @brief filters for messages listing. Currently it filters 
    only Read status, email message type and an auto filter for 
    parameter mask. One or more values can be bit ORed to use the filter
    value for  MapcMasGetMessagesListingRequest() */
typedef enum
{
    mapc_no_filtering       = 0x00, /*!< No filtering */
    mapc_filter_out_sms_gsm = 0x01, /*!< Filter out GSM SMS */
    mapc_filter_out_sms_cdma= 0x02, /*!< Filter out GSM CDMA */
    mapc_filter_out_email   = 0x04, /*!< Filter out EMAIL */
    mapc_filter_out_mms     = 0x08, /*!< Filter out  MMS */
    mapc_filter_unread      = 0x10, /*!< Get only the unread messages */
    mapc_filter_read        = 0x20, /*!< Get only the read messages */
    mapc_filter_params      = 0x100 /*!< Use an auto filter for ParameterMask*/

} MapcMessageFilter;

#ifndef DO_NOT_DOCUMENT

typedef enum
{
    /* Initialization */
    MAPC_MNS_START_CFM = MAPC_MESSAGE_BASE,
    MAPC_MNS_SHUTDOWN_CFM,
    MAPC_MNS_CONNECT_IND,
    MAPC_MNS_CONNECT_CFM,
    MAPC_MNS_DISCONNECT_IND,
    MAPC_MAS_CONNECT_CFM,
    MAPC_MAS_DISCONNECT_IND,
    MAPC_MAS_SET_NOTIFICATION_CFM,
    MAPC_MNS_SEND_EVENT_IND,
    MAPC_MAS_SET_FOLDER_CFM,
    MAPC_MAS_GET_FOLDER_LISTING_CFM,
    MAPC_MAS_GET_MESSAGES_LISTING_CFM,
    MAPC_MAS_GET_MESSAGE_CFM,
    MAPC_MAS_PUT_MESSAGE_CFM,
    MAPC_MAS_UPDATE_INBOX_CFM,
    MAPC_MAS_SET_MESSAGE_STATUS_CFM,

    MAPC_API_MESSAGE_END /* Start of Internal messages here */

} MapcMessageId;

#endif /* DO_NOT_DOCUMENT */

/*! @brief This message is generated as a result of a call to
    MapcMnsStart() 

    The library returns the registered MNS channel and a valid SDP handle 
    on success. The application must use the same channel and handle while
    calling MapcMnsShutdown() to stop the notification service. 
    The application must set the security requirements
    for this channel using ConnectionSmRegisterIncomingService() before 
    accept the incoming connections. 
*/
typedef struct 
{
    MapcStatus  status;         /*!< The status of the Notification Service */
    uint8       mnsChannel;     /*!< The RFCOMM server channel for MNS */
    uint32      sdpHandle;      /*!< SDP Record Service Handle */

}MAPC_MNS_START_CFM_T; 

/*! @brief This message is generated as a result of a call to
    MapcMnsShutdown()

    The library return mapc_success on successfully stopping the MNS 
    service.
*/
typedef struct
{
    MapcStatus  status;
}MAPC_MNS_SHUTDOWN_CFM_T;

/*! @brief Message indication for an incoming connect request from the MSE

    The application must respond to this message by calling 
    MapcMnsConnectResponse() using the same parameters provided in this 
    message. 
*/
typedef struct
{
    bdaddr  addr;           /*!< Address of the remote MSE device */
    uint8   mnsChannel;     /*!< The local channel received this connection */
    Sink    connectID;      /*!< Connection ID for this Connection */
    
}MAPC_MNS_CONNECT_IND_T;


/*! @brief This message is generated as a result of a call to
    MapcMnsConnectResponse()

    On a successful MNS connection establishment, the library generates
    an intermediate message with mapc_pending status followed by a 
    message with mapc_success status. On connection failure, the library 
    returns this message with mapc_failure.
*/
typedef struct{

    Mns         mnsSession;     /*!< The session handle. Invalid on failure */
    MapcStatus  status;         /*!< The Status of the MAP session. */
    bdaddr      addr;           /*!< BDAddress of the remote device */
    uint8       mnsChannel;     /*!< Channel associated with the session */

}MAPC_MNS_CONNECT_CFM_T;


/*! @brief The message generated on disconnecting the MNS session.

    The application should call MapcMnsDisconnectResponse() any time after 
    disconnection of the session  to remove the session handle associated
    with this  connection.  
*/
typedef struct{
    Mas     mnsSession;        /*!< The session handle of the MNS session */

}MAPC_MNS_DISCONNECT_IND_T;

 
/*! @brief This message is generated as a result of a call to
    MapcMasConnectRequest()

    On a successful MAS connection establishment, the library generates
    an intermediate message with mapc_pending status followed by a 
    message with mapc_success status. On connection failure, the library 
    returns this message with mapc_failure.
*/
typedef struct{
    Mas         masSession;    /*!< The session handle. Invalid on failure */ 
    MapcStatus  status;        /*!< The Status of the MAP session. */
    bdaddr      addr;          /*!< BDAddress of the remote device */
    uint8       masChannel;    /*!< rfcChannel associated with the session */

}MAPC_MAS_CONNECT_CFM_T;


/*! @brief  The message generated on disconnecting the MAS session.

    The application should call MapcMasDisconnectResponse() after disconnection
    to remove the session handle associated with this connection.  
    
*/
typedef struct{
    Mas     masSession;        /*!< The session handle of the MAS session */

}MAPC_MAS_DISCONNECT_IND_T;


/*! @brief This message is generated as a result of a call to
    MapcMasSetNotificationRequest()

    The library returns the status of MapcMasSetNotification() request 
    using this message. On success, it returns mapc_success.
*/
typedef struct{
    Mas     masSession;      /*!< The session handle of the MAS session */ 
    MapcStatus  status;      /*!< Status of the operation */

}MAPC_MAS_SET_NOTIFICATION_CFM_T;


/*! @brief This message is generated as a result of a call to
    MapcMasSetFolderRequest()

   The Mapc lib sent this message as a result of MapcMasSetFolderRequest().
    On success, it returns mapc_success.   
*/
typedef struct{
    Mas         masSession;   /*!< The session handle of the MAS session */ 
    MapcStatus  status;       /*!< Status of the operation */

} MAPC_MAS_SET_FOLDER_CFM_T;


/*! @brief This message is generated as a result of a call to
    MapcMasGetFolderListingRequest().

    The confirmation Message on receiving the message listing from the MSE.
    The application must either call MapcMasGetComplete() to terminate the
    operation or call MapcMasGetContinue() to get the rest of the message
    listing, if the status is mapc_pending.  
*/ 
typedef struct{
    Mas         masSession;     /*!< The session handle of the MAS session */ 
    MapcStatus  status;         /*!< Status of the operation */
    uint16      folderListSize; /*!< Number of available folders. 0xFFFF if  
                                     this information is unavailable. */
    uint16      sourceLen;      /*!< Data length in the source */
    Source      folderListing;  /*!< The folder-listing object */
} MAPC_MAS_GET_FOLDER_LISTING_CFM_T;


/*! @brief This message is generated as a result of a call to
    MapcMasGetMessagesListingRequest().

    The confirmation Message on receiving the messages listing from the MSE.
    The application must either call MapcMasGetComplete() to terminate the
    operation or call MapcMasGetContinue() to get the rest of the message
    listing, if the status is mapc_pending. 
*/ 
typedef struct{
    Mas         masSession;     /*!< The session handle of the MAS session */
    MapcStatus  status;         /*!< Status of the operation */
    uint16      msgListSize;    /*!< Size of available messages */
    uint16      sourceLen;      /*!< Data length in the source */
    Source      messagesListing;/*!< The Messages-Listing Object */ 
} MAPC_MAS_GET_MESSAGES_LISTING_CFM_T;


/*! @brief This message is generated as a result of a call to
    MapcMasGetMessageRequest().

    The confirmation Message on receiving the message from the MSE.
    The application must either call MapcMasGetComplete() to terminate the
    operation or call MapcMasGetContinue() to get the rest of the message,
    if the status is mapc_pending. 
*/ 
typedef struct{
    Mas         masSession;     /*!< The session handle of the MAS session */
    MapcStatus  status;         /*!< Status of the operation */
    uint16      sourceLen;      /*!< Data length in the source */
    Source      bMessage;       /*!< The bMessage Object */ 
} MAPC_MAS_GET_MESSAGE_CFM_T;


/*! @brief This message is generated as a result of a call to
    MapcMasPutMessageRequest().

    The confirmation Message for MapcMasPutMessageRequest() and 
    MapcMasPutContinue() operations. The application must call 
    MapcMasPutContinue() if there is more data to push (i.e the received
    status is mapc_pending ). The final success confirmation message will
    contain a handle of maximum size of 16 hex digits assigned to the message
    by the MSE. Each hex digit is represented in one byte. 
*/ 
typedef struct{
    Mas         masSession;  /*!< The session handle of the MAS session */
    MapcStatus  status;      /*!< Status of the operation */
    uint16      handleLen;   /*!< The length of the handle */
    uint8       handle[1];   /*!< handle of length handleLen */
} MAPC_MAS_PUT_MESSAGE_CFM_T; 


/*! @brief This message is generated as a result of a call to
    MapcMasUpdateInboxRequest()

    The library returns the status of MapcMasUpdateInbox() request 
    using this message. On success, it returns mapc_success.
*/
typedef struct{
    Mas     masSession;      /*!< The session handle of the MAS session */ 
    MapcStatus  status;      /*!< Status of the operation */

}MAPC_MAS_UPDATE_INBOX_CFM_T;


/*! @brief This message is generated as a result of a call to
    MapcMasSetMessageStatusRequest()

    The library returns the status of MapcMasSetMessageStatusRequest() 
    using this message. On success, it returns mapc_success.
*/
typedef struct{
    Mas     masSession;      /*!< The session handle of the MAS session */ 
    MapcStatus  status;      /*!< Status of the operation */
}MAPC_MAS_SET_MESSAGE_STATUS_CFM_T;

/*!
    @brief The message generated for new message notifications.

    The Mapc library notifies the application on receiving SendEvent_Req 
    from the remote device. A single SendEvent_Req may be notified using 
    multiple indications depends on the number of OBEX put packets. Each IND
    message must be acknowledged using MapcMnsSendEventResponse() by the 
    application.
*/
typedef struct{
    Mns     mnsSession;     /*!< The MNS session handle */
    uint8   masInstanceId;  /*!< Remote MAS instance */
    bool    moreData;       /*!< TRUE if there is more data to come */
    uint16  sourceLen;      /*!< Data length in the source */
    Source  eventReport;    /*!< Source containing the event Report Object */
}MAPC_MNS_SEND_EVENT_IND_T;


/*!
    @brief Start the Message Notification Service.

    @param theAppTask  The current application task. The library notifies
    this task on receiving any incoming MNS connection request from the 
    MSE device.

    @param popSdpRec  Set this flag to TRUE to register a default SDP
    record using a default RFCOMM server channel. Otherwise the library
    expects the application to register the SDP record after calling this
    API.

    @param mnsChannel  RFCOMM Server channel to be allocated for MNS.
    If popSdpRec is set to TRUE, the library ignores this value and 
    registers a default MAP Server channel allocated by the MAP library.
    Otherwise, this must be matching with the RFCOMM Server channel in 
    the application SDP record.

    The application can start only one Message Notification Service. For
    better utilization of resources, the application must turn ON the
    Message Notification Service just before turning on the Register
    Notification by calling MapMnsSetRegisterNotification().
    The expected return status values are mapc_success on successfully
    starting the service, mapc_mns_started if the service is already
    running and mapc_failure if the library is unable to start the
    notification service.

    @return Message @link MAPC_MNS_START_CFM_T MAPC_MNS_START_CFM @endlink
    is sent to the application.
*/
void MapcMnsStart( Task theAppTask, bool popSdpRec, uint8 mnsChannel );

/*!
    @brief  Shutdown the MNS Service

    @param recordHandle - The SDP record handle of the MNS Service.

    @param mnsChannel  - The MNS Server channel. 

    The application can use this API to stop the MNS service. This API
    will not disconnect the existing MNS connections, but it will not 
    accept any new MNS connections after calling this API. This API
    unregister the SDP record and deallocate the MNS server channel.
*/
void MapcMnsShutdown( uint32 recordHandle, uint8 mnsChannel );

/*!
    @brief Confirm the deletion of a MNS session.

    @param mnsSession  The session handle of the MNS session to be deleted.

    The application must call this API on receiving @link
    MAPC_MNS_DISCONNECT_IND_T MAPC_MNS_DISCONNECT_IND@endlink
    to confirm the disconnection and release of session resources.
*/
void MapcMnsDisconnectResponse( Mns mnsSession );

/*!
    @brief Accept or reject the incoming MNS Connection.

    @param theAppTask  The application task. The MAPC library returns
    one or more MAPC_MNS_CONNECT_CFM Messages to this application task as a 
    result to this API request.

    @param addr  The Bluetooth device address of the remote MSE device.

    @param mnsChannel  The MNS RFCOMM Channel provided in the 
    MAPC_MNS_CONNECT_IND message.

    @param accept  TRUE to accept the connection.

    @param connectID  The Connection ID (Sink) provided in the 
    MAPC_MNS_CONNECT_IND message.

    The application must call this API on receiving the message
    @link MAPC_MNS_CONNECT_IND_T MAPC_MNS_CONNECT_IND@endlink from the 
    library for incoming connections. The 
    library returns one or more @link MAPC_MNS_CONNECT_CFM_T 
    MAPC_MNS_CONNECT_CFM @endlink as a result of this
    API Call. After establishing a session, the library notifies 
    @link MAPC_MNS_CONNECT_CFM_T 
    MAPC_MNS_CONNECT_CFM @endlink to the application with  mapc_pending.
    After a successful MNS Connection, it notifies @link 
    MAPC_MNS_CONNECT_CFM_T  MAPC_MNS_CONNECT_CFM @endlink 
    with mapc_success.
    
*/ 
void MapcMnsConnectResponse( Task theAppTask, 
                             const bdaddr* addr,
                             uint8  mnsChannel,
                             bool   accept,
                             Sink   connectID );


/*!
    @brief Initiate an SDP search to the remote MSE device.

    @param theAppTask  The application task. The Connection library returns 
    CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM to this task after processing 
    this API.

    @param addr  The Bluetooth device address of the remote device.

    This is an utility API to frame a Service Attribute Search Request 
    using ConnectionSdpServiceSearchAttributeRequest() to initiate an SDP
    Service Search Attribute request to a peer MSE device to fetch the 
    Service Name , Protocol Descriptor list, MASInstanceID and supported
    Message Types of all available MAS Service records at the MSE device. 

    @return Message @link CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T 
    CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM@endlink 
    is sent to the application  with the attributes. 
    The application can use sdp_parse library to extract the values. 
   
    Refer definition of  CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T 
    in connection.h for more details. 
 
*/
void MapcMasSdpAttrSearchRequest( Task theAppTask, const bdaddr* addr );

/*!
    @brief Establish one or more Message Access Service (MAS) sessions.

    @param theAppTask  The application task. The MAPC library returns
    one or more MAPC_MAS_CONNECT_CFM Messages to this application task as a 
    result to this API request.

    @param addr  The Bluetooth device address of the remote MSE device.

    @param masChannel  The MAS RFCOMM Channel. The application can extract
    a valid remote MAS channel from the result of MapcMasSdpAttrSearchRequest().
    The application must set the security requirements for this channel 
    using ConnectionSmSetSecurityLevel() or 
    ConnectionSmRegisterOutgoingService() before calling this API.

    The library returns one or more @link MAPC_MAS_CONNECT_CFM_T
    MAPC_MAS_CONNECT_CFM @endlink as a result of this
    API Call. After establishing a session, the library notifies 
    @link MAPC_MAS_CONNECT_CFM_T MAPC_MAS_CONNECT_CFM @endlink 
    to  the application with  mapc_pending for 
    each session in progress. After a successful MAS Connection, it notifies
     @link MAPC_MAS_CONNECT_CFM_T MAPC_MAS_CONNECT_CFM @endlink 
    with mapc_success.
*/
void MapcMasConnectRequest( Task theAppTask, const bdaddr* addr, uint8 masChannel ); 

/*!
    @brief Disconnect the MAS session.

    @param masSession  The MAS session handle.

    This API initiates the MAP session disconnection.

    The library returns @link MAPC_MAS_DISCONNECT_IND_T MAPC_MAS_DISCONNECT_IND
    @endlink message on disconnecting the 
    MAP connection. The application must respond that message using 
    MapcMasDisconnectResponse() to free the resources. 
*/

void MapcMasDisconnectRequest( Mas masSession );

/*!
    @brief Confirm the deletion of a MAS session.

    @param masSession  The session handle of the MAS session to be deleted.

    The application must call this API after receiving the message @link
    MAPC_MAS_DISCONNECT_IND_T MAPC_MAS_DISCONNECT_IND @endlink to confirm 
    the disconnection and release of session resources.
   
*/
void MapcMasDisconnectResponse( Mas masSession );

/*!
    @brief Register or Unregister the message notifications. 

    @param masSession  The session handle of the MAS session.

    @param regStatus   TRUE to register and FALSE to unregister.

    This API allows the application to set the Notification mode on the
    MSE side. The application must set the device to connectable mode 
    and start the MNS service by calling MapcMnsStart() before turning
    ON the notification. The application may shutdown the MNS service
    by calling MapcMnsShutdown() if the Notification mode is turned OFF
    on all connected MAS sessions or no MAS connections are available. 

    @return Message @link 
    MAPC_MAS_SET_NOTIFICATION_CFM_T MAPC_MAS_SET_NOTIFICATION_CFM
    @endlink is sent to the application. 
*/
void MapcMasSetNotificationRequest( Mas masSession, bool regStatus );

/*!
    @brief Acknowledge on receiving an event report Indication.

    @param mnsSession  The session handle of the MNS session
    
    @param response  mapc_success on receiving a full valid event report.

    This function is called on receipt of a message 
    @link MAPC_MNS_SEND_EVENT_IND_T MAPC_MNS_SEND_EVENT_IND @endlink.
    
    The application shall not 
    access the source buffer received in the indication after calling this 
    API. If the application has received the partial event report and 
    expecting more, it must copy the partial data and call this API 
    using mapc_pending response. 
*/
void MapcMnsSendEventResponse( Mns mnsSession, MapcResponse response );


/*!
    @brief Request to navigate the folders of MSE

    @param masSession  The MAS session handle

    @param back  TRUE to go backwards before applying the name if exists. 

    @param name  The name of the folder. 

    @param len  The length on the name string.

   The API function to navigate the folders of the MSE. 

    back=TRUE and name=NULL is equivalent to "cd .." or navigate one level up,
    back=FALSE and name=NULL is equivalent to "cd ~" or navigate to root. 

    @return Message @link 
    MAPC_MAS_SET_FOLDER_CFM_T MAPC_MAS_SET_FOLDER_CFM
    @endlink is sent to the application. 
*/
void MapcMasSetFolderRequest( Mas masSession, 
                              bool back, 
                              const uint8* name, 
                              uint16 len );
/*!
    @brief Request to get the remote folder listing

    @param masSession  The MAS session handle

    @param maxListCount - The maximum number of folders to be listed. 

    @param listStartOffset - Offset of the first entry to be listed 

    The API function is to retrieve the folder-listing object from the 
    current folder of the MSE.The maxListCount value must not be zero.

    @return Message @link 
    MAPC_MAS_GET_FOLDER_LISTING_CFM_T MAPC_MAS_GET_FOLDER_LISTING_CFM
    @endlink is sent to the application. 
*/
void MapcMasGetFolderListingRequest( Mas masSession, 
                                     uint16 maxListCount, 
                                     uint16 listStartOffset);

/*!
    @brief Request to get the list of messages from the MSE folder

    @param masSession  The MAS session handle

    @param name Name of the child folder or Null for the current folder.

    @param nameLen length of name

    @param maxListCount - The maximum number of messages to be listed. 

    @param listStartOffset - Offset of the first entry to be listed 

    @param filter - Message filter. If there are more filters to apply,
                    bitwise OR 2 or more values of MapcMessageFilter and use.

    This API function is to retrieve the Messages-listing object from the 
    current folder of the MSE. The maxListCount value must not be zero.

    @return Message @link 
    MAPC_MAS_GET_MESSAGES_LISTING_CFM_T MAPC_MAS_GET_MESSAGES_LISTING_CFM
    @endlink is sent to the application. 
*/
void MapcMasGetMessagesListingRequest( Mas masSession, 
                                       const uint8* name,
                                       uint16 nameLen,
                                       uint16 maxListCount, 
                                       uint16 listStartOffset,
                                       MapcMessageFilter filter );

/*!
    @brief Request to retrieve the message

    @param  masSession The MAS session handle

    @param  handle The 16 hex digit message handle in byte sequence. 

    @param  native TRUE for Native and False for UTF-8 message.

    This API function is to retrieve the message from the current folder
    of the MSE. The size of the handle must be 16 character length, 
    otherwise the library panics.

    @return Message @link 
    MAPC_MAS_GET_MESSAGE_CFM_T MAPC_MAS_GET_MESSAGE_CFM
    @endlink is sent to the application. 
*/
void MapcMasGetMessageRequest( Mas masSession,
                               const uint8* handle,
                               bool native );

/*!
    @brief  Request to continue the Get operations

    @param  masSession The MAS session handle

    The application must use this API function to continue the get operation
    for folder listing or message listing or message if its corresponding 
    _CFM message returns with mapc_pending.
*/
void MapcMasGetContinue( Mas masSession );

/*!
    @brief  Request to Abort the Get operation and free up the get buffer.

    @param  masSession The MAS session handle

    The application must use this API function to abort the get operation
    for folder listing or message listing or message and free up the 
    used resources.
*/
void MapcMasGetComplete( Mas masSession );

/*!
    @brief Request to update the MSE Inbox

    @param  masSession The MAS session handle

    This API function is to request the MSE for updating the INBOX for
    new messages from the network. 

    @return Message @link 
    MAPC_MAS_UPDATE_INBOX_CFM_T MAPC_MAS_UPDATE_INBOX_CFM
    @endlink is sent to the application. 
*/
void MapcMasUpdateInboxRequest( Mas masSession );

/*!
    @brief Request to set the message status

    @param  masSession The MAS session handle

    @param  handle The 8 byte message handle in byte sequence 

    @param  status Message status to be set.

    This API function is to set the status of the message to delete, read 
    or unread. 

    @return Message @link 
    MAPC_MAS_SET_MESSAGE_STATUS_CFM_T MAPC_MAS_SET_MESSAGE_STAUS_CFM
    @endlink is sent to the application. 
*/
void MapcMasSetMessageStatusRequest( Mas masSession,
                                     const uint8* handle,
                                     MapcMessageStatus status );

/*!
    @brief Request to upload a new message.

    @param  masSession The MAS session handle

    @param  name Name of the message folder

    @param  nameLen Folder name length

    @param  native TRUE for Native and False for UTF-8 message.

    @param  moreData TRUE if the message is partial and more data 
            to send later.

    @param  message The partial or complete message.

    The application must call MapcMasPutContinue() if the  
    return status is mapc_pending.

    @return Message @link 
    MAPC_MAS_PUT_MESSAGE_CFM_T MAPC_MAS_PUT_MESSAGE_CFM
    @endlink is sent to the application. 

   
*/
void MapcMasPutMessageRequest( Mas masSession, 
                               const uint8* name,
                               uint16 nameLen,
                               bool native,
                               bool moreData,
                               Source message );
/*!
    @brief Continue the partial message upload

    @param  masSession The MAS session handle

    @param  moreData TRUE if the message is partial and more data to send.

    @param  message The partial or complete message.

    This function is called on receipt of a message 
    @link MAPC_MAS_PUT_MESSAGE_CFM_T  MAPC_MAS_PUT_MESSAGE_CFM@endlink
    with status as mapc_pending.

*/
void MapcMasPutContinue( Mas masSession,
                         bool moreData,
                         Source message );

#endif /* MAPC_H_ */



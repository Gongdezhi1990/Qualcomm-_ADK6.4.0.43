/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc.h
    
DESCRIPTION
    Interface definition for the PhoneBook Access Profile Client Library.

*/
/*!
@file pbapc.h
@brief PhoneBook Access Profile Client Library.


Library Dependecies : connection, region, service, obex, bdaddr
        
Library variants:-
        pbapc - pbapc with no debug
        pbapc_debug - pbapc with debug checks
        pbapc_debug_print - pbapc with debug checks and print output
*/

#ifndef    PBAP_CLIENT_H_
#define    PBAP_CLIENT_H_

#include <library.h>
#include <message.h>
#include <bdaddr_.h>

/*!
    @brief Size of a MD5 Digest String
*/
#define PBAPC_OBEX_SIZE_DIGEST  16
#define MAX_PBAPC_CONNECTIONS    2 
/*!
    @brief PBAP Client status. 
*/
typedef enum
{
    /*! Last operation was successful. */
    pbapc_success,                    
    /*! Last operation failed. */
    pbapc_failure,        
    /*! Last operation was aborted. */
    pbapc_aborted,
    /*! The last operation is pending or is in progress */
    pbapc_pending,
    /*! Client is not idle, so cannot perform the current operation. */
    pbapc_not_idle,
    /*! Operation failed due to being in the wrong state.*/
    pbapc_wrong_state,
    /*! Unable to register the SDP record due to a lack of resources */
    pbapc_sdp_failure_resource,
    /*! Unable to register the SDP record due to Bluestack */
    pbapc_sdp_failure_bluestack,
    /*! Remote host has disconnected or the link has been lost. */
    pbapc_remote_disconnect,
    
    /*! Not authorised to access this phonebook */
    pbapc_spb_unauthorised = 0x10,
    /*! The server does not contain this repository */
    pbapc_spb_no_repository,
    /*! Phonebook does not exist */
    pbapc_spb_not_found,
    
    /*! No resources to generate application specific parameters header 
        for PullvCardList. */
    pbapc_vcl_no_param_resources = 0x20,
    /*! A phonebook folder was specified for PullvCardList where there 
        are no sub-folders (i.e. in pb). */
    pbapc_vcl_no_pbook_folder,
    /*! A phonebook folder was specified for PullvCardList which is invalid */
    pbapc_vcl_invalid_pbook,
    
    /*! No resources to generate application specific parameters header for
        PullvCardEntry. */
    pbapc_vce_no_param_resources = 0x30,
    /*! No resources to generate the vCard entry name for PullvCardEntry. */
    pbapc_vce_no_name_resources,
    /*! Invalid entry for this phonebook for PullvCardEntry.  Only folder
        'pb' can contain an entry 0. */
    pbapc_vce_invalid_entry,
    
    /*! No resources to generate application specific parameters header 
        for PullPhonebook. */
    pbapc_ppb_no_param_resources = 0x40,
    /*! No resources to generate the phonebook name for PullPhonebook. */
    pbapc_ppb_no_name_resources,
    /*! No name for PullPhonebook when it is required.  
        e.g. server is not in a phonebook directory */
    pbapc_ppb_no_required_name,
    /*! The server does not contain this repository */
    pbapc_ppb_no_repository,
    
    /*! Request to get the server properties failed due to an SDP error */
    pbapc_prop_sdp_error,

#ifndef DO_NOT_DOCUMENT
    pbapc_end_of_status_list
#endif
} PbapcLibStatus, pbapc_lib_status;

/*!
    @brief options value used for OBEX Authetication 
*/
typedef enum
{
    /*! Full Access with no user ID */
    pbapc_obex_auth_no_option = 0x0,

    /*! user ID required with full permissions */
    pbapc_obex_auth_with_id = 0x01,

    /*! Read only permissions with no user ID */
    pbapc_obex_auth_read_only = 0x02,

    /*! user ID required and Read only */ 
    pbapc_obex_auth_read_only_id = 0x03
} PbapcObexAuthOptions;

/*!
    @brief Order values for use with the PullvCardListing function.
*/
typedef enum 
{
    /*! Indexed. */
    pbap_order_idx = 0x00,
    /*! Alphanumeric */
    pbap_order_alpha = 0x01, 
    /*! Phonetic */
    pbap_order_phone = 0x02

}PbapcOrderValues, pbap_order_values;

/*!
    @brief Search Attributes to use with the PullvCardListing function.
*/
typedef enum 
{
    /*! Name. */
    pbap_search_name = 0x00,
    /*! Number */
    pbap_search_number = 0x01, 
    /*! Sound */
    pbap_search_sound = 0x02

} PbapcSearchValues, pbap_search_values;

/* Pbap link priority is used to identify different pbapc links to
   AG devices using the order in which the devices were connected. 
*/
typedef enum
{
    /*! First pbapc link */
    pbapc_primary_link,
    /*! Second pbapc link */
    pbapc_secondary_link, 
    /*! Invalid pbapc link */
    pbapc_invalid_link
    
} pbapc_link_priority;

/*! 
    @brief Handle of the current PBAP Client Session.
   
    For client-side session management, this value can be used in comparisons,
    but it's meaning cannot be relied upon.
 */
struct __pbapcState;
typedef struct __pbapcState PBAPC; 

/*
    Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
    /* Session Control */
    PBAPC_INIT_CFM = PBAPC_MESSAGE_BASE,
    
    PBAPC_CONNECT_CFM,
    PBAPC_AUTH_REQUEST_IND,
    PBAPC_AUTH_RESPONSE_CFM,
    PBAPC_DISCONNECT_IND,

    /* PhoneBook Download */ 
    PBAPC_PULL_PHONEBOOK_CFM,
    PBAPC_SET_PHONEBOOK_CFM,
    PBAPC_PULL_VCARD_LISTING_CFM,
    PBAPC_PULL_VCARD_ENTRY_CFM,
    
    PBAPC_MESSAGE_TOP
} PbapcMessageId;

#endif

/*!
    @brief Valid phonebook repositories.
*/
typedef enum 
{
    pbap_current,
    pbap_local, 
    pbap_sim1, 
    pbap_any /* local or/and SIM1 */
} PbapcPhoneRepository, pbap_phone_repository;

/*!
    @brief Valid phonebook folders. 
*/
typedef enum
{
    pbap_telecom, 
    pbap_pb, 
    pbap_ich, 
    pbap_och, 
    pbap_mch, 
    pbap_cch,
    pbap_root
} PbapcPhoneBook, pbap_phone_book;

#ifndef DO_NOT_DOCUMENT
#define pbap_b_unknown pbap_root 
#define pbap_r_unknown pbap_any
#endif /* DO_NOT_DOCUMENT */

/*!
    @brief vCard formats to use with the PullvCardEntry and 
    PullPhonebook functions.
*/
typedef enum 
{
    /*! vCard 2.1. */
    pbap_format_21 = 0x00,
    /*! vCard 3.0 */
    pbap_format_30 = 0x01,
    /*! Use default value */
    pbap_format_def
} PbapcFormatValues, pbap_format_values;

/*!
    @brief The 64 bit Filter Attribute Mask values
*/
typedef struct{
    /*! First 0-28 bits for fixed attributes mask 29-31 are reserved */
    uint32  filterLow;

    /*! bit 0-6 are reserverd bit 7 to enable proprietary bits and
        bits 8-31 contains the proprietary attributes */
    uint32  filterHigh;
         
}PbapcPhoneBookFilter;

/*!
    @brief The application parameters for Phone book Pull operation
*/
typedef struct{
    /*! The filter attribute mask of 64 bits. set to 0 for not using this.*/
    PbapcPhoneBookFilter  filter;

    /*! Requested phonebook format */
    PbapcFormatValues     format;

    /*! Maximum number of entries requested */
    uint16                maxList;
    
    /*! Start offset for the list */
    uint16                listStart;

}PbapcPullPhonebookParams;

/*!
    @brief The application parameters for pull vCard listing function.
*/
typedef struct{
    /*! Result sort order. */
    PbapcOrderValues    order;
    
    /*! Search attribute to search */
    PbapcSearchValues   srchAttr;

    /*! Search String */
    const uint8*        srchVal;

    /*! Search string length. Search string must be less than 25 chars */
    uint8              srchValLen;

    /*! Maximum number of entries requested */
    uint16                maxList;
    
    /*! Start offset for the list */
    uint16                listStart;
   
}PbapcPullvCardListParams;

/*!
    @brief The application parameters for pull vCard entry function.
*/
typedef struct
{
    /*! The filter attribute mask of 64 bits. set to 0 for not using this.*/
    PbapcPhoneBookFilter  filter;

    /*! Requested phonebook format */
    PbapcFormatValues     format;

}PbapcPullvCardEntryParams;

/*!
    @brief This message is generated as a result of a call to
    PbapcInit().
*/
typedef struct
{
    /*! The status of the PbapInit attempt. */
    pbapc_lib_status status;

    /*! The sdp Handle valid on success */
    uint32  sdpHandle;

} PBAPC_INIT_CFM_T;

/*!
    
    @brief This message is generated as a result of a call to
    PbapcConnectRequest().
*/
typedef struct
{
    /*! PBAPC Session Handle.*/
    uint16     device_id;
    
    /*! BDADDR of the remote device */
    bdaddr     bdAddr;

    /*! The current status of the PBAPC library. */
    pbapc_lib_status status;         

    /*! Maximum size of packet transferable during this session. Equals
      min(client_packet_size , server_packet_size). */
    uint16          packetSize;        

    /*! Supported repositories.  */
    PbapcPhoneRepository   repositories;

} PBAPC_CONNECT_CFM_T;

/*!
    @brief This message is received when the remote server requests
     authentication during connection.
*/
typedef struct
{
    /*! PBAPC Session Handle.*/
    uint16     device_id;     
    
    /*! The Challenge nonce string */ 
    uint8 nonce[PBAPC_OBEX_SIZE_DIGEST];

    /*! The options field in the challenge */
    PbapcObexAuthOptions options;
    
    /*! Length of realm string */
    uint16 sizeRealm;

    /*! The realm string */
    const uint8* realm;

} PBAPC_AUTH_REQUEST_IND_T;

/*!
    @brief This message is received when the remote server sends the 
     authentication response during the connection.
*/
typedef struct
{
    /*! PBAPC Session Handle.*/
    uint16     device_id;       
    
    /*! The digest response string */ 
    uint8 digest[PBAPC_OBEX_SIZE_DIGEST];

} PBAPC_AUTH_RESPONSE_CFM_T;


/*!
    
    @brief This message is generated as a result of a call to
    PbapcPullPhonebookRequest().

    The application must either call PbapcPullComplete() to
    terminate the operation or call PbapcPullContinue() to get more data
    if the status is pbapc_pending.
*/
typedef struct
{
    /*! PBAPC Session Handle.*/
    uint16     device_id;

    /*! Status of the Pull operation */
    pbapc_lib_status status;

    /*! Size of the phonebook if the request param maxList was 0. */
    uint16    pbookSize;

    /*! New Missed Calls valid only for mch phone book object. */
    uint8    newMissCalls;

    /*! Source containing the phone book data. */
    Source  src;

    /*! Data length in the source */
    uint16  dataLen;
} PBAPC_PULL_PHONEBOOK_CFM_T;


/*!
    @brief This message is generated as a result of a call to
    PbapcSetPhonebookRequest().
*/
typedef struct
{
    /*! PBAPC Session Handle.*/
    uint16     device_id;

    /*! The current status of the PBAPC library. */
    pbapc_lib_status status;

    /*! The current server repository */
    PbapcPhoneRepository repository;

    /*! The current server phonebook */
    PbapcPhoneBook  phonebook;

} PBAPC_SET_PHONEBOOK_CFM_T;


/*!
    
    @brief This message is generated as a result of a call to
    PbapcPullVcardListingRequest()

    attempt. The application must either call PbapcPullComplete() to
    terminate the operation or call PbapcPullContinue() to get more data
    if the status is pbapc_pending.
*/
typedef struct
{
    /*! PBAPC Session Handle.*/
    uint16     device_id;

    /*! Status of the Pull operation */
    pbapc_lib_status status;

    /*! Size of the phonebook if the request param maxList was 0. */
    uint16    pbookSize;

    /*! New Missed Calls valid only for mch phone book object. */
    uint8    newMissCalls;

    /*! Source containing the phone book data. */
    Source  src;

    /*! Data length in the source */
    uint16  dataLen;
} PBAPC_PULL_VCARD_LISTING_CFM_T;

/*!

    @brief This message is generated as a result of a call to
    PbapcPullVcardEntryRequest().

    The application must either call PbapcPullComplete() to
    terminate the operation or call PbapcPullContinue() to get more data
    if the status is pbapc_pending.
*/
typedef struct
{
    /*! PBAPC Session Handle.*/
    uint16     device_id;

    /*! Status of the Pull operation */
    pbapc_lib_status status;

    /*! Source containing the vCard data. */
    Source  src;

    /*! Data length in the source */
    uint16  dataLen;
} PBAPC_PULL_VCARD_ENTRY_CFM_T;


/*!

    @brief This message is generated as a result of a call to
    PbapcDisconnectRequest().

    When the application receives this message the PBAPC session has already
    been released and shall not be used anymore. To create a new session
    call to PbapcConnectRequest() is required.

*/
typedef struct
{
    /*! PBAPC Session Handle.*/
    uint16     device_id;

    /*! The current status of the PBAPC library. */
    pbapc_lib_status status;
} PBAPC_DISCONNECT_IND_T;


/*   Downstream API for the PBAP Client Library   */

/*!
    @brief Initialise the PBAP Client Library.

    @param theAppTask The current application task.

    This API registers the SDP record for the PCE device. The application
    must call  this API only once during initialization of the PCE device.

    @return Message @link PBAPC_INIT_CFM_T PBAPC_INIT_CFM@endlink is 
    sent to the application with the result of the initialization attempt.
*/
void PbapcInit( Task theAppTask );

        
/*!
    @brief Open an PBAPC Connection with a server.

    @param theAppTask The current application task.
    @param *bdAddr The Bluetooth address of the device being replied to.
    
    This will make a Bluetooth connection and establish an Obex 
    session with the server.
    
    @return Message #PBAPC_CONNECT_CFM_T PBAPC_CONNECT_CFM is sent to the
            application.
*/
void PbapcConnectRequest( Task theAppTask, const bdaddr *bdAddr );

/*!
    @brief et the device_id of PBAPC link connected to bdAddr

    @param *bdAddr The Bluetooth address of the device.
    
    @return device id of the link
*/
uint16 PbapcGetLinkFrmAddr(const bdaddr *bdAddr);

/*
    @brief Respond to an authentication challenge during connect.

    @param device_id PBAPC Session Handle.
    @param digest MD5 Digest response. Must be a 16byte string.
    @param sizeUserId Length of the optional User Id string.
    @param userId optional User Id string.
    @param nonce 16 byte MD5 Digest Nonce to be sent in the Challenge. This
           value can be NULL if remote authentication is not required.

    This function is to respond to the OBEX authentication request from the 
    server on receving the message  @link PBAPC_AUTH_REQUEST_IND 
    PBAPC_AUTH_REQUEST_IND_T@endlink  . The userId must be present if
    the options field in the received message
    has requested for user id (i.e pbapc_obex_auth_with_id or 
    obex_auth_read_only_id ).

    @return Message @link PBAPC_AUTH_RESPONSE_CFM_T PBAPC_AUTH_RESPONSE_CFM
    @endlink is sent to the application.
         
*/
void PbapcConnectAuthResponse( uint16 device_id,
                               const uint8 *digest, 
                               uint16 sizeUserId, 
                               const uint8 *userId, 
                               const uint8 *nonce);

/*!
    @brief Download the phonebook using PullPhonebook Function

    @param device_id PBAPC Session Handle.
    @param repository Repository containing the phonebook to PULL
    @param phonebook Phonebook to PULL
    @param params The application parameters for phonebook PULL operation. 

    This function is to initiate the PullPhoneBook function. 
    If there is no application parameters for this function, params must
    be set to NULL. 
    
    @return Message @link PBAPC_PULL_PHONEBOOK_CFM_T PBAPC_PULL_PHONEBOOK_CFM
    @endlink  is sent to the application.
    
*/
void PbapcPullPhonebookRequest( uint16 device_id, 
                                PbapcPhoneRepository repository, 
                                PbapcPhoneBook phonebook,
                                const PbapcPullPhonebookParams *params );

/*!
    @brief Change the current active phonebook.  

    @param device_id PBAPC Session Handle.
    @param repository Repository containing the new phonebook.
    @param phonebook The new phonebook.

    This function is to change the phonebook directory.    
    To change to a different phonebook in the same repository,
    use pbap_current as the repository. 
    
    @return Message @link PBAPC_SET_PHONEBOOK_CFM_T PBAPC_SET_PHONEBOOK_CFM 
    @endlink  is sent to the application.
*/
void PbapcSetPhonebookRequest( uint16 device_id, 
                               PbapcPhoneRepository repository, 
                               PbapcPhoneBook phonebook );


/*!
    @brief Pull the vCard listing of the current phonebook.

    @param device_id PBAPC Session Handle.
    @param phonebook Name of the phonebook folder to retrieve. 
    @param params The application parameters for Pull vCard function.

    This function is to pull a list of vCards from a phonebook directory.
    If there is no application parameters for this function, params must
    be set to NULL. To get the Vcard listing of current phonebook use
    pbap_root as the phonebook.

    @return Message @link PBAPC_PULL_VCARD_LISTING_CFM_T 
    PBAPC_PULL_VCARD_LISTING_CFM @endlink  is sent to the application.
*/
void PbapcPullVcardListingRequest( uint16 device_id, 
                                   PbapcPhoneBook phonebook, 
                                   const PbapcPullvCardListParams *params );

/*!
    @brief Pull  specific vCard object from the current phonebook.

    @param device_id PBAPC Session Handle.
    @param entry Entry number of the vCard to download.
    @param params The application parameters for Pull vCard function.

    This function is to get a specific vCard from the current phonebook.
    If there is no application parameters for this function, params must
    be set to NULL

    @return Message @link PBAPC_PULL_VCARD_ENTRY_CFM_T 
    PBAPC_PULL_VCARD_ENTRY_CFM @endlink is sent to the application.
*/
void PbapcPullVcardEntryRequest( uint16 device_id,
                                 uint32 entry, 
                                 const PbapcPullvCardEntryParams* params );

/*!
    @brief Pull more data from the remote server

    @param device_id PBAPC Session Handle.

    This function is to get more data for the ongoing Pull operation if 
    it has returned with a status pbapc_pending. Before
    calling this function, the application must consume any Source data 
    received in the last PBAPC_PULL_PHONE_BOOK_CFM or 
    PBAPC_PULL_VCARD_LISTING_CFM or PBAPC_PULL_VCARD_ENTRY_CFM message.

    @return The library returns one of the PBAPC_PULL_*_CFM message
    depends on the ongoing pull operation with more data.
*/
void PbapcPullContinue(uint16 device_id);

/*!
    @brief Request to terminate the ongoing Pull operation

    @param device_id PBAPC Session Handle.

    This function is to free all resources and complete a pull operation. 
    If the pull operation is already in progress it aborts the operation.
    Before calling this function, the application must consume any Source
    data received in the last PBAPC_PULL_PHONE_BOOK_CFM or  
    PBAPC_PULL_VCARD_LISTING_CFM or PBAPC_PULL_VCARD_ENTRY_CFM message.

    @return The library returns one of the PBAPC_PULL_*_CFM message
    on aborting an ongoing pull operation. If there is no pull operation in 
    progress it just free up the resources without returning any message.
*/
void PbapcPullComplete(uint16 device_id);


/*!
    @brief Disconnect from an server.  
    @param device_id PBAPC Session Handle.
    
    @return Mesasge @link PBAPC_DISCONNECT_IND_T PBAPC_DISCONNECT_IND@endlink
    is sent to the application.
*/
void PbapcDisconnectRequest(uint16 device_id);

/*!
    @brief Check how many PBAPC links have been connected
    
    @return the number of pbapc links.
*/
uint8 PbapcGetNoOfConnection(void);
        
/*!
	@brief Obtain the properties of the remote server.
	@param device_id PBAPC Session Handle.
	
	@return the server to obtain the supported repositories mask.
*/
uint8 PbapcGetServerProperties(uint16 device_id);

/*!
	@brief Obtain the sink of the corresponding pbap link.
	@param device_id PBAPC Session Handle.
	
	@return the sink of obex connection.
*/
Sink PbapcGetSink(uint16 device_id);

#endif /* PBAP_CLIENT_H_ */


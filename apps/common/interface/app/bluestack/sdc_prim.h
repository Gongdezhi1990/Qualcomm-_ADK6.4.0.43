/*!

Copyright (c) 2001 - 2016 Qualcomm Technologies International, Ltd.
  

\file   sdc_prim.h

\brief  This file contains Service Discovery Client specific type definitions

        The interface between the Service Discovery Application (SDA) and the
        Service Discovery Client (SDC) sub-system.
*/

#ifndef BLUESTACK__SDC_PRIM_H     /* Once is enough */
#define BLUESTACK__SDC_PRIM_H

#include "bluestack/bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SDC_RES_TABLE \
    SDC_RES_TABLE_X(SDC_SUCCESS,                    = 0x0000)               SDC_RES_TABLE_SEP /*!< The success code */  \
    \
    /* SDC open search result codes */ \
    SDC_RES_TABLE_X(SDC_OPEN_SEARCH_BUSY,           = SDC_ERRORCODE_BASE)   SDC_RES_TABLE_SEP /*!< Currently performing search */ \
    SDC_RES_TABLE_X(SDC_OPEN_SEARCH_OPEN,           SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< SDC already has connection open */ \
    SDC_RES_TABLE_X(SDC_OPEN_DISCONNECTED,          SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Remote server device disconnected */ \
    \
    /* SDC_*_CFM.response Codes */ \
    SDC_RES_TABLE_X(SDC_ERROR_RESPONSE_PDU,         SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< SDP_ErrorResponse PDU received */ \
    SDC_RES_TABLE_X(SDC_NO_RESPONSE_DATA,           SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Empty response - no results */ \
    SDC_RES_TABLE_X(SDC_CON_DISCONNECTED,           SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Remote device disconnected */ \
    SDC_RES_TABLE_X(SDC_CONNECTION_ERROR_UNKNOWN,   SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Remote device refused connection */ \
    SDC_RES_TABLE_X(SDC_CONFIGURE_ERROR,            SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< L2CAP config failed */ \
    SDC_RES_TABLE_X(SDC_SEARCH_DATA_ERROR,          SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Search data is invalid */ \
    SDC_RES_TABLE_X(SDC_DATA_CFM_ERROR,             SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Failed to transmit PDU */ \
    SDC_RES_TABLE_X(SDC_SEARCH_BUSY,                SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Search is busy */ \
    SDC_RES_TABLE_X(SDC_RESPONSE_PDU_HEADER_ERROR,  SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Header error in response from SDS */ \
    SDC_RES_TABLE_X(SDC_RESPONSE_PDU_SIZE_ERROR,    SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Size error in response from SDS */ \
    SDC_RES_TABLE_X(SDC_RESPONSE_TIMEOUT_ERROR,     SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Timeout awaiting response from SDS */ \
    SDC_RES_TABLE_X(SDC_SEARCH_SIZE_TO_BIG,         SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Search size exceeds L2CAP MTU */ \
    SDC_RES_TABLE_X(SDC_RESPONSE_OUT_OF_MEMORY,     SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Response too big for memory */ \
    SDC_RES_TABLE_X(SDC_RESPONSE_TERMINATED,        SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< The search was terminated */ \
    \
    /* SDC_CLOSE_SEARCH_IND. result Codes */ \
    SDC_RES_TABLE_X(SDC_SDS_DISCONNECTED,           SDC_RES_TABLE_DUMMY)    SDC_RES_TABLE_SEP /*!< Disconnected by SDS server */ \
    SDC_RES_TABLE_X(SDC_SDC_DISCONNECTED,           SDC_RES_TABLE_DUMMY)                      /*!< Disconnected by SDC client */

#define SDC_RES_TABLE_DUMMY
#define SDC_RES_TABLE_X(a, b) a b
#define SDC_RES_TABLE_SEP ,

typedef enum
{
    SDC_RES_TABLE
} SDC_RESPONSE_T;

#undef SDC_RES_TABLE_DUMMY
#undef SDC_RES_TABLE_X
#undef SDC_RES_TABLE_SEP

/*------------------------------------------------
     Specific success codes
--------------------------------------------------*/
#define SDC_OPEN_SEARCH_OK   SDC_SUCCESS  /* Open search succeeded */
#define SDC_RESPONSE_SUCCESS SDC_SUCCESS  /* Search succeeded */


/*-------------------------------------------------- 
 
     PDU code Codes 

     These codes are those returned as the ErrorCode parameter of the
     SDP_ErrorResponse PDU. See the Bluetooth spec, section 4.4.1 for details.
--------------------------------------------------*/
#define PDU_ERROR_VERSION        0x0001
#define PDU_ERROR_SR_HANDLE      0x0002
#define PDU_ERROR_SYNTAX         0x0003
#define PDU_ERROR_PDU_SIZE       0x0004
#define PDU_ERROR_CONT_STATE     0x0005
#define PDU_ERROR_INSUFF_RES     0x0006


/*------------------------------------------------
     Protocol Data Unit PDU ID's
         (Not to be used within BlueStack SDP primitives)
--------------------------------------------------*/
#ifndef BCHS_WINCE      /*      also defined in win ce          */
#define SDP_ERROR_RESPONSE                    0x01
#define SDP_SERVICE_SEARCH_REQUEST            0x02
#define SDP_SERVICE_SEARCH_RESPONSE           0x03
#define SDP_SERVICE_ATTRIBUTE_REQUEST         0x04
#define SDP_SERVICE_ATTRIBUTE_RESPONSE        0x05
#define SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST  0x06
#define SDP_SERVICE_SEARCH_ATTRIBUTE_RESPONSE 0x07
#endif

/*-------------------------------------------------------------
SDC Primitives

SDC primitives occupy the number space from SDC_PRIM_BASE to
(SDC_PRIM_BASE | 0x00FF).
---------------------------------------------------------------*/

#define SDC_PRIM_DOWN SDC_PRIM_BASE
#define SDC_PRIM_UP (SDC_PRIM_BASE | 0x0040)

typedef enum sdc_prim_tag
{
    ENUM_SDC_SERVICE_SEARCH_REQ = SDC_PRIM_DOWN,
    ENUM_SDC_SERVICE_ATTRIBUTE_REQ,
    ENUM_SDC_SERVICE_SEARCH_ATTRIBUTE_REQ,
    ENUM_SDC_OPEN_SEARCH_REQ,
    ENUM_SDC_CLOSE_SEARCH_REQ,
    ENUM_SDC_CONFIG_REQ,
    ENUM_SDC_TERMINATE_PRIMITIVE_REQ,

    ENUM_SDC_SERVICE_SEARCH_CFM = SDC_PRIM_UP,
    ENUM_SDC_SERVICE_ATTRIBUTE_CFM,
    ENUM_SDC_SERVICE_SEARCH_ATTRIBUTE_CFM,
    ENUM_SDC_OPEN_SEARCH_CFM,
    ENUM_SDC_CLOSE_SEARCH_IND,
    ENUM_SDC_CONFIG_CFM
} SDC_PRIM_T;

#define SDC_SERVICE_SEARCH_REQ           ((sdc_prim_t)(ENUM_SDC_SERVICE_SEARCH_REQ))
#define SDC_SERVICE_SEARCH_CFM           ((sdc_prim_t)(ENUM_SDC_SERVICE_SEARCH_CFM))
#define SDC_SERVICE_ATTRIBUTE_REQ        ((sdc_prim_t)(ENUM_SDC_SERVICE_ATTRIBUTE_REQ))
#define SDC_SERVICE_ATTRIBUTE_CFM        ((sdc_prim_t)(ENUM_SDC_SERVICE_ATTRIBUTE_CFM))
#define SDC_SERVICE_SEARCH_ATTRIBUTE_REQ ((sdc_prim_t)(ENUM_SDC_SERVICE_SEARCH_ATTRIBUTE_REQ))
#define SDC_SERVICE_SEARCH_ATTRIBUTE_CFM ((sdc_prim_t)(ENUM_SDC_SERVICE_SEARCH_ATTRIBUTE_CFM))
#define SDC_TERMINATE_PRIMITIVE_REQ      ((sdc_prim_t)(ENUM_SDC_TERMINATE_PRIMITIVE_REQ))
#define SDC_OPEN_SEARCH_REQ              ((sdc_prim_t)(ENUM_SDC_OPEN_SEARCH_REQ))
#define SDC_OPEN_SEARCH_CFM              ((sdc_prim_t)(ENUM_SDC_OPEN_SEARCH_CFM))
#define SDC_CLOSE_SEARCH_REQ             ((sdc_prim_t)(ENUM_SDC_CLOSE_SEARCH_REQ))
#define SDC_CONFIG_REQ                   ((sdc_prim_t)(ENUM_SDC_CONFIG_REQ))
#define SDC_CONFIG_CFM                   ((sdc_prim_t)(ENUM_SDC_CONFIG_CFM))
#define SDC_CLOSE_SEARCH_IND             ((sdc_prim_t)(ENUM_SDC_CLOSE_SEARCH_IND))

/*----------------------------------
 end of #define of primitives
------------------------------------*/

/*----------------------------------
 primitive type for sdp
------------------------------------*/
typedef uint16_t        sdc_prim_t;             /* same as l2cap_prim.h */

/*----------------------------------
 other parameter typedefs
------------------------------------*/

/*----------------------------------------------------------------------------*
 * PURPOSE
 *       Submits a search request to the SDC sub-system.  The device
 *       specified by bd_addr is searched using the search pattern
 *       specified by *srch_pttrn.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;               /* Always SDC_SERVICE_SEARCH_REQ*/
    phandle_t           phandle;            /* routing handle */
    uint16_t            con_ctx;            /* Reserved - shall be set to 0 */
    BD_ADDR_T           bd_addr;            /* Address of Bluetooth device */
                                            /* to be searched */
    uint16_t            size_srch_pttrn;    /* size of the search pattern */
    uint8_t             *srch_pttrn;        /* Pointer to search pattern */
                                            /* described as data element */
                                            /* sequence */
    uint16_t            max_num_recs;       /* Maximum number of service */
                                            /* records to be returned */
} SDC_SERVICE_SEARCH_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *     Returns the results of a search request.  The number of
 *     service records is returned in num_recs_ret, the size of the service
 *     records is returned in size_rec_list and the service record handles are
 *     returned via the *rec_list pointer.
 *     It also indicates whether or not a search is successful,
 *     using response, and if an error has occurred it indicates the error code
 *     in err_code.  The *err_info is used as a place-holder for error-specific
 *     information that may be used in future versions of the specification.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;           /* Always SDC_SERVICE_SEARCH_CFM */
    phandle_t           phandle;        /* as supplied in REQ */
    uint16_t            con_ctx;        /* Reserved - shall be set to 0 */
    BD_ADDR_T           bd_addr;        /* Address of Bluetooth device */
    uint16_t            num_recs_ret;   /* Number of service records */
                                        /* returned */
    uint16_t            size_rec_list;  /* size of the rec_list in bytes */
    uint8_t             *rec_list;      /* Pointer to list of */
                                        /* ServiceRecordHandles */
    uint16_t            response;       /* Indicates whether or not the */
                                        /* search was successful */
    uint16_t            err_code;       /* Error code if one has occurred */
    uint16_t            size_err_info;  /* size of the err_info */
    uint8_t             *err_info;      /* Error-specific information - not*/
                                        /* used in current version of spec */
} SDC_SERVICE_SEARCH_CFM_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Submits an attribute request to the SDC sub-system.  The attributes
 *      specified by *attr_list, in the device specified by bd_addr, are
 *      obtained from the service record with a ServiceRecordsHandle of
 *      svc_rec_hndl.
 *      The results of the search are the attribute values of the attributes
 *      specified in *attr_list.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;           /* Always */
                                        /* SDC_SERVICE_ATTRIBUTE_REQ */
    phandle_t           phandle;        /* routing handle */
    uint16_t            con_ctx;        /* Reserved - shall be set to 0 */
    BD_ADDR_T           bd_addr;        /* Address of Bluetooth device */
                                        /* to be searched */
    uint32_t            svc_rec_hndl;   /* ServiceRecordHandle of */
                                        /* service being queried */
    uint16_t            size_attr_list; /* size of the attr_list in bytes */
    uint8_t             *attr_list;     /* Pointer to attribute list */
                                        /* described as data element */
                                        /* sequence */

    /* The maximum number of attribute bytes to be returned in each response
     * packet. If multiple responses are required, they may each contain
     * up to this amount of attribute data.
     */
    uint16_t            max_num_attr;
} SDC_SERVICE_ATTRIBUTE_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Returns the results of a service attribute request. The attribute list
 *      is contained in 'attr_list' and its size in 'size_attr_list'. The event
 *      also indicates whether or not the request is successful, using
 *      response, and if an error has occurred it indicates the error code in
 *      err_code. The *err_info is used as a place-holder for error-specific
 *      information that may be used in future versions of the specification.
 *
 *      NOTE - The initial Data Element Size descriptor, which contains the
 *          overall size of the service record, is stripped off by SDP and is
 *          not included in the 'attr_list' data.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;           /* Always */
                                        /* SDC_SERVICE_ATTRIBUTE_CFM */
    phandle_t           phandle;        /* as supplied in REQ */
    uint16_t            con_ctx;        /* Reserved - shall be set to 0 */
    BD_ADDR_T           bd_addr;
    uint16_t            size_attr_list; /* size of the attr_list in bytes */

    /* The attribute list, described as a data element sequence */
    uint8_t             *attr_list;

    uint16_t            response;       /* Indicates whether or not the */
                                        /* search was successful */
    uint16_t            err_code;       /* Error code if one has occurred*/
    uint16_t            size_err_info;  /* size of the err_info */
    uint8_t             *err_info;      /* Error-specific information - */
                                        /* not used in current version */
                                        /* of spec */
} SDC_SERVICE_ATTRIBUTE_CFM_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *       Submits a search attribute request to the SDC sub-system, which is the
 *       same as a combination of SDC_SRCH_REQ and SDC_ATTR_REQ.  The device
 *       specified by bd_addr is searched using the search pattern
 *       specified by *srch_pttrn.  The results of the search are the
 *       attribute values of the attributes specified in *attr_list.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;               /* Always */
                                            /* SDC_SERVICE_SEARCH_ATTRIBUTE_REQ */
    phandle_t           phandle;            /* routing handle */
    uint16_t            con_ctx;            /* Reserved - shall be set to 0 */
    BD_ADDR_T           bd_addr;            /* Address of Bluetooth device */
                                            /* to be searched */
    uint16_t            size_srch_pttrn;    /* size of the search pattern - bytes */
    uint8_t             *srch_pttrn;        /* Pointer to search pattern */
                                            /* described as data element */
                                            /* sequence */
    uint16_t            size_attr_list;     /* size of the attr list - bytes */
    uint8_t             *attr_list;         /* Pointer to attribute list */
                                            /* described as data element */
                                            /* sequence */

    /* The maximum number of attribute bytes to be returned in each response
     * packet. If multiple responses are required, they may each contain
     * up to this amount of attribute data.
     */
    uint16_t            max_num_attr;
} SDC_SERVICE_SEARCH_ATTRIBUTE_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Returns the results of a service search attribute request. One primitive
 *      is issued for each service record received from the server, containing
 *      the attribute list in 'attr_list' and its size in 'size_attr_list'.
 *      The total size of the entire response (over all services and primitives)
 *      is contained in 'total_response_size'.
 *
 *      NOTE - The initial Data Element Size descriptor, which contains the
 *          overall size of the service record, is stripped off by SDP and is
 *          not included in the 'attr_list' data.
 *
 *      SDP collates the responses arriving from the server and sends only
 *      complete service records to the application. If a service record turns
 *      out to be too large for the available memory, the issued primitive will
 *      have 'size_attr_list = 0' and 'attr_list = NULL', and 'response' will
 *      contain the error SDC_RESPONSE_OUT_OF_MEMORY. SDP will discard the
 *      large service record, and will continue to handle subsequent records.
 *      Therefore the application will continue to receive primitives for the
 *      other service records, memory permitting.
 *
 *      The 'more_to_come' flag indicates to the application whether there are
 *      more service records to come. When the last service record is sent,
 *      'more_to_come' is set to FALSE.
 *
 *      Note that SDP may stop issuing service records if it detects an error,
 *      this is indicated to the application by setting 'more_to_come = FALSE'
 *      and setting the 'response' and 'err_code' fields accordingly. The
 *      'response' field indicates locally-detected error conditions, such as
 *      invalid responses from the server; the 'err_code' field indicates errors
 *      reported from the server.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;           /* Always */
                                        /* SDC_SERVICE_SEARCH_ATTRIBUTE_CFM */
    phandle_t           phandle;        /* as supplied in REQ */
    uint16_t            con_ctx;        /* Reserved - shall be set to 0 */
    BD_ADDR_T           bd_addr;

    /* The total size of the entire response in bytes. Note that SDP will
     * strip out the initial DES descriptor of each service record, so
     * applications should not expect to receive this many bytes. This
     * value is purely informational.
     */
    uint16_t            total_response_size;

    /* The attribute list and size for a single service, described as a data
     * element sequence. For multiple services, this primitive will be
     * issued multiple times, each one containing the attribute data for
     * one service.
     */
    uint16_t            size_attr_list;
    uint8_t             *attr_list;

    /* Non-zero if there are still some more service records to come. FALSE
     * means that the query has finished.
     */
    bool_t              more_to_come;

    uint16_t            response;       /* Indicates whether or not the */
                                        /* search was successful */
    uint16_t            err_code;       /* Error code if one has occurred*/
    uint16_t            size_err_info;  /* size of the error info */
    uint8_t             *err_info;      /* Error-specific information  */
                                        /* not used in current version */
                                        /* of spec */
} SDC_SERVICE_SEARCH_ATTRIBUTE_CFM_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Terminates the currently active search primtive. If a search is in
 *      progress then the corresponding search cfm primitive will be issued
 *      with its response set to SDC_RESPONSE_TERMINATED.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;    /* Always SDC_TERMINATE_PRIMITIVE_REQ */
    phandle_t           phandle; /* routing handle */
} SDC_TERMINATE_PRIMITIVE_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *        Submits the opening of a permanent connection request.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;     /* Always SDC_OPEN_SEARCH_REQ */
    phandle_t           phandle;  /* routing handle */
    uint16_t            con_ctx;  /* Reserved - shall be set to 0 */
    BD_ADDR_T           bd_addr;  /* Address of Bluetooth device to be */
                                  /* searched */
} SDC_OPEN_SEARCH_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *       Submits the response to the opening of a permanent connection
 *       request.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;     /* Always SDC_OPEN_SEARCH_CFM */
    phandle_t           phandle;  /* routing handle */
    uint16_t            con_ctx;  /* Reserved - shall be set to 0 */
    uint16_t            result;   /* result from open search req */
} SDC_OPEN_SEARCH_CFM_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Requests closure of the permanent connection previously opened with
 *      SDC_OPEN_SEARCH_REQ. When the connection has closed, an
 *      SDC_CLOSE_SEARCH_IND primitive is issued, indicating to the application
 *      that it may now begin another search.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;     /* Always SDC_CLOSE_SEARCH_REQ */
    phandle_t           phandle;  /* routing handle */
} SDC_CLOSE_SEARCH_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *       Configure SDP parameters
 *
 *       Apply L2CAP MTU and configure optional behaviour.
 *
 *       Currently there is no optional behaviour to configure,
 *       so all bits in the flags bitfield shall be 0.
 *
 *       If you just want to set optional behaviour and leave the L2CAP MTU
 *       unchanged then set the MTU to 0, which will always be ignored.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;       /* Always SDC_CONFIG_REQ */
    phandle_t           phandle;    /* routing handle */
    uint16_t            l2cap_mtu;  /* L2CAP MTU value */
    uint16_t            flags;      /* Bitfield - currently all bits reserved (zero) */
} SDC_CONFIG_REQ_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Response to SDC_CONFIG_REQ
 *
 *      Returns the values of l2cap_mtu and flags in effect as a result of
 *      the SDC_CONFIG_REQ. If SDC_CONFIG_REQ was completely successful then
 *      the values of l2cap_mtu and flags will match those in SDC_CONFIG_REQ.
 *
 **---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;       /* Always SDC_CONFIG_CFM */
    phandle_t           phandle;    /* routing handle */
    uint16_t            l2cap_mtu;  /* Actual L2CAP MTU value used */
    uint16_t            flags;      /* Bitfield - currently all bits reserved (zero) */
} SDC_CONFIG_CFM_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Indicates that the connection previously opened with SDC_OPEN_SEARCH_REQ
 *      has now closed. Applications calling SDC_CLOSE_SEARCH_REQ should wait
 *      for SDC_CLOSE_SEARCH_IND before attempting to open another search.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;     /* Always SDC_CLOSE_SEARCH_IND */
    phandle_t           phandle;  /* routing handle */
    uint16_t            con_ctx;  /* Reserved - shall be set to 0 */
    uint16_t            result;   /* result connection error */
} SDC_CLOSE_SEARCH_IND_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *              Common elements of "upstream" SDC and SDS primitives
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;         /* Whatever */
    phandle_t           phandle;      /* routing handle */
} SDC_SDS_UPSTREAM_COMMON_T;

typedef SDC_SDS_UPSTREAM_COMMON_T SDC_UPSTREAM_COMMON_T;

/*------------------------------------------------
 Union of the primitives
-------------------------------------------------*/

typedef union
{
    sdc_prim_t                            type;
    SDC_UPSTREAM_COMMON_T                 sdc_upstream_common;

    SDC_SERVICE_SEARCH_REQ_T              sdc_service_search_req;
    SDC_SERVICE_SEARCH_CFM_T              sdc_service_search_cfm;
    SDC_SERVICE_ATTRIBUTE_REQ_T           sdc_service_attribute_req;
    SDC_SERVICE_ATTRIBUTE_CFM_T           sdc_service_attribute_cfm;
    SDC_SERVICE_SEARCH_ATTRIBUTE_REQ_T    sdc_service_search_attribute_req;
    SDC_SERVICE_SEARCH_ATTRIBUTE_CFM_T    sdc_service_search_attribute_cfm;
    SDC_OPEN_SEARCH_REQ_T                 sdc_open_search_req;
    SDC_OPEN_SEARCH_CFM_T                 sdc_open_search_cfm;
    SDC_CLOSE_SEARCH_REQ_T                sdc_close_search_req;
    SDC_CLOSE_SEARCH_IND_T                sdc_close_search_ind;
    SDC_CONFIG_REQ_T                      sdc_config_req;
    SDC_CONFIG_CFM_T                      sdc_config_cfm;
    SDC_TERMINATE_PRIMITIVE_REQ_T         sdc_terminate_primitive_req;
} SDC_UPRIM_T;

#ifdef __cplusplus
}
#endif

#endif /* ndef _SDC_PRIM_H */


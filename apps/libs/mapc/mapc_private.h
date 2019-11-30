/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    mapc_private.h
    
DESCRIPTION
    Private internal header file for the MAP Client library
*/

#ifndef MAPC_INTERNAL_H
#define MAPC_INTERNAL_H

#include <message.h>
#include <bdaddr_.h>

#define OBEX_GETPUT_CFM_T OBEX_PUT_CFM_T

/* Macros */
#define mapcFrameListFolderParams( session, count, offset ) \
            mapcFrameListAppParams( session, count, offset, \
                                    MAPC_LIST_APP_PARAM_SIZE , \
                              PanicUnlessMalloc( MAPC_LIST_APP_PARAM_SIZE ) )

/* Filter - Subject, date, sender, read */
#define MAPC_DEF_FILTER_PARAM     0x00001007

/* Mask the filters from the application parameter */
#define MAPC_READ_STATUS_MASK     0x30 /* Use upper nibble for Read Status */
#define MAPC_READ_STATUS_SHIFT    4    /* Shift 1 nibble for Read Status */ 
#define MAPC_MSG_TYPE_MASK        0x0F

#define MAPC_LIST_APP_PARAM_SIZE      ( MAPC_2BYTE_APP_HDR_SIZE * 2 )

/* Maximum number of bytes expected per record */
#define MAPC_MAX_SDP_ATTR_SIZE      256

/* Fixed Header sizes  */
/* Fixed header space for app Header(6), type(3) and Body(4) */
#define MAPC_NOTIFICATION_HDR_SIZE          13
/* Fixed header space for app header(11) + type(3) */ 
#define MAPC_GET_FOLDER_HDR_SIZE            14
/* Fixed header space - appParam(11) + type(3) + name(3)*/
#define MAPC_GET_MSG_LIST_HDR_SIZE          17
/* Fixed header space - name(21) + Type (3) + appParam(9) + body (4) */
#define MAPC_SET_MSG_HDR_SIZE               37
/* Fixed header space - body(4) */
#define MAPC_UPDATE_INBOX_HDR_SIZE          4
/* Fixed header space -  name(21) + type(3) + appParam(9) */
#define MAPC_GET_MSG_HDR_SIZE               33
/* Fixed header space - type(3) + appParam(12) + name(3) */
#define MAPC_PUT_MSG_HDR_SIZE              18


#define MAPC_BYTE_SIZE                      1
#define MAPC_2BYTE_SIZE                     2
#define MAPC_4BYTE_SIZE                     4
#define MAPC_BYTE_APP_HDR_SIZE              3
#define MAPC_2BYTE_APP_HDR_SIZE             4
#define MAPC_4BYTE_APP_HDR_SIZE             6

/* size of the message handle is 16 hex digits */
#define MAPC_HANDLE_LEN                     16 

/* Listing object must have less than this size  */
#define MAPC_MAX_LIST_OBJECT_SIZE       0xFFFF

/* Default Application parameter values */
#define MAPC_PUSH_TRANSPARENT           0
#define MAPC_PUSH_RETRY                 1

#define IsMapMasClient(session) ( session == mapc_mas_client )

#define MAPC_IDLE( session ) session->state = mapc_idle
#define MAPC_DISCONNECTED( session ) session->state = mapc_session_progress
#define MAPC_DISCONNECTING( session ) session->state = mapc_disconnect_progress
#define MAPC_CONNECT_PENDING( session ) session->state = mapc_connect_progress
#define MAPC_CONNECTED( session )  session->state = mapc_connected
#define MAPC_SET_FUNCTION( session, func) session->function = func
#define MAPC_RESET_FUNCTION( session ) session->function = mapc_no_function

#define IsMapcIdle( session ) ( session->state == mapc_idle )
#define IsMapcDisconnected( session ) \
                        ( session->state == mapc_session_progress )
#define IsMapcDisconnecting( session ) \
                        ( session->state == mapc_disconnect_progress )
#define IsMapcConnecting( session )   \
                        ( session->state == mapc_connect_progress )
#define IsMapcConnected( session ) \
                        ( session->state == mapc_connected )
/* Mapc Function Free */
#define IsMapcFuncFree( session ) \
                        ( session ->function == mapc_no_function )

#define IsMapcFuncBlocked( session ) \
                        ( session -> function != mapc_no_function ) 




typedef struct __MAPC *Mapc;
/* Internal Message Structures */
typedef struct 
{
    Task    theAppTask;
    bdaddr  addr;
    uint8   channel;    

}MAPC_INT_CONNECT_REQ_T;

typedef struct
{
    Task    theAppTask;
    bdaddr  addr;
    uint8   channel;
    bool    accept;
    Sink    sink;
}MAPC_INT_CONNECT_RESP_T;


/* Internal Task Delete Request. This is used for 
MAPC_MAS_DISCONNECT_IND, MAPC_MNS_DISCONNECT_IND, MapcMasDeleteSession()
and MapcMnsDeleteSession  */
typedef struct{
    Mapc   session;

}MAPC_INT_TASK_DELETE_T, MAPC_DISCONNECT_IND_T;

/* Send Set Message Notification request */
typedef struct{
    Mapc   session;
    bool   status;

}MAPC_INT_SET_NOTIFICATION_T;

typedef struct{
    Mapc    session;    /* Mapc Session */
    uint16  respCode;   /* OBEX Response Code */
}MAPC_INT_SEND_EVENT_RESP_T;

typedef struct{
    Mapc    session;
    bool    flag;
    uint16  nameLen; 
    uint8   name[1];
} MAPC_INT_SET_FOLDER_T;

typedef struct{
    Mapc    session;
    uint16  maxListCount;
    uint16  startOffset;

}MAPC_INT_GET_FOLDER_T;

typedef struct{
    Mapc    session;
    uint16  nameLen;
    uint16  maxListCount;
    uint16  startOffset;
    uint8   filter;
    uint8   name[1];

}MAPC_INT_GET_MSG_LIST_T;

typedef struct{
    Mapc    session;
    bool    charSet;
    bool    final;
    Source  data;
    uint16    nameLen;
    uint8    name[1];
}MAPC_INT_PUT_MSG_T;

typedef struct{
    Mapc    session;
    bool    final;
    Source  data;
}MAPC_INT_PUT_CONTINUE_T;

typedef struct{
    Mapc session;
    bool proceed;
}MAPC_INT_GET_CONTINUE_T;

typedef struct{
    Mapc    session;
    uint8   charSet;
    uint8   handle[MAPC_HANDLE_LEN];
}MAPC_INT_GET_MSG_T;

typedef struct{
    Mapc    session;
}MAPC_INT_UPDATE_INBOX_T;

typedef struct{
    Mapc    session;
    uint8   indicator;
    uint8   value;
    uint8   handle[MAPC_HANDLE_LEN];
}MAPC_INT_SET_MSG_STATUS_T;

/* SDP Registration state */
typedef enum
{
    mapc_sdp_reg_none = 0,
    mapc_sdp_reg_pending,
    mapc_sdp_reg_progress,
    mapc_sdp_registered
}MapcRecState;

/* MAPC Profile task structure */
typedef struct 
{
    Task            regApp;     /* The App Task before session creation*/
    MapcRecState    eSdpState;  /* SDP Record State */    
}MapcLibState;

/* MAPC Role */
typedef enum
{
    mapc_mns_server,            /* MNS Server session */
    mapc_mas_client             /* MAS Client session */
}MapcRole;

/* Current state of the connection */
typedef enum
{
    mapc_idle,                  /* idle state */
    mapc_session_progress,      /* Session creation or deletion in progress */
    mapc_connect_progress,      /* OBEX Connect in Progress */
    mapc_connected,             /* OBEX connected */
    mapc_disconnect_progress    /* OBEX_Disconnect in progress */
}MapcState;


/* Mapc active function */
typedef enum
{
    mapc_no_function,           /* No command is active */
    mapc_send_event,            /* Send Event in progress */
    mapc_reg_notification,      /* Register Notification in progress */
    mapc_set_folder,            /* Folder navigation in progress */
    mapc_folder_listing,        /* Get folder listing in progress */
    mapc_message_listing,       /* Get message listing in progress */
    mapc_get_message,           /* Retrieving of a message in progress */
    mapc_message_status,        /* Modify the status of a message */
    mapc_push_message,          /* Pushing of a message in progress */
    mapc_update_inbox           /* Update message inbox */
}MapcFunction;

typedef enum
{
    mapc_app_param_invalid,
    mapc_app_max_list_count ,               /* Tag ID = 0x01 */
    mapc_app_start_offset,                  /* 0x02 */
    mapc_app_filter_message_type,           /* 0x03 */
    mapc_app_filter_period_begin,           /* 0x04 */
    mapc_app_filter_period_end ,            /* 0x05 */
    mapc_app_filter_read_status,            /* 0x06 */
    mapc_app_filter_recipient,              /* 0x07 */
    mapc_app_filter_orginator,              /* 0x08 */
    mapc_app_filter_priority,               /* 0x09 */
    mapc_app_attachment,                    /* 0x0A */
    mapc_app_transparent,                   /* 0x0B */
    mapc_app_retry,                         /* 0x0C */
    mapc_app_new_message,                   /* 0x0D */
    mapc_app_notification_status,           /* 0x0E */
    mapc_app_mas_instance_id ,              /* 0x0F */
    mapc_app_param_mask,                    /* 0x10 */
    mapc_app_folder_listing_size,           /* 0x11 */
    mapc_app_message_listing_size,          /* 0x12 */
    mapc_app_subject_length,                /* 0x13 */
    mapc_app_char_set,                      /* 0x14 */
    mapc_app_fraction_request,              /* 0x15 */
    mapc_app_fraction_deliver,              /* 0x16 */
    mapc_app_status_indicator,              /* 0x17 */
    mapc_app_status_value,                  /* 0x18 */
    mapc_app_mse_time                       /* 0x19 */
}MapcAppParams;


/* MAPC Session task structure */

struct __MAPC
{
    TaskData        task;       /* The MAPC Session TaskData */
    Task            appTask;    /* The application task for this session */
    Obex            session;    /* The OBEX Session Task */
    uint16          maxPktLen;  /* Maximum OBEX packet length */
    MapcRole        role;       /* Is it a MAS or MNS session? */
    MapcState       state;      /* Current state of the connection */
    MapcFunction    function;   /* Current on-going Map function */
    uint16          lastAppParam;/* context app value */
    Source          srcOut;     /* Saved src for outgoing multi packet
                                   Used for Uploading message */
    bool            finalPut;   /* More application data is expected to push */
};



/***********************************************************************
 Internal task management  routines
************************************************************************/
MapcState mapcGetSdpState( void );
void mapcSetSdpState( MapcState state );
void mapcProfileHandler(Task task, MessageId id, Message message);
void mapcSessionHandler(Task task, MessageId id, Message message);
void mapcTaskInitialize( Mapc mapcTask, Task appTask, MapcRole role);
void mapcHandleConnectCfm( Mapc mapcTask, OBEX_CONNECT_CFM_T* cfm );
void mapcHandleDeleteSessionInd( Mapc mapcTask,
                                 OBEX_DELETE_SESSION_IND_T *ind );
void mapcMasHandleGetPutCfm( Mapc masTask, OBEX_GETPUT_CFM_T* message );
void mapcMasHandleSetPathCfm( Mapc masTask, OBEX_SET_PATH_CFM_T* message );
/************************************************************************
 Profile handler functions 
************************************************************************/
void mapcIntDeleteTask( MAPC_INT_TASK_DELETE_T* req );
void mapcIntConnectResp( MAPC_INT_CONNECT_RESP_T *rsp );
void mapcMnsIntSendEventResp( MAPC_INT_SEND_EVENT_RESP_T *rsp );

/*************************************************************************
 Downstream functions 
*************************************************************************/
void mapcRegisterChannel( uint8 mnsChannel );
void mapcRegSdpRecord( void );
void mapcIntConnectReq( MAPC_INT_CONNECT_REQ_T *req );
void mapcMasIntSetNotification( MAPC_INT_SET_NOTIFICATION_T* message );
void mapcMasIntSetFolder( MAPC_INT_SET_FOLDER_T* message );
void mapcMasIntGetFolder( MAPC_INT_GET_FOLDER_T* message );
void mapcMasIntGetMsgList( MAPC_INT_GET_MSG_LIST_T* message );
void mapcMasIntSetMessage( MAPC_INT_SET_MSG_STATUS_T* message );
void mapcMasIntUpdateInbox( MAPC_INT_UPDATE_INBOX_T* message );
void mapcMasIntGetMsg( MAPC_INT_GET_MSG_T* message );
void mapcMasIntGetContinue( MAPC_INT_GET_CONTINUE_T* message);
void mapcMasIntPutMessage( MAPC_INT_PUT_MSG_T* message);
void mapcMasIntPutContinue( MAPC_INT_PUT_CONTINUE_T* message);

/************************************************************************
Upstream Handling functions
************************************************************************/
void mapcMnsConnectInd( OBEX_CONNECT_IND_T* msg );
void mapcMnsHandlePutInd( Mapc mnsTask, OBEX_PUT_IND_T* putInd );



#endif /* MAPC_INTERNAL_H */


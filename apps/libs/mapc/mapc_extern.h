/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    mapc_external.h
    
DESCRIPTION
    This header between is shared between the API modules and the Internal 
    functions. This is an interface file to the MAP API routines to access
    internal functionalities of MAP. 
*/

#ifndef MAPC_EXTERN_H
#define MAPC_EXTERN_H

#include <mapc.h>

/* Macros for creating messages */
#define MAKE_MAPC_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T)
#define MAKE_MAPC_MESSAGE_WITH_LEN(TYPE, LEN) \
    TYPE##_T *message = (TYPE##_T *)PanicUnlessMalloc(sizeof(TYPE##_T) + LEN - 1)

#ifdef MAPC_DEBUG
#include <stdio.h>
#include <panic.h>
#define MAPC_ERR(x) {printf x; Panic();}
#define MAPC_INFO(x) {printf x;}
#define MAPC_ASSERT(x) {if(!(x)) Panic();}
#define MAPC_ASSERT_RANGE(x, min, max) {if(((x)<(min))&&((x)>(max))) Panic();}
#else
#define MAPC_ERR(x)
#define MAPC_INFO(x)
#define MAPC_ASSERT(x) if(!x) return;
#define MAPC_ASSERT_RANGE(x, min, max) {if(((x)<(min))&&((x)>(max)))return ;}
#endif /* MAPC_DEBUG */

/* Maximum number of MAS Connections */
#define MAPC_MAS_MAX_CONNECTION     7

#define mapcMasSetNotificationCfm( session, status ) \
            mapcStatusCfm( session, MAPC_MAS_SET_NOTIFICATION_CFM, status )
#define mapcMasSetFolderCfm( session, status ) \
            mapcStatusCfm( session, MAPC_MAS_SET_FOLDER_CFM, status )
#define mapcMasUpdateInboxCfm( session , status ) \
            mapcStatusCfm( session, MAPC_MAS_UPDATE_INBOX_CFM, status )
#define mapcMasSetMessageCfm( session, status ) \
            mapcStatusCfm( session, MAPC_MAS_SET_MESSAGE_STATUS_CFM, status )

#define mapcMasGetFolderErrCfm( session, status) \
            mapcMasListingCfm( \
                session, MAPC_MAS_GET_FOLDER_LISTING_CFM, status, 0, 0, 0 )
#define mapcMasGetFolderCfm( session, status, total, len, src) \
            mapcMasListingCfm( session, MAPC_MAS_GET_FOLDER_LISTING_CFM,\
                               status, total, len, src )

#define mapcMasGetMsgListErrCfm( session, status) \
            mapcMasListingCfm( \
                session, MAPC_MAS_GET_MESSAGES_LISTING_CFM, status, 0, 0, 0 )
#define mapcMasGetMsgListCfm( session, status, total, len, src) \
            mapcMasListingCfm( session, MAPC_MAS_GET_MESSAGES_LISTING_CFM, \
                                status, total, len, src )

#define mapcMasGetMsgErrCfm( session, status ) \
            mapcMasGetMessageCfm( session, status, 0 , 0 )

#define mapcMasPutMsgCfm( session, status ) \
            mapcMasPutMessageCfm( session , status, 0 , 0 )
                               

/* Internal Messages */
typedef enum
{
    MAPC_INT_CONNECT_REQ = MAPC_API_MESSAGE_END,
    MAPC_INT_TASK_DELETE,
    MAPC_INT_SET_NOTIFICATION,
    MAPC_INT_CONNECT_RESP,
    MAPC_INT_SEND_EVENT_RESP,
    MAPC_INT_SET_FOLDER,
    MAPC_INT_GET_FOLDER, 
    MAPC_INT_GET_MSG_LIST,
    MAPC_INT_GET_MSG,
    MAPC_INT_GET_CONTINUE,
    MAPC_INT_UPDATE_INBOX,
    MAPC_INT_SET_MSG_STATUS,
    MAPC_INT_STATUS_CFM,
    MAPC_INT_FUNCTION_CFM,
    MAPC_INT_PUT_MSG,
    MAPC_INT_PUT_CONTINUE,

    /* Library Message limit */
    MAPC_INT_MESSAGE_END

} MapcIntMessageId;

typedef struct __MAPC MAPC;

/* This must map to MAPC_MAS_CONNECT_CFM_T and MAPC_MNS_CONNECT_CFM_T */
typedef struct{
    MAPC*       session;        /* The session handle. Invalid on failure */ 
    MapcStatus  status;         /* The Status of the MAP session. */
    bdaddr      addr;           /* BD Address of the remote device */
    uint8       channel;        /* rfcChannel associated with the session */
 
}MAPC_CONNECT_CFM_T;

typedef struct{
    MAPC*      session;         /* The session handle. */
    MapcStatus status;          /* The function status */

} MAPC_INT_STATUS_CFM_T;

typedef struct{
    MAPC*      session;      /* The session handle. */
    MapcStatus status;       /* The function status */
    uint16     remainLen;    /* Only valid for the first packet. */
    uint16     srcLen;       /* Source len */
    Source     src;          /* Source containing the OBEX headers*/

} MAPC_INT_FUNCTION_CFM_T;


/**************************************************************************
 Internal functions exposed to the APIs
***************************************************************************/
bool mapcRegisterMnsApp( Task theAppTask, bool popSdpRec, uint8 mnsChannel);
void mapcUnregisterMnsApp( uint32 recordHandle, uint8 mnsChannel );
Task mapcGetRegAppTask( void );
Task mapcGetProfileTask( void );
void mapcUnregMnsApp( void );
void mapcSdpSearch( Task theTask, const bdaddr* addr );
void mapcPostConnectReq( Task theTask, const bdaddr* addr, uint8 channel);
void mapcPostTaskDelete( MAPC* session );
void mapcPostRegisterNotification( MAPC* session, bool regStatus );
void mapcPostConnectResp(  Task theAppTask, 
                           const bdaddr* addr,
                           uint8  channel,
                           bool   accept,
                           Sink   connectID );
void mapcPostMnsSendEventResp( MAPC* session, uint16 response);
void mapcPostMasSetFolder( MAPC* session, 
                           bool up, 
                           const uint8* name, 
                           uint16 len );
void mapcPostMasGetFolderListing( MAPC* masSession, 
                                  uint16 maxListCount, 
                                  uint16 listStartOffset);

void mapcPostMasGetMessagesListing( Mas masSession, 
                                   const uint8* name,
                                   uint16 nameLen,
                                   uint16 maxListCount, 
                                   uint16 listStartOffset,
                                   MapcMessageFilter  filter);
void mapcPostMasGetMessage( Mas session, const uint8* handle, bool native );
void mapcPostMasContinue( Mas masSession, bool proceed );
void mapcPostMasUpdateInbox( MAPC* session ); 
void mapcPostMasSetMessageStatus( MAPC* session,
                                  const uint8* handle, 
                                  uint8 indicator, 
                                  uint8 value );

void mapcMasListingCfm( MAPC* session, 
                         MapcMessageId id,
                         MapcStatus status , 
                         uint16  totalLen,
                         uint16 len, 
                         Source src );

void mapcMasGetMessageCfm( MAPC* session, 
                           MapcStatus status , 
                           uint16 len, 
                           Source src );

void mapcMasPutMessageCfm( MAPC* session, 
                           MapcStatus status , 
                           uint16 len, 
                           const uint8* nameHdr );

void mapcPostMasPutMessage( Mas session, 
                            const uint8* name,
                            uint16 nameLen,
                            bool native,
                            bool moreData,
                            Source message );

void mapcPostMasPutContinue( Mas session, 
                             bool moreData,
                             Source message );


/**************************************************************************
 Function prototypes for Message API despatcher routines. These functions 
 are defined in mapc_messages.c
***************************************************************************/
void mapcMnsStartCfm( MapcStatus status , uint8 channel, uint32 handle );
void mapcConnectCfm( MAPC*        mapSession,  
                     MapcStatus   status,
                    const bdaddr  *addr,
                     uint8        channel);
void mapcDisconnectInd( MAPC* mapSession); 
void mapcStatusCfm( MAPC* session, MapcMessageId id, MapcStatus status); 
void mapcMnsSendEventInd( MAPC* session, bool final, uint16 len, Source src);
void mapcMnsShutdownCfm( MapcStatus status );
#endif /* MAPC_EXTERN_H */


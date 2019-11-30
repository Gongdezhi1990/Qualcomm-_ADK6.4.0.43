/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    obex_external.h
    
DESCRIPTION
    This header between is shared between the API modules and the Internal 
    functions.
*/

#ifndef OBEX_EXTERN_H
#define OBEX_EXTERN_H

#include <obex.h> 

/* Macros for creating messages */
#define MAKE_OBEX_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T)
#define MAKE_OBEX_MESSAGE_WITH_LEN(TYPE, LEN) \
    TYPE##_T *message = (TYPE##_T *)PanicUnlessMalloc(sizeof(TYPE##_T) + LEN - 1)

#ifdef OBEX_DEBUG
#include <stdio.h>
#include <panic.h>
#define OBEX_ERR(x) {printf x; Panic();}
#define OBEX_INFO(x) {printf x;}
#define OBEX_ASSERT(x) {if(!(x)) Panic();}
#define OBEX_ASSERT_RANGE(x, min, max) {if(((x)<(min))&&((x)>(max))) Panic();}
#else
#define OBEX_ERR(x)
#define OBEX_INFO(x)
#define OBEX_ASSERT(x) if(!x) return;
#define OBEX_ASSERT_RANGE(x, min, max) {if(((x)<(min))&&((x)>(max)))return ;}
#endif /* OBEX_DEBUG */

#define IsUnicodeHdr(op)    (((op) & 0xC0) == 0x00) /* Bits 8, 7 = 00 */
#define IsByteSeqHdr(op)    (((op) & 0xC0) == 0x40) /* Bits 8, 7 = 01 */
#define IsByteHdr(op)       (((op) & 0xC0) == 0x80) /* Bits 8, 7 = 10 */
#define IsUintHdr(op)       (((op) & 0xC0) == 0xC0) /* Bits 8, 7 = 11 */


#define OBEX_INVALID_UINT32    0xFFFFFFFFu
#define OBEX_INVALID_UINT16    0xFFFFu
#define OBEX_INVALID_UINT8     0xFFu
#define OBEX_EMPTY_BODY_SIZE                4  /* Empty body Hdr size*/

/* OBEX packet Opcodes */
#define OBEX_PUT                0x02
#define OBEX_PUT_FINAL          0x82
#define OBEX_GET                0x03
#define OBEX_GET_FINAL          0x83
#define OBEX_CONNECT_OPCODE     0x80
#define OBEX_SET_PATH           0x85
#define OBEX_ABORT              0xFF
#define OBEX_DISCONNECT         0x81

         


/* Client status macros */
#define obexSetPathCfm( session, status ) \
            obexStatusCfm( session, status, OBEX_SET_PATH_CFM, 0, 0 )
#define obexDeleteCfm( session, status ) \
            obexStatusCfm( session, status, OBEX_DELETE_CFM, 0, 0 )
#define obexGetErrCfm( session, status ) \
            obexStatusCfm( session, status, OBEX_GET_CFM, 0, 0 )
#define obexPutErrCfm( session, status) \
            obexStatusCfm( session, status, OBEX_PUT_CFM, 0, 0 )


#ifdef GOEP_VERSION_2_0

#define obexGetClientRole(c) (c.l2capObex)?obex_l2cap_client:obex_rfcomm_client 
#define obexGetServerRole(c) (c.l2capObex)?obex_l2cap_server:obex_rfcomm_server


#define OBEX_SRM_MASK 0x10
#define OBEX_SRMP_MASK 0x01

#else

#define obexGetClientRole(c) obex_rfcomm_client
#define obexGetServerRole(c) obex_rfcomm_server
#define obexNewSrmPacket(s,m,w) obexNewPacket(s,OBEX_EMPTY_BODY_SIZE,OBEX_GET)
                                

#endif /* GOEP_VERSION_2_0 */



/* OBEX transport information */
typedef struct 
{
    /* TRUE if OBEX is over L2CAP */
    bool    l2capObex;

    /* The L2CAP PSM value if l2capObex is TRUE, otherwise RFCOMM channel */
    union{
        uint8  channel;
        uint16 psm;
    }u;

}ObexChannel;

/* Obex Connection information */

typedef struct
{
    union
    {
        Sink sink;
        struct
        {
            uint8 identifier;
            uint16 connId;
        } l2cap;        
    } u;
}ObexConnId;


/* Role of the OBEX session */
typedef enum
{
    obex_rfcomm_server,
    obex_rfcomm_client,
    obex_l2cap_server,
    obex_l2cap_client

}ObexRole;

/* Internal Message ID */
typedef enum
{
    OBEX_MESSAGE_STATUS ,
    OBEX_MESSAGE_MORE_DATA,
    OBEX_MESSAGE_IND,
    OBEX_MESSAGE_DELETE_TASK,
    OBEX_INT_MESSAGE_END
} ObexIntMessageId;

/* Message for all packet CFM messages */
typedef struct{
    Obex        session;
    ObexStatus  status;
    uint16      hdrsLen;
    Source      hdrs;
} OBEX_MESSAGE_STATUS_T;

/* Message for all packet IND messages */
typedef struct{
    Obex        session;
    bool        final;
    uint16      hdrsLen;
    Source      hdrs;
} OBEX_MESSAGE_IND_T;

/************************************************************************
Task Initialization and Management routines
*************************************************************************/
Task obexConnectRequest( const bdaddr          *addr,
                         ObexChannel          channel,
                        const ObexConnParams  *connParams );
Task obexConnectResponse( ObexConnId            connId,
                          ObexChannel           channel,
                          bool                  accept,
                          const ObexConnParams* connParams);  


void obexDeleteSessionTask( Obex session );
Sink obexGetSink( Obex session );

/***********************************************************************
All Message Handling routines defined in obex_messages.c
*************************************************************************/
void obexCreateSessionCfm(  Obex    session,
                            ObexStatus   status,
                            const bdaddr *addr);
void obexConnectCfm( Obex         session,
                     ObexStatus   status );
void obexDeleteSessionInd( Obex session );
void obexStatusCfm( Obex        session, 
                    ObexStatus  status,
                    ObexMessageId id,
                    Source  src,
                    uint16  srcLen );
void obexCommandInd( Obex        session, 
                     ObexMessageId id,
                     bool          final,
                     Source        src,
                     uint16        srcLen );
void obexAbortInd( Obex session ); 
void obexAuthReqInd( Obex session );
void obexAuthClgInd( Obex session, const uint8* pkt, uint16 pktLen );
void obexAuthRspCfm( Obex session, const uint8* pkt, uint16 pktLen );

/***********************************************************************
All packet and utility routines
*************************************************************************/
uint16 obexNewPacket( Obex session, uint16 size, uint8 opcode);
void obexObjDelete( Obex session );
void obexObjFlush( Obex session, bool out );


/**********************************************************************
All Command and Response routines
***********************************************************************/
void obexPutReq( Obex session, uint8 opcode );
void obexGetReq( Obex session, uint8 opcode );
void obexSendResponse( Obex session, ObexResponse resp );
void obexSendErrorResponse( Obex session, ObexResponse response );
void obexSetPathReq( Obex session, uint8 flags );
void obexDeleteReq( Obex session );
void obexAbortReq( Obex session );
void obexDisconnectReq( Obex session );
void obexSourceEmpty( Obex session );

#ifdef GOEP_VERSION_2_0

uint16 obexNewSrmPacket( Obex session, bool srm, bool srmWait );

#endif /* GOEP_VERSION_2_0 */

#endif /* OBEX_EXTERN_H */


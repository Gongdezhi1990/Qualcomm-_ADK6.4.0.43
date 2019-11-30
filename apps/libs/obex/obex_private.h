/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    obex_private.h
    
DESCRIPTION
	Private internal header file for the OBEX Client library
*/

#ifndef OBEX_INTERNAL_H
#define OBEX_INTERNAL_H

/* RFCOMM Frame size */
#define     OBEX_MAX_RFC_FRAME_SIZE  0

/*! RFCOMM BUffer size is 2K-128. Do not exceed 50% for a single OBEX packet */
#define     OBEX_MAX_PACKET_SIZE    (895)
#define     OBEX_MIN_PACKET_SIZE    (255)
#define     OBEX_MAX_TRANSMIT       0xFFFF
#define     OBEX_DEFAULT_MAX_TRANSMIT  0x0000

/* Size of Packets */
#define OBEX_PKT_HDR_SIZE                   3  /* Packet header size */
#define OBEX_PKT_CONNECT_HDR_SIZE           7  /* Header for Connect */
#define OBEX_PKT_SETPATH_HDR_SIZE           5
#define OBEX_BYTE_HDR_SIZE                  2
#define OBEX_UINT_HDR_SIZE                  5
#define OBEX_SEQ_HDR_SIZE                   3
#define OBEX_MIN_PUT_REQ_SIZE               7 /* Header + empty body */ 
#define OBEX_MIN_GET_REQ_SIZE               8 /* Header + empty name */ 
#define OBEX_TLV_HDR_SIZE                   2
#define OBEX_TOTAL_OPTIONS_SIZE             3
#define OBEX_TOTAL_DIGEST_SIZE              18/* TLV SIZE + OBEX_SIZE_DIGEST*/
#define OBEX_MAX_VAL_SIZE                   20
#define OBEX_REQ_NONCE_TAG                  0x00
#define OBEX_RSP_DIGEST_TAG                 0x00
#define OBEX_REQ_OPTIONS_TAG                0x01
#define OBEX_RSP_USRID_TAG                  0x01
#define OBEX_REQ_REALM_TAG                  0x02
#define OBEX_RSP_NONCE_TAG                  0x02
 

/* Packet Header Opcodes */
/* OBEX Version */
#define OBEX_OBEX_VERSION       0x10

/* The Internal Header IDs */
#define OBEX_CONNECTID_HDR         0xCB
#define OBEX_WHO_HDR               0x4A
#define OBEX_TARGET_HDR            0x46
#define OBEX_AUTH_CHALLENGE        0x4D
#define OBEX_AUTH_RESPONSE         0x4E
#define OBEX_SRM_HDR               0x97
#define OBEX_SRMP_HDR              0x98


/* CL_RFCOMM_SERVER_CONNECT_CFM_T & CL_RFCOMM_CLIENT_CONNECT_CFM_T are same*/
#define  OBEX_SESSION_CFM_T  CL_RFCOMM_CLIENT_CONNECT_CFM_T 

#define obexGetChannel( c ) (c.l2capObex)? c.u.psm:c.u.channel

#ifdef GOEP_VERSION_2_0

#define SET_OBEX_SRM_LOCK(s) (s)->srmLock = TRUE;
#define IsObexL2capChannel( c ) (c.l2capObex)?TRUE:FALSE
#define IsObexSrmEnabled( s ) ( (s->srm == obex_srm_enable) || \
                                (s->srm == obex_srm_enable_wait ) || \
                                (s->srm == obex_srm_enable_flow ) )  
#define IsObexSrmFlowGo(s)    (s->srm == obex_srm_enable )
#define IsObexSrmFlowEnabled(s) ( (s)->srm == obex_srm_enable_flow )
#define IsObexSrmpAllowed( s) ( (s->srm != obex_srm_enable ) && \
                                (s->srm != obex_srm_deactivate ) )
#define IsObexSrmAllowed( s ) ( IsObexL2cap(s) && IsObexReady( s ) )
#define IsObexSrmDeactive(s, srm) ( (IsObexServer(s) || srm == 0 ) &&  \
                                (s)->srm == obex_srm_deactivate)  

#define obexResetSrmParams( s ) { (s)->srm = obex_srm_deactivate; \
                                  (s)->srmLock = 0;}



#else
#define SET_OBEX_SRM_LOCK(s) 
#define IsObexSrmFlowGo(s) FALSE
#define IsObexSrmFlowEnabled(s)  FALSE
#define IsObexL2capChannel( channel ) FALSE
#define IsObexSrmEnabled( s )  FALSE
#define obexHandleSrmSpace( s )


#endif /* GOEP_VERSION_2_0 */

/* Task Handling Macros */
#define OBEX_OBJ_DELETE( session ) ( session->pktLen = 0)

/* State Management Macros */
#define SET_OBEX_IN_SESSION( session ) session->state = obex_session
#define SET_OBEX_IN_CONNECT( session ) session->state = obex_connect
#define SET_OBEX_IN_AUTH( session ) session->state = obex_auth_connect
#define SET_OBEX_IN_IDLE( session ) session->state = obex_idle
#define SET_OBEX_CONNECTED( session ) session->state = obex_connected
#define SET_OBEX_IN_PUT( session ) session->state = obex_put
#define SET_OBEX_IN_GET( session ) session->state = obex_get
#define SET_OBEX_IN_SETPATH( session ) session->state = obex_setpath
#define SET_OBEX_IN_DELETE( session ) session->state = obex_delete
#define SET_OBEX_IN_ABORT( session )  session->state = \
        (session->state == obex_put )? obex_abort_put: obex_abort_get;
#define SET_OBEX_IN_DISCONNECT( session ) session->state = obex_disconnect

#define IsObexInIdle( session ) (session->state == obex_idle)
#define IsObexDisconnected( session ) (session->state == obex_session)
#define IsObexInConnect( session ) (session->state == obex_connect)
#define IsObexConnected( session ) (session->state == obex_connected)
#define IsObexReady( session )     (session->state >= obex_connected)
#define IsObexInPut( session )     (session->state == obex_put)
#define IsObexInGet( session )     (session->state == obex_get)
#define IsObexConnecting( session ) ( (session->state == obex_connect) \
                                 || (session->state == obex_auth_connect)) 
#define IsObexInAuthConnect(session) (session->state == obex_auth_connect)

#define IsObexServer( session ) ((session->role == obex_rfcomm_server) || \
                                 (session->role == obex_l2cap_server) )
#define IsObexClient( session ) ( (session->role == obex_rfcomm_client) || \
                                  (session->role == obex_l2cap_client ) )
#define IsObexL2cap( session ) (( session->role == obex_l2cap_server ) || \
                                ( session->role == obex_l2cap_client ) )

#define IsObexObjExists( session ) ( session->pktLen > 0 )
#define obexSinkSpace( session )  \
                    (SinkClaim( session->sink, 0) - session->pktLen )
#define IsObexFinalCmd( cmd )  (cmd & 0x80)

/* Packet management Macros */
#define OBEX_UINT16_TO_UINT8(dest, src, offset) \
        (((dest)[offset] = (src) >> 8) & ((dest)[offset+1] = (src) & 0xFF))

#define OBEX_UINT8_TO_UINT16(dest, src, offset) \
        (dest= ((src[offset] << 8) |  (src[offset+1] & 0xFF)))  

/* Intermediate Unauthorized response */
#define OBEX_UNAUTHORIZED_RSP       0x41
#define OBEX_SUCCESS                0xA0
#define OBEX_CONTINUE               0x90

typedef enum
{
    obex_srm_deactivate,  /* SRM Mode is not activated */
    obex_srm_disable,     /* waiting for the server or app response */
    obex_srm_disable_wait,/* obex_srm_disable with srmp wait */
    obex_srm_enable,      /* SRM enabled and data flow to remote */
    obex_srm_enable_flow, /* SRM enabled and data flow from remote */
    obex_srm_enable_wait  /* SRM enabled but waiting */ 
} ObexSrmMode;


typedef enum
{
    obex_idle,         /* OBEX IDLE. Waiting to remove the session */
    obex_session,      /* RFCOMM Connect/Disconnect in progress */
    obex_connect,      /* OBEX Connection is in progress */
    obex_auth_connect, /* OBEX Authenticated connect in progress */
    obex_connected,    /* OBEX Connection established */
    obex_get,          /* OBEX GET command is active */
    obex_put,          /* OBEX PUT command is active */
    obex_setpath,      /* OBEX Set path is active - Client only state*/
    obex_delete,       /* OBEX delete operation - Client only state */
    obex_abort_get,    /* Aborting Multipacket OBEX GET- Client only state */
    obex_abort_put,    /* Aborting Multipacket OBEX PUT- Client only state */
    obex_disconnect    /* Disconnecting OBEX - Client only state */ 

}ObexState;


struct __obex
{
    TaskData     task;           /* The Task Data */               
    Task         theApp;         /* The application Task */
    Sink         sink;           /* The Rfcomm Sink */
    uint8        channel;        /* Associated RFCOMM Channel or L2CAP PSM*/
    uint16       maxPktLen;      /* The OBEX max pkt len */
    ObexRole     role;           /* Role of the session */
    ObexState    state;          /* State of OBEX session */
    uint16       pktLen;         /* Length of the outgoing packet */
    uint16       srcUsed;        /* Data in the Source is already used */
    uint16       sizeTargetWho;  /* Size of the Target/Who */
    const uint8* targetWho;      /* Target/Who Header */
    bool         auth;           /* TRUE for authenticated session */
    uint32       connID;         /* Connection Identifier Header */
#ifdef GOEP_VERSION_2_0
    ObexSrmMode srm;             /* Status of srm */
    bool        srmLock;         /* Lock set for the SRM */
#endif /* GOEP_VERSION_2_0 */
};


void obexProfileHandler( Task task, MessageId id, Message message );


/***********************************************************************
    Client handler routines
************************************************************************/
bool obexConnectReq( Obex session, bool flush );
uint16 obexHandleConnectResponse( Obex session, Source source, uint16 pktLen );
uint16 obexHandleGetPutResponse( Obex session, Source source, uint16 pktLen );
void obexHandleAbortResponse( Obex session , ObexStatus status);
void obexHandleDisconnectResponse( Obex session );
uint16 obexHandleResponse( Obex session, Source source, uint16 pktLen );

/***********************************************************************
    Server handler routines
************************************************************************/
uint16 obexHandleCommand( Obex session, Source source, uint16 pktLen );


                           
/***************************************************************************
    OBEX packet handler routines 
***************************************************************************/
bool obexFrameConnectPkt( Obex session, uint8 opcode );
void obexFlushPacket( Obex session, uint8 opcode );
void obexHandleIncomingPacket( Obex session );
bool obexValidateConnectionID(Obex session, Source source, uint16 *len);
void obexSendAuthPacket( Obex session, uint8 hdr, uint16 size, Source src );
uint16 obexSinkSlack( Obex session );
bool obexSinkClaim( Obex session, uint16 len );

/**************************************************************************
    OBEX Session Handler routines
****************************************************************************/
bool obexStoreConnectionID( Obex session, const uint8* pkt, uint16 len );
bool obexValidateSession( Obex session, const uint8* pkt, uint16 len );
bool obexAuthenticateSession( Obex session, const uint8* pkt, uint16* len);
void obexDisconnectSession( Obex session );
void obexHandleSessionCfm( Obex       session,
                           ObexStatus status,
                           const bdaddr* addr,
                           uint16  channel );
#ifdef GOEP_VERSION_2_0

void obexL2capConnectReq( Obex sessionTask,
                          const bdaddr* addr, 
                          uint16 psm );
void obexL2capConnectResp( Obex sessionTask,
                           bool accept,
                           uint16 psm,
                           ObexConnId id );
void obexHandleL2capConnectCfm( Obex session, Message message );
void obexChangeSrmMode( Obex session, ObexSrmMode mode );
void obexHandleSrm( Obex session, uint8 cmd, Source src, uint16 len );
void obexHandleSrmSpace( Obex session );
void obexHandleL2capDisconnect( Obex session, uint8 id );

#else

#define obexHandleL2capDisconnect( s, i )
#define obexResetSrmParams( a ) 
/* Dummy macros to replace l2cap functions in v1.1 */
#define obexL2capConnectResp( a, b, c,d ) return NULL
#define obexL2capConnectReq( a, b, c ) return NULL
#define obexHandleL2capConnectCfm( a, b )
#define obexHandleSrm( s, i, m, l )
#define obexChangeSrmMode(s, m)

#endif /* GOEP_VERSION_2_0 */

                                     

/**************************************************************************
    OBEX Object handler routines
****************************************************************************/
uint16 obexFetchHeader(const uint8* pkt, uint16 len, uint8 opcode);
bool obexAddUint32Header( Obex session, uint8 opcode, uint32 value );
bool obexAddUint8Header( Obex session, uint8 opcode, uint8 value );
uint8 obexGetUint8Header( const uint8* pkt, uint16 len, uint8 opcode );
bool obexAddEmptyHeader( Obex session, uint8 hdr, uint16 size );
bool obexAddUnicodeHeader( Obex session, uint8 hdr, uint16 size, Source src );
bool obexAddSeqHeader( Obex session, uint8 hdr, uint16 size, Source src );
uint32 obexGetUint32Header( const uint8* pkt, uint16 len, uint8 opcode );
uint8* obexGetSeqHeader( const uint8* pkt, uint16* len, uint8 opcode );
const uint8* obexGetTLVHeaderValue( const uint8* pkt,
                                    uint16 *pktLen,
                                    uint8  tag ); 
uint16 obexFrameTLVHeader( uint8* pkt,
                           uint8  tag,
                           uint16 len,
                           const uint8* data);
const uint8* obexGetDigest( const uint8* pkt,
                            uint16 *len,
                            uint8 hdrId );

#endif /* OBEX_INTERNAL_H */


/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc_extern.h
    
DESCRIPTION
    Header file shared between interface and private modules

*/

#ifndef    PBAPC_EXTERN_H_
#define    PBAPC_EXTERN_H_

#include <panic.h>
#include <pbapc.h>

/* Macros for creating messages */
#define MAKE_PBAPC_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T)
#define MAKE_PBAPC_MESSAGE_WITH_LEN(TYPE, LEN) \
    TYPE##_T *message = (TYPE##_T *)PanicUnlessMalloc(sizeof(TYPE##_T) + LEN - 1)

#define handleGoepAuthRspCfm(s,m)  pbapcMsgSendAuthRspCfm( s, (m)->request )

#define pbapcMsgSendPullListcfm pbapcMsgSendPullPhoneBookCfm

#define pbapcMsgSendPullPhonebookErrCfm( p, s ) \
            pbapcMsgSendPullPhoneBookCfm( p, PBAPC_PULL_PHONEBOOK_CFM, s, \
                                          0, 0, 0, 0 )

#define pbapcMsgSendPullvCardErrCfm( p, s ) \
            pbapcMsgSendPullVcardCfm( p, s, 0, 0 )

#define pbapcMsgSendPullvCardListErrCfm pbapcMsgSendPullPhonebookErrCfm

#define pbapcMsgSendSetPbCfm( p, s )  pbapcMsgSendSetPhonebookCfm\
                            ( p, s,  (p)->pb.currRepos, (p)->pb.currPb ) 
       

#define PBAPC_MAX_SRCH_STR_LEN  30
#define PBAPC_NO_PAYLOAD 0

#ifdef PBAPC_LIBRARY_DEBUG
    #include <panic.h>
    #include <stdio.h>
    #define PBAPC_LOG(x)    printf x;
    #define PBAPC_DEBUG(x) {printf x;}
    #define PBAPC_ASSERT(c, x) { if (!(c)) { printf x; Panic();} }
#else
    #define PBAPC_LOG(x)  
    #define PBAPC_DEBUG(x)
    #define PBAPC_ASSERT(c, x)
#endif

#define PBAPC_INVALID_REPOSITORY 0xFF;
                            
/* Provide a reference for all PBAPC lib modules - should not be exposed at API level */
extern PBAPC *Pbapc[MAX_PBAPC_CONNECTIONS];

#define PBAPC_INT_MESSAGE_BASE  0x0
enum
{
    PBAPC_INT_REG_SDP = PBAPC_INT_MESSAGE_BASE,

    PBAPC_INT_CONNECT,
    PBAPC_INT_TASK_DELETE,
    PBAPC_INT_AUTH_CLG,
    PBAPC_INT_AUTH_RESP,
    PBAPC_INT_DISCONNECT,

    PBAPC_INT_GET_PHONEBOOK,
    PBAPC_INT_SET_PHONEBOOK,
    PBAPC_INT_GET_VCARD_LIST,
    PBAPC_INT_GET_VCARD,
    PBAPC_INT_GET_CONTINUE,

    PBAPC_INT_ENDOFLIST
};


/* Internal Message Structures */
typedef struct
{
    bdaddr    bdAddr;
} PBAPC_INT_CONNECT_T;

typedef struct
{
    uint8 nonce[PBAPC_OBEX_SIZE_DIGEST];
} PBAPC_INT_AUTH_CLG_T;

typedef struct
{
    uint8 digest[PBAPC_OBEX_SIZE_DIGEST];
    uint16 sizeUserId;
    uint8 userId[1];
} PBAPC_INT_AUTH_RESP_T;

typedef struct
{
    PbapcPhoneRepository repository;
    PbapcPhoneBook phonebook;
    uint32 filterLo;
    uint32 filterHi;
    PbapcFormatValues format;
    uint16 maxList;
    uint16 listStart;
} PBAPC_INT_GET_PHONEBOOK_T;

typedef struct
{
    bool proceed;
}PBAPC_INT_GET_CONTINUE_T;
 
typedef struct
{  
    PbapcPhoneBook      phonebook;
    PbapcOrderValues    order;
    PbapcSearchValues   srchAttr;
    uint16              maxList;
    uint16              listStart;
    uint8               srchValLen;
    uint8               srchVal[1];
}PBAPC_INT_GET_VCARD_LIST_T;

typedef struct
{
    PbapcPhoneRepository repository;
    PbapcPhoneBook phonebook;
} PBAPC_INT_SET_PHONEBOOK_T;

typedef struct
{
    uint32 entry;
    uint32 filterLo;
    uint32 filterHi;
    PbapcFormatValues format;
} PBAPC_INT_GET_VCARD_T;


Task pbapcCreateTask( Task theAppTask );
Task pbapcGetAppTask( PBAPC *state );
uint8 pbapcGetSupportedRepositories( PBAPC* state );

/* functions to send messages to the app */
void pbapcMsgInitCfm( Task              theAppTask, 
                      uint32            sdpHandle, 
                      PbapcLibStatus    status );

void pbapcMsgSendSetPhonebookCfm( PBAPC *state,
                                  PbapcLibStatus status,
                                  PbapcPhoneRepository repos,
                                  PbapcPhoneBook pb );  

void pbapcMsgSendPullPhoneBookCfm ( PBAPC *state,
                                    MessageId id,
                                    PbapcLibStatus status,
                                    uint16 pbSize,
                                    uint8 newMisscall,
                                    uint16 len,
                                    Source src );

void pbapcMsgSendPullVcardCfm ( PBAPC *state,
                                PbapcLibStatus status,
                                uint16 len,
                                Source src );

void pbapcMsgSendConnectCfm( Task theAppTask,
                             PBAPC *state, 
                             const bdaddr* addr,
                             PbapcLibStatus status, 
                             uint8  repository,
                             uint16 pktSize );

void pbapcMsgSendAuthReqInd( PBAPC *state,
                             const uint8 *nonce,
                             PbapcObexAuthOptions options,
                             uint16 sizeRealm, 
                             const uint8* realm );

void pbapcMsgSendAuthRspCfm( PBAPC *state,
                             const uint8* digest );

void pbapcMsgSendDisconnectCfm( PBAPC *state, PbapcLibStatus status);
 

#endif /* PBAPC_EXTERN_H_ */
 




/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_private.h
DESCRIPTION
    Private Header file for the iAP library. The library declares internal data
    structures, enumerated types, and APIs used internally to the library

*/
#ifndef __IAP2_PRIVATE_H__
#define __IAP2_PRIVATE_H__

#include <stdlib.h>
#include <panic.h>
#include <message.h>
#include "iap2.h"
#include "iap2_debug.h"

/* Macro for allocating message structures */
/* TYPE##_T *NAME = PanicUnlessNew(TYPE##_T) */
/* MESSAGE_MAKE(NAME,TYPE##_T) */
#define MAKE_MESSAGE(NAME, TYPE) \
    TYPE##_T *NAME = PanicUnlessNew(TYPE##_T)
#define MAKE_MESSAGE_WITH_LEN(NAME, TYPE, LEN) \
    TYPE##_T *NAME = (TYPE##_T*)PanicUnlessMalloc(sizeof(TYPE##_T) + ((LEN) ? (LEN) - 1 : 0))
#define MAKE_MESSAGE_WITH_STATUS(NAME, TYPE, STATUS) \
    MAKE_MESSAGE(NAME, TYPE); \
    NAME->status = STATUS;

#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif

#ifndef IAP2_DEFAULT_CHANNEL
#define IAP2_DEFAULT_CHANNEL        RESERVED_DEFAULT_CHANNEL
#endif

#define DEFAULT_SESSION_ID_CTRL     (20)
#define DEFAULT_SESSION_ID_FTP      (21)
#define DEFAULT_SESSION_ID_EA       (22)

#define USB_HOST_COMPONENT_ID       (30)
#define BT_COMPONENT_ID             (31)

#define IAP2_SEQ_NUM_START          (0x00)

typedef enum
{
    IAP2_INTERNAL_CP_INIT_CFM,
    IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM,
    IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM,
    IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM,
    IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM,
    IAP2_INTERNAL_CONNECT_TIMEOUT,
    IAP2_INTERNAL_CONNECT_SDP_SEARCH_REQ,
    IAP2_INTERNAL_LINK_INIT_REQ,
    IAP2_INTERNAL_LINK_SYNC_REQ,
    IAP2_INTERNAL_LINK_SYNC_IND,
    IAP2_INTERNAL_LINK_AUTH_IND,
    IAP2_INTERNAL_LINK_IDENT_REQ,
    IAP2_INTERNAL_LINK_EAK_IND,
    IAP2_INTERNAL_LINK_DETACH_REQ,
    IAP2_INTERNAL_ACK_FLUSH_REQ,
    IAP2_INTERNAL_PACKET_RETRANS_REQ
} Iap2InternalMessageId;

typedef enum
{
    iap2_link_error,                /* 0 */
    iap2_link_connecting_local,     /* 1 */
    iap2_link_connecting_remote,    /* 2 */
    iap2_link_connecting_init,      /* 3 */
    iap2_link_initialised,          /* 4 */
    iap2_link_sync_retry,           /* 5 */
    iap2_link_sync_wait,            /* 6 */
    iap2_link_sync_done,            /* 7 */
    iap2_link_auth_cert_req,        /* 8 */
    iap2_link_auth_cert_done,       /* 9 */
    iap2_link_auth_challenge_req,   /* 10 */
    iap2_link_auth_challenge_wait,  /* 11 */
    iap2_link_auth_challenge_done,  /* 12 */
    iap2_link_ident_start,          /* 13 */
    iap2_link_ident_wait,           /* 14 */
    iap2_link_ready,                /* 15 */
    iap2_link_detaching,            /* 16 */
    iap2_link_detached,             /* 17 */
    iap2_link_disconnecting         /* 18 */
} iap2_link_state_t;

typedef enum
{
    iap2_session_type_ctrl,
    iap2_session_type_ftp,
    iap2_session_type_ea,
    iap2_session_type_none
} iap2_session_type_t;

typedef enum
{
    iap2_ctrl_bit_slp = 1 << 3,
    iap2_ctrl_bit_rst = 1 << 4,
    iap2_ctrl_bit_eak = 1 << 5,
    iap2_ctrl_bit_ack = 1 << 6,
    iap2_ctrl_bit_syn = 1 << 7
} iap2_ctrl_bit_t;

typedef struct _iap2_packet
{
    TaskData task;

    unsigned seq_num:8;
    unsigned session_type:2;
    unsigned retrans_count:5;
    unsigned static_payload:1;

    union
    {
        uint16 ctrl_message;
        uint16 ea_session;
    } id;

    uint16 size_payload;
    uint8 *payload;

    struct _iap2_packet *next;
} iap2_packet;

struct _iap2_link
{
    TaskData task;
    bdaddr remote_addr;
    Sink sink;

    iap2_link_param param;

    iap2_transport_t transport:8;
    iap2_link_state_t state:8;

    unsigned last_sent_seq:8;
    unsigned last_sent_ack:8;
    unsigned last_received_seq:8;

    unsigned session_id_ctrl:8;
    unsigned session_id_ftp:8;
    unsigned session_id_ea:8;

    iap2_packet *tx_queue;

    struct _iap2_link *next;
};

typedef struct
{
    TaskData task;
    Task client_task;
    unsigned retry:8;
    unsigned checksum:8;
} iap2_cp_data;

typedef struct
{
    TaskData                task;
    Task                    app_task;

    iap2_cp_data            cp;

    iap2_config             *config;
    
    uint8                   server_channel;

    iap2_link               *links;
} iap2_lib_data;

typedef struct
{
    iap2_status_t status;
} IAP2_INTERNAL_CP_INIT_CFM_T;

typedef struct
{
    iap2_status_t status;
    uint16 auth_cert_len;
} IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM_T;

typedef struct
{
    iap2_status_t status;
    uint8 checksum;
} IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM_T;

typedef struct
{
    iap2_status_t status;
    uint16 challenge_rsp_len;
} IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM_T;

typedef IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM_T IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM_T;

typedef struct
{
    uint16 size_payload;
    uint8 payload[1];
} IAP2_INTERNAL_LINK_SYNC_IND_T;

typedef IAP2_INTERNAL_LINK_SYNC_IND_T IAP2_INTERNAL_LINK_EAK_IND_T;
typedef IAP2_CONTROL_MESSAGE_IND_T IAP2_INTERNAL_LINK_AUTH_IND_T;

typedef struct
{
    iap2_link *link;
} IAP2_INTERNAL_PACKET_RETRANS_REQ_T;

extern iap2_lib_data *iap2_lib;

/* Message handler */
void iap2LibHandler(Task task, MessageId id, Message message);
void iap2LinkHandler(Task task, MessageId id, Message message);
void iap2PacketHandler(Task task, MessageId id, Message message);

/* Common function */
uint8 iap2Checksum(uint8 *data, uint16 size);

#endif /* __IAP2_PRIVATE_H__ */

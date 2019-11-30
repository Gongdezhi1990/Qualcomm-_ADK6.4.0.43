/****************************************************************************
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_private.h    
DESCRIPTION
    Header file for the HID profile library containing private members
*/

#ifndef HID_PRIVATE_H_
#define HID_PRIVATE_H_

#include "hid.h"

#include <connection.h>
#include <message.h>
#include <app/message/system_message.h>
#include <stdlib.h>

#ifdef HID_DEBUG_LIB
#include <stdio.h>
#define HID_DEBUG(x) {printf x; Panic();}
#define HID_PRINT(x) {printf x;}
#else
#define HID_DEBUG(x)
#define HID_PRINT(x)
#endif

#ifdef HID_DEBUG_LIB
#define HID_ASSERT(x) {if (!(x)) Panic();}
#else
#define HID_ASSERT(x) ((void)0)
#endif

/* Macros for creating messages */
#define MAKE_HID_MESSAGE(TYPE) \
    TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_HID_MESSAGE_OR_NULL(TYPE) \
    TYPE##_T *message = (TYPE##_T *)malloc(sizeof(TYPE##_T));
#define MAKE_HID_MESSAGE_WITH_LEN(TYPE, LEN) \
    TYPE##_T *message = (TYPE##_T *)PanicUnlessMalloc(sizeof(TYPE##_T) + (LEN) - 1);
#define MAKE_HID_MESSAGE_WITH_LEN_OR_NULL(TYPE, LEN) \
    TYPE##_T *message = (TYPE##_T *)malloc(sizeof(TYPE##_T) + (LEN) - 1);

/* HID L2CAP PSMs */
#define HID_PSM_CONTROL     (0x0011)
#define HID_PSM_INTERRUPT   (0x0013)

/* HID Profile Library private messages */
#define HID_INTERNAL_MESSAGE_BASE (0x0)

/* Timeout in seconds between control channel connected and interrupt channel connection */
#define HID_CONNECT_TIMEOUT		(10)

/* Timeout in seconds to complete L2CAP connection */
#define HID_L2CAP_TIMEOUT		(10)

/* Timeout in seconds to complete HID control request */
#define HID_REQUEST_TIMEOUT		(20)

/* Timeout in seconds to complete remote disconnection */
#define HID_DISCONNECT_TIMEOUT	(5)

/* Packet types */
#define HID_HANDSHAKE		0
#define HID_CONTROL			1
#define HID_GET_REPORT		4
#define HID_SET_REPORT		5
#define HID_GET_PROTOCOL	6
#define HID_SET_PROTOCOL	7
#define HID_GET_IDLE		8
#define HID_SET_IDLE		9
#define HID_DATA			10
#define HID_DATC			11

/* Maximum length of a GET_REPORT header */
#define HID_GET_REPORT_HEADER_MAX_LEN   (4)

/* Message ID base for IND & CFM messages */
#define HID_IND_BASE		(HID_GET_REPORT_IND - HID_GET_REPORT)
#define HID_CFM_BASE		(HID_GET_REPORT_CFM - HID_GET_REPORT)


/* Enum for profile handler messages */
enum
{
    /* Initialisation */
    HID_INTERNAL_INIT_REQ = HID_INTERNAL_MESSAGE_BASE,
    HID_INTERNAL_INIT_CFM,
    HID_INTERNAL_CONNECT_CONTROL_IND,
    HID_INTERNAL_CONNECT_INTERRUPT_IND,
    HID_INTERNAL_CONNECT_REQ,
    HID_INTERNAL_CONNECT_RES,
    HID_INTERNAL_DISCONNECT_REQ,
    HID_INTERNAL_CONNECT_TIMEOUT_IND,
    HID_INTERNAL_DISCONNECT_TIMEOUT_IND,    
#ifdef HID_DEVICE
    HID_INTERNAL_GET_IDLE_RES,
    HID_INTERNAL_SET_IDLE_RES,
    HID_INTERNAL_GET_PROTOCOL_RES,
    HID_INTERNAL_SET_PROTOCOL_RES,
    HID_INTERNAL_GET_REPORT_RES,
    HID_INTERNAL_SET_REPORT_RES,
#endif
    HID_INTERNAL_REQUEST_TIMEOUT_IND,
    HID_INTERNAL_CONTROL_REQ,
    HID_INTERNAL_CHECK_CONTROL_REQ
};

typedef struct
{
    const hid_config *config;
} HID_INTERNAL_INIT_REQ_T;

typedef struct
{
    hid_init_status status;
} HID_INTERNAL_INIT_CFM_T;

typedef struct
{
    Task app_task;
    bdaddr bd_addr;
    uint16 hid_conftab_length;
    const uint16 *hid_conftab;
} HID_INTERNAL_CONNECT_REQ_T;

typedef struct
{
    Task app_task;
    bool accept;
    uint16 hid_conftab_length;
    const uint16 *hid_conftab;
} HID_INTERNAL_CONNECT_RES_T;

typedef struct
{
    bdaddr bd_addr;
    uint16 psm;
    uint8 identifier;
    uint16 connection_id;
} HID_INTERNAL_CONNECT_IND_T;

typedef struct
{
    HID *hid;
    hid_control_op op;
} HID_INTERNAL_CONTROL_REQ_T; 

#ifdef HID_DEVICE
typedef struct
{
    HID *hid;
    hid_status status;
    uint8 idle_rate;
} HID_INTERNAL_GET_IDLE_RES_T;

typedef struct
{
    HID *hid;
    hid_status status;
} HID_INTERNAL_SET_IDLE_RES_T;

typedef struct
{
    HID *hid;
    hid_status status;
    hid_protocol protocol;
} HID_INTERNAL_GET_PROTOCOL_RES_T;

typedef struct
{
    HID *hid;
    hid_status status;
} HID_INTERNAL_SET_PROTOCOL_RES_T;

typedef struct
{
    HID *hid;
    hid_status status;
    hid_report_type report_type;
    uint8 report_id;
    uint16 report_length;
    uint8 report_data[1];
} HID_INTERNAL_GET_REPORT_RES_T;

typedef struct
{
    HID *hid;
    hid_status status;
} HID_INTERNAL_SET_REPORT_RES_T;
#endif /* HID_DEVICE */

/* HID states */
typedef enum 
{
    hidIdle,                        /*!< The HID is initialised and ready for connection.*/
    hidConnectingLocal,             /*!< The HID is currently connecting (locally initiated).*/
    hidConnectingRemote,            /*!< The HID is currently connecting.*/
    hidConnected,                   /*!< The HID is connected.*/
    hidDisconnectingLocal,          /*!< The HID is disconnecting, initiated locally. */
    hidDisconnectingRemote,         /*!< The HID is disconnecting, initiated remotely. */
    hidDisconnectingFailed,         /*!< The HID is disconnecting, initiated due to connection failure. */
    hidDisconnected                 /*!< The HID is disconnected. */
} hidState;

/* HID Packet reassembly status */
typedef enum
{
    hidPacketStatusCompleted,
    hidPacketStatusPending,
    hidPacketStatusFailed,
    hidPacketStatusBusy
} hidPacketStatus;

/* HID connection states */
typedef enum
{
    hidConIdle,
    hidConConnecting,
    hidConConnected,
    hidConDisconnecting,
    hidConDisconnected
} hidConnectionState;

/* HID connection structure */
typedef struct
{
    hidConnectionState  state;
    uint8              identifer;
    union
    {
        uint16          id;
        Sink            sink;
    } con;
    uint16              remote_mtu;
} hidConnection;

#define MAX_NUM_CONNECTIONS (2)
#define HID_CON_CONTROL     (0)
#define HID_CON_INTERRUPT   (1)

/* HID instance structure */
struct __HID_LIB;
struct __HID
{
    TaskData                     task;
    Task                         app_task;
    hidState                     state:4;
    hid_disconnect_status        disconnect_status:4;
    hid_connect_status           connect_status:4;
    hidConnection                connection[MAX_NUM_CONNECTIONS];
    uint16                       hid_conftab_length;
    const uint16                      *hid_conftab;
    uint16                       packet_pending;
    uint8                       *packet_data;
    uint16                       packet_size;
    bdaddr                       remote_addr;
    struct __HID                *next;
    struct __HID                *prev;
    struct __HID_LIB            *hid_lib;
};

/* HID library structure */
struct __HID_LIB
{
    TaskData                task;
    Task                    app_task;
    int                     interrupt_psm:1;
    int                     control_psm:1;
    int                     sdp_record:1;
    struct __HID           *list;
};

#endif /* HID_PRIVATE_H_ */

/****************************************************************************
Copyright (c) 2011 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_private.h
    
DESCRIPTION
    This file contains the type definitions and function prototypes for the 
    GATT Library
*/

#ifndef GATT_PRIVATE_H_
#define GATT_PRIVATE_H_

#include <app/message/system_message.h>
#include <app/bluestack/types.h>
#include <bdaddr.h>
#include <app/bluestack/hci.h>
#include <app/bluestack/l2cap_prim.h>
#include <app/bluestack/att_prim.h>

#include <connection.h>
#include <region.h>
#include <message.h>
#include <panic.h>
#include "gatt.h"
#include "gatt_config.h"


/* macro for allocating message structures */
#define MAKE_GATT_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T);
#define MAKE_ATT_PRIM(TYPE) MESSAGE_MAKE(prim,TYPE##_T); prim->type = TYPE;

/* Assumes message struct with
 *    uint16 size_value;
 *    uint8 value[1];
 */
#define MAKE_GATT_MESSAGE_WITH_LEN_U8(TYPE, LEN)                           \
    TYPE##_T *message = (TYPE##_T*)PanicUnlessMalloc(sizeof(TYPE##_T) + \
                                                     ((LEN) ? (LEN) - 1 : 0))

#define MAKE_GATT_MESSAGE_WITH_VALUE(TYPE, SIZE, VALUE) \
        MAKE_GATT_MESSAGE_WITH_LEN_U8(TYPE, SIZE);          \
        memmove(message->value, (VALUE), (SIZE));           \
        message->size_value = (SIZE)
        
/* Assumes message struct with
 *    uint16 size_value;
 *    uint16 value[1];
 */
#define MAKE_GATT_MESSAGE_WITH_LEN_U16(TYPE, LEN)                           \
        TYPE##_T *message = (TYPE##_T*)PanicUnlessMalloc( \
                        sizeof(TYPE##_T) + \
                        sizeof(uint16) * ((LEN) ? (LEN) - 1 : 0))

/* macro for GATT Library debug lib printfs */
#ifdef GATT_DEBUG_LIB
#include <panic.h>
#include <stdio.h>
#define GATT_DEBUG_INFO(x)  {printf x;}
#define GATT_DEBUG(x)       {printf x;  Panic();}
#else
#define GATT_DEBUG_INFO(x)
#define GATT_DEBUG(x)
#endif

/* 
 * Internal Library messages 
 */

#define MAX_ATT_CONNECTIONS     (6)
#define GATT_DEFAULT_MTU        (23)
#define GATT_HEADER_BYTES       (3)

enum
{
    GATT_INTERNAL_CONNECT_REQ  = 0,
    GATT_INTERNAL_DISCONNECT_REQ,
    GATT_INTERNAL_EXCHANGE_MTU_REQ,
    GATT_INTERNAL_DISCOVER_ALL_PRIMARY_SERVICES_REQ,
    GATT_INTERNAL_DISCOVER_PRIMARY_SERVICE_REQ,
    GATT_INTERNAL_FIND_INCLUDES_REQ,
    GATT_INTERNAL_DISCOVER_ALL_CHARACTERISTICS_REQ,
    GATT_INTERNAL_DISCOVER_ALL_DESCRIPTORS_REQ,
    GATT_INTERNAL_READ_REQ,
    GATT_INTERNAL_READ_BY_UUID_REQ,
    GATT_INTERNAL_READ_LONG_REQ,
    GATT_INTERNAL_READ_MULTIPLE_REQ,
    GATT_INTERNAL_WRITE_REQ,
    GATT_INTERNAL_WRITE_LONG_REQ,
    GATT_INTERNAL_WRITE_RELIABLE_REQ,
    GATT_INTERNAL_EXECUTE_WRITE_REQ,
    GATT_INTERNAL_ACCESS_RES,
    GATT_INTERNAL_HANDLE_VALUE_REQ,
    GATT_INTERNAL_HANDLE_VALUE_NTF,
    GATT_INTERNAL_DELETE_CONN,

    /* Library message limit */
    GATT_INTERNAL_MESSAGE_TOP
};


typedef enum
{
    gatt_state_uninitialised        = 0x00,
    gatt_state_initialising         = 0x01,
    gatt_state_gatt_caching_init    = 0x02,
    gatt_state_gatt_caching_cont    = 0x03,
    gatt_state_initialised          = 0x04
} gatt_states_t;

/* Scenarios must be non-zero, max scenario id = 31 */
typedef enum
{
    gatt_ms_none,    
    gatt_ms_connect_req,
    gatt_ms_connect_rsp,
    gatt_ms_ble_connect_rsp,
    gatt_ms_exchange_mtu,
    gatt_ms_discover_all_primary_services,
    gatt_ms_discover_primary_service,
    gatt_ms_discover_all_bredr_services,
    gatt_ms_discover_bredr_service,
    gatt_ms_find_includes,
    gatt_ms_discover_all_characteristics,
    gatt_ms_discover_all_descriptors,
    gatt_ms_read,
    gatt_ms_read_by_uuid,
    gatt_ms_read_long,
    gatt_ms_read_multiple,
    gatt_ms_write_without_response,
    gatt_ms_signed_write_without_response,
    gatt_ms_write_characteristic_value,
    gatt_ms_write_long_characteristic_value,
    gatt_ms_prepare_write,
    gatt_ms_execute_write,
    gatt_ms_notification_req,
    gatt_ms_indication_req,
    gatt_ms_access_ind_queued_write,
    gatt_ms_last
} gatt_msg_scen_t;

/*
 * GATT Task Data Structure 
 */

typedef struct
{
    Task                    conn_req_task;
} gatt_connect_state;


typedef struct
{
    uint16                  size;
    uint16                  *ptr;
} database_t;

/*
 * Temporary data stored during an ongoing procedure
 */
typedef struct 
{
    gatt_uuid_type_t uuid_type;
    gatt_uuid_t uuid[4];
} gatt_discover_primary_service_t;

typedef struct
{
    bdaddr addr;
} gatt_discover_all_bredr_services_t;

typedef struct
{
    bdaddr addr;
} gatt_discover_bredr_service_t;

typedef struct 
{
    uint16 start;
    uint16 end;
} gatt_find_includes_t;

typedef struct 
{
    uint16 start;
    uint16 end;
} gatt_discover_all_characteristics_t;

typedef struct 
{
    uint16 start;
    uint16 end;
} gatt_discover_all_descriptors_t;

typedef struct 
{
    uint16 handle;
} gatt_read_t;

typedef struct 
{
    uint16 end;
    gatt_uuid_type_t uuid_type;
    gatt_uuid_t uuid[4];
} gatt_read_by_uuid_t;

typedef struct 
{
    uint16 handle;
    uint16 offs;
} gatt_read_long_t;

typedef struct 
{
    uint16 handle;
    uint16 offs;
    uint16 size_value;
    uint8 *value;
} gatt_write_reliable_t;

typedef struct
{
    uint16 handle;
    uint16 notif_ind_flags;
} gatt_notif_ind_t;

/* Data associated with GATT each connection - gets initialised to 0 
 * And reset when the scenario is unlocked.
 */
typedef struct
{
    Task                        app;    /* Application lock task */
    uint16                      scenario;
    void                        *stash; /* pending message for the app */
    union 
    {
#if (GATT_FEATURES & GATT_DISC_PRIMARY_SERVICE)
        gatt_discover_primary_service_t discover_primary_service;
        gatt_discover_all_bredr_services_t discover_all_bredr_services;
        gatt_discover_bredr_service_t discover_bredr_service;
#endif
        gatt_find_includes_t find_includes;
        gatt_discover_all_characteristics_t discover_all_characteristics;
        gatt_discover_all_descriptors_t discover_all_descriptors;
        gatt_read_t read;
#if (GATT_FEATURES & GATT_READ_BY_UUID)
        gatt_read_by_uuid_t read_by_uuid;
#endif
        gatt_read_long_t read_long;
        gatt_write_reliable_t write_reliable;
        gatt_notif_ind_t notif_ind;
    } req;
} conn_data_t;

typedef struct
{
    uint16                  cid;
    Task                    task;
    unsigned                closing:1;
    unsigned                mtu_exchanged:1;
    unsigned                bredr:1;            /* ATT bearer is BR/EDR */
    unsigned                mtu:10;
    unsigned                change_aware:1;
    unsigned                robust_caching_enabled:1; /* Can probably turn into its own bitfield once more CSF are added. */
    unsigned                serv_chg_cccd_enable:1;
    
    conn_data_t             data;
} cid_map_t;

typedef union _gatt_state_u { 
    /* Used during initialisation */
    database_t                  database;

    /* Used once initialised */
    cid_map_t              cid_map[MAX_ATT_CONNECTIONS];
} gatt_state_u;

typedef struct
{
    TaskData                        task;
    Task                            theAppTask;
    Task                            outgoing_conn_req_task;
    Task                            incoming_conn_ind_task;
    gatt_states_t                   state:3;
    unsigned                        connect_count:5;
    gatt_state_u                    u;
    uint16                          flags;
    uint16                          service_changed_local_handle;
} gattState;

/*
 * Macros for stash handling
 */
/* create a stash pointer */
#define STASH(conn, stash, TYPE)                                        \
    GATT_##TYPE##_CFM_T *(stash) = (GATT_##TYPE##_CFM_T*)((conn)->data.stash)
/* make a new stash */
#define MAKE_STASH(conn, stash, TYPE)                            \
    (conn)->data.stash = (stash) =                               \
        (GATT_##TYPE##_CFM_T*)PanicNull(calloc(1, sizeof(GATT_##TYPE##_CFM_T)))
#define MAKE_STASH_WITH_LEN(conn, stash, TYPE, len)              \
    (conn)->data.stash = (stash) =                               \
        (GATT_##TYPE##_CFM_T*)PanicNull(calloc(1, sizeof(GATT_##TYPE##_CFM_T) + len))

/* Maximum amount of data in read blob response */
#define MAX_BLOB_SIZE(mtu)      ((mtu) - 1)

/* Maximum amount of data in prepare write request */
#define MAX_PREPARE_SIZE(mtu)   ((mtu) - 5)

/* Maximum Queued Write Access Indication */
#define MAX_ACCESS_IND_LEN  (80)

/* Internal storage sizes for UUIDs */
#define GATT_UUID16_SIZE        (1 * sizeof(uint32))
#define GATT_UUID128_SIZE       (4 * sizeof(uint32))
#define GATT_UUID_SIZE          GATT_UUID128_SIZE

/* Structure to access common values in internal messages.
   Where used, this structure shall be at the beginning of 
   the internal message structure */
typedef struct
{
    Task task;          /* Client task */
    uint16 cid;         /* Remote cid */
} gatt_internal_t;

typedef struct
{
    Task                            theAppTask;
    typed_bdaddr                    taddr;
    gatt_connection_type            connection;
    bool                            connection_timeout;
} GATT_INTERNAL_CONNECT_REQ_T;

typedef struct
{
    uint16                          cid;
} GATT_INTERNAL_DISCONNECT_REQ_T;


typedef struct
{
    gatt_internal_t                 common;
    uint16                          mtu;
} GATT_INTERNAL_EXCHANGE_MTU_REQ_T;

typedef struct
{
    uint16                          cid;
    uint16                          mtu;
} GATT_INTERNAL_EXCHANGE_MTU_RES_T;

typedef struct 
{
    gatt_internal_t                 common;
} GATT_INTERNAL_DISCOVER_ALL_PRIMARY_SERVICES_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    gatt_uuid_type_t                uuid_type;
    gatt_uuid_t                     uuid[4];
} GATT_INTERNAL_DISCOVER_PRIMARY_SERVICE_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    uint16                          start;
    uint16                          end;
} GATT_INTERNAL_FIND_INCLUDES_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    uint16                          start;
    uint16                          end;
} GATT_INTERNAL_DISCOVER_ALL_CHARACTERISTICS_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    uint16                          start;
    uint16                          end;
} GATT_INTERNAL_DISCOVER_ALL_DESCRIPTORS_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    uint16                          handle;
} GATT_INTERNAL_READ_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    uint16                          start;
    uint16                          end;
    gatt_uuid_type_t                uuid_type;
    gatt_uuid_t                     uuid[4];
} GATT_INTERNAL_READ_BY_UUID_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    uint16                          handle;
} GATT_INTERNAL_READ_LONG_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    uint16                          num_handles;
    uint16                          handles[1];
} GATT_INTERNAL_READ_MULTIPLE_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    uint16                          id;
    uint16                          handle;
    uint16                          offs;
    uint16                          size_value;
    uint8                           value[1];
} GATT_INTERNAL_WRITE_REQ_T;

typedef struct
{
    gatt_internal_t                 common;
    uint8                           flags;
} GATT_INTERNAL_EXECUTE_WRITE_REQ_T;

typedef struct
{
    uint16                          cid;
    uint16                          handle;
    uint16                          result;
    uint16                          size_value;
    uint8                           value[1];
} GATT_INTERNAL_ACCESS_RES_T;

typedef struct
{
    Task                            theAppTask;
    uint16                          cid;
    uint16                          handle;
    uint16                          flags;
    uint16                          size_value;
    uint8                           value[1];
} GATT_INTERNAL_HANDLE_VALUE_REQ_T;

typedef GATT_INTERNAL_HANDLE_VALUE_REQ_T GATT_INTERNAL_HANDLE_VALUE_NTF_T;

typedef struct
{
    Task                            theAppTask;
    uint16                          cid;
    gatt_status_t                   status;
} GATT_INTERNAL_DELETE_CONN_T;

/* gatt_message_handler.c */
void gattMessageHandler(Task task, MessageId id, Message message);

/* gatt_init.c */
Task gattGetTask(void);
Task gattGetAppTask(void);
void gattInitCidTaskMap(void);
bool gattMaxConnections(void);
cid_map_t *gattFindConn(uint16 cid);
cid_map_t *gattFindConnOrPanic(uint16 cid);
cid_map_t *gattFindConnAddr(const bdaddr *addr);
bool gattCidIsValid(uint16 cid);
cid_map_t *gattAddCid(uint16 cid, Task task);
void gattDeleteCid(uint16 cid);
conn_data_t *gattGetCidData(uint16 cid);
bool gattLockCid(uint16 cid, gatt_msg_scen_t scenario);
void gattUnlockCid(uint16 cid);
Task gattGetCidMappedTask(uint16 cid);
bool gattMtuExchanged(uint16 cid);
void gattSetMtuExchanged(uint16 cid);
void gattHandleAttRegisterCfm(gattState *theGatt, const ATT_REGISTER_CFM_T  *cfm);
void gattHandleAttAddDbCfm(gattState *theGatt, const ATT_ADD_DB_CFM_T *cfm);
void gattInitGattCaching(const ATT_READ_BY_TYPE_CFM_T *m);
void gattFindServChanged(const ATT_READ_BY_TYPE_CFM_T *m);

/* gatt_connect.c */
void gattHandleInternalConnectReq(
    gattState *theGatt,
    const GATT_INTERNAL_CONNECT_REQ_T *req
    );
void gattHandleAttConnectCfm(gattState *theGatt, const ATT_CONNECT_CFM_T *cfm);

/* gatt_disconnect.c */
void gattHandleInternalDisconnectReq(const GATT_INTERNAL_DISCONNECT_REQ_T *req);
void gattHandleAttDisconnectInd(const ATT_DISCONNECT_IND_T *ind);
void gattHandleAttConnectInd(const ATT_CONNECT_IND_T *ind);
void gattHandleInternalDeleteConn(const GATT_INTERNAL_DELETE_CONN_T *msg);

/* gatt_access.c */
void gattHandleInternalAccessRes(const GATT_INTERNAL_ACCESS_RES_T *res);
void gattHandleAttAccessInd(const ATT_ACCESS_IND_T *ind);

/* gatt_notification_indication.c */
void gattHandleInternalHandleValueReq(const GATT_INTERNAL_HANDLE_VALUE_REQ_T *req);
void gattHandleInternalHandleValueNtf(const GATT_INTERNAL_HANDLE_VALUE_REQ_T *req);
void gattHandleAttHandleValueNtfCfm(const ATT_HANDLE_VALUE_NTF_CFM_T *cfm);
void gattHandleAttHandleValueCfm(const ATT_HANDLE_VALUE_CFM_T *cfm);
void gattHandleAttHandleValueInd(const ATT_HANDLE_VALUE_IND_T *ind);

/* gatt_common.c */
void gatt_get_att_uuid(
    gatt_uuid_t *uuid,
    gatt_uuid_type_t *uuid_type,
    uint16 size_value,
    const uint8 *value);
void gatt_get_sdp_uuid(
    gatt_uuid_t *uuid,
    gatt_uuid_type_t *uuid_type,
    Region *val);
uint8 gatt_read8(uint8 **data);
uint16 gatt_read16(uint8 **data);

/* gatt_downstream.c */
void gatt_exchange_mtu_req(
    uint16 cid,
    uint16 mtu);
void gatt_exchange_mtu_rsp(
    uint16 cid,
    uint16 mtu);
void gatt_read_by_group_type_req(
    uint16 cid,
    uint16 start,
    uint16 end,
    gatt_uuid_type_t uuid_type,
    const gatt_uuid_t *uuid);
void gatt_find_by_type_value_req(
    uint16 cid,
    uint16 start,
    uint16 end,
    uint16 type,
    gatt_uuid_type_t uuid_type,
    const gatt_uuid_t *uuid);
void gatt_read_by_type_req(
    uint16 cid,
    uint16 start,
    uint16 end,
    gatt_uuid_type_t uuid_type,
    const gatt_uuid_t *uuid);
void gatt_read_req(
    uint16 cid,
    uint16 handle);
void gatt_write_req(
    uint16 cid,
    uint16 handle,
    uint16 flags,
    uint16 size_value,
    const uint8 *value);
void gatt_write_cmd(
    uint16 cid,
    uint16 handle,
    uint16 size_value,
    const uint8 *value);
void gatt_prepare_write_req(
    uint16 cid,
    uint16 handle,
    uint16 offs,
    uint16 size_value,
    const uint8 *value);
void gatt_execute_write_req(
    uint16 cid,
    uint16 flags);
void gatt_find_info_req(
    uint16 cid,
    uint16 start,
    uint16 end);
void gatt_read_blob_req(
    uint16 cid,
    uint16 offs,
    uint16 handle);
void gatt_read_multi_req(
    uint16 cid,
    uint16 size_handles,
    const uint16 *handles);

/* gatt_message.c */
bool gatt_message_prepare(
    cid_map_t *conn,
    bool *more, bool *send,
    att_result_t result,
    bool final,
    bool *more_to_come,
    uint16 id);
void gatt_message_send(
    cid_map_t *conn,
    uint16 id);
gatt_status_t gatt_message_status(
    att_result_t result);
void gattSetConnState(
    cid_map_t *conn,
    Task task,
    gatt_msg_scen_t sm);

/* gatt_server_config.c */
void gattHandleInternalExchangeMtuReq(
    const GATT_INTERNAL_EXCHANGE_MTU_REQ_T *m);
void gattHandleAttExchangeMtuCfm(
    const ATT_EXCHANGE_MTU_CFM_T *m);
void gattHandleAttExchangeMtuInd(
    gattState *theGatt, 
    const ATT_EXCHANGE_MTU_IND_T *m);

/* gatt_primary_discovery.c */
void gattHandleClSdpServiceSearchAttributeCfm(
    const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *m);
void gattHandleInternalDiscoverAllPrimaryServicesReq(
    const GATT_INTERNAL_DISCOVER_ALL_PRIMARY_SERVICES_REQ_T *m);
void gattHandleAttReadByGroupTypeCfm(
    const ATT_READ_BY_GROUP_TYPE_CFM_T *m);
void gattHandleInternalDiscoverPrimaryServiceReq(
    const GATT_INTERNAL_DISCOVER_PRIMARY_SERVICE_REQ_T *m);
void gattHandleAttFindByTypeValueCfm(
    const ATT_FIND_BY_TYPE_VALUE_CFM_T *m);

/* gatt_relationship_discovery.c */
void gattHandleInternalFindIncludesReq(
    const GATT_INTERNAL_FIND_INCLUDES_REQ_T *m);
void gattHandleAttReadByTypeCfmInclude(
    const ATT_READ_BY_TYPE_CFM_T *m);
void gattHandleAttReadCfmInclude(
    const ATT_READ_CFM_T *m);

/* gatt_characteristic_discovery.c */
void gattHandleGattDiscoverAllCharacteristicsReq(
    const GATT_INTERNAL_DISCOVER_ALL_CHARACTERISTICS_REQ_T *m);
void gattHandleAttReadByTypeCfmCharacteristic(
    const ATT_READ_BY_TYPE_CFM_T *m);

/* gatt_descriptor_discovery.c */
void gattHandleInternalDiscoverAllDescriptorsReq(
    const GATT_INTERNAL_DISCOVER_ALL_DESCRIPTORS_REQ_T *m);
void gattHandleAttFindInfoCfm(
    const ATT_FIND_INFO_CFM_T *m);

/* gatt_characteristic_read.c */
void gattHandleInternalReadReq(
    const GATT_INTERNAL_READ_REQ_T *m);
void gattHandleAttReadCfmRead(
    const ATT_READ_CFM_T *m);

void gattHandleInternalReadByUuidReq(
    const GATT_INTERNAL_READ_BY_UUID_REQ_T *m);
void gattHandleAttReadByTypeCfmRead(
    const ATT_READ_BY_TYPE_CFM_T *m);

void gattHandleInternalReadLongReq(
    const GATT_INTERNAL_READ_LONG_REQ_T *m);
void gattHandleAttReadBlobCfm(
    const ATT_READ_BLOB_CFM_T *m);
    
void gattHandleInternalReadMultipleReq(
    const GATT_INTERNAL_READ_MULTIPLE_REQ_T *m);
void gattHandleAttReadMultiCfm(
    const ATT_READ_MULTI_CFM_T *m);

/* gatt_characteristic_write.c */
void gattHandleInternalWriteReq(
    const GATT_INTERNAL_WRITE_REQ_T *m);
void gattHandleAttWriteCfm(
    const ATT_WRITE_CFM_T *m);
void gattHandleAttWriteCmdCfm(
    const ATT_WRITE_CMD_CFM_T *m);
void gattHandleAttPrepareWriteCfm(
    const ATT_PREPARE_WRITE_CFM_T *m);
void gattHandleAttExecuteWriteCfm(
    const ATT_EXECUTE_WRITE_CFM_T *m);
void gattHandleInternalExecuteWriteReq(
    const GATT_INTERNAL_EXECUTE_WRITE_REQ_T *m);
    
/* gatt_caching.c */
void gattHandleAttChangeAwareInd(const ATT_CHANGE_AWARE_IND_T *ind);
void gattNewConnectionGattCaching(gattState *theGatt, typed_bdaddr taddr, cid_map_t *conn);


#endif  /* GATT_PRIVATE_H_ */

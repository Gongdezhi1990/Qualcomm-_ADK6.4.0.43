/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

*/

#ifndef GATTMANAGER_DATA_H_
#define GATTMANAGER_DATA_H_

#include <library.h>
#include <gatt.h>

typedef enum __gatt_manager_initialisation_state
{
    gatt_manager_initialisation_state_not_initialised,
    gatt_manager_initialisation_state_registration,
    gatt_manager_initialisation_state_registering,
    gatt_manager_initialisation_state_initialised,
    gatt_manager_initialisation_state_max

} gatt_manager_initialisation_state_t;

typedef enum __gatt_manager_advertising_state
{
    gatt_manager_advertising_state_idle,
    gatt_manager_advertising_state_queued,
    gatt_manager_advertising_state_requested,
    gatt_manager_advertising_state_advertising,
    gatt_manager_advertising_state_max

} gatt_manager_advertising_state_t;

typedef struct __gatt_manager_server_lookup_data
{
    Task        task;
    uint16      start_handle;
    uint16      end_handle;
    bool        pending_write;

} gatt_manager_server_lookup_data_t;

typedef struct __gatt_manager_client_lookup_data
{
    Task        task;
    uint16      cid;
    uint16      start_handle;
    uint16      end_handle;
} gatt_manager_client_lookup_data_t;

typedef struct __gatt_manager_resolve_server_handle
{
    Task        task;
    uint16      handle;
    uint16      adjusted;

} gatt_manager_resolve_server_handle_t;

typedef struct __gatt_manager_data_iterator
{
    uint16  iterator;
} gatt_manager_data_iterator_t;

typedef struct __gatt_manager_data *gatt_manager_data_t;


/*
 * Initialisation related functions
 * ****************************************************************************/

gatt_manager_data_t gattManagerDataInit(void(*handler)(Task, MessageId, Message),
                                        Task application_task);
bool gattManagerDataIsInit(void);

void gattManagerDataDeInit(void);


/*
 * GATT Manager State functions
 * ****************************************************************************/

void gattManagerDataInitialisationState_NotInitialised(void);

void gattManagerDataInitialisationState_Registration(void);

void gattManagerDataInitialisationState_Registering(void);

void gattManagerDataInitialisationState_Initialised(void);

gatt_manager_initialisation_state_t gattManagerDataGetInitialisationState(void);


/*
 * GATT Manager task functions
 * ****************************************************************************/

Task gattManagerDataGetTask(void);

Task gattManagerDataGetApplicationTask(void);


/*
 * GATT Manager Advertising State functions
 * ****************************************************************************/

void gattManagerDataAdvertisingState_Idle(void);

void gattManagerDataAdvertisingState_Queued(void);

void gattManagerDataAdvertisingState_Requested(void);

void gattManagerDataAdvertisingState_Advertising(void);

gatt_manager_advertising_state_t gattManagerDataGetAdvertisingState(void);

uint16 * gattManagerDataGetAdvertisingRequestedFlag(void);

/*
 * GATT Manager Cancel pending connection functions
 * ****************************************************************************/

void gattManagerDataCancelPending(void);

void gattManagerDataPendingCancelled(void);

bool gattManagerDataIsCancelPending(void);

/*
 * GATT Manager Server functions
 * ****************************************************************************/

bool gattManagerDataAddServer(const gatt_manager_server_registration_params_t *server);

uint16 gattManagerDataServerCount(void);

uint16 gattManagerDataGetServerDatabaseHandle(Task task, uint16 handle);

gatt_manager_server_lookup_data_t * gattManagerDataFindServerTask(uint16 handle);

bool gattManagerDataResolveServerHandle(gatt_manager_resolve_server_handle_t * data);

bool gattManagerDataServerIteratorStart(gatt_manager_data_iterator_t *iter);

const gatt_manager_server_lookup_data_t * gattManagerDataServerIteratorNext(gatt_manager_data_iterator_t *iter);

void gattManagerDataSetServerPendingWriteFlag(uint16 handle);

void gattManagerDataSetApplicationPendingWriteFlag(void);

bool gattManagerDataServerIteratorPrepareWriteFlagsNext(gatt_manager_server_lookup_data_t *server, gatt_manager_data_iterator_t *iter);

bool gattManagerDataServerGetPrepareWriteFlag(Task task);

void gattManagerDataServerSetExecuteWriteResult(uint16 result);

uint16 gattManagerDataServerGetExecuteWriteResult(void);

bool gattManagerDataServerClearPrepareWriteFlag(Task task);

/*
 * GATT Manager Client functions
 * *****************************************************************************/

bool gattManagerDataAddClient(const gatt_manager_client_registration_params_t *client);

bool gattManagerDataRemoveClient(const Task client);

const gatt_manager_client_lookup_data_t * gattManagerDataGetClientByTask(const Task client);

Task gattManagerDataGetClientByCid(uint16 handle, uint16 cid);


/*
 * GATT Manager GATT DB functions
 * *****************************************************************************/

void gattManagerDataSetConstDB(const uint16* db_ptr, uint16 size);

const void * gattManagerDataGetDB(void);

uint16 gattManagerDataGetDBSize(void);


/*
 * GATT Manager functions for the handler task when Connecting to remote Server
 * *****************************************************************************/

void gattManagerDataSetRemoteServerConnectHandler(Task conn_task);

void gattManagerDataSetRemoteServerConnectCid(uint16 cid);

Task gattManagerDataGetRemoteServerConnectHandler(void);

uint16 gattManagerDataGetRemoteServerConnectCid(void);

Task * gattManagerDataGetPointerToRemoteServerConnectHandler(void);

/*
 * GATT Manager functions for tracking remote connecting client
 * *****************************************************************************/

Task gattManagerDataGetRemoteClientConnectTask(void);

void gattManagerDataSetRemoteClientConnectTask(Task task);

uint16 gattManagerDataGetRemoteClientConnectCid(void);

void gattManagerDataSetRemoteClientConnectCid(uint16 cid);

#endif /* GATTMANAGER_DATA_H_ */

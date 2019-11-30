/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

*/

#include <stdlib.h>
#include <string.h>

#include "gatt_manager_internal.h"
#include "gatt_manager_data.h"

typedef struct __gatt_manager_server_lookup
{
    uint16                            count;
    gatt_manager_server_lookup_data_t *table;

} gatt_manager_server_lookup_t;

typedef struct __gatt_manager_client_lookup
{
    uint16                            count;
    gatt_manager_client_lookup_data_t *table;

} gatt_manager_client_lookup_t;

typedef struct __gatt_manager_db
{
    uint16       size;
    const uint16 *ptr;    /* Const GATT database */

} gatt_manager_db_t;

typedef struct __gatt_manager_connecting_client
{
    Task    task;
    uint16  cid;

} gatt_manager_connecting_client_t;

typedef struct __gatt_manager_connecting_server
{
    Task    task;
    uint16  cid;

} gatt_manager_connecting_server_t;

struct __gatt_manager_data
{
    TaskData                            gatt_manager_task;
    Task                                application_task;
    gatt_manager_initialisation_state_t initialisation_state:gatt_manager_initialisation_state_max;
    gatt_manager_advertising_state_t    advertising_state:gatt_manager_advertising_state_max;
    uint16                              advertising_requested_flag;
    gatt_manager_db_t                   db;
    gatt_manager_server_lookup_t        server_lookup;
    gatt_manager_client_lookup_t        client_lookup;
    gatt_manager_connecting_client_t    remote_client_connect;  /* When waiting for a client device to connect, this data keeps track of the connection */
    gatt_manager_connecting_server_t    remote_server_connect;  /* When waiting for a server device connection, this data keeps track of the connection */
    uint16                              execute_write_result;
    BITFIELD                            cancel_pending_connection:1;
    BITFIELD                            application_pending_write:1;
};

static gatt_manager_data_t gatt_manager_data = NULL;

/*
 * Initialisation related functions
 * ****************************************************************************/
gatt_manager_data_t gattManagerDataInit(void(*gatt_manager_handler)(Task, MessageId, Message),
                                        Task application_task)
{
    if (NULL == gatt_manager_handler ||
        NULL == application_task)
    {
        return NULL;
    }

    GATT_MANAGER_PANIC_NOT_NULL(gatt_manager_data, ("GM: Already initialised!"));

    gatt_manager_data = (gatt_manager_data_t)malloc(sizeof(struct __gatt_manager_data));
    GATT_MANAGER_PANIC_NULL(gatt_manager_data, ("GM: Malloc Failed!"));

    memset((void*)gatt_manager_data, 0, sizeof(struct __gatt_manager_data));
    gatt_manager_data->gatt_manager_task.handler = gatt_manager_handler;
    gatt_manager_data->application_task = application_task;
    return gatt_manager_data;
}

bool gattManagerDataIsInit(void)
{
    return NULL != gatt_manager_data;
}

void gattManagerDataDeInit(void)
{
    if(gattManagerDataIsInit())
    {
        free(gatt_manager_data->server_lookup.table);
        free(gatt_manager_data->client_lookup.table);
        free(gatt_manager_data);
        gatt_manager_data = NULL;
    }
}


/*
 * GATT Manager State functions
 * ****************************************************************************/

void gattManagerDataInitialisationState_NotInitialised(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->initialisation_state = gatt_manager_initialisation_state_not_initialised;
    }
}

void gattManagerDataInitialisationState_Registration(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->initialisation_state = gatt_manager_initialisation_state_registration;
    }
}

void gattManagerDataInitialisationState_Registering(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->initialisation_state = gatt_manager_initialisation_state_registering;
    }
}

void gattManagerDataInitialisationState_Initialised(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->initialisation_state = gatt_manager_initialisation_state_initialised;
    }
}

gatt_manager_initialisation_state_t gattManagerDataGetInitialisationState(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->initialisation_state;
    }

    return gatt_manager_initialisation_state_not_initialised;
}


/*
 * GATT Manager task functions
 * ****************************************************************************/

Task gattManagerDataGetTask(void)
{
    if (gattManagerDataIsInit())
    {
        return &gatt_manager_data->gatt_manager_task;
    }

    return NULL;
}

Task gattManagerDataGetApplicationTask(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->application_task;
    }

    return NULL;
}


/*
 * GATT Manager Advertising State functions
 * ****************************************************************************/
static void checkAdvertisingRequested(void)
{
    if(gatt_manager_data->advertising_state == gatt_manager_advertising_state_requested)
    {
        gatt_manager_data->advertising_requested_flag = 1;
    }
    else
    {
        gatt_manager_data->advertising_requested_flag = 0;
    }
}

void gattManagerDataAdvertisingState_Idle(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->advertising_state = gatt_manager_advertising_state_idle;
        checkAdvertisingRequested();
    }
}

void gattManagerDataAdvertisingState_Queued(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->advertising_state = gatt_manager_advertising_state_queued;
        checkAdvertisingRequested();
    }
}

void gattManagerDataAdvertisingState_Requested(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->advertising_state = gatt_manager_advertising_state_requested;
        checkAdvertisingRequested();
    }
}

void gattManagerDataAdvertisingState_Advertising(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->advertising_state = gatt_manager_advertising_state_advertising;
        checkAdvertisingRequested();
    }
}

gatt_manager_advertising_state_t gattManagerDataGetAdvertisingState(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->advertising_state;
    }

    return gatt_manager_advertising_state_idle;
}

uint16 * gattManagerDataGetAdvertisingRequestedFlag(void)
{
    if (gattManagerDataIsInit())
    {
        return &gatt_manager_data->advertising_requested_flag;
    }

    return NULL;
}

/*
 * GATT Manager Cancel pending connection functions
 * ****************************************************************************/

void gattManagerDataCancelPending(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->cancel_pending_connection = TRUE;
    }
}

void gattManagerDataPendingCancelled(void)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->cancel_pending_connection = FALSE;
    }
}

bool gattManagerDataIsCancelPending(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->cancel_pending_connection;
    }

    return FALSE;
}


/*
 * GATT Manager GATT DB functions
 * *****************************************************************************/

void gattManagerDataSetConstDB(const uint16* db_ptr, uint16 size)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->db.ptr = db_ptr;
        gatt_manager_data->db.size = size;
    }
}

const void * gattManagerDataGetDB(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->db.ptr;
    }

    return NULL;
}

uint16 gattManagerDataGetDBSize(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->db.size;
    }

    return 0;
}


/*
 * GATT Manager Server functions
 * *****************************************************************************/
bool gattManagerDataServerIteratorStart(gatt_manager_data_iterator_t *iter)
{
    if(NULL != iter)
    {
        iter->iterator = 0;
        return TRUE;
    }

    return FALSE;
}

const gatt_manager_server_lookup_data_t * gattManagerDataServerIteratorNext(gatt_manager_data_iterator_t *iter)
{
    const gatt_manager_server_lookup_data_t *data = NULL;

    if(NULL != iter)
    {
        if(iter->iterator < gatt_manager_data->server_lookup.count)
        {
            data = &gatt_manager_data->server_lookup.table[iter->iterator];
            ++iter->iterator;
        }
    }

    return data;
}

static bool gattManagerDataServerHandlesValid(const gatt_manager_server_registration_params_t *server)
{
    uint16 index;
    if (0 == gatt_manager_data->server_lookup.count)
    {
        return TRUE;
    }

    for (index = 0; index < gatt_manager_data->server_lookup.count; ++index)
    {
        if ((server->start_handle >= gatt_manager_data->server_lookup.table[index].start_handle &&
             server->start_handle <= gatt_manager_data->server_lookup.table[index].end_handle) ||
            (server->end_handle >= gatt_manager_data->server_lookup.table[index].start_handle &&
             server->end_handle <= gatt_manager_data->server_lookup.table[index].end_handle))
        {
            return FALSE;
        }
    }

    return TRUE;
}


bool gattManagerDataAddServer(const gatt_manager_server_registration_params_t *server)
{
    void * ptr;
    size_t realloc_size;
    uint16 idx;

    if (!gattManagerDataIsInit() ||
        NULL == server)
    {
        return FALSE;
    }

    if (NULL == gattManagerDataGetDB() ||
        !gattManagerDataServerHandlesValid(server))
    {
        return FALSE;
    }

    realloc_size = (gatt_manager_data->server_lookup.count + 1) *
                   sizeof(gatt_manager_server_lookup_data_t);
    ptr = realloc(gatt_manager_data->server_lookup.table, realloc_size);
    GATT_MANAGER_PANIC_NULL(ptr, ("GM: Realloc Failed!"));

    gatt_manager_data->server_lookup.table = (gatt_manager_server_lookup_data_t*)ptr;
    idx = gatt_manager_data->server_lookup.count;
    gatt_manager_data->server_lookup.table[idx].task  = server->task;

    gatt_manager_data->server_lookup.table[idx].start_handle = server->start_handle;
    gatt_manager_data->server_lookup.table[idx].end_handle = server->end_handle;
    
    gatt_manager_data->server_lookup.table[idx].pending_write = FALSE;

    ++gatt_manager_data->server_lookup.count;
    return TRUE;
}

uint16 gattManagerDataServerCount(void)
{
    return gatt_manager_data->server_lookup.count;
}

uint16 gattManagerDataGetServerDatabaseHandle(Task task, uint16 handle)
{
    unsigned index;

    if (NULL == gatt_manager_data ||
        NULL == gatt_manager_data->server_lookup.table ||
        NULL == task)
    {
        return 0;
    }

    for (index = 0; index < gatt_manager_data->server_lookup.count; ++index)
    {
        if (gatt_manager_data->server_lookup.table[index].task == task)
        {
            /* Convert the handle value passed in to the real value of the handle in the registered DB */
            uint16 adjusted_handle = (gatt_manager_data->server_lookup.table[index].start_handle - 1) + handle;

            /* Ensure the handle requested is not out of range for the service */
            if ( (adjusted_handle >= gatt_manager_data->server_lookup.table[index].start_handle) &&
                 (adjusted_handle <= gatt_manager_data->server_lookup.table[index].end_handle) )
            {
                return adjusted_handle;
            }
        }
    }

    return 0;
}


gatt_manager_server_lookup_data_t * gattManagerDataFindServerTask(uint16 handle)
{
    uint16 index;

    if (NULL == gatt_manager_data ||
        NULL == gatt_manager_data->server_lookup.table)
    {
        return NULL;
    }

    for (index = 0; index < gatt_manager_data->server_lookup.count; ++index)
    {
        if ( (handle >= gatt_manager_data->server_lookup.table[index].start_handle) &&
             (handle <= gatt_manager_data->server_lookup.table[index].end_handle) )
        {
            return &gatt_manager_data->server_lookup.table[index];
        }
    }

    return NULL;
}

bool gattManagerDataResolveServerHandle(gatt_manager_resolve_server_handle_t * data)
{
    unsigned index;

    if (NULL == gatt_manager_data ||
        NULL == gatt_manager_data->server_lookup.table ||
        NULL == data)
    {
        return FALSE;
    }

    for (index = 0; index < gatt_manager_data->server_lookup.count; ++index)
    {
        if ( (gatt_manager_data->server_lookup.table[index].start_handle <= data->handle) &&
             (gatt_manager_data->server_lookup.table[index].end_handle   >= data->handle) )
        {
            data->adjusted = ((data->handle - gatt_manager_data->server_lookup.table[index].start_handle) + 1);
            data->task = gatt_manager_data->server_lookup.table[index].task;
            return TRUE;
        }
    }

    return FALSE;
}

void gattManagerDataSetServerPendingWriteFlag(uint16 handle)
{
    uint16 index;

    if (NULL == gatt_manager_data ||
        NULL == gatt_manager_data->server_lookup.table)
    {
        return;
    }
    
    /* Set the pending write flag on the server that uses the specified handle */

    for (index = 0; index < gatt_manager_data->server_lookup.count; ++index)
    {
        if ( (handle >= gatt_manager_data->server_lookup.table[index].start_handle) &&
             (handle <= gatt_manager_data->server_lookup.table[index].end_handle) )
        {
            gatt_manager_data->server_lookup.table[index].pending_write = TRUE;
        }
    }
}

void gattManagerDataSetApplicationPendingWriteFlag(void)
{
    /* Set the pending write flag on the application server that uses the specified handle */
    
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->application_pending_write = TRUE;
    }  
}

bool gattManagerDataServerIteratorPrepareWriteFlagsNext(gatt_manager_server_lookup_data_t *server, gatt_manager_data_iterator_t *iter)
{
    /* Returns the next server that has the panding write flag set */
    
    if(NULL != iter)
    {
        while(iter->iterator < gatt_manager_data->server_lookup.count)
        {
            if (gatt_manager_data->server_lookup.table[iter->iterator].pending_write)
            {
                /* flag set, return data for this server */
                server->task = gatt_manager_data->server_lookup.table[iter->iterator].task;
                server->start_handle = gatt_manager_data->server_lookup.table[iter->iterator].start_handle;
                server->end_handle = gatt_manager_data->server_lookup.table[iter->iterator].end_handle;
                ++iter->iterator;    
                return TRUE;
            }
            else
            {
                /* flag not set, try the next server */
                ++iter->iterator;
            }
        }
        /* none of the servers have a flag set, try the app */
        if ((iter->iterator == gatt_manager_data->server_lookup.count) &&
            gatt_manager_data->application_pending_write)
        {
            server->task = gattManagerDataGetApplicationTask();
            server->start_handle = 0;
            server->end_handle = 0;
            ++iter->iterator;
            return TRUE;
        }
    }
    return FALSE;
}

bool gattManagerDataServerGetPrepareWriteFlag(Task task)
{
    uint16 index;

    if (NULL == gatt_manager_data ||
        NULL == gatt_manager_data->server_lookup.table)
    {
        return FALSE;
    }
    
    /* Returns the pending write flag for the server that uses the specified Task */

    /* is this a server? */
    for (index = 0; index < gatt_manager_data->server_lookup.count; ++index)
    {
        if (task == gatt_manager_data->server_lookup.table[index].task)
        {
            return gatt_manager_data->server_lookup.table[index].pending_write;
        }
    }
    /* is it the app? */
    if (task == gattManagerDataGetApplicationTask())
    {
        return gatt_manager_data->application_pending_write;
    }
    return FALSE;
}

void gattManagerDataServerSetExecuteWriteResult(uint16 result)
{
    /* Sets the overall Execute Write response code */
    
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->execute_write_result = result;
    }
}

uint16 gattManagerDataServerGetExecuteWriteResult(void)
{
    /* Gets the overall Execute Write response code */
    
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->execute_write_result;
    }
    return gatt_status_failure;
}

bool gattManagerDataServerClearPrepareWriteFlag(Task task)
{
    uint16 index;
    bool pending_write_flags_clear = TRUE;

    if (NULL == gatt_manager_data ||
        NULL == gatt_manager_data->server_lookup.table)
    {
        return FALSE;
    }

    /* Clear pending write flag from the server with the specified task.
       The function will return TRUE if the pending write flag from all the servers have been cleared.
    */
    
    /* is this a server? */
    for (index = 0; index < gatt_manager_data->server_lookup.count; ++index)
    {
        if (task == gatt_manager_data->server_lookup.table[index].task)
        {
            gatt_manager_data->server_lookup.table[index].pending_write = FALSE;
        }
        else if (gatt_manager_data->server_lookup.table[index].pending_write)
        {
            pending_write_flags_clear = FALSE;
        }
    }
    /* is it the app? */
    if (index >= gatt_manager_data->server_lookup.count && task == gattManagerDataGetApplicationTask())
    {
        gatt_manager_data->application_pending_write = FALSE;
    }
    else if (gatt_manager_data->application_pending_write)
    {
        pending_write_flags_clear = FALSE;
    }
    
    return pending_write_flags_clear;
}


/*
 * GATT Manager Client functions
 * ****************************************************************************/

static void allocateNewClientEntry(void)
{
    void * ptr;
    size_t size_db = sizeof(gatt_manager_client_lookup_data_t) *
                     (gatt_manager_data->client_lookup.count + 1);

    ptr = realloc(gatt_manager_data->client_lookup.table, size_db );
    GATT_MANAGER_PANIC_NULL(ptr, ("GM: Realloc Failed!"));

    gatt_manager_data->client_lookup.table = (gatt_manager_client_lookup_data_t*)ptr;
}

static bool removeClientLookupElement(uint16 delete_index)
{
    unsigned loop_index = 0;
    void * reallocated = NULL;

    /* Shift elements down one position to fill gap left by deleted element */
    for (loop_index = delete_index + 1;
         loop_index < gatt_manager_data->client_lookup.count;
         ++loop_index)
    {
        gatt_manager_data->client_lookup.table[loop_index - 1] =
                gatt_manager_data->client_lookup.table[loop_index];
    }

    /* Size is now one less */
    --gatt_manager_data->client_lookup.count;

    if (gatt_manager_data->client_lookup.count)
    {
        /* Reallocate memory that stores lookup table */
        reallocated = realloc(gatt_manager_data->client_lookup.table,
                              sizeof(gatt_manager_client_lookup_data_t) *
                              gatt_manager_data->client_lookup.count);

        if (reallocated == NULL)
        {
            /* Realloc should never fail. If it does, then the original 
             * block is still valid and unchanged ( it just has an 
             * unused entry at the end. */
            return TRUE;
        }

        gatt_manager_data->client_lookup.table = (gatt_manager_client_lookup_data_t*)reallocated;
    }
    else
    {
        free(gatt_manager_data->client_lookup.table);
        gatt_manager_data->client_lookup.table = NULL;
    }

    return TRUE;
}

bool gattManagerDataAddClient(const gatt_manager_client_registration_params_t *client)
{
    if (!gattManagerDataIsInit() ||
        NULL == client)
    {
        return FALSE;
    }

    allocateNewClientEntry();
    gatt_manager_data->client_lookup.table[gatt_manager_data->client_lookup.count].task  = client->client_task;
    gatt_manager_data->client_lookup.table[gatt_manager_data->client_lookup.count].cid   = client->cid;
    gatt_manager_data->client_lookup.table[gatt_manager_data->client_lookup.count].start_handle = client->start_handle;
    gatt_manager_data->client_lookup.table[gatt_manager_data->client_lookup.count].end_handle   = client->end_handle;
    ++gatt_manager_data->client_lookup.count;
    return TRUE;
}

bool gattManagerDataRemoveClient(const Task client)
{
    uint16 index = 0;
    bool rt = FALSE;
    if (!gattManagerDataIsInit() ||
        NULL == gatt_manager_data->client_lookup.table ||
        NULL == client)
    {
        return FALSE;
    }

    for (index = 0; index < gatt_manager_data->client_lookup.count; ++index)
    {
        if (gatt_manager_data->client_lookup.table[index].task == client)
        {
            if(removeClientLookupElement(index))
            {
                rt =  TRUE;
            }
            break;
        }
    }

    return rt;
}

const gatt_manager_client_lookup_data_t * gattManagerDataGetClientByTask(const Task client)
{
    uint16 index;

    if (!gattManagerDataIsInit() ||
        NULL == client)
    {
        return NULL;
    }

    for (index = 0; index < gatt_manager_data->client_lookup.count; ++index)
    {
        if (gatt_manager_data->client_lookup.table[index].task == client)
        {
            return &gatt_manager_data->client_lookup.table[index];
        }
    }

    return NULL;
}


Task gattManagerDataGetClientByCid(uint16 handle, uint16 cid)
{
    uint16 index;

    if (NULL == gatt_manager_data ||
        NULL == gatt_manager_data->client_lookup.table)
    {
        return NULL;
    }

    for (index = 0; index < gatt_manager_data->client_lookup.count; ++index)
    {
        if ((gatt_manager_data->client_lookup.table[index].start_handle <= handle) &&
            (gatt_manager_data->client_lookup.table[index].end_handle >= handle) &&
            (gatt_manager_data->client_lookup.table[index].cid == cid))
        {
            return gatt_manager_data->client_lookup.table[index].task;
        }
    }

    return NULL;
}


/*
 * GATT Manager functions for the handler task when Connecting to remote Server
 * *****************************************************************************/
void gattManagerDataSetRemoteServerConnectHandler(Task conn_task)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->remote_server_connect.task = conn_task;
        gatt_manager_data->remote_server_connect.cid = INVALID_CID;
    }
}

void gattManagerDataSetRemoteServerConnectCid(uint16 cid)
{
    if (   gattManagerDataIsInit()
        && gatt_manager_data->remote_server_connect.task)
    {
        gatt_manager_data->remote_server_connect.cid = cid;
    }
}


Task gattManagerDataGetRemoteServerConnectHandler(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->remote_server_connect.task;
    }

    return NULL;
}

uint16 gattManagerDataGetRemoteServerConnectCid(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->remote_server_connect.cid;
    }

    return INVALID_CID;
}

Task * gattManagerDataGetPointerToRemoteServerConnectHandler(void)
{
    if (gattManagerDataIsInit())
    {
        return &gatt_manager_data->remote_server_connect.task;
    }

    return NULL;
}

/*
 * GATT Manager functions for tracking remote connecting client
 * *****************************************************************************/
Task gattManagerDataGetRemoteClientConnectTask(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->remote_client_connect.task;
    }

    return NULL;
}

void gattManagerDataSetRemoteClientConnectTask(Task task)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->remote_client_connect.task = task;
    }
}

uint16 gattManagerDataGetRemoteClientConnectCid(void)
{
    if (gattManagerDataIsInit())
    {
        return gatt_manager_data->remote_client_connect.cid;
    }

    return 0;
}

void gattManagerDataSetRemoteClientConnectCid(uint16 cid)
{
    if (gattManagerDataIsInit())
    {
        gatt_manager_data->remote_client_connect.cid = cid;
    }
}

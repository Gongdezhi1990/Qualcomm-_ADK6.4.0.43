/* Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_SHIM_LAYER_H
#define GATT_SHIM_LAYER_H

#include <gatt.h>

typedef struct 
{
    gatt_status_t     status;     
} GATT_INIT_CFM_TEST_EXTRA_T;

/* flatten the uuid uint32 array into a bigendian uint8 array. */
typedef struct
{
    gatt_status_t status;    
    uint16 cid;
    uint16 handle;
    uint16 end;
    bool more_to_come;
    uint16 size_uuid;
    uint8 uuid[1];
} GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM_TEST_EXTRA_T;

typedef struct
{
    gatt_status_t status;    
    uint16 cid;
    uint16 handle;
    uint16 end;
    bool more_to_come;
    uint16 size_uuid;
    uint8 uuid[1];
} GATT_DISCOVER_PRIMARY_SERVICE_CFM_TEST_EXTRA_T;

typedef struct
{
    gatt_status_t status;    
    uint16 cid;
    uint16 handle;
    uint16 end;
    bool more_to_come;
    uint16 size_uuid;
    uint8 uuid[1];
} GATT_FIND_INCLUDED_SERVICES_CFM_TEST_EXTRA_T;

typedef struct 
{
    gatt_status_t status;
    uint16 cid;
    uint16 handle;
    uint8 properties;
    bool more_to_come;
    uint16 size_uuid;
    uint8 uuid[1];
} GATT_DISCOVER_ALL_CHARACTERISTICS_CFM_TEST_EXTRA_T;

typedef struct 
{
    gatt_status_t status;
    uint16 cid;
    uint16 handle;
    bool more_to_come;
    uint16 size_uuid;
    uint8 uuid[1];
} GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_TEST_EXTRA_T;


#define GATT_SHIM_MESSAGE_BASE
typedef enum
{
    GATT_INIT_CFM_TEST_EXTRA = GATT_MESSAGE_TOP,
    GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM_TEST_EXTRA, 
    GATT_DISCOVER_PRIMARY_SERVICE_CFM_TEST_EXTRA,
    GATT_FIND_INCLUDED_SERVICES_CFM_TEST_EXTRA,
    GATT_DISCOVER_ALL_CHARACTERISTICS_CFM_TEST_EXTRA,
    GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_TEST_EXTRA,

    /* Library message limit*/
    GATT_SHIM_MESSAGE_TOP
} GattShimMessageId;


void GattHandleComplexMessage(Task task, MessageId id, Message message);

void GattInitTestExtraDefault(Task theAppTask, uint16 size_database, uint8* database);

void GattDiscoverPrimaryServiceRequestTestExtraDefault(
        Task                    theAppTask,
        uint16                  cid,
        uint16                  size_uuid,
        const                   uint8 *uuid
        );


#endif /* GATT_SHIM_LAYER_H */

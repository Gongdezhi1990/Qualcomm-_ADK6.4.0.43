/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    resource_manager.c

DESCRIPTION
    Short description about what functions the source file contains.
*/

#include <stdlib.h>
#include <string.h>

#include <panic.h>
#include <print.h>
#include <vmtypes.h>

#include <resource_manager.h>
#include <resource_manager_private.h>


typedef struct {
    rm_callback acquired_cb;
    rm_callback queued_cb;
} rm_resource_t;

/* The internal data structure for the state of each resource. */
static rm_resource_t *resources = NULL;


/******************************************************************************/
static void resourceManagerInit(void)
{
    uint16 size = rm_resource_last * sizeof(rm_resource_t);

    PanicNotNull(resources);

    resources = PanicUnlessMalloc(size);
    memset(resources, 0, size);
}

/******************************************************************************/
void resourceManagerDeInit(void)
{
    if (resources)
    {
        free(resources);
        resources = NULL;
    }
}

/******************************************************************************/
rm_status_t ResourceManagerAcquireResource(rm_resource_id_t res, rm_callback callback)
{
    rm_status_t status = rm_status_success;
    rm_resource_t *resource = NULL;

    PRINT(("ResourceManagerAcquireResource\n"));

    if (!resources)
        resourceManagerInit();

    if (res >= rm_resource_last || !callback)
        return rm_status_bad_parameter;

    resource = &resources[res];

    if (resource->acquired_cb == callback
        || resource->queued_cb == callback)
        Panic();

    if (!resource->acquired_cb)
    {
        /* Resource is free to acquire */
        resource->acquired_cb = callback;
    }
    else if (!resource->queued_cb)
    {
        /* Request first client to release resource. */
        if ((resource->acquired_cb)(rm_msg_release_req, res))
        {
            /* Resource has been released; set new client as owner and queue previous client. */
            resource->queued_cb = resource->acquired_cb;
            resource->acquired_cb = callback;
        }
        else
        {
            /* Resource not released so queue the new client. */
            resource->queued_cb = callback;
            status = rm_status_in_use_queued;
        }
    }
    else
    {
        /* Resource is in use and has another client waiting on it. */
        status = rm_status_in_use_rejected;
    }

    return status;
}

/******************************************************************************/
void ResourceManagerReleaseResource(rm_resource_id_t res, rm_callback callback)
{
    rm_resource_t *resource = NULL;

    PRINT(("ResourceManagerReleaseResource\n"));

    if (!resources)
        resourceManagerInit();

    if (res >= rm_resource_last || !callback)
        Panic();

    resource = &resources[res];

    if (callback == resource->acquired_cb)
    {
        if (resource->queued_cb && (resource->queued_cb)(rm_msg_available_ind, res))
        {
            resource->acquired_cb = resource->queued_cb;
            resource->queued_cb = NULL;
        }
        else
        {
            resource->acquired_cb = NULL;
            resource->queued_cb = NULL;
        }
    }
    else if (callback == resource->queued_cb)
    {
        resource->queued_cb = NULL;
    }
}

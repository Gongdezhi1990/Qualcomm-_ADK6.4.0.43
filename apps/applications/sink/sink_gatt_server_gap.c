/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_gap.c

DESCRIPTION
    Routines to handle messages sent from the GATT GAP Server task.
*/

#include "sink_ble.h"
#include "sink_gatt_db.h"
#include "sink_gatt_server_gap.h"

#include "sink_ble_advertising.h"
#include "sink_debug.h"
#include "sink_development.h"
#include "sink_gatt_device.h"
#include "sink_gatt_common.h"

#include <gatt_gap_server.h>

#include <stdlib.h>
#include <csrtypes.h>
#include <message.h>


#ifdef GATT_ENABLED


#ifdef DEBUG_GATT
#define GATT_INFO(x) DEBUG(x)
#define GATT_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else 
#define GATT_INFO(x) 
#define GATT_ERROR(x) 
#endif


/*******************************************************************************
NAME
    sinkGattGapServerInitialiseTask
    
DESCRIPTION
    Initialise the GAP server task.
    NOTE: This function will modify *ptr.
    
PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.
    
RETURNS
    TRUE if the GAP server task was initialised, FALSE otherwise.
*/
bool sinkGattGapServerInitialiseTask(uint16 **ptr)
{
    if (ptr)
    {
        if (GattGapServerInit((GGAPS*)*ptr, sinkGetBleTask(),
                                         HANDLE_GAP_SERVICE,
                                         HANDLE_GAP_SERVICE_END) == gatt_gap_server_status_success)
        {
            GATT_INFO(("GATT GAP Server initialised\n"));
            gattServerSetServicePtr(ptr, gatt_server_service_gap);
            *ptr += ADJ_GATT_STRUCT_OFFSET(GGAPS);
            return TRUE;
        }
        else
        {
            GATT_INFO(("GATT GAP Server init failed\n"));
        }
    }
    return FALSE;
}


/*******************************************************************************
NAME
    sinkGattGapServerHandleReadAppearanceInd
    
DESCRIPTION
    Function to handle when a remote device wants to read the appearance 
    characteristic of the GAP service.
    
PARAMETERS
    GATT_GAP_SERVER_READ_APPEARANCE_IND_T message
    
RETURNS
    TRUE if the initialisation request was successful, FALSE otherwise.
*/
static bool sinkGattGapServerHandleReadAppearanceInd(GATT_GAP_SERVER_READ_APPEARANCE_IND_T *msg)
{
    /* Appearance=unknown; refer to Bluetooth Sig website for appearance values */
    uint8 appearance[2] = {0x00, 0x00};
    uint16 index = gattCommonConnectionFindByCid(msg->cid);
    gatt_gap_server_status_t status = gatt_gap_server_status_failed;

    if(index != GATT_INVALID_INDEX)
        status = GattGapServerReadAppearanceResponse(GATT_SERVER.gap_server, msg->cid, (2*sizeof(uint8)), appearance);
    
    if (status == gatt_gap_server_status_success)
        return TRUE;

    return FALSE;
}

/***************************************************************************/
static void gattGapServerNameReadResponse(uint16 size_local_name, uint8 *local_name)
{
    uint16 index;

#ifdef GATT_BISTO_COMM_SERVER
    local_name = bleBistoPrefixedName(local_name, &size_local_name);
#endif

    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
        if (GATT[index].cid)
        {
            if (GATT_SERVER.gap_read_name.requested)
            {
                if (GATT_SERVER.gap_read_name.offset >= size_local_name)
                {
                    GATT_INFO(("GATT Client: Read GAP name response Invalid cid[0x%x]\n", GATT[index].cid));
                    GattGapServerReadDeviceNameResponse(GATT_SERVER.gap_server,
                                                         GATT[index].cid,
                                                         0,
                                                         NULL);
                }
                else
                {
                    GATT_INFO(("GATT Client: Read GAP name response cid[0x%x] size[0x%x] offset[0x%x]\n", 
                                       GATT[index].cid,
                                       size_local_name,
                                       GATT_SERVER.gap_read_name.offset));
                    GattGapServerReadDeviceNameResponse(GATT_SERVER.gap_server,
                                                         GATT[index].cid,
                                                         ((size_local_name - GATT_SERVER.gap_read_name.offset) * sizeof(uint8)),
                                                         &local_name[GATT_SERVER.gap_read_name.offset]);
                }
                GATT_SERVER.gap_read_name.requested = FALSE;
                GATT_SERVER.gap_read_name.offset = 0;
            }
        }
    }

#ifdef GATT_BISTO_COMM_SERVER
    free(local_name);
#endif
}

/******************************************************************************/
void sinkGattGapServerSendLocalNameResponse(CL_DM_LOCAL_NAME_COMPLETE_T * msg)
{ 
    gattGapServerNameReadResponse(msg->size_local_name, msg->local_name);
}

/***************************************************************************/
static bool gattGapServerNameReadRequested(uint16 cid, uint16 offset)
{
    uint16 index = gattCommonConnectionFindByCid(cid);
    if (GATT_INVALID_INDEX != index)
    {
        GATT_INFO(("GATT Client: Read GAP name request cid[0x%x] offset[0x%x]\n", cid, offset));
        /* Store the data to use for the response */
        GATT_SERVER.gap_read_name.requested = TRUE;
        GATT_SERVER.gap_read_name.offset = offset;
        return TRUE;
    }
    
    return FALSE;
}

/******************************************************************************/
gap_msg_status_t sinkGattGapServerMsgHandler(Task task, MessageId id, Message message)
{
    gap_msg_status_t status = gap_msg_failed;
    UNUSED(task);
    
    switch(id)
    {
        case GATT_GAP_SERVER_READ_DEVICE_NAME_IND:
        {
            GATT_GAP_SERVER_READ_DEVICE_NAME_IND_T* ind
             = (GATT_GAP_SERVER_READ_DEVICE_NAME_IND_T*)message;
            GATT_INFO(("GATT_GAP_SERVER_READ_DEVICE_NAME_IND\n"));
            if (gattGapServerNameReadRequested(ind->cid, ind->name_offset))
            {
                status = gap_msg_read_name_required;
            }
        }
        break;
        case GATT_GAP_SERVER_READ_APPEARANCE_IND:
        {
            GATT_INFO(("GATT_GAP_SERVER_READ_APPEARANCE_IND\n"));
            sinkGattGapServerHandleReadAppearanceInd((GATT_GAP_SERVER_READ_APPEARANCE_IND_T *)message);
            status = gap_msg_success;
        }
        break;
        default:
        {
            GATT_ERROR(("GATT GAP Server task unhandled msg[%x]\n", id));
        }
    }
    
    return status;
}


#endif /* GATT_ENABLED */

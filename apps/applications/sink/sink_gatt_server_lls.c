/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_lls.c

DESCRIPTION
    Routines to handle messages sent from the GATT Link Loss Server Task.
*/

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* Application headers */
#include "sink_gatt_db.h"
#include "sink_gatt_server_lls.h"
#include "sink_ble.h"
#include "sink_debug.h"
#include "sink_gatt_server.h"
#include "sink_configmanager.h"
#include "sink_events.h"
#include "sink_main_task.h"

#include "sink_gatt_common.h"

#ifdef GATT_LLS_SERVER

#include "config_definition.h"
#include "sink_gatt_server_lls_config_def.h"
#include <config_store.h>
#ifdef DEBUG_GATT_LLS_SERVER
#define GATT_LLS_SERVER_DEBUG(x) DEBUG(x)
#else
#define GATT_LLS_SERVER_DEBUG(x) 
#endif


/*******************************************************************************
NAME
    sinkGattGetLinkLossTimer_s
    
DESCRIPTION
    Get link loss Timer_s.
    
RETURNS    
    uint16  
*/
uint16 sinkGattGetLinkLossTimer_s(void)
{
    uint16 ret_val = 0;
    gatt_lls_readonly_config_def_t *lls_r_config = NULL;
    
    if (configManagerGetReadOnlyConfig(GATT_LLS_READONLY_CONFIG_BLK_ID, (const void **)&lls_r_config))
    {    
        ret_val = lls_r_config->LinkLossTimer_s;
        configManagerReleaseConfig(GATT_LLS_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

bool sinkGattSetLinkLossTimer_s(uint16 timeout)
{
    gatt_lls_readonly_config_def_t *lls_r_config = NULL;
    
    if (configManagerGetWriteableConfig(GATT_LLS_READONLY_CONFIG_BLK_ID, (void **)&lls_r_config, 0))
    {    
        lls_r_config->LinkLossTimer_s = timeout;
        configManagerUpdateWriteableConfig(GATT_LLS_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*******************************************************************************
NAME
    sinkGattLLSGetlinkLossAlertstopTimeout_s
    
DESCRIPTION
    Get LinkLoss Alert Stop Timeout_s.
    
RETURNS    
    uint16  
*/
uint16 sinkGattGetlinkLossAlertstopTimeout_s(void)
{
    uint16 ret_val = 0;
    gatt_lls_readonly_config_def_t *lls_r_config = NULL;

    if (configManagerGetReadOnlyConfig(GATT_LLS_READONLY_CONFIG_BLK_ID, (const void **)&lls_r_config))
    {    
        ret_val = lls_r_config->LinkLossAlertStopTimeout_s;
        configManagerReleaseConfig(GATT_LLS_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

bool sinkGattSetlinkLossAlertstopTimeout_s(uint16 timeout)
{
    gatt_lls_readonly_config_def_t *lls_r_config = NULL;
    
    if (configManagerGetWriteableConfig(GATT_LLS_READONLY_CONFIG_BLK_ID, (void **)&lls_r_config, 0))
    {    
        lls_r_config->LinkLossAlertStopTimeout_s = timeout;
        configManagerUpdateWriteableConfig(GATT_LLS_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}
/*******************************************************************************
NAME
    sinkGattLinkLossServerInitialiseTask

DESCRIPTION
    Initialise the Link Loss server task.
    NOTE: This function will modify *ptr.

PARAMETERS
    @param ptr Pointer to allocated memory to store server tasks rundata.

RETURNS
    TRUE if the Link Loss server task was initialised, FALSE otherwise.

*/
bool sinkGattLinkLossServerInitialiseTask(uint16 **ptr)
{
    /* Initialises the Link Loss Service Library. */
    if(GattLinkLossServerInit(sinkGetBleTask(), (GLLSS_T*)*ptr, HANDLE_LINK_LOSS_SERVICE, 
                                               HANDLE_LINK_LOSS_SERVICE_END))
    {
        GATT_LLS_SERVER_DEBUG(("GATT_LLS : Initialised\n"));
        *ptr += ADJ_GATT_STRUCT_OFFSET(GLLSS_T);
        return TRUE;
    }
    else
    {
        GATT_LLS_SERVER_DEBUG(("GATT_LLS : Initialization Failed\n"));
        return FALSE;
    }
}

/*******************************************************************************
NAME
    sinkGattHandleLinkUpInd

DESCRIPTION
    Handle Link Up indication to stop alter

PARAMETERS
    @param cid Connection Id for the link

RETURNS
    void

*/
void sinkGattHandleLinkUpInd(uint16 cid)
{
    /* Find the GATT connection index for a given connection identifier. */
    uint16 index = gattCommonConnectionFindByCid(cid);

    GATT_LLS_SERVER_DEBUG(("GATT_LLS : sinkGattHandleLinkUpInd for GATT connection index =[0x%x]\n", index));

    if(index != GATT_INVALID_INDEX)
    {
        GATT_LLS_SERVER_DEBUG(("GATT_LLS : Stop Alert.\n"));
        /* Stop timer and stop alert */
        sinkGattLinkLossAlertStop();
    }
}

/*******************************************************************************
NAME
    sinkGattHandleLinkLossInd

DESCRIPTION
    Handle Link Loss indication to alter if required

PARAMETERS
    @param cid Connection Id for the link

RETURNS
    void

*/
void sinkGattHandleLinkLossInd(uint16 cid)
{
    bool alertStopTimer = FALSE;
    gatt_link_loss_alert_level alert_level = gatt_link_loss_alert_level_reserved;

    /* Find the GATT connection index for a given connection identifier. */
    uint16 index = gattCommonConnectionFindByCid(cid);

    GATT_LLS_SERVER_DEBUG(("GATT_LLS : sinkGattHandleLinkLossInd for GATT connection index =[0x%x]\n", index));

    /* Check if the cid is same of the connected device */
    if(index != GATT_INVALID_INDEX)
    {
        /* Read the alert level proper server */
        alert_level = GATT_SERVER.alert_level;

        /* Reset the alert level */
        GATT_SERVER.alert_level = gatt_link_loss_alert_level_no;

        GATT_LLS_SERVER_DEBUG(("GATT_LLS : Alert level =[0x%x]\n", index));

        /* Only if the alert level is set, handle it properly */
        switch(alert_level)
        {
            /* If the current link loss alert level is “No Alert," then no need to alert on link loss. */
            case gatt_link_loss_alert_level_no:
            break;

            case gatt_link_loss_alert_level_mild:
            {
                /* Generate Mild Alert level event */
                alertStopTimer = TRUE;
                sinkGattLinkLossAlertStop();
                GATT_LLS_SERVER_DEBUG(("GATT_LLS : Send Mild AlertAlert\n"));
                sinkGattLinkLossAlertMild(0);
            }
            break;

            case gatt_link_loss_alert_level_high:
            {
                /* Generate High Alert level event */
                alertStopTimer = TRUE;
                sinkGattLinkLossAlertStop();
                GATT_LLS_SERVER_DEBUG(("GATT_LLS : Send High AlertAlert\n"));
                sinkGattLinkLossAlertHigh(0);
            }
            break;
        
            default:
            break;
        }

        if(alertStopTimer)
        {
            /* Start the stop alert timeout if either High/Mild alert is set */
            MessageSendLater(&theSink.task, EventSysLlsAlertTimeout , 0, D_SEC(sinkGattGetlinkLossAlertstopTimeout_s()));
        }
    }
}

/*******************************************************************************
NAME
    sinkGattLinkLossAlertStop

DESCRIPTION
    This function stops alerting link loss.

PARAMETERS
    void

RETURNS
    void

*/
void sinkGattLinkLossAlertStop(void)
{
    GATT_LLS_SERVER_DEBUG(("GATT_LLS : sinkGattLinkLossAlertStop.\n"));
    MessageCancelAll(&theSink.task, EventSysLlsAlertTimeout);
    MessageCancelAll(&theSink.task, EventSysLlsAlertMild);
    MessageCancelAll(&theSink.task, EventSysLlsAlertHigh);
}

/*******************************************************************************
NAME
    handleAlertLevelChangeInd

DESCRIPTION
    Handle when a GATT_LLS_ALERT_LEVEL_CHANGE_IND message is recieved.

PARAMETERS
    @param ind Pointer to a GATT_LLS_ALERT_LEVEL_CHANGE_IND message.

RETURNS
    void

*/
static void handleAlertLevelChangeInd(const GATT_LLS_ALERT_LEVEL_CHANGE_IND_T * ind)
{
    /* Find the GATT connection index for a given connection identifier. */
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    GATT_LLS_SERVER_DEBUG(("GATT_LLS : GATT_LLS_ALERT_LEVEL_CHANGE_IND LLS=[0x%p] cid=[0x%x]\n", (void *)ind->link_loss_server, ind->cid));

    if(index != GATT_INVALID_INDEX)
    {
        GATT_LLS_SERVER_DEBUG(("GATT_LLS : Alert level is set to [0x%x]\n", ind->alert_level));
        GATT_SERVER.alert_level = (gatt_link_loss_alert_level)ind->alert_level;
    }
}

/*******************************************************************************
NAME
    handleAlertLevelReadReq

DESCRIPTION
    Handle when a GATT_LLS_ALERT_LEVEL_CHANGE_IND message is recieved.

PARAMETERS
    @param ind Pointer to a GATT_LLS_ALERT_LEVEL_CHANGE_IND message.

RETURNS
    void

*/
static void handleAlertLevelReadReq(const GATT_LLS_ALERT_LEVEL_READ_REQ_T * req)
{
    /* Find the GATT connection index for a given connection identifier. */
    uint16 index = gattCommonConnectionFindByCid(req->cid);

    GATT_LLS_SERVER_DEBUG(("GATT_LLS : GATT_LLS_ALERT_LEVEL_READ_REQ LLS=[0x%p] cid=[0x%x]\n", (void *)req->link_loss_server, req->cid));

    if(index != GATT_INVALID_INDEX)
    {
        GATT_LLS_SERVER_DEBUG(("GATT_LLS : GattLinkLossServerReadLevelResponse\n"));
        GattLinkLossServerReadLevelResponse(req->link_loss_server, req->cid, GATT_SERVER.alert_level);
    }
}

/*******************************************************************************
NAME
    sinkGattLinkLossServerMsgHandler

DESCRIPTION
    Handle messages from the GATT Link Loss Server library.

PARAMETERS
    @param task The task the message is delivered.
    @param id The ID for the GATT message.
    @param message Payload The message payload.

RETURNS
    void

*/
void sinkGattLinkLossServerMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    switch(id)
    {
        case GATT_LLS_ALERT_LEVEL_CHANGE_IND:
        {
            GATT_LLS_SERVER_DEBUG(("GATT_LLS : Link Loss Alert level changed\n"));
            handleAlertLevelChangeInd((const GATT_LLS_ALERT_LEVEL_CHANGE_IND_T*)message);
        }
        break;

        case GATT_LLS_ALERT_LEVEL_READ_REQ:
        {
            GATT_LLS_SERVER_DEBUG(("GATT_LLS : Link Loss Alert level read request\n"));
            handleAlertLevelReadReq((const GATT_LLS_ALERT_LEVEL_READ_REQ_T*)message);
        }
        break;

        default:
             GATT_LLS_SERVER_DEBUG(("GATT_LLS : Unknown message from LLS lib\n"));
         break;
    }
}

#endif /* GATT_LLS_SERVER */

bool sinkGattLinkLossServiceEnabled(void)
{
#ifdef GATT_LLS_SERVER
    return TRUE;
#else
    return FALSE;
#endif
}

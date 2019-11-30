/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_init.c        

DESCRIPTION
    This file contains the initialisation code for the AVRCP profile library.

NOTES

*/
/****************************************************************************
    Header files
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <panic.h>
#include <connection.h>
#include <bdaddr.h>
#include "avrcp_init.h"
#include "avrcp_profile_handler.h"

static AvrcpDeviceTask gAvrcpDeviceSettings;
static const TaskData avrcpInitTask = {avrcpInitHandler};

/* The global node pointer for the AVRCP instance */
avrcpList *avrcpListHead = 0;  /* Pointer located in global space */

/****************************************************************************
 *NAME    
 *  avrcpInitProfile 
 *
 *DESCRIPTION
 *  Initialize the profile settings 
 **************************************************************************/
static void avrcpInitProfile( avrcp_device_role role,
                             uint8 tg_features, 
                             uint8 ct_features, 
                             uint8 extensions )
{
    /* Feature bits and extensions need to be aligned only for 
       Target Category 1 or 3 */
    if(tg_features & (AVRCP_CATEGORY_1 | AVRCP_CATEGORY_3))
    {
        /* AVRCP_VERSION_1_3 support must be turned ON to support 
           Player application settings. Group Navigation support is 
           mandatory for v1.3 */
        if(extensions & AVRCP_VERSION_1_3)
        {
            tg_features |= AVRCP_GROUP_NAVIGATION;

            if((extensions & AVRCP_VERSION_1_4) == AVRCP_VERSION_1_3)
            {
                /* Version 1.4 features are not supported */
                tg_features &= AVRCP_1_4_EXTENSION_MASK;
            }
        }
        else
        {
            /* None of the extensions are valid */
            extensions = 0;

            /* version 1.3 features are not supported */
            tg_features &= AVRCP_1_3_EXTENSION_MASK;
        }
    }

    /* Set the SDP Bit for Browsing if the controller supports 
       Browsing channel */
    if((extensions & AVRCP_BROWSING_SUPPORTED) == AVRCP_BROWSING_SUPPORTED)
    {
        ct_features |= AVRCP_VIRTUAL_FILE_SYSTEM_BROWSING;
         
    }

     /* Set the Supported features values */
    gAvrcpDeviceSettings.bitfields.device_type = role;
    gAvrcpDeviceSettings.bitfields.local_target_features = tg_features;
    gAvrcpDeviceSettings.bitfields.local_controller_features = ct_features; 
    gAvrcpDeviceSettings.bitfields.local_extensions = extensions;
    gAvrcpDeviceSettings.bitfields.unused = 0;


}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalInitReq
 *
 *DESCRIPTION
 *   Handle the internal initialization. Register with L2CAP and initialize
 *   browsing. 
 **************************************************************************/
void avrcpHandleInternalInitReq( const AVRCP_INTERNAL_INIT_REQ_T *req)
{
    uint16 psm = AVCTP_PSM;
    gAvrcpDeviceSettings.app_task = req->connectionTask;
    avrcpInitProfile(req->init_params.device_type,
                    req->init_params.supported_target_features,
                    req->init_params.supported_controller_features,
                    req->init_params.profile_extensions);

    /* If either browsing is supported or if CAT-1 or CAT-3 Target, register 
      * for BROWSING PSM with L2CAP */
   if((isAvrcpBrowsingEnabled(avrcpGetDeviceTask())) ||
       (isAvrcpTargetCat1Supported(avrcpGetDeviceTask())) ||
       (isAvrcpTargetCat3Supported(avrcpGetDeviceTask())))
    {
        psm = AVCTP_BROWSING_PSM;
    }
    ConnectionL2capRegisterRequest( (Task)&avrcpInitTask, psm, 0);
}

/****************************************************************************
 *NAME    
 *   avrcpGetInitTask
 *
 *DESCRIPTION
 *  Get the Initailization Task; 
 **************************************************************************/
Task avrcpGetInitTask( void )
{
    return (Task)&avrcpInitTask;
}

/****************************************************************************
 *NAME    
 *   avrcpGetDeviceTask
 *
 *DESCRIPTION
 *  Get the device Task
 **************************************************************************/
AvrcpDeviceTask* avrcpGetDeviceTask(void)
{
    return &gAvrcpDeviceSettings;
}

/****************************************************************************
*NAME    
*  avrcpInitDefaults
*
*DESCRIPTION
*  Initialize the Default AVRCP task values.
*****************************************************************************/
void avrcpInitDefaults( AVRCP       *avrcp,
                        avrcpState   state)
{
    avrcpSetState(avrcp, state);
    avrcp->pending = avrcp_none;
    avrcp->bitfields.block_received_data = 0;
    avrcp->sink = 0;
    avrcp->bitfields.cmd_transaction_label = 1;
    avrcp->tg_registered_events = 0;
    avrcp->ct_registered_events = 0;
    avrcp->continuation_pdu = 0;
    avrcp->bitfields.fragment = avrcp_packet_type_single;
    avrcp->continuation_data = 0;
    avrcp->sdp_search_mode = (uint16)avrcp_sdp_search_none;
    avrcp->bitfields.sdp_search_repeat = 0;
    avrcp->av_msg = NULL;    
    avrcp->av_msg_len = 0;
    avrcp->data_app_ind = 0;
    avrcp->bitfields.remote_features = 0;
    avrcp->bitfields.remote_extensions = 0;
    avrcp->av_max_data_size = AVRCP_AVC_MAX_DATA_SIZE;
}


/****************************************************************************
*NAME    
*  avrcpAddTaskToList
*
*DESCRIPTION
*  Add the AVRCP Task to the list.
******************************************************************************/
static void avrcpAddTaskToList(AVRCP *avrcp, const bdaddr *bd_addr)
{
    avrcpList *newList;

    /* Create an AVRCP instance node. */
    newList = PanicUnlessMalloc(sizeof(avrcpList));
    memset(newList, 0, sizeof(newList));

    /* Associate AVRCP task with the Instance. */
    newList->avrcp = avrcp;

    /* Store the bluetooth address of the device. */
    if(bd_addr)
    {
        memcpy(&(newList->avrcp->bd_addr),
            bd_addr,
            sizeof(bdaddr));
    }
    newList->next = NULL;

    if (avrcpListHead == NULL)
    {
        /* Head of the AVRCP node. */
        avrcpListHead = newList;
        avrcpListHead->next = NULL;
    }
    else
    {
        /* AVRCP node already exist so add the node at the head. */
        newList->next = avrcpListHead;
        avrcpListHead = newList;
    }
}


/****************************************************************************
*NAME
*  avrcpFindTask
*
*DESCRIPTION
*  Find the AVRCP Task from BT address.
*/
AVRCP *avrcpFindTask(const bdaddr *bd_addr)
{
    avrcpList *list = avrcpListHead;
    while (list != NULL)
    {
        AVRCP *avrcp = list->avrcp;

        /* Check whether the connetion exist for this particular device address. */
        if (BdaddrIsSame(bd_addr, &avrcp->bd_addr))
            return avrcp;

        list = list->next;
    }
    return NULL;
}


/****************************************************************************
*NAME    
*  avrcpCreateTask
*
*DESCRIPTION
*  Create the AVRCP Task and initiaze the default values.
******************************************************************************/
AVRCP *avrcpCreateTask( Task         client, 
                        avrcpState   state,
                        const bdaddr *bd_addr)
{
    AVRCP_AVBP_INIT    *shared_block;
    AVRCP              *avrcp;

    AVRCP_INFO(("avrcpCreateTask, creating task\n"));

    /* Create a new Task */
     /* If either browsing is supported or if CAT-1 or CAT-3 Target, initialize
       * the browsing handler*/
    if((isAvrcpBrowsingEnabled(avrcpGetDeviceTask())) ||
        (isAvrcpTargetCat1Supported(avrcpGetDeviceTask())) ||
        (isAvrcpTargetCat3Supported(avrcpGetDeviceTask())))
    {
        shared_block = PanicUnlessNew(AVRCP_AVBP_INIT);
        avrcp = &shared_block->avrcp;

        /* Initalize browsing */
        avrcpInitBrowsing(shared_block);
    }
    else
    {
        avrcp = PanicUnlessNew(AVRCP);

        /* No browsing supported */
        avrcp->avbp_task = NULL;
    }

    /* Set the handler function */
    avrcp->task.handler = avrcpProfileHandler;

    /* Set the static Task data */
    avrcp->clientTask = client;   
 
    /* Set the Intial configuration Data */
    avrcp->l2cap_mtu = AVRCP_MTU_DEFAULT; 

    /* Initialize the default values */
    avrcpInitDefaults(avrcp, state);

    /* Initialize datacleanup handler */
    (avrcp->dataFreeTask.cleanUpTask).handler = avrcpDataCleanUp;
    avrcp->dataFreeTask.sent_data = 0;

    /* Create the  Avrcp instance. */
    avrcpAddTaskToList(avrcp, bd_addr);

    return avrcp;
}

/****************************************************************************
*NAME    
*   avrcpResetValues
*
*DESCRIPTION
*  Reset the local state values to their initial states. 
*    
******************************************************************************/
void avrcpResetValues(AVRCP* avrcp)
{

    /* Reset the Browsing channel entries if it is present */
    if(avrcp->avbp_task)
    {
       avrcpResetAvbpValues((AVBP*)avrcp->avbp_task);      
    }

    /* Free any memory that may be allocated */
    avrcpSourceProcessed(avrcp, TRUE);

   /* Reset the local state values to their initial states */
    avrcpInitDefaults(avrcp, avrcpReady);

    /* Cancel all pending messages */
    MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
    MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT);
    MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_CONNECT_REQ);

}


/****************************************************************************
*NAME    
*    avrcpResetAvbpValues
*
*DESCRIPTION
*   Reset the browsing channel Task 
****************************************************************************/
void avrcpResetAvbpValues(AVBP* avbp)
{
    if(!avbp) return;

    /* Reset the local Values to their initial states */
    avrcpSetAvbpState(avbp, avbpReady);
    avbp->avbp_sink = 0;
    avbp->avbp_sink_data = 0;
    avbp->avbp_mtu = AVRCP_MTU_DEFAULT;    /* 672 */ 
    avbp->blocking_cmd = AVRCP_INVALID_PDU_ID;
    avbp->bitfields.trans_id = 0;
    
    /* Cancel all AVBP Timers if running */
    MessageCancelAll(&avbp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
    MessageCancelAll(&avbp->task, AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT);
}


/****************************************************************************
*NAME    
*   avrcpInitBrowsing
*
*DESCRIPTION
*    Initialise all members of the AVBP task data structure and register 
*    AVCTP_Browsing PSM with L2CAP. 
****************************************************************************/
void avrcpInitBrowsing(AVRCP_AVBP_INIT *shared_init_block)
{
    AVBP   *avbp= &shared_init_block->avbp;
    AVRCP  *avrcp = &shared_init_block->avrcp;

    /* Set Browsing Channel Profile Handler */
    avbp->task.handler = avbpProfileHandler;

    /* Associated AVRCP Task. */
    avbp->avrcp_task = &avrcp->task;
    avrcpResetAvbpValues(avbp);

    /* Associate the AVBP task with AVRCP */
    avrcp->avbp_task = &avbp->task;
}

/****************************************************************************
*NAME    
*   avrcpSendInitCfmToClient
*
*DESCRIPTION
*  Post processing of init procedure. Set the states and send AVRCP_INIT_CFM 
*  to the application.
****************************************************************************/
void avrcpSendInitCfmToClient( avrcp_unreg_status   state,
                               uint32             sdp_handle, 
                               avrcp_status_code  status)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INIT_CFM);
    message->status = status;
    message->sdp_handle = sdp_handle;

    if(status == avrcp_fail)
    {
        switch( state )
        {
        case avrcp_unreg_all:
            ConnectionL2capUnregisterRequest((Task)&avrcpInitTask,AVCTP_PSM);

        case avrcp_unreg_browse: /* fall through */
            if((isAvrcpBrowsingEnabled(avrcpGetDeviceTask())) ||
                (isAvrcpTargetCat1Supported(avrcpGetDeviceTask())) ||
                (isAvrcpTargetCat3Supported(avrcpGetDeviceTask())))
            {
                ConnectionL2capUnregisterRequest((Task)&avrcpInitTask,
                                                 AVCTP_BROWSING_PSM);
            }

        default:
            break;
        }
    }
                                         
    MessageSend((Task)gAvrcpDeviceSettings.app_task, AVRCP_INIT_CFM, message);
}


                
/***************************************************************************
*NAME    
*   AvrcpInit
*
*DESCRIPTION
*   Initialize AVRCP 
****************************************************************************/
void AvrcpInit(Task theAppTask, const avrcp_init_params *config)
{
    /* Must pass in valid configuration parameters. */
    if (!config || !theAppTask)
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INIT_CFM);
        message->status = avrcp_fail;
        MessageSend(theAppTask, AVRCP_INIT_CFM, message);
        return;
    }
    else
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_INIT_REQ);
        message->connectionTask = theAppTask;
        message->init_params = *config;

        /* Configure the device type */
#ifdef AVRCP_CT_ONLY_LIB 
        message->init_params.device_type = avrcp_ct;
        message->init_params.supported_target_features = 0;
#else 
#ifdef AVRCP_TG_ONLY_LIB
        message->init_params.device_type = avrcp_tg;
        message->init_params.supported_controller_features = 0;
#endif /* AVRCP_TG_ONLY_LIB */
#endif /* AVRCP_CT_ONLY_LIB */  

        MessageSend((Task) &avrcpInitTask, AVRCP_INTERNAL_INIT_REQ, message);
    }
}

/****************************************************************************
*NAME    
*   avrcpDeleteTaskFromList
*
*DESCRIPTION
*   Delete a dynamically allocated AVRCP LIst. 
*****************************************************************************/
static void avrcpDeleteTaskFromList(AVRCP *avrcp)
{
    avrcpList *temp;
    avrcpList *prev;
    temp = avrcpListHead;
    prev = avrcpListHead;

    /* Check whether list is present. return if not. */
    if (temp == NULL)
        return;

    if (temp != NULL && temp->avrcp == avrcp)
    {
        avrcpListHead = temp->next;   /* Changed head */
        free(temp);               /* free old head */
        return;
    }

    /* Search for the key to be deleted, keep track of the
     * previous node as we need to change 'prev->next' */
    while (temp != NULL && temp->avrcp != avrcp)
    {
        prev = temp;
        temp = temp->next;
    }

    /* If List was not present in linked list */
    if (temp == NULL) return;

    /* Unlink the node from linked list. */
    prev->next = temp->next;

    /* Free memory. */
    free(temp);
}

/****************************************************************************
*NAME    
*   avrcpHandleDeleteTask
*
*DESCRIPTION
*   Delete a dynamically allocated AVRCP and AVBP task instance. 
*   Before deleting make  sure all messages for that task are flushed 
*   from the message queue.
*****************************************************************************/
void avrcpHandleDeleteTask(AVRCP *avrcp)
{

    if(avrcp->sdp_search_mode != avrcp_sdp_search_none)
    {
        MessageSendConditionally(&avrcp->task, AVRCP_INTERNAL_TASK_DELETE_REQ, 0,
                                                (uint16*)&avrcp->sdp_search_mode);
    }
    else
    {
        /* Delete Browsing Channel Task */
        if(avrcp->avbp_task)
        {
            MessageFlushTask(avrcp->avbp_task);
        }
     
        /* Delete Control Channel Task */
        MessageFlushTask(&avrcp->task);

        avrcpDeleteTaskFromList(avrcp);
        free(avrcp);
    }
}


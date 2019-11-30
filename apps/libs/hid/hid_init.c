/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    init.c        
DESCRIPTION
    This file contains the initialisation code for the Spp profile library.
*/

#include "hid_profile_handler.h"
#include "hid_init.h"
#include "hid_sdp.h"
#include "hid_private.h"
#include "hid_common.h"

/****************************************************************************
NAME 
    HidInit 
DESCRIPTION
    Initialise the HID library.
RETURNS
    void
*/
void HidInit(Task theAppTask, const hid_config *config)
{
    HID_LIB *hidLib = PanicUnlessNew(HID_LIB);
    MAKE_HID_MESSAGE(HID_INTERNAL_INIT_REQ);    
            
    /* Clear flags */
    hidLib->interrupt_psm = hidLib->control_psm = FALSE;
#ifdef HID_DEVICE
    hidLib->sdp_record = FALSE;
#else
    hidLib->sdp_record = TRUE;
#endif    

    /* Set the handler function */
    hidLib->task.handler = hidLibProfileHandler;
 
    /* Store the app task so we know where to return responses */
    hidLib->app_task = theAppTask;
 
    /* Clear HID instance linked list */
    hidLib->list = NULL;
    
    /* Send an internal init message to kick off initialisation */
    message->config = config;
    MessageSend(&hidLib->task, HID_INTERNAL_INIT_REQ, message);
}


/****************************************************************************
NAME 
    hidHandleInternalInitReq
DESCRIPTION
    Send internal init req message 
RETURNS
    void
*/
void hidHandleInternalInitReq(HID_LIB *hidLib, const HID_INTERNAL_INIT_REQ_T *req)
{
    HID_PRINT(("hidHandleInternalInitReq\n"));    
    
    /* Register HID PSMs with the Connection library */
    ConnectionL2capRegisterRequest(&hidLib->task, HID_PSM_CONTROL, 0);
    ConnectionL2capRegisterRequest(&hidLib->task, HID_PSM_INTERRUPT, 0);
    
#ifdef HID_DEVICE
    /* Register service record */
    ConnectionRegisterServiceRecord(&hidLib->task, req->config->service_record_size, req->config->service_record);
#endif
}

/****************************************************************************
NAME	
    hidHandleInternalInitCfm
DESCRIPTION
    Send init cfm message to App.
RETURNS
    void
*/
void hidHandleInternalInitCfm(HID_LIB *hid_lib, const HID_INTERNAL_INIT_CFM_T *cfm)
{
    MAKE_HID_MESSAGE(HID_INIT_CFM);    
    HID_PRINT(("hidHandleInternalInitCfm\n"));
    
    /* Just send a confirm back to the App */
    message->status = cfm->status;
    message->hid_lib = hid_lib;
    MessageSend(hid_lib->app_task, HID_INIT_CFM, message);
}

/****************************************************************************
NAME	
    hidHandleInternalInitCfm
DESCRIPTION
    Send internal init cfm message to HID task.
RETURNS
    void
*/
void hidSendInternalInitCfm(HID_LIB *hidLib, hid_init_status status)
{
    /* Only send message if initialisation failed or we have initialised everything */
    if ((status != hid_init_success) || 
        (hidLib->sdp_record && hidLib->interrupt_psm && hidLib->control_psm))
    {   
        MAKE_HID_MESSAGE(HID_INTERNAL_INIT_CFM);    
            
        /* Send internal init confirm message */
        message->status = status;	
        MessageSend(&hidLib->task, HID_INTERNAL_INIT_CFM, message);
    }
}


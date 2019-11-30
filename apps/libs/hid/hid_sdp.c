/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_sdp.c        

DESCRIPTION
    

NOTES

*/

#include <assert.h>
#include <panic.h>
#include <string.h>
#include <service.h>

#include "hid_common.h"
#include "hid_sdp.h"
#include "hid_init.h"


/****************************************************************************
NAME	
    hidHandleSdpRegisterCfm

DESCRIPTION
    Outcome of HID service register request.

RETURNS
    void
*/
void hidHandleSdpRegisterCfm(HID_LIB *hidLib, const CL_SDP_REGISTER_CFM_T *cfm)
{
    HID_PRINT(("hidHandleSdpRegisterCfm\n"));    
            
    /* Check if SDP registration was successful */
    if (cfm->status == success)
    {
        /* Service record register succeeded */     
        /* Store the service handle */
        hidLib->sdp_record = TRUE;
        
        /* Send internal init confirm (only sent if initialisation complete) */
        hidSendInternalInitCfm(hidLib, hid_init_success);
    }
    else
    {
        /* Service record register failed - we don't get more detail than this! */
        /* Reset the service handle */
        hidLib->sdp_record = FALSE;
        
        /* Send internal init confirm */
        hidSendInternalInitCfm(hidLib, hid_init_sdp_reg_fail);
    }   
}

/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_hs.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"
#include "hfp_common.h"

/****************************************************************************
NAME    
    HfpHsButtonPressRequest

DESCRIPTION
    Send button press command if AG is connected using HSP

MESSAGE RETURNED
    HFP_HS_BUTTON_PRESS_CFM,

RETURNS
    void
*/
void HfpHsButtonPressRequest(hfp_link_priority priority)
{
    hfpSendCommonInternalMessagePriority(HFP_INTERNAL_AT_CKPD_REQ, priority);
}

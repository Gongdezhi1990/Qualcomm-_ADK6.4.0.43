/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_hs_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_hs_handler.h"
#include "hfp_send_data.h"
#include "hfp_link_manager.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME    
    hfpSendHsButtonPress

DESCRIPTION
    Send a button press AT cmd to the AG.

RETURNS
    void
*/
void hfpSendHsButtonPress(hfp_link_data* link, hfp_at_cmd pending_cmd)
{
    char ckpd[] = "AT+CKPD=200\r";
    hfpSendAtCmd(link, sizeof(ckpd)-1, ckpd, pending_cmd);
}


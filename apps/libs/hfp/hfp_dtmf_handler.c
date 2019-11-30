/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_dtmf_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_dtmf_handler.h"
#include "hfp_send_data.h"

#include <stdio.h>
#include <string.h>


/****************************************************************************
NAME    
    hfpHandleDtmfRequest

DESCRIPTION
    Handle a request to send a DTMF tone to the AG.

RETURNS
    void
*/
void hfpHandleDtmfRequest(const HFP_INTERNAL_AT_VTS_REQ_T *req)
{
    char dtmf[20];
    sprintf(dtmf, "AT+VTS=%c\r", req->dtmf);

    /* Send the AT cmd over the air */
    hfpSendAtCmd(req->link, (uint16)strlen(dtmf), dtmf, hfpVtsCmdPending);
}

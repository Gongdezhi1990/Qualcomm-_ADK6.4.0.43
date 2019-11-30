/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_extended_error_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_extended_error_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME    
    hfpHandleExtendedErrorReq

DESCRIPTION
    Request that extended error reporting is enabled on the AG.

RETURNS
    void
*/
void hfpHandleExtendedErrorReq(hfp_link_data* link)
{
    /* Only send the cmd if the AG supports the extended error result codes feature.*/
    if(theHfp->bitfields.extended_errors && agFeatureEnabled(link, AG_EXTENDED_ERROR_CODES))
    {
        char cmee[] = "AT+CMEE=1\r";
    
        /* Send the AT cmd over the air */
        hfpSendAtCmd(link, sizeof(cmee)-1, cmee, hfpCmeeCmdPending);
    }
    else
    {
        /* Send CSRSF */
        hfpSendCommonInternalMessage(HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ, link);
    }
}


/****************************************************************************
NAME    
    hfpHandleCmeeAtAck

DESCRIPTION
    Extended error reporting request command has been acknowledged by the AG.

RETURNS
    void
*/
void hfpHandleCmeeAtAck(hfp_link_data* link)
{
    /* Send CSRSF */
    hfpSendCommonInternalMessage(HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ, link);
}

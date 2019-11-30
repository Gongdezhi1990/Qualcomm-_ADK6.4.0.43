/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_nrec_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_send_data.h"
#include "hfp_nrec_handler.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME    
    hfpHandleNrEcDisable

DESCRIPTION
    Send a request to the AG to disable its Noise Reduction (NR) and Echo
    Cancellation (EC) capabilities.

RETURNS
    void
*/
void hfpHandleNrEcDisable(hfp_link_data* link)
{
    /* Only send the AT cmd if the AG and HFP support this functionality */
    if(theHfp->bitfields.disable_nrec && agFeatureEnabled(link, AG_NREC_FUNCTION) && hfFeatureEnabled(HFP_NREC_FUNCTION))
    {
        /* Send the AT cmd over the air */
        char nrec[] = "AT+NREC=0\r";
        hfpSendAtCmd(link, sizeof(nrec)-1, nrec, hfpNrecCmdPending);
    }
    else
    {
        /* Request response and hold status if AG supports it */
        hfpSendCommonInternalMessage(HFP_INTERNAL_AT_BTRH_STATUS_REQ, link);
    }
}


/****************************************************************************
NAME    
    hfpHandleNrecAtAck

DESCRIPTION
    Received an ack from the AG for the AT+NREC cmd.

RETURNS
    void
*/
void hfpHandleNrecAtAck(hfp_link_data* link)
{
    /* Request response and hold status if AG supports it */
    hfpSendCommonInternalMessage(HFP_INTERNAL_AT_BTRH_STATUS_REQ, link);
}


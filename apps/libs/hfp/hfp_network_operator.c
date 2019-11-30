/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_network_operator.c        

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"

#include <panic.h>


/****************************************************************************
NAME    
    HfpNetworkOperatorRequest

DESCRIPTION
    Requests network operator information from the AG. An SLC for the supplied
    profile priority must already be established before calling this
    function. The network operator information received from the AG will be
    sent to the application using a HFP_NETWORK_OPERATOR_IND message.  The 
    application will receive a HFP_NETWORK_OPERATOR_CFM message to indicate 
    that the request has been completed.

MESSAGE RETURNED
    HFP_NETWORK_OPERATOR_CFM

RETURNS
    void
*/
void HfpNetworkOperatorRequest(hfp_link_priority priority)
{
    hfpSendCommonInternalMessagePriority(HFP_INTERNAL_AT_COPS_REQ, priority);
}

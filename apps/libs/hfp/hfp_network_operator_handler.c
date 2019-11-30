/****************************************************************************
Copyright (c) 2005 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_network_operator_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_network_operator_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"
#include "hfp_link_manager.h"

#include <panic.h>
#include <string.h>

#define HFP_COPS_LONG_ALPHANUMERIC  0   /* 16 char max */
#define HFP_COPS_SHORT_ALPHANUMERIC 1   /* 8 char max  */
#define HFP_COPS_NUMERIC            2   /* Numeric ID  */

static void handleNetworkOperator(hfp_link_data* link, uint8 mode, uint16 format, uint16 size_name, const uint8 *name)
{
    /* size_name should be <= 16 but limit array size incase */
    if(size_name >= HFP_MAX_ARRAY_LEN)
        size_name = HFP_MAX_ARRAY_LEN - 1;
    
    /* Only supply operator name if it's in the correct format */
    if (format != HFP_COPS_LONG_ALPHANUMERIC)
        size_name = 0;
        
    /* Silently ignore AT command if we are not HFP */
    if (hfpLinkIsHfp(link))
    {
        /* Allow room to NULL terminate string */
        MAKE_HFP_MESSAGE_WITH_LEN(HFP_NETWORK_OPERATOR_IND, size_name + 1);
        message->priority = hfpGetLinkPriority(link);
        message->mode = mode;
        message->size_operator_name = size_name;
        memmove(message->operator_name, name, size_name);
        /* Make sure string is NULL terminated */
        message->operator_name[size_name] = '\0';
        
        MessageSend(theHfp->clientTask, HFP_NETWORK_OPERATOR_IND, message);
    }
}


static void sendNetworkOperatorReportingFormat(hfp_link_data* link)
{
    /* Set COPS format to HFP_COPS_LONG_ALPHANUMERIC */
    char cops[] = "AT+COPS=3,0\r";

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, sizeof(cops)-1, cops, hfpCopsFormatCmdPending);
}


static void sendNetworkOperatorRequest(hfp_link_data* link)
{
    char cops[] = "AT+COPS?\r";

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, sizeof(cops)-1, cops, hfpCopsReqCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleNetworkOperatorReq

DESCRIPTION
    Request network operator information from the AG.

RETURNS
    void
*/
void hfpHandleNetworkOperatorReq(hfp_link_data* link)
{
    if (link->bitfields.at_cops_format_set)
    {    /* Reporting format already set, just issue a request for information */
        sendNetworkOperatorRequest(link);
    }
    else
    {    /* Need to set the reporting format first */
        sendNetworkOperatorReportingFormat(link);
    }
}


/****************************************************************************
NAME    
    hfpHandleCopsFormatAtAck

DESCRIPTION
    Network operator reporting format command has been acknowledged (completed)
    by the AG.

RETURNS
    void
*/
MessageId hfpHandleCopsFormatAtAck(hfp_link_data* link, hfp_lib_status status)
{
    if (status == hfp_success)
    {    /* Reporting format now set, issue request for information */
        link->bitfields.at_cops_format_set = TRUE;
        sendNetworkOperatorRequest(link);
        return HFP_NO_CFM;
    }
#ifndef HFP_MIN_CFM
    return HFP_NETWORK_OPERATOR_CFM;
#else
    return HFP_NO_CFM;
#endif
}


/****************************************************************************
NAME    
    hfpHandleNetworkOperatorMode

DESCRIPTION
    Network operator info indication received from the AG containing only the
    network mode.

AT INDICATION
    +COPS

RETURNS
    void
*/
void hfpHandleNetworkOperatorMode(Task link_ptr, const struct hfpHandleNetworkOperatorMode *ind)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    handleNetworkOperator(link, (uint8)ind->mode, 0, 0, 0);
}


/****************************************************************************
NAME    
    hfpHandleNetworkOperatorModeName

DESCRIPTION
    Network operator info indication received from the AG containing both the
    network mode and operator name.

AT INDICATION
    +COPS

RETURNS
    void
*/
void hfpHandleNetworkOperatorModeName(Task link_ptr, const struct hfpHandleNetworkOperatorModeName *ind)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    handleNetworkOperator(link, (uint8)ind->mode, ind->format, ind->operator.length, ind->operator.data);
}


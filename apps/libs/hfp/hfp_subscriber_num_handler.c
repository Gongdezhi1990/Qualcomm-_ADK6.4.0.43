/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_subscriber_num_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_subscriber_num_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"
#include "hfp_link_manager.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME    
    hfpHandleSubscriberNumberGetReq

DESCRIPTION
    Request subscriber number information from the AG.

RETURNS
    void
*/
void hfpHandleSubscriberNumberGetReq(hfp_link_data* link)
{
    const char cnum[] = "AT+CNUM\r";

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, sizeof(cnum)-1, cnum, hfpCnumCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleSubscriberNumber

DESCRIPTION
    Subscriber number info indication received from the AG

AT INDICATION
    +CNUM

RETURNS
    void
*/
void hfpHandleSubscriberNumber(Task link_ptr, const struct hfpHandleSubscriberNumber *ind)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    uint16 size_number = ind->number.length;
    
    if(size_number >= HFP_MAX_ARRAY_LEN)
        size_number = HFP_MAX_ARRAY_LEN - 1;
        
    {
        /* Allow room to NULL terminate string */
        MAKE_HFP_MESSAGE_WITH_LEN(HFP_SUBSCRIBER_NUMBER_IND, size_number + 1);
        message->priority = hfpGetLinkPriority(link);
        message->service = (hfp_subscriber_service)ind->service;
        message->number_type = hfpConvertNumberType(ind->type);
        message->size_number = size_number;
        memmove(message->number, ind->number.data, size_number);
        message->number[size_number] = '\0';
        MessageSend(theHfp->clientTask, HFP_SUBSCRIBER_NUMBER_IND, message);
    }
}

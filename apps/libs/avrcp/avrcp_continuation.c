/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_continuation.c

DESCRIPTION
   APIs for Continuation feature. 

NOTES

*/


/****************************************************************************
    Header files
*/

#include <panic.h>
#include "avrcp_metadata_transfer.h"


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */

/****************************************************************************
*NAME    
*    AvrcpRequestContinuingResponseRequest
*
*DESCRIPTION
*  API function to request Continuing Response.
*    
*PARAMETERS
*   avrcp            - Task
*   pdu_id           - PDU_ID of the received fragmented message which requires
*                      a continuation response. 
*
*RETURN
*  AVRCP_REQUEST_CONTINUING_RESPONSE_CFM 
*******************************************************************************/
void AvrcpRequestContinuingResponseRequest(AVRCP *avrcp, uint16 pdu_id)
{
    uint8 params[] = {0};
    avrcp_status_code status;

    params[0] = pdu_id & 0xFF;

    if(avrcp->bitfields.fragment != avrcp_packet_type_single) 
    {
        /* Ignore the request from app since the library has not received
           the complete packet */
        avrcpUnblockReceivedData(avrcp);
        return;
    }

    status = avrcpMetadataStatusCommand(avrcp, 
                                    AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID,
                                    avrcp_request_continuation, 1, 
                                    params, 0,0);

    if (status != avrcp_success)
    {
        MAKE_AVRCP_MESSAGE(AVRCP_REQUEST_CONTINUING_RESPONSE_CFM);

        message->avrcp = avrcp;
        message->status = status;
        message->pdu_id = params[0];

#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
        message->transaction = 0;
#endif
        MessageSend(avrcp->clientTask, 
                    AVRCP_REQUEST_CONTINUING_RESPONSE_CFM, message);
    }
}


/****************************************************************************
*NAME    
*    AvrcpAbortContinuing    
*
*DESCRIPTION
*  API function to Abort Continuing Response.
*    
*PARAMETERS
*   avrcp            - Task
*   pdu_id           - PDU_ID of the received fragmented message to abort.
*
*RETURN
*  AVRCP_ABORT_CONTINUING_RESPONSE_CFM 
*******************************************************************************/
void AvrcpAbortContinuingResponseRequest(AVRCP *avrcp, uint16 pdu_id)
{
    uint8 params[] = {0};
    avrcp_status_code status;

    params[0] = pdu_id & 0xFF;

    avrcp->bitfields.fragment = avrcp_packet_type_end;
    avrcpUnblockReceivedData(avrcp);

    status = avrcpMetadataStatusCommand(avrcp,
                                  AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID, 
                                  avrcp_abort_continuation, 1, params, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendCommonMetadataCfm(avrcp, status, 
                                   AVRCP_ABORT_CONTINUING_RESPONSE_CFM);
    }
}
#endif /* !AVRCP_TG_ONLY_LIB */


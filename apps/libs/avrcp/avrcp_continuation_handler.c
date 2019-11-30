/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_continuation_handler.c

DESCRIPTION
   Internal functions handling the Continuation feature.  

NOTES

*/


/****************************************************************************
    Header files
*/
#include <source.h>
#include <panic.h>

#include "avrcp_continuation_handler.h"
#include "avrcp_metadata_transfer.h"

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
 *NAME    
 *    abortContinuation    
 *
 *DESCRIPTION
 *   Abort the stored Continuation response packet. 
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 *****************************************************************************/
void abortContinuation(AVRCP *avrcp)
{
    /* Abort sending continuing packets back to CT. */
    if (avrcp->continuation_data)
        SourceEmpty(avrcp->continuation_data);

    MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET);
    avrcp->continuation_pdu = 0;
}

/****************************************************************************
 *NAME    
 *    avrcpHandleRequestContinuingCommand    
 *
 *DESCRIPTION
 *   Handle the continuing command received from the CT.
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 * pdu          -       PDU ID of the Continuation response.
 *****************************************************************************/
void avrcpHandleRequestContinuingCommand(AVRCP *avrcp, uint16 pdu)
{
    /* Make sure the continuing PDU requested is the same as the one we 
       are storing data for. */
    if (avrcp->continuation_pdu == pdu)
    {
        /* A message will be sent on this variable being reset, 
           if there is more data to be sent. */
        avrcp->continuation_pdu = 0;
    }
    else
    {
        avrcpUnblockReceivedData(avrcp);
    }
}

/****************************************************************************
 *NAME    
 *    avrcpHandleAbortContinuingCommand    
 *
 *DESCRIPTION
 *  Handle the abort continuing command received from the CT.
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 * pdu          -       PDU ID of the Continuation response to be aborted.
 *****************************************************************************/
void avrcpHandleAbortContinuingCommand(AVRCP *avrcp, uint16 pdu)
{
    /* Send response back stating if the abort was successful. */
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_ABORT_CONTINUING_RES);

    if (avrcp->continuation_pdu == pdu)
    {
        /* Forget about the data stored. */
        abortContinuation(avrcp);
        message->response = avctp_response_accepted;
    }
    else
    {
        /* The abort continuing PDU requested is not the same as the
           one we are storing data for, so reject. */
        message->response = avctp_response_rejected;
    }

    MessageSend(&avrcp->task, AVRCP_INTERNAL_ABORT_CONTINUING_RES, message);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalAbortContinuingResponse    
 *
 *DESCRIPTION
 *  Prepare to send abort continuing response to the CT.
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 * res          -       Internal request Abort Continuation response.
 *****************************************************************************/
void avrcpHandleInternalAbortContinuingResponse(AVRCP       *avrcp, 
                const AVRCP_INTERNAL_ABORT_CONTINUING_RES_T *res)
{
    /* Send a response to the abort continuing request. */
    avrcpSendMetadataResponse(avrcp,  res->response, 
                              AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID, 0, 
                              avrcp_packet_type_single, 0, 0, 0);

}

/****************************************************************************
 *NAME    
 *    avrcpStoreNextContinuationPacket    
 *
 *DESCRIPTION
 * Store the fragmented metadata packet for Continuation response 
 * at the Target.
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 * Source       -       Source data from the application to be send.
 * param_length -       length of data in the Source.
 * pdu_id       -       AVRCP command PDU waiting for continuation request.
 * response     -       AVCTP response. 
 * packet_size  -       size of the request packet
 *****************************************************************************/
void avrcpStoreNextContinuationPacket(  AVRCP               *avrcp, 
                                        Source              data, 
                                        uint16              param_length, 
                                        uint16              pdu_id, 
                                        uint16              response)
{
    /* Store futher fragments until the CT request them. */
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET);
    message->data = data;
    message->param_length = param_length;
    message->pdu_id = pdu_id;
    message->response = response;
   
    /* Store the current pdu id for conditionall trigger on receiving 
       Request Continuation command from the peer. avrcp->continuation_pdu
       always contain either 0 or same pdu_id. So there is no seperate  
       check require before overwriting. If there was any continuation PDU 
       pending it must have be cleared when it received any other command
       other than Request Continuation or Abort Continuation command from the 
       Controller. */ 
    avrcp->continuation_pdu = pdu_id;
    avrcp->continuation_data = data;

    MessageSendConditionally(&avrcp->task,
                             AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET,
                             message, &avrcp->continuation_pdu);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleNextContinuationPacket    
 *
 *DESCRIPTION
 * Prepare to send the next packet of fragmented data, that the CT 
 * has requested.
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 * ind          -       Internal request for next Continuation packet.
 *****************************************************************************/
void avrcpHandleNextContinuationPacket(AVRCP                    *avrcp, 
                const AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET_T *ind)
{
    /* The next fragment has been requested, so send it. */
    uint16 data_length = avrcp->av_max_data_size;
    avrcp_packet_type packet_type = avrcp_packet_type_continue;

    if (ind->param_length < data_length)
    {
       /* This is the last fragment to be sent. */
       data_length = ind->param_length;
       packet_type = avrcp_packet_type_end;
       avrcp->continuation_pdu = ind->pdu_id;
    }
    else
    {
       /* There are more fragments to be sent, 
          store the data for the following fragments. */
       avrcpStoreNextContinuationPacket(avrcp, 
                               ind->data, 
                               ind->param_length - data_length,
                               ind->pdu_id,
                               ind->response);
    }

    /* This is a fragmented response. */
    avrcpSendMetadataResponse(avrcp,  (avrcp_response_type)ind->response, 
                              (uint8)ind->pdu_id, ind->data, 
                              packet_type, data_length, 0, 0);

}

#endif /* !AVRCP_CT_ONLY_LIB*/

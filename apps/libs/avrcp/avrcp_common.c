/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_common.c

DESCRIPTION
    This file defines all common utility functions used through out the library.

NOTES

*/


/****************************************************************************
    Header files
*/
#include <stream.h>
#include <sink.h>
#include <source.h>
#include <panic.h>
#include <stdlib.h>
#include <message.h>
#include <string.h>
#include <vmal.h>
#include "avrcp_private.h"
#include "avrcp_common.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
* NAME    
*  avrcpUpdateTransactionLabel
*
* DESCRIPTION
*  update the transaction label.
*
***************************************************************************/
static void avrcpUpdateTransactionLabel( AVRCP* avrcp )
{
    unsigned id = 0;
    unsigned events =  avrcp->ct_registered_events;
    uint8 tl = AVRCP_NEXT_TRANSACTION(avrcp->bitfields.cmd_transaction_label);
                                    
    while ((id < AVRCP_MAX_NUM_EVENTS) && events)
    {
        /* If we have a registered event, check for a collision */
        if( ( events & 0x01 ) &&
            ( avrcp->ct_notify_transaction_label[id] == tl ) )
        {
            /* Collided, so get the next id and re-start the check*/
            tl = AVRCP_NEXT_TRANSACTION( tl );

            id = 0;
            events = avrcp->ct_registered_events;
        }
        else
        {
            id++;
            events >>= 1;
        }
    }

    avrcp->bitfields.cmd_transaction_label= tl;
}


/****************************************************************************
* NAME    
*   avrcpGetNextTransactionLabel    
*
* DESCRIPTION
*  Get the transaction label and update it to the next unique label.
***************************************************************************/
uint8 avrcpGetNextTransactionLabel(AVRCP *avrcp)
{
   uint8 tl = avrcp->bitfields.cmd_transaction_label;

   /* Post update the transaction label to an unique label */
   avrcpUpdateTransactionLabel( avrcp ); 
   return tl;
}

/****************************************************************************
* NAME    
*   convertResponseToStatus    
*
* DESCRIPTION
*  Map the received response code to a corresponding Status value.
*
*******************************************************************************/
avrcp_status_code convertResponseToStatus(avrcp_response_type resp)
{
    /* Turn a response into a simplified state that the AVRCP library
     can return to the application. */
    switch (resp)
    {
    case avctp_response_accepted:
    case avctp_response_in_transition:
    case avctp_response_stable:
    case avctp_response_changed:
        return avrcp_success;
    case avctp_response_not_implemented:
    case avctp_response_bad_profile:
        return avrcp_unsupported;
    case avctp_response_interim:
        return avrcp_interim_success;
    case avctp_response_rejected:
        return avrcp_rejected;
    default:
        return avrcp_fail;
    }    
}

/****************************************************************************
*NAME    
*   avrcpSendCommonStatusCfm
*
*DESCRIPTION
*    Send a Common Status Message
*
*RETURNS
*    void
******************************************************************************/
void avrcpSendCommonStatusCfm(AVRCP*            avrcp,
                              avrcp_status_code status,
                              uint16            message_id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_STATUS_CFM);
    message->status = status;
    message->avrcp = avrcp;
    MessageSend(avrcp->clientTask, message_id, message);
}


/****************************************************************************
* NAME    
* avrcpSendCommonFragmentedMetadataCfm    
*
* DESCRIPTION
*   Send MetaData confirm to the CT application with the extracted data from 
*   the response
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendCommonFragmentedMetadataCfm(AVRCP         *avrcp, 
                                          avrcp_status_code  status,
                                          uint16         id, 
                                          uint16         metadata_packet_type,
                                          uint16         data_length, 
                                          const uint8*   data)
{
    uint16 offset=0;
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_FRAGMENTED_METADATA_CFM);
    
    message->avrcp = avrcp;
    message->status = status;

#ifdef AVRCP_ENABLE_DEPRECATED
    if(avrcp->av_msg)
    {
        message->transaction = (avrcp->av_msg[AVCTP_HEADER_START_OFFSET] >> 
                                AVCTP0_TRANSACTION_SHIFT);
    }
    else
    {
        message->transaction =0;
    }

    message->no_packets = 1;
    message->ctp_packet_type = AVCTP0_PACKET_TYPE_SINGLE;
    message->data_offset = 0;
#endif

    message->metadata_packet_type = metadata_packet_type;
    message->number_of_data_items = 0;
    message->data=0;
    message->size_data=0;

    if(!(status & AVRCP_ERROR_STATUS_BASE) && 
        (data_length))
    {
        if ((metadata_packet_type == avrcp_packet_type_single) || 
            (metadata_packet_type == avrcp_packet_type_start))
        {
            offset=1;
            message->number_of_data_items = data[0];
        }

        if(data_length > offset)
        {        
            message->size_data = data_length-offset;
            message->data = avrcpSourceFromConstData(avrcp, 
                                                 data+offset, 
                                                 message->size_data);
        }
    }

    MessageSend(avrcp->clientTask, id, message);
    return;
}

#ifdef AVRCP_ENABLE_DEPRECATED
/****************************************************************************
* NAME    
* avrcpSendCommonMetadataCfm    
*
* DESCRIPTION
* This is a deprecated function and will be replaced by 
* avrcpSendCommonStatusCfm
* 
* PARAMETERS
*    *response       - Application Response , standardised by this function
*   command_type    - Should be one of the values AVRCP0_CTYPE_CONTROL, 
*                        AVRCP0_CTYPE_STATUS or AVRCP0_CTYPE_NOTIFY
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendCommonMetadataCfm(AVRCP *avrcp, 
                                avrcp_status_code status, 
                                uint16 id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_METADATA_CFM_MESSAGE);

    message->avrcp = avrcp;
    message->status = status;

    if(avrcp->av_msg)
    {
        message->transaction = (avrcp->av_msg[AVCTP_HEADER_START_OFFSET] >> 
                                AVCTP0_TRANSACTION_SHIFT);
    }
    
    MessageSend(avrcp->clientTask, id, message);
}
#endif /* AVRCP_ENABLE_DEPRECATED */

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */


/****************************************************************************
* NAME    
*    avrcpGetErrorStatusCode
*
* DESCRIPTION
*   Responses to AVRCP Specific Browsing Commands and AVRCP Specific 
*   AV/C Commands. Contain 1 byte Error Status Code. This function extracts
*   the Error Status code from the application response. Append the error
*   code to the pdu if required.
*
* PARAMETERS
*   response       - Application Response , standardised by this function
*   command_type   - Should be one of the values AVRCP0_CTYPE_CONTROL, 
*                        AVRCP0_CTYPE_STATUS or AVRCP0_CTYPE_NOTIFY
*
* RETURNS
*    void
********************************************************************************/
uint8 avrcpGetErrorStatusCode(avrcp_response_type *response, uint8 command_type)
{
    uint8 error_code = AVRCP_STATUS_SUCCESS;

    /* if response value is any specific rejected error response values,
       convert the response to avrcp_response_rejected after extracting the 
       error status  code from  the response value, otherwise standardize the
       response.*/

    if(*response < avrcp_response_rejected_invalid_pdu)
    {
        switch(*response)
        {
        case avctp_response_accepted:
        case avctp_response_changed: /* fall through */
        case avctp_response_stable: /* fall through */
            
            if(command_type == AVRCP0_CTYPE_STATUS)
            {
                *response = avctp_response_stable;
            }
            else if(command_type == AVRCP0_CTYPE_CONTROL)
            {
                *response = avctp_response_accepted;
            }
            else
            {
                *response = avctp_response_changed;
            }
            break;
        
        case avctp_response_interim:
            /* accept */
            break;
        case avctp_response_rejected:
            error_code = AVRCP_STATUS_INTERNAL_ERROR;
            break;
        default:
            /* ignore */
            break;            
        }
    }
    else
    {
        /* return correspondent status code */
        error_code = *response & AVRCP_RESPONSE_ERROR_STATUS_MASK;
        *response = avctp_response_rejected;
    }

    return error_code;

}


/****************************************************************************
* NAME    
* avrcpSendCommonMetadataInd    
*
* DESCRIPTION
*   Send MetaData Indication to the TG application if there was no data 
*   received part of Metadata request message.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendCommonMetadataInd(AVRCP *avrcp, uint16 id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_METADATA_IND_MESSAGE);

    message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED
    message->transaction = avrcp->bitfields.rsp_transaction_label;
#endif

    MessageSend(avrcp->clientTask, id, message);
}

/****************************************************************************
* NAME    
* avrcpSendCommonFragmentedMetadataInd    
*
* DESCRIPTION
*   Send MetaData Indication to the TG application if there was data 
*   received part of Metadata request message.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendCommonFragmentedMetadataInd(  AVRCP     *avrcp, 
                                            uint16     id, 
                                            uint16     number_of_data_items, 
                                            uint16     data_length, 
                                            Source     source)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_FRAGMENTED_METADATA_IND);

    message->avrcp = avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED
    message->transaction = avrcp->bitfields.rsp_transaction_label;
    message->no_packets = 1; /* Deprecated */
    message->ctp_packet_type = AVCTP0_PACKET_TYPE_SINGLE;
    message->data_offset = 0; /* Deprecated */
    message->metadata_packet_type = avrcp_packet_type_single;
#endif

    message->number_of_data_items = number_of_data_items;
    message->size_data = data_length;
    message->data = source;

    MessageSend(avrcp->clientTask, id, message);
}

/****************************************************************************
*NAME    
*    AvrcpSetMetadataResponsePDUDataSize   
*
*DESCRIPTION
*    API function to restrict metadata response PDU size from AVRCP_AVC_MAX_DATA_SIZE 
*    to data_size.
*
*RETURNS
* status
******************************************************************************/
bool AvrcpSetMetadataResponsePDUDataSize(AVRCP *avrcp, 
                                     uint16 data_size)
{
    /* data_size should less than or equal to the max AVRCP_AVC_MAX_DATA_SIZE. 
    * If it is more than max AVRCP_AVC_MAX_DATA_SIZE then library will return FALSE
    */
    if(avrcp->av_max_data_size <= AVRCP_AVC_MAX_DATA_SIZE)
    {
       avrcp->av_max_data_size = data_size;
       return TRUE;       
    }
    
    return FALSE;
}

#endif /* !AVRCP_CT_ONLY_LIB*/

/****************************************************************************
*NAME    
*    avrcpSendCommonCfmMessageToApp
*
*DESCRIPTION
*    Create a common CFM message (many messages sent to the app
*    have the form of the message below and a common function can be used to
*    allocate them). Send the message not forgetting to set the correct 
*    message id.
*
******************************************************************************/
void avrcpSendCommonCfmMessageToApp(uint16              message_id,
                                    avrcp_status_code   status, 
                                    Sink                sink, 
                                    AVRCP               *avrcp)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_CFM_MESSAGE);
    message->status = status;
    message->sink = sink;
    message->avrcp = avrcp;
    MessageSend(avrcp->clientTask, message_id, message);
}

/****************************************************************************
* NAME    
*   avrcpGetCompanyId    
*
* DESCRIPTION
*  Utility function to extract the 3 byte company ID from the data packet.
*
*PARAMETRS:
*   ptr     -   Data packet
*   offset  -   Offset to start of company ID in the data packet.
*******************************************************************************/
uint32 avrcpGetCompanyId(const uint8 *ptr, uint16 offset)
{
    uint32 cid = 0;
    uint32 cid_tmp = 0;

    cid_tmp = (uint32) ptr[offset];
    cid = (cid_tmp << 16);
    cid_tmp = (uint32) ptr[offset+1];
    cid |= (cid_tmp << 8);
    cid_tmp = (uint32) ptr[offset+2];
    cid |= cid_tmp;

    return cid;
}

/****************************************************************************
* NAME    
*   avrcpGrabSink    
*
* DESCRIPTION
*  Grab Sink space  to send AVCTP data packet
*
*PARAMETRS:
*   Sink     -   L2CAP sink
*   size     -   Requested Sink Space to reserve.
*******************************************************************************/
uint8 *avrcpGrabSink(Sink sink, uint16 size)
{
    uint8 *dest = SinkMap(sink);
    uint16 claim_result = SinkClaim(sink, size);
    if (claim_result == 0xffff)
    {
        AVRCP_DEBUG(("SinkClaim return Invalid Offset"));
        return NULL;
    }

    return (dest + claim_result);
}

/**************************************************************************
 * NAME
 *  avrcpSourceDrop
 *
 * DESCRIPTION 
 *  Drop the AVRCP Source 
 *************************************************************************/
void avrcpSourceDrop(AVRCP* avrcp)
{
    Source source = StreamSourceFromSink(avrcp->sink);

    if(avrcp->av_msg_len)   
    {
        uint16 data_drop = avrcp->av_msg_len;

        /* the packet being dropped may be mapped in from the source
         * in which case we can drop the source. Otherwise it is in 
         * an allocated buffer and we need to discard the buffer
         */
        if( SourceMap(source) != avrcp->av_msg ) 
        {
            if( avrcp->bitfields.fragment == avrcp_packet_type_single)
            {
                data_drop = 0;
                free(avrcp->av_msg);
                avrcp->av_msg= NULL;
                avrcp->av_msg_len = 0; 
                avrcp->bitfields.fragment = avrcp_packet_type_single;
            }
            else
            {
                data_drop -= AVRCP_AVC_START_HEADER_SIZE;
                avrcp->av_msg_len = AVRCP_AVC_START_HEADER_SIZE;
            }
        }
        else
        {
            avrcp->av_msg= NULL;
            avrcp->av_msg_len = 0;
        }

        if( data_drop )
        {
            /* Data is from the Source. Flush it */
            SourceDrop(source, data_drop );
            if(source == avrcp->data_app_ind) 
            {
                avrcp->data_app_ind = 0;
            }
        }
    }
    else
    {
        avrcp->av_msg = NULL;
    }
}


/****************************************************************************
* NAME    
*   avrcpSourceProcessed    
*
* DESCRIPTION
*  Free all memory resources. 
*
*  PARAMETERS
*  avrcp  - AVRCP instance
*  intern - TRUE - Call initiated by internal function. Free all resources.
*           FALSE- Call Initiated by app. Free only the external resource if
*                  other shared resources are not yet processed by the library.
*
*******************************************************************************/
void avrcpSourceProcessed(AVRCP *avrcp, bool intern)
{
    if(avrcp->bitfields.fragment == avrcp_packet_type_end )
    {
        avrcp->bitfields.fragment = avrcp_packet_type_single;
    }

    if(!avrcp->bitfields.block_received_data)
    {
        Source source = StreamSourceFromSink(avrcp->sink);

        avrcpSourceDrop(avrcp);

        if(SourceBoundary(source))
        {
            /* There is data in the L2CAP sink to be processed.
                 Message Internally to trigger*/
            MessageSend(&avrcp->task, AVRCP_INTERNAL_MESSAGE_MORE_DATA, 0);
        }

        if(avrcp->data_app_ind && (!intern))
        {
            MessageStreamTaskFromSink(StreamSinkFromSource(avrcp->data_app_ind), (Task)NULL);
            SourceEmpty(avrcp->data_app_ind);
            avrcp->data_app_ind = 0;
            if(avrcp->dataFreeTask.sent_data)
            {
                /* Source empty will trigger message MESSAGE_SOURCE_EMPTY
                   and free sent_data in the CleanUpTask. But this may 
                   create a race as well. So don't wait for that message 
                   and free the sent_data here for the application data */
                free(avrcp->dataFreeTask.sent_data);
                avrcp->dataFreeTask.sent_data = 0;
            }
        }
    }
}


/****************************************************************************
* NAME    
*    convertUint8ValuesToUint32
*
* DESCRIPTION
*   Utility functions to convert 4 data octets to a UINT32 value.
*****************************************************************************/
uint32 convertUint8ValuesToUint32(const uint8 *ptr)
{
    return ((((uint32)ptr[0] << 24) & 0xFF000000u) | 
            (((uint32)ptr[1] << 16) & 0x00FF0000u) | 
            (((uint32)ptr[2] << 8) & 0x0000FF00u)  | 
            ((uint32)ptr[3] & 0x000000FFu));
}

/****************************************************************************
* NAME    
*   convertUint32ToUint8Values
*
* DESCRIPTION
*   Utility functions to convert 32 bit value to 4 data octets.
*****************************************************************************/
void convertUint32ToUint8Values(uint8 *ptr, uint32 value)
{
    ptr[0] = (value >> 24) & 0xFF;
    ptr[1] = (value >> 16) & 0xFF;
    ptr[2] = (value >> 8) & 0xFF;
    ptr[3] = value & 0xFF;
}

/****************************************************************************
* NAME    
* avrcpSourceFromConstData    
*
* DESCRIPTION
*  Create a Source by allocating memory and copying data from constant memory.   
*
* RETURNS
*    void
*******************************************************************************/
Source avrcpSourceFromConstData(AVRCP *avrcp, const uint8 *data, uint16 length)
{
    uint8* ptr = NULL;
    Source src = StreamSourceFromSink(avrcp->sink);
    uint16 data_drop = avrcp->av_msg_len;

    /* If application does not free the data yet. Force free */
    if( avrcp->data_app_ind && (src != avrcp->data_app_ind) )
    {
        MessageStreamTaskFromSink(StreamSinkFromSource(avrcp->data_app_ind), (Task)NULL);
        if(avrcp->dataFreeTask.sent_data)
        {
            free(avrcp->dataFreeTask.sent_data);
            avrcp->dataFreeTask.sent_data = 0;
        }
        SourceEmpty(avrcp->data_app_ind);
    }

    if(avrcp->bitfields.fragment == avrcp_packet_type_single)
    {
        /* allocate memory */
        ptr = (uint8*)malloc(length);
    }
    else
    {
        data_drop -= AVRCP_AVC_START_HEADER_SIZE;
    }

    if( ptr )
    {
        memmove(ptr, data, length);
        src = avrcpSourceFromData(avrcp, ptr, length);
    }
    else
    {
         if(avrcp->bitfields.block_received_data== avrcp_none)
         {
            avrcpBlockReceivedData(avrcp, avrcp_app_unknown, 0);
         }
         data_drop -= length;
         SourceDrop(src, data_drop );
         avrcp->av_msg_len -= data_drop;
    }

    avrcp->data_app_ind = src;
    return src;
}

/****************************************************************************
* NAME    
* avrcpSourceFromData    
*
* DESCRIPTION
*  Create a Source from the passed memory location.
*
* RETURNS
*    void
*******************************************************************************/
Source avrcpSourceFromData(AVRCP *avrcp, uint8 *data, uint16 length)
{
    /* Create a source from the data */
    Source src = StreamRegionSource(data, length);

    /* Register a task for freeing the data and store a ptr to it */
    avrcp->dataFreeTask.sent_data = data;
    MessageStreamTaskFromSink(StreamSinkFromSource(src), 
                    &avrcp->dataFreeTask.cleanUpTask);

    return src;
}


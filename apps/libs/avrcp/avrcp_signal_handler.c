/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_signal_handler.c        

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include <stream.h>
#include <source.h>
#include <panic.h>
#include <sink.h>
#include <string.h>
#include "avrcp_signal_handler.h"
#include "avrcp_signal_passthrough.h"
#include "avrcp_signal_unit_info.h"
#include "avrcp_signal_vendor.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_notification_handler.h"


/****************************************************************************
*NAME    
*    avrcpHandleReceivedData
*
*DESCRIPTION
*    This function is called to Handle the Data Received
*    
*PARAMETERS
*   avrcp                   - Task
*
*RETURN
*****************************************************************************/
void avrcpHandleReceivedData(AVRCP *avrcp)
{
    Source source = StreamSourceFromSink(avrcp->sink);
    uint16 packet_size;

    while (((packet_size = SourceBoundary(source)) != 0) && 
            (!avrcp->bitfields.block_received_data))
    {
        if(avrcpAvctpReceiveMessage( avrcp, SourceMap(source), packet_size))
        {
            if ((avrcp->av_msg[AVCTP_HEADER_START_OFFSET] & 
                 AVCTP0_CR_MASK) == AVCTP0_CR_COMMAND)
            {
            #ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
                avrcpHandleCommand(avrcp);
            #else
                avrcpSourceProcessed(avrcp, TRUE);
            #endif /* !AVRCP_CT_ONLY_LIB*/
            }
            else
            {
            #ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
                avrcpHandleResponse(avrcp);
            #else
                avrcpSourceProcessed(avrcp, TRUE);
            #endif /* !AVRCP_TG_ONLY_LIB */
            }
        }
    }
}

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/**************************************************************************
*NAME    
*    avrcpHandleResponse
*
*DESCRIPTION
*    This function is called to Handle the Response Received
*    
*PARAMETERS
*   avrcp                   - Task
*
*RETURN
****************************************************************************/
void avrcpHandleResponse(AVRCP *avrcp)
{
    const uint8 *ptr = avrcp->av_msg;
    uint16 ctype_offset = AVCTP_SINGLE_PKT_HEADER_SIZE;
    uint16 packet_type = ptr[AVCTP_HEADER_START_OFFSET] & 
                         AVCTP0_PACKET_TYPE_MASK;
    uint16 packet_size = avrcp->av_msg_len;
   
    if (packet_type == AVCTP0_PACKET_TYPE_START)
    {
        ctype_offset =AVCTP_START_PKT_HEADER_SIZE;
    } 

    /* Now Skip the AVCTP Header */
    ptr+= ctype_offset;
    packet_size-= ctype_offset;


    switch(ptr[AVRCP_OPCODE_OFFSET])
    {
    case AVRCP2_PASSTHROUGH:
        avrcpHandlePassthroughResponse(avrcp, ptr, packet_size);
        break;

    case AVRCP2_UNITINFO:
        avrcpHandleUnitInfoResponse(avrcp, ptr, packet_size);
        break;

    case AVRCP2_SUBUNITINFO:
        avrcpHandleSubUnitInfoResponse(avrcp, ptr, packet_size);
        break;

    case AVRCP2_VENDORDEPENDENT:
        avrcpHandleVendorResponse(avrcp, ptr, packet_size);
        break;

    default:
        /* The source has been processed so drop it here. */
        avrcpSourceProcessed(avrcp, TRUE);
    }
}

/****************************************************************************
* NAME    
* avrcpHandleInternalWatchdogTimeout    
*
* DESCRIPTION
* Handler function to handle the response timeout at CT if the TG has not 
* responded for an outstanding command. This function sends a corresponding 
* failure confirmation message to the application.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpHandleInternalWatchdogTimeout(AVRCP *avrcp)
{
    if (avrcp->sink)
    {
        AVRCP_INFO(("avrcpHandleInternalWatchdogTimeout\n"));
        switch (avrcp->pending)
        {
            case avrcp_passthrough:
                avrcpSendPassthroughCfmToClient(avrcp, avrcp_timeout);
                break;
            case avrcp_unit_info:
                avrcpSendUnitInfoCfmToClient(avrcp, avrcp_timeout, 0, 0,
                                             (uint32) 0);
                break;
            case avrcp_subunit_info:
                avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_timeout, 0, 0);
                break;
            case avrcp_vendor:
                avrcpSendVendordependentCfmToClient(avrcp, avrcp_timeout, 0);
                break;
            default:
                if (avrcp->pending >= avrcp_get_caps)
                    avrcpSendMetadataFailCfmToClient(avrcp, avrcp_timeout);
                break;
        }        
    }
    avrcp->pending = avrcp_none;
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
*NAME    
*    avrcpHandleCommand
*
*DESCRIPTION
*    This function is called to Handle the command Received
*    
*PARAMETERS
*   avrcp                   - Task
*
*RETURN
***************************************************************************/
void avrcpHandleCommand(AVRCP *avrcp)
{
    const uint8 *ptr = avrcp->av_msg;
    uint16 ctype_offset = AVCTP_SINGLE_PKT_HEADER_SIZE;
    uint16 packet_type = ptr[AVCTP_HEADER_START_OFFSET] 
                        & AVCTP0_PACKET_TYPE_MASK;
    uint16 packet_size = avrcp->av_msg_len;

    if (packet_type == AVCTP0_PACKET_TYPE_START)
    {
        uint16 opcode_offset = AVCTP_START_PKT_HEADER_SIZE + 
                               AVRCP_OPCODE_OFFSET;

        /* 1 extra Byte in header for Num Packets. Skip that */
        ctype_offset = AVCTP_START_PKT_HEADER_SIZE;

        /* Fragmented packets should be Vendor Dependent or Pass Through */
        if((ptr[opcode_offset] != AVRCP2_VENDORDEPENDENT) &&
           (ptr[opcode_offset] != AVRCP2_PASSTHROUGH))
        {
            /* Send Error Response */
            avrcpSendAvcResponse(avrcp, AVRCP_START_PKT_HEADER_SIZE, 
                                    avctp_response_rejected, 0, (Source)NULL);
        }
    }

    /* Now Skip the AVCTP Header */
    ptr+= ctype_offset;
    packet_size -= ctype_offset;

    
    if ((ptr[AVRCP_CTYPE_OFFSET] == AVRCP0_CTYPE_CONTROL) && 
        (ptr[AVRCP_OPCODE_OFFSET] == AVRCP2_PASSTHROUGH))
    {
        avrcpHandlePassthroughCommand(avrcp, ptr, packet_size);
    }
    else if ((ptr[AVRCP_CTYPE_OFFSET] == AVRCP0_CTYPE_STATUS) && 
            (ptr[AVRCP_OPCODE_OFFSET] == AVRCP2_UNITINFO))
    {
        avrcpHandleUnitInfoCommand(avrcp, ptr, packet_size);
    }
    else if ((ptr[AVRCP_CTYPE_OFFSET] == AVRCP0_CTYPE_STATUS) && 
            (ptr[AVRCP_OPCODE_OFFSET] == AVRCP2_SUBUNITINFO))
    {
        avrcpHandleSubUnitInfoCommand(avrcp, ptr, packet_size);
    }
    else if (ptr[AVRCP_OPCODE_OFFSET] == AVRCP2_VENDORDEPENDENT)
    {
        avrcpHandleVendorCommand(avrcp, ptr, packet_size);
    }
    else
    {
         avrcpSendAvcResponse(avrcp, AVRCP_START_PKT_HEADER_SIZE, 
                                    avctp_response_not_implemented, 0, (Source)NULL);
    }
}



/****************************************************************************
*NAME    
*    avrcpSendAvcResponse    
*
*DESCRIPTION
*    This function is called to Frame the AVRCP AVC Response packet and send
*   to avctp. It sends the response only if the command is outstanding, else
*   ignore the packet.
*    
*PARAMETERS
*   avrcp                   - Task
*   uint16 hdr_size         - Header size of AVRCP Response message 
*                               (same as Command)
*   avrcp_response_type  response - Response to send
*   uint16 data_len         - AVRCP message Response parameters to be send 
*                                   (Max is 512-hdr_size)
*   Source   data           - AVRCP Message data to append after AVRCP
*                             message head    
*
*RETURN
*****************************************************************************/
void avrcpSendAvcResponse(AVRCP                *avrcp,
                          uint16                hdr_size,  
                          avrcp_response_type   response,
                          uint16                data_len,
                          Source                data)  
{
    uint16 avctp_header_size; 
    uint8 *rsp;
    uint16 packet_size = data_len + hdr_size; 
    uint16 max_mtu = avrcp->l2cap_mtu - AVCTP_SINGLE_PKT_HEADER_SIZE;
    uint8  cmd_offset=AVCTP_SINGLE_PKT_HEADER_SIZE;
    Sink sink = avrcp->sink;
    

    if((avrcp->av_msg == SourceMap(StreamSourceFromSink(sink))) &&
        (SourceBoundary(StreamSourceFromSink(sink)) == 0 ))
    {
        AVRCP_INFO(("avrcpSendAvcResponse: Lost the command\n"));
        avrcp->av_msg = NULL;
        avrcp->av_msg_len = 0;
    }
 
    /* Send the response only if there is a outstanding command */
    if(avrcp->av_msg) 
    {

        /* Total Packet size of first AVCTP packet depends on Fragmentation */
        avctp_header_size = (packet_size > max_mtu)? 
                    AVCTP_START_PKT_HEADER_SIZE:AVCTP_SINGLE_PKT_HEADER_SIZE;

        /* Size of the First Packet */
        packet_size +=  avctp_header_size;

        /* Check Sink Space is available for the packet */
        if(packet_size <= SinkSlack(sink))
        {
            /* Change packet_size = avctp+avrcp header size */
            packet_size = avctp_header_size + hdr_size;

            /* Grab Sink for this Size and Set the AVRCP Header.
               Check is not required since SinkSlacks returns success  */ 
            rsp=avrcpGrabSink(sink, packet_size);

            /* Set the AVRCP Header from the command. We never store
               continue packet here
               check for only start since default is single */
            if((avrcp->av_msg[AVCTP_HEADER_START_OFFSET]
                       & AVCTP0_PACKET_TYPE_MASK)  == AVCTP0_PACKET_TYPE_START)
            {
                cmd_offset = AVCTP_START_PKT_HEADER_SIZE;
            }
            
            /*  Assume avrcp_av_msg is of correct size. 
                This was validated upon receiving the command */
           memmove(&rsp[avctp_header_size], &avrcp->av_msg[cmd_offset], 
                   hdr_size);

            /* Copy the first AVCTP octet from command */
            rsp[AVCTP_HEADER_START_OFFSET] =
                     avrcp->av_msg[AVCTP_HEADER_START_OFFSET];

           /* Set PID and response  */
            if(response == avctp_response_bad_profile)
            {   
                /* Set Invalid PID for Bad Profile  */
                if(data_len)
                {
                    /* Drop the Source Data */
                    SourceDrop(data, data_len);
                }

                data_len = hdr_size = 0;
                rsp[AVCTP_HEADER_START_OFFSET] |= AVCTP0_IPID; 
            }
            else
            {
                /* Copy the PID and Response */
                memmove(&rsp[avctp_header_size-AVCTP_PID_SIZE],
                        &avrcp->av_msg[cmd_offset-AVCTP_PID_SIZE],
                        AVCTP_PID_SIZE);
                rsp[avctp_header_size] = response;
            }

            /* Send the AVCTP Packet */
            avrcpAvctpSendMessage(  avrcp,
                                    AVCTP0_CR_RESPONSE,
                                    rsp, hdr_size, 
                                    data_len, data);
        }
        else
        {
            if(data_len)
            {
               /* Drop the Source Data */
               SourceDrop(data, data_len);
            }
        }

    }
    else
    {
        AVRCP_INFO(("avrcpSendAvcResponse: Lost the command\n"));
    }

    /*
     * Interim responses are allowed for Vendor dependent commands.
     * In that case, don't free the command till the final response. 
     * All Meta data Response packets are handled in SendMetaDataResponse(). 
     * Checking the hdr_size to 11 instead of 10 to avoid conflict with the 
     * Group response header which is also 10. This may cause some Meta
     * data responses which does not come with any fixed data will be 
     * handled here (Ex: some continuation response) and it will not 
     * affect the program logic other than a duplicate call to 
     * avrcpSourceProcessed().
     */
    if((response != avctp_response_interim) &&
      (hdr_size < AVRCP_MIN_AVC_PACKET_SIZE))
    {
        /* Allow data arriving to be processed. */
        avrcpUnblockReceivedData(avrcp);
    }

    return;
}                         
                            
#endif /* !AVRCP_CT_ONLY_LIB*/



/****************************************************************************
* NAME    
* avrcpBlockReceivedData    
*
* DESCRIPTION
* Block processing of any incoming data at the TG till the library sends a
* response to the outstanding command. 
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   pending     -   Outstanding command in process.
*   data        -   Optional data associated with the command 
*                   for unique identification of the command. 
*
* RETURNS
*    void
*******************************************************************************/
void avrcpBlockReceivedData(AVRCP *avrcp, 
                            avrcpPending pending,
                            uint16 data)
{
    uint32 timeout = (pending == avrcp_app_unknown)? 
                      AVRCP_APP_WATCHDOG_TIMEOUT:
                      AVCTP_SEND_RESPONSE_TIMEOUT;

    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT);
    avrcp->bitfields.block_received_data = pending;
    message->pending_command = pending;
    message->data = data;

    
    MessageSendLater(&avrcp->task, AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT,
                     message, timeout);
}


/****************************************************************************
* NAME    
* avrcpUnblockReceivedData    
*
* DESCRIPTION
* Start processing the data at TG after clearing the outstanding command.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpUnblockReceivedData(AVRCP *avrcp)
{
    avrcp->bitfields.block_received_data = 0;
    MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT);

    /* Process the Source to Unblock receiving data */
    avrcpSourceProcessed(avrcp, TRUE);
}


/****************************************************************************
* NAME    
* avrcpHandleInternalSendResponseTimeout    
*
* DESCRIPTION
* Handler function to handle the application response timeout at TG if the 
* application is not responding to the Outstanding command from CT. This
* function rejects the outstanding command from the CT and start processing 
* new commands. 
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance
*   res         -   pending command waiting for a response from the application. 
*
* RETURNS
*    void
*******************************************************************************/
void avrcpHandleInternalSendResponseTimeout(AVRCP *avrcp, 
        const AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT_T *res)
{
#ifndef AVRCP_CT_ONLY_LIB
    /* Send a reject response if the application fails to send a response
        to the pending command    within the alloted time. */
        
    switch (res->pending_command)
    {
    case avrcp_passthrough:
        sendPassThroughResponse(avrcp, avctp_response_rejected);
        break;
    case avrcp_unit_info:
        sendUnitInfoResponse(avrcp, 0, 0, 0, 0);
        break;
    case avrcp_subunit_info:
        sendSubunitInfoResponse(avrcp, 0, 0);
        break;
    case avrcp_vendor:
        sendVendorDependentResponse(avrcp, avctp_response_rejected);
        break;
    case avrcp_next_group:
    case avrcp_previous_group: /* Fall Through */
        sendGroupResponse(avrcp, avctp_response_rejected);
        break;

    case avrcp_get_caps:                /* fall through */
    case avrcp_list_app_attributes:     /* fall through */
    case avrcp_list_app_values:         /* fall through */
    case avrcp_get_app_values:          /* fall through */
    case avrcp_set_app_values:          /* fall through */
    case avrcp_get_app_attribute_text:  /* fall through */
    case avrcp_get_app_value_text:      /* fall through */
    case avrcp_character_set:           /* fall through */
    case avrcp_battery_information:     /* fall through */
    case avrcp_get_element_attributes:  /* fall through */
    case avrcp_get_play_status:         /* fall through */
    case avrcp_absolute_volume:         /* fall through */
    case avrcp_set_addressed_player:    /* fall through */
    case avrcp_add_to_now_playing:      /* fall through */
    case avrcp_play_item:               /* fall through */
        avrcpHandleInternalRejectMetadataResponse(avrcp, 
                                        avrcp_response_rejected_internal_error,
                                        res->pending_command);
        break;

    case avrcp_playback_status:
    case avrcp_track_changed:           /* fall through */
    case avrcp_track_reached_end:       /* fall through */
    case avrcp_track_reached_start:     /* fall through */
    case avrcp_playback_pos_changed:    /* fall through */
    case avrcp_batt_status_changed:     /* fall through */
    case avrcp_system_status_changed:   /* fall through */
    case avrcp_player_setting_changed:  /* fall through */
    case avrcp_playing_content_changed: /* fall through */
    case avrcp_available_player_changed:/* fall through */
    case avrcp_addressed_player_changed:/* fall through */
    case avrcp_uids_changed:            /* fall through */
    case avrcp_volume_changed:          /* fall through */
        avrcpRejectRegisterNotifications(avrcp, 
                                  SetNotificationBit(res->pending_command), 
                                  avrcp_response_rejected_internal_error);
        break;
    case avrcp_none:
    case avrcp_app_unknown: /* fall through */
    case avrcp_request_continuation:/* fall through */
    case avrcp_abort_continuation: /* fall through */
    default:                       /* fall through */

        /* Unblock Data to receive */
        avrcpUnblockReceivedData(avrcp);
        break;
    }        
#else
    avrcpUnblockReceivedData(avrcp);
#endif /* AVRCP_CT_ONLY_LIB */
}



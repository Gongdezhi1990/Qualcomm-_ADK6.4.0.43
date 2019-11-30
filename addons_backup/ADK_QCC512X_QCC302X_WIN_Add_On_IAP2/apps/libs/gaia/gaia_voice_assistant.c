/*****************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
  @file gaia_voice_assistant.c
  @brief Implementation of Voice Assistant functionality over GAIA.
*/

#include <sink.h>
#include <vm.h>

#include "gaia_private.h"
#include "gaia_transport.h"
#include "gaia_voice_assistant.h"

#define GAIA_VA_MAX_EA_FRAMES (3)


/*************************************************************************
NAME
    gaiaBuildVaDataPacket
    
DESCRIPTION
    Build a GAIA_COMMAND_VA_VOICE_DATA packet with given data
*/
static void gaiaBuildVaDataPacket(uint8 *packet, const uint8 *data, uint16 size_data) 
{
    static const uint8 va_voice_data_header[] =
    {
        GAIA_SOF,
        GAIA_VERSION,
        GAIA_PROTOCOL_FLAG_NONE,
        0,  /* payload length */
        HIGH(GAIA_VENDOR_QTIL),
        LOW(GAIA_VENDOR_QTIL),
        HIGH(GAIA_COMMAND_VA_VOICE_DATA),
        LOW(GAIA_COMMAND_VA_VOICE_DATA)
    };
    
    memmove(packet, va_voice_data_header, GAIA_OFFS_PAYLOAD);
    packet[GAIA_OFFS_PAYLOAD_LENGTH] = size_data;
    memmove(packet + GAIA_OFFS_PAYLOAD, data, size_data);
}




/*************************************************************************
NAME
    gaiaVoiceAssistantCapable
    
DESCRIPTION
    Determines if the transport is capable of supporting Voice Assistant
*/
static bool gaiaVoiceAssistantCapable(gaia_transport *transport)
{
    return (transport->type == gaia_transport_spp) ||
           (transport->type == gaia_transport_rfcomm);
}


/*************************************************************************
NAME
    gaiaVoiceAssistantFindTransport
    
DESCRIPTION
    Returns the single transport instance where the Host supports Voice
    Assistant or NULL if there is none
    
RETURNS
    Pointer to the GAIA transport instance
*/
static gaia_transport *gaiaVoiceAssistantFindTransport(void)
{
    if (gaia)
    {
        uint16 idx;
        
        for (idx = 0; idx < gaia->transport_count; ++idx)
        {
            if (gaia->transport[idx].has_voice_assistant)
            {
                return &gaia->transport[idx];
            }
        }
    }
    
	return NULL;
}


/*************************************************************************
NAME
    gaiaVoiceAssistantSendStartCfm

DESCRIPTION
    Send a GAIA_VA_START_CFM message to the client task
*/
static void gaiaVoiceAssistantSendStartCfm(gaia_transport *transport, bool success)
{
    MESSAGE_PMAKE(cfm, GAIA_VA_START_CFM_T);
    cfm->transport = (GAIA_TRANSPORT *) transport;
    cfm->success = success;
    MessageSend(gaia->app_task, GAIA_VA_START_CFM, cfm);
}


/*************************************************************************
NAME
    gaiaVoiceAssistantSendVoiceEndCfm
    
DESCRIPTION
    Send a GAIA_VA_VOICE_END_CFM message to the client task
*/
static void gaiaVoiceAssistantSendVoiceEndCfm(gaia_transport *transport, bool success)
{
    MESSAGE_PMAKE(cfm, GAIA_VA_VOICE_END_CFM_T);
    cfm->transport = (GAIA_TRANSPORT *) transport;
    cfm->success = success;
    MessageSend(gaia->app_task, GAIA_VA_VOICE_END_CFM, cfm);
}


/*************************************************************************
NAME
    gaiaVoiceAssistantSendVoiceEndInd
    
DESCRIPTION
    Send a GAIA_VA_VOICE_END_IND message to the client task to indicate
    that the Host has detected the end of voice input
*/
static void gaiaVoiceAssistantSendVoiceEndInd(gaia_transport *transport)
{
    MESSAGE_PMAKE(ind, GAIA_VA_VOICE_END_IND_T);
    ind->transport = (GAIA_TRANSPORT *) transport;
    MessageSend(gaia->app_task, GAIA_VA_VOICE_END_IND, ind);
}


/*************************************************************************
NAME
    gaiaVoiceAssistantSendAnswerStartInd
    
DESCRIPTION
    Send a GAIA_VA_ANSWER_START_IND message to the client task to indicate
    that the Host is starting its voice response
*/
static void gaiaVoiceAssistantSendAnswerStartInd(gaia_transport *transport)
{
    MESSAGE_PMAKE(ind, GAIA_VA_ANSWER_START_IND_T);
    ind->transport = (GAIA_TRANSPORT *) transport;
    MessageSend(gaia->app_task, GAIA_VA_ANSWER_START_IND, ind);
}


/*************************************************************************
NAME
    gaiaVoiceAssistantSendAnswerEndInd
DESCRIPTION
    Send a GAIA_VA_ANSWER_END_IND message to the client task to indicate
    that the Host has ended its voice response
*/
static void gaiaVoiceAssistantSendAnswerEndInd(gaia_transport *transport)
{
    MESSAGE_PMAKE(ind, GAIA_VA_ANSWER_END_IND_T);
    ind->transport = (GAIA_TRANSPORT *) transport;
    MessageSend(gaia->app_task, GAIA_VA_ANSWER_END_IND, ind);
}


/*************************************************************************
NAME
    gaiaVoiceAssistantSendCancelCfm
DESCRIPTION
    Send a GAIA_VA_CANCEL_CFM message to the client task
*/
static void gaiaVoiceAssistantSendCancelCfm(gaia_transport *transport, bool success)
{
    MESSAGE_PMAKE(cfm, GAIA_VA_CANCEL_CFM_T);
    cfm->transport = (GAIA_TRANSPORT *) transport;
    cfm->success = success;
    MessageSend(gaia->app_task, GAIA_VA_CANCEL_CFM, cfm);
}


/*************************************************************************
NAME
    gaiaVoiceAssistantSendCancelInd
DESCRIPTION
    Send a GAIA_VA_CANCEL_IND message to the client task
*/
static void gaiaVoiceAssistantSendCancelInd(gaia_transport *transport, uint8 reason)
{
    MESSAGE_PMAKE(ind, GAIA_VA_CANCEL_IND_T);
    ind->transport = (GAIA_TRANSPORT *) transport;
    ind->reason = reason;
    MessageSend(gaia->app_task, GAIA_VA_CANCEL_IND, ind);
}


/*************************************************************************
NAME
    gaiaVoiceAssistantSendVoiceDataRequestInd
    
DESCRIPTION
    Send a GAIA_VA_DATA_REQUEST_IND message to the client task to
    indicate that the Host is ready for voice data
*/
static void gaiaVoiceAssistantSendVoiceDataRequestInd(gaia_transport *transport)
{
    MESSAGE_PMAKE(ind, GAIA_VA_DATA_REQUEST_IND_T);
    ind->transport = (GAIA_TRANSPORT *) transport;
    MessageSend(gaia->app_task, GAIA_VA_DATA_REQUEST_IND, ind);
}


/*************************************************************************
NAME
    gaiaVoiceAssistantCheckHostVersion

DESCRIPTION
    If the transport is capable of supporting Voice Assistant, send
    GAIA_COMMAND_VA_CHECK_VERSION to check that the Host supports the
    Device's version of the Voice Assistant protocol.
*/
void gaiaVoiceAssistantCheckHostVersion(gaia_transport *transport)
{
    if (gaiaVoiceAssistantCapable(transport))
    {
        const uint8 my_version[] = {GAIA_VA_VERSION_MAJOR, GAIA_VA_VERSION_MINOR};
    
        GAIA_VA_DEBUG(("gaia: va: req %u.%u\n", my_version[0], my_version[1]));
    
        send_simple_command(transport, GAIA_COMMAND_VA_CHECK_VERSION, 2, my_version);
    }
    else
    {
        GAIA_DEBUG(("gaia: va: no support\n"));
    }
}


/*************************************************************************
NAME
    gaiaHandleVoiceAssistantCommand

DESCRIPTION
    Handle a Voice Assistant protocol command or return FALSE if we can't
*/
bool gaiaHandleVoiceAssistantCommand(gaia_transport *transport, uint16 command_id,
                                      uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case GAIA_COMMAND_VA_CHECK_VERSION | GAIA_ACK_MASK:
    /*  Host has acknowledged our VA version check  */
        transport->has_voice_assistant = (payload_length && (payload[0] == GAIA_STATUS_SUCCESS));
        GAIA_VA_DEBUG(("gaia: va: rsp %c\n", transport->has_voice_assistant ? 'Y' : 'N'));
        return TRUE;
        
    case GAIA_COMMAND_VA_START | GAIA_ACK_MASK:
    /*  Host has acknowledged our VA Start request  */
        gaiaVoiceAssistantSendStartCfm(transport, payload_length && (payload[0] == GAIA_STATUS_SUCCESS));
        return TRUE;
        
    case GAIA_COMMAND_VA_CANCEL | GAIA_ACK_MASK:
    /*  Host has acknowledged our VA Cancel request  */
        gaiaVoiceAssistantSendCancelCfm(transport, payload_length && (payload[0] == GAIA_STATUS_SUCCESS));
        return TRUE;
        
    case GAIA_COMMAND_VA_VOICE_END | GAIA_ACK_MASK:
    /*  Host has acknowledged our VA Voice End message  */
        gaiaVoiceAssistantSendVoiceEndCfm(transport, payload_length && (payload[0] == GAIA_STATUS_SUCCESS));
        return TRUE;
     
    case GAIA_COMMAND_VA_VOICE_DATA_REQUEST:
    /*  Host has requested voice data from Device; indicate this to client app  */
        gaiaVoiceAssistantSendVoiceDataRequestInd(transport);
        send_success(transport, GAIA_COMMAND_VA_VOICE_DATA_REQUEST);
        return TRUE;
        
    case GAIA_COMMAND_VA_VOICE_END:
    /*  Host has detected the end of voice input; indicate this to client app  */
        gaiaVoiceAssistantSendVoiceEndInd(transport);
        send_success(transport, GAIA_COMMAND_VA_VOICE_END);
        return TRUE;

    case GAIA_COMMAND_VA_ANSWER_START:
    /*  Host has signalled the start of voice response; indicate this to client app  */
        gaiaVoiceAssistantSendAnswerStartInd(transport);
        send_success(transport, GAIA_COMMAND_VA_ANSWER_START);
        return TRUE;
        
    case GAIA_COMMAND_VA_ANSWER_END:
    /*  Host has signalled the end of voice response; indicate this to client app  */
        gaiaVoiceAssistantSendAnswerEndInd(transport);
        send_success(transport, GAIA_COMMAND_VA_ANSWER_END);
        return TRUE;
        
    case GAIA_COMMAND_VA_CANCEL:
    /*  Host wishes to cancel VA session; indicate this to client app  */
        if ((payload_length == 1) && (payload[0] < gaia_va_cancel_reason_top))
        {
            gaia_va_cancel_reason_t reason = payload[0];
            
            gaiaVoiceAssistantSendCancelInd(transport, reason);
            send_success(transport, GAIA_COMMAND_VA_CANCEL);
        }
        else
        {
            send_invalid_parameter(transport, GAIA_COMMAND_VA_CANCEL);
        }
        
        return TRUE;
    }
    
    return FALSE;
}


/*************************************************************************
NAME
    GaiaHasVoiceAssistant

DESCRIPTION
    Returns TRUE if the connected host supports our VA protocol.
*/
bool GaiaHasVoiceAssistant(GAIA_TRANSPORT *transport)
{
    bool has_voice_assistant = FALSE;
    
    if (transport)
    {
        has_voice_assistant = ((gaia_transport *) transport)->has_voice_assistant;
    }
    
    return has_voice_assistant;
}


/*************************************************************************
NAME
    GaiaVoiceAssistantStartRequest

DESCRIPTION
    Requests the host device to initiate a VA session.
*/
void GaiaVoiceAssistantStartRequest(GAIA_TRANSPORT *transport)
{
    if (GaiaHasVoiceAssistant(transport))
    {
        send_simple_command(transport, GAIA_COMMAND_VA_START, 0, NULL);
    }
    else
    {
        gaiaVoiceAssistantSendStartCfm((gaia_transport *) transport, FALSE);
    }
}


/*************************************************************************
NAME
    GaiaVoiceAssistantVoiceEnd

DESCRIPTION
   Indicates to the Host that the Device has detected the end of voice
   input
   
*/
void GaiaVoiceAssistantVoiceEnd(GAIA_TRANSPORT *transport)
{
    if (GaiaHasVoiceAssistant(transport))
    {
        send_simple_command(transport, GAIA_COMMAND_VA_VOICE_END, 0, NULL);
    }
    else
    {
        gaiaVoiceAssistantSendVoiceEndCfm((gaia_transport *) transport, FALSE);
    }
}


/*************************************************************************
NAME
    GaiaVoiceAssistantCancelRequest

DESCRIPTION
    Requests the host device to cancel a VA session.
*/
void GaiaVoiceAssistantCancelRequest(GAIA_TRANSPORT *transport, gaia_va_cancel_reason_t reason)
{
    if (GaiaHasVoiceAssistant(transport))
    {
        uint8 payload = (uint8) reason;
        
        send_simple_command(transport, GAIA_COMMAND_VA_CANCEL, 1, &payload);
    }
    else
    {
        gaiaVoiceAssistantSendCancelCfm((gaia_transport *) transport, FALSE);
    }
}

/*************************************************************************
NAME
    gaiaVoiceAssistantFlushData
    
DESCRIPTION
    Helper function which flushes voice assistant data to GAIA sink
*/

static bool gaiaVoiceAssistantFlushData(Sink sink, uint16 payload_size, const uint8 *data)
{
    bool ok = FALSE;
    uint16 gaia_packet_len = payload_size + GAIA_OFFS_PAYLOAD;
    
    GAIA_VA_DEBUG(("gaia: va: payload %u packet %u\n", payload_size, gaia_packet_len));
    
    if (SinkClaim(sink, gaia_packet_len) != BAD_SINK_CLAIM)
    {
        uint8 *dest = SinkMap(sink);
        
        gaiaBuildVaDataPacket(dest, data, payload_size);
        
        if(SinkFlush(sink, gaia_packet_len))
        {
            GAIA_VA_DEBUG(("gaia: va: flushed %u\n", payload_size));
            ok = TRUE;
        }
        else
        {
            GAIA_VA_DEBUG(("gaia: va: flush failed\n"));
        }
    }
    else
    {
        GAIA_VA_DEBUG(("gaia: va: claim failed \n"));
    }
    
    return ok;
}




/*************************************************************************
NAME
    gaiaVoiceAssistantSendToSink

DESCRIPTION
    Moves one or more GAIA_COMMAND_VA_VOICE_DATA headers plus voice data
    to the sink and flushes them.
    
RETURNS
    Length of voice data sent
*/
static uint16 gaiaVoiceAssistantSendToSink(gaia_transport *transport,
                                           uint16 length,
                                           const uint8 *data,
                                           uint16 num_packets)
{
    uint16 payload_len = 0;  /*Size of payload in each GAIA packet*/
    uint16 sent_data_len = 0; /* Lenght of VA data sent */
    uint16 remaining_packet = num_packets;
    uint16 max_gaia_va_packet = (GAIA_MAX_PAYLOAD / (length/num_packets));
    Sink sink = transport->state.spp.sink;

    if (sink)
    {
        GAIA_VA_DEBUG(("gaia: va: max %u packets\n",max_gaia_va_packet));
        while(remaining_packet)
        {
            /* Check the available sink size */
            uint16 num_pkt_can_send = 0;
            GAIA_VA_DEBUG(("gaia: va: %u packets\n",remaining_packet));

            num_pkt_can_send = ((SinkSlack(sink) - GAIA_OFFS_PAYLOAD)/(length/num_packets));
            /* is that we have more space than max packet? */
            if(num_pkt_can_send > max_gaia_va_packet)
                num_pkt_can_send = max_gaia_va_packet;

            /* is that we have more space, but less packet to send? */
            if(remaining_packet < num_pkt_can_send)
                num_pkt_can_send = remaining_packet;
            /* calculate the payload_len based on number of packets to send */
            payload_len = num_pkt_can_send*(length/num_packets);   

            if(payload_len && gaiaVoiceAssistantFlushData(sink,payload_len, &data[sent_data_len]))
            {
                remaining_packet -= num_pkt_can_send;
                sent_data_len += payload_len;
            }
            else
            {
                GAIA_VA_DEBUG(("gaia: va: send failed \n"));
                break;
            }
            
            GAIA_VA_DEBUG(("gaia: va: sent %u bytes\n", sent_data_len));
        }
    }
    
    return sent_data_len;
}

/*************************************************************************
NAME
    GaiaVoiceAssistantSendData
    
DESCRIPTION
    Send voice data to the (single if any) connected Host supporting VA.
    Returns the number of voice data octets sent.
    
NOTE
    This function does not have a transport parameter.
    The implicit GAIA_COMMAND_VA_VOICE_DATA is not acknowledged.
    The caller is responsible for handling failure.
*/
uint16 GaiaVoiceAssistantSendData(uint16 length, const uint8 *data, uint16 num_packets)
{
    uint16 sent_data_len = 0;
    gaia_transport *transport = gaiaVoiceAssistantFindTransport();
        
    if (transport)
    {
        {
            sent_data_len = gaiaVoiceAssistantSendToSink(transport, length, data, num_packets);
        }
    }
    
    return sent_data_len;
}

/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_packet_handler.c

DESCRIPTION

NOTES

*/



/****************************************************************************
    Header files
*/

#include "a2dp_private.h"
#include "a2dp_packet_handler.h"
#include "a2dp_command_handler.h"
#include "a2dp_data_block.h"
#include "a2dp_process_command.h"
#include "a2dp_api.h"

#include <sink.h>
#include <source.h>
#include <stream.h>
#include <string.h>
#include <stdlib.h>
#include <print.h>


/****************************************************************************
NAME
    validateCommandPDU

DESCRIPTION
    Does a quick check to see if the length of a command packet is reasonable
    for its type.  Typically this is just the mandatory fields, so any optional
    fields should be checked during processing.

RETURNS
    bool - TRUE if packet is reasonable
*/
static const uint8 avdtp_min_pdu_length[14] =
{
    2, /* avdtp_null                 */
    2, /* avdtp_discover             */
    3, /* avdtp_get_capabilities     */
    5, /* avdtp_set_configuration    */
    3, /* avdtp_get_configuration    */
    5, /* avdtp_reconfigure          */
    3, /* avdtp_open                 */
    3, /* avdtp_start                */
    3, /* avdtp_close                */
    3, /* avdtp_suspend              */
    3, /* avdtp_abort                */
    3, /* avdtp_security_control     */
    3, /* avdtp_get_all_capabilities */
    5  /* avdtp_delayreport          */
};

/****************************************************************************/
uint8 *a2dpGrabSink(Sink sink, uint16 size)
{
    uint8 *dest = SinkMap(sink);
    uint16 claim_result = SinkClaim(sink, size);
    if (claim_result == 0xffff)
    {
        return NULL;
    }

    return (dest + claim_result);
}

static bool sinkInsertHeaderAndFlushData(Sink sink, uint16 amount_to_flush, const uint8 *header, uint16 hdrlen)
{
    uint16 prev_sink_claimed = SinkClaim(sink, hdrlen);

    if (prev_sink_claimed != 0xFFFF)
    {
        uint8 * sink_base = SinkMap(sink);

        /* Copy all existing data in the buffer down to make room for the 
         * the header */
        memmove(sink_base + hdrlen, sink_base, prev_sink_claimed);
        memmove(sink_base, header, hdrlen);
        SinkFlush(sink, amount_to_flush + hdrlen);

        return TRUE;
    }

    return FALSE;
}


/****************************************************************************/
static bool sendPacket(remote_device *device, uint8 signalling_header, uint8 signal_id, uint16 payload_size)
{
    uint8 header[3];
    signalling_channel *signalling = &device->signal_conn;

#ifdef DEBUG_PRINT_ENABLED
    uint16 i;
    uint8 *sink_data = SinkMap(signalling->connection.active.sink);
    PRINT(("sendPacket device=%p signalling_header=%X signal_id=%u payload_size=%u  <", (void*)device, signalling_header, signal_id, payload_size));
    for(i=0; i<payload_size; i++)  PRINT(("%X ",sink_data[i]));
    PRINT((">\n"));
#endif

    if (payload_size < ((uint16)signalling->connection.active.bitfields.mtu - 1))
    {   /* Enough space in MTU to fit packet and two byte header */
        header[0] = (signalling_header & 0xF3) | (avdtp_packet_type_single << 2);
        header[1] = signal_id;
        return sinkInsertHeaderAndFlushData(signalling->connection.active.sink, payload_size, header, 2);
    }
    else
    {   /* Packet won't fit in MTU and needs to be fragmented */
        /* Start Packet */
        header[0] = (signalling_header & 0xF3) | (avdtp_packet_type_start << 2);
        header[1] = (uint8)((payload_size + signalling->connection.active.bitfields.mtu) / (signalling->connection.active.bitfields.mtu - 1));    /* Total number of fragments, inc start packet */
        header[2] = signal_id;
        payload_size -= (signalling->connection.active.bitfields.mtu - 3);
        if (!sinkInsertHeaderAndFlushData(signalling->connection.active.sink, signalling->connection.active.bitfields.mtu-3, header, 3))
        {
            return FALSE;
        }

        /* Continuation Packets */
        header[0] = (signalling_header & 0xF3) | (avdtp_packet_type_continue << 2);
        while (payload_size > ((uint16)signalling->connection.active.bitfields.mtu - 1))
        {
            payload_size -= (signalling->connection.active.bitfields.mtu - 1);
            if (!sinkInsertHeaderAndFlushData(signalling->connection.active.sink, signalling->connection.active.bitfields.mtu-1, header, 1))
            {
                return FALSE;
            }
        }

        /* End Packet */
        header[0] = (signalling_header & 0xF3) | (avdtp_packet_type_end << 2);
        return sinkInsertHeaderAndFlushData(signalling->connection.active.sink, payload_size, header, 1);
    }
}

/*****************************************************************************/
void a2dpSendAccept (remote_device *device, avdtp_signal_id signal_id, uint16 payload_size)
{
    uint8 transaction_label = device->signal_conn.connection.active.bitfields.received_transaction_label << 4;

    /* Clear pending transaction flag as we are now sending a response */
    device->signal_conn.status.pending_received_transaction = FALSE;

    PRINT(("a2dpSendAccept device=%p signal_id=%u payload_size=%u\n", (void*)device, signal_id, payload_size));

    PanicFalse( sendPacket(device, transaction_label | avdtp_message_type_response_accept, signal_id, payload_size) );

}

void a2dpSendReject (remote_device *device, avdtp_signal_id signal_id, uint16 error_code)
{
    uint8 *payload;
    uint8 payload_size;

    PRINT(("a2dpSendReject device=%p signal_id=%u error_code=%u", (void*)device, signal_id, error_code));

    /* TODO: Convert this to a table */
    if ( (signal_id == avdtp_set_configuration) ||
         (signal_id == avdtp_reconfigure) ||
         (signal_id == avdtp_start) ||
         (signal_id == avdtp_suspend) )
    {
        payload_size = 2;
    }
    else
    {
        payload_size = 1;
    }

    if ( (payload = a2dpGrabSink(device->signal_conn.connection.active.sink, payload_size))!=NULL )
    {
        uint8 transaction_label = device->signal_conn.connection.active.bitfields.received_transaction_label << 4;

        /* Clear pending transaction flag as we are now sending a response */
        device->signal_conn.status.pending_received_transaction = FALSE;

        if (payload_size > 1)
        {   /* Top byte of error code contains additional, signal id specific, information */
            *payload++ = (uint8)(error_code>>8);
        }

        /* The actual error code */
        *payload = error_code&0xFF;

        PanicFalse( sendPacket(device, transaction_label | avdtp_message_type_response_reject, signal_id, payload_size) );
    }
}

void a2dpSendGeneralReject (remote_device *device)
{
    const uint8 *ptr = device->signal_conn.connection.active.received_packet;
    uint8 transaction_label = device->signal_conn.connection.active.bitfields.received_transaction_label << 4;

    /* Clear pending transaction flag as we are now sending a response */
    device->signal_conn.status.pending_received_transaction = FALSE;

    PRINT(("a2dpSendGeneralReject device=%p\n", (void*)device));

    if ((device->bitfields.profile_version > avdtp_version_unknown) && (device->bitfields.profile_version < avdtp_version_13))
    {
        PanicFalse( sendPacket(device, transaction_label, avdtp_null, 0) );
    }
    else
    {
        PanicFalse( sendPacket(device, transaction_label | avdtp_message_type_general_reject, ptr[1], 0) );
    }
}


bool a2dpSendCommand(remote_device *device, uint8 signal_id, const uint8* payload_data, uint16 payload_size)
{
    uint8  command_header[2];
    uint16 command_header_size = 1;
    uint8* payload;
    Sink sink = device->signal_conn.connection.active.sink;

    PRINT(("a2dpSendCommand device=%p signal_id=%u payload_size=%u\n", (void*)device, signal_id, payload_size));

    if (!sink || !SinkIsValid(sink))
    {
        return FALSE;
    }

    if (device->signal_conn.status.pending_issued_transaction && (signal_id != avdtp_close) && (signal_id != avdtp_abort))
    {   /* command is already pending. */
        return FALSE;
    }

    /* The discover command has no header */
    if (signal_id == avdtp_discover)
    {
        command_header_size = 0;
    }
    else
    {
        command_header[0] = (uint8)(device->remote_sep.seid << 2);
        if (signal_id == avdtp_set_configuration)
        {
            command_header[1] = (uint8)(a2dpGetLocalSeid(device) << 2);
            command_header_size = 2;
        }
    }

    if ((payload = a2dpGrabSink(sink, payload_size + command_header_size)) == NULL)
    {
        return FALSE;
    }

    device->signal_conn.connection.active.bitfields.issued_transaction_label = (device->signal_conn.connection.active.bitfields.issued_transaction_label + 1) & 0xF;

    if(device->signal_conn.connection.active.bitfields.issued_transaction_label == 0)
    {
        device->signal_conn.connection.active.bitfields.issued_transaction_label = 1;
    }

    device->signal_conn.status.pending_issued_transaction = TRUE;

    if (command_header_size)
    {
        memcpy(payload,command_header,command_header_size);
        payload += command_header_size;
    }

    if (payload_data != NULL)
    {
        memmove(payload, payload_data, payload_size);
    }

    return sendPacket(device, (device->signal_conn.connection.active.bitfields.issued_transaction_label << 4) | avdtp_message_type_command, signal_id, payload_size + command_header_size);
}


/****************************************************************************
NAME
    processGeneralRejectResponse

DESCRIPTION
    The remote device has rejected our command.

RETURNS
    void
*/
static void processGeneralRejectResponse(remote_device *device)
{
    const uint8 *ptr = device->signal_conn.connection.active.received_packet;

    if (device->signal_conn.status.pending_issued_transaction && (device->signal_conn.connection.active.bitfields.issued_transaction_label != (ptr[0] >> 4)))
    {
        PRINT(("No pending issued transaction or invalid transaction id\n"));
        return;
    }

    /* handle reject for recoverable states such as
       optional features. */
    if (device->signal_conn.status.stream_state == avdtp_stream_local_suspending)
    {
        a2dpMediaSuspendCfm(device, &device->media_conn[0], a2dp_rejected_by_remote_device);

        /* return to open */
        a2dpSetStreamState(device, avdtp_stream_open);
    }
#if 0   /* TODO: */
    else if (device->signal_conn.status.stream_state == avdtp_stream_reconfiguring)
    {
        a2dpMediaReconfigureCfm(device, &device->media_conn[0], a2dp_rejected_by_remote_device);

        /* return to open */
        a2dpSetSignallingState(avdtp_stream_open);
    }
#endif
    else
    {
        /* can't handle rejection - abort */
        a2dpStreamAbort(device);
    }
}


/*****************************************************************************/
static bool handleReceiveCommand(remote_device *device)
{
    const uint8 *ptr = device->signal_conn.connection.active.received_packet;
    uint16 packet_size = device->signal_conn.connection.active.bitfields.received_packet_length;
    Sink sink = device->signal_conn.connection.active.sink;

    PRINT(("handleReceiveCommand %d - ", device->bitfields.device_id));

    if (!sink || !SinkIsValid(sink))
    {
        PRINT(("invalid sink\n"));
        return FALSE;
    }

    if (device->signal_conn.status.pending_received_transaction)
    {
        PRINT(("pending received transaction\n"));
        return FALSE;
    }

    /* Store the received transaction label for later use */
    device->signal_conn.connection.active.bitfields.received_transaction_label = (ptr[0] >> 4) & 0xF;
    device->signal_conn.status.pending_received_transaction = TRUE;

    /* check command is a reasonable length.
       We do this check here to prevent the other
       functions having to do it.
    */
    if(ptr[1] > avdtp_max_signal_id)
    {   /* Unknown signal id, send a general reject */
        PRINT(("unknown signal id\n"));
        a2dpSendGeneralReject(device);
        return TRUE;
    }

    if (packet_size < avdtp_min_pdu_length[ptr[1]])
    {   /* PDU is too short for specified signal id */
        PRINT(("PDU too short\n"));
        a2dpSendReject(device, ptr[1], avdtp_bad_length);
        return TRUE;
    }

    /* Call handler function to process command */
    switch (ptr[1])
    {
        case avdtp_null:
            PRINT(("avdtp_null\n"));
            if (device->bitfields.profile_version < avdtp_version_13)
            {  /* Strangely, the response to an unsupported command is General Reject which has a
                  null message-type and so looks like a command. We therefore need to process as a response! */
                processGeneralRejectResponse(device);
            }
            else
            {   /* For AVDTP v1.3, this is a reserved command, so issue a general reject */
                a2dpSendGeneralReject(device);
            }
            break;

        case avdtp_discover:
            PRINT(("avdtp_discover\n"));
            a2dpHandleDiscoverCommand(device);
            break;

        case avdtp_get_capabilities:
            PRINT(("avdtp_get_capabilities\n"));
            a2dpHandleGetCapabilitiesCommand(device);
            break;

        case avdtp_set_configuration:
            PRINT(("avdtp_set_configuration\n"));
            a2dpHandleSetConfigurationCommand(device);
            break;

        case avdtp_get_configuration:
            PRINT(("avdtp_get_configuration\n"));
            a2dpHandleGetConfigurationCommand(device);
            break;

        case avdtp_reconfigure:
            PRINT(("avdtp_reconfigure\n"));
            a2dpHandleReconfigureCommand(device);
            break;

        case avdtp_open:
            PRINT(("avdtp_open\n"));
            a2dpHandleOpenCommand(device);
            break;

        case avdtp_start:
            PRINT(("avdtp_start\n"));
            if (!a2dpHandleStartCommand(device))
            {
                device->signal_conn.status.pending_received_transaction = FALSE;
                return FALSE;
            }
            break;

        case avdtp_close:
            PRINT(("avdtp_close\n"));
            if (!a2dpHandleCloseCommand(device))
            {
                device->signal_conn.status.pending_received_transaction = FALSE;
                return FALSE;
            }
            break;

        case avdtp_suspend:
            PRINT(("avdtp_suspend\n"));
            a2dpHandleSuspendCommand(device);
            break;

        case avdtp_abort:
            PRINT(("avdtp_abort\n"));
            a2dpHandleAbortCommand(device);
            break;

        case avdtp_security_control:
            PRINT(("avdtp_security_control\n"));
            a2dpSendGeneralReject(device);  /* TODO: is this correct? */
            break;
            
        case avdtp_get_all_capabilities:
            PRINT(("avdtp_get_all_capabilities\n"));
            a2dpHandleGetAllCapabilitiesCommand(device);
            break;
            
        case avdtp_delayreport:
            PRINT(("avdtp_delayreport\n"));
            a2dpHandleDelayReportCommand(device);
            break;
            
        default:
            /* Should have been caught earlier */
            Panic();
            break;
    }

    return TRUE;
}

/*****************************************************************************/
static void handleReceiveResponse(remote_device *device)
{
    signalling_channel *signalling = &device->signal_conn;
    const uint8* ptr = signalling->connection.active.received_packet;
    uint16 packet_size = signalling->connection.active.bitfields.received_packet_length;

    PRINT(("handleReceiveResponse %d - ", device->bitfields.device_id));

    if (signalling->status.pending_issued_transaction && (signalling->connection.active.bitfields.issued_transaction_label != (ptr[0] >> 4)))
    {   /* Transaction id does not match */
        PRINT(("No pending issued transaction or invalid transaction id\n"));
        return;
    }

    /* clear pending transaction as we have a response */
    signalling->status.pending_issued_transaction = FALSE;

    /* check the PDU is reasonable */
    if (packet_size < 2)
    {   /* Must contain at least a header and signal id */
        PRINT(("invalid PDU size\n"));
        return;
    }

    switch (ptr[1])
    {
        case avdtp_discover:
            PRINT(("avdtp_discover\n"));
            a2dpHandleDiscoverResponse(device);
            break;

        case avdtp_get_capabilities:
            PRINT(("avdtp_get_capabilities\n"));
            a2dpHandleGetCapabilitiesResponse(device);
            break;

        case avdtp_set_configuration:
            PRINT(("avdtp_set_configuration\n"));
            a2dpHandleSetConfigurationResponse(device);
            break;

        case avdtp_get_configuration:
            /* We never send the request, so don't expect a reply! */
            PRINT(("avdtp_get_configuration\n"));
            break;

        case avdtp_reconfigure:
            PRINT(("avdtp_reconfigure\n"));
            a2dpHandleReconfigureResponse(device);
            break;

        case avdtp_open:
            PRINT(("avdtp_open\n"));
            a2dpHandleOpenResponse(device);
            break;

        case avdtp_start:
            PRINT(("avdtp_start\n"));
            a2dpHandleStartResponse(device);
            break;

        case avdtp_close:
            PRINT(("avdtp_close\n"));
            a2dpHandleCloseResponse(device);
            break;

        case avdtp_suspend:
            PRINT(("avdtp_suspend\n"));
            a2dpHandleSuspendResponse(device);
            break;

        case avdtp_abort:
            PRINT(("avdtp_abort\n"));
            a2dpHandleAbortResponse(device);
            break;

        case avdtp_security_control:
            /* We never send the request, so don't expect a reply! */
            PRINT(("avdtp_security_control\n"));
            break;
            
        case avdtp_get_all_capabilities:
            PRINT(("avdtp_get_all_capabilities\n"));
            a2dpHandleGetAllCapabilitiesResponse(device);
            break;
            
        case avdtp_delayreport:
            PRINT(("avdtp_delayreport\n"));
            a2dpHandleDelayReportResponse(device);
            break;
            
        default:
            /* Silently ignore responses with an invalid signal id */
            break;
    }
}

/****************************************************************************
NAME
    handleBadHeader

DESCRIPTION
    This function is called when a complete signal packet is received which
    contains an invalid header.

RETURNS
    void
*/
static void handleBadHeader(remote_device *device)
{
    signalling_channel *signalling = &device->signal_conn;
    const uint8 *ptr = signalling->connection.active.received_packet;
    uint16 packet_size = signalling->connection.active.bitfields.received_packet_length;
    Sink sink = signalling->connection.active.sink;

    PRINT(("handleBadHeader\n"));

    if (!sink || !SinkIsValid(sink))
        return;

    if (packet_size < 2)
        return;
        
    /* As per AVDTP specification version and section 8.4.6 The receiver of an AVDTP signalling
        message shall not interpret or pass corrupted messages to the upper layer. Those messages
        are discarded and not signalled to the local upper layer; no signalling message is returned to 
        the sender. Possible corrupted messages are:

        All incoming messages with the message type set to RFD = 2 bits value 01*/

    if (ptr[1] == avdtp_discover)
    {
        return;
    }

    /* Store the received transaction label for later use */
    device->signal_conn.connection.active.bitfields.received_transaction_label = (ptr[0] >> 4) & 0xF;
    device->signal_conn.status.pending_received_transaction = TRUE;

    /* reject header */
    a2dpSendReject(device, ptr[1], avdtp_bad_header_format);
}


/****************************************************************************
NAME
    processSignalPacket

DESCRIPTION
    This function is called to process a complete signal packet receiving on
    the signalling channel

RETURNS
    void
*/
static bool processSignalPacket(remote_device *device)
{
    uint8 message_type = *device->signal_conn.connection.active.received_packet & 0x03;

#ifdef DEBUG_PRINT_ENABLED
    uint16 i;
    PRINT(("processSignalPacket length=%u  <",device->signal_conn.connection.active.bitfields.received_packet_length));
    for(i=0;i<device->signal_conn.connection.active.bitfields.received_packet_length;i++) PRINT(("%X ",device->signal_conn.connection.active.received_packet[i]));
    PRINT((">\n"));
#endif

    /* decode header fields */
    switch (message_type)
    {
        case avdtp_message_type_command:
            /* process the command */
            if ( !handleReceiveCommand(device) )
            {   /* it was inappropriate to process the command, so go away and come back later */
                PRINT((" (deferred)\n"));
                return FALSE;
            }
            break;
        
        case avdtp_message_type_general_reject:
            if (device->bitfields.profile_version < avdtp_version_13)
            {
                handleBadHeader(device);
            }
            else
            {
                processGeneralRejectResponse(device);
            }
            break;
            
        case avdtp_message_type_response_accept:
        case avdtp_message_type_response_reject:
            handleReceiveResponse(device);
            break;
    }

    return TRUE;
}


/****************************************************************************
NAME
    a2dpHandleSignalPacket

DESCRIPTION
    This function is called to process signal packets arriving on the signalling channel.
    It performs reassembly of fragmented, if necessary, and passes the signal to processSignalPacket()

RETURNS
    void
*/
void a2dpHandleSignalPacket(remote_device *device)
{
    if ( device && (device->bitfields.profile_version != avdtp_version_not_requested) )
    {
        uint16 packet_size;
        signalling_channel *signalling = &device->signal_conn;
        Source source = StreamSourceFromSink(signalling->connection.active.sink);

        if (!source || !SourceIsValid(source))
        {
            return;
        }

        while ((packet_size = SourceBoundary(source)) != 0)
        {
            const uint8 *ptr = SourceMap(source);
            uint16 type = (ptr[0]>>2) & 3;

            switch (type)
            {
                case avdtp_packet_type_single:
                {
                    bool processed;

                    if (signalling->connection.active.received_packet != NULL)
                    {   /* Received a single packet in the middle of reassembling a fragment */
                        free(signalling->connection.active.received_packet);
                    }
                    /* Packets longer than 1 part are malloc'd so we can't keep the 'const' on the pointer, cast away */
                    signalling->connection.active.received_packet = (uint8*)ptr;
                    signalling->connection.active.bitfields.received_packet_length = packet_size;

                    /* complete packet, process it now */
                    processed = processSignalPacket( device );
                    signalling->connection.active.received_packet = NULL;

                    if ( !processed )
                    {   /* exit early so we don't drop the packet */
                        return;
                    }
                }
                break;

                case avdtp_packet_type_start:
                {
                    if (signalling->connection.active.received_packet != NULL)
                    {
                        /*
                            We've received a start message part
                            way through reassembling another message.
                            Discard the old and start again.
                        */
                        free(signalling->connection.active.received_packet);
                    }

                    /*
                        Allocate some memory to store the data until
                        the next fragment arrives.
                    */
                    signalling->connection.active.received_packet = (uint8 *)malloc(packet_size-1);
                    if (signalling->connection.active.received_packet != NULL)
                    {
                        /* copy the packet into RAM but discard the
                           Number of Fragments field as we don't trust the
                           remote device to set it correctly. */
                        signalling->connection.active.received_packet[0] = ptr[0];
                        memmove(signalling->connection.active.received_packet+1, ptr+2, packet_size-2);
                        signalling->connection.active.bitfields.received_packet_length = packet_size-1;
                    }
                }
                break;

                case avdtp_packet_type_continue:
                case avdtp_packet_type_end:
                {
                    uint8 *new_rx_packet;

                    /* ignore continuation unless we've seen the start */
                    if (signalling->connection.active.received_packet == NULL)
                        break;

                    /* check the transaction label is the same as the start */
                    if ((signalling->connection.active.received_packet[0] & 0xf0) != (ptr[0] & 0xf0))
                        break;

                    /* grow the reassembled packet to add the new fragment */
                    new_rx_packet = realloc(signalling->connection.active.received_packet, signalling->connection.active.bitfields.received_packet_length + packet_size - 1);
                    if (!new_rx_packet)
                    {
                        /* Failed to grow the packet. Release what we have and abandon processing */
                        free(signalling->connection.active.received_packet);
                        signalling->connection.active.received_packet = NULL;
                        break;
                    }
                    signalling->connection.active.received_packet = new_rx_packet;

                    /* copy the continuation fragment (less the header) */
                    memmove(signalling->connection.active.received_packet+signalling->connection.active.bitfields.received_packet_length, ptr+1, packet_size-1);
                    signalling->connection.active.bitfields.received_packet_length += packet_size-1;

                    if (type == avdtp_packet_type_end)
                    {
                        processSignalPacket( device );
                        /* free the message */
                        free(signalling->connection.active.received_packet);
                        signalling->connection.active.received_packet = NULL;
                    }
                }
                break;
            }

            /* discard the fragment */
            SourceDrop(source, packet_size);
        }
    }
}


/****************************************************************************
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_control_handler.c        
DESCRIPTION
    handles transmitting and receiving HID packets on the control channel.
*/

#include <source.h>
#include <sink.h>
#include <stream.h>
#include <string.h>

#include "hid.h"
#include "hid_private.h"
#include "hid_common.h"
#include "hid_connect.h"
#include "hid_control_handler.h"

/* Local Function Prototypes */
static void hidHandshakeSend(HID *hid, hid_status result);
static void hidPacketSend(HID *hid, uint8 header, const uint8 *data, uint16 length);
static hidPacketStatus hidPacketStart(HID *hid, uint16 packet, const uint8 *data, uint8 length);
static void hidPacketStop(HID *hid, const uint8 *data, uint16 length);
static hidPacketStatus hidPacketAppend(HID *hid, const uint8 *data, uint16 length);
static void hidPacketFailed(HID *hid, uint16 packet, hid_status status);

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleHidGetReportInd
DESCRIPTION
    This function handles the HID_GET_REPORT packet.
RETURNS
    void
*/
static void hidHandleHidGetReportInd(HID *hid, const uint8 *data, uint16 length)
{
    MAKE_HID_MESSAGE_OR_NULL(HID_GET_REPORT_IND);
    HID_PRINT(("hidHandleHidGetReportInd, length %d\n", length));
    
    /* Check we allocated message */
    if (message != NULL)
    {
        /* Get report type */
        message->report_type = data[0] & 0x03;
    
        /* Trim the GET_REPORT packet to its maximum permissable length */
        if(length > HID_GET_REPORT_HEADER_MAX_LEN)
            length = HID_GET_REPORT_HEADER_MAX_LEN;
        
        /* Check if we have a report id, we do if length is even */
        if (length & 0x01)
            message->report_id = 0xff;
        else
            message->report_id = data[1];

        /* Check if buffer size is specified */
        if (data[0] & (1 << 3))
        {
            /* NOTE: Limit for payload WITHOUT header */
            message->report_length = ((data[length - 1] << 8) | data[length - 2]);
        }
        else
        {
            /* Report the size as 0xFFFF - should be big enough for the response! */
            message->report_length = 0xFFFF;
        }
    
        /* Send HID_GET_REPORT_IND to application */
        message->hid = hid;
        MessageSend(hid->app_task, HID_GET_REPORT_IND, message);
    }
    else
    {
        hidPacketFailed(hid, HID_GET_REPORT_IND, hid_failed);
    }
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleHidSetReportInd
DESCRIPTION
    This function handles the HID_SET_REPORT packet.
RETURNS
    void
*/
static void hidHandleHidSetReportInd(HID *hid, const uint8 *data, uint16 length)
{
    MAKE_HID_MESSAGE_WITH_LEN_OR_NULL(HID_SET_REPORT_IND, length - 1);        
    HID_PRINT(("hidHandleHidSetReportInd, length %d\n", length));
    
    /* Check if we allocated message */
    if (message != NULL)
    {
        /* Get report type */
        message->report_type = data[0] & 0x03;
    
        /* Get report length */
        message->size_report_data = length - 1;
    
        /* Copy report */
        memcpy(message->report_data, &data[1], length - 1);
    
        /* Send HID_SET_REPORT_IND to application */
        message->hid = hid;
        MessageSend(hid->app_task, HID_SET_REPORT_IND, message);
    }
    else
    {
        hidPacketFailed(hid, HID_SET_REPORT_IND, hid_failed);
    }
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleHidGetIdleInd
DESCRIPTION
    This function handles the HID_GET_IDLE packet.
RETURNS
    void
*/
static void hidHandleHidGetIdleInd(HID *hid, const uint8 *data, uint16 length)
{
    MAKE_HID_MESSAGE_OR_NULL(HID_GET_IDLE_IND);
    HID_PRINT(("hidHandleHidGetIdleInd\n"));
    UNUSED(length);
    UNUSED(*data);

    /* Check if we allocated message */
    if (message != NULL)
    {
        /* Send HID_GET_IDLE_IND to application */
        message->hid = hid;
        MessageSend(hid->app_task, HID_GET_IDLE_IND, message);
    }
    else
    {
        hidPacketFailed(hid, HID_GET_IDLE_IND, hid_failed);
    }
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleHidSetIdleInd
DESCRIPTION
    This function handles the HID_SET_IDLE packet.
RETURNS
    void
*/
static void hidHandleHidSetIdleInd(HID *hid, const uint8 *data, uint16 length)
{
    MAKE_HID_MESSAGE_OR_NULL(HID_SET_IDLE_IND);
    HID_PRINT(("hidHandleHidSetIdleInd\n"));
    UNUSED(length);

    /* Check if we allocated message */
    if (message != NULL)
    {
        /* Send HID_SET_IDLE_IND to application */
        message->hid = hid;
        message->idle_rate = data[1];
        MessageSend(hid->app_task, HID_SET_IDLE_IND, message);
    }
    else
    {
        hidPacketFailed(hid, HID_SET_IDLE_IND, hid_failed);
    }
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleHidGetProtocolInd
DESCRIPTION
    This function handles the HID_GET_PROTOCOL packet.
RETURNS
    void
*/
static void hidHandleHidGetProtocolInd(HID *hid, const uint8 *data, uint16 length)
{
    MAKE_HID_MESSAGE_OR_NULL(HID_GET_PROTOCOL_IND);
    HID_PRINT(("hidHandleHidGetProtocolInd\n"));
    UNUSED(length);
    UNUSED(*data);

    /* Check if we allocate message */
    if (message != NULL)
    {
        /* Send HID_GET_PROTOCOL_IND to application */
        message->hid = hid;
        MessageSend(hid->app_task, HID_GET_PROTOCOL_IND, message);
    }
    else
    {
        hidPacketFailed(hid, HID_GET_PROTOCOL_IND, hid_failed);
    }
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleHidSetProtocolInd
DESCRIPTION
    This function handles the HID_SET_PROTOCOL packet.
RETURNS
    void
*/
static void hidHandleHidSetProtocolInd(HID *hid, const uint8 *data, uint16 length)
{
    MAKE_HID_MESSAGE_OR_NULL(HID_SET_PROTOCOL_IND);
    HID_PRINT(("hidHandleHidSetProtocolInd\n"));
    UNUSED(length);

    /* Check if we allocated message */
    if (message != NULL)
    {
        /* Send HID_SET_PROTOCOL_IND to application */
        message->hid = hid;
        message->protocol = data[0] & 0x01;
        MessageSend(hid->app_task, HID_SET_PROTOCOL_IND, message);
    }
    else
    {
        hidPacketFailed(hid, HID_SET_PROTOCOL_IND, hid_failed);
    }
}
#endif


/****************************************************************************
NAME    
    hidPacketFailed
DESCRIPTION
    This function is called to send a confirmation to a request that has
    failed.
RETURNS
    void
*/
static void hidPacketFailed(HID *hid, uint16 packet, hid_status status)
{
    HID_PRINT(("hidPacketFailed\n"));

    /* Check if any packet pending */
    if (hid->packet_data != NULL)
    {
        /* Free packet first, so we have memory to send error message */
        free(hid->packet_data);
        hid->packet_data = NULL;
    }

    /* Handle packet type */
    switch (packet)
    {
#ifdef HID_DEVICE
        case HID_GET_REPORT_IND:
        case HID_SET_REPORT_IND:
        case HID_GET_PROTOCOL_IND:
        case HID_SET_PROTOCOL_IND:
        case HID_SET_IDLE_IND:
        case HID_GET_IDLE_IND:
        {
            /* Tell host */
            hidHandshakeSend(hid, status);
        }
        break;
#endif
        default:
            HID_DEBUG(("hidPacketFailed, unknown packet=%d\n", packet));
            break;
    }

    /* Cancel pending HID request timer */
    MessageCancelAll(&hid->task, HID_INTERNAL_REQUEST_TIMEOUT_IND);

    /* Clear packet pending */
    hid->packet_pending = 0;    
}

/****************************************************************************
NAME    
    hidPacketStop
DESCRIPTION
    This function is called when a resposne to a HID request has been
    received. 
RETURNS
    void
*/
void static hidPacketStop(HID *hid, const uint8 *data, uint16 length)
{
    HID_PRINT(("hidPacketStop\n"));

    /* Handle packet type */
    switch (hid->packet_pending)
    {
#ifdef HID_DEVICE
        case HID_GET_REPORT_IND:
            hidHandleHidGetReportInd(hid, data, length);
            break;
        case HID_SET_REPORT_IND:
            hidHandleHidSetReportInd(hid, data, length);
            break;
        case HID_GET_PROTOCOL_IND:
            hidHandleHidGetProtocolInd(hid, data, length);
            break;
        case HID_SET_PROTOCOL_IND:
            hidHandleHidSetProtocolInd(hid, data, length);
            break;                
        case HID_GET_IDLE_IND:
            hidHandleHidGetIdleInd(hid, data, length);
            break;
        case HID_SET_IDLE_IND:
            hidHandleHidSetIdleInd(hid, data, length);
            break;
#endif
    }
    
    /* Free any packet */
    if (hid->packet_data != NULL)
    {
        free(hid->packet_data);
        hid->packet_data = NULL;
    }

    /* Cancel pending HID request timer */
    MessageCancelAll(&hid->task, HID_INTERNAL_REQUEST_TIMEOUT_IND);
}

/****************************************************************************
NAME    
    hidPacketStart
DESCRIPTION
    This function attempts to start a control request, if a previous request
    is still pending then this function will return indicating a failure.
    otherwise it will store the pending request type and start a timer.    
RETURNS
    TRUE if request can be started, or FALSE if previous request still pending.
*/
static hidPacketStatus hidPacketStart(HID *hid, uint16 packet, const uint8 *data, uint8 length)
{
    /* Check if already request in progress */
    if (hid->packet_pending != 0)
    {
        HID_PRINT(("hidPacketStart packet=%d (BUSY)\n", packet));

        /* Return indicating packet was rejected because we are busy */
        return hidPacketStatusBusy;
    }    

    HID_PRINT(("hidPacketStart packet=%d\n", packet));

    /* Store packet type */
    hid->packet_pending = packet;
    hid->packet_data = NULL;
    hid->packet_size = 0;

    /* Check if we need to allocate memory for packet data */
    if (data != NULL)
    {
        /* Check if we have complete packet */
        if (length < HID_L2CAP_MTU)
        {
            /* Packet completed */
            hidPacketStop(hid, data, length);

            /* Return indicating we have completed the packet */
            return hidPacketStatusCompleted;
        }
        else
        {
            /* Allocate memory for packet */
            hid->packet_data = malloc(length);
                
            /* Check if allocation was successful */
            if (hid->packet_data != NULL)
            {
                /* Store first part of packet */
                memcpy(hid->packet_data, data, length);
                hid->packet_size = length;

                /* Start timer */
                HID_PRINT(("hidPacketStart, start timer\n"));
                MessageSendLater(&hid->task, HID_INTERNAL_REQUEST_TIMEOUT_IND, 0, D_SEC(HID_REQUEST_TIMEOUT));

                /* Return indicating packet is pending */
                return hidPacketStatusPending;
            }
            else
            {
                /* Packet has failed due to insufficent memory */
                hidPacketFailed(hid, packet, hid_failed);

                /* Return indicating packet has failed */
                return hidPacketStatusFailed;
            }
        }
    }
    else
    {
        /* Start timer */
        HID_PRINT(("hidPacketStart, start timer\n"));
        MessageSendLater(&hid->task, HID_INTERNAL_REQUEST_TIMEOUT_IND, 0, D_SEC(HID_REQUEST_TIMEOUT));

        /* Return indicating packet is pending */
        return hidPacketStatusPending;
    }
}

/****************************************************************************
NAME    
    hidPacketAppend
DESCRIPTION
    This function appends received data to the end of the current packet.
RETURNS
    void
*/
static hidPacketStatus hidPacketAppend(HID *hid, const uint8 *data, uint16 length)
{
    /* Check if we should be handling a packet */
    if (hid->packet_pending)
    {
        /* Attempt to allocate a bigger block */
        uint8 *packet_data = realloc(hid->packet_data, hid->packet_size + length);
        if (packet_data != NULL)
        {
            /* Store pointer to new block */
            hid->packet_data = packet_data;
                    
            /* Copy data to end of block */
            memcpy(packet_data + hid->packet_size, data, length);
            hid->packet_size += length;
                    
            /* Check if this is the last packet */
            if (length < HID_L2CAP_MTU)
            {
                /* Packet completed */
                hidPacketStop(hid, hid->packet_data, hid->packet_size);

                /* Return indicating packet completed */
                return hidPacketStatusCompleted;
            }
            else
            {
                /* Return indicating packet still pending */
                return hidPacketStatusPending;
            }
        }
        else
        {
            /* No memory, report failure */
            hidPacketFailed(hid, hid->packet_pending, hid_failed);

            /* Return indicating failure */
            return hidPacketStatusFailed;
        }
    }
    else
    {
        return hidPacketStatusFailed;
    }
}

/****************************************************************************
NAME    
    hidPacketSend
DESCRIPTION
    This function sends data packets over the control channel and carries out segmentation
    if necessary.
RETURNS
    void
*/
static void hidPacketSend(HID *hid, uint8 header, const uint8 *data, uint16 length)
{
    Sink sink = hid->connection[HID_CON_CONTROL].con.sink;
    HID_PRINT(("hidPacketSend, header=%x, length=%d\n", header, length));

    /* Check if we have a sink */
    if (sink)
    {
        uint16 mtu = hid->connection[HID_CON_CONTROL].remote_mtu;   

        /* Build packets */
        for (;;)
        {
            /* Get pointer to stream */
            uint8 *ptr = SinkMap(sink);

            /* Get packet data size (excludes header) */
            uint16 size = ((length + 1) > mtu) ? (mtu - 1) : length;

            /* Claim space in sink */
            uint16 offset = SinkClaim(sink, size + 1);
            if (offset == 0xFFFF)
                break;

            /* Store header */
            ptr[offset] = header;

            /* Copy data into packet */
            memcpy(&ptr[offset + 1], data, size);

            /* Flush packet */
            SinkFlush(sink, size + 1);

            /* Quit loop if we just handled a packet smaller than the MTU */
            if (size < (mtu - 1))
                break;

            /* Update header */
            header = (HID_DATC << 4) | (header & 0x0F);

            /* Adjust length remaining */
            length -= size;
            data += size;
        }
    }
}

/****************************************************************************
NAME    
    hidControlGetMinimumPacketSize
DESCRIPTION
    This function returns minimum packet length (including header for a packet)    
RETURNS
    void
*/
static uint16 hidControlGetMinimumPacketSize(uint8 header)
{
    switch (header)
    {
        case HID_CONTROL:
            return 1;
        case HID_GET_REPORT:
            return 1;
        case HID_SET_REPORT:
            return 2;
        case HID_GET_PROTOCOL:
            return 1;
        case HID_SET_PROTOCOL:
            return 1;
        case HID_GET_IDLE:
            return 1;
        case HID_SET_IDLE:
            return 2;
        default:
            return 0;
    }
}

/****************************************************************************
NAME    
    hidHandleHidControl
DESCRIPTION
    This function handles the HID_CONTROL packet.
RETURNS
    void
*/
static void hidHandleHidControl(HID *hid, hid_control_op operation)
{
    HID_PRINT(("hidHandleHidControl, operation=%d\n", operation));
    switch (operation)
    {
        case hid_control_op_nop:
            break;
#ifdef HID_DEVICE            
        case hid_control_op_hard_reset:
        case hid_control_op_soft_reset:
        case hid_control_op_suspend:
        case hid_control_op_exit_suspend:
        {
            MAKE_HID_MESSAGE(HID_CONTROL_IND);
        
            /* Send HID_CONTROL_IND to application */
            message->hid = hid;
            message->operation = operation;
            MessageSend(hid->app_task, HID_CONTROL_IND, message);
        }
        break;
#endif            
        case hid_control_op_unplug:
        {
            /* Set disconnect reason */
            hid->disconnect_status = hid_disconnect_virtual_unplug;

            /* Move to disconnecting local state */
            hidSetState(hid, hidDisconnectingLocal);
        }
        break;
            
        default:
        {
            /* Invalid operation */
            hidHandshakeSend(hid, hid_invalid_param);
        }
        break;
    }
}

/****************************************************************************
NAME    
    hidHandleControlPacket
DESCRIPTION
    This function decodes packets received in the control channel.
RETURNS
    void
*/
void hidHandleControlPacket(HID *hid, Source source)
{
    uint16 packet_size;
    Sink sink = StreamSinkFromSource(source);

    /* Check source is for control channel */
    if (sink == hid->connection[HID_CON_CONTROL].con.sink)
    {
        HID_PRINT(("hidHandleControlPacket, sink=%x (control)\n", (int)sink));

        /* Loop, pulling packets from source */
        while ((packet_size = SourceBoundary(source)) != 0) 
        {
            const uint8 *ptr = SourceMap(source);
            uint8 header = ptr[0] >> 4;
        
            /* Check packet is at least minimum size */
            if (packet_size >= hidControlGetMinimumPacketSize(header))
            {
                HID_PRINT(("hidHandleControlPacket, header=%d\n", header));

                /* Decode header */
                switch (header)
                {
                    case HID_CONTROL:
                    {
                        /* Handle control packet */
                        hidHandleHidControl(hid, ptr[0] & 0x0F);
                    }
                    break;
#ifdef HID_DEVICE          
                    case HID_GET_REPORT:
                    case HID_SET_REPORT:
                    case HID_GET_PROTOCOL:
                    case HID_SET_PROTOCOL:
                    case HID_GET_IDLE:
                    case HID_SET_IDLE:
                    {
                        hidPacketStatus packet_status;

                        /* Attempt to start packet */
                        packet_status = hidPacketStart(hid, header + HID_IND_BASE, ptr, packet_size);

                        /* Check if device is currently busy */
                        if (packet_status == hidPacketStatusBusy)
                            hidHandshakeSend(hid, hid_busy);
                    }
                    break;                
#endif /* HID_DEVICE */
                    case HID_DATC:
                    {
                        hidPacketStatus packet_status;

                        /* Append to end of packet */
                        packet_status = hidPacketAppend(hid, &ptr[1], packet_size - 1);
                        UNUSED(packet_status);
                    }
                    break;
                    default:
                    {
                        /* Unknown control packet, return unsupported */
                        hidHandshakeSend(hid, hid_unsupported);
                    }
                    break;
                }
            }
            else
            {
                /* Invalid packet size */
                hidHandshakeSend(hid, hid_invalid_param);
            }
        
              /* Discard the packet */
            SourceDrop(source, packet_size);
        }
    }
    /* Check source is for interrupt channel */
    else if (sink == hid->connection[HID_CON_INTERRUPT].con.sink)
    {
        HID_PRINT(("hidHandleControlPacket, sink=%x (interrupt)\n", (int)sink));

        /* Just leave packet in source */
    }
    else
        Panic();
}

/****************************************************************************
NAME    
    hidHandshakeSend
DESCRIPTION
    This function sends a handshake packet to the host.
RETURNS
    void
*/
static void hidHandshakeSend(HID *hid, hid_status result)
{
    HID_PRINT(("hidHandshakeSend, result=%d\n", result));
    hidPacketSend(hid, (HID_HANDSHAKE << 4) | result, 0, 0);
}

/****************************************************************************
NAME    
    hidHandleInternalControlRequest
DESCRIPTION
    This function handles the HID_INTERNAL_CONTROL_REQ message.
RETURNS
    void
*/
void hidHandleInternalControlRequest(HID *hid, const HID_INTERNAL_CONTROL_REQ_T *req)
{
#ifdef HID_DEVICE
    /* Device can only send VIRTUAL_UNPLUG */
    PanicFalse(req->op == hid_control_op_unplug);
#endif
    hidPacketSend(hid, (HID_CONTROL << 4) | req->op, 0, 0);
}

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleInternalGetIdleResponse
DESCRIPTION
    This function handles the HID_INTERNAL_GET_IDLE_RES message.
RETURNS
    void
*/
void hidHandleInternalGetIdleResponse(HID *hid, const HID_INTERNAL_GET_IDLE_RES_T *res)
{
    hid->packet_pending = 0;    
    if (res->status == hid_success)
        hidPacketSend(hid, (HID_DATA << 4), &res->idle_rate, 1);
    else
        hidHandshakeSend(hid, res->status);
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleInternalSetIdleResponse
DESCRIPTION
    This function handles the HID_INTERNAL_SET_IDLE_RES messages.
RETURNS
    void
*/
void hidHandleInternalSetIdleResponse(HID *hid, const HID_INTERNAL_SET_IDLE_RES_T *res)
{
    hid->packet_pending = 0;    
    hidHandshakeSend(hid, res->status);
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleInternalGetProtocolResponse
DESCRIPTION
    This function handles the HID_INTERNAL_GET_PROTOCOL_RES message.
RETURNS
    void
*/
void hidHandleInternalGetProtocolResponse(HID *hid, const HID_INTERNAL_GET_PROTOCOL_RES_T *res)
{
    hid->packet_pending = 0;    
    if (res->status == hid_success)
    {
        uint8 protocol = res->protocol;
        hidPacketSend(hid, (HID_DATA << 4), &protocol, 1);
    }
    else
        hidHandshakeSend(hid, res->status);
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleInternalSetProtocolResponse
DESCRIPTION
    This function handles the HID_INTERNAL_SET_PROTOCOL_RES message.
RETURNS
    void
*/
void hidHandleInternalSetProtocolResponse(HID *hid, const HID_INTERNAL_SET_PROTOCOL_RES_T *res)
{
    hid->packet_pending = 0;    
    hidHandshakeSend(hid, res->status);
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleInternalGetReportResponse
DESCRIPTION
    This function handles the HID_INTERNAL_GET_REPORT_RES message.
RETURNS
    void
*/
void hidHandleInternalGetReportResponse(HID *hid, const HID_INTERNAL_GET_REPORT_RES_T *res)
{
    hid->packet_pending = 0;    
    if (res->status == hid_success)
    {
        /* Send the report on the control channel */
        hidPacketSend(hid, (HID_DATA << 4) | res->report_type, res->report_data, res->report_length);
    }
    else
        hidHandshakeSend(hid, res->status);
}
#endif

#ifdef HID_DEVICE
/****************************************************************************
NAME    
    hidHandleInternalSetReportResponse
DESCRIPTION
    This function handles the HID_INTERNAL_SET_REPORT_RES message.
RETURNS
    void
*/
void hidHandleInternalSetReportResponse(HID *hid, const HID_INTERNAL_SET_REPORT_RES_T *res)
{    
    hid->packet_pending = 0;    
    hidHandshakeSend(hid, res->status);
}
#endif

/****************************************************************************
NAME    
    hidHandleInternalRequestTimeout
DESCRIPTION
    This function is called when a resposne to a HID request hasn't been
    received within the timeout period.  A confirm is sent to the application
    with an timeout error status set.
RETURNS
    void
*/
void hidHandleInternalRequestTimeout(HID *hid)
{
    HID_PRINT(("hidHandleInternalRequestTimeout\n"));

    /* Tell application/host that packet has timed out */
    hidPacketFailed(hid, hid->packet_pending, hid_timeout);
}

/****************************************************************************
NAME    
    hidInterruptReport
DESCRIPTION
    This function handles the sending a report on the interrupt channel.
RETURNS
    void
*/
void HidInterruptReport(HID *hid, hid_report_type report_type, uint16 report_length, const uint8 *data)
{
    /* Get interrupt sink */
    Sink sink = hid->connection[HID_CON_INTERRUPT].con.sink;
    
    /* Get pointer to stream */
    uint8 *ptr = SinkMap(sink);

    uint16 offset = SinkClaim(sink, report_length + 1);
    if (offset == 0xFFFF)
        return;

    /* Store header */
    ptr[offset] = (HID_DATA << 4) | (report_type & 0x03);

    /* Copy data into packet */
    memcpy(&ptr[offset + 1], data, report_length);

    /* Flush packet */
    SinkFlush(sink, report_length + 1);
}

void HidControl(HID *hid, hid_control_op op)
{
    MAKE_HID_MESSAGE(HID_INTERNAL_CONTROL_REQ);
    message->hid = hid;
    message->op = op;
    MessageSend(&hid->task, HID_INTERNAL_CONTROL_REQ, message);
}

#ifdef HID_DEVICE
void HidGetReportResponse(HID *hid, hid_status status, hid_report_type report_type, uint16 report_length, const uint8 *data)
{
    HID_INTERNAL_GET_REPORT_RES_T *message_res = NULL;

    if (data != NULL)
    {
        MAKE_HID_MESSAGE_WITH_LEN(HID_INTERNAL_GET_REPORT_RES, report_length);
        message_res = message;
        memcpy(message_res->report_data, data, report_length);
    }
    else
    {
        MAKE_HID_MESSAGE(HID_INTERNAL_GET_REPORT_RES);
        message_res = message;
    }
    HID_PRINT(("HidGetReportResponse\n"));
    message_res->hid = hid;
    message_res->status = status;
    message_res->report_type = report_type;
    message_res->report_length = report_length;
    MessageSend(&hid->task, HID_INTERNAL_GET_REPORT_RES, message_res);
}
#endif

#ifdef HID_DEVICE
void HidSetReportResponse(HID *hid, hid_status status)
{
    MAKE_HID_MESSAGE(HID_INTERNAL_SET_REPORT_RES);
    HID_PRINT(("HidSetReportResponse\n"));
    message->hid = hid;
    message->status = status;
    MessageSend(&hid->task, HID_INTERNAL_SET_REPORT_RES, message);
}
#endif

#ifdef HID_DEVICE
void HidGetIdleResponse(HID *hid, hid_status status, uint8 idle_rate)
{
    MAKE_HID_MESSAGE(HID_INTERNAL_GET_IDLE_RES);
    message->hid = hid;
    message->status = status;
    message->idle_rate = idle_rate;
    MessageSend(&hid->task, HID_INTERNAL_GET_IDLE_RES, message);
}
#endif

#ifdef HID_DEVICE
void HidSetIdleResponse(HID *hid, hid_status status)
{
    MAKE_HID_MESSAGE(HID_INTERNAL_SET_IDLE_RES);
    message->hid = hid;
    message->status = status;
    MessageSend(&hid->task, HID_INTERNAL_SET_IDLE_RES, message);
}
#endif

#ifdef HID_DEVICE
void HidGetProtocolResponse(HID *hid, hid_status status, hid_protocol protocol)
{
    MAKE_HID_MESSAGE(HID_INTERNAL_GET_PROTOCOL_RES);
    message->hid = hid;
    message->status = status;
    message->protocol = protocol;
    MessageSend(&hid->task, HID_INTERNAL_GET_PROTOCOL_RES, message);
}
#endif

#ifdef HID_DEVICE
void HidSetProtocolResponse(HID *hid, hid_status status)
{
    MAKE_HID_MESSAGE(HID_INTERNAL_SET_PROTOCOL_RES);
    message->hid = hid;
    message->status = status;
    MessageSend(&hid->task, HID_INTERNAL_SET_PROTOCOL_RES, message);
}
#endif

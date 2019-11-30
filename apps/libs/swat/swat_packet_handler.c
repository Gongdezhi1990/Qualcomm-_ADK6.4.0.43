/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_packet_handler.c

DESCRIPTION
    Contains functions related to sending / recieving SWAT packets
*/

/****************************************************************************
    Header files
*/

#include "swat.h"
#include "swat_private.h"
#include "swat_device_manager.h"
#include "swat_l2cap_handler.h"
#include "swat_command_handler.h"
#include "swat_packet_handler.h"
#include "swat_state_manager.h"

/* Firmware includes */
#include <sink.h>
#include <source.h>
#include <stream.h>
#include <string.h> /* memmove */

#include <print.h>

/*****************************************************************************/
bool swatSendData(remoteDevice *device, uint8 *data, uint16 len)
{
    uint8 * sink_ptr;      /* Ptr to the first byte in the sink */
    uint16 claim_result;   /* Result from the sink claim */
    
    /* Get the signalling data from the Sink */
    claim_result = SinkClaim(device->signalling_sink, len);
    if (claim_result == 0xffff)
    {
        SWAT_DEBUG(("[SWAT] swatSendData failed claim\n"));
        return FALSE;
    }
    else
    {
        sink_ptr = SinkMap(device->signalling_sink);
        
        if (sink_ptr)
        {   
            sink_ptr += claim_result;                           /* Point to the claimed data area of the sink */
            memmove(sink_ptr, data, len);                       /* Copy data to the sink */
            return(SinkFlush(device->signalling_sink, len));    /* Send the data */
        }
        else
        {
            SWAT_DEBUG(("[SWAT] swatSendData failed ptr\n"));
            return FALSE;
        }
    }
}


/*****************************************************************************/
void swatHandleSwatSignallingData(Source source)
{
    uint16 packet_size, data_size;
    const uint8 *ptr = SourceMap(source);
    
    /* Get the device that sent the signalling data */
    remoteDevice* device = swatFindDeviceFromSink( StreamSinkFromSource(source) );
    
    packet_size = SourceSize(source);
    
    /* Process each signalling packet in the source (may be more than one signal in the source) */
    while(packet_size >= PKT_PAYLOAD)
    {
        /* calculate size of packet */              
        data_size = ((ptr[PKT_PAYLOAD_LEN] & 0x7f) + 3);
        
        /* Process the signalling data by the remote device */
        switch(ptr[PKT_SIGNAL_ID])
        {
            /*****************************************************************************/
            case SWAT_OPEN:
            {
                /* send to command handler, include message payload  */
                MAKE_SWAT_MESSAGE_WITH_LEN(SWAT_COMMAND_OPEN, data_size);
                message->device = device;                    
                memmove(&message->data[0], &ptr[0], data_size);
                
                /* If message is a response, send immediatley, otherwise add to queue */
                if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
                {
                    MessageSend(&swat->command_task, SWAT_COMMAND_OPEN, message);
                }
                else
                {
                    SWAT_DEBUG(("[SWAT] SWAT_OPEN_CMD [%x]\n", device->signalling_block));
                    
                    /* Action the message or add to queue if already handling another message */
                    MessageSendConditionally(&swat->command_task, SWAT_COMMAND_OPEN, message, &device->signalling_block);
                }
                
                /* command actioned, discard buffer data leaving any additional data behind */
                SourceDrop(source, data_size);
                /* having removed a packet reset the pointer to the start of any more packets */
                ptr = SourceMap(source);
            }
            break;
            /*****************************************************************************/
            case SWAT_CLOSE:
            {
                /* send to command handler, include message payload  */
                MAKE_SWAT_MESSAGE_WITH_LEN(SWAT_COMMAND_CLOSE, data_size);
                message->device = device;                    
                memmove(&message->data[0], &ptr[0], data_size);
                
                /* If message is a response, send immediatley, otherwise add to queue */
                if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
                {
                    MessageSend(&swat->command_task, SWAT_COMMAND_CLOSE, message);
                }
                else
                {
                    SWAT_DEBUG(("[SWAT] SWAT_CLOSE_CMD [%x]\n", device->signalling_block));
                    
                    /* Action the message or add to queue if already handling another message */
                    MessageSendConditionally(&swat->command_task, SWAT_COMMAND_CLOSE, message, &device->signalling_block);
                }
                
                /* command actioned, discard buffer data leaving any additional data behind */
                SourceDrop(source, data_size);
                /* having removed a packet reset the pointer to the start of any more packets */
                ptr = SourceMap(source);
            }
            break;
            /*****************************************************************************/
            case SWAT_START:
            {
                /* send to command handler, include message payload  */
                MAKE_SWAT_MESSAGE_WITH_LEN(SWAT_COMMAND_START, data_size);
                message->device = device;                    
                memmove(&message->data[0], &ptr[0], data_size);
                
                /* If message is a response, send immediatley, otherwise add to queue */
                if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
                {
                    MessageSend(&swat->command_task, SWAT_COMMAND_START, message);
                }
                else
                {
                    SWAT_DEBUG(("[SWAT] SWAT_START_CMD [%x]\n", device->signalling_block));
                    
                    /* Action the message or add to queue if already handling another message */
                    MessageSendConditionally(&swat->command_task, SWAT_COMMAND_START, message, &device->signalling_block);
                }
                
                /* command actioned, discard buffer data leaving any additional data behind */
                SourceDrop(source, data_size);
                /* having removed a packet reset the pointer to the start of any more packets */
                ptr = SourceMap(source);
            }
            break;
            /*****************************************************************************/
            case SWAT_SUSPEND:
            {
                /* send to command handler, include message payload  */
                MAKE_SWAT_MESSAGE_WITH_LEN(SWAT_COMMAND_SUSPEND, data_size);
                message->device = device;                    
                memmove(&message->data[0], &ptr[0], data_size);
                
                /* If message is a response, send immediatley, otherwise add to queue */
                if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
                {
                    MessageSend(&swat->command_task, SWAT_COMMAND_SUSPEND, message);
                }
                else
                {
                    SWAT_DEBUG(("[SWAT] SWAT_SUSPEND_CMD [%x]\n", device->signalling_block));
                    
                    /* Action the message or add to queue if already handling another message */
                    MessageSendConditionally(&swat->command_task, SWAT_COMMAND_SUSPEND, message, &device->signalling_block);    
                }
                
                /* command actioned, discard buffer data leaving any additional data behind */
                SourceDrop(source, data_size);
                /* having removed a packet reset the pointer to the start of any more packets */
                ptr = SourceMap(source);
            }
            break;
            /*****************************************************************************/
            case SWAT_SET_VOLUME:
            {
                /* send to command handler, include message payload  */
                MAKE_SWAT_MESSAGE_WITH_LEN(SWAT_COMMAND_SET_VOLUME, data_size);
                message->device = device;                    
                memmove(&message->data[0], &ptr[0], data_size);
                
                /* If message is a response, send immediatley, otherwise add to queue */
                if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
                {
                    MessageSend(&swat->command_task, SWAT_COMMAND_SET_VOLUME, message);
                }
                else
                {
                    SWAT_DEBUG(("[SWAT] SWAT_SET_VOLUME_CMD [%x]\n", device->signalling_block));
                    
                    /* Action the message or add to queue if already handling another message */
                    MessageSendConditionally(&swat->command_task, SWAT_COMMAND_SET_VOLUME, message, &device->signalling_block);
                }
                
                /* command actioned, discard buffer data leaving any additional data behind */
                SourceDrop(source, data_size);
                /* having removed a packet reset the pointer to the start of any more packets */
                ptr = SourceMap(source);
            }
            break;
            /*****************************************************************************/
            case SWAT_SAMPLE_RATE:
            {
                /* send to command handler, include message payload  */
                MAKE_SWAT_MESSAGE_WITH_LEN(SWAT_COMMAND_SAMPLE_RATE, data_size);
                message->device = device;
                memmove(&message->data[0], &ptr[0], data_size);
                
                /* If message is a response, send immediatley, otherwise add to queue */
                if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
                {
                    MessageSend(&swat->command_task, SWAT_COMMAND_SAMPLE_RATE, message);
                }
                else
                {
                    SWAT_DEBUG(("[SWAT] SWAT_SAMPLE_RATE_CMD [%x]\n", device->signalling_block));
                    
                    /* Action the message or add to queue if already handling another message */
                    MessageSendConditionally(&swat->command_task, SWAT_COMMAND_SAMPLE_RATE, message, &device->signalling_block);
                }
                
                /* Command actioned, discard buffer data leaving any additional data behind */
                SourceDrop(source, data_size);
                /* Having removed a packet reset the pointer to the start of any more packets */
                ptr = SourceMap(source);
            }
            break;
            /*****************************************************************************/
            case SWAT_GET_VERSION:
            {
                /* send to command handler, include message payload  */
                MAKE_SWAT_MESSAGE_WITH_LEN(SWAT_COMMAND_GET_VERSION, data_size);
                message->device = device;
                memmove(&message->data[0], &ptr[0], data_size);
                
                /* If message is a response, send immediatley, otherwise add to queue */
                if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
                {
                    MessageSend(&swat->command_task, SWAT_COMMAND_GET_VERSION, message);
                }
                else
                {
                    SWAT_DEBUG(("[SWAT] SWAT_COMMAND_GET_VERSION [%x]\n", device->signalling_block));
                    
                    /* Action the message or add to queue if already handling another message */
                    MessageSendConditionally(&swat->command_task, SWAT_COMMAND_GET_VERSION, message, &device->signalling_block);
                }
                
                /* Command actioned, discard buffer data leaving any additional data behind */
                SourceDrop(source, data_size);
                /* Having removed a packet reset the pointer to the start of any more packets */
                ptr = SourceMap(source);
            }
            break;
            /*****************************************************************************/
            case SWAT_GENERAL_REJECT:
            {
                /* send to command handler, include message payload  */
                MAKE_SWAT_MESSAGE_WITH_LEN(SWAT_COMMAND_GENERAL_REJECT, data_size);
                message->device = device;                    
                memmove(&message->data[0], &ptr[0], data_size);
                
                /* Handle the GENERAL_REJECT command as it won't affect ongoing operations */
                MessageSend(&swat->command_task, SWAT_COMMAND_GENERAL_REJECT, message);
                
                /* command actioned, discard buffer data leaving any additional data behind */
                SourceDrop(source, data_size);
                /* having removed a packet reset the pointer to the start of any more packets */
                ptr = SourceMap(source);
            }
            break;
            /*****************************************************************************/
            default:
            {
                /* send to command handler, include message payload  */
                MAKE_SWAT_MESSAGE(SWAT_COMMAND_DEFAULT);
                message->device = device;                    
                
                MessageSend(&swat->command_task, SWAT_COMMAND_DEFAULT, message);
                
                /* block any more signalling commands until current process has completed */
                device->signalling_block = DATA_BLOCK;
                
                /* Discard entire buffer as got out of sync */
                SourceDrop(source, packet_size);
            }
        }
        /* recalculate the packet size of any data left in the source */
        packet_size = SourceSize(source);    
    }
}

/****************************************************************************
Copyright (c) 2017  Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_host_trb.c

DESCRIPTION
    Implementation of an AHI transport using the Transaction Bridge.
*/
/*!
@file   ahi_host_trb.c
@brief  Implementation of an AHI transport using the Transaction Bridge.

        This is a VM application module that acts as the 'glue' between
        the host interface traps and the AHI transport API.
*/

#include "sink_debug.h"
#include <message.h>
#include <panic.h>
#include <print.h>
#include <sink.h>
#include <source.h>
#include <stdlib.h>
#include <stream.h>
#include <string.h>
#include <host.h>

#include "ahi.h"
#include "ahi_host_trb.h"

#ifdef DEBUG_AHI
    #define AHI_DEBUG(x) DEBUG(x)
#else
    #define AHI_DEBUG(x)
#endif
#ifdef ENABLE_AHI_TRB

/* AHI transport task for data going to Host */
static void ahiTransportHandleMessage(Task task, MessageId id, Message message);
static const TaskData ahiTask = { ahiTransportHandleMessage };

/* Host Stream task for data arriving from Host. */
static void hostCommsHandleMessage(Task task, MessageId id, Message message);
static const TaskData hcTask = { hostCommsHandleMessage };

static void handleTransportSendData(Task task, Message message);
static void AhiTrbHostHandleMessage(MessageId id, Message message);

#define AHI_TRB_RETRY_DELAY_MS          5

#define HOST_COMMS_HEADER_SIZE_WORDS    2
#define HOST_COMMS_SIZE_WORD            0
#define HOST_COMMS_CHANNEL_WORD         1
#define HOST_COMMS_PAYLOAD_WORD         2


/******************************************************************************
    Private functions 
*/

/******************************************************************************
NAME
    handleTransportSendData

DESCRIPTION
    Send data contained in a AHI_TRANSPORT_SEND_DATA message to the host.

RETURNS
    n/a
*/
static void handleTransportSendData(Task task, Message message)
{
    AHI_TRANSPORT_SEND_DATA_T *msg = (AHI_TRANSPORT_SEND_DATA_T *)message;
    uint8 *byte_data;
    uint16 *word_data;
    uint16 host_msg_size_in_words = HOST_COMMS_HEADER_SIZE_WORDS;
    host_msg_size_in_words += msg->size/sizeof(uint16) + msg->size%sizeof(uint16);

#ifdef HYDRACORE
    UNUSED(task);
#endif

    AHI_DEBUG(("AHI: handleTransportSendData payload 0x%p size %u\n", msg->payload, msg->size));

    /* Transfer the AHI message payload to a new Host Comms message and pass to HostSendMessage. */
    byte_data = PanicUnlessMalloc(host_msg_size_in_words*sizeof(uint16));
    word_data = (uint16 *)byte_data;
    word_data[HOST_COMMS_SIZE_WORD] = host_msg_size_in_words;
    word_data[HOST_COMMS_CHANNEL_WORD] = 0;
    memmove(&byte_data[HOST_COMMS_PAYLOAD_WORD*sizeof(uint16)], msg->payload, msg->size);

    if (HostSendMessage(word_data))
    {
        AhiTransportSendDataAck();
    }
    else
    {
        AHI_TRANSPORT_SEND_DATA_T *repost;

        AHI_DEBUG(("AHI:   Failed to send %u bytes - retrying", msg->size));

        /* Retry by sending the same message back to ourselves. */
        repost = PanicUnlessMalloc(sizeof(AHI_TRANSPORT_SEND_DATA_T) + msg->size);
        repost->size = msg->size;
        memmove(repost->payload, msg->payload, msg->size);
        MessageSendLater(task, AHI_TRANSPORT_SEND_DATA, repost, AHI_TRB_RETRY_DELAY_MS);

        free(byte_data);
    }
}

/******************************************************************************
NAME
    ahiTransportHandleMessage

DESCRIPTION
    Message handler for messages from the AHI library.

    Usually the message will be a request to send data to the Host.

RETURNS
    n/a
*/
static void ahiTransportHandleMessage(Task task, MessageId id, Message message)
{
    AHI_DEBUG(("AHI: ahiTransportHandleMessage id 0x%x\n", id));

    switch (id)
    {
    case AHI_TRANSPORT_SEND_DATA:
        handleTransportSendData(task, message);
        break;
    default:
        AHI_DEBUG(("AHI: Unexpected ahi msg id 0x%x\n", id));
        break;
    }
}

/******************************************************************************
NAME
    hostCommsHandleMessage

DESCRIPTION
    Message handler for messages from the Host via Test Tunnel.

RETURNS
    n/a
*/
static void hostCommsHandleMessage(Task task, MessageId id, Message message)
{
    AHI_DEBUG(("AHI: hostCommsHandleMessage id 0x%x\n", id));

#ifdef HYDRACORE
    UNUSED(task);
#endif

    switch (id)
    {
    case MESSAGE_FROM_HOST:
        AhiTrbHostHandleMessage(id, message);
        break;
    default:
        AHI_DEBUG(("AHI: Unexpected Host Comms msg id 0x%x\n", id));
        break;
    }
}

/******************************************************************************
NAME
    AhiTrbHostHandleMessage
DESCRIPTION
    Message handler for messages from the Host.
RETURNS
    n/a
*/
void AhiTrbHostHandleMessage(MessageId id, Message message)
{
    UNUSED(id);
    uint8* msg = (uint8 *)message;
    uint8* payload = &msg[HOST_COMMS_PAYLOAD_WORD*sizeof(uint16)];
    AHI_DEBUG(("AHI: AhiTrbHostHandleMessage id 0x%x\n", id));
    AhiTransportProcessData((TaskData*)&ahiTask, payload);
}

/******************************************************************************
    Public functions
*/
void AhiTrbHostInit(void)
{
    MessageHostCommsTask((TaskData*)&hcTask);
}

#endif /* ENABLE_AHI_TRB */


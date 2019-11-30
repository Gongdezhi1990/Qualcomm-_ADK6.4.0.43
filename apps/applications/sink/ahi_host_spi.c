/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_host_spi.c

DESCRIPTION
    Implementation of an AHI transport using the BCCMDs that work over SPI.

*/
/*!
@file   ahi_host_spi.c
@brief  Implementation of an AHI transport using the BCCMDs that work over SPI.

        This is a VM application module that acts as the 'glue' between
        The new traps for BCCMDs that carry data to the VM and the AHI transport API.
*/

#include <host.h>
#include <message.h>
#include <panic.h>
#include <print.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ahi.h"
#include "byte_utils.h"

#include "ahi_host_spi.h"
#include "sink_debug.h"


#ifdef DEBUG_AHI
    #define AHI_DEBUG(x) DEBUG(x)
#else
    #define AHI_DEBUG(x)
#endif


#ifdef ENABLE_AHI_SPI

/* PLACEHOLDERS IF COMPILING ON FIRMWARE WITHOUT THE NEW TRAPS */
#ifndef MESSAGE_HOST_BCCMD

/*!
  #MESSAGE_HOST_BCCMD: Message block from Host received as BCCMD packet.
 */
typedef struct
{
    uint16 len;                         /*!< The length of the message block */
    uint16 message[1];                  /*!< Message block (payload) */
} MessageHostBccmd;

#define MESSAGE_HOST_BCCMD              (SYSTEM_MESSAGE_BASE_ + 62)

static Task MessageHostBccmdTask(Task task)
{
    return task;
}

static bool HostSendBccmdMessage(const uint16 *msg, uint16 length)
{
    return TRUE;
}

#endif
/* END OF PLACEHOLDERS */


/* Host Bccmd task for data arriving from Host. */
static void hostBccmdHandleMessage(Task task, MessageId id, Message message);
static const TaskData hbTask = { hostBccmdHandleMessage };

/* AHI transport task for data going to Host */
static void ahiTransportHandleMessage(Task task, MessageId id, Message message);
static const TaskData ahiTask = { ahiTransportHandleMessage };

static void handleTransportSendData(Task task, Message message);

/* Message delay (ms) when retrying a data send. */
#define AHI_SPI_RETRY_DELAY 5


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
    uint16 size_words;
    AHI_TRANSPORT_SEND_DATA_T *msg = (AHI_TRANSPORT_SEND_DATA_T *)message;

#ifdef HYDRACORE    
    UNUSED(task);
#endif    

    AHI_DEBUG(("AHI: handleTransportSendData payload 0x%p size %u\n", msg->payload, msg->size));

    size_words = ((msg->size + 1) >> 1);
    if(!HostSendBccmdMessage((uint16 *)msg->payload, size_words))
    {
        AHI_TRANSPORT_SEND_DATA_T *repost;

        AHI_DEBUG(("AHI:   Failed to send %u bytes - retrying\n", msg->size));

        /* Retry by sending the same message back to ourselves. */
        repost = PanicUnlessMalloc(sizeof(AHI_TRANSPORT_SEND_DATA_T) + msg->size);
        repost->size = msg->size;
        memcpy(repost->payload, msg->payload, msg->size);
        MessageSendLater(task, AHI_TRANSPORT_SEND_DATA, repost, AHI_SPI_RETRY_DELAY);
        return;
    }

    AhiTransportSendDataAck();
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
    handleMessageHostBccmd

DESCRIPTION
    Send data (from the Host) in a MESSAGE_HOST_BCCMD message to AHI.

RETURNS
    n/a
*/
static void handleMessageHostBccmd(Task task, Message message)
{
    uint8 *data;
    uint16 bytes;

    MessageHostBccmd *msg = (MessageHostBccmd *)message;

    AHI_DEBUG(("AHI: handleMessageHostBccmd len %u\n", msg->len));

    /* The data in msg->message is in packed format (two bytes per word),
       but AHI expects it unpacked (one byte in the LSB per word).
       So we need a temporary buffer to hold the unpacked data. */
    bytes = (msg->len << 1);
    data = PanicUnlessMalloc(bytes);
    ByteUtilsMemCpyToStream(data, (uint8 *)msg->message, bytes);
    
#ifdef DEBUG_AHI
    AHI_DEBUG(("AHI: bytes %u data [ ", bytes));
    {
        uint16 i;
        for (i = 0; i < bytes; i++)
        {
            AHI_DEBUG(("0x%x ", data[i]));
        }
    }
    AHI_DEBUG((" ]\n"));
#endif

    AhiTransportProcessData((TaskData *)&ahiTask, data);

    free(data);
}

/******************************************************************************
NAME
    hostBccmdHandleMessage

DESCRIPTION
    Message handler for messages from the Host over Bccmd.

RETURNS
    n/a
*/
static void hostBccmdHandleMessage(Task task, MessageId id, Message message)
{
    AHI_DEBUG(("AHI: hostBccmdHandleMessage id 0x%x\n", id));

    switch (id)
    {
    case MESSAGE_HOST_BCCMD:
        handleMessageHostBccmd(task, message);
        break;
    default:
        AHI_DEBUG(("AHI: Unexpected Host Bccmd msg id 0x%x\n", id));
        break;
    }
}

/******************************************************************************
    Public functions 
*/

/******************************************************************************
NAME
    AhiSpiHostInit

DESCRIPTION
    Initialise the SPI Host AHI transport.

RETURNS
    n/a
*/
void AhiSpiHostInit(void)
{
    AHI_DEBUG(("AHI: AhiSpiHostInit\n"));    

    MessageHostBccmdTask((TaskData*)&hbTask);
}

#endif /* ENABLE_AHI_SPI */

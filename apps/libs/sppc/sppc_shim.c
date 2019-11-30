/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sppc_shim.c
    
DESCRIPTION
	SPP Server Shim Layer
	
*/

#include <panic.h>
#include <message.h>
#include <stdlib.h>
#include <string.h> /* for memset */
#include <stream.h>
#include <source.h>
#include <sink.h>
#include <vm.h>
#include "sppc_shim.h"

void SppcHandleComplexMessage(Task task, MessageId id, Message message)
{
	switch (id)
	{
		case SPP_MESSAGE_MORE_DATA:
		{
			/* Move data from source into message */
			Source src = ((SPP_MESSAGE_MORE_DATA_T*)message)->source;
			SPPC_MESSAGE_MORE_DATA_TEST_EXTRA_T *pdu;
			const uint8 *s = SourceMap(src);
			uint16 len = SourceBoundary(src);
				
			pdu = malloc(sizeof(SPPC_MESSAGE_MORE_DATA_TEST_EXTRA_T)+len);
			memset(pdu, 0, sizeof(SPPC_MESSAGE_MORE_DATA_TEST_EXTRA_T));
				
			pdu->sink = StreamSinkFromSource(src);
			pdu->size_data = len;
				
			memmove(pdu->data, s, len);
				
			SourceDrop(src, len);
			
			MessageSend(task, SPPC_MESSAGE_MORE_DATA_TEST_EXTRA, pdu);
		}
        break;

        case SPP_MESSAGE_MORE_SPACE:
            break;

        case SPP_CLIENT_CONNECT_CFM:
            {
                SPPC_CLIENT_CONNECT_CFM_TEST_EXTRA_T *msg_out = PanicUnlessNew(SPPC_CLIENT_CONNECT_CFM_TEST_EXTRA_T);
                *msg_out = *((SPPC_CLIENT_CONNECT_CFM_TEST_EXTRA_T *)message);
                MessageSend(task, SPPC_CLIENT_CONNECT_CFM_TEST_EXTRA, msg_out);
            }
            break;

        case SPP_DISCONNECT_IND:
            {
                SPPC_DISCONNECT_IND_TEST_EXTRA_T *msg_out = PanicUnlessNew(SPPC_DISCONNECT_IND_TEST_EXTRA_T);
                *msg_out = *((SPPC_DISCONNECT_IND_TEST_EXTRA_T *)message);
                MessageSend(task, SPPC_DISCONNECT_IND_TEST_EXTRA, msg_out);
            }
            break;

        case SPP_DISCONNECT_CFM:
            {
                SPPC_DISCONNECT_CFM_TEST_EXTRA_T *msg_out = PanicUnlessNew(SPPC_DISCONNECT_CFM_TEST_EXTRA_T);
                *msg_out = *((SPPC_DISCONNECT_CFM_TEST_EXTRA_T *)message);
                MessageSend(task, SPPC_DISCONNECT_CFM_TEST_EXTRA, msg_out);
            }
            break;


		default:
		{
			Panic();
		}
        break;
	}
}

void SppcConnectRequestTestExtra(Task theAppTask, const bdaddr *bd_addr, const uint16 security_channel, uint16 max_payload_size)
{
    SppConnectRequest(theAppTask, bd_addr, security_channel, max_payload_size);
}

void SppcDisconnectResponseTestExtra(SPPC *sppc)
{
    SppDisconnectResponse((SPP *)sppc);
}

void SppcDisconnectRequestTestExtra(SPPC *sppc)
{
    SppDisconnectRequest((SPP *)sppc);
}


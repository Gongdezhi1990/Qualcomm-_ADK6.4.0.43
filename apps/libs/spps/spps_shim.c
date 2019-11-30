/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    spps_shim.c
    
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
#include "spps_shim.h"

void SppsHandleComplexMessage(Task task, MessageId id, Message message)
{
	switch (id)
	{
		case SPP_MESSAGE_MORE_DATA:
		{
			/* Move data from source into message */
			Source src = ((SPP_MESSAGE_MORE_DATA_T*)message)->source;
			SPPS_MESSAGE_MORE_DATA_TEST_EXTRA_T *pdu;
			const uint8 *s = SourceMap(src);
			uint16 len = SourceBoundary(src);
				
			pdu = malloc(sizeof(SPPS_MESSAGE_MORE_DATA_TEST_EXTRA_T)+len);
			memset(pdu, 0, sizeof(SPPS_MESSAGE_MORE_DATA_TEST_EXTRA_T));
				
			pdu->sink = StreamSinkFromSource(src);
			pdu->size_data = len;
				
			memmove(pdu->data, s, len);
				
			SourceDrop(src, len);
			
			MessageSend(task, SPPS_MESSAGE_MORE_DATA_TEST_EXTRA, pdu);
		}
        break;
        
        case SPP_MESSAGE_MORE_SPACE:
            break;

        case SPP_START_SERVICE_CFM:
            {
                SPPS_START_SERVICE_CFM_TEST_EXTRA_T *msg_out = PanicUnlessNew(SPPS_START_SERVICE_CFM_TEST_EXTRA_T);
                *msg_out = *((SPPS_START_SERVICE_CFM_TEST_EXTRA_T *)message);
                MessageSend(task, SPPS_START_SERVICE_CFM_TEST_EXTRA, msg_out);
            }
            break;

        case SPP_STOP_SERVICE_CFM:
            {
                SPPS_STOP_SERVICE_CFM_TEST_EXTRA_T *msg_out = PanicUnlessNew(SPPS_STOP_SERVICE_CFM_TEST_EXTRA_T);
                *msg_out = *((SPPS_STOP_SERVICE_CFM_TEST_EXTRA_T *)message);
                MessageSend(task, SPPS_STOP_SERVICE_CFM_TEST_EXTRA, msg_out);
            }
            break;

        case SPP_CONNECT_IND:
            {
                SPPS_CONNECT_IND_TEST_EXTRA_T *msg_out = PanicUnlessNew(SPPS_CONNECT_IND_TEST_EXTRA_T);
                *msg_out = *((SPPS_CONNECT_IND_TEST_EXTRA_T *)message);
                MessageSend(task, SPPS_CONNECT_IND_TEST_EXTRA, msg_out);
            }
            break;

        case SPP_SERVER_CONNECT_CFM:
            {
                SPPS_SERVER_CONNECT_CFM_TEST_EXTRA_T *msg_out = PanicUnlessNew(SPPS_SERVER_CONNECT_CFM_TEST_EXTRA_T);
                *msg_out = *((SPPS_SERVER_CONNECT_CFM_TEST_EXTRA_T *)message);
                MessageSend(task, SPPS_SERVER_CONNECT_CFM_TEST_EXTRA, msg_out);
            }
            break;

        case SPP_DISCONNECT_IND:
            {
                SPPS_DISCONNECT_IND_TEST_EXTRA_T *msg_out = PanicUnlessNew(SPPS_DISCONNECT_IND_TEST_EXTRA_T);
                *msg_out = *((SPPS_DISCONNECT_IND_TEST_EXTRA_T *)message);
                MessageSend(task, SPPS_DISCONNECT_IND_TEST_EXTRA, msg_out);
            }
            break;

        case SPP_DISCONNECT_CFM:
            {
                SPPS_DISCONNECT_CFM_TEST_EXTRA_T *msg_out = PanicUnlessNew(SPPS_DISCONNECT_CFM_TEST_EXTRA_T);
                *msg_out = *((SPPS_DISCONNECT_CFM_TEST_EXTRA_T *)message);
                MessageSend(task, SPPS_DISCONNECT_CFM_TEST_EXTRA, msg_out);
            }
            break;
        
		default:
		{
			Panic();
		}
        break;
	}
}

void SppsStartServiceTestExtra(Task theAppTask)
{
    SppStartService(theAppTask);
}

void SppsStopServiceTestExtra(Task theAppTask)
{
    SppStopService(theAppTask);
}

void SppsConnectResponseTestExtra(Task theAppTask, const bdaddr *bd_addr, const bool response, const Sink sink, const uint8 local_server_channel, const uint16 max_payload_size)
{
    SppConnectResponse(theAppTask,bd_addr,response,sink,local_server_channel,max_payload_size);
}


void SppsDisconnectResponseTestExtra(SPPS *spps)
{
    SppDisconnectResponse((SPP *)spps);
}

void SppsDisconnectRequestTestExtra(SPPS *spps)
{
    SppDisconnectRequest((SPP *)spps);
}


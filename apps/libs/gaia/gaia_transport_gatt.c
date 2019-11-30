/*****************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.
*/
#include "gaia.h"
#include "gaia_private.h"
#include "gaia_transport_gatt.h"
#include "gaia_transport.h"
#include "gaia_transport_common.h"
#include <source.h>
#include <stdio.h>
#include <sink.h>
#include <vm.h>

#ifdef GAIA_TRANSPORT_GATT

#include "gaia_db.h"

static void process_command(gaia_transport *transport, uint16 size_command, uint8 *command)
{
/*  Short packets are by definition badly framed and hence silently ignored  */
    if (size_command >= GAIA_GATT_OFFS_PAYLOAD)
    {
        uint8 *payload = NULL;
        uint16 size_payload = size_command - GAIA_GATT_OFFS_PAYLOAD;
        uint16 vendor_id = (command[GAIA_GATT_OFFS_VENDOR_ID_H] << 8) | command[GAIA_GATT_OFFS_VENDOR_ID_L];
        uint16 command_id = (command[GAIA_GATT_OFFS_COMMAND_ID_H] << 8) | command[GAIA_GATT_OFFS_COMMAND_ID_L];
#ifdef DEBUG_GAIA_TRANSPORT
        uint16 i;
    
        GAIA_TRANS_DEBUG(("gaia: command:"));
        for (i = 0; i < size_command; ++i)
        {
            GAIA_TRANS_DEBUG((" %02X",  command[i]));
        }
        GAIA_TRANS_DEBUG(("\n"));
#endif
        if(size_payload)
        {
      	     payload = command + GAIA_GATT_OFFS_PAYLOAD;
        }
        transport->state.gatt.size_response = 5;
        memcpy(transport->state.gatt.response, command, 4);
        transport->state.gatt.response[GAIA_GATT_OFFS_COMMAND_ID_H] |= GAIA_ACK_MASK_H;
        transport->state.gatt.response[GAIA_GATT_OFFS_PAYLOAD] = GAIA_STATUS_IN_PROGRESS;
        
        gaiaProcessCommand(transport, vendor_id, command_id, size_payload, payload);
    }
}

static bool process_data(gaia_transport *transport, uint16 size_data, uint8 *data)
{
#ifdef DEBUG_GAIA_TRANSPORT
    uint16 i;
    
    GAIA_TRANS_DEBUG(("gaia: size_data:%d\n",size_data));
    GAIA_TRANS_DEBUG(("gaia: data:"));
    for (i = 0; i < size_data; ++i)
    {
        GAIA_TRANS_DEBUG((" %02X",  data[i]));
    }
    GAIA_TRANS_DEBUG(("\n"));
#else   /* DEBUG_GAIA_TRANSPORT*/
    UNUSED(size_data);
    UNUSED(data);
#endif  /* DEBUG_GAIA_TRANSPORT */

   if(gaia->data_endpoint_mode == GAIA_DATA_ENDPOINT_MODE_RWCP)
   {
       return (gaiaProcessData(transport, size_data, data));
   }
   else
   {
       UNUSED(transport);
   }
   
   return TRUE;
}

/*! @brief
 */
void GaiaConnectGatt(uint16 cid)
{
    gaia_transport *transport = gaiaTransportFindFree();
    bool ok = FALSE;
    
    if (transport)
    {
        transport->type = gaia_transport_gatt;
        transport->state.gatt.cid = cid;
        
        transport->state.gatt.config_not = TRUE; /* we're really supposed to persist these */
        transport->state.gatt.config_ind = FALSE;
    
        ok = TRUE;
    }
    
    gaiaTransportCommonSendGaiaConnectInd(transport, ok);
}


/*! @brief
 */
void GaiaDisconnectGatt(uint16 cid)
{
    gaia_transport *transport = gaiaTransportFromCid(cid);

    GAIA_TRANS_DEBUG(("gaia: disconnect cid=0x%04X tra=0x%p\n",
        cid, (void *) transport));

    if(transport)   
    {
        gaiaTransportCommonSendGaiaDisconnectInd(transport);       
    }
}


/*************************************************************************
NAME
    gaiaTransportGattRegisterServer
    
DESCRIPTION
    Register the GAIA GATT server with the GATT Manager
*/
void gaiaTransportGattRegisterServer(uint16 start_handle, uint16 end_handle)
{
    gatt_manager_status_t status;
    gatt_manager_server_registration_params_t registration_params;
    
    registration_params.task = &gaia->task_data;
    registration_params.start_handle = start_handle;
    registration_params.end_handle = end_handle;
            
    status = GattManagerRegisterServer(&registration_params);
    GAIA_TRANS_DEBUG(("GM reg %u\n", status));
    
    gaiaTransportCommonSendGaiaStartServiceCfm(gaia_transport_gatt, NULL, status == gatt_manager_status_success);
}


/*************************************************************************
NAME
    gaiaTransportGattRes
    
DESCRIPTION
    Copy a response to the transport buffer and notify the central
*/
void gaiaTransportGattRes(gaia_transport *transport, uint16 size_response, uint8 *response, uint8 handle)
{
    if(transport != NULL)
    {
        if ((transport->state.gatt.data_config_not) && (handle == HANDLE_GAIA_DATA_ENDPOINT))
        {
            GattManagerRemoteClientNotify(&gaia->task_data, transport->state.gatt.cid, handle, size_response, response);
            return;
        }

        if (size_response >= GAIA_GATT_OFFS_PAYLOAD)
        {
            if (transport->state.gatt.config_not)
            {
                GAIA_TRANS_DEBUG(("gaia: not %02X%02X %02X%02X\n", response[0], response[1], response[2], response[3]));
                GattManagerRemoteClientNotify(&gaia->task_data, transport->state.gatt.cid, handle, size_response, response);
            }

            if (transport->state.gatt.config_ind)
            {
                GAIA_TRANS_DEBUG(("gaia: ind %02X%02X %02X%02X\n", response[0], response[1], response[2], response[3]));
                GattManagerRemoteClientIndicate(&gaia->task_data, transport->state.gatt.cid, handle, size_response, response);
            }
        
            if (response[GAIA_GATT_OFFS_COMMAND_ID_H] & GAIA_ACK_MASK_H)
            {
                /*  Response is an acknowledgement; cache it  */
                if (size_response > GATT_BUFFER_SIZE)
                {
                    /*  No space to store the response; store vendor + command + status only  */
                    size_response = GAIA_GATT_OFFS_PAYLOAD + 1;
                }
            
                transport->state.gatt.size_response = size_response;
                memcpy(transport->state.gatt.response, response, size_response);
            }
        }        
    }
}


/*************************************************************************
NAME
    populate_response_payload
    
DESCRIPTION
    Build the payload buffer for the response to be sent.
    If unpack is true then the payload is treated as uint16[]
 */
static void populate_response_payload(void *payload, uint8 size_payload,
                                     uint8 *response_payload, bool unpack)
{
    if (unpack)
    {
        uint16 *data = (uint16 *) payload;
        
        while (size_payload--)
        {
            *response_payload++ = HIGH(*data);
            *response_payload++ = LOW(*data++);
        }
    }
    else
    {
        uint8 *data = (uint8 *) payload;
        
        while (size_payload--)
        {
            *response_payload++ = *data++;
        }
    }
}


/*************************************************************************
NAME
    populate_response_header
    
DESCRIPTION
    Build the header for the response to be sent.
 */
static void populate_response_header(uint16 vendor_id, uint16 command_id, uint8 *response_header)
{
    *response_header++ = HIGH(vendor_id);
    *response_header++ = LOW(vendor_id);
    *response_header++ = HIGH(command_id);
    *response_header++ = LOW(command_id);
}


/*************************************************************************
NAME
    calculate_response_size
    
DESCRIPTION
    Returns payload response size based on the unpack parameter.
 */
static uint16 calculate_response_size(uint8 size_payload, bool unpack)
{
    uint16 size;

    size = (unpack? (GAIA_GATT_OFFS_PAYLOAD + 2 * size_payload) :
                    (GAIA_GATT_OFFS_PAYLOAD + size_payload));

    return size;
}


/*************************************************************************
NAME
    gaiaTransportGattSend
    
DESCRIPTION
    Build and send a short format GAIA packet
    If unpack is true then the payload is treated as uint16[]
    
    0 bytes  1        2        3        4               len+5
    +--------+--------+--------+--------+ +--------+--/ /---+
    |   VENDOR ID     |   COMMAND ID    | | PAYLOAD   ...   |
    +--------+--------+--------+--------+ +--------+--/ /---+
 */

void gaiaTransportGattSend(gaia_transport *transport, 
                           uint16 vendor_id,
                           uint16 command_id,
                           uint8 status, 
                           uint8 size_payload,
                           void *payload,
                           bool unpack)
{
    uint16 size_response;
    uint8 *response;

    size_response = calculate_response_size(size_payload, unpack); 

    if (status != GAIA_STATUS_NONE)
    {
        ++size_response;
    }
    
    response = malloc(size_response);
    if (response)
    {
        uint8 *r = response;

        populate_response_header(vendor_id, command_id, r);
        r += GAIA_GATT_OFFS_PAYLOAD;

        if (status != GAIA_STATUS_NONE)
        {
            *r++ = status;
        }

        populate_response_payload(payload, size_payload, r, unpack);

        gaiaTransportGattRes(transport, size_response, response, HANDLE_GAIA_RESPONSE_ENDPOINT);
        free(response);
    }
}


/*************************************************************************
NAME
    gaiaTransportAttStreamsSend
    
DESCRIPTION
    Build and send a short format GAIA packet via ATT Streams
    If unpack is true then the payload is treated as uint16[]

    Over the air packet format:
    0 bytes  1        2        3        4               len+5
    +--------+--------+--------+--------+ +--------+--/ /---+
    |   VENDOR ID     |   COMMAND ID    | | PAYLOAD   ...   |
    +--------+--------+--------+--------+ +--------+--/ /---+
 */

bool gaiaTransportAttStreamsSend(gaia_transport *transport, 
                                 uint16 vendor_id,
                                 uint16 command_id,
                                 uint8 status, 
                                 uint8 size_payload,
                                 void *payload,
                                 bool unpack)
{
    uint16 size_response;
    uint8 *response;
    bool send_status = FALSE;

    if(!transport->state.gatt.snk)
    {
        GAIA_TRANS_DEBUG(("Stream not configured\n"));
        return FALSE;
    }
    GAIA_TRANS_DEBUG(("gaiaTransportAttStreamsSend=> SinkSlack size-%d\n",SinkSlack(transport->state.gatt.snk)));

    size_response = calculate_response_size(size_payload, unpack);

    if (status != GAIA_STATUS_NONE)
    {
        ++size_response;
    }
    /* Add size for gaia handle when sending via stream */
    size_response += GAIA_HANDLE_SIZE;

    response = malloc(size_response);

    if (response)
    {
        uint8 *r = response;

        /* Prepending the response endpoint handle to which the data is sent over the air */
        *r++ = LOW(transport->state.gatt.handle_response_endpoint);
        *r++ = HIGH(transport->state.gatt.handle_response_endpoint);

        populate_response_header(vendor_id, command_id, r);
        r = r + GAIA_GATT_OFFS_PAYLOAD;

        if (status != GAIA_STATUS_NONE)
        {
            *r++ = status;
        }

        populate_response_payload(payload, size_payload, r, unpack);

        if(SinkSlack(transport->state.gatt.snk) >= size_response )
        {
            uint8 *snkData = SinkMap(transport->state.gatt.snk);
            uint16 offset = SinkClaim(transport->state.gatt.snk, size_response);

            GAIA_TRANS_DEBUG(("gaiaTransportAttStreamsSend snkData-->%d, size_response-->%d\n",offset, size_response));
            memcpy(snkData+offset, response, size_response);
            SinkFlush(transport->state.gatt.snk, size_response);
            GAIA_TRANS_DEBUG(("gaiaTransportAttStreamsSend:Notification Response flushed\n"));
            send_status = TRUE;
        }
        else
        {
            GAIA_TRANS_DEBUG(("gaiaTransportAttStreamsSend:Notification was not sent via streams\n"));
            send_status = FALSE;
            
            /*Workaround to handle unavailability of Streams Sink buffer. Applicable only in case of UPGRADE DATA_BYTES_REQ is sent, to avoid upgrade stalls*/
            if (isHostUpgradeDataBytesReq(payload, size_payload))
            {
                GAIA_TRANS_DEBUG(("gaiaTransportAttStreamsSend:send_internal_att_streams_buffer_unavailable\n"));

                send_internal_att_streams_buffer_unavailable(transport, 
                                 vendor_id,
                                 command_id,
                                 status, 
                                 size_payload,
                                 payload,
                                 unpack);
            }
        }

        free(response);
    }

    return send_status;
}

/*************************************************************************
NAME
    gaiaTransportGattSendPacket
    
DESCRIPTION
    Convert internal (long) format to GATT (short) format response
    If <task> is not NULL, send a confirmation message
*/
void gaiaTransportGattSendPacket(Task task, gaia_transport *transport, uint16 length, uint8 *data)
{
    uint16 size_response = length - GAIA_OFFS_PAYLOAD + GAIA_GATT_OFFS_PAYLOAD;
    uint8 *response = data + GAIA_OFFS_PAYLOAD - GAIA_GATT_OFFS_PAYLOAD;
    
    gaiaTransportGattRes(transport, size_response, response, HANDLE_GAIA_RESPONSE_ENDPOINT);
    
    if (task)
    {
        gaiaTransportCommonSendGaiaSendPacketCfm(transport, data, TRUE);
    }
    else
    {
        free(data);
    }
}

/*************************************************************************
NAME
    gaiaTransportIsTransportActive

DESCRIPTION
    Checks the status of the GAIA transport returns TRUE if the status is Active.
*/
static bool gaiaTransportIsTransportActive(uint16 idx)
{
    if(gaia)
    {
        if(gaia->transport[idx].state.gatt.active)
        {
            return TRUE;
        }
    }

    return FALSE;
}


/*************************************************************************
NAME
    GaiaTransportIsTransportOverGatt

DESCRIPTION
    Checks if the GAIA transport is over GATT.
*/	

static bool GaiaTransportIsTransportOverGatt(uint16 idx)
{
    if(gaia)
    {
        
        if (gaia->transport[idx].type == gaia_transport_gatt)	
        {
            return TRUE;
        }
    }
	
    return FALSE;

}

/*************************************************************************
NAME
    gaiaTransportCanAccessGaia
	
DESCRIPTION
    Checks active instance of the GAIA transport and updates the cid accordingly. 	
*/

static void gaiaTransportCanAccessGaia(uint16 cid)
{
    gaia_transport *transport = NULL;

    if(gaia)
    {
        uint16 idx;
    
        for(idx= 0 ; idx < gaia->transport_count; ++idx)
        {
            if(GaiaTransportIsTransportOverGatt(idx) && !(gaiaTransportIsTransportActive(idx))) 
            {
                transport = &gaia->transport[idx];
                break;
            }
        }
    }
	
    if(transport)
    {
        transport->state.gatt.cid = cid;
        transport->state.gatt.active = TRUE;
    }

}

/*! @brief
 */
void gaiaHandleGattManagerAccessInd(GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    gaia_transport *transport;
    uint16 handle = ind->handle;
    uint16 flags = ind->flags;
    gatt_status_t status = gatt_status_success;
    uint8 *response = NULL;
    uint16 size_response = 0;
    
    GAIA_TRANS_DEBUG(("gaia: acc cid=0x%04X hdl=0x%04X flg=%c%c%c%c off=%u siz=%u\n",
        ind->cid, ind->handle, 
        flags & ATT_ACCESS_PERMISSION       ? 'p' : '-',
        flags & ATT_ACCESS_WRITE_COMPLETE   ? 'c' : '-',
        flags & ATT_ACCESS_WRITE            ? 'w' : '-',
        flags & ATT_ACCESS_READ             ? 'r' : '-',
        ind->offset, ind->size_value));
    
    gaiaTransportCanAccessGaia(ind->cid);
    transport = gaiaTransportFromCid(ind->cid);

    if (transport)
    {
        if (flags == (ATT_ACCESS_PERMISSION | ATT_ACCESS_WRITE_COMPLETE | ATT_ACCESS_WRITE))
        {
            if (handle == HANDLE_GAIA_COMMAND_ENDPOINT)
            {
                process_command(transport, ind->size_value, ind->value);
            }
            else if (handle == HANDLE_GAIA_DATA_ENDPOINT)
            {
                 process_data(transport, ind->size_value, ind->value);
            }
            else if (handle == HANDLE_GAIA_RESPONSE_CLIENT_CONFIG)
            {
                transport->state.gatt.config_not = (ind->value[0] & 1) != 0;
                transport->state.gatt.config_ind = (ind->value[0] & 2) != 0;
                
                GAIA_TRANS_DEBUG(("gaia: cli not=%u ind=%u\n", 
                    transport->state.gatt.config_not, 
                    transport->state.gatt.config_ind));
            }
            else if (handle == HANDLE_GAIA_DATA_CLIENT_CONFIG)
            {
                transport->state.gatt.data_config_not = (ind->value[0] & 1) != 0;
                transport->state.gatt.data_config_ind = (ind->value[0] & 2) != 0;
                
                GAIA_TRANS_DEBUG(("gaia: cli not=%u ind=%u\n", 
                    transport->state.gatt.data_config_not, 
                    transport->state.gatt.data_config_ind));
            }
            else
            {
                status = gatt_status_write_not_permitted;
            }
        }
        else if (flags == (ATT_ACCESS_PERMISSION | ATT_ACCESS_READ))
        {
            if (handle == HANDLE_GAIA_RESPONSE_ENDPOINT)
            {
                response = transport->state.gatt.response;
                size_response = transport->state.gatt.size_response;
            }
            else if (handle == HANDLE_GAIA_DATA_ENDPOINT)
            {
            }
            else
            {
                status = gatt_status_read_not_permitted;
            }
        }
        else
        {
            status = gatt_status_request_not_supported;
        }
        
        if (handle)
        {
        /*  Handle 0 is handled by the demultiplexer  */
            GattManagerServerAccessResponse(&gaia->task_data, ind->cid, ind->handle, status, size_response, response);
        }
    }
    else
    {
        /* If there is is an access indication, it is required to respond */
        GattManagerServerAccessResponse(&gaia->task_data, ind->cid, ind->handle, gatt_status_insufficient_resources, 0, NULL);
        GAIA_TRANS_DEBUG(("gaia: no transport\n"));
    }
}

/*! @brief
 */
void gaiaTransportGattDisconnectReq(gaia_transport *transport)
{
    GattManagerDisconnectRequest(transport->state.gatt.cid);
}

/*! @brief
 */
void gaiaTransportGattDisconnectRes(gaia_transport *transport)
{
    gaiaTransportTidyUpOnDisconnection(transport);
}

/*************************************************************************
NAME
    gaiaTransportGattDropState
    
DESCRIPTION
    Clear down GATT-specific components of transport state
*/
void gaiaTransportGattDropState(gaia_transport *transport)
{
    memset(&transport->state.gatt, 0, sizeof transport->state.gatt);
}

/*************************************************************************
NAME
    gaiaTransportGattProcessSource
    
DESCRIPTION Send the data from stream transport for processing by rwcp/upgrade
   
*/
bool gaiaTransportGattProcessSource(void)
{
    uint16 packetSize = 0;
    uint8 *p_source;
    Source gatt_source = gaiaTransportGetSource(gaia->upgrade_transport);

    if(gaia->upgrade_large_data.in_progress)
        return FALSE;

    packetSize = SourceBoundary(gatt_source);
    GAIA_TRANS_DEBUG(("gaiaTransportGattProcessSource: packetSize %d\n",packetSize));
    if(packetSize)
    {
        bool sent_to_upgrade = TRUE;
        p_source = (uint8 *)SourceMap(gatt_source);
        sent_to_upgrade = process_data(gaia->upgrade_transport, ((packetSize)-GAIA_HANDLE_SIZE), (p_source + GAIA_HANDLE_SIZE));

        /* Read the ATT stream packet data again if the packets are not sent to upgrade library(control/out of sequence packets) */
        if(!sent_to_upgrade)
        {
            SourceDrop(gatt_source, packetSize);
            GAIA_TRANS_DEBUG(("gaiaTransportGattProcessSource: Out of sequence packets: src size after drop-->%d\n",
                              SourceSize(gatt_source)));
            return TRUE;
        }

        if(gaia->upgrade_large_data.in_progress == FALSE )
        {
            SourceDrop(gatt_source, packetSize);
            GAIA_TRANS_DEBUG(("gaiaTransportGattProcessSource: src size after drop-->%d\n",SourceSize(gatt_source)));
            /* Fall through to return FALSE from this condition */
        }
    }

    return FALSE;
}


/*! @brief
 */
bool gaiaTransportGattHandleMessage(Task task, MessageId id, Message message)
{
    bool msg_handled = FALSE;
    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
        case MESSAGE_MORE_DATA:
        {
            gaia_transport *t = gaia->upgrade_transport;

            GAIA_TRANS_DEBUG(("gaia: MESSAGE_MORE_DATA\n"));
            if (t && (t->type == gaia_transport_gatt))
            {
                while(gaiaTransportGattProcessSource());
                msg_handled = TRUE;
            }
        }
        break;

        default:
            /* indicate we couldn't handle the message */
            GAIA_TRANS_DEBUG(("gaia: gatt: unhandled 0x%04X\n", id));
            break;
    }

    return msg_handled;
}

/*! @brief
 */
Sink gaiaTransportGattGetSink(gaia_transport *transport)
{
    return transport->state.gatt.snk;
}
#endif /* GAIA_TRANSPORT_GATT */

/*****************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.
*/

/*!
  @file gaia_transport_iap2.c
  @brief Implementation of GAIA over an iAP2 External Accessory session.
*/

#include <ps.h>
#include <stdio.h> /* !!FIXME */

#include "gaia_private.h"
#include "gaia_transport.h"
#include "gaia_transport_common.h"
#include "gaia_transport_iap2.h"

#ifdef GAIA_TRANSPORT_IAP2

/*************************************************************************
NAME    
    find_session_transport
    
DESCRIPTION
    Find the transport associated with an iAP2 session.

RETURNS
    Pointer to transport or NULL if not found.
    
**************************************************************************/
static gaia_transport *find_session_transport(iap2_link *link, uint16 session_id)
{
    gaia_transport *transport = NULL;
	uint16 idx;

	for (idx = 0; idx < gaia->transport_count; ++idx)
    {
        transport = &gaia->transport[idx];
        
		if (transport->type == gaia_transport_iap2
				&& transport->state.iap2.link == link
                && transport->state.iap2.session_id == session_id)
        {
			break;
		}
	}
    
    GAIA_DEBUG(("gaia: fst %p %u: %p\n", (void *) link, session_id, (void *) transport));
	return transport;
}


/*************************************************************************
NAME    
    disconnect_transport
    
DESCRIPTION
    Disconnect the given GAIA transport from iAP2.

RETURNS
    void
    
**************************************************************************/
static void disconnect_transport(gaia_transport *transport)
{
    transport->state.iap2.link = NULL;
    transport->state.iap2.session_id = 0;
    
    gaiaTransportCommonSendGaiaDisconnectInd(transport);
}


/*************************************************************************
NAME    
    disconnect_link
    
DESCRIPTION
    Disconnect any GAIA transport on the given iAP2 link.

RETURNS
    void
    
**************************************************************************/
static void disconnect_link(iap2_link *link)
{
	uint16 idx;

	for (idx = 0; idx < gaia->transport_count; ++idx)
    {
        gaia_transport *transport = &gaia->transport[idx];
        
		if (transport->type == gaia_transport_iap2
            && transport->state.iap2.link == link)
        {
            disconnect_transport(transport);
		}
	}
}


/*************************************************************************
NAME    
    gaiaIap2SendOrFreeData
    
DESCRIPTION
    Send data on the given iAP2 transport or free the data if it could
    not be queued for transmission.

RETURNS
    TRUE if the data was queued (and will be freed later by the iAP2
    library)
    
**************************************************************************/
bool gaiaIap2SendOrFreeData(gaia_transport *transport, uint16 size_data, uint8 *data)
{
    iap2_status_t status;
    bool ok;
    
    status = Iap2EAMessage(transport->state.iap2.link,
                           transport->state.iap2.session_id,
                           size_data,
                           data,
                           FALSE);
                           
    ok = (status == iap2_status_success);
    
    if (!ok)
    {
        free(data);
    }
    
    return ok;
}


/*************************************************************************
NAME    
    GaiaHandleIap2EaSessionStartInd
    
DESCRIPTION
    Handle an IAP2_EA_SESSION_START_IND message from the iap2 library.
    Find a GAIA transport slot with a matching link ID and no session.
    If successful, set the session ID and send a connection indication to
    the client task.

RETURNS
    void
    
**************************************************************************/
void GaiaHandleIap2EaSessionStartInd(const IAP2_EA_SESSION_START_IND_T *ind)
{
    if (gaia)
    {
        gaia_transport *transport;
        
        GAIA_DEBUG(("gaia: IAP2_EA_SESSION_START_IND: link %p proto %u session %u\n",
                       (void *) ind->link,
                       ind->protocol_id,
                       ind->session_id));
        
        transport = gaiaTransportFindFree();
        
        if (transport)
        {
            transport->type = gaia_transport_iap2;
            transport->state.iap2.link = ind->link;
            transport->state.iap2.session_id = ind->session_id;
            
            gaiaTransportCommonSendGaiaConnectInd(transport, TRUE);
        }
        else
        {
            gaiaTransportCommonSendGaiaConnectInd(NULL, FALSE);
        }
    }
}


/*************************************************************************
NAME    
    GaiaHandleIap2EaSessionStopInd
    
DESCRIPTION
    Handle an IAP2_EA_SESSION_STOP_IND message from the iap2 library.
    Find the GAIA transport slot with matching link ID and session ID.
    If successful, clear the slot's session ID and send a disconnection
    indication to the client task.

RETURNS
    void
    
**************************************************************************/
void GaiaHandleIap2EaSessionStopInd(const IAP2_EA_SESSION_STOP_IND_T *ind)
{
    gaia_transport *transport;
    
    GAIA_DEBUG(("gaia: IAP2_EA_SESSION_STOP_IND: link %p session %u\n",
                   (void *) ind->link,
                   ind->session_id));
                   
    transport = find_session_transport(ind->link, ind->session_id);
    
    if (transport)
    {
        disconnect_transport(transport);
    }
}


/*************************************************************************
NAME    
    GaiaHandleIap2DisconnectInd
    
DESCRIPTION
    Handle an IAP2_DISCONNECT_IND message from the iap2 library.
    Indicate disconnection of all transports on the indicated link.

RETURNS
    void
    
**************************************************************************/
void GaiaHandleIap2DisconnectInd(const IAP2_DISCONNECT_IND_T *ind)
{
    disconnect_link(ind->link);
}


/*************************************************************************
NAME    
    GaiaHandleIap2DisconnectCfm
    
DESCRIPTION
    Handle an IAP2_DISCONNECT_CFM message from the iap2 library.
    Indicate disconnection of all transports on the indicated link.

RETURNS
    void
    
**************************************************************************/
void GaiaHandleIap2DisconnectCfm(const IAP2_DISCONNECT_CFM_T *cfm)
{
    disconnect_link(cfm->link);
}


/*************************************************************************
NAME    
    GaiaHandleIap2EaSessionDataInd
    
DESCRIPTION
    Interpret an External Accessory data packet as a GAIA command.
    
RETURNS
    void
    
**************************************************************************/
void GaiaHandleIap2EaSessionDataInd(const IAP2_EA_SESSION_DATA_IND_T *ind)
{
    gaia_transport *transport = find_session_transport(ind->link, ind->session_id);
    
    GAIA_DEBUG(("gaia: IAP2_EA_SESSION_DATA_IND: link %p session %u size %u\n",
                   (void *) ind->link,
                   ind->session_id,
                   ind->size_payload));
    
    if (transport)
    {
        uint8 *payload = (uint8 *) ind->payload;
        uint16 size_payload = ind->size_payload;
        
        while ((size_payload >= GAIA_OFFS_PAYLOAD) && (payload[GAIA_OFFS_SOF] == GAIA_SOF))
        {
            uint16 size_packet = GAIA_OFFS_PAYLOAD + payload[GAIA_OFFS_PAYLOAD_LENGTH];
            
            GAIA_DEBUG(("gaia: ea %u of %u\n", size_packet, size_payload));
           
            gaiaTransportProcessPacket(transport, payload);
            
            payload += size_packet;
            size_payload -= size_packet;
        }
    }
}


/*************************************************************************
NAME
    gaiaTransportIap2StartService
    
DESCRIPTION
    Start a GAIA service over iAP
    
*/
void gaiaTransportIap2StartService(void)
{
    bool ok = TRUE;   
    gaiaTransportCommonSendGaiaStartServiceCfm(gaia_transport_iap2, NULL, ok);
}


/*************************************************************************
NAME
    gaiaTransportIap2SendPacket
    
DESCRIPTION
    Send the passed packet over an iAP2 External Accessory transport
    If <task> is not NULL, send a confirmation message
*/
void gaiaTransportIap2SendPacket(Task task, gaia_transport *transport, uint16 length, uint8 *data)
{
    bool ok;
    
    ok = gaiaIap2SendOrFreeData(transport, length, data);
    
    if (task)
    {
        gaiaTransportCommonSendGaiaSendPacketCfm(transport, NULL, ok);
    }
}


/*************************************************************************
NAME
    gaiaTransportIap2Send
    
DESCRIPTION
    Build and send a GAIA protocol packet over iAP2
*/
void gaiaTransportIap2Send(gaia_transport *transport, 
                           uint16 vendor_id,
                           uint16 command_id,
                           uint8 status, 
                           uint8 size_payload,
                           void *payload)
{
    uint16 size_packet = GAIA_OFFS_PAYLOAD + size_payload + (status != GAIA_STATUS_NONE);
    uint8 *packet = malloc(size_packet);
    
    if (packet)
    {
        uint8 *p = packet;
        
        *p++ = GAIA_SOF;
        *p++ = GAIA_VERSION;
        *p++ = GAIA_PROTOCOL_FLAG_NONE;
        *p++ = size_payload + (status != GAIA_STATUS_NONE);
        *p++ = HIGH(vendor_id);
        *p++ = LOW(vendor_id);
        *p++ = HIGH(command_id);
        *p++ = LOW(command_id);

        if (status != GAIA_STATUS_NONE)
        {
            *p++ = status;
        }
        
        memmove(packet + GAIA_OFFS_PAYLOAD, payload, size_payload);
        
        gaiaIap2SendOrFreeData(transport, size_packet, packet);
    }
}


/*************************************************************************
NAME
    gaiaTransportIap2DropState
    
DESCRIPTION
    Clear down iAP2-specific components of transport state
*/
void gaiaTransportIap2DropState(gaia_transport *transport)
{
    memset(&transport->state.iap2, 0, sizeof transport->state.iap2);
}


/*************************************************************************
NAME
    gaiaTransportIap2GetBdAddr
    
DESCRIPTION
    Get the Bluetooth Device Address of the Host on the given transport
    
RETURNS
    TRUE on success.
*/
bool gaiaTransportIap2GetBdAddr(gaia_transport *transport, typed_bdaddr *taddr)
{
    bdaddr addr;
    bool ok;
    
    ok = (Iap2GetBdaddrForLink(transport->state.iap2.link, &addr) == iap2_status_success);
    
    if (ok)
    {
        taddr->type = TYPED_BDADDR_PUBLIC;
        taddr->addr = addr;
    }
 
    return ok;
}

void gaiaTransportIap2DisconnectReq(gaia_transport *transport)
{
    if (transport->type == gaia_transport_iap2)
    {
        Iap2DisconnectRequest(transport->state.iap2.link);
    }
}

#endif /* def GAIA_TRANSPORT_IAP2 */

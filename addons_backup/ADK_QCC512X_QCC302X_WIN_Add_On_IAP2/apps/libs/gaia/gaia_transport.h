/*****************************************************************
Copyright (c) 2011 - 2018 Qualcomm Technologies International, Ltd.
*/

#ifndef _GAIA_TRANSPORT_H
#define _GAIA_TRANSPORT_H

#include<bdaddr.h>

/*! @brief Attempt to connect Gaia to a device over a given transport.
 */
void gaiaTransportConnectReq(gaia_transport *transport, tp_bdaddr *address);

/*! @brief Attempt to disconnect Gaia over a given transport.
 */
void gaiaTransportDisconnectReq(gaia_transport *transport);

/*! @brief Respond to a Gaia disconnection indication over a given transport.
 */
void gaiaTransportDisconnectRes(gaia_transport *transport);

/*! @brief Attempt to find a free transport slot
 */
gaia_transport *gaiaTransportFindFree(void);
        
/*! @brief Attempt to find the tranport associated with a GATT CID
 */
gaia_transport *gaiaTransportFromCid(uint16 cid);

/*! @brief Start Gaia as a server on a given transport.
 *
 * NOTE - only applicable to the SPP transport.
 */
void gaiaTransportStartService(gaia_transport_type transport_type);

/*! @brief Start Gaia as a GATT server.
 */
void gaiaTransportStartGattServer(uint16 start_handle, uint16 end_handle);

/*! @brief Stop Gaia as a server on a given transport.
 */
void gaiaTransportStopService(gaia_transport_type transport_type, uint16 link_id);

/*! @brief Transmit a Gaia packet over a given transport.
 */
void gaiaTransportSendPacket(Task task, gaia_transport *transport, uint16 length, uint8 *data);

/*! @brief Get the stream source for a given transport.
 *
 * NOTE - only applicable to the SPP transport.
 */
Source gaiaTransportGetSource(gaia_transport *transport);

/*! @brief Get the stream sink for a given transport.
 */
Sink gaiaTransportGetSink(gaia_transport *transport);

/*! @brief Get the id for a given transport.
 */
uint16 gaiaTransportGetId(gaia_transport *transport);

/*! @brief Get the transport type for a given transport.
 */
gaia_transport_type gaiaTransportGetType(gaia_transport *transport);

/*! @brief Analyse an inbound command packet and process the command
*/
void gaiaTransportProcessPacket(gaia_transport *transport, uint8 *packet);

/*! @brief Deserialise a command from a stream transport and send it for processing
 */
void gaiaTransportProcessSource(gaia_transport *transport);

/*! @brief Pass incoming message for handling by a given transport.
 */
bool gaiaTransportHandleMessage(Task task, MessageId id, Message message);

/*! @brief
 */
void gaiaTransportGattSend(gaia_transport *transport, 
                           uint16 vendor_id,
                           uint16 command_id,
                           uint8 status, 
                           uint8 size_payload,
                           void *payload,
                           bool unpack);

/*! @brief Pass incoming message via ATT streams.
     returns TRUE if successfully sent via streams.
 */
bool gaiaTransportAttStreamsSend(gaia_transport *transport, 
                                 uint16 vendor_id,
                                 uint16 command_id,
                                 uint8 status, 
                                 uint8 size_payload,
                                 void *payload,
                                 bool unpack);

/*! @brief
 */
void gaiaTransportSinkSend(gaia_transport *transport, 
                           uint16 vendor_id,
                           uint16 command_id,
                           uint8 status, 
                           uint8 size_payload,
                           void *payload,
                           bool unpack);

/*! @brief Clear down state of given transport.
 */
void gaiaTransportDropState(gaia_transport *transport);

/*! @brief Clean transport state on disconnection.
 */
void gaiaTransportTidyUpOnDisconnection(gaia_transport *transport);

#endif /* _GAIA_TRANSPORT_H */

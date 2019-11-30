/*****************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.
*/

/*!
  @file gaia_transport_iap2.h
  @brief Implementation of GAIA over an iAP2 External Accessory session.
*/

#ifndef _GAIA_TRANSPORT_IAP2_H_
#define _GAIA_TRANSPORT_IAP2_H_

#include "gaia_private.h"

#ifdef GAIA_TRANSPORT_IAP2

/*! @brief Start GAIA service over iAP2 transport.
 */
void gaiaTransportIap2StartService(void);

/*! @brief Send data over iAP2 link or free the data on error.
 */
bool gaiaIap2SendOrFreeData(gaia_transport *transport, uint16 size_data, uint8 *data);

/*! @brief Send a GAIA command over an iAP2 transport.
 */
void gaiaTransportIap2Send(gaia_transport *transport, 
                           uint16 vendor_id,
                           uint16 command_id,
                           uint8 status, 
                           uint8 size_payload,
                           void *payload);

/*! @brief Send a formatted GAIA packet over iAP2 transport.
 */
void gaiaTransportIap2SendPacket(Task task,
                                 gaia_transport *transport, 
                                 uint16 length, 
                                 uint8 *data);

/*! @brief Get the Bluetooth Device Address of the Host on the given transport.
 */
bool gaiaTransportIap2GetBdAddr(gaia_transport *transport, typed_bdaddr *taddr);

/*! @brief Clear down iAP2-specific components of transport state
 */
void gaiaTransportIap2DropState(gaia_transport *transport);

void gaiaTransportIap2DisconnectReq(gaia_transport *transport);

#endif /* def GAIA_TRANSPORT_IAP2 */

#endif /* def _GAIA_TRANSPORT_IAP2_H_ */

/*****************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.
*/

#ifndef _GAIA_TRANSPORT_GATT_H_
#define _GAIA_TRANSPORT_GATT_H_

#ifdef GAIA_TRANSPORT_GATT

/*! @brief
 */
void gaiaTransportGattInit(gaia_transport *transport);

/*! @brief
 */
void gaiaTransportGattRegisterServer(uint16 start_handle, uint16 end_handle);

/*! @brief
 */
void gaiaTransportGattConnectRes(gaia_transport *transport);

/*! @brief
 */
void gaiaTransportGattDisconnectReq(gaia_transport *transport);

/*! @brief
 */
void gaiaTransportGattDisconnectRes(gaia_transport *transport);

/*! @brief
 */
void gaiaTransportGattDropState(gaia_transport *transport);

/*! @brief
 */
Sink gaiaTransportGattGetSink(gaia_transport *transport);

/*! @brief
 */
void gaiaTransportGattSendPacket(Task task, gaia_transport *transport, uint16 length, uint8 *data);

/*! @brief Send a GATT response.
 */
void gaiaTransportGattRes(gaia_transport *transport, uint16 size_response, uint8 *response, uint8 handle);

/*! @brief
 */
bool gaiaTransportGattHandleMessage(Task task, MessageId id, Message message);

/*! @brief Process data arrived at GATT source
 */
bool gaiaTransportGattProcessSource(void);

/*! @brief
 */
void gaiaHandleGattManagerAccessInd(GATT_MANAGER_SERVER_ACCESS_IND_T *ind);

#endif /* GAIA_TRANSPORT_Gatt */

#endif /* _GAIA_TRANSPORT_GATT_H_ */

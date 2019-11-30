/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_transport.h

*/

#ifndef __AMA_TRANSPORT_H_
#define __AMA_TRANSPORT_H_

#include "ama.h"

/* Internal APIs */

typedef bool (*AMA_TRAN_TX_CALLBACK) (uint8* data, uint16 size);

void AmaTransportInit(void);
void amaTransportBtClassicInit(bdaddr* bd_addr);
bool AmaTransportIsBtClassic(void);
void AmaTransportResetParseState(void);
uint16 amaTransportGetNumberOfTranports(void);
void amaTranportConnectBtClassic(bdaddr* bd_addr);
bdaddr* amaTransportGetPeerAddr(void);
void AmaTransportSwitch(ama_transport_t transport);
void AmaTransportUpgrade(ama_transport_t transport);
void amaTranportSendProtoBuf(uint8* data, uint16 length);
bdaddr* amaGetLocalAddress(void);
void amaStoreLocalAddress(bdaddr* bd_addr);
bool AmaCheckVersionAndStream(uint16 version, uint16 streamId);
bool amaTranportStreamData(uint8* stream_data, uint16 length);
bool amaTransportParseData(const uint8* data, uint16 size);
void amaTransportStorePeerAddr(bdaddr* bd_addr);
void amaTransportSetTxCallback(AMA_TRAN_TX_CALLBACK cb, ama_transport_t id);



#endif /* __AMA_TRANSPORT_H_ */

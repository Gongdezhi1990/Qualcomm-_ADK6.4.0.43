/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_transport.c

*/

#include <bdaddr.h>
#include <connection.h>
#include <message.h>
#include <panic.h>
#include <region.h>
#include <service.h>
#include <sink.h>
#include <source.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stream.h>
#include <util.h>
#include <vm.h>
#include <transform.h>
#include <gatt_ama_server.h>
#include <ama.h>
#include "ama_transport.h"
#include "ama_receive_command.h"
#include "ama_debug.h"
#include "ama_rfcomm.h"
#include "ama_private.h"
#include "ama_iap2.h"


#define AMA_TRANSPORT_STREAM_ID_PROTO_BUFF 0x0
#define AMA_TRANSPORT_STREAM_ID_VOICE 0x1

typedef enum
{
    amaParseTransportStateIdle,
    amaParseTransportStateHeader,
    amaParseTransportStateLength,
    amaParseTransportStateBody,
    amaParseTransportLast
}amaParseTransportState_t;



typedef struct __amatrans_t
{
    ama_transport_t amaTransport;
    ama_transport_t amaUpgradeTransport;

    uint8 storedStreamId;
    uint8 storedVersion;

    amaParseTransportState_t  amaParseTransportState;

    bdaddr peer_classic_addr;
    bdaddr local_classic_addr;

    AMA_TRAN_TX_CALLBACK amaTranTxCallback[NUMBER_OF_SUPPORTED_TRANSPORTS];

    ama_transport_t numberOfSupportTransport;

}amatrans_t;


static amatrans_t amaTransport;



#define AMA_HEADER_VERSION_OFFSET 12
#define AMA_HEADER_VERSION_MASK 0xF000
#define AMA_HEADER_STREAM_ID_OFFSET 7
#define AMA_HEADER_STREAM_ID_MASK 0x0F80

#define AMA_HEADER_LENTGH_MASK 0x0001

#define AMA_VERSION_EXCHANGE_SIZE 20

#define PACKET_INVALID_LENGTH 0xFFFF
static uint16 header = 0;
static uint16 protoPktLength = PACKET_INVALID_LENGTH;
static uint8* protoBody = NULL;
static uint16 bytesCopied = 0;

void amaTransportSetTxCallback(AMA_TRAN_TX_CALLBACK cb, ama_transport_t id)
{
    if(id == ama_transport_none)
    {
        return;
    }

    amaTransport.amaTranTxCallback[id] = cb;

    amaTransport.numberOfSupportTransport = MAX(amaTransport.numberOfSupportTransport,id + 1);
}

void AmaTransportInit(void)
{
    AmaTransportSwitch(ama_transport_ble);

    amaTransport.amaUpgradeTransport = ama_transport_none;

    amaTransport.amaParseTransportState = amaParseTransportStateIdle;

    amaTransport.numberOfSupportTransport = 0;

    amaTransportSetTxCallback(SendAmaNotification, ama_transport_ble);
    amaTransportSetTxCallback(AmaRFCommSendData, ama_transport_rfcomm);
}

uint16 amaTransportGetNumberOfTranports(void)
{
    return amaTransport.numberOfSupportTransport;
}


void AmaTransportUpgrade(ama_transport_t transport)
{
    amaTransport.amaUpgradeTransport = transport;
}

void AmaTransportSwitch(ama_transport_t transport)
{
    MAKE_AMA_MESSAGE(AMA_SWITCH_TRANSPORT_IND);

    AMA_DEBUG((" AmaTransportSwitch %d\n", transport ));

    amaTransport.amaTransport = amaTransport.amaUpgradeTransport;

    amaTransport.amaUpgradeTransport = ama_transport_none;

    amaTransport.amaTransport = transport;

    message->transport = amaTransport.amaTransport;

    amaSendSinkMessage(AMA_SWITCH_TRANSPORT_IND, message);
}

ama_transport_t AmaTransportGet(void)
{
    return amaTransport.amaTransport;
}

bool AmaCheckVersionAndStream(uint16 version, uint16 streamId)
{
    amaTransport.storedVersion = version;
    amaTransport.storedStreamId = streamId;
    return TRUE;
}


void AmaTransportSendVersion(uint8 major, uint8 minor )
{
    AMA_DEBUG((" AmaTransportSendVersion Major %d Minor %d\n", major, minor ));

    uint8 version[AMA_VERSION_EXCHANGE_SIZE] = {0};

    version[1] = 0x03;
    version[0] = 0xFE;
    version[2] = major;
    version[3] = minor;

    /* Transport packet size */
    version[4] = 0;
    version[5] = 0;

    /* Maximum transactional data size */
    version[6] = 0;
    version[7] = 0;

    if(amaTransport.amaTranTxCallback[amaTransport.amaTransport])
    {
        amaTransport.amaTranTxCallback[amaTransport.amaTransport](version, AMA_VERSION_EXCHANGE_SIZE);
    }
}


bool AmaTransportIsBle(void)
{
    return (amaTransport.amaTransport == ama_transport_ble);
}

bool AmaTransportIsBtClassic(void)
{
    return (amaTransport.amaTransport == ama_transport_rfcomm);
}

void AmaTransportResetParseState(void)
{
    header = 0;
    protoPktLength = PACKET_INVALID_LENGTH;

    free(protoBody);
    protoBody = NULL;

    bytesCopied = 0;
    amaTransport.amaParseTransportState = amaParseTransportStateIdle;
}

bool amaTransportParseData(const uint8* stream, uint16 size)
{

    AMA_DEBUG((" AMA TRANSPORT amaTransportParseData, size is %d\n", size ));

    while(size)
    {
        switch(amaTransport.amaParseTransportState)
        {
            case amaParseTransportStateIdle:
                if(size > 0)
                {
                    header = (uint16)stream[0]<<8;
                    amaTransport.amaParseTransportState = amaParseTransportStateHeader;
                    stream++;
                    size--;
                }

                if(size > 0)
                {
                    header |= (uint16)stream[0];
                    amaTransport.amaParseTransportState = amaParseTransportStateLength;
                    stream++;
                    size--;
                }
                break;

            case amaParseTransportStateHeader:
                if(size > 0)
                {
                    header |= (uint16)stream[0];
                    amaTransport.amaParseTransportState = amaParseTransportStateLength;
                    stream++;
                    size--;
                }
                break;

            case amaParseTransportStateLength:
                if(size > 0)
                {
                    if(header & AMA_HEADER_LENTGH_MASK)
                    {
                        if(protoPktLength == PACKET_INVALID_LENGTH)
                        {
                            protoPktLength = (uint16)stream[0]<<8;
                        }
                        else
                        {
                            protoPktLength |= (uint16)stream[0];
                            amaTransport.amaParseTransportState = amaParseTransportStateBody;
                        }
                        stream++;
                        size--;
                    }
                    else
                    {
                        protoPktLength = (uint16)stream[0];
                        amaTransport.amaParseTransportState = amaParseTransportStateBody;
                        stream++;
                        size--;
                    }

                    if(amaTransport.amaParseTransportState == amaParseTransportStateBody)
                    {
                        if(protoBody != NULL)
                            Panic();
                        protoBody = PanicUnlessMalloc(protoPktLength);
                    }
                }
                break;

            case amaParseTransportStateBody:
                {
                    uint16 remainToCopy = protoPktLength - bytesCopied;
                    uint16 bytesToCopy = MIN(size,remainToCopy);

                    AMA_DEBUG((" AMA TRANSPORT amaParseTransportStateBody bytes to Copy is %d\n", bytesToCopy ));

                    if(bytesToCopy)
                    {
                        memcpy(protoBody + bytesCopied, stream, bytesToCopy);
                        bytesCopied += bytesToCopy;
                        stream += bytesToCopy;
                        size -= bytesToCopy;
                    }

                    remainToCopy = protoPktLength - bytesCopied;

                    if(remainToCopy == 0)
                    {
                        /* received a complete protobuff packet */

                        uint16 streamId = (header & AMA_HEADER_STREAM_ID_MASK)>>AMA_HEADER_STREAM_ID_OFFSET;
                        uint16 version = (header & AMA_HEADER_VERSION_MASK)>>AMA_HEADER_VERSION_OFFSET;

                        AmaCheckVersionAndStream(version, streamId);

                        amaReceiveCommand((char*)protoBody, protoPktLength);

                        AmaTransportResetParseState();
                    }
                }
                break;

            default:
                break;

        }
    } /* end if while(size) */

    return (amaTransport.amaParseTransportState == amaParseTransportStateIdle);
}


bool amaTranportStreamData(uint8* stream_data, uint16 length)
{
    uint8 headerSize = 4;
    uint16 streamHeader = 0;

    streamHeader = (amaTransport.storedVersion<<AMA_HEADER_VERSION_OFFSET) & AMA_HEADER_VERSION_MASK;
    streamHeader |= (AMA_TRANSPORT_STREAM_ID_VOICE<<AMA_HEADER_STREAM_ID_OFFSET) & AMA_HEADER_STREAM_ID_MASK;

    if(length > 255)
    {
        streamHeader |= AMA_HEADER_LENTGH_MASK;

        stream_data[2] = (uint8) (length>>8);
        stream_data[3] = (uint8) (length & 0xFF);
    }
    else
    {
        stream_data[2] = length;

        headerSize = 3;
    }

    stream_data[0] = (uint8) (streamHeader>>8);
    stream_data[1] = (uint8) (streamHeader & 0xFF);

    length += headerSize;


    return amaTransport.amaTranTxCallback[amaTransport.amaTransport](stream_data,length);
}



void amaTranportSendProtoBuf(uint8* data, uint16 length)
{
    uint16 streamHeader = 0;
    uint8 headerSize = 3;

    streamHeader = (amaTransport.storedVersion<<AMA_HEADER_VERSION_OFFSET) & AMA_HEADER_VERSION_MASK;
    streamHeader |= (AMA_TRANSPORT_STREAM_ID_PROTO_BUFF<<AMA_HEADER_STREAM_ID_OFFSET) & AMA_HEADER_STREAM_ID_MASK;

    if(length > 255)
    {
        streamHeader |= AMA_HEADER_LENTGH_MASK;

        data[2] = (uint8) (length>>8);
        data[3] = (uint8) (length & 0xFF);

        headerSize = 4;
    }
    else
    {
        data[2] = length;
    }

    data[0] = (uint8) (streamHeader>>8);
    data[1] = (uint8) (streamHeader & 0xFF);

    length += headerSize;

    AMA_DEBUG(("TX tran %d len %d\n", (uint8)amaTransport.amaTransport, length));


    amaTransport.amaTranTxCallback[amaTransport.amaTransport](data,length);

}



void amaTransportBtClassicInit(bdaddr* bd_addr)
{
    AmaRfCommInit(NULL, bd_addr);
}


void AmaTransportStorePeerAddr(bdaddr* bd_addr)
{
        amaTransport.peer_classic_addr = *bd_addr;
}


void amaStoreLocalAddress(bdaddr* bd_addr)
{
    amaTransport.local_classic_addr = *bd_addr;
}


bdaddr* amaGetLocalAddress(void)
{
    return &amaTransport.local_classic_addr;
}



bdaddr* amaTransportGetPeerAddr(void)
{
        return &amaTransport.peer_classic_addr;
}


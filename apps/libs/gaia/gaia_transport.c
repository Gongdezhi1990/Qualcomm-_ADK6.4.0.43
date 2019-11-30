/*****************************************************************
Copyright (c) 2011 - 2018 Qualcomm Technologies International, Ltd.
 */

#include "gaia.h"
#include "gaia_private.h"
#include "gaia_transport.h"
#include "gaia_transport_common.h"
#include "upgrade.h"
#include "transport_manager.h"
#include <stream.h>
#include <audio.h>
#include <source.h>
#include <sink.h>
#include <stdlib.h>
#include <stdio.h>
#include <panic.h>
#include <print.h>
#include <string.h>
#include <vmal.h>
#include <connection.h>
#include <bdaddr.h>
#ifdef GAIA_HAVE_SPP_CLIENT
#include <sppc.h>
#endif
#include <library.h>

#ifdef GAIA_TRANSPORT_GATT
#include "gaia_transport_gatt.h"
#endif



static const uint8 gaia_transport_rfcomm_service_record[] =
{
    0x09, 0x00, 0x01,           /*  0  1  2  ServiceClassIDList(0x0001) */
    0x35,   17,                 /*  3  4     DataElSeq 17 bytes */
    0x1C, 0x00, 0x00, 0x11, 0x07, 0xD1, 0x02, 0x11, 0xE1, 0x9B, 0x23, 0x00, 0x02, 0x5B, 0x00, 0xA5, 0xA5,       
                                /*  5 .. 21  UUID GAIA (0x00001107-D102-11E1-9B23-00025B00A5A5) */
    0x09, 0x00, 0x04,           /* 22 23 24  ProtocolDescriptorList(0x0004) */
    0x35,   12,                 /* 25 26     DataElSeq 12 bytes */
    0x35,    3,                 /* 27 28     DataElSeq 3 bytes */
    0x19, 0x01, 0x00,           /* 29 30 31  UUID L2CAP(0x0100) */
    0x35,    5,                 /* 32 33     DataElSeq 5 bytes */
    0x19, 0x00, 0x03,           /* 34 35 36  UUID RFCOMM(0x0003) */
    0x08, SPP_DEFAULT_CHANNEL,  /* 37 38     uint8 RFCOMM channel */
#define GAIA_RFCOMM_SR_CH_IDX (38)
    0x09, 0x00, 0x06,           /* 39 40 41  LanguageBaseAttributeIDList(0x0006) */
    0x35,    9,                 /* 42 43     DataElSeq 9 bytes */
    0x09,  'e',  'n',           /* 44 45 46  Language: English */
    0x09, 0x00, 0x6A,           /* 47 48 49  Encoding: UTF-8 */
    0x09, 0x01, 0x00,           /* 50 51 52  ID base: 0x0100 */
    0x09, 0x01, 0x00,           /* 53 54 55  ServiceName 0x0100, base + 0 */
    0x25,   4,                 /* 56 57     String length 4 */
    'G', 'A', 'I', 'A',          /* 58 59 60 61  "GAIA" */
};

static const uint8 gaia_transport_spp_service_record[] =
{
    0x09, 0x00, 0x01,           /*  0  1  2  ServiceClassIDList(0x0001) */
    0x35,    3,                 /*  3  4     DataElSeq 3 bytes */
    0x19, 0x11, 0x01,           /*  5  6  7  UUID SerialPort(0x1101) */
    0x09, 0x00, 0x04,           /*  8  9 10  ProtocolDescriptorList(0x0004) */
    0x35,   12,                 /* 11 12     DataElSeq 12 bytes */
    0x35,    3,                 /* 13 14     DataElSeq 3 bytes */
    0x19, 0x01, 0x00,           /* 15 16 17  UUID L2CAP(0x0100) */
    0x35,    5,                 /* 18 19     DataElSeq 5 bytes */
    0x19, 0x00, 0x03,           /* 20 21 22  UUID RFCOMM(0x0003) */
    0x08, SPP_DEFAULT_CHANNEL,  /* 23 24     uint8 RFCOMM channel */
#define GAIA_SR_CH_IDX (24)
    0x09, 0x00, 0x06,           /* 25 26 27  LanguageBaseAttributeIDList(0x0006) */
    0x35,    9,                 /* 28 29     DataElSeq 9 bytes */
    0x09,  'e',  'n',           /* 30 31 32  Language: English */
    0x09, 0x00, 0x6A,           /* 33 34 35  Encoding: UTF-8 */
    0x09, 0x01, 0x00,           /* 36 37 38  ID base: 0x0100 */
    0x09, 0x01, 0x00,           /* 39 40 41  ServiceName 0x0100, base + 0 */
    0x25,   4,                 /* 42 43     String length 4 */
    'G', 'A', 'I', 'A',          /* 44 45 46 47 "GAIA" */
    0x09, 0x00, 0x09,       /* 48 49 50  BluetoothProfileDescriptorList(0x0009) */
    0x35, 0x06,             /* 51 52     DataElSeq 3 bytes */
    0x19, 0x11, 0x01,       /* 53 54 55  UUID SerialPort(0x1101) */   
    0x09, 0x01, 0x02,       /* 56 57 58  SerialPort Version (0x0102) */
};


typedef struct {
    Source source;
    uint16 idx;
    uint16 expected;
    uint16 packet_length;
    uint16 data_length;
    uint8 *data;
    uint8 *packet;
    uint8 flags;
    uint8 check;
} gaia_transport_process_source_data_t;

static gaia_transport_type transport_registered = gaia_transport_none;

#ifdef GAIA_HAVE_SPP_CLIENT
/*! @brief 
 */
static void gaiaTransportSppConnectReq(gaia_transport *transport, bdaddr *address)
{
    SppConnectRequest(&gaia->task_data, address, 0, 0);
}

/*! @brief Attempt to find the tranport associated with an RFCOMM channel */
static gaia_transport *gaiaTransportFromRfcommChannel(uint16 channel) 
{   
    uint16 idx;    
    for (idx = 0; idx < gaia->transport_count; ++idx) 
    {        
        if (((gaia->transport[idx].type == gaia_transport_rfcomm)                
            || (gaia->transport[idx].type == gaia_transport_spp))                
            && (gaia->transport[idx].state.spp.rfcomm_channel == channel))
            return &gaia->transport[idx];    
    }    
    return NULL;
}
#endif /* GAIA_HAVE_SPP_CLIENT */

static void gaiaTransportSdpRegister(gaia_transport_type transport_type, uint8 channel)
{
    const uint8 *sr;
    uint16 size_of_rec;
    if(transport_type == gaia_transport_rfcomm)
    {
        /* Default to use const record */
        size_of_rec = sizeof(gaia_transport_rfcomm_service_record);
        sr = gaiaTransportCommonServiceRecord(gaia_transport_rfcomm_service_record, GAIA_RFCOMM_SR_CH_IDX, channel);
    }
    else
    {
        /* Default to use const record */
        size_of_rec = sizeof(gaia_transport_spp_service_record);
        sr = gaiaTransportCommonServiceRecord(gaia_transport_spp_service_record, GAIA_SR_CH_IDX, channel);
    }

    if(!sr)
        GAIA_PANIC();

    /* Store the RFCOMM channel */
    gaia->spp_listen_channel = channel;
    GAIA_TRANS_DEBUG(("gaia: ch %u\n", channel));

    /* Register the SDP record */
    ConnectionRegisterServiceRecord(&gaia->task_data, size_of_rec, sr);

}

/*************************************************************************
NAME
    gaiaTransportStreamSendPacket
    
DESCRIPTION
    Copy the passed packet to the transport sink and flush it
    If <task> is not NULL, send a confirmation message
*/
static void gaiaTransportStreamSendPacket(Task task, gaia_transport *transport, uint16 length, uint8 *data)
{
    bool status = FALSE;
    gaia_transport_type type = transport->type;
    uint16 cid = transport->state.spp.rfcomm_channel;
    uint16 space_available;

    if (gaia)
    {
        space_available = TransportMgrGetAvailableSpace(type,cid);

        if (space_available<length)
        {
            GAIA_TRANS_DEBUG(("gaia: no sink\n"));
        }
        else
        {
            status = TransportMgrWriteData(type,cid,data,length);

#ifdef DEBUG_GAIA_TRANSPORT
            {
                uint16 idx;
                GAIA_DEBUG(("gaia: put"));
                for (idx = 0; idx < length; ++idx)
                    GAIA_DEBUG((" %02x", data[idx]));
                GAIA_DEBUG(("\n"));
            }
#endif
            if(status == TRUE)
                status = TransportMgrDataSend(type, cid, length);
        }   
    }
    
    if (task)
        gaiaTransportCommonSendGaiaSendPacketCfm(transport, data, status);
    
    else
        free(data);
}



/*! @brief Attempt to find the tranport associated with a sink
 */
static gaia_transport *gaiaTransportFromSink(Sink sink) 
{
    uint16 idx;

    for (idx = 0; idx < gaia->transport_count; ++idx)
        if (gaiaTransportGetSink(&gaia->transport[idx]) == sink)
            return &gaia->transport[idx];
    return NULL;
}

/*! @brief Attempt to find the tranport associated with id and type
 */
static gaia_transport *gaiaTransportFromIdType(uint16 Id, gaia_transport_type type) 
{
    uint16 idx;

    for (idx = 0; idx < gaia->transport_count; ++idx)
        if ((gaiaTransportGetId(&gaia->transport[idx]) == Id) &&
             (gaiaTransportGetType(&gaia->transport[idx])== type))
            return &gaia->transport[idx];

    return NULL;
}

/*! @brief Notify clients of disconnection.
 */
static void gaiaTransportNotifyClientsOfDisconnection(gaia_transport *transport)
{
    if (transport == gaia->upgrade_transport)
    {
        UpgradeTransportDisconnectRequest();
    }
}


/*! @brief Attempt to find a free transport slot
 */
gaia_transport *gaiaTransportFindFree(void)
{
    if (gaia)
    {
        uint16 idx;
        
        for (idx = 0; idx < gaia->transport_count; ++idx)
        {
            if (gaia->transport[idx].type == gaia_transport_none)
            {
                return &gaia->transport[idx];
            }
        }
    }

    return NULL;
}


#ifdef GAIA_TRANSPORT_GATT
/*! @brief Attempt to find the tranport associated with a GATT CID
 */
gaia_transport *gaiaTransportFromCid(uint16 cid) {
    uint16 idx;

    for (idx = 0; idx < gaia->transport_count; ++idx) {
        if (gaia->transport[idx].type == gaia_transport_gatt
                && gaia->transport[idx].state.gatt.cid == cid) {
            return &gaia->transport[idx];
        }
    }
    return NULL;
}
#endif /* GAIA_TRANSPORT_GATT */

/*! @brief Attempt to connect Gaia to a device over SPP transport.
 */
void gaiaTransportConnectReq(gaia_transport *transport, tp_bdaddr *address)
{
#if defined GAIA_TRANSPORT_SPP && defined GAIA_HAVE_SPP_CLIENT
    if (transport == NULL)
    {
        transport = gaiaTransportFindFree();
        if (transport != NULL)
        {
            transport->type = gaia_transport_spp;
        }
    }
#else
    UNUSED(address);
#endif

    if (transport == NULL)
    {
        gaiaTransportCommonSendGaiaConnectCfm(NULL, FALSE);
        return;
    }
    
    switch (transport->type)
    {
#if defined GAIA_TRANSPORT_SPP && defined GAIA_HAVE_SPP_CLIENT
        case gaia_transport_spp:
            gaiaTransportSppConnectReq(transport, address->taddr.addr);
            break;
#endif
#if defined GAIA_TRANSPORT_RFCOMM 
        case gaia_transport_rfcomm:
            TransportMgrConnect(transport->type, gaia->spp_listen_channel, SPP_DEFAULT_CHANNEL, address);
            break;
#endif
        default:
            GAIA_TRANS_DEBUG(("Unknown Gaia transport %d\n", transport->type));
            GAIA_PANIC();
            gaiaTransportCommonSendGaiaConnectCfm(transport, FALSE);
            break;
    }
}

/*! @brief Attempt to disconnect Gaia over a given transport.
 */
void gaiaTransportDisconnectReq(gaia_transport *transport) {
    switch (transport->type) 
    {
#if defined GAIA_TRANSPORT_RFCOMM || defined GAIA_TRANSPORT_SPP
    case gaia_transport_rfcomm:
    case gaia_transport_spp:
        TransportMgrDisconnect(transport->type, gaiaTransportGetSink(transport));
        break;
#endif
#ifdef GAIA_TRANSPORT_GATT
    case gaia_transport_gatt:
        gaiaTransportGattDisconnectReq(transport);
        transport->state.gatt.active = FALSE;
        break;
#endif
    default:
        GAIA_TRANS_DEBUG(("Unknown Gaia transport %d\n", transport->type));
        GAIA_PANIC();
        break;
    }
}

/*! @brief Respond to a Gaia disconnection indication over a given transport.
 */
void gaiaTransportDisconnectRes(gaia_transport *transport) {
    switch (transport->type) 
    {
#if defined GAIA_TRANSPORT_RFCOMM || defined GAIA_TRANSPORT_SPP
    case gaia_transport_rfcomm:
    case gaia_transport_spp:
        UNUSED(transport);
        break;
#endif
#ifdef GAIA_TRANSPORT_GATT
    case gaia_transport_gatt:
        gaiaTransportGattDisconnectRes(transport);
        break;
#endif
    default:
        GAIA_TRANS_DEBUG(("Unknown Gaia transport %d\n", transport->type));
        GAIA_PANIC();
        break;
    }
}

/*! @brief Clear down state of given transport.
 */
void gaiaTransportDropState(gaia_transport *transport)
{
    if (transport) {
        GAIA_TRANS_DEBUG(("gaia: drop state t=%p %d o=%p\n",
                        (void *) transport, transport->type, (void *) gaia->outstanding_request));

        if (gaia->outstanding_request == transport)
            gaia->outstanding_request = NULL;

        if (gaia->upgrade_transport == transport)
            gaia->upgrade_transport = NULL;

        if (gaia->data_endpoint_mode)
            gaia->data_endpoint_mode = GAIA_DATA_ENDPOINT_MODE_NONE;

        if (gaia->pfs_state != PFS_NONE) {
            GAIA_TRANS_DEBUG(("gaia: drop pfs %d s=%lu r=%lu\n",
                            gaia->pfs_state, gaia->pfs_sequence, gaia->pfs_raw_size));

            gaia->pfs_sequence = 0;
            gaia->pfs_raw_size = 0;
            SinkClose(gaia->pfs.sink);
            gaia->pfs_state = PFS_NONE;
        }

        if (AudioBusyTask() == &gaia->task_data)
            SetAudioBusy(NULL);

        switch (transport->type)
        {
        case gaia_transport_none:
            break;
            
#if defined GAIA_TRANSPORT_RFCOMM || defined GAIA_TRANSPORT_SPP
        case gaia_transport_rfcomm:
        case gaia_transport_spp:
            transport->state.spp.sink = NULL;
            break;
#endif

#ifdef GAIA_TRANSPORT_GATT
        case gaia_transport_gatt:
            gaiaTransportGattDropState(transport);
            break;
#endif
        default:
            GAIA_TRANS_DEBUG(("Unknown Gaia transport %d\n", transport->type));
            GAIA_PANIC();
            break;
        }

        transport->connected = FALSE;
        transport->enabled = FALSE;
        transport->has_voice_assistant = FALSE;
        transport->type = gaia_transport_none;

        /* No longer have a Gaia connection over this transport, ensure we reset any threshold state */
        gaiaTransportCommonCleanupThresholdState(transport);
    }
    else
    {
        GAIA_TRANS_DEBUG(("gaia: no transport\n"));
    }
}

/*! @brief Start Gaia as a server on a given transport.
 */
void gaiaTransportStartService(gaia_transport_type transport_type) 
{   

    transport_mgr_link_cfg_t link_cfg;

    switch (transport_type) 
    {
#if defined GAIA_TRANSPORT_RFCOMM || defined GAIA_TRANSPORT_SPP
        case gaia_transport_rfcomm:
        case gaia_transport_spp:
            transport_registered = transport_type;
            link_cfg.type = transport_mgr_type_rfcomm;
            link_cfg.trans_info.non_gatt_trans.trans_link_id = SPP_DEFAULT_CHANNEL;
            TransportMgrRegisterTransport(&gaia->task_data, &link_cfg);
            break;
#endif

#ifdef GAIA_TRANSPORT_GATT
    case gaia_transport_gatt:
        break;
#endif

    default:
        GAIA_TRANS_DEBUG(("Unknown Gaia transport %d\n", transport_type));
        GAIA_PANIC();
        gaiaTransportCommonSendGaiaStartServiceCfm(transport_type, NULL, FALSE);
        break;
    }

}

void gaiaTransportStartGattServer(uint16 start_handle, uint16 end_handle) 
{
#ifdef GAIA_TRANSPORT_GATT
    gaiaTransportGattRegisterServer(start_handle, end_handle);
#else
    UNUSED(start_handle);
    UNUSED(end_handle);
    GAIA_TRANS_DEBUG(("Gaia GATT transport not enabled\n"));
    GAIA_PANIC();
#endif    
}


/*! @brief Stop Gaia as a server on a given transport.
 */
void gaiaTransportStopService(gaia_transport_type transport_type, uint16 link_id) 
{
    switch (transport_type) 
    {
#if defined GAIA_TRANSPORT_RFCOMM || defined GAIA_TRANSPORT_SPP
        case gaia_transport_rfcomm:
        case gaia_transport_spp:
            TransportMgrDeRegisterTransport(&gaia->task_data, transport_type, link_id);
            break;
#endif

        default:
            GAIA_TRANS_DEBUG(("Unknown Gaia transport %d\n", transport_type));
            GAIA_PANIC();
            break;
    }

}

/*! @brief Transmit a Gaia packet over a given transport.
 */
void gaiaTransportSendPacket(Task task, gaia_transport *transport,
        uint16 length, uint8 *data) 
{
    switch (transport->type) 
    {
        case gaia_transport_none:
            /*  Transport has been cleared down  */
            GAIA_TRANS_DEBUG(("gaia: transport %p gone\n", (void *) transport));
            gaiaTransportCommonSendGaiaSendPacketCfm(transport, data, FALSE);
            break;

#if defined GAIA_TRANSPORT_RFCOMM || defined GAIA_TRANSPORT_SPP 
        case gaia_transport_rfcomm:
        case gaia_transport_spp:
            gaiaTransportStreamSendPacket(task, transport, length, data);
            break;
#endif

#ifdef GAIA_TRANSPORT_GATT
        case gaia_transport_gatt:
            gaiaTransportGattSendPacket(task, transport, length, data);
            break;
#endif
        default:
            GAIA_TRANS_DEBUG(("gaia: unknown transport %d\n", transport->type));
            GAIA_PANIC();
            break;
    }
}

/*! @brief Get the stream source for a given transport.
 */
Source gaiaTransportGetSource(gaia_transport *transport) {
    switch (transport->type) 
    {
        case gaia_transport_none:
            break;

#if defined GAIA_TRANSPORT_RFCOMM || defined GAIA_TRANSPORT_SPP 
        case gaia_transport_rfcomm:
        case gaia_transport_spp:
            return StreamSourceFromSink(gaiaTransportGetSink(transport));
#endif

#ifdef GAIA_TRANSPORT_GATT
    case gaia_transport_gatt:
        return StreamSourceFromSink(gaiaTransportGattGetSink(transport));
#endif


        default:
            GAIA_TRANS_DEBUG(("Unknown Gaia transport %d\n", transport->type));
            GAIA_PANIC();
            break;
    }
    return NULL;
}


/*! @brief Get the stream sink for a given transport.
 */
Sink gaiaTransportGetSink(gaia_transport *transport) {
    switch (transport->type) 
    {
        case gaia_transport_none:
            break;

#if defined GAIA_TRANSPORT_RFCOMM || defined GAIA_TRANSPORT_SPP
        case gaia_transport_rfcomm:
        case gaia_transport_spp:
            return transport->state.spp.sink;
#endif

#ifdef GAIA_TRANSPORT_GATT
    case gaia_transport_gatt:
        return gaiaTransportGattGetSink(transport);
#endif
        default:
            GAIA_TRANS_DEBUG(("Unknown Gaia transport %d\n", transport->type));
            GAIA_PANIC();
            return BAD_SINK;
    }
    return BAD_SINK;
}

/*! @brief Get the id for a given transport.
 */
uint16 gaiaTransportGetId(gaia_transport *transport) {
    switch (transport->type) 
    {
        case gaia_transport_none:
            break;

#if defined GAIA_TRANSPORT_RFCOMM || defined GAIA_TRANSPORT_SPP
        case gaia_transport_rfcomm:
        case gaia_transport_spp:
            return transport->state.spp.rfcomm_channel;
#endif

#ifdef GAIA_TRANSPORT_GATT
    case gaia_transport_gatt:
        return transport->state.gatt.cid;
#endif
        default:
            GAIA_TRANS_DEBUG(("Unknown Gaia transport %d\n", transport->type));
            GAIA_PANIC();
            return BAD_ID;
    }
    return BAD_ID;
}

/*! @brief Get the transport type for a given transport.
 */
gaia_transport_type gaiaTransportGetType(gaia_transport *transport)
{
    return transport->type;
}

/*! @brief Analyse an inbound command packet and process the command
 */
void gaiaTransportProcessPacket(gaia_transport *transport, uint8 *packet) {
    /*  0 bytes  1        2        3        4        5        6        7        8      len+8      len+9
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     *  |  SOF   |VERSION | FLAGS  | LENGTH |    VENDOR ID    |   COMMAND ID    | | PAYLOAD   ...   | | CHECK  |
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     */
    uint8 protocol_version = packet[GAIA_OFFS_VERSION];
    uint8 payload_length = packet[GAIA_OFFS_PAYLOAD_LENGTH];
    uint16 vendor_id = W16(packet + GAIA_OFFS_VENDOR_ID);
    uint16 command_id = W16(packet + GAIA_OFFS_COMMAND_ID);
    uint8 *payload = NULL;

    if(payload_length)
    {
        payload = packet + GAIA_OFFS_PAYLOAD;
    }

    if (protocol_version == GAIA_VERSION)
    {
        gaiaProcessCommand(transport, vendor_id, command_id, payload_length, payload);
    }
}

/*! @brief Deserialise a command from a stream transport and send it for processing
 */
void gaiaTransportProcessSource(gaia_transport *transport)
{
    Source source = gaiaTransportGetSource(transport);
    gaia_transport_type type = transport->type;
    uint16 cid = transport->state.spp.rfcomm_channel;

    if (SourceIsValid(source))
    {
        gaia_transport_process_source_data_t *locals = PanicUnlessMalloc(sizeof(*locals));

        locals->source = source;
        locals->idx = 0;
        locals->expected = GAIA_OFFS_PAYLOAD;
        locals->packet_length = 0;
        locals->data_length = TransportMgrGetAvailableDataSize(type,cid);
        locals->data = (uint8*)TransportMgrReadData(type, cid);
        locals->packet = NULL;
        locals->flags = 0;
        locals->check = 0;

        GAIA_TRANS_DEBUG(("gaia: process_source_data: %d\n", locals->data_length));
#ifdef DEBUG_GAIA_TRANSPORT
        if (locals->data_length == 0)
        GAIA_DEBUG(("gaia: done\n"));
        else
        {
            uint16 i;
            GAIA_DEBUG(("gaia: got"));
            for (i = 0; i < locals->data_length; ++i)
            GAIA_DEBUG((" %02x", locals->data[i]));
            GAIA_DEBUG(("\n"));
        }
#endif

        if (locals->data_length >= GAIA_OFFS_PAYLOAD) {
            while ((locals->idx < locals->data_length)
                    && (locals->packet_length < locals->expected)) {
                if (locals->packet_length > 0) {
                    if (locals->packet_length == GAIA_OFFS_FLAGS)
                        locals->flags = locals->data[locals->idx];

                    else if (locals->packet_length == GAIA_OFFS_PAYLOAD_LENGTH) {
                        locals->expected = GAIA_OFFS_PAYLOAD
                                + locals->data[locals->idx]
                                + ((locals->flags & GAIA_PROTOCOL_FLAG_CHECK) ?
                                        1 : 0);
                        GAIA_TRANS_DEBUG(("gaia: expect %d + %d + %d = %d\n",
                                        GAIA_OFFS_PAYLOAD, locals->data[locals->idx], (locals->flags & GAIA_PROTOCOL_FLAG_CHECK) ? 1 : 0, locals->expected));
                    }

                    locals->check ^= locals->data[locals->idx];
                    ++locals->packet_length;
                }

                else if (locals->data[locals->idx] == GAIA_SOF) {
                    locals->packet = locals->data + locals->idx;
                    locals->packet_length = 1;
                    locals->check = GAIA_SOF;
                }

                ++locals->idx;
            }

            if (locals->packet_length == locals->expected) {
                if (((locals->flags & GAIA_PROTOCOL_FLAG_CHECK) == 0)
                        || (locals->check == 0))
                    gaiaTransportProcessPacket(transport, locals->packet);

                else {
                    GAIA_TRANS_DEBUG(("gaia: bad chk\n"));
                }

                if(!gaia->upgrade_large_data.in_progress)
                {
                    TransportMgrDataConsumed(type, cid, locals->idx);
                    GAIA_TRANS_DEBUG(("SourceDrop (gaia1): %x", locals->idx));
                }
            }

            else if (locals->packet_length == 0) {
                /*  No start-of-frame; drop the lot  */
                GAIA_TRANS_DEBUG(("gaia: no sof\n"));
                if (!gaia->upgrade_large_data.in_progress) {
                    TransportMgrDataConsumed(type, cid, locals->data_length);
                    GAIA_TRANS_DEBUG(("SourceDrop (gaia2): %x", locals->data_length));
                }
            }

            if (locals->idx < locals->data_length) {
                MESSAGE_PMAKE(more, GAIA_INTERNAL_MORE_DATA_T); GAIA_TRANS_DEBUG(("gaia: more: %d < %d\n", locals->idx, locals->data_length));
                more->transport = transport;
                MessageSendLater(&gaia->task_data, GAIA_INTERNAL_MORE_DATA, more,
                        APP_BUSY_WAIT_MILLIS);
            }
        }

        free(locals);
        locals = 0;
    }
}


/*! @brief Handle incoming messages for each transport
 */
bool gaiaTransportHandleMessage(Task task, MessageId id, Message message)
{
    bool msg_handled = TRUE;    /* default position is we've handled the message */

    switch (id)
    {
        case GAIA_INTERNAL_MORE_DATA:
            {
                GAIA_INTERNAL_MORE_DATA_T *m = (GAIA_INTERNAL_MORE_DATA_T *) message;
                GAIA_TRANS_DEBUG(("gaia: GAIA_INTERNAL_MORE_DATA: t=%p\n", (void *) m->transport));
                gaiaTransportProcessSource(m->transport);
            }
            break;

        case TRANSPORT_MGR_MORE_DATA:
            {
                TRANSPORT_MGR_MORE_DATA_T *m = (TRANSPORT_MGR_MORE_DATA_T*) message;
                gaia_transport *t = gaiaTransportFromIdType(m->trans_link_info, m->type);
                GAIA_TRANS_DEBUG(("gaia: TRANSPORT_MGR_MORE_DATA: t=%p\n", (void *) t));
                
                if (t != NULL)
                    gaiaTransportProcessSource(t);
                
                else
                    msg_handled = FALSE;
            }
            break;

        case TRANSPORT_MGR_REGISTER_CFM:
            {
                TRANSPORT_MGR_REGISTER_CFM_T *m = (TRANSPORT_MGR_REGISTER_CFM_T *) message;
                GAIA_TRANS_DEBUG(("gaia: TRANSPORT_MGR_REGISTER_CFM: %d = %d\n", m->link_cfg.trans_info.non_gatt_trans.trans_link_id, m->status));
                
                if (m->status)
                {
                    if(m->link_cfg.type== transport_mgr_type_rfcomm)
                        gaiaTransportSdpRegister(transport_registered, m->link_cfg.trans_info.non_gatt_trans.trans_link_id);
                    else
                        gaiaTransportCommonSendGaiaStartServiceCfm(m->link_cfg.type, NULL, TRUE);
                }
                else
                    gaiaTransportCommonSendGaiaStartServiceCfm(gaia_transport_none, NULL, FALSE);
            }
            break;
        
            
        case CL_SDP_REGISTER_CFM:
            {
                CL_SDP_REGISTER_CFM_T *m = (CL_SDP_REGISTER_CFM_T *) message;
                GAIA_TRANS_DEBUG(("gaia: CL_SDP_REGISTER_CFM: %d\n", m->status));
                
                if (m->status == sds_status_success)
                {
                    if (gaia->spp_sdp_handle == 0)
                        gaiaTransportCommonSendGaiaStartServiceCfm(transport_registered, NULL, TRUE);
                    
                    gaia->spp_sdp_handle = m->service_handle;
                }
                
                else
                    gaiaTransportCommonSendGaiaStartServiceCfm(transport_registered, NULL, FALSE);
            }
            break;

        case CL_SDP_UNREGISTER_CFM:
            {
                CL_SDP_UNREGISTER_CFM_T *m = (CL_SDP_UNREGISTER_CFM_T *) message;
                GAIA_TRANS_DEBUG(("gaia: CL_SDP_UNREGISTER_CFM: %d\n", m->status));
                if (m->status == sds_status_success)
                {
                    /* Reset sdp handle */
                    gaia->spp_sdp_handle = 0;
                    
                    /*  Get another channel from the pool  */
                    gaiaTransportStartService(transport_registered);
                }
            }
            break;
            
        case TRANSPORT_MGR_LINK_CREATED_CFM:
            {
                TRANSPORT_MGR_LINK_CREATED_CFM_T *m = (TRANSPORT_MGR_LINK_CREATED_CFM_T *) message;
                gaia_transport *transport = gaiaTransportFindFree();

                GAIA_TRANS_DEBUG(("gaia: TRANSPORT_MGR_LINK_CREATED_CFM\n"));
                
                if (transport == NULL)
                {
                    GAIA_TRANS_DEBUG(("gaia: Disconnecting Transport as no free GAIA transport slot\n"));
                    TransportMgrDisconnect(m->link_cfg.type, m->trans_sink);
                }
                else
                {
                    if(m->status)
                    {
                        if(m->link_cfg.type== transport_mgr_type_rfcomm)
                        {
                            transport->type = gaia_transport_rfcomm;
                            transport->state.spp.sink = m->trans_sink;
                            transport->state.spp.rfcomm_channel = m->link_cfg.trans_info.non_gatt_trans.trans_link_id;

                            /* Unregister service record from SDP until RFCOMM/SPP link is disconnected */
                            ConnectionUnregisterServiceRecord(task, gaia->spp_sdp_handle);

                        }
                        transport->connected = TRUE;
                        transport->enabled = TRUE;

                        /* Send Connection Indication to application */
                        gaiaTransportCommonSendGaiaConnectInd(transport, TRUE);
                    }                    
                    else
                    {
                        transport->type = gaia_transport_none;
                        gaiaTransportCommonSendGaiaConnectInd(transport, FALSE);
                    }
                }
            }
            break;

        case TRANSPORT_MGR_LINK_DISCONNECTED_CFM:
            {
                TRANSPORT_MGR_LINK_DISCONNECTED_CFM_T *m = (TRANSPORT_MGR_LINK_DISCONNECTED_CFM_T *) message;
                gaia_transport *transport = gaiaTransportFromSink(m->trans_sink);
                
                GAIA_TRANS_DEBUG(("gaia: TRANSPORT_MGR_LINK_DISCONNECTED_CFM: %d\n", m->status));

                if(transport != NULL && m->status)
                {
                    if(transport->type == gaia_transport_rfcomm)
                    {
                        /* Release channel for re-use  */
                        gaiaTransportStopService(transport_mgr_type_rfcomm, transport->state.spp.rfcomm_channel);
                    }

                    gaiaTransportCommonSendGaiaDisconnectCfm(transport);
                    gaiaTransportFlushInput(transport);
                    /* Free up transport */
                    gaiaTransportTidyUpOnDisconnection(transport);
                }
                else if(transport == NULL)
                {
                    GAIA_TRANS_DEBUG(("gaia: TRANSPORT_MGR_LINK_DISCONNECTED_CFM: NULL tranport ptr, no GAIA transport to tidy.\n"));
                }
            }
            break;

        case TRANSPORT_MGR_DEREGISTER_CFM:
            {
                GAIA_TRANS_DEBUG(("gaia: TRANSPORT_MGR_DERREGISTER_CFM\n"));
            }
            break;

#ifdef GAIA_HAVE_SPP_CLIENT
        case SPP_MESSAGE_MORE_DATA:
            {
                SPP_MESSAGE_MORE_DATA_T *m = (SPP_MESSAGE_MORE_DATA_T *) message;
                gaia_transport *t = gaiaTransportFromSink(StreamSinkFromSource(m->source));
                GAIA_TRANS_DEBUG(("gaia: SPP_MESSAGE_MORE_DATA: t=%04x\n", (uint16) t));

                if (t && (t->type == gaia_transport_spp))
                    gaiaTransportProcessSource(t);
                
                else
                    msg_handled = FALSE;
            }
            break;

            
        case SPP_CLIENT_CONNECT_CFM:
            {
                SPP_CLIENT_CONNECT_CFM_T *m = (SPP_CLIENT_CONNECT_CFM_T *) message;
                bool success = m->status == spp_connect_success;

                GAIA_TRANS_DEBUG(("gaia: SPP_CLIENT_CONNECT_CFM: %u\n", m->status));

                if (m->status != spp_connect_pending)
                {
                    gaia_transport *transport = gaiaTransportFromRfcommChannel(0);

                    if (transport != NULL)
                    {
                        transport->enabled = TRUE;
                        transport->connected = success;
                        transport->state.spp.sink = m->sink;
                    }

                    gaiaTransportCommonSendGaiaConnectCfm(transport, success);
                }
            }
            break;

            case SPP_DISCONNECT_IND:
            {
                SPP_DISCONNECT_IND_T *m = (SPP_DISCONNECT_IND_T *) message;
                gaia_transport *transport = gaiaTransportFromSink(m->sink);

                SppDisconnectResponse(m->spp);
                gaiaTransportCommonSendGaiaDisconnectInd(transport);
            }
            break;
#endif /* GAIA_HAVE_SPP_CLIENT */

#if defined GAIA_TRANSPORT_GATT
            case MESSAGE_MORE_DATA:
            {
                gaia_transport *t = gaia->upgrade_transport;
                msg_handled = FALSE;
                
                GAIA_TRANS_DEBUG(("gaia: MESSAGE_MORE_DATA\n"));
                if (t && (t->type == gaia_transport_gatt))
                {
                    while(gaiaTransportGattProcessSource());
                    msg_handled = TRUE;
                }
            }
            break;
#endif

        default:
            {
                /* indicate we couldn't handle the message */
                GAIA_DEBUG(("gaia: unh 0x%04X\n", id));
                msg_handled = FALSE;
            }
            break;
    }

    return msg_handled;
}


/*! @brief Clean transport state on disconnection.
 */
void gaiaTransportTidyUpOnDisconnection(gaia_transport *transport)
{
    gaiaTransportNotifyClientsOfDisconnection(transport);
    gaiaTransportDropState(transport);
}

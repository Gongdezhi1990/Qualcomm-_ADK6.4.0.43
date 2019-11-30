/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_rfcomm.c

DESCRIPTION
    Ama RFCOMM functions
*/

#include <bdaddr.h>
#include <connection.h>
#include <message.h>
#include <panic.h>
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
#include "ama_speech.h"
#include "ama.h"
#include "ama_transport.h"
#include "ama_debug.h"
#include "ama_rfcomm.h"
#include "ama_private.h"
#include "ama_state.h"
#include "transport_manager.h"

#define AMA_RFCOMM_CHANNEL 19

static const uint8 ama_rfcomm_service_record_search_pattern[] =
{
    /* DataElSeq 17 bytes */
    0x35,
    0x11,
        /* 16 byte uuid      931C7E8A-540F-4686-B798-E8DF0A2AD9F7 */
        0x1c,
        0x93, 0x1c, 0x7e, 0x8a, 0x54, 0x0f, 0x46, 0x86,
        0xb7, 0x98, 0xe8, 0xdf, 0x0a, 0x2a, 0xd9, 0xf7
};


static const uint8 ama_rfcomm_sdp_search_attribute[] =
{
    /* DataElSeq, 3 bytes */
    0x35,
    0x03,
        /* ProtocolDescriptorList(0x0004) */
        0x09,
            0x00, 0x04
};



static const uint8 ama_rfcomm_service_record[] =
{
    /* ServiceClassIDList(0x0001) */
    0x09,                                   /*       #define ATTRIBUTE_HEADER_16BITS   0x09 */
        0x00, 0x01,
    /* DataElSeq 17 bytes */
    0x35,         /*  #define DATA_ELEMENT_SEQUENCE  0x30    ,    #define DE_TYPE_SEQUENCE       0x01     #define DE_TYPE_INTEGER        0x03 */
    0x11,        /*   size  */
        /* 16 byte uuid      931C7E8A-540F-4686-B798-E8DF0A2AD9F7 */
        0x1c,
        0x93, 0x1c, 0x7e, 0x8a, 0x54, 0x0f, 0x46, 0x86,
        0xb7, 0x98, 0xe8, 0xdf, 0x0a, 0x2a, 0xd9, 0xf7,
    /* ProtocolDescriptorList(0x0004) */
    0x09,
        0x00, 0x04,
    /* DataElSeq 12 bytes */
    0x35,
    0x0c,
        /* DataElSeq 3 bytes */
        0x35,
        0x03,
            /* uuid L2CAP(0x0100) */
            0x19,
            0x01, 0x00,
        /* DataElSeq 5 bytes */
        0x35,
        0x05,
            /* uuid RFCOMM(0x0003) */
            0x19,
            0x00, 0x03,
            /* uint8 RFCOMM_DEFAULT_CHANNEL */
            0x08,
                AMA_RFCOMM_CHANNEL
};

#define RFCOMM_CHANNEL_INVALID 0xFF

typedef struct
{
    TaskData    rfcomm_task;
    Task        ama_task;
    bdaddr      peer_bd_addr;
    Sink        rfcomm_sink;
    uint8       rfcomm_channel;          /* local */
    uint32      service_handle;
    bool        rfcomm_client;
}ama_rfcomm_data_t;


static ama_rfcomm_data_t ama_rfcomm_data;


static void amaRfCommMessageHandler(Task task, MessageId id, Message message);
static void amaSdpServiceSearchCfm(CL_SDP_SERVICE_SEARCH_CFM_T* cfm);
void amaSdpServiceRequest( bdaddr *bd_addr);
static void amaSdpRegisterCfm(CL_SDP_REGISTER_CFM_T * cfm);



bdaddr* amaGetRfCommPeerAddress(void)
{
    return &ama_rfcomm_data.peer_bd_addr;
}


bool AmaRfCommInit(Task task,  bdaddr *bd_addr)
{
    AMA_DEBUG(("AmaRfCommInit \n"));

    /* initialise message handler */
    memset(&ama_rfcomm_data, 0, sizeof(ama_rfcomm_data) );
    ama_rfcomm_data.rfcomm_task.handler = amaRfCommMessageHandler;
    ama_rfcomm_data.ama_task = task;

    ama_rfcomm_data.rfcomm_channel = AMA_RFCOMM_CHANNEL;

    if(NULL != bd_addr)
        ama_rfcomm_data.peer_bd_addr = *bd_addr;

    ConnectionReadLocalAddr(&ama_rfcomm_data.rfcomm_task);

    transport_mgr_link_cfg_t linkCfg;
    linkCfg.type = transport_mgr_type_rfcomm;
    linkCfg.trans_info.non_gatt_trans.trans_link_id = ama_rfcomm_data.rfcomm_channel;

    if(transport_mgr_status_success != TransportMgrRegisterTransport(ama_rfcomm_data.ama_task, &linkCfg) )
        return FALSE;

    return TRUE;
}

void amaRegisterRfCommSdp(uint16 trans_link_id)
{
    uint8 *service_record;
    
    ama_rfcomm_data.rfcomm_channel = trans_link_id;
    if (trans_link_id == AMA_RFCOMM_CHANNEL)
    {
        service_record = (uint8 *) ama_rfcomm_service_record;
    }
    else
    {
        service_record = PanicUnlessMalloc(sizeof ama_rfcomm_service_record);
        memmove(service_record, ama_rfcomm_service_record, sizeof ama_rfcomm_service_record);
        service_record[sizeof ama_rfcomm_service_record - 1] = trans_link_id;
    }

    ConnectionRegisterServiceRecord(&ama_rfcomm_data.rfcomm_task, sizeof ama_rfcomm_service_record, service_record);
}

void amaRfcommLinkCreatedCfm(TRANSPORT_MGR_LINK_CREATED_CFM_T *cfm)
{
    AMA_DEBUG(("TRANSPORT_MGR_LINK_CREATED_CFM sts=%d typ=%d lnk=%d snk=%p\n", cfm->status, cfm->link_cfg.type, cfm->link_cfg.trans_info.non_gatt_trans.trans_link_id, (void *) cfm->trans_sink));

    if (cfm->link_cfg.trans_info.non_gatt_trans.trans_link_id == ama_rfcomm_data.rfcomm_channel)
    {
        ama_rfcomm_data.rfcomm_sink = cfm->trans_sink;

        if(AmaTransportGet() == ama_transport_ble)
        {
            AMA_DEBUG(("AMA Implicit upgrade to RFCOMM\n"));
            AmaTransportSwitch(ama_transport_rfcomm);
            ama_rfcomm_data.peer_bd_addr = cfm->addr.taddr.addr;
        }

        amaSendSinkMessage(AMA_SEND_TRANSPORT_VERSION_ID, NULL);

    }
    else
    {
        Panic();
    }
}

void amaRfcommLinkDisconnectedCfm(TRANSPORT_MGR_LINK_DISCONNECTED_CFM_T *cfm)
{
    AMA_DEBUG(("TRANSPORT_MGR_LINK_DISCONNECTED_CFM\n"));

    if (cfm->trans_sink == ama_rfcomm_data.rfcomm_sink)
    {
        MessageStreamTaskFromSink(ama_rfcomm_data.rfcomm_sink, NULL);
        ama_rfcomm_data.rfcomm_sink = NULL;
        BdaddrSetZero(&ama_rfcomm_data.peer_bd_addr);
    }
}


static void amaSdpRegisterCfm(CL_SDP_REGISTER_CFM_T * cfm)
{
    AMA_DEBUG(("SDP cfm %d\n", cfm->status));

    if( cfm->status == sds_status_success)
    {
        ama_rfcomm_data.service_handle = cfm->service_handle;
    }
}

/* **************   SDP    ******************* */
void amaSdpServiceRequest( bdaddr *bd_addr)
{
    ama_rfcomm_data.peer_bd_addr = *bd_addr; /* maybe another data member to be more clear .... */

    AMA_DEBUG(("AMA SDP ServiceRequest"));

    ConnectionSdpServiceSearchAttributeRequest(&ama_rfcomm_data.rfcomm_task, bd_addr , 32,
        sizeof(ama_rfcomm_service_record_search_pattern), ama_rfcomm_service_record_search_pattern,
        sizeof(ama_rfcomm_sdp_search_attribute), ama_rfcomm_sdp_search_attribute);
}


static void amaSdpServiceSearchCfm(CL_SDP_SERVICE_SEARCH_CFM_T * cfm)
{
    uint16 i;

    AMA_DEBUG(("AMA SDP_SERVICE_SEARCH_CFM %u n %d e %d s %d ---- ", cfm->status, cfm->num_records, cfm->error_code, cfm->size_records));

    for(i=0; i<cfm->size_records;i++)
    {
        AMA_DEBUG((" %x", cfm->records[i]));
    }
    AMA_DEBUG(("\n"));
}


bool AmaRFCommSendData(uint8* data, uint16 length)
{
#define BAD_SINK_CLAIM (0xFFFF)

    bool status = FALSE;

    if( ama_rfcomm_data.rfcomm_sink)
    {
        Sink sink =  ama_rfcomm_data.rfcomm_sink;
        AMA_DEBUG(("RS %d\n",SinkSlack(sink)));
        if(SinkClaim(sink, length) != BAD_SINK_CLAIM)
        {
            uint8 *sink_data = SinkMap(sink);
            memmove(sink_data, data, length);
            status = SinkFlush(sink, length);

            if(status)
            {
                AMA_DEBUG(("!"));
            }
            else
            {
                AMA_DEBUG(("#"));
            }
        }
        else
        {
            AMA_DEBUG(("*"));
        }
    }
    return status;
}


static void amaRfCommMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    AMA_DEBUG(("AMA AmaRfCommMessageHandler id %x \n", id));

    switch (id)
    {
        case CL_DM_LOCAL_BD_ADDR_CFM:
            {
                AMA_DEBUG(("AMA CL_DM_LOCAL_BD_ADDR_CFM id=%04x\n"));
                bdaddr* bd_addr = &((CL_DM_LOCAL_BD_ADDR_CFM_T *)message)->bd_addr;
                amaStoreLocalAddress(bd_addr);
            }
            break;
        /*                      SDP                          */
        case CL_SDP_REGISTER_CFM:
            AMA_DEBUG(("AMA CL_SDP_REGISTER_CFM id=%04x\n",CL_SDP_REGISTER_CFM));
            amaSdpRegisterCfm((CL_SDP_REGISTER_CFM_T *) message);
            break;

        case CL_SDP_SERVICE_SEARCH_CFM:
            AMA_DEBUG(("AMA CL_SDP_SERVICE_SEARCH_CFM\n"));
            amaSdpServiceSearchCfm((CL_SDP_SERVICE_SEARCH_CFM_T *) message);
            break;

        case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
            AMA_DEBUG(("AMA CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM\n"));
            break;

        case CL_SDP_SERVICE_SEARCH_REF_CFM:
        case CL_SDP_ATTRIBUTE_SEARCH_CFM:
        case CL_SDP_ATTRIBUTE_SEARCH_REF_CFM:
        case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_REF_CFM:
            AMA_DEBUG(("AMA unknown SDP id %d %d\n", CL_SDP_REGISTER_CFM,  id));
            break;

        default:
            AMA_DEBUG(("AMA rfCommMessageHandler unknown message=%x\n", id));
            break;

    }

}



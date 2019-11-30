/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_connect.c
DESCRIPTION
    This file defines all APIs for Connection management. The iAP2 library allows
	multiple device connections at a time, but it is the applications responsibility
    to maintain those connections and control the optimal number of maximum connections.
*/

#include <connection_no_ble.h>
#include <vm.h>
#include <region.h>
#include <service.h>
#include <stream.h>
#include <sink.h>
#include <source.h>
#include <vmal.h>
#include <message.h>
#include "iap2_connect.h"
#include "iap2_link.h"
#include "iap2_packet.h"

static const rfcomm_config_params iap2_rfcomm_config_params =
{
    RFCOMM_DEFAULT_PAYLOAD_SIZE,
    RFCOMM_DEFAULT_MODEM_SIGNAL,
    RFCOMM_DEFAULT_BREAK_SIGNAL,
    RFCOMM_DEFAULT_MSC_TIMEOUT
};

static const uint8 iap2_sdp_search_pattern[] =
{
    /* DataElSeq 17 bytes */
    0x35,
    0x11,
        /* 16 byte uuid  0x00000000decafadedecadeafdecacafe */
        0x1c,
        0x00, 0x00, 0x00, 0x00, 0xde, 0xca, 0xfa, 0xde,
        0xde, 0xca, 0xde, 0xaf, 0xde, 0xca, 0xca, 0xfe
};

static const uint8 iap2_sdp_search_attribute[] =
{
    /* DataElSeq, 3 bytes */
    0x35,
    0x03,
        /* ProtocolDescriptorList(0x0004) */
        0x09,
            0x00, 0x04
};

static bool iap2LinkParamValidate(const iap2_link_param *param)
{
    if (!param)
        return FALSE;

    if (param->MaxNumOutstandingPackets < 1 ||
        param->MaxNumOutstandingPackets > 127)
        return FALSE;

    if (param->MaxRxPacketLength < 24)
        return FALSE;

    if (param->RetransmissionTimeout < 20)
        return FALSE;

    if (param->CumulativeAckTimeout < 10 ||
        param->CumulativeAckTimeout > (param->RetransmissionTimeout >> 1))
        return FALSE;

    if (param->MaxNumRetransmissions < 1 ||
        param->MaxNumRetransmissions > 30)
        return FALSE;

    if (param->MaxCumulativeAcks > MIN(127, param->MaxNumOutstandingPackets))
        return FALSE;

    return TRUE;
}

static bool iap2ServiceRecordFindRfcommServerChannel(const uint8 size_service_record, const uint8* service_record, Region *value)
{
    ServiceDataType type;
    Region record;
    Region protocols;
    Region protocol;

    record.begin = service_record;
    record.end   = service_record + size_service_record;

    while (ServiceFindAttribute(&record, saProtocolDescriptorList, &type, &protocols))
    {
        if (type == sdtSequence)
        {
            while (ServiceGetValue(&protocols, &type, &protocol))
            {
                if (type == sdtSequence &&
                    ServiceGetValue(&protocol, &type, value) &&
                    type == sdtUUID &&
                    RegionMatchesUUID32(value, UUID_RFCOMM) &&
                    ServiceGetValue(&protocol, &type, value) &&
                    type == sdtUnsignedInteger)
                {
                    /* found the server channel field */
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

void Iap2ConnectRequest(bdaddr *bd_addr, uint16 timeout, const iap2_link_param *param)
{
    iap2_link *link = Iap2GetLinkForBdaddr(bd_addr);

    DEBUG_PRINT_CONNECT(("iAP2 connect request for %02X:%02X:%02X:%02X:%02X:%02X\n",
                 (uint8)(bd_addr->nap >> 8) & 0xff,
                 (uint8)(bd_addr->nap) & 0xff,
                 (uint8)(bd_addr->uap) & 0xff,
                 (uint8)(bd_addr->lap >> 16) & 0xff,
                 (uint8)(bd_addr->lap >> 8) & 0xff,
                 (uint8)(bd_addr->lap) & 0xff));

    if (link)
    {
        if (link->state >= iap2_link_initialised)
        {
            /* Link already existed, notify app */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_link_already_existed);

            DEBUG_PRINT_CONNECT(("iAP2 link already existed link=0x%p state=%d\n", (void*)link, link->state));

            msg->link = link;
            msg->bd_addr = *bd_addr;
            msg->sink = link->sink;
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
        }
    }
    else
    {
        /* No link to the host now, check the link parameters first */
        if (!iap2LinkParamValidate(param))
        {
            /* Link parameters not valid, notify the app */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_invalid_param);

            DEBUG_PRINT_CONNECT(("iAP2 link param invalid\n"));

            msg->link = NULL;
            msg->bd_addr = *bd_addr;
            msg->sink = NULL;
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
        }
        else
        {
            /* Allocate the link */
            link = iap2LinkAllocate(iap2_transport_bluetooth);

            if (!link)
            {
                /* Link allocate failed, out of resources */
                MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_out_of_resources);

                DEBUG_PRINT_CONNECT(("iAP2 link allocate failed\n"));

                msg->link = NULL;
                msg->bd_addr = *bd_addr;
                msg->sink = NULL;
                MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
            }
            else
            {
                DEBUG_PRINT_CONNECT(("iAP2 link allocated link=0x%p\n", (void*)link));

                /* We have a new link instance */
                link->remote_addr = *bd_addr;
                link->state = iap2_link_connecting_local;
                link->param = *param;

                /* Schedule the SDP search */
                MessageSend(&link->task, IAP2_INTERNAL_CONNECT_SDP_SEARCH_REQ, NULL);

                /* Schedule a connect timeout message */
                MessageSendLater(&link->task, IAP2_INTERNAL_CONNECT_TIMEOUT, NULL, MAX(timeout, IAP2_CONNECT_RETRY_DELAY - 1));
            }
        }
    }
}

void Iap2ConnectCancelRequest(bdaddr *bd_addr)
{
    /* Find the link */
    iap2_link *link = Iap2GetLinkForBdaddr(bd_addr);

    DEBUG_PRINT_CONNECT(("iAP2 connect cancel request link=0x%p\n", (void*)link));

    if (link && link->state == iap2_link_connecting_local)
    {
        DEBUG_PRINT_CONNECT(("iAP2 link connect request canceled\n"));
        iap2LinkDeallocate(link);
    }
}

void Iap2ConnectResponse(iap2_link *link, bool accept, const iap2_link_param *param)
{
    if (iap2LinkValidate(link) && link->state == iap2_link_connecting_remote && link->sink)
    {
        DEBUG_PRINT_CONNECT(("iAP2 connect response link=0x%p accept=%d\n", (void*)link, accept));

        if (!accept)
        {
            /* Reject the link if the app rejects it */
            ConnectionRfcommConnectResponse(&link->task,
                                            FALSE,
                                            link->sink,
                                            iap2_lib->server_channel,
                                            &iap2_rfcomm_config_params);
            iap2LinkDeallocate(link);
        }
        else if (!iap2LinkParamValidate(param))
        {
            /* Link parameters not valid, reject the link and notify the app */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_invalid_param);

            DEBUG_PRINT_CONNECT(("iAP2 link param invalid\n"));

            msg->link = NULL;
            msg->bd_addr = link->remote_addr;
            msg->sink = NULL;
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);

            ConnectionRfcommConnectResponse(&link->task,
                                            FALSE,
                                            link->sink,
                                            iap2_lib->server_channel,
                                            &iap2_rfcomm_config_params);
            iap2LinkDeallocate(link);
        }
        else
        {
            /* Store the link parameters and accept the link */
            link->param = *param;

            ConnectionRfcommConnectResponse(&link->task,
                                            TRUE,
                                            link->sink,
                                            iap2_lib->server_channel,
                                            &iap2_rfcomm_config_params);
        }
    }
}

void Iap2DisconnectRequest(iap2_link *link)
{
    DEBUG_PRINT_CONNECT(("iAP2 disconnect request link=0x%p\n", (void*)link));

    if (iap2LinkValidate(link))
    {
        if (link->sink)
        {
            /* Disconnect the sink first */
            DEBUG_PRINT_CONNECT(("iAP2 disconnect RFCOMM sink=0x%p\n", (void*)link->sink));

            /* Disconnect the streams */
            StreamDisconnect(NULL, link->sink);
            StreamDisconnect(StreamSourceFromSink(link->sink), NULL);

            link->state = iap2_link_disconnecting;
            ConnectionRfcommDisconnectRequest(&link->task, link->sink);
        }
        else
        {
            /* No sink, notify the app */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_DISCONNECT_CFM, iap2_status_success);
            msg->link = link;
            msg->bd_addr = link->remote_addr;
            iap2LinkDeallocate(link);
            MessageSend(iap2_lib->app_task, IAP2_DISCONNECT_CFM, msg);
        }
    }
    else
        DEBUG_PRINT_CONNECT(("iAP2 disconnect request link not found\n"));
}

void iap2HandleClSdpServiceSearchAttributeCfm(CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
    iap2_link *link = Iap2GetLinkForBdaddr(&cfm->bd_addr);

    if (link && link->state == iap2_link_connecting_local)
    {
        Region channel;

        DEBUG_PRINT_CONNECT(("iAP2 SDP search confirm link=0x%p status=%d\n", (void*)link, cfm->status));

        if (cfm->status == sdp_response_success &&
            iap2ServiceRecordFindRfcommServerChannel(cfm->size_attributes, cfm->attributes, &channel))
        {
            DEBUG_PRINT_CONNECT(("iAP2 RFCOMM connect request channel=%d\n", (uint8)RegionReadUnsigned(&channel)));
            /* SDP search succeeded, locate the link and and find the RFCOMM channel */
            ConnectionRfcommConnectRequest(&link->task,
                                           &cfm->bd_addr,
                                           iap2_lib->server_channel,
                                           RegionReadUnsigned(&channel),
                                           &iap2_rfcomm_config_params);
        }
        else
        {
            /* We have a security issue, deallocate the link and notify the app */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_fail);

            DEBUG_PRINT_CONNECT(("iAP2 SDP search failed \n"));

            msg->link = NULL;
            msg->bd_addr = cfm->bd_addr;
            msg->sink = NULL;
            iap2LinkDeallocate(link);
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
        }
    }
}

void iap2HandleClRfcommConnectInd(CL_RFCOMM_CONNECT_IND_T *ind)
{
    iap2_link *link = Iap2GetLinkForBdaddr(&ind->bd_addr);

    DEBUG_PRINT_CONNECT(("iAP2 RFCOMM connect ind link=0x%p channel=%d addr=%02X:%02X:%02X:%02X:%02X:%02X\n",
                 (void*)link,
                 ind->server_channel,
                 (uint8)(ind->bd_addr.nap >> 8) & 0xff,
                 (uint8)(ind->bd_addr.nap) & 0xff,
                 (uint8)(ind->bd_addr.uap) & 0xff,
                 (uint8)(ind->bd_addr.lap >> 16) & 0xff,
                 (uint8)(ind->bd_addr.lap >> 8) & 0xff,
                 (uint8)(ind->bd_addr.lap) & 0xff));

    if (link)
    {
        /* We have a link to the same host, reject it anyway */
        DEBUG_PRINT_CONNECT(("iAP2 link already existed state=%d\n", link->state));
        ConnectionRfcommConnectResponse(&iap2_lib->task, FALSE, ind->sink, iap2_lib->server_channel, NULL);
    }
    else if (ind->server_channel == iap2_lib->server_channel)
    {
        /* No link to the host now, create one */
        link = iap2LinkAllocate(iap2_transport_bluetooth);

        if (!link)
        {
            /* Link allocate failed, reject the connect ind */
            DEBUG_PRINT_CONNECT(("iAP2 link allocate failed\n"));
            ConnectionRfcommConnectResponse(&iap2_lib->task, FALSE, ind->sink, iap2_lib->server_channel, NULL);
        }
        else
        {
            MAKE_MESSAGE(msg, IAP2_CONNECT_IND);

            DEBUG_PRINT_CONNECT(("iAP2 link allocated link=0x%p\n", (void*)link));

            /* We have a new link instance */
            link->remote_addr = ind->bd_addr;
            link->sink = ind->sink;
            link->state = iap2_link_connecting_remote;

            /* Notify the app */
            msg->link = link;
            msg->bd_addr = link->remote_addr;
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_IND, msg);
        }
    }
}

void iap2HandleInternalConnectTimeout(iap2_link *link)
{
    if (link->state == iap2_link_connecting_local)
    {
        /* We have a timeout, deallocate the link and notify the app */
        MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_timeout);

        DEBUG_PRINT_CONNECT(("iAP2 connect timeout link=0x%p\n", (void*)link));

        msg->link = NULL;
        msg->bd_addr = link->remote_addr;
        msg->sink = NULL;
        iap2LinkDeallocate(link);
        MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
    }
}

void iap2HandleInternalConnectSdpSearchReq(iap2_link *link)
{
    if (link->state == iap2_link_connecting_local)
    {
        DEBUG_PRINT_CONNECT(("iAP2 connect SDP search link=0x%p\n", (void*)link));

        /* Perform a SDP search */
        /* Route the confirmation to lib task in case we have a connection timetout */
        ConnectionSdpServiceSearchAttributeRequest(&iap2_lib->task,
                                                   &link->remote_addr,
                                                   0x40,
                                                   sizeof(iap2_sdp_search_pattern),
                                                   iap2_sdp_search_pattern,
                                                   sizeof(iap2_sdp_search_attribute),
                                                   iap2_sdp_search_attribute);
    }
}

void iap2HandleClRfcommClientConnectCfm(iap2_link *link, const CL_RFCOMM_CLIENT_CONNECT_CFM_T *cfm)
{
    DEBUG_PRINT_CONNECT(("iAP2 RFCOMM client connect cfm link=0x%p status=%d\n", (void*)link, cfm->status));

    if (link->state == iap2_link_connecting_local)
    {
        if (cfm->status == rfcomm_connect_success)
        {
            DEBUG_PRINT_CONNECT(("iAP2 RFCOMM client connected\n"));

            link->state = iap2_link_connecting_init;
            link->sink = cfm->sink;
            MessageStreamTaskFromSink(cfm->sink, &link->task);

            /* Configure sink not to send MESSAGE_MORE_SPACE */
            SinkConfigure(cfm->sink, VM_SINK_MESSAGES, VM_MESSAGES_NONE);

            /* Configure source to only send one outstanding MESSAGE_MORE_DATA */
            SourceConfigure(StreamSourceFromSink(cfm->sink), VM_SOURCE_MESSAGES, VM_MESSAGES_SOME);

            MessageSend(&link->task, IAP2_INTERNAL_LINK_INIT_REQ, NULL);
        }
        else if (cfm->status == rfcomm_connect_rejected_security ||
                 cfm->status == l2cap_connect_failed_security)
        {
            /* We have a security issue, deallocate the link and notify the app */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_security_fail);

            DEBUG_PRINT_CONNECT(("iAP2 RFCOMM connect failed security\n"));

            msg->link = NULL;
            msg->bd_addr = cfm->addr;
            msg->sink = NULL;
            iap2LinkDeallocate(link);
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
        }
        else if (cfm->status != rfcomm_connect_pending)
        {
            /* We have a general failure, reschedule the SDP search */
            DEBUG_PRINT_CONNECT(("iAP2 SDP search retry\n"));
            MessageSendLater(&link->task, IAP2_INTERNAL_CONNECT_SDP_SEARCH_REQ, NULL, IAP2_CONNECT_RETRY_DELAY);
        }
    }
}

void iap2HandleClRfcommServerConnectCfm(iap2_link *link, const CL_RFCOMM_SERVER_CONNECT_CFM_T *cfm)
{
    DEBUG_PRINT_CONNECT(("iAP2 RFCOMM server connect cfm link=0x%p status=%d\n", (void*)link, cfm->status));

    if (link->state == iap2_link_connecting_remote)
    {
        if (cfm->status == rfcomm_connect_success)
        {
            DEBUG_PRINT_CONNECT(("iAP2 RFCOMM server connected\n"));

            link->state = iap2_link_connecting_init;
            link->sink = cfm->sink;
            MessageStreamTaskFromSink(cfm->sink, &link->task);

            /* Configure sink not to send MESSAGE_MORE_SPACE */
            SinkConfigure(cfm->sink, VM_SINK_MESSAGES, VM_MESSAGES_NONE);

            /* Configure source to only send one outstanding MESSAGE_MORE_DATA */
            SourceConfigure(StreamSourceFromSink(cfm->sink), VM_SOURCE_MESSAGES, VM_MESSAGES_SOME);

            MessageSend(&link->task, IAP2_INTERNAL_LINK_INIT_REQ, NULL);
        }
        else if (cfm->status != rfcomm_connect_pending)
        {
            DEBUG_PRINT_CONNECT(("iAP2 RFCOMM server connect failed\n"));
            /* We have a general failure, deallocate the link */
            iap2LinkDeallocate(link);
        }
    }

}

void iap2HandleClRfcommDisconnectInd(iap2_link *link, const CL_RFCOMM_DISCONNECT_IND_T *ind)
{
    DEBUG_PRINT_CONNECT(("iAP2 RFCOMM disconnect ind link=0x%p state=%d status=%d\n", (void*)link, link->state, ind->status));

    if (link->state >= iap2_link_ident_start)
    {
        /* Notify the app with the disconnected link */
        MAKE_MESSAGE(msg, IAP2_DISCONNECT_IND);

        /* Disconnect the streams */
        StreamDisconnect(NULL, ind->sink);
        StreamDisconnect(StreamSourceFromSink(ind->sink), NULL);

        msg->link = link;
        msg->bd_addr = link->remote_addr;

        if (ind->status == rfcomm_disconnect_l2cap_link_loss)
            msg->status = iap2_status_link_loss;
        else if (ind->status == rfcomm_disconnect_success ||
                 ind->status == rfcomm_disconnect_normal_disconnect)
            msg->status = iap2_status_success;
        else
            msg->status = iap2_status_fail;

        iap2RemoveLinkFromSessionAndMessage(link);

        MessageSend(iap2_lib->app_task, IAP2_DISCONNECT_IND, msg);
    }

    ConnectionRfcommDisconnectResponse(ind->sink);
    iap2LinkDeallocate(link);
}

void iap2HandleClRfcommDisconnectCfm(iap2_link *link, const CL_RFCOMM_DISCONNECT_CFM_T *cfm)
{
    if (link->state >= iap2_link_ident_start)
    {
        /* Notify the app with the disconnected link */
        MAKE_MESSAGE(msg, IAP2_DISCONNECT_CFM);

        DEBUG_PRINT_CONNECT(("iAP2 RFCOMM disconnect cfm link=0x%p status=%d\n", (void*)link, cfm->status));

        /* Disconnect the streams */
        StreamDisconnect(NULL, cfm->sink);
        StreamDisconnect(StreamSourceFromSink(cfm->sink), NULL);

        msg->link = link;
        msg->bd_addr = link->remote_addr;

        if (cfm->status == rfcomm_disconnect_l2cap_link_loss)
            msg->status = iap2_status_link_loss;
        else if (cfm->status == rfcomm_disconnect_success ||
                 cfm->status == rfcomm_disconnect_normal_disconnect)
            msg->status = iap2_status_success;
        else
            msg->status = iap2_status_fail;

        iap2RemoveLinkFromSessionAndMessage(link);

        MessageSend(iap2_lib->app_task, IAP2_DISCONNECT_CFM, msg);
    }
    else
    {
        /* We are in iap2_link_connecting_init, notify the app with connect fail */
        MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_fail);
        msg->link = NULL;
        msg->bd_addr = link->remote_addr;
        msg->sink = NULL;
        MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
    }

    iap2LinkDeallocate(link);
}


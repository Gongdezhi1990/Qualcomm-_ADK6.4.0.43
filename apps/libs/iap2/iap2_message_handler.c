/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_message_handler.c
DESCRIPTION
    This file handles the message handler to be routed to the specific sub-module.
	iAP2 init messages are routed to initalization sub module, Link messages are routed to
	link sub module, iAP2 packet messages are routed to packet sub module.
*/

#include <connection_no_ble.h>
#include "iap2_private.h"
#include "iap2_init.h"
#include "iap2_connect.h"
#include "iap2_link.h"
#include "iap2_link_setup.h"
#include "iap2_packet.h"

void iap2LibHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch (id)
    {
        case IAP2_INTERNAL_CP_INIT_CFM:
        iap2HandleInternalCpInitCfm((const IAP2_INTERNAL_CP_INIT_CFM_T *)message);
        break;

        case CL_RFCOMM_REGISTER_CFM:
        iap2HandleRfcommRegisterCfm((const CL_RFCOMM_REGISTER_CFM_T *)message);
        break;

        case CL_SDP_REGISTER_CFM:
        iap2HandleClSdpRegisterCfm((const CL_SDP_REGISTER_CFM_T *)message);
        break;

        case CL_RFCOMM_CONNECT_IND:
        iap2HandleClRfcommConnectInd((CL_RFCOMM_CONNECT_IND_T *)message);
        break;

        case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
        iap2HandleClSdpServiceSearchAttributeCfm((CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *)message);
        break;

        default:
        DEBUG_PRINT(("iap2LibHandler unknown msg=%04x\n", id));
        break;
    }
}

void iap2LinkHandler(Task task, MessageId id, Message message)
{
    iap2_link *link = (iap2_link *)task;

    if (!iap2LinkValidate(link))
    {
        DEBUG_PRINT(("iap2LinkHandler invalid link=0x%p\\n", (void*)link));
        return;
    }

    switch (id)
    {
        /* Connection related messages */
        case IAP2_INTERNAL_CONNECT_TIMEOUT:
        iap2HandleInternalConnectTimeout(link);
        break;

        case IAP2_INTERNAL_CONNECT_SDP_SEARCH_REQ:
        iap2HandleInternalConnectSdpSearchReq(link);
        break;

        case CL_RFCOMM_CLIENT_CONNECT_CFM:
        iap2HandleClRfcommClientConnectCfm(link, (const CL_RFCOMM_CLIENT_CONNECT_CFM_T *)message);
        break;

        case CL_RFCOMM_SERVER_CONNECT_CFM:
        iap2HandleClRfcommServerConnectCfm(link, (const CL_RFCOMM_SERVER_CONNECT_CFM_T *)message);
        break;

        case CL_RFCOMM_DISCONNECT_IND:
        iap2HandleClRfcommDisconnectInd(link, (const CL_RFCOMM_DISCONNECT_IND_T *)message);
        break;

        case CL_RFCOMM_DISCONNECT_CFM:
        iap2HandleClRfcommDisconnectCfm(link, (const CL_RFCOMM_DISCONNECT_CFM_T *)message);
        break;

        case CL_RFCOMM_CONTROL_IND:
        DEBUG_PRINT(("iAP2 RFCOMM control ind sink=0x%p break=%d modem=0x%02X\n",
                     (void*)((CL_RFCOMM_CONTROL_IND_T *)message)->sink,
                     ((CL_RFCOMM_CONTROL_IND_T *)message)->break_signal,
                     ((CL_RFCOMM_CONTROL_IND_T *)message)->modem_signal));
        break;

        /* Link related messages */
        case IAP2_INTERNAL_LINK_INIT_REQ:
        iap2HandleInternalLinkInitReq(link);
        break;

        case IAP2_INTERNAL_LINK_SYNC_REQ:
        iap2HandleInternalLinkSyncReq(link);
        break;

        case IAP2_INTERNAL_LINK_SYNC_IND:
        iap2HandleInternalLinkSyncInd(link, (const IAP2_INTERNAL_LINK_SYNC_IND_T *)message);
        break;

        case IAP2_INTERNAL_LINK_AUTH_IND:
        iap2HandleInternalLinkAuthInd(link, (const IAP2_INTERNAL_LINK_AUTH_IND_T *)message);
        break;

        case IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM:
        iap2HandleCpReadAuthCertLenCfm(link, (const IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM_T *)message);
        break;

        case IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM:
        iap2HandleCpReadAuthCertCfm(link, (const IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM_T *)message);
        break;

        case IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM:
        iap2HandleCpWriteChallengeDataCfm(link, (const IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM_T *)message);
        break;

        case IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM:
        iap2HandleCpReadChallengeRspCfm(link, (const IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM_T *)message);
        break;

        case IAP2_INTERNAL_LINK_IDENT_REQ:
        iap2HandleInternalLinkIdentReq(link);
        break;

        case IAP2_INTERNAL_LINK_DETACH_REQ:
        iap2HandleInternalLinkDetachReq(link);
        break;

        case IAP2_INTERNAL_LINK_EAK_IND:
        iap2HandleInternalLinkEakInd(link, (const IAP2_INTERNAL_LINK_EAK_IND_T *)message);
        break;

        case MESSAGE_MORE_DATA:
        iap2HandleMessageMoreData(link, (MessageMoreData *)message);
        break;

        case IAP2_INTERNAL_ACK_FLUSH_REQ:
        iap2HandleInternalAckFlushReq(link);
        break;

        default:
        DEBUG_PRINT(("iap2LinkHandler unknown msg=%04x\n", id));
        break;
    }
}

void iap2PacketHandler(Task task, MessageId id, Message message)
{
    iap2_packet *packet = (iap2_packet *)task;

    switch (id)
    {
        case IAP2_INTERNAL_PACKET_RETRANS_REQ:
        iap2HandleInternalPacketRetransReq(packet, (IAP2_INTERNAL_PACKET_RETRANS_REQ_T *)message);
        break;

        default:
        DEBUG_PRINT(("iap2PacketHandler unknown msg=%04x\n", id));
        break;
    }
}


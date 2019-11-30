/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_link_setup.c
DESCRIPTION
    Handles link setup information with remote device.
*/
#include <connection_no_ble.h>
#include <stream.h>
#include <sink.h>
#include <source.h>
#include <vm.h>
#include <bdaddr.h>
#include "iap2_link.h"
#include "iap2_link_setup.h"
#include "iap2_packet.h"
#include "iap2_cp.h"
#include "iap2_eap.h"

static const uint8 iap2_init_seq[] = {0xFF, 0x55, 0x02, 0x00, 0xEE, 0x10};
static const uint8 iap1_lingo_ipodack[] = {0x55, 0x04, 0x00, 0x02, 0x04, 0xEE, 0x08};
static const uint8 iap1_lingo_req_ident [] = {0x55, 0x02, 0x00, 0x00, 0xFE};
static const uint8 iap1_incompatible_seq[] = {0xFF, 0x55, 0x0E, 0x00, 0x13, 0xFF,
                                              0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                              0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEB};

static int iap2CheckInitSequence(uint8 *data, uint16 size_data)
{
    uint16 i = 0;

    while (1)
    {
        /* Find the first byte which is 0xFF */
        for (; i < size_data && data[i] != 0xFF; i++);

        if (size_data - i < sizeof(iap2_init_seq))
            /* Not enough data */
            break;
        else if (!memcmp(&data[i], iap2_init_seq, sizeof(iap2_init_seq)))
            /* Sequence matched, return with bytes to drop */
            return (i + sizeof(iap2_init_seq));
        else
            i++;
    }

    return -1;
}

static int iap1CheckInitSequence(uint8 *data, uint16 size_data)
{
    uint16 i = 0;

    while (1)
    {
        /* Find the first byte which is not 0xFF */
        for (; i < size_data && data[i] == 0xFF; i++);

        if (size_data - i >= sizeof(iap1_lingo_ipodack) &&
            !memcmp(&data[i], iap1_lingo_ipodack, sizeof(iap1_lingo_ipodack)))
            /* Sequence matched, return with bytes to drop */
            return (i + sizeof(iap1_lingo_ipodack));
        else if (size_data - i >= sizeof(iap1_lingo_req_ident) &&
                 !memcmp(&data[i], iap1_lingo_req_ident, sizeof(iap1_lingo_req_ident)))
            /* Sequence matched, return with bytes to drop */
            return (i + sizeof(iap1_lingo_req_ident));
        else if (size_data - i < sizeof(iap1_lingo_req_ident))
            /* Not enough data */
            break;
        else
            i++;
    }

    return -1;
}

static uint16 iap2IdentInfoPayloadSize(void)
{
    uint16 i;
    uint16 size = 0;
    uint16 ea_protocol_count = iap2EAGetNumProtocols();

    size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->device_name);
    size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->model_id);
    size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->manufacturer);
    size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->serial_number);
    size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->firmware_version);
    size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->hardware_version);
    size += (iap2_lib->config->app_match_team_id ? Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->app_match_team_id) : 0);
    /* Current language */
    size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->current_language);
    /* Supported languages */
    size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->current_language);
    /* Power Providing Capability */
    size += Iap2CtrlParamWriteEnum(NULL, 0, 0);
    /* Minimum Current Drawn */
    size += Iap2CtrlParamWriteUint16(NULL, 0, 0);

    /* Bluetooth Transport Component */
    if (!BdaddrIsZero(&iap2_lib->config->local_bdaddr))
    {
        size += IAP2_CTRL_PARAM_HEADER_LEN;
        size += Iap2CtrlParamWriteUint16(NULL, 0, 0);
        size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->device_name);
        size += Iap2CtrlParamWriteNone(NULL, 0);
        size += Iap2CtrlParamWriteBlob(NULL, 0, NULL, 6);
    }

    /* EA Protocols */
    for (i = 1; i <= ea_protocol_count; ++i)
    {
        size += IAP2_CTRL_PARAM_HEADER_LEN;
        size += Iap2CtrlParamWriteUint8(NULL, 0, 0);
        size += Iap2CtrlParamWriteString(NULL, 0, iap2EAGetName(i));
        size += Iap2CtrlParamWriteEnum(NULL, 0, 0);
    }

    /* iAP2 HID components */
    for (i = 0; i < iap2_lib->config->iap2_hid_component_count; i++)
    {
        size += IAP2_CTRL_PARAM_HEADER_LEN;
        size += Iap2CtrlParamWriteUint16(NULL, 0, 0);
        size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->iap2_hid_components[i].name);
        size += Iap2CtrlParamWriteEnum(NULL, 0, 0);
    }

    /* BT HID component */
    if (iap2_lib->config->bt_hid_component)
    {
        size += IAP2_CTRL_PARAM_HEADER_LEN;
        size += Iap2CtrlParamWriteUint16(NULL, 0, 0);
        size += Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->bt_hid_component->name);
        size += Iap2CtrlParamWriteEnum(NULL, 0, 0);
        size += Iap2CtrlParamWriteUint16(NULL, 0, 0);
    }

    /* Messages */
    size += Iap2CtrlParamWriteBlob(NULL, 0, NULL, iap2_lib->config->messages_tx_count << 1);
    size += Iap2CtrlParamWriteBlob(NULL, 0, NULL, iap2_lib->config->messages_rx_count << 1);

    return size;
}

void iap2HandleInternalLinkInitReq(iap2_link *link)
{
    DEBUG_PRINT_LINK(("iAP2 link init req link=0x%p\n", (void*)link));

    if (link->state == iap2_link_connecting_init)
    {
        /* Send the 6 bytes initialisation sequence */
        if (SinkSlack(link->sink) >= sizeof(iap2_init_seq))
        {
            memmove(SinkMap(link->sink) + SinkClaim(link->sink, sizeof(iap2_init_seq)),
                    iap2_init_seq,
                    sizeof(iap2_init_seq));
            SinkFlush(link->sink, sizeof(iap2_init_seq));
        }

        /* Create a 1Hz timer to re-send this sequence */
        MessageSendLater(&link->task, IAP2_INTERNAL_LINK_INIT_REQ, NULL, D_SEC(1));
    }
}

uint16 iap2HandleDataLinkInit(iap2_link *link, uint8 *data, uint16 size_data)
{
    int size_processed = 0;

    /* We are initialising the link, try to match initialising sequence */
    if ((size_processed = iap2CheckInitSequence(data, size_data)) >= 0)
    {
        /* iAP2 Sequence matched, start link synchronization */
        DEBUG_PRINT_LINK(("iAP2 sequence matched\n"));
        link->state = iap2_link_initialised;
        MessageSend(&link->task, IAP2_INTERNAL_LINK_SYNC_REQ, NULL);
    }
    else if ((size_processed = iap1CheckInitSequence(data, size_data)) >= 0)
    {
        /* iAP1 sequence matched, notify the app and respond with incompatible */
        MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_incompatible);

        DEBUG_PRINT_LINK(("iAP1 sequence matched\n"));

        link->state = iap2_link_error;
        msg->link = link;
        msg->bd_addr = link->remote_addr;
        msg->sink = link->sink;
        MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);

        if (SinkSlack(link->sink) >= sizeof(iap1_incompatible_seq))
        {
            memmove(SinkMap(link->sink) + SinkClaim(link->sink, sizeof(iap1_incompatible_seq)),
                    iap1_incompatible_seq,
                    sizeof(iap1_incompatible_seq));
            SinkFlush(link->sink, sizeof(iap1_incompatible_seq));
        }
    }

    return MAX(size_processed, 0);
}

void iap2HandleInternalLinkSyncReq(iap2_link *link)
{
    DEBUG_PRINT_LINK(("iAP2 link sync req link=0x%p\n", (void*)link));

    /* Send link synchronization packet */
    if (link->state == iap2_link_initialised || link->state == iap2_link_sync_retry)
    {
        iap2_status_t status = iap2_status_out_of_resources;
        uint16 size_payload = IAP2_SYNC_PAYLOAD_BASE_LEN;
        iap2_packet *packet;

        if (link->param.hasFileTransferSession)
            size_payload += 3;

        if (link->param.hasExternalAccessorySession)
            size_payload += 3;

        packet = iap2PacketAllocate(size_payload);

        DEBUG_PRINT_LINK(("iAP2 link generate sync payload state=%d\n", link->state));

        if (packet)
        {
            uint16 i = 0;

            memset(packet->payload, 0, size_payload);

            /* Version is always 1 */
            packet->payload[i++] = 1;

            /* Link parameters */
            packet->payload[i++] = link->param.supportZeroAckLink ? 127 : link->param.MaxNumOutstandingPackets;
            packet->payload[i++] = link->param.MaxRxPacketLength >> 8;
            packet->payload[i++] = link->param.MaxRxPacketLength & 0xff;

            if (!link->param.supportZeroAckLink)
            {
                /* The following parameters must be zero for ZeroACK/ZeroRetransmit link */
                packet->payload[i++] = link->param.RetransmissionTimeout >> 8;
                packet->payload[i++] = link->param.RetransmissionTimeout & 0xff;
                packet->payload[i++] = link->param.CumulativeAckTimeout >> 8;
                packet->payload[i++] = link->param.CumulativeAckTimeout & 0xff;
                packet->payload[i++] = link->param.MaxNumRetransmissions;
                packet->payload[i++] = link->param.MaxCumulativeAcks;
            }
            else
                i += 6;

            /* Control session */
            packet->payload[i++] = link->session_id_ctrl;
            packet->payload[i++] = iap2_session_type_ctrl;
            packet->payload[i++] = 1;

            /* File transfer session */
            if (link->param.hasFileTransferSession)
            {
                packet->payload[i++] = link->session_id_ftp;
                packet->payload[i++] = iap2_session_type_ftp;
                packet->payload[i++] = 1;
            }

            /* EA session */
            if (link->param.hasExternalAccessorySession)
            {
                packet->payload[i++] = link->session_id_ea;
                packet->payload[i++] = iap2_session_type_ea;
                packet->payload[i++] = 1;
            }

            if ((status = iap2PacketEnqueue(link, packet)) == iap2_status_success)
                link->state = iap2_link_sync_wait;
            else
                iap2PacketDeallocate(packet);
        }

        if (status != iap2_status_success)
        {
            /* Packet failed to allocate, notify the app */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, status);
            msg->link = NULL;
            msg->bd_addr = link->remote_addr;
            msg->sink = NULL;
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
            link->state = iap2_link_error;

            /* Disconnect the link */
            ConnectionRfcommDisconnectRequest(&link->task, link->sink);
        }
    }
}

void iap2HandleInternalLinkSyncInd(iap2_link *link, const IAP2_INTERNAL_LINK_SYNC_IND_T *ind)
{
    if (link->state == iap2_link_sync_wait)
    {
        bool param_valid = FALSE;
        uint16 value;

        /* Check the SYN payload */

        do
        {
            /* SYN paylod too short */
            if (ind->size_payload < IAP2_SYNC_PAYLOAD_BASE_LEN)
                break;

            /* Link version */
            if (ind->payload[0] != 1)
                break;

            /* Maximum Number of Outstanding Packets */
            if (ind->payload[1] < 1 || ind->payload[1] > 127)
                break;

            /* Maximum Received Packet Length */
            value = ((uint16)ind->payload[2] << 8) | ind->payload[3];
            if (value < 24)
                break;

            if (link->param.supportZeroAckLink)
            {
                /* We tried to negotiate a ZeroACK/ZeroRetransmit link */
                if (ind->payload[4] != 0 ||
                    ind->payload[5] != 0 ||
                    ind->payload[6] != 0 ||
                    ind->payload[7] != 0 ||
                    ind->payload[8] != 0 ||
                    ind->payload[9] != 0)
                {
                    /* iOS rejected our parameters, most likely it is an iOS before 9.0 */
                    /* Retry link synchronization with different parameters */
                    param_valid = FALSE;
                    link->param.supportZeroAckLink = FALSE;
                    break;
                }
            }
            else
            {
                /* Retransmission Timeout */
                value = ((uint16)ind->payload[4] << 8) | ind->payload[5];
                if (value < 20)
                    break;

                /* Cumulative Acknowledgement Timeout */
                value = ((uint16)ind->payload[6] << 8) | ind->payload[7];
                if (value < 10 || value > ((((uint16)ind->payload[4] << 8) | ind->payload[5]) >> 1))
                    break;

                /* Maximum Number of Retransmissions */
                if (ind->payload[8] < 1 || ind->payload[8] > 30)
                    break;

                /* Maximum Cumulative Acknowledgements */
                if (ind->payload[9] > MIN(127, ind->payload[1]))
                    break;
            }

            /* Everything verified */
            param_valid = TRUE;
        } while (0);

        DEBUG_PRINT_LINK(("iAP2 link param valid=%d\n", param_valid));

        if (param_valid)
        {
            uint16 i;

            /* Store the SYN payload and move to the next step */
            link->param.MaxNumOutstandingPackets = ind->payload[1];
            link->param.MaxRxPacketLength = ((uint16)ind->payload[2] << 8) | ind->payload[3];
            link->param.RetransmissionTimeout = ((uint16)ind->payload[4] << 8) | ind->payload[5];
            link->param.CumulativeAckTimeout = ((uint16)ind->payload[6] << 8) | ind->payload[7];
            link->param.MaxNumRetransmissions = ind->payload[8];
            link->param.MaxCumulativeAcks = ind->payload[9];

            /* Store the session number */
            link->session_id_ctrl = 0;
            link->session_id_ftp = 0;
            link->session_id_ea = 0;
            i = 10;
            while (i + 3 <= ind->size_payload)
            {
                switch (ind->payload[i + 1])
                {
                    case iap2_session_type_ctrl:
                        link->session_id_ctrl = ind->payload[i];
                        break;

                    case iap2_session_type_ftp:
                        link->session_id_ftp = ind->payload[i];
                        break;

                    case iap2_session_type_ea:
                        link->session_id_ea = ind->payload[i];
                        break;

                    default:
                        break;
                }

                i += 3;
            }

            link->state = iap2_link_sync_done;
            iap2PacketAckFlush(link, link->last_received_seq);
        }
        else
        {
            /* We need to retry syncronization */
            link->state = iap2_link_sync_retry;
            MessageSend(&link->task, IAP2_INTERNAL_LINK_SYNC_REQ, NULL);
        }
    }
}

void iap2HandleInternalLinkAuthInd(iap2_link *link, const IAP2_INTERNAL_LINK_AUTH_IND_T *ind)
{
    /* Messages handled by this function:
       RequestAuthenticationCertificate
       RequestAuthenticationChallengeResponse */

    if (link->state == iap2_link_sync_done &&
        ind->message_id == RequestAuthenticationCertificate)
    {
        /* Ack the packet now since this process will take very long */
        if (!link->param.supportZeroAckLink)
            iap2PacketAckFlush(link, link->last_received_seq);

        link->state = iap2_link_auth_cert_req;
        /* Read the authentication certificate length first */
        DEBUG_PRINT_LINK(("iAP2 read authentication cert len link=0x%p\n", (void*)link));
        iap2CpReadAuthCertLen(link);
    }
    else if (link->state == iap2_link_auth_cert_done &&
             ind->message_id == RequestAuthenticationChallengeResponse)
    {
        uint16 challenge_data_len = 0;
        uint8 *challenge_data = (uint8*)ind->payload;

        /* Ack the packet now since this process will take very long */
        if (!link->param.supportZeroAckLink)
            iap2PacketAckFlush(link, link->last_received_seq);

        /* Find the challenge data in the payload */
        while (challenge_data < ind->payload + ind->size_payload)
        {
            if (challenge_data[2] == 0 && challenge_data[3] == 0)
            {
                challenge_data_len = (((uint16)challenge_data[0] << 8) | (uint16)challenge_data[1]) - IAP2_CTRL_PARAM_HEADER_LEN;
                challenge_data += IAP2_CTRL_PARAM_HEADER_LEN;
                break;
            }
            else
                challenge_data += ((uint16)challenge_data[0] << 8) | (uint16)challenge_data[1];
        }

        link->state = iap2_link_auth_challenge_req;
        /* Write the challenge data to coprocessor */
        DEBUG_PRINT_LINK(("iAP2 write challenge data link=0x%p len=%d\n", (void*)link, challenge_data_len));
        iap2CpWriteChallengeData(link , challenge_data_len, challenge_data);
    }
}

void iap2HandleCpReadAuthCertLenCfm(iap2_link *link, const IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM_T *cfm)
{
    DEBUG_PRINT_LINK(("iAP2 read authentication cert len cfm status=%d link=0x%p len=%d\n", cfm->status, (void*)link, cfm->auth_cert_len));

    if (link->state == iap2_link_auth_cert_req)
    {
        iap2_status_t status = iap2_status_coprocessor_fail;

        if (cfm->status == iap2_status_success)
        {
            iap2_packet *packet = iap2PacketAllocate(0);

            /* Create a special packet */
            if (packet)
            {
                packet->session_type = iap2_session_type_ctrl;
                packet->id.ctrl_message = (uint16)AuthenticationCertificate;
                /* Payload contains control parameter header */
                packet->size_payload = cfm->auth_cert_len + IAP2_CTRL_PARAM_HEADER_LEN;

                if ((status = iap2PacketEnqueue(link, packet)) != iap2_status_success)
                    iap2PacketDeallocate(packet);
            }
            else
                status = iap2_status_out_of_resources;
        }

        if (status != iap2_status_success)
        {
            /* Failed */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, status);
            msg->link = NULL;
            msg->bd_addr = link->remote_addr;
            msg->sink = NULL;
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
            link->state = iap2_link_error;

            /* Disconnect the link */
            ConnectionRfcommDisconnectRequest(&link->task, link->sink);
        }
    }
}

void iap2HandleCpReadAuthCertCfm(iap2_link *link, const IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM_T *cfm)
{
    DEBUG_PRINT_LINK(("iAP2 read authentication cert cfm status=%d link=0x%p\n", cfm->status, (void*)link));

    if (link->state == iap2_link_auth_cert_req)
    {
        if (cfm->status == iap2_status_success)
        {
            /* Write the final checksum into sink then we have a complete packet */
            if (SinkSlack(link->sink) > 0)
            {
                uint8 *sink_ptr = SinkMap(link->sink) + SinkClaim(link->sink, 1);
                *sink_ptr = (0 - cfm->checksum) & 0xff;
                SinkFlush(link->sink, 1);
                link->state = iap2_link_auth_cert_done;
            }
            else
            {
                MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM, iap2_status_success);
                msg->checksum = cfm->checksum;
                MessageSend(&link->task, IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM, msg);
            }
        }
        else
        {
            /* Coprocessor failed */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_coprocessor_fail);
            msg->link = NULL;
            msg->bd_addr = link->remote_addr;
            msg->sink = NULL;
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
            link->state = iap2_link_error;

            /* Disconnect the link */
            ConnectionRfcommDisconnectRequest(&link->task, link->sink);
        }
    }
}

void iap2HandleCpWriteChallengeDataCfm(iap2_link *link, const IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM_T *cfm)
{
    DEBUG_PRINT_LINK(("iAP2 write challenge data cfm status=%d link=0x%p\n", cfm->status, (void*)link));

    if (link->state == iap2_link_auth_challenge_req)
    {
        iap2_status_t status = iap2_status_coprocessor_fail;

        if (cfm->status == iap2_status_success)
        {
            iap2_packet *packet = iap2PacketAllocate(0);

            /* Create a special packet */
            if (packet)
            {
                packet->session_type = iap2_session_type_ctrl;
                packet->id.ctrl_message = (uint16)AuthenticationResponse;
                /* Payload contains control parameter header */
                packet->size_payload = cfm->challenge_rsp_len + IAP2_CTRL_PARAM_HEADER_LEN;

                if ((status = iap2PacketEnqueue(link, packet)) != iap2_status_success)
                    iap2PacketDeallocate(packet);
            }
            else
                status = iap2_status_out_of_resources;
        }

        if (status != iap2_status_success)
        {
            /* Failed */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, status);
            msg->link = NULL;
            msg->bd_addr = link->remote_addr;
            msg->sink = NULL;
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
            link->state = iap2_link_error;

            /* Disconnect the link */
            ConnectionRfcommDisconnectRequest(&link->task, link->sink);
        }
    }

}

void iap2HandleCpReadChallengeRspCfm(iap2_link *link, const IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM_T *cfm)
{
    DEBUG_PRINT_LINK(("iAP2 read challenge rsp cfm status=%d link=0x%p\n", cfm->status, (void*)link));

    if (link->state == iap2_link_auth_challenge_req)
    {
        if (cfm->status == iap2_status_success)
        {
            /* Write the final checksum into sink then we have a complete packet */
            if (SinkSlack(link->sink) > 0)
            {
                uint8 *sink_ptr = SinkMap(link->sink) + SinkClaim(link->sink, 1);
                *sink_ptr = (0 - cfm->checksum) & 0xff;
                SinkFlush(link->sink, 1);
                link->state = iap2_link_auth_challenge_wait;
            }
            else
            {
                MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM, iap2_status_success);
                msg->checksum = cfm->checksum;
                MessageSend(&link->task, IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM, msg);
            }
        }
        else
        {
            /* Coprocessor access failed */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_CONNECT_CFM, iap2_status_out_of_resources);
            msg->link = NULL;
            msg->bd_addr = link->remote_addr;
            msg->sink = NULL;
            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, msg);
            link->state = iap2_link_error;

            /* Disconnect the link */
            ConnectionRfcommDisconnectRequest(&link->task, link->sink);
        }
    }
}

void iap2HandleInternalLinkIdentReq(iap2_link *link)
{
    DEBUG_PRINT_LINK(("iAP2 link ident req link=0x%p\n", (void*)link));

    if (link->state == iap2_link_ident_start)
    {
        iap2_status_t status = iap2_status_out_of_resources;
        uint16 size_payload;
        iap2_packet *packet;

        size_payload = iap2IdentInfoPayloadSize();

        /* Write identification information directly into Sink to avoid allocating huge VM buffer */
        packet = iap2PacketAllocate(0);

        if (packet)
        {
            packet->size_payload = size_payload;
            packet->session_type = iap2_session_type_ctrl;
            packet->id.ctrl_message = (uint16)IdentificationInformation;

            if ((status = iap2PacketEnqueue(link, packet)) == iap2_status_success)
                link->state = iap2_link_ident_wait;
            else
                iap2PacketDeallocate(packet);
        }

        if (status != iap2_status_success)
        {
            /* Packet allocate failed, notify the app */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_IDENTIFICATION_CFM, status);
            msg->link = link;
            msg->error_mask = 0;
            MessageSend(iap2_lib->app_task, IAP2_IDENTIFICATION_CFM, msg);
        }
    }
}

void iap2IdentInfoPayloadGenerate(uint8 *payload)
{
    uint16 i;
    uint16 group_len;
    uint16 ea_protocol_count = iap2EAGetNumProtocols();
    uint8 protocol_id;

    payload += Iap2CtrlParamWriteString(payload, IAP2_IDENT_INFO_PARAM_NAME, iap2_lib->config->device_name);
    payload += Iap2CtrlParamWriteString(payload, IAP2_IDENT_INFO_PARAM_MODEL_ID, iap2_lib->config->model_id);
    payload += Iap2CtrlParamWriteString(payload, IAP2_IDENT_INFO_PARAM_MANUFACTURER, iap2_lib->config->manufacturer);
    payload += Iap2CtrlParamWriteString(payload, IAP2_IDENT_INFO_PARAM_SERIAL_NUMBER, iap2_lib->config->serial_number);
    payload += Iap2CtrlParamWriteString(payload, IAP2_IDENT_INFO_PARAM_FIRMWARE_VER, iap2_lib->config->firmware_version);
    payload += Iap2CtrlParamWriteString(payload, IAP2_IDENT_INFO_PARAM_HARDWARE_VER, iap2_lib->config->hardware_version);

    payload += Iap2CtrlParamWriteHeader(payload, IAP2_IDENT_INFO_PARAM_MESSAGES_TX, iap2_lib->config->messages_tx_count << 1);
    for (i = 0; i < iap2_lib->config->messages_tx_count; i++)
    {
        *payload++ = (iap2_lib->config->messages_tx[i] >> 8) & 0xff;
        *payload++ = iap2_lib->config->messages_tx[i] & 0xff;
    }

    payload += Iap2CtrlParamWriteHeader(payload, IAP2_IDENT_INFO_PARAM_MESSAGES_RX, iap2_lib->config->messages_rx_count << 1);
    for (i = 0; i < iap2_lib->config->messages_rx_count; i++)
    {
        *payload++ = (iap2_lib->config->messages_rx[i] >> 8) & 0xff;
        *payload++ = iap2_lib->config->messages_rx[i] & 0xff;
    }

    payload += Iap2CtrlParamWriteEnum(payload, IAP2_IDENT_INFO_PARAM_POWER_CAP, iap2_lib->config->power_capability);
    payload += Iap2CtrlParamWriteUint16(payload, IAP2_IDENT_INFO_PARAM_MAX_CURRENT, iap2_lib->config->max_current_draw);

    for (protocol_id = 1; protocol_id <= ea_protocol_count; ++protocol_id)
    {
        const char *name = iap2EAGetName(protocol_id);

        group_len = Iap2CtrlParamWriteUint8(NULL, 0, 0) +
                    Iap2CtrlParamWriteString(NULL, 0, name) +
                    Iap2CtrlParamWriteEnum(NULL, 0, 0);

        payload += Iap2CtrlParamWriteHeader(payload, IAP2_IDENT_INFO_PARAM_EAP, group_len);
        payload += Iap2CtrlParamWriteUint8(payload, IAP2_EAP_PARAM_ID, protocol_id);
        payload += Iap2CtrlParamWriteString(payload, IAP2_EAP_PARAM_NAME, name);
        payload += Iap2CtrlParamWriteEnum(payload, IAP2_EAP_PARAM_MATCH_ACTION, iap2EAGetMatchAction(protocol_id));
    }

    if (iap2_lib->config->app_match_team_id)
    {
        payload += Iap2CtrlParamWriteString(payload, IAP2_IDENT_INFO_PARAM_TEAM_ID, iap2_lib->config->app_match_team_id);
    }

    payload += Iap2CtrlParamWriteString(payload, IAP2_IDENT_INFO_PARAM_CURRENT_LANG, iap2_lib->config->current_language);
    payload += Iap2CtrlParamWriteString(payload, IAP2_IDENT_INFO_PARAM_SUPPORT_LANG, iap2_lib->config->current_language);

    if (!BdaddrIsZero(&iap2_lib->config->local_bdaddr))
    {
        group_len = Iap2CtrlParamWriteUint16(NULL, 0, 0) +
                    Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->device_name) +
                    Iap2CtrlParamWriteNone(NULL, 0) +
                    Iap2CtrlParamWriteBlob(NULL, 0, NULL, 6);

        payload += Iap2CtrlParamWriteHeader(payload, IAP2_IDENT_INFO_PARAM_BT_TRANS, group_len);
        payload += Iap2CtrlParamWriteUint16(payload, IAP2_BT_PARAM_ID, IAP2_BLUETOOTH_TRANSPORT_ID);
        payload += Iap2CtrlParamWriteString(payload, IAP2_BT_PARAM_NAME, iap2_lib->config->device_name);
        payload += Iap2CtrlParamWriteNone(payload, IAP2_BT_PARAM_SUPPORT_IAP2);
        payload += Iap2CtrlParamWriteHeader(payload, IAP2_BT_PARAM_MAC_ADDR, 6);
        *payload++ = (iap2_lib->config->local_bdaddr.nap >> 8) & 0xff;
        *payload++ = iap2_lib->config->local_bdaddr.nap & 0xff;
        *payload++ = iap2_lib->config->local_bdaddr.uap;
        *payload++ = (iap2_lib->config->local_bdaddr.lap >> 16) & 0xff;
        *payload++ = (iap2_lib->config->local_bdaddr.lap >> 8) & 0xff;
        *payload++ = iap2_lib->config->local_bdaddr.lap & 0xff;
    }

    for (i = 0; i < iap2_lib->config->iap2_hid_component_count; i++)
    {
        group_len = Iap2CtrlParamWriteUint16(NULL, 0, 0) +
                    Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->iap2_hid_components[i].name) +
                    Iap2CtrlParamWriteEnum(NULL, 0, 0);

        payload += Iap2CtrlParamWriteHeader(payload, IAP2_IDENT_INFO_PARAM_IAP2_HID, group_len);
        payload += Iap2CtrlParamWriteUint16(payload, IAP2_HID_PARAM_ID, iap2_lib->config->iap2_hid_components[i].component_id);
        payload += Iap2CtrlParamWriteString(payload, IAP2_HID_PARAM_NAME, iap2_lib->config->iap2_hid_components[i].name);
        payload += Iap2CtrlParamWriteEnum(payload, IAP2_HID_PARAM_FUNCTION, iap2_lib->config->iap2_hid_components[i].function);
    }

    /* BT HID component */
    if (iap2_lib->config->bt_hid_component)
    {
        group_len = Iap2CtrlParamWriteUint16(NULL, 0, 0) +
                    Iap2CtrlParamWriteString(NULL, 0, iap2_lib->config->bt_hid_component->name) +
                    Iap2CtrlParamWriteEnum(NULL, 0, 0) +
                    Iap2CtrlParamWriteUint16(NULL, 0, 0);

        payload += Iap2CtrlParamWriteHeader(payload, IAP2_IDENT_INFO_PARAM_BT_HID, group_len);
        payload += Iap2CtrlParamWriteUint16(payload, IAP2_BT_HID_PARAM_ID, iap2_lib->config->bt_hid_component->component_id);
        payload += Iap2CtrlParamWriteString(payload, IAP2_BT_HID_PARAM_NAME, iap2_lib->config->bt_hid_component->name);
        payload += Iap2CtrlParamWriteEnum(payload, IAP2_BT_HID_PARAM_FUNCTION, iap2_lib->config->bt_hid_component->function);
        payload += Iap2CtrlParamWriteUint16(payload, IAP2_BT_HID_PARAM_TRANS_ID, IAP2_BLUETOOTH_TRANSPORT_ID);
    }
}


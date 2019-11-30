/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_ba_broadcaster_packetiser.c
 
DESCRIPTION
    Functionality related to controlling the broadcast packetiser and
    erasure coding from the application is encapsulated here.

    This file has two purposes:
    1. Hide details of setting up erasure coding and broadcast packetiser.
    2. Isolate the rest of the application from erasure coding and broadcast packetiser headers,
       to simplify build process on architectures which doesn't have that headers.
*/

#include "sink_ba_broadcaster_packetiser.h"
#include "sink_debug.h"

#if defined(ENABLE_BROADCAST_AUDIO) && defined(BROADCAST_PACKETISER_IS_CONTROLLED_BY_APP)

#include <broadcast_context.h>
#include <erasure_coding.h>
#include <broadcast_packetiser.h>
#include <rtime.h>
#include <panic.h>

#include <string.h>

#include "sink_ba.h"

/* Debugging defines */
#ifdef DEBUG_BA_BROADCASTER
#define DEBUG_BROADCASTER(x) DEBUG(x)
#else
#define DEBUG_BROADCASTER(x)
#endif

/* The erasure coded broadcast packet targets 2-DH5 packets */
#define BT_PACKET_2DH5_MAX_OCTETS 679

/* This is the minimum time before the TTP before which the receiver needs to receive
   the broadcast packet in order to decode/process the data and render */
#define RX_LATENCY_US 50000

#define DEFAULT_STATS_INTERVAL_IN_SEC (4)

static const ec_params_t *getEcParams(const erasure_coding_scheme_t ec_scheme)
{
    const ec_params_t *ec_params;

    if(ec_scheme == EC_SCHEME_2_5)
    {
        ec_params = &ec_params_2_5;
    }
    else
    {
        ec_params = &ec_params_3_9;
    }

    return ec_params;
}

void sinkBroadcasterInitErasureCoding(void)
{
    ec_config_tx_t ec_config;
    ec_handle_tx_t ec_handle;

    memset(&ec_config, 0, sizeof(ec_config));
    ec_config.sink = BroadcastContextGetSink();
    ec_config.csb_interval = US_PER_SLOT * CSB_INTERVAL_SLOTS;
    ec_config.extra_latency = RX_LATENCY_US;
    ec_config.stream_id = (uint8)BroadcastContextGetStreamId();
    ec_config.params = getEcParams(BroadcastContextGetEcScheme());
    ec_config.mtu = BT_PACKET_2DH5_MAX_OCTETS;

    DEBUG_BROADCASTER(("Erasure Coding: stream id %d, EC scheme %s\n", BroadcastContextGetStreamId(),
            BroadcastContextGetEcScheme() == EC_SCHEME_2_5 ? "2:5" : "3:9"));

    ec_handle = ErasureCodingTxInit(&ec_config);
    PanicNull(ec_handle);
    BroadcastContextSetEcTxHandle(ec_handle);
}

void sinkBroadcasterDestroyErasureCoding(void)
{
    if(BroadcastContextGetEcTxHandle())
    {
        ErasureCodingTxDestroy(BroadcastContextGetEcTxHandle());
        BroadcastContextSetEcTxHandle(NULL);
    }
}

static void setupPacketiserConfig(broadcast_packetiser_config_t *bp_config, Task task)
{
    broadcast_encr_config_t *aes_config;

    memset(bp_config, 0, sizeof(*bp_config));
    bp_config->client_task = task;
    bp_config->ec_handle.tx = BroadcastContextGetEcTxHandle();
    bp_config->scmst = packetiser_helper_scmst_copy_prohibited;
    bp_config->stats_interval = US_PER_SEC * DEFAULT_STATS_INTERVAL_IN_SEC;

    aes_config = BroadcastContextGetEncryptionConfig();
    if(aes_config)
    {
        bp_config->aesccm_disabled = FALSE;
        memcpy(bp_config->aesccm.key, &aes_config->seckey[1], sizeof(bp_config->aesccm.key));
        memcpy(bp_config->aesccm.fixed_iv, aes_config->fixed_iv, sizeof(bp_config->aesccm.fixed_iv));
        bp_config->aesccm.dynamic_iv = aes_config->variant_iv;
    }
    else
    {
        bp_config->aesccm_disabled = TRUE;
    }
}

void sinkBroadcasterInitPacketiser(Task task)
{
    broadcast_packetiser_config_t config;

    setupPacketiserConfig(&config, task);

    BroadcastPacketiserInit(&config);

    BroadcastContextSetBroadcastBusy(TRUE);
}

void sinkBroadcasterDestroyPacketiser(void)
{
    if(BroadcastContextGetBpTxHandle())
    {
        BroadcastPacketiserDestroy(BroadcastContextGetBpTxHandle());
        BroadcastContextSetBpTxHandle(NULL);
    }
}

static bp_msg_handler_result_t handlePacketiserInitCfm(Message message)
{
    const BROADCAST_PACKETISER_INIT_CFM_T *msg = (const BROADCAST_PACKETISER_INIT_CFM_T *)message;

    BroadcastContextSetBroadcastBusy(FALSE);

    if(msg->initialisation_success)
    {
        BroadcastContextSetBpTxHandle(msg->broadcast_packetiser);
        broadcastPacketiserInitialisedCfm(msg->initialisation_success);
        return BROADCASTER_BP_INIT_SUCCESS;
    }
    else
    {
        sinkBroadcasterDestroyErasureCoding();
        return BROADCASTER_BP_INIT_FAIL;
    }
}

static void handleStats(Message message)
{
#ifdef DEBUG_BA_BROADCASTER
    BROADCAST_PACKETISER_STATS_BROADCASTER_IND_T *msg = (BROADCAST_PACKETISER_STATS_BROADCASTER_IND_T *)message;
    DEBUG_BROADCASTER(("BA TX: ptx %d ftx %d fl %d\n", msg->stats.broadcast_packets_transmitted,
                       msg->stats.audio_frames_transmitted, msg->stats.audio_frames_late));
#else
    UNUSED(message);
#endif

}

bool sinkBroadcasterIsItPacketiserMessage(MessageId id)
{
    if(id >= BROADCAST_PACKETISER_MESSAGE_BASE && id < BROADCAST_PACKETISER_MESSAGE_TOP)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bp_msg_handler_result_t sinkBroadcasterHandlePacketiserMessages(MessageId id, Message message)
{
    bp_msg_handler_result_t result = BROADCASTER_BP_HANDLED_INTERNALLY;

    switch(id)
    {
    case BROADCAST_PACKETISER_INIT_CFM:
        result = handlePacketiserInitCfm(message);
        break;

    case BROADCAST_PACKETISER_STATS_BROADCASTER_IND:
        handleStats(message);
        break;

    default:
        DEBUG_BROADCASTER(("Unhandled broadcast packetiser msg id 0x%x\n", id));
        break;
    }

    return result;
}

bool sinkBroadcasterIsPacketiserControlledByApp(void)
{
    return TRUE;
}

#endif

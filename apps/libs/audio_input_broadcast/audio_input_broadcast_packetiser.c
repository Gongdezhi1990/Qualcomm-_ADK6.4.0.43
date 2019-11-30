/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_broadcast_packetiser.c

DESCRIPTION
    Create a broadcast_packetiser configured for the receiver role.

    This is only a minimal prototype implementation so that we can get the
    source of the audio packets and connect it to the sink input of the
    audio graph.

    This code is largely lifted from csb_receiver_packetiser.c but there are
    many dependencies on the main app that are not always available in an
    audio plugin. The overall design needs more thought.
*/

#include <audio.h>
#include <audio_config.h>
#include <audio_plugin_music_variants.h>
#include <broadcast.h>
#include <broadcast_cmd.h>
#include <broadcast_context.h>
#include <broadcast_packetiser.h>
#include <broadcast_stream_service_record.h>
#include <gain_utils.h>
#include <panic.h>
#include <print.h>
#include <rtime.h>
#include <scm.h>
#include <sink.h>
#include <source.h>
#include <stream.h>

#include "audio_input_broadcast.h"
#include "audio_input_broadcast_connect.h"
#include "audio_input_broadcast_private.h"
#include "audio_input_broadcast_packetiser.h"



static broadcast_packetiser_t *bp;
static ec_handle_rx_t ec;


/*! @brief Setup configuration and create a receiver packetiser. 
 */
void audioInputBroadcastPacketiserCreate(Source ba_receive_source, Sink audio_frame_sink)
{
    broadcast_packetiser_config_t bp_config;
    broadcast_encr_config_t *encr = NULL;
    ec_config_rx_t ec_config;
    uint16 interval;

    PRINT(("BA RX: BP: Attempt create source:%x sink:%x\n", ba_receive_source, audio_frame_sink));

    /* Create erasure_coding instance */
    ec_config.packet_client = NULL;
    ec_config.change_client = (Task)&csr_ba_receiver_decoder_plugin.data;
    ec_config.source = ba_receive_source;
    BroadcastReceiverGetCSBInterval(BroadcastContextGetBroadcastHandle(), &interval);
    ec_config.csb_interval = (rtime_t)interval * US_PER_SLOT;
    /* default to invalid stream ID, the packetiser will send us an
     * indication informing us of the actual stream ID it finds. */
    ec_config.stream_id = BSSR_STREAM_ID_INVALID;
    /* Since the stream id is unknown, so is the ec type */
    ec_config.params = NULL;

    ec = ErasureCodingRxInit(&ec_config);
    PanicNull(ec);

    /* Set broadcast_packetiser params */
    memset(&bp_config, 0, sizeof(bp_config));
    bp_config.ec_handle.rx = ec;

    bp_config.role = broadcast_packetiser_role_receiver;
    bp_config.client_task = (Task)&csr_ba_receiver_decoder_plugin.data;

    bp_config.port.sink = audio_frame_sink;

    /* configure encryption, it is valid not to have any, in which case disable
     * encryption */
    encr = BroadcastContextGetEncryptionConfig();
    if (encr)
    {
        memcpy(&bp_config.aesccm.key[0], &encr->seckey[1], sizeof(bp_config.aesccm.key));
        memcpy(&bp_config.aesccm.fixed_iv[0], &encr->fixed_iv[0], sizeof(bp_config.aesccm.fixed_iv));
        bp_config.aesccm.dynamic_iv = encr->variant_iv;
        bp_config.aesccm_disabled = FALSE;
    }
    else
    {
        bp_config.aesccm_disabled = TRUE;
    }
    
    /* CSB config */
    bp_config.stats_interval = MS_PER_SEC * 500;

    /* default sample rate to unknown, so we will similarly get an
     * indication from the packetiser of the actual sample rate in
     * the broadcast and can configure our audio with correct CELT
     * parameters */
    bp_config.sample_rate = rtime_sample_rate_unknown;

    /* not used in receiver, just set to 0 */
    bp_config.scmst = 0;
    bp_config.volume = 0;

    /* Disconnect the source in case it was being ConnectDispose'd */
    StreamDisconnect(ba_receive_source, NULL);

    PanicFalse(SinkMapInit(audio_frame_sink, STREAM_TIMESTAMPED, AUDIO_FRAME_METADATA_LENGTH));
    /* Try and create a packetiser */
    BroadcastPacketiserInit(&bp_config);
}

/*! @brief Teardown the packetiser.
 
    May cause receive to be automatically restarted if we were tearing
    down during handling of a receiver timeout indication.

    May cause receive to be automatically restarted if we were tearing
    down during handling of a sample rate change indication from the
    packetiser.
 */
void audioInputBroadcastPacketiserDestroy(Sink audio_frame_sink)
{
    PRINT(("BA RX: BP: Destroy\n"));

    if (!bp)
        return;

    if (audio_frame_sink)
        PanicFalse(SinkUnmap(audio_frame_sink));

    ErasureCodingRxDestroy(ec);
    ec = NULL;

    BroadcastPacketiserDestroy(bp);
    bp = NULL;

    BroadcastContextSetBroadcastBusy(FALSE);
}

void audioInputBroadcastPacketiserUpdateSink(Sink audio_frame_sink)
{
    Sink old_sink;

    if(audio_frame_sink)
    {
        PanicFalse(SinkMapInit(audio_frame_sink, STREAM_TIMESTAMPED, AUDIO_FRAME_METADATA_LENGTH));
    }

    old_sink = BroadcastPacketiserSetSink(bp, audio_frame_sink);

    if(old_sink)
    {
        PanicFalse(SinkUnmap(old_sink));
    }

}

broadcast_packetiser_t *audioInputBroadcastGetPacketiser(void)
{
    return bp;
}

ec_handle_rx_t audioInputBroadcastGetErasureCoding(void)
{
    return ec;
}


/******************************************************************************
 * Message Handlers
 ******************************************************************************/
/*! @brief */
static void audioInputBroadcastPacketiserHandleInitCfm(Task task, const BROADCAST_PACKETISER_INIT_CFM_T *message, audio_input_context_t *ctx)
{
    if (message->initialisation_success)
    {
        PRINT(("BA RX: BP: Packetiser created\n"));

        /* Successfully created a packetiser, which will have an SCM transport
         * associated, we can now enable the SCM receiver instance and
         * tell it that the transport task is the packetiser. */
        BroadcastCmdScmEnable(message->lib_task);
        BroadcastCmdScmRegisterDisableHandler(task);

        /* remember the packetiser handle for future API usage */
        PanicNotNull(bp);
        bp = message->broadcast_packetiser;
        
        BroadcastContextSetBroadcastBusy(TRUE);

        /* Now we are ready to complete the connection */
        AudioInputCommonConnect(ctx, task);
    }
    else
    {
        /* If we can't create the packetiser then we can't complete the 
           connecting state. */
        AudioInputCommmonSendError(task);
    }
}

/*! @brief In debug builds, display broaqdcast_packetiser stats. */
static void audioInputBroadcastPacketiserHandleBpStatsReceiverInd(const BROADCAST_PACKETISER_STATS_RECEIVER_IND_T *ind)
{
    broadcast_encr_config_t *encr = NULL;
    const broadcast_packetiser_stats_receiver_t *stats = &ind->stats;
    ba_receiver_plugin_context_t *b_ctx = audioInputBroadcastGetContext();

    PanicNull(b_ctx);

    /* Print broadcast packet stats */
    PRINT(("BA RX: BP: bpr=%u,bpdt=%u,bpi=%u,bpusr=%u, ae=%u,afr=%u,afl=%u,afns=%u\n",
               stats->broadcast_packets_received,
               stats->broadcast_packet_duplicate_ttp,
               stats->broadcast_packet_invalid,
               stats->broadcast_packet_unsupported_sample_rate,
               stats->authentication_errors,
               stats->audio_frames_received,
               stats->audio_frames_late,
               stats->audio_frames_no_space));

    /* if we're getting mac failures, but we think we have a variant IV,
     * then look again, it probably changed */
    encr = BroadcastContextGetEncryptionConfig();
    if (encr && (stats->authentication_errors != 0) && (encr->variant_iv != 0))
    {
        PRINT(("BA RX: BP: AUTHENTICATION FAILURES - Search for new Variant IV\n"));
        BroadcastStopReceiver(BroadcastContextGetBroadcastHandle());
        /* Post a Upstream DSP message to App to handle this */
        MessageSend(b_ctx->app_task, AUDIO_BA_RECEIVER_START_SCAN_VARIANT_IV, NULL);
    }
}

/*! @brief In debug builds, display erasure_coding stats. */
static void audioInputBroadcastPacketiserHandleEcStatsReceiverInd(const ERASURE_CODING_STATS_RX_IND_T *ind)
{
    uint32 i;

    PRINT(("BA RX: EC: ecp={"));
    for (i = 0; i < ARRAY_DIM(ind->stats.coding_id); i++)
    {
        PRINT(("%u,", ind->stats.coding_id[i]));
    }
    PRINT(("} eccii=%u,ecus=%u,ecf=%u\n",
               ind->stats.coding_id_invalid,
               ind->stats.stream_id_unsupported,
               ind->stats.decode_failures));
}

/*! @brief New volume from Broadcaster, send up to the app to handle. */
static void audioInputBroadcastPacketiserHandleVolumeChangeInd(const BROADCAST_PACKETISER_VOLUME_CHANGE_IND_T *ind)
{
    PRINT(("BA RX: BP: vol change %d\n", ind->volume));

    int volume_in_db = ConvertBroadcastVolumeTodB((uint16)ind->volume, BroadcastContextGetVolumeTable());
    AudioConfigSetMasterVolume(volume_in_db);
    AudioMixerSetVolume();
}

/*! @brief */
static void audioInputBroadcastPacketiserHandleScmstChangeInd(const BROADCAST_PACKETISER_SCMST_CHANGE_IND_T *message)
{
    PRINT(("BA RX: BP: scmst changed\n"));

    /* Ignore */
    UNUSED(message);
}

static bool getCeltCodecConfig(uint32 rate, uint8 stream_id, codec_config_celt *codec_config)
{
    uint16 bssr_rate = rate == 44100 ? BSSR_CODEC_FREQ_44100HZ : BSSR_CODEC_FREQ_48KHZ;
    bool found = FALSE;
    uint16 bssr_len_words;
    const uint8* bssr;

    if (BroadcastContextGetBssrConfig(&bssr, &bssr_len_words))
    {
        if (bssrGetCodecConfigCeltByRate(bssr, (uint16) (bssr_len_words *  sizeof(uint16)), stream_id, bssr_rate, codec_config))
            found = TRUE;
    }

    return found;
}

static bool isSampleRateValid(rtime_sample_rate_t sample_rate)
{
    if(sample_rate == rtime_sample_rate_48000 ||
       sample_rate == rtime_sample_rate_44100)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static bool updateCodecParams(rtime_sample_rate_t sample_rate)
{
    bool rc = FALSE;
    uint32 rate = sample_rate == rtime_sample_rate_44100 ? 44100 : 48000;
    codec_config_celt codec_config;

    /* find out codec parameters for new sample rate
     * teardown/recreate audio chain in kymera
     * reconfigure packetiser */
    if (getCeltCodecConfig(rate, (uint8) BroadcastContextGetStreamId(), &codec_config))
    {
        audio_input_context_t *audio_ctx = &audioInputBroadcastGetContext()->audio_ctx;
        /* csb_receiver example app completely tears down the operator
           chain and re-creates it with the new rate.
           I am not sure this will work nicely with the audio plugins
           framework so I'll use the approach stopping the chain,
           re-configuting it and then starting it again.
        */
        audioInputBroadcastConnectReconfigureDecoder(audio_ctx, rate, &codec_config);

        /* We haven't destroyed and re-created the chain so the currently
           connected sink should still be valid */
        BroadcastPacketiserSetCodecParameters(bp,
                                              codec_config.frame_size,
                                              codec_config.frame_samples);
        rc = TRUE;
    }

    return rc;
}

/*! @brief Handle a change of sample rate. */
static void audioInputBroadcastPacketiserHandleSampleRateChangeInd(const BROADCAST_PACKETISER_SAMPLE_RATE_CHANGE_IND_T *ind)
{
    bool rc = FALSE;
    ba_receiver_plugin_context_t * b_ctx = audioInputBroadcastGetContext();

    PRINT(("BA RX: BP: Sample Rate Changed Ind %u\n", ind->sample_rate));

    PanicNull(bp);

    /* is the sample rate sane? */
    if(isSampleRateValid(ind->sample_rate))
    {
        if(b_ctx->received_sample_rate != ind->sample_rate)
        {
            b_ctx->received_sample_rate = ind->sample_rate;

            rc = updateCodecParams(ind->sample_rate);
        }
    }

    /* inform packetiser of result; carry on playing or we can't handle the change */
    BroadcastPacketiserSampleRateChangeResponse(bp, rc);
}

/* This function doesn't validate BSSR record, it must be done earlier */
static const ec_params_t *getEcParamsForStreamId(const uint8 *bssr, uint16 bssr_len, uint8 stream_id)
{
    uint8 ec_type = 0;
    const ec_params_t *ec_params = NULL;

    if(bssrGetErasureCode(bssr, bssr_len, stream_id, &ec_type))
    {
        if(ec_type == BSSR_ERASURE_CODE_2_5)
        {
            ec_params = &ec_params_2_5;
        }
        else
        {
            ec_params = &ec_params_3_9;
        }

        PRINT(("BA RX: BP: stream id %d, ec scheme %s\n", stream_id ,
                ec_type == BSSR_ERASURE_CODE_2_5 ? "2:5" : "3:9"));
    }

    return ec_params;
}

static void updateCodecParamsForExistingSampleRate(void)
{
    ba_receiver_plugin_context_t * b_ctx = audioInputBroadcastGetContext();

    if(isSampleRateValid(b_ctx->received_sample_rate))
    {
        updateCodecParams(b_ctx->received_sample_rate);
    }
}

/*! @brief Handle a change of stream ID. */
static void audioInputBroadcastPacketiserHandleEcStreamIdChangeInd(const ERASURE_CODING_RX_STREAM_ID_CHANGE_IND_T *ind)
{
    uint16 bssr_len_words;
    const uint8* bssr;
    const ec_params_t *new_params = NULL;
    bool stream_can_be_received = FALSE;

    PanicNull(bp);

    PRINT(("BA RX: BP: Stream Changed Ind ID:%u\n", ind->stream_id));

    /* validate that we have BSSR details for the stream ID and will be able
     * to handle it */
    if (BroadcastContextGetBssrConfig(&bssr, &bssr_len_words))
    {
        uint16 bssr_len = (uint16) (bssr_len_words * sizeof(uint16));

        if (BroadcastContextStreamCanBeReceived(bssr, bssr_len, ind->stream_id))
        {
            /* remember the stream ID, so that if we lose the broadcaster
             * we'll go back to the same stream ID if it comes back */
            BroadcastContextSetStreamId(ind->stream_id);
            new_params = getEcParamsForStreamId(bssr, bssr_len, ind->stream_id);
            stream_can_be_received = TRUE;
        }
    }

    /* inform packetiser if we can or can't handle this stream ID */
    ErasureCodingRxStreamIDChangeResponse(ec, new_params);

    if (stream_can_be_received)
    {
        updateCodecParamsForExistingSampleRate();
    }
}

/*! @brief Handle a pending change to the AFH channel map */
static void audioInputBroadcastPacketiserHandleEcAfhChannelMapChangePendingInd(const ERASURE_CODING_RX_AFH_CHANNEL_MAP_CHANGE_PENDING_IND_T *ind)
{
    /* unhandled */
    UNUSED(ind);
    PanicNull(BroadcastContextGetBroadcastHandle());
    PRINT(("BA RX: BP: AFH channel map change pending\n"));
    BroadcastStartReceiver(BroadcastContextGetBroadcastHandle(), broadcast_mode_synctrain_only, 0x0000);
}

/*! @brief Return TRUE if id is a brodcast_packetiser message id */
bool audioInputBroadcastPacketiserIsBroadcastPacketiserMessageId(MessageId id)
{
    return (id >= BROADCAST_PACKETISER_MESSAGE_BASE && id < BROADCAST_PACKETISER_MESSAGE_TOP) ||
            (id >= ERASURE_CODING_MESSAGE_BASE && id < ERASURE_CODING_MESSAGE_TOP);
}

/*! @brief Handle Broadcast Packetiser Library messages. */
void audioInputBroadcastPacketiserHandleMessage(Task task, MessageId id, Message message, audio_input_context_t *ctx)
{
    switch (id)
    {
        case BROADCAST_PACKETISER_INIT_CFM:
            audioInputBroadcastPacketiserHandleInitCfm(task, (const BROADCAST_PACKETISER_INIT_CFM_T *) message, ctx);
            break;

        case BROADCAST_PACKETISER_STATS_RECEIVER_IND:
            audioInputBroadcastPacketiserHandleBpStatsReceiverInd((const BROADCAST_PACKETISER_STATS_RECEIVER_IND_T *) message);
            break;

        case BROADCAST_PACKETISER_VOLUME_CHANGE_IND:
            audioInputBroadcastPacketiserHandleVolumeChangeInd((const BROADCAST_PACKETISER_VOLUME_CHANGE_IND_T *) message);
            break;
        case BROADCAST_PACKETISER_SCMST_CHANGE_IND:
            audioInputBroadcastPacketiserHandleScmstChangeInd((const BROADCAST_PACKETISER_SCMST_CHANGE_IND_T *)message);
            break;
        case BROADCAST_PACKETISER_SAMPLE_RATE_CHANGE_IND:
            audioInputBroadcastPacketiserHandleSampleRateChangeInd((const BROADCAST_PACKETISER_SAMPLE_RATE_CHANGE_IND_T *) message);
            break;
        case ERASURE_CODING_RX_STREAM_ID_CHANGE_IND:
            audioInputBroadcastPacketiserHandleEcStreamIdChangeInd((const ERASURE_CODING_RX_STREAM_ID_CHANGE_IND_T *) message);
            break;
        case ERASURE_CODING_RX_AFH_CHANNEL_MAP_CHANGE_PENDING_IND:
            audioInputBroadcastPacketiserHandleEcAfhChannelMapChangePendingInd((const ERASURE_CODING_RX_AFH_CHANNEL_MAP_CHANGE_PENDING_IND_T *) message);
            break;
        case ERASURE_CODING_STATS_RX_IND:
            audioInputBroadcastPacketiserHandleEcStatsReceiverInd((const ERASURE_CODING_STATS_RX_IND_T *) message);
            break;

        /* Not applicable in the receiver role */
        case BROADCAST_PACKETISER_STATS_BROADCASTER_IND:
            /* fall-thru */
        case BROADCAST_PACKETISER_START_OF_STREAM_IND:
            Panic();
            break;

        default:
            PRINT(("BA RX: BP: Unhandled message id %x\n", id));
            break;
    }
}


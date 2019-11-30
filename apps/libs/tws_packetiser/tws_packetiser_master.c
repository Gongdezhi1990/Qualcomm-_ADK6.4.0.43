/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_packetiser_master.c
*/

#include <tws_packetiser.h>
#include <packet_master.h>
#include <tws_packetiser_private.h>
#include <packetiser_helper.h>
#include <panic.h>
#include <string.h>
#include <stdlib.h>
#include <stream.h>

#define TP_INTERNAL_MSG_BASE        (0)
#define TP_INTERNAL_MSG_PERIOD_MS   (25)

typedef enum __tp_internal_msg
{
    TP_INTERNAL_TX_PACKET_MSG = TP_INTERNAL_MSG_BASE,
    TP_INTERNAL_MSG_TOP
} tp_internal_msg_t;

/* Classifications for the amount time before the TTP */
typedef enum __time_before_ttp_state
{
    TIME_BEFORE_TTP_EARLY,
    TIME_BEFORE_TTP_MARGINAL,
    TIME_BEFORE_TTP_LATE
} time_before_ttp_state_t;

typedef enum __process_header_actions
{
    WAIT,
    WRITE_HEADER,
    DROP,
} process_header_action_t;

struct __tws_packetiser_master
{
    /*! Task for this instance of the library */
    TaskData lib_task;

    /*! The wall-clock TTP of the current packet */
    rtime_t ttp_wallclock;

    /*! The local-clock base-TTP of the current packet */
    rtime_t ttp_local;

    /*! The time before the ttp at which the packet was transmitted */
    int32 tx_time_before_ttp;

    /*! The configuration */
    tws_packetiser_master_config_t config;

    /*! An instance of the wallclock state. */
    wallclock_state_t wallclock_state;

    /*! The packet master */
    packet_master_t packet;

    /*! Flag is set to false after the first packet has been processed. */
    bool first_packet;

    /*! The previous value of this state */
    time_before_ttp_state_t time_before_ttp_state_prev;


};

static const packet_master_functions_t *packet_funcs[] = {
    [TWS_PACKETISER_MASTER_MODE_TWS] = &packet_master_funcs_tws,
    [TWS_PACKETISER_MASTER_MODE_RTP] = &packet_master_funcs_rtp,
    [TWS_PACKETISER_MASTER_MODE_TWS_PLUS] = &packet_master_funcs_tws_plus,
    [TWS_PACKETISER_MASTER_MODE_NO_HEADER] = &packet_master_funcs_no_header,
};

/* Write the header to the packet */
static bool tpWriteHeader(tws_packetiser_master_t *tp, rtime_t ttp)
{
    uint32 maxlen = tp->config.mtu;
    uint8 *buffer = tpSinkMapAndClaim(tp->config.sink, maxlen);

    if (buffer)
    {
        PanicFalse(tp->packet.funcs->packetInit(&tp->packet, buffer, maxlen));
        PanicFalse(RtimeLocalToWallClock(&tp->wallclock_state, ttp, &tp->ttp_wallclock));
        tp->packet.funcs->writeHeader(&tp->packet, tp->ttp_wallclock, tp->config.scmst);
        tp->ttp_local = ttp;
        return TRUE;
    }
    else
    {
        TP_DEBUG1("TPMASTER:    tpWriteHeader() failed claimed:%d", SinkClaim(tp->config.sink, 0));
    }
    /* A MESSAGE_MORE_SPACE should arrive later signalling space in the sink.
       If no MESSAGE_MORE_SPACE is received (e.g. a link loss) see MessageSendLater()
       in caller. */
    return FALSE;
}

/* Drop an audio frame from the source */
static void tpDropAudioFrame(tws_packetiser_master_t *tp, audio_frame_metadata_t *fmd)
{
    uint32 frame_len = PanicZero(SourceBoundary(tp->config.source));
    const uint8 *frame_src = PanicNull((uint8*)SourceMap(tp->config.source));

    tp->packet.funcs->droppedAudioFrame(&tp->packet, frame_src, frame_len, fmd);
    SourceDrop(tp->config.source, frame_len);
}

static bool tpSourceHasEnoughDataToFillPacket(tws_packetiser_master_t *tp)
{
    uint32 header_length = tp->packet.funcs->headerLength(&tp->packet, 1);
    return SourceSize(tp->config.source) >= (tp->config.mtu - header_length);
}

static time_before_ttp_state_t tpClassifyTimeBeforeTTP(tws_packetiser_master_t *tp, rtime_t ttp)
{
    time_before_ttp_state_t state;
    int32 time_before_ttp = RtimeTimeBeforeTTP(ttp);
    tp->tx_time_before_ttp = time_before_ttp;
    if (rtime_gt(time_before_ttp, tp->config.time_before_ttp_to_tx))
    {
        state = TIME_BEFORE_TTP_EARLY;
    }
    else if (rtime_lt(time_before_ttp, tp->config.tx_deadline))
    {
        TP_DEBUG1("TPMASTER: frame late %d", time_before_ttp);
        state = TIME_BEFORE_TTP_LATE;
    }
    else
    {
        state = TIME_BEFORE_TTP_MARGINAL;
    }
    return state;
}

static process_header_action_t tpDecideAction(tws_packetiser_master_t *tp, rtime_t ttp)
{
    process_header_action_t action = DROP;
    time_before_ttp_state_t time_before_ttp_state = tpClassifyTimeBeforeTTP(tp, ttp);
    switch (time_before_ttp_state)
    {
        case TIME_BEFORE_TTP_EARLY:
            action = tpSourceHasEnoughDataToFillPacket(tp) ? WRITE_HEADER : WAIT;
        break;

        case TIME_BEFORE_TTP_MARGINAL:
            action = (tp->time_before_ttp_state_prev == TIME_BEFORE_TTP_EARLY) ? WRITE_HEADER : DROP;
        break;        

        case TIME_BEFORE_TTP_LATE:
        default:
            action = DROP;
        break;
    }
    tp->time_before_ttp_state_prev = time_before_ttp_state;
    return action;
}

static bool tpProcessHeader(tws_packetiser_master_t *tp)
{
    audio_frame_metadata_t fmd;

    /* A message may already be queued for the frame at the head of the source -
       this will happen if the time to transmit the frame is in the future, and
       this function is re-called (e.g. because of a MESSAGE_MORE_DATA) before
       the queued message is delivered. In all cases, just cancel any queued
       message - it will be re-sent if necessary */
    PanicFalse(MessageCancelAll(&tp->lib_task, TP_INTERNAL_TX_PACKET_MSG) <= 1);

    while (PacketiserHelperAudioFrameMetadataGetFromSource(tp->config.source, &fmd))
    {
        process_header_action_t action = tpDecideAction(tp, fmd.ttp);
        uint32 delay_ms;
        switch (action)
        {
            case WAIT:
                delay_ms = rtime_sub(tp->tx_time_before_ttp, tp->config.time_before_ttp_to_tx) / US_PER_MS;
                TP_DEBUG3("TPMASTER: ProcessHeader wait %d %d delay_ms %d", tp->tx_time_before_ttp, SourceSize(tp->config.source), delay_ms);
                MessageSendLater(&tp->lib_task, TP_INTERNAL_TX_PACKET_MSG, NULL, delay_ms);
                return FALSE;
            
            case WRITE_HEADER:
                TP_DEBUG1("TPMASTER: ProcessHeader write header %d", tp->tx_time_before_ttp);
                if (tpWriteHeader(tp, fmd.ttp))
                {
                    return TRUE;
                }
                else
                {
                    /* In case no further sink/source messages are received
                       (e.g. sink and source are full), this message will trigger 
                       the packetiser to drop late frames from the audio source. */
                    delay_ms = (tp->tx_time_before_ttp / US_PER_MS);
                    MessageSendLater(&tp->lib_task, TP_INTERNAL_TX_PACKET_MSG, NULL, delay_ms);
                    return FALSE;
                }

            case DROP:
            default:
                tpDropAudioFrame(tp, &fmd);
                TP_DEBUG1("TPMASTER: ProcessHeader drop %d", tp->tx_time_before_ttp);
            break;
        }
    }
    return FALSE;
}

static void tpWriteFrames(tws_packetiser_master_t *tp)
{
    audio_frame_metadata_t fmd;

    while (PacketiserHelperAudioFrameMetadataGetFromSource(tp->config.source, &fmd))
    {
        uint32 frame_len = PanicZero(SourceBoundary(tp->config.source));
        const uint8 *frame_src = PanicNull((uint8*)SourceMap(tp->config.source));

        if (tp->packet.funcs->writeAudioFrame(&tp->packet, frame_src, frame_len, &fmd))
        {
            SourceDrop(tp->config.source, frame_len);
            TP_DEBUG2("TPMASTER:    Wrote Frame: %d %d", fmd.ttp, frame_len);
        }
        else
        {
            TP_DEBUG("TPMASTER:    !Wrote Frame");
            break;
        }
    }
}

static void tpTransmitPacket(tws_packetiser_master_t *tp)
{
    uint32 packet_len;
    tp->packet.funcs->finalise(&tp->packet);
    packet_len = tp->packet.funcs->packetLength(&tp->packet);
    PanicFalse(SinkFlush(tp->config.sink, packet_len));
    TP_DEBUG3("TPMASTER: Transmitted packet with TTP 0x%x, %d, %d",
                tp->ttp_wallclock, RtimeTimeBeforeTTP(tp->ttp_local), packet_len);
}

static void messageHandler(Task task, MessageId id, Message message)
{
    tws_packetiser_master_t *tp =(tws_packetiser_master_t *)task;
    UNUSED(message);
    switch (id)
    {
        case MESSAGE_MORE_DATA:
        case MESSAGE_MORE_SPACE:
        case TP_INTERNAL_TX_PACKET_MSG:
        {
            while (tpProcessHeader(tp))
            {
                tpWriteFrames(tp);
                tpTransmitPacket(tp);

                if(tp->first_packet)
                {
                    tp->first_packet = FALSE;
                    MessageSendLater(tp->config.client, TWS_PACKETISER_FIRST_AUDIO_FRAME_IND,
                            NULL, tp->tx_time_before_ttp/US_PER_MS);
                }
            }
            break;
        }
        case MESSAGE_SOURCE_EMPTY:
            break;
        default:
            TP_DEBUG1("TP: Unhandled message, %u", id);
            break;
    }
}

tws_packetiser_master_t* TwsPacketiserMasterInit(tws_packetiser_master_config_t *config)
{
    tws_packetiser_master_t *tp = NULL;

    /* TODO: reinstate SourceIsValid() when B-231645 is fixed */
    if (SinkIsValid(config->sink) &&
        (NULL != config->source) && /*SourceIsValid(config->source) && */
        config->client &&
        config->mode < TWS_PACKETISER_MASTER_MODES)
    {
        /* Create a new instance of the library */
        tp = calloc(1, sizeof(*tp));
        if (tp)
        {
            tp->packet.funcs = packet_funcs[config->mode];
            if (tp->packet.funcs->init(&tp->packet, config->codec, config->cp_header_enable))
            {
                tp->config = *config;
                TP_DEBUG1("TP: TwsPacketiserMasterInit: codec type %u", config->codec);
                tp->lib_task.handler = messageHandler;

                tp->time_before_ttp_state_prev = TIME_BEFORE_TTP_EARLY;
                tp->first_packet = TRUE;

                MessageStreamTaskFromSink(config->sink, &tp->lib_task);
                MessageStreamTaskFromSource(config->source, &tp->lib_task);

                RtimeWallClockEnable(&tp->wallclock_state, config->sink);

                PanicFalse(SourceMapInit(config->source, STREAM_TIMESTAMPED, AUDIO_FRAME_METADATA_LENGTH));
                SourceConfigure(config->source, VM_SOURCE_MESSAGES, VM_MESSAGES_SOME);
                SinkConfigure(config->sink, VM_SINK_MESSAGES, VM_MESSAGES_SOME);
                
                MessageSend(&tp->lib_task, TP_INTERNAL_TX_PACKET_MSG, NULL);
            }
            else
            {
                free(tp);
                tp = NULL;
            }
        }
    }
    return tp;
}

uint32 TwsPacketiserMasterHeaderLength(tws_packetiser_master_t *tp, uint32 number_audio_frames)
{
    return tp->packet.funcs->headerLength(&tp->packet, number_audio_frames);
}

void TwsPacketiserMasterDestroy(tws_packetiser_master_t *tp)
{
    uint32 i;

    MessageStreamTaskFromSink(tp->config.sink, NULL);
    MessageStreamTaskFromSource(tp->config.source, NULL);
    RtimeWallClockDisable(&tp->wallclock_state);

    SourceUnmap(tp->config.source);
    
    /* Flush / cancel all messages */
    MessageFlushTask(&tp->lib_task);
    for (i = TWS_PACKETISER_MESSAGE_BASE;
         i < TWS_PACKETISER_MESSAGE_TOP;
         i++)
    {
        MessageCancelAll(tp->config.client, i);
    }

    free(tp);
}

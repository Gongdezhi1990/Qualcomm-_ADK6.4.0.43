/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_packetiser_slave.c
*/

#include <tws_packetiser.h>
#include <packet_slave.h>
#include <tws_packetiser_private.h>
#include <packetiser_helper.h>
#include <system_clock.h>
#include <panic.h>
#include <string.h>
#include <stdlib.h>
#include <stream.h>

#define TP_INTERNAL_MSG_BASE 0

/* Frames received with a TTP further in the future than this limit will be dropped */ 
#define FRAME_FUTURE_LIMIT_US 800000

typedef enum __tp_internal_msg
{
    TP_INTERNAL_RX_PACKET_MSG = TP_INTERNAL_MSG_BASE,
    TP_INTERNAL_MSG_TOP
} tp_internal_msg_t;

struct __tws_packetiser_slave
{
    /*! Task for this instance of the library */
    TaskData lib_task;

    /*! The wall-clock TTP of the current packet */
    rtime_t ttp_wallclock;

    /*! The local-clock base-TTP of the current packet */
    rtime_t ttp_local;

    /*! The number of frames in the packet. */
    uint32 frame_count;

    /*! The time before the ttp at which the packet was received */
    int32 rx_time_before_ttp;

    /*! The configuration */
    tws_packetiser_slave_config_t config;

    /*! An instance of the wallclock state. */
    wallclock_state_t wallclock_state;

    /*! Flag is set to false after the first packet has been processed. */
    bool first_packet;

    /*! Mini sample period adjustment is stored when read from packet containing
        first fragment of audio frame. */
    rtime_spadj_mini_t spadj_mini;

    /*! Occasionally, a packet with a new ttp will received having previously received incomplete
        fragments of another frame. The fragments will be written in claimed space in the sink.
        The new fragment will need to overwrite the existing data in the claiming space. This variable
        is used to track the amount of pre-claimed data in the sink */
    uint32 excess_claimed;
};

/* Send message to client when the scmst type changes */
static void handleScmstChange(tws_packetiser_slave_t *tp,
                              packetiser_helper_scmst_t scmst)
{
    if (scmst != tp->config.scmst)
    {
        MESSAGE_MAKE(msg, TWS_PACKETISER_SLAVE_SCMST_CHANGE_IND_T);
        msg->tws_packetiser = tp;
        msg->scmst = scmst;
        MessageSend(tp->config.client, TWS_PACKETISER_SLAVE_SCMST_CHANGE_IND, msg);
        tp->config.scmst = scmst;
    }
}

static uint8 *sinkGetWriteAddr(tws_packetiser_slave_t *tp, uint32 len)
{
    Sink sink = tp->config.sink;
    uint8 *dest = SinkMap(sink);
    if (dest)
    {
        int32 extra = len - tp->excess_claimed;
        uint32 claimed = SinkClaim(sink, 0);
        dest += (claimed - tp->excess_claimed);
        if (extra >= 0)
        {
            if (SinkClaim(sink, extra) == 0xFFFF)
            {
                return NULL;
            }
            tp->excess_claimed = 0;
        }
        else
        {
            tp->excess_claimed = -extra;
        }
    }
    return dest;
}

/* no_mini_spadj is TRUE if the frame has no mini spadj */
static bool audioFrameReadSuccessfully(tws_packetiser_slave_t *tp,
                                       packet_slave_t *tws_packet,
                                       frame_info_t *frame_info,
                                       bool no_mini_spadj,
                                       uint32 *frame_number)
{
    if (no_mini_spadj || tws_packet->funcs->readMiniSpadj(tws_packet, &tp->spadj_mini))
    {
        if (tws_packet->funcs->readAudioFrameInfo(tws_packet, frame_info))
        {
            uint8 *dest = sinkGetWriteAddr(tp, frame_info->length);
            if (dest)
            {
                return tws_packet->funcs->readAudioFrame(tws_packet, dest, frame_info->length, frame_number);
            }
        }
    }
    return FALSE;
}

static bool timeBeforeTtpIsWithinLimits(int32 time_before_ttp)
{
    bool too_early = rtime_gt(time_before_ttp, FRAME_FUTURE_LIMIT_US);
    bool too_late =  rtime_lt(time_before_ttp, 0);
    if (too_early || too_late)
    {
        return FALSE;
    }
    return TRUE;
}

static bool tpReadPacket(tws_packetiser_slave_t *tp)
{
    packet_slave_t tws_packet;
    packetiser_helper_scmst_t scmst;
    audio_frame_metadata_t fmd = {0};
    bool complete;
    rtime_t ttp_wallclock;
    const packet_slave_functions_t *packet_funcs[] = {
        [TWS_PACKETISER_SLAVE_MODE_TWS] = &packet_slave_funcs_tws,
        [TWS_PACKETISER_SLAVE_MODE_TWS_PLUS] = &packet_slave_funcs_tws_plus
    };
    tws_packet.funcs = packet_funcs[tp->config.mode];

    const uint8 *src = SourceMap(tp->config.source);
    uint16 len = SourceBoundary(tp->config.source);

    if (tws_packet.funcs->init(&tws_packet, src, len, &tp->config) &&
        tws_packet.funcs->readHeader(&tws_packet, &ttp_wallclock, &scmst, &complete))
    {
        frame_info_t frame_info;
        bool no_mini_spadj = TRUE;
        uint32 frame_number = 0;
        uint32 ttp_bits = tws_packet.funcs->getTTPLenBits(&tws_packet);
        if (ttp_bits < 32)
        {
            rtime_t wallclock_current;
            RtimeLocalToWallClock(&tp->wallclock_state, SystemClockGetTimerTime(), &wallclock_current);
            RtimeWallClockExtend(&tp->wallclock_state, ttp_wallclock, ttp_bits, wallclock_current, &ttp_wallclock);
        }
        PanicFalse(RtimeWallClockToLocal(&tp->wallclock_state, ttp_wallclock, &fmd.ttp));

        handleScmstChange(tp, scmst);
        tp->ttp_local = fmd.ttp;

        /* A new TTP indicates the start of a new fragment set (or single packet),
           in which case the (or each) audio frame will have a mini spadj.
           All packets containing fragments of the same frame must have the same TTP.
           Only the first fragment of the audio frame is preceeded by a mini spadj */        
        if (ttp_wallclock != tp->ttp_wallclock)
        {
            no_mini_spadj = FALSE;
            tp->ttp_wallclock = ttp_wallclock;
            tp->excess_claimed = SinkClaim(tp->config.sink, 0);
        }

        TP_DEBUG1("TPSLAVE: Received packet with TTP 0x%x", ttp_wallclock);

        /* Read all the frames directly into the sink */
        while(audioFrameReadSuccessfully(tp, &tws_packet, &frame_info, no_mini_spadj, &frame_number))
        {
            if (complete)
            {
                rtime_t frame_time;
                int32 time_before_ttp = RtimeTimeBeforeTTP(fmd.ttp);
                if (timeBeforeTtpIsWithinLimits(time_before_ttp))
                {
                    /* Convert fmd structure to bytes then flush the frame */
                    uint8 fmdbin[AUDIO_FRAME_METADATA_LENGTH];

                    if(0 == frame_number)
                    {
                        tp->rx_time_before_ttp = time_before_ttp;
                    }

                    fmd.sample_period_adjustment = RtimeSpadjMiniToFull(tp->spadj_mini);
                    PacketiserHelperAudioFrameMetadataSet(&fmd, fmdbin);
                    PanicFalse(SinkFlushHeader(tp->config.sink,
                                               SinkClaim(tp->config.sink, 0) - tp->excess_claimed,
                                               fmdbin, sizeof(fmdbin)));
                    TP_DEBUG2("TPSLAVE:    0x%x, %d", fmd.ttp, time_before_ttp);
                }
                else
                {
                    TP_DEBUG2("TPSLAVE:!**!0x%x, %d", fmd.ttp, time_before_ttp);
                }
                /* Update the TTP */
                frame_time = RtimeSamplesToTime(frame_info.samples,
                                                tp->config.sample_rate,
                                                fmd.sample_period_adjustment);
                fmd.ttp = rtime_add(fmd.ttp, frame_time);
            }
        }

        tws_packet.funcs->unInit(&tws_packet);
    }
    if (len)
    {
        SourceDrop(tp->config.source, len);
        return TRUE;
    }
    return FALSE;
}

static void messageHandler(Task task, MessageId id, Message message)
{
    tws_packetiser_slave_t *tp =(tws_packetiser_slave_t *)task;
    UNUSED(message);
    switch (id)
    {
        case MESSAGE_MORE_DATA:
        case MESSAGE_MORE_SPACE:
        case TP_INTERNAL_RX_PACKET_MSG:
            /* Read all the packets available */
            while(tpReadPacket(tp))
            {
                if(tp->first_packet)
                {
                    tp->first_packet = FALSE;
                    MessageSendLater(tp->config.client, TWS_PACKETISER_FIRST_AUDIO_FRAME_IND,
                            NULL, tp->rx_time_before_ttp/US_PER_MS);
                }
            }
            break;
        case MESSAGE_SOURCE_EMPTY:
            break;
        default:
            TP_DEBUG1("TP: Unhandled message, %u", id);
            break;
    }
}

tws_packetiser_slave_t* TwsPacketiserSlaveInit(tws_packetiser_slave_config_t *config)
{
    tws_packetiser_slave_t *tp = NULL;

    /* TODO: reinstate SinkIsValid() when B-231645 is fixed */
    if ((NULL != config->sink) && /*SinkIsValid(config->sink) &&*/
        SourceIsValid(config->source) &&
        config->client &&
        config->mode < TWS_PACKETISER_SLAVE_MODES)
    {
        /* Create a new instance of the library */
        tp = calloc(1, sizeof(*tp));
        if (tp)
        {
            tp->config = *config;
            TP_DEBUG1("TP: TwsPacketiserSlaveInit: codec type %u", config->codec);
            tp->lib_task.handler = messageHandler;
            tp->first_packet = TRUE;
            tp->excess_claimed = 0;

            MessageStreamTaskFromSink(config->sink, &tp->lib_task);
            MessageStreamTaskFromSource(config->source, &tp->lib_task);

            RtimeWallClockEnable(&tp->wallclock_state, StreamSinkFromSource(config->source));

            PanicFalse(SinkMapInit(config->sink, STREAM_TIMESTAMPED, AUDIO_FRAME_METADATA_LENGTH));
            SourceConfigure(config->source, VM_SOURCE_MESSAGES, VM_MESSAGES_SOME);
            SinkConfigure(config->sink, VM_SINK_MESSAGES, VM_MESSAGES_SOME);

            MessageSend(&tp->lib_task, TP_INTERNAL_RX_PACKET_MSG, NULL);
        }
    }
    return tp;
}

void TwsPacketiserSlaveDestroy(tws_packetiser_slave_t *tp)
{
    uint32 i;

    MessageStreamTaskFromSink(tp->config.sink, NULL);
    MessageStreamTaskFromSource(tp->config.source, NULL);
    RtimeWallClockDisable(&tp->wallclock_state);
    
    SinkUnmap(tp->config.sink);

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

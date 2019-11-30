/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       
\brief      Implementation of SCO forwarding.

            This file covers management of a single L2CAP link to the peer device
            and transmission of data over the link.

            There is some interaction with \ref av_headset_kymera.c, and 
            \ref av_headset_con_manager.c.
*/
#define SFWD_DEBUG

#include <panic.h>
#include <ps.h>
#include <system_clock.h>
#include <rtime.h>
#include <bluestack/hci.h>
#include <bluestack/l2cap_prim.h>
#include <tws_packetiser.h>
#include <util.h>
#include <service.h>
#include "av_headset.h"
#include "av_headset_log.h"
#include "av_headset_sdp.h"

/*  This is the number of message sends from scofwd calling
    SendOTAControlMessage(SFWD_OTA_MSG_SETUP);
    to the AVRCP flushing the message to the sink. It comprises:
        SendOTAControlMessage()
            appPeerSigMsgChannelTxRequest() -> PEER_SIG_INTERNAL_MSG_CHANNEL_TX_REQ
        appPeerSigHandleInternalMsgChannelTxRequest()
            appPeerSigVendorPassthroughRequest()
                appAvrcpVendorPassthroughRequest() -> AV_INTERNAL_AVRCP_VENDOR_PASSTHROUGH_REQ
        appAvrcpHandleInternalAvrcpVendorPassthroughRequest()
            AvrcpPassthroughRequest() -> AVRCP_INTERNAL_PASSTHROUGH_REQ
        avrcpHandleInternalPassThroughReq()
            avrcpAvctpSendMessage()
                SinkFlush()
    For SCO forwarding, the Kymera SCO start is delayed by this number of message
    sends to allow the OTA control message to be flushed before incurring the long,
    blocking kymera start calls.
*/
#define SFWD_SCO_START_MSG_DELAY 4

/* This is the maximum number of messages taken to suspend A2DP if it was active
    when SCO forwarding receive was received. Message is resent if av is streaming.

    AV_INTERNAL_A2DP_SUSPEND_MEDIA_REQ
    KYMERA_INTERNAL_A2DP_STOP
 */
#define SFWD_SCO_RX_START_MSG_DELAY 2



#define SCOFWD_TRANS_DEBUG
#ifdef SCOFWD_TRANS_DEBUG
uint16 ScoFwd_trans_debug_limit = 10;
uint16 ScoFwd_trans_debug_tx_count = 0;
uint16 ScoFwd_trans_debug_rx_count = 0;
#define SCOFWD_TRANS_TX_DEBUG_LOG(x)           if (ScoFwd_trans_debug_tx_count == ScoFwd_trans_debug_limit-1) { \
                                                   DEBUG_LOG(x); \
                                               } \
                                               ScoFwd_trans_debug_tx_count++; \
                                               ScoFwd_trans_debug_tx_count = ScoFwd_trans_debug_tx_count % ScoFwd_trans_debug_limit;

#define SCOFWD_TRANS_TX_DEBUG_LOGF(x, ...)     if (ScoFwd_trans_debug_tx_count == ScoFwd_trans_debug_limit-1) { \
                                                   DEBUG_LOGF(x, __VA_ARGS__); \
                                               } \
                                               ScoFwd_trans_debug_tx_count++; \
                                               ScoFwd_trans_debug_tx_count = ScoFwd_trans_debug_tx_count % ScoFwd_trans_debug_limit;

#define SCOFWD_TRANS_RX_DEBUG_LOG(x)           if (ScoFwd_trans_debug_rx_count == ScoFwd_trans_debug_limit-1) { \
                                                   DEBUG_LOG(x); \
                                               } \
                                               ScoFwd_trans_debug_rx_count++; \
                                               ScoFwd_trans_debug_rx_count = ScoFwd_trans_debug_rx_count % ScoFwd_trans_debug_limit;
#define SCOFWD_TRANS_RX_DEBUG_LOGF(x, ...)     if (ScoFwd_trans_debug_rx_count == ScoFwd_trans_debug_limit-1) { \
                                                   DEBUG_LOGF(x, __VA_ARGS__); \
                                               } \
                                               ScoFwd_trans_debug_rx_count++; \
                                               ScoFwd_trans_debug_rx_count = ScoFwd_trans_debug_rx_count % ScoFwd_trans_debug_limit;
#else   /* SCOFWD_TRANS_DEBUG */
#define SCOFWD_TRANS_TX_DEBUG_LOG(x)        DEBUG_LOG(x)
#define SCOFWD_TRANS_TX_DEBUG_LOGF(x)       DEBUG_LOGF(x, __VA_ARGS__)
#define SCOFWD_TRANS_RX_DEBUG_LOG(x)        DEBUG_LOG(x)
#define SCOFWD_TRANS_RX_DEBUG_LOGF(x)       DEBUG_LOGF(x, __VA_ARGS__)
#endif  /* SCOFWD_TRANS_DEBUG */


#define US_TO_MS(us) ((us) / 1000)

#define SCO_METADATA_OK                 0
#define SCO_METADATA_NOTHING_RECEIVED   2

#define appGetScoFwdStats()     (&globalApp.scoFwd.stats)
#define appScoFwdGetState()     appGetScoFwd()->state

static void appScoFwdProcessReceivedAirPacket(uint16 avail);
void ttp_stats_print(void);

/* SCO TTP Management functions
 The following are used for recording a received TTP, finding the next one
 and identifying any duplicates from late packets / race conditions.
*/
static bool appScoFwdTTPIsExpected(rtime_t new_ttp);
static void set_last_received_ttp(rtime_t ttp_passed_down);
static void clear_last_received_ttp(void);
static bool get_next_expected_ttp(rtime_t *next_ttp);
static bool get_next_ttp_before(rtime_t received_ttp,rtime_t *target_time);
static bool have_no_received_ttp(void);

/* Functions to manage packets arriving late.
   A timer is set for an interval before the expected TTP
   such that we have time to trigger a fake packet.
 */
static void handle_late_packet_timer(void);
static void start_late_packet_timer(rtime_t last_received_TTP);
static void cancel_late_packet_timer(void);

    /* Which leads to faking packets at just the right time */
static void insert_fake_packet_at(rtime_t new_ttp,rtime_t debug_ttp);
static void insert_fake_packets_before(rtime_t next_received_ttp,rtime_t debug_ttp);

static void appScoFwdProcessReceivedAirFrame(const uint8 **pSource, uint8 frame_length);
static void appScoFwdKickProcessing(void);
static void appScoFwdSetState(scoFwdState new_state);

static void appScoFwdNotifyAudioDisappeared(void);

static void appScoFwdSendHfpVolumeToSlave(uint8 volume);

/* Value used to set metadata for SCO blocks sent into audio chain */
static uint16 sco_metadata_used_btclock = 0;


#define SHORT_TTP(x) (((x)/1000)%1000)

#define SFWD_NO_RECEIVED_AUDIO(scofwd) ((scofwd)->lost_packets == 32)

#define ALL_SINKS_CONFIGURED(scofwd) ((scofwd)->link_sink && (scofwd)->forwarded_sink)
#define ALL_SOURCES_CONFIGURED(scofwd) ((scofwd)->link_source && (scofwd)->forwarding_source)

#define appScoFwdInActiveState() (   SFWD_STATE_CONNECTED_ACTIVE_RECEIVE == appScoFwdGetState() \
                                  || SFWD_STATE_CONNECTED_ACTIVE_SEND == appScoFwdGetState())

/*! Macro for creating messages */
#define MAKE_SFWD_INTERNAL_MESSAGE(TYPE) \
    SFWD_INTERNAL_##TYPE##_T *message = PanicUnlessNew(SFWD_INTERNAL_##TYPE##_T);


/*! \brief Internal message IDs */
enum
{
    SFWD_INTERNAL_BASE,
    SFWD_INTERNAL_LINK_CONNECT_REQ = SFWD_INTERNAL_BASE,
    SFWD_INTERNAL_LINK_DISCONNECT_REQ,
    SFWD_INTERNAL_START_RX_CHAIN,
    SFWD_INTERNAL_STOP_RX_CHAIN,
    SFWD_INTERNAL_KICK_PROCESSING,
    SFWD_INTERNAL_MIC_CHAIN_DETAILS,
    SFWD_INTERNAL_RX_AUDIO_MISSING,
    SFWD_INTERNAL_ENABLE_FORWARDING,
    SFWD_INTERNAL_DISABLE_FORWARDING,
    SFWD_INTERNAL_PLAY_RING,
    SFWD_INTERNAL_ROLE_NOTIFY,

    SFWD_TIMER_BASE = SFWD_INTERNAL_BASE + 0x80,
    SFWD_TIMER_LATE_PACKET = SFWD_TIMER_BASE,
};


typedef struct
{
    Sink    sink;
} SFWD_INTERNAL_CHAIN_DETAILS_T;

typedef struct
{
    hci_role    role;
} SFWD_INTERNAL_ROLE_NOTIFY_T;

typedef SFWD_INTERNAL_CHAIN_DETAILS_T SFWD_INTERNAL_MIC_CHAIN_DETAILS_T;
#define TTP_STATS_RANGE         (appConfigScoFwdVoiceTtpMs())
#define TTP_STATS_NUM_CELLS     20
#define TTP_STATS_CELL_SIZE     (TTP_STATS_RANGE / TTP_STATS_NUM_CELLS)
#define TTP_STATS_MIN_VAL       (  appConfigScoFwdVoiceTtpMs() \
                                 - US_TO_MS(SFWD_RX_PROCESSING_TIME_NORMAL_US) \
                                 - TTP_STATS_NUM_CELLS * TTP_STATS_CELL_SIZE)
#define TTP_STATS_MAX_VAL       (TTP_STATS_MIN_VAL + TTP_STATS_CELL_SIZE * TTP_STATS_NUM_CELLS)

typedef struct {
    uint32 entries;
    uint32 sum;
} TTP_STATS_CELL;
TTP_STATS_CELL ttp_stats[TTP_STATS_NUM_CELLS+2] = {0};


static void ttp_stats_add(unsigned ttp_in_future_ms)
{
    int cell;

    if (ttp_in_future_ms < TTP_STATS_MIN_VAL)
    {
        cell = TTP_STATS_NUM_CELLS;
    }
    else if (ttp_in_future_ms >= TTP_STATS_MAX_VAL)
    {
        cell = TTP_STATS_NUM_CELLS + 1;
    }
    else 
    {
        cell = (ttp_in_future_ms - TTP_STATS_MIN_VAL) / TTP_STATS_CELL_SIZE;
        if (cell > TTP_STATS_NUM_CELLS + 1)
            Panic();
    }

    ttp_stats[cell].entries++;
    ttp_stats[cell].sum += ttp_in_future_ms;
}

static void ttp_stats_print_cell(unsigned minval,unsigned maxval,unsigned entries,unsigned average)
{
    UNUSED(minval);
    UNUSED(maxval);
    UNUSED(entries);
    UNUSED(average);
    DEBUG_LOGF("\t%4d,%-4d,\t%6d,\t%4d",minval,maxval,entries,average);
}

void ttp_stats_print(void)
{
    int cell;

    DEBUG_LOG("TTP STATS");
    DEBUG_LOG("\tCELL RANGE,\tNum,\tAverage");

    for (cell = 0;cell < TTP_STATS_NUM_CELLS;cell++)
    {
        unsigned minval = TTP_STATS_MIN_VAL + (cell * TTP_STATS_CELL_SIZE);
        unsigned maxval = minval + TTP_STATS_CELL_SIZE - 1;
        unsigned average = ttp_stats[cell].entries ? ((ttp_stats[cell].sum + ttp_stats[cell].entries/2)/ttp_stats[cell].entries) : 0;
        ttp_stats_print_cell(minval,maxval,ttp_stats[cell].entries,average);
    }
    if (ttp_stats[cell].entries)
    {
        ttp_stats_print_cell(-1000,TTP_STATS_MIN_VAL -1,ttp_stats[cell].entries,(ttp_stats[cell].sum + ttp_stats[cell].entries/2)/ttp_stats[cell].entries);
    }
    cell++;
    if (ttp_stats[cell].entries)
    {
        ttp_stats_print_cell(TTP_STATS_MAX_VAL+1,1000,ttp_stats[cell].entries,(ttp_stats[cell].sum + ttp_stats[cell].entries/2)/ttp_stats[cell].entries);
    }
}

#ifndef INCLUDE_SCOFWD_TEST_MODE
#define appScoFwdDropPacketForTesting() FALSE
#else
static bool appScoFwdDropPacketForTesting(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    static unsigned consecutive_packets_dropped = 0;

    if (theScoFwd->percentage_to_drop)
    {
        unsigned random_percent = UtilRandom() % 100;

        /* First decide about any packet repeat.
           That way, if we are not repeating on purpose - we then
           check against the normal drop percentage.
           Without this the actual %age dropped is noticably off. */

        if (consecutive_packets_dropped)
        {
            if (theScoFwd->drop_multiple_packets < 0)
            {
                if (consecutive_packets_dropped < -theScoFwd->drop_multiple_packets)
                {
                    consecutive_packets_dropped++;
                    if (consecutive_packets_dropped == -theScoFwd->drop_multiple_packets)
                    {
                        consecutive_packets_dropped = 0;
                    }
                    return TRUE;
                }
            }
            else
            {
                if (random_percent < theScoFwd->drop_multiple_packets)
                {
                    return TRUE;
                }
            }
            consecutive_packets_dropped = 0;
        }

        if (random_percent  < theScoFwd->percentage_to_drop)
        {
            consecutive_packets_dropped = 1;
            return TRUE;
        }
    }

    consecutive_packets_dropped = 0;
    return FALSE;
}
#endif /* INCLUDE_SCOFWD_TEST_MODE */


/* Track good/bad packets */
static void updatePacketStats(bool good_packet)
{
    scoFwdReceivedPacketStats *stats = appGetScoFwdStats();

    stats->lost_packets -= ((stats->packet_history & 0x80000000u) == 0x80000000u);
    stats->lost_packets += (good_packet == FALSE);
    stats->packet_history <<= 1;
    stats->packet_history |= (good_packet == FALSE);

}

/* Track good/bad packets */
static void clearPacketStats(void)
{
    scoFwdReceivedPacketStats *stats = appGetScoFwdStats();

    stats->lost_packets = 0;
    stats->packet_history = 0;
    stats->audio_missing = FALSE;
}

static void actionPacketStats(void)
{
    scoFwdReceivedPacketStats *stats = appGetScoFwdStats();

    if (SFWD_NO_RECEIVED_AUDIO(stats))
    {
        if (!stats->audio_missing)
        {
            stats->audio_missing = TRUE;
            appScoFwdNotifyAudioDisappeared();
        }
    }
    else
    {
        stats->audio_missing = FALSE;
    }
}



static uint8 *sfwd_tx_help_write_ttp(uint8* buffer,rtime_t ttp)
{
    *buffer++ = (ttp >> 16) & 0xff;
    *buffer++ = (ttp >> 8) & 0xff;
    *buffer++ = ttp & 0xff;
    return buffer;
}


static const uint8 *sfwd_rx_help_read_ttp(const uint8* buffer,rtime_t *ttp)
{
    rtime_t result;
    result = *buffer++ & 0xFF;
    result = (result << 8) + (*buffer++ & 0xFF);
    result = (result << 8) + (*buffer++ & 0xFF);
    *ttp = result;
    return buffer;
}


static uint8* uint16Write(uint8 *dest, uint16 val)
{
    dest[0] = val & 0xff;
    dest[1] = (val >> 8) & 0xff;
    return dest + 2;
}

static void ScoMetadataAdvance(void)
{
    sco_metadata_used_btclock += 24;
}


/*! \brief The Async WBS Decoder expects to have time information about the
    packet, which is information normally supplied by the SCO endpoint.
    Populate this here.
    \param buffer Address to write metadata.
    \param missing_packet FALSE if the metadata describes a normally
    received packet, TRUE if the metadata describes a missed packet, which
    should cause the async WBC decoder to invoke PLC. */
static uint8 *ScoMetadataSet(uint8 *buffer, bool missing_packet)
{
    buffer = uint16Write(buffer, 0x5c5c);
    buffer = uint16Write(buffer, 5);
    buffer = uint16Write(buffer, missing_packet ? 0 : SFWD_AUDIO_FRAME_OCTETS);
    buffer = uint16Write(buffer, missing_packet ? SCO_METADATA_NOTHING_RECEIVED : SCO_METADATA_OK);
    buffer = uint16Write(buffer, sco_metadata_used_btclock);
    return buffer;
}


/*  Create a timer so that we are able to substitute a packet in time for it to be
    sent to the speaker. 

    In normal processing this timer is sometime in the future, and will normally be
    cancelled when an audio packet is received. 

    When there is a long gap in audio, then a late packet timer will follow a late 
    packet timer and we are more prone to the expected time for the timer being in 
    the past. In these cases send the timer expiry immediately.
 */
static void start_late_packet_timer(rtime_t ttp)
{
    rtime_t target_callback_time = rtime_sub(ttp,SFWD_LATE_PACKET_OFFSET_TIME_US);
    rtime_t now = SystemClockGetTimerTime();
    int32 ms_delay = US_TO_MS(rtime_add(rtime_sub(target_callback_time, now), 999));

    cancel_late_packet_timer();

    if (ms_delay < appConfigScoFwdVoiceTtpMs())
    {
        if (ms_delay < 0)
        {
            /* If expected time is a reasonable time in the past, process immediately */
            if (ms_delay > -appConfigScoFwdVoiceTtpMs())
            {
                MessageSend(appGetScoFwdTask(), SFWD_TIMER_LATE_PACKET, NULL);
            }
            else
            {
                DEBUG_LOGF("start_late_packet_timer. Timer too far in the past to process %d",ms_delay);
            }
        }
        else
        {
            MessageSendLater(appGetScoFwdTask(), SFWD_TIMER_LATE_PACKET, NULL, ms_delay);
        }
    }
}

static void cancel_late_packet_timer(void)
{
    uint16 cancel_count = MessageCancelAll(appGetScoFwdTask(), SFWD_TIMER_LATE_PACKET);

    if (cancel_count  > 1)
    {
        DEBUG_LOGF("cancel_late_packet_timer. More than one time cancelled - %d ???",cancel_count);
    }
}


static void handle_late_packet_timer(void)
{
    rtime_t next_ttp;

    if (get_next_expected_ttp(&next_ttp))
    {
        insert_fake_packet_at(next_ttp,0);
    }
    else
    {
        DEBUG_LOG("handle_late_packet_timer. Late packet timer expired, but nothing to do.");
    }

}

void insert_fake_packet_at(rtime_t new_ttp,rtime_t debug_ttp)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    uint8 hdr[AUDIO_FRAME_METADATA_LENGTH];
    uint16 offset;

    start_late_packet_timer(new_ttp);

    if (appScoFwdTTPIsExpected(new_ttp))
    {
        updatePacketStats(FALSE);
        actionPacketStats();

        if (debug_ttp)
        {
            DEBUG_LOGF("FAKE-CATCHUP @ %d 0x%06x",SHORT_TTP(new_ttp),debug_ttp);
        }
        else
        {
            DEBUG_LOGF("FAKE-TIMEOUT @ %d",SHORT_TTP(new_ttp));
        }
        if ((offset = SinkClaim(theScoFwd->forwarded_sink,SFWD_WBS_DEC_KICK_SIZE)) != 0xFFFF)
        {
            uint8* snk = SinkMap(theScoFwd->forwarded_sink) + offset;
            audio_frame_metadata_t md = {0,0,0};
            ScoMetadataSet(snk, TRUE);
            md.ttp = new_ttp;

            PacketiserHelperAudioFrameMetadataSet(&md, hdr);

            SinkFlushHeader(theScoFwd->forwarded_sink,
                    SFWD_WBS_DEC_KICK_SIZE,hdr,AUDIO_FRAME_METADATA_LENGTH);
        }
        ScoMetadataAdvance();

        /* Advance the TTP (by saving it), even if we could not write the fake */
        set_last_received_ttp(new_ttp);
    }
    else
    {
        if (debug_ttp)
        {
            DEBUG_LOGF("NO-CATCHUP @ %d %d",SHORT_TTP(new_ttp),debug_ttp);
        }
        else
        {
            DEBUG_LOGF("NO-TIMEOUT @ %d",SHORT_TTP(new_ttp));
        }
    }
}


static bool appScoFwdTTPIsExpected(rtime_t new_ttp)
{
    rtime_t expected_ttp;

    if (!get_next_expected_ttp(&expected_ttp))
    {
        return TRUE;
    }
    if (rtime_lt(new_ttp,rtime_sub(expected_ttp,SFWD_PACKET_INTERVAL_MARGIN_US)))
    {
        return FALSE;
    }
    return TRUE;
}

static void set_last_received_ttp(rtime_t ttp_passed_down)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    /* Make sure the low bit of TTP is set so it's distinct from 0 */
    theScoFwd->ttp_of_last_received_packet = ttp_passed_down | 1; 
}

static void clear_last_received_ttp(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    theScoFwd->ttp_of_last_received_packet = 0;
}

static bool have_no_received_ttp(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    return (theScoFwd->ttp_of_last_received_packet == 0);
}

static bool get_next_expected_ttp(rtime_t *next_ttp)
{
    if (next_ttp)
    {
        *next_ttp = 0;

        if (!have_no_received_ttp())
        {
            scoFwdTaskData *theScoFwd = appGetScoFwd();

            *next_ttp = rtime_add(theScoFwd->ttp_of_last_received_packet, SFWD_PACKET_INTERVAL_US);
            return TRUE;
        }
    }
    return FALSE;
}


static bool get_next_ttp_before(rtime_t received_ttp,rtime_t *target_time)
{
    rtime_t next_expected_ttp;

    if (target_time)
    {
        *target_time = 0;

        if (get_next_expected_ttp(&next_expected_ttp))
        {
            if (rtime_gt(received_ttp,rtime_add(next_expected_ttp, SFWD_PACKET_INTERVAL_MARGIN_US)))
            {
                *target_time = next_expected_ttp;
                return TRUE;
            }
        }
    }

    return FALSE;
}

/** Insert any fake packets needed, on the assumption that the supplied
    TTP is the time for the packet we have just received.
 */
static void insert_fake_packets_before(rtime_t next_received_ttp,rtime_t debug_ttp)
{
    rtime_t target_time;

    while (get_next_ttp_before(next_received_ttp,&target_time))
    {
        insert_fake_packet_at(target_time,debug_ttp);
    }
}

/*! Check the contents of a WBS frame about to be sent to the air.
    We substitute a similar header on the receiving side, so this
    acts as a sanity check for unexpected behaviour 

    \todo Consider removing panic before full release
    */
static void check_valid_WBS_frame_header(const uint8 *pSource)
{
    if (    pSource[0] != 0x01
         || (   pSource[1] != 0x08
             && pSource[1] != 0x38
             && pSource[1] != 0xC8
             && pSource[1] != 0xF8)
         || pSource[2] != 0xAD
         || pSource[3] != 0x00
         || pSource[4] != 0x00)
    {
         DEBUG_LOGF("Unexpected WBS frame to air. Header %02X %02X %02X %02X",pSource[0],pSource[1],pSource[2],pSource[3]);
         /* \todo reinstate gingerly... */
//         Panic();
    }
}

static void sfwd_tx_queue_next_packet(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    Source audio_source = theScoFwd->forwarding_source;
    audio_frame_metadata_t md;
    bool frame_sent = FALSE;
    Sink air_sink = theScoFwd->link_sink;
    uint16 packet_size = 0;
    int32 future_ms=0;
    uint16 total_available_data = SourceSize(audio_source);

    if (total_available_data  < SFWD_AUDIO_FRAME_OCTETS)
    {
        return;
    }

    uint16 estimated_frames = total_available_data / SFWD_AUDIO_FRAME_OCTETS;
    if (estimated_frames > SFWD_TX_PACKETISER_MAX_FRAMES_BEHIND)
    {
        uint16  boundary;
        DEBUG_LOGF("TOO MANY FRAMES. %d",estimated_frames);

        while (   estimated_frames > SFWD_TX_PACKETISER_MAX_FRAMES_BEHIND
               && 0 != (boundary = SourceBoundary(audio_source)))
        {
            estimated_frames--;
            SourceDrop(audio_source,boundary);
        }
    }

    /* We only send one packet here, but may also discard some - so use a loop */
    while (   PacketiserHelperAudioFrameMetadataGetFromSource(audio_source,&md) 
           && (!frame_sent))
    {
        rtime_t ttp_in = md.ttp;
        uint16 avail = SourceBoundary(audio_source);
        int32 diff = rtime_sub(ttp_in,SystemClockGetTimerTime());

        if (diff < SFWD_MIN_TRANSIT_TIME_US)
        {
            DEBUG_LOGF("DISCARD (%dms). Now %8d TTP %8d",US_TO_MS(diff),
                                    SystemClockGetTimerTime(),ttp_in);
            SourceDrop(audio_source,avail);
            continue;
        }

        if (appScoFwdDropPacketForTesting())
        {
            SourceDrop(audio_source,avail);
            continue;
        }

        packet_size =   avail
                      + SFWD_TX_PACKETISER_FRAME_HDR_SIZE
                      - SFWD_STRIPPED_HEADER_SIZE;

        uint16 offset = SinkClaim(air_sink, packet_size);
        if (offset == 0xFFFF)
        {
            /* No space for this packet, so exit loop as 
               wont be any space for more packets */
            DEBUG_LOG("Dropped TX packet as buffer full");
            SourceDrop(audio_source,avail);
            break;
        }
        uint8 *base = SinkMap(air_sink);
        
        rtime_t ttp_out;
        RtimeLocalToWallClock(&theScoFwd->wallclock,ttp_in,&ttp_out);

        future_ms = US_TO_MS(diff);
        ttp_stats_add(future_ms);

        uint8 *framebase = base + offset;
        uint8 *writeptr = framebase;

        writeptr = sfwd_tx_help_write_ttp(writeptr,ttp_out);
        const uint8 *pSource = SourceMap(audio_source);

        /* Copy audio data into buffer to the air, removing the header */
        memcpy(writeptr,&pSource[SFWD_STRIPPED_HEADER_SIZE],avail - SFWD_STRIPPED_HEADER_SIZE);

        check_valid_WBS_frame_header(pSource);

        SourceDrop(audio_source,avail);
        SinkFlush(air_sink, packet_size);
        frame_sent = TRUE;

        SCOFWD_TRANS_TX_DEBUG_LOGF("TX 1 frame [%3d octets]. TTP in future by %dms",packet_size,future_ms);
    }
}

static void SendOTAControlMessageWithPayload(uint8 ota_msg_id, const uint8* payload, size_t payload_size)
{
    bdaddr peer;
    int message_size = payload_size + 1;    /* +1 is for the ota_msg_id over the payload size */
    uint8* message = NULL;
    
    /* build message with payload if required */
    if (payload)
    {
        message = PanicUnlessMalloc(message_size);
        message[0] = ota_msg_id;
        memcpy(&message[1], payload, payload_size);
    }

    if (appDeviceGetPeerBdAddr(&peer))
    {
        DEBUG_LOGF("SendOTAControlMessageWithPayload. OTA CMD 0x%02x requested",ota_msg_id);
        appPeerSigMsgChannelTxRequest(appGetScoFwdTask(),
                                      &peer,
                                      PEER_SIG_MSG_CHANNEL_SCOFWD,
                                      payload ? message : &ota_msg_id,
                                      message_size);
    }
    else
    {
        DEBUG_LOGF("SendOTAControlMessageWithPayload. OTA CMD 0x%02x discarded. NO PEER?",ota_msg_id);
    }

    if (message)
    {
        free(message);
    }
}

static void SendOTAControlMessage(uint8 ota_msg_id)
{
    SendOTAControlMessageWithPayload(ota_msg_id, NULL, 0);
}

static void appScoFwdProcessForwardedAudio(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    uint16 avail;

    while ((avail = SourceBoundary(theScoFwd->link_source)) != 0)
    {
        appScoFwdProcessReceivedAirPacket(avail);
    }
}

/*! Handle incoming new SCO volume, apply locally */
static void appScoFwdHandleOTASetVolume(const uint8* msg, int msg_size)
{
    UNUSED(msg_size);
    DEBUG_LOGF("New volume from master %u", msg[0]); 
    appKymeraScoSetVolume(msg[0]);
}

static void appScoFwdPlayRingAtWcTime(rtime_t sync_time)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    DEBUG_LOG("appScoFwdHandleOTARing");

    wallclock_state_t wc_state;

    if (RtimeWallClockGetStateForSink(&wc_state, theScoFwd->wallclock_sink))
    {
        rtime_t local;

        if (RtimeWallClock24ToLocal(&wc_state, sync_time, &local))
        {
            /* Convert to milliseconds in the future */
            int32_t delay = rtime_sub(local, VmGetTimerTime()) / 1000;

            /* If value is negative, do not ring */
            if(delay < 0)
            {
                DEBUG_LOG("appScoFwdHandleOTARing delay is negative, don't ring.");
                return;
            }

            DEBUG_LOGF("appScoFwdHandleOTARing sync_time: %u, delay: %u", sync_time, delay);

            /* Wait for specified delay */
            MessageSendLater(&theScoFwd->task, SFWD_INTERNAL_PLAY_RING, 0, delay);
        }
        else
            Panic();
    }

}

/*! Handle OTA ring, use the BT wallclock to synchronize the starting of the ring tones. */
static void appScoFwdHandleOTARing(const uint8* msg, int msg_size)
{
    if(msg_size == 3)
    {
        rtime_t sync_time;

        sfwd_rx_help_read_ttp(msg, &sync_time);
        appScoFwdPlayRingAtWcTime(sync_time);
    }
}

/*! \brief Utility funtion to read a signed 16-bit value from OTA payload. */
static int16 appScoFwdOTAPayloadReadInt16(const uint8* payload)
{
    return (int16)(((uint16)(payload[1] << 8)) | ((uint16)(payload[0])));
}

/*! \brief Utility funtion to write a signed 16-bit value to OTA payload. */
static void appScoFwdOTAPayloadWriteInt16(uint8* payload, int16 val)
{
    payload[0] = val & 0xFF;
    payload[1] = (val >> 8) & 0xFF;
}

static void ProcessOTAControlMessage(uint8 ota_msg_id, const uint8* payload, int payload_size)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    DEBUG_LOGF("ProcessOTAControlMessage. OTA message ID 0x%02X",ota_msg_id);

    switch (ota_msg_id)
    {
        case SFWD_OTA_MSG_SETUP:
            MessageSend(appGetScoFwdTask(), SFWD_INTERNAL_START_RX_CHAIN, NULL);
            break;

        case SFWD_OTA_MSG_TEARDOWN:
            MessageSend(appGetScoFwdTask(), SFWD_INTERNAL_STOP_RX_CHAIN, NULL);
            break;

        case SFWD_OTA_MSG_MIC_SETUP:
            DEBUG_LOG("Received indication that MIC set up remotely");
            /* Now we can configure the MIC forwarding as required */
            appScoFwdMicForwardingEnable(appGetKymera()->mic == MIC_SELECTION_REMOTE);
            break;

        case SFWD_OTA_MSG_INCOMING_CALL:
            DEBUG_LOG("SCO Forwarding notified of incoming call");
            theScoFwd->peer_incoming_call = TRUE;
            break;

        case SFWD_OTA_MSG_INCOMING_ENDED:
            DEBUG_LOG("SCO Forwarding notified of incoming call END");
            theScoFwd->peer_incoming_call = FALSE;            
            appScoFwdRingCancel();
            break;

        case SFWD_OTA_MSG_SET_VOLUME:
            appScoFwdHandleOTASetVolume(payload, payload_size);
            break;

        case SFWD_OTA_MSG_CALL_ANSWER:
            DEBUG_LOG("SCO Forwarding PEER ANSWERING call");
            appHfpCallAccept();
            break;

        case SFWD_OTA_MSG_CALL_REJECT:
            DEBUG_LOG("SCO Forwarding PEER REJECTING call");
            appHfpCallReject();
            break;

        case SFWD_OTA_MSG_CALL_HANGUP:
            DEBUG_LOG("SCO Forwarding PEER ending call");
            appHfpCallHangup();
            break;

        case SFWD_OTA_MSG_CALL_VOICE:
            DEBUG_LOG("SCO Forwarding PEER call voice");
            appHfpCallVoice();
            break;

        case SFWD_OTA_MSG_VOLUME_START:
            {
                int16 steps = appScoFwdOTAPayloadReadInt16(payload);
                DEBUG_LOGF("SCO Forwarding PEER sent volume change start %u", steps);
                if (appHfpIsScoActive() || appHfpIsConnected())
                {
                    appHfpVolumeStart(steps);
                }
            }
            break;
        case SFWD_OTA_MSG_VOLUME_STOP:
            {
                int16 steps = appScoFwdOTAPayloadReadInt16(payload);
                DEBUG_LOGF("SCO Forwarding PEER sent volume change stop %u", steps);
                if (appHfpIsScoActive() || appHfpIsConnected())
                {
                    appHfpVolumeStop(steps);
                }
            }
            break;

        case SFWD_OTA_MSG_RING:
            {
                DEBUG_LOG("SCO Forwarding PEER ring");
                appScoFwdHandleOTARing(payload, payload_size);
            }
            break;

        case SFWD_OTA_MSG_MICFWD_START:
            {
                DEBUG_LOG("MIC Forwarding Start");
                appKymeraScoForwardingPause(FALSE);
            }
            break;

        case SFWD_OTA_MSG_MICFWD_STOP:
            {
                DEBUG_LOG("MIC Forwarding Stop");
                appKymeraScoForwardingPause(TRUE);
            }
            break;

        default:
            DEBUG_LOG("Unhandled OTA");
            Panic();
            break;
    }
}


/* This function does basic analysis on a packet received over the
   air. This can be a command, or include multiple SCO frames 

   Guaranteed to consume 'avail' octets from the source.
   */
static void appScoFwdProcessReceivedAirPacket(uint16 avail)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    Source air_source = theScoFwd->link_source;
    const uint8 *pSource = SourceMap(air_source);

    if (!theScoFwd->forwarded_sink)
    {
        DEBUG_LOG("No sink at present");
    }
    else if (avail < SFWD_STRIPPED_AUDIO_FRAME_OCTETS)
    {
        DEBUG_LOGF("Too little data for a packet %d < %d",avail,SFWD_STRIPPED_AUDIO_FRAME_OCTETS);
        Panic();
    }
    else
    {
        appScoFwdProcessReceivedAirFrame(&pSource, SFWD_STRIPPED_AUDIO_FRAME_OCTETS);
    }

    SourceDrop(air_source,avail);
}

/* This function processes a single SCO frame received over the air */
static void appScoFwdProcessReceivedAirFrame(const uint8 **ppSource,uint8 frame_length)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    bool    setup_late_packet_timer = FALSE;
    rtime_t frame_ttp = 0;
    rtime_t ttp_ota;
    uint8 hdr[AUDIO_FRAME_METADATA_LENGTH];

    *ppSource = sfwd_rx_help_read_ttp(*ppSource,&ttp_ota);
    RtimeWallClock24ToLocal(&theScoFwd->wallclock, ttp_ota, &frame_ttp);

    /* The WBS encoder should subtract a magic number from the TTP to deal with 
       the way encode/decode works. i.e. The first sample fed in comes *out* 
       after some analysis magic, so the first sample out is actually earlier. 
     */
    frame_ttp = rtime_sub(frame_ttp,SFWD_WBS_UNCOMPENSATED_OFFSET_US);
    int32 diff = rtime_sub(frame_ttp,SystemClockGetTimerTime());

    /* Only bother to process the frame if we think we can get to it
        in time. */
    if (diff >= SFWD_RX_PROCESSING_TIME_NORMAL_US)
    {
        ttp_stats_add(US_TO_MS(diff));

        setup_late_packet_timer = TRUE;

        insert_fake_packets_before(frame_ttp,ttp_ota);

        /* There is a chance that we have already processed this TTP
          or it is sufficiently far out we're just a bit confused.
          Check this and just process good packets. */
        if (appScoFwdTTPIsExpected(frame_ttp))
        {
            uint16 offset;
            uint16 audio_bfr_len = frame_length + SFWD_STRIPPED_HEADER_SIZE + SFWD_SCO_METADATA_SIZE;

            updatePacketStats(TRUE);

            /* Advance the TTP now, even though the block may not fit into the audio buffer */
            set_last_received_ttp(frame_ttp);

#if 1
            if ((offset = SinkClaim(theScoFwd->forwarded_sink,audio_bfr_len)) != 0xFFFF)
            {
                uint8* snk = SinkMap(theScoFwd->forwarded_sink) + offset;
                audio_frame_metadata_t md = {0,0,0};

                snk = ScoMetadataSet(snk, FALSE);
                md.ttp = frame_ttp;

                SCOFWD_TRANS_RX_DEBUG_LOGF("REAL  @ %03d 0x%06x future %-6d",
                                        SHORT_TTP(frame_ttp),ttp_ota,US_TO_MS(diff));

                /* SCO frames start with some metadata that is
                   fixed / not used. We remove this when forwarding SCO
                   so reinsert values here */
                *snk++ = 0x1;
                *snk++ = 0x18;  /* 18 is not a typical value for this field, 
                                   but works and chosen in preference to 0,
                                   which doesn't */
                *snk++ = 0xAD;  /* msbc Syncword */
                *snk++ = 0;
                *snk++ = 0;

                memcpy(snk,*ppSource,frame_length);

                PacketiserHelperAudioFrameMetadataSet(&md, hdr);

                SinkFlushHeader(theScoFwd->forwarded_sink,audio_bfr_len,hdr,AUDIO_FRAME_METADATA_LENGTH);
            }
            else
            {
                DEBUG_LOGF("STALL @ %03d 0x%06x future %-6d. No space for %d",
                                    SHORT_TTP(frame_ttp),ttp_ota,
                                    US_TO_MS(diff),frame_length + SFWD_SCO_METADATA_SIZE);
            }
#else
            DEBUG_LOGF("RX @ %03d 0x%06x future %-6d",
                                SHORT_TTP(frame_ttp),ttp_ota, US_TO_MS(diff));
            
#endif            
            ScoMetadataAdvance();
        }
        else
        {
                DEBUG_LOGF("UNEXP @ %03d 0x%06x future %-6d",
                                    SHORT_TTP(frame_ttp),ttp_ota,US_TO_MS(diff));
        }
    }
    else
    {
        DEBUG_LOGF("LATE  @ %03d 0x%06x future %-6d",
                                SHORT_TTP(frame_ttp),ttp_ota,US_TO_MS(diff));
    }

    if (setup_late_packet_timer)
    {
        start_late_packet_timer(frame_ttp);
    }

    *ppSource += frame_length;
}
static bool appScoFwdStateCanConnect(void)
{
    return appScoFwdGetState() == SFWD_STATE_IDLE;
}


void appScoFwdClearForwarding(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    if (theScoFwd->forwarding_source)
    {
        MessageStreamTaskFromSource(theScoFwd->forwarding_source, NULL);
        PanicFalse(SourceUnmap(theScoFwd->forwarding_source));
        theScoFwd->forwarding_source = NULL;
    }
}

void appScoFwdClearForwardingReceive(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    /* Unmap the timestamped endpoint, as the audio/apps0 won't */
    if (theScoFwd->forwarded_sink)
    {
        PanicFalse(SinkUnmap(theScoFwd->forwarded_sink));
        theScoFwd->forwarded_sink = NULL;
    }

    clear_last_received_ttp();
    clearPacketStats();
}

static void appScoFwdEnterIdle(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    DEBUG_LOG("appScoFwdEnterIdle");

    theScoFwd->link_sink = (Sink)NULL;
    theScoFwd->link_source = (Source)NULL;
}

static void appScoFwdExitIdle(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    DEBUG_LOG("appScoFwdExitIdle");

    theScoFwd->pending_connects = 0;
}

static void appScoFwdEnterSdpSearch(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    bdaddr peer_bd_addr;

    DEBUG_LOG("appScoFwdEnterSdpSearch");
    PanicFalse(appDeviceGetPeerBdAddr(&peer_bd_addr));

    /* Perform SDP search */
    ConnectionSdpServiceSearchAttributeRequest(&theScoFwd->task, &peer_bd_addr, 0x32,
                                               appSdpGetScoFwdServiceSearchRequestSize(), appSdpGetScoFwdServiceSearchRequest(),
                                               appSdpGetScoFwdAttributeSearchRequestSize(), appSdpGetScoFwdAttributeSearchRequest());
}

static void appScoFwdConnectL2cap(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    bdaddr peer_bd_addr;

    static const uint16 l2cap_conftab[] =
    {
        /* Configuration Table must start with a separator. */
            L2CAP_AUTOPT_SEPARATOR,
        /* Flow & Error Control Mode. */
            L2CAP_AUTOPT_FLOW_MODE,
        /* Set to Basic mode with no fallback mode */
                BKV_16_FLOW_MODE( FLOW_MODE_BASIC, 0 ),
        /* Local MTU exact value (incoming). */
            L2CAP_AUTOPT_MTU_IN,
        /*  Exact MTU for this L2CAP connection - 672. */
                672,
        /* Remote MTU Minumum value (outgoing). */
            L2CAP_AUTOPT_MTU_OUT,
        /*  Minimum MTU accepted from the Remote device. */
                48,
         /* Local Flush Timeout  - Accept Non-default Timeout*/
            L2CAP_AUTOPT_FLUSH_OUT,
                BKV_UINT32R(SFWD_FLUSH_MIN_US,SFWD_FLUSH_MAX_US),
            L2CAP_AUTOPT_FLUSH_IN,
                BKV_UINT32R(SFWD_FLUSH_MIN_US,SFWD_FLUSH_MAX_US),

        /* Configuration Table must end with a terminator. */
            L2CAP_AUTOPT_TERMINATOR
    };

    DEBUG_LOG("appScoFwdEnterConnectingMaster");
    PanicFalse(appDeviceGetPeerBdAddr(&peer_bd_addr));

    ConnectionL2capConnectRequest(appGetScoFwdTask(),
                                  &peer_bd_addr,
                                  theScoFwd->local_psm, theScoFwd->remote_psm,
                                  CONFTAB_LEN(l2cap_conftab),
                                  l2cap_conftab);

    theScoFwd->pending_connects++;
}

static void appScoFwdEnterConnected(void)
{
    bdaddr peer_addr;

    DEBUG_LOG("appScoFwdEnterConnected");

    if (appDeviceGetPeerBdAddr(&peer_addr))
        appDeviceSetScoFwdWasConnected(&peer_addr, TRUE);

    /* If we first connect with an incoming call active, then we
       need to inform our peer - otherwise the UI won't work. */
    if (appHfpIsCallIncoming())
        SendOTAControlMessage(SFWD_OTA_MSG_INCOMING_CALL);
    
    /* Check if we have a SCO active and start forwarding if we do */
    if (appHfpIsScoActive())
        appScoFwdSetState(SFWD_STATE_CONNECTED_ACTIVE);
}

static void appScoFwdExitConnected(void)
{
    DEBUG_LOG("appScoFwdExitConnected");
}

static void appScoFwdEnterConnectedActive(void)
{
    DEBUG_LOG("appScoFwdEnterConnectedActive");
    
    /* Set the SCO_ACTIVE event to run rules to check if we need
     * to enable SCO forwarding. Use an EVENT as we maybe waiting
     * for peer sync to complete still so can't check the peer status */
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_SCO_ACTIVE);
}

static void appScoFwdExitConnectedActive(void)
{
    DEBUG_LOG("appScoFwdExitConnectedActive");
    
    /* Set the SCO_INACTIVE event to run rules to check if we need
     * disconnect handset. */
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_SCO_INACTIVE);
}

static void appScoFwdWallclockEnable(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    PanicFalse(RtimeWallClockEnable(&theScoFwd->wallclock, theScoFwd->link_sink));
}

static void appScoFwdWallclockDisable(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    PanicFalse(RtimeWallClockDisable(&theScoFwd->wallclock));
}

static void appScoFwdEnterConnectedActiveSendPendingRoleInd(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdEnterConnectedActiveSendPendingRoleInd");   
    
    appLinkPolicyUpdateRoleFromSink(theScoFwd->link_sink);
}

static void appScoFwdEnterConnectedActiveSend(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdEnterConnectedActiveSend");

    clear_last_received_ttp();
    clearPacketStats();
    
    /* Get slave into active receive */
    SendOTAControlMessage(SFWD_OTA_MSG_SETUP);

    /* about to start forwarding, sync slave with current HFP volume */
    appScoFwdSendHfpVolumeToSlave(appHfpGetVolume());

    /* Prevent role-switch on this ACL, as a role-switch will mess up the wallclock */
    appLinkPolicyPreventRoleSwitchForSink(theScoFwd->link_sink);

    /* Enable wallclock for SCO packet timestamps */
    appScoFwdWallclockEnable();

    /* Tell kymera to startup the forwarding components of the SCO chain */
    appKymeraScoStartForwarding(appScoFwdGetSink(),
                                appDeviceIsPeerMicForwardSupported());
}


static void appScoFwdExitConnectedActiveSend(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdExitConnectedActiveSend");

    /* Tell slave to stop, move it back to connected state */
    SendOTAControlMessage(SFWD_OTA_MSG_TEARDOWN);

    /* Tell kymera to stop the forwarding components of the SCO chain */
    appKymeraScoStopForwarding();

    /* Disable wallclock, not needed anymore */
    appScoFwdWallclockDisable();

    /* Allow role-switch on this ACL again */
    appLinkPolicyAllowRoleSwitchForSink(theScoFwd->link_sink);
}

static void appScoFwdEnterConnectedActiveReceive(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdEnterConnectedActiveReceive");

    appAvStreamingSuspend(AV_SUSPEND_REASON_SCOFWD);

    /* Prevent role-switch on this ACL, as a role-switch will mess up the wallclock */
    appLinkPolicyPreventRoleSwitchForSink(theScoFwd->link_sink);

    /* Enable wallclock for SCO packet timestamps */
    appScoFwdWallclockEnable();

    /* Start Kymera receive chain */
    appKymeraScoSlaveStart(theScoFwd->link_source, appGetHfp()->volume, 
                                    appDeviceIsPeerMicForwardSupported(),
                                    SFWD_SCO_RX_START_MSG_DELAY);

    clear_last_received_ttp();
}

static void appScoFwdExitConnectedActiveReceive(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    DEBUG_LOG("appScoFwdExitConnectedActiveReceive");

    appKymeraScoSlaveStop();

    /* Clean up local SCO forwarding slave forwarding state */
    appScoFwdClearForwardingReceive();
    if (appConfigMicForwardingEnabled())
        appScoFwdClearForwarding();

    appAvStreamingResume(AV_SUSPEND_REASON_SCOFWD);

    /* Disable wallclock, not needed anymore */
    appScoFwdWallclockDisable();

    /* Allow role-switch on this ACL again */
    appLinkPolicyAllowRoleSwitchForSink(theScoFwd->link_sink);
}

static void appScoFwdEnterDisconnecting(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdEnterDisconnecting");

    ConnectionL2capDisconnectRequest(appGetScoFwdTask(),
                                     theScoFwd->link_sink);
}


static void appScoFwdExitInitialising(void)
{
    MessageSend(appGetAppTask(), SFWD_INIT_CFM, NULL);
}


/*! \brief Set the SCO forwarding FSM state

    Called to change state.  Handles calling the state entry and exit
    functions for the new and old states.
*/
static void appScoFwdSetState(scoFwdState new_state)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    bdaddr peer;
    scoFwdState old_state = theScoFwd->state;

    DEBUG_LOGF("appScoFwdSetState(%d) from %d", new_state, old_state);

    switch (old_state)
    {
        case SFWD_STATE_NULL:
        case SFWD_STATE_SDP_SEARCH:
        case SFWD_STATE_CONNECTING:
        case SFWD_STATE_DISCONNECTING:
            break;

        case SFWD_STATE_IDLE:
            appScoFwdExitIdle();
            break;

        case SFWD_STATE_CONNECTED_ACTIVE_RECEIVE:
            appScoFwdExitConnectedActiveReceive();
            break;

        case SFWD_STATE_CONNECTED_ACTIVE_SEND:
            appScoFwdExitConnectedActiveSend();
            break;

        case SFWD_STATE_INITIALISING:
            appScoFwdExitInitialising();
            break;

        default:
            break;
    }

    /* Set new state */
    theScoFwd->state = new_state;
    theScoFwd->lock = new_state & SFWD_STATE_LOCK_MASK;
    
    if (appScoFwdIsConnectedActive(old_state & ~SFWD_STATE_LOCK_MASK) &&
        !appScoFwdIsConnectedActive(new_state & ~SFWD_STATE_LOCK_MASK))
        appScoFwdExitConnectedActive();

    if (((old_state & ~SFWD_STATE_LOCK_MASK) >= SFWD_STATE_CONNECTED) &&
        ((new_state & ~SFWD_STATE_LOCK_MASK) <  SFWD_STATE_CONNECTED))
        appScoFwdExitConnected();

    if (((new_state & ~SFWD_STATE_LOCK_MASK) >= SFWD_STATE_CONNECTED) &&
        ((old_state & ~SFWD_STATE_LOCK_MASK) <  SFWD_STATE_CONNECTED))
        appScoFwdEnterConnected();

    if (appScoFwdIsConnectedActive(new_state & ~SFWD_STATE_LOCK_MASK) &&
        !appScoFwdIsConnectedActive(old_state & ~SFWD_STATE_LOCK_MASK))
        appScoFwdEnterConnectedActive();
    
    switch (new_state)
    {
        case SFWD_STATE_IDLE:
            appScoFwdEnterIdle();
            break;

        case SFWD_STATE_NULL:
            DEBUG_LOG("appScoFwdSetState, null");
            break;

        case SFWD_STATE_INITIALISING:
            DEBUG_LOG("appScoFwdSetState, initialising");
            break;

        case SFWD_STATE_SDP_SEARCH:
            appScoFwdEnterSdpSearch();
            break;

        case SFWD_STATE_CONNECTING:
            DEBUG_LOG("appScoFwdSetState, connecting");
            break;

        case SFWD_STATE_CONNECTED_ACTIVE_SEND:
            appScoFwdEnterConnectedActiveSend();
            break;
            
        case SFWD_STATE_CONNECTED_ACTIVE_SEND_PENDING_ROLE_IND:
            appScoFwdEnterConnectedActiveSendPendingRoleInd();
            break;        

        case SFWD_STATE_CONNECTED_ACTIVE_RECEIVE:
            appScoFwdEnterConnectedActiveReceive();
            break;

        case SFWD_STATE_DISCONNECTING:
            appScoFwdEnterDisconnecting();
            break;

        default:
            break;
    }

    if (appDeviceGetPeerBdAddr(&peer))
    {
        appLinkPolicyUpdatePowerTable(&peer);
    }
}


static void appScoFwdInitPacketising(Source audio_source)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    theScoFwd->forwarding_source = audio_source;

    PanicNull(theScoFwd->link_sink);
    PanicNull(theScoFwd->forwarding_source);

    MessageStreamTaskFromSink(theScoFwd->link_sink, appGetScoFwdTask());
    MessageStreamTaskFromSource(theScoFwd->forwarding_source, appGetScoFwdTask());

    PanicFalse(SourceMapInit(theScoFwd->forwarding_source, STREAM_TIMESTAMPED, AUDIO_FRAME_METADATA_LENGTH));
    PanicFalse(SourceConfigure(theScoFwd->forwarding_source, VM_SOURCE_MESSAGES, VM_MESSAGES_ALL));
    PanicFalse(SinkConfigure(theScoFwd->link_sink, VM_SINK_MESSAGES, VM_MESSAGES_ALL));

    appScoFwdKickProcessing();
}


void appScoFwdInitScoPacketising(Source audio_source)
{
    appScoFwdInitPacketising(audio_source);
    ConnectionWriteFlushTimeout(appScoFwdGetSink(), SFWD_FLUSH_TARGET_SLOTS);
}

void appScoFwdInitMicPacketising(Source audio_source)
{
    appScoFwdInitPacketising(audio_source);

    SendOTAControlMessage(SFWD_OTA_MSG_MIC_SETUP);

    ConnectionWriteFlushTimeout(appScoFwdGetSink(), SFWD_FLUSH_TARGET_SLOTS);
}

void appScoFwdMicForwardingEnable(bool enable)
{
    if (enable)
    {
        SendOTAControlMessage(SFWD_OTA_MSG_MICFWD_START);
    }
    else
    {
        SendOTAControlMessage(SFWD_OTA_MSG_MICFWD_STOP);
    }
}

void appScoFwdNotifyRole(hci_role role)
{
    MAKE_SFWD_INTERNAL_MESSAGE(ROLE_NOTIFY);
    message->role = role;
    MessageSend(appGetScoFwdTask(), SFWD_INTERNAL_ROLE_NOTIFY, message);
}

void appScoFwdNotifyIncomingSink(Sink sco_sink)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    if (SinkMapInit(sco_sink, STREAM_TIMESTAMPED, AUDIO_FRAME_METADATA_LENGTH))
    {   
        theScoFwd->forwarded_sink = sco_sink;
    
        PanicFalse(SinkConfigure(theScoFwd->forwarded_sink, VM_SINK_MESSAGES, VM_MESSAGES_ALL));
        PanicFalse(SourceConfigure(theScoFwd->link_source, VM_SOURCE_MESSAGES, VM_MESSAGES_ALL));

        appScoFwdKickProcessing();
    }
    else
    {
        /* SinkMapInit failed, probably because SCO has already gone */
    }
}

void appScoFwdNotifyIncomingMicSink(Sink mic_sink)
{
    MAKE_SFWD_INTERNAL_MESSAGE(MIC_CHAIN_DETAILS);

    message->sink = mic_sink;
    MessageSend(appGetScoFwdTask(),SFWD_INTERNAL_MIC_CHAIN_DETAILS,message);
}


/*! When a link and chain are initialised there are occasions where
    the buffer messages (MESSAGE_MORE_DATA/MESSAGE_MORE_SPACE) can be
    lost or mishandled, leaving a situation where the buffers have data
    and space, but we will never process it.

    This function can be used to kick the processing when the chains
    are definitely ready for use.
 */
static void appScoFwdKickProcessing(void)
{
    MessageSend(appGetScoFwdTask(),SFWD_INTERNAL_KICK_PROCESSING,NULL);
}


/* We have a request to make sure that the L2CAP link to our peer
   is established.

   If the link is *not* already up, take steps to connect it */
static void appScoFwdHandleLinkConnectReq(void)
{
    if (appScoFwdStateCanConnect())
        appScoFwdSetState(SFWD_STATE_SDP_SEARCH);
}

static void appScoFwdHandleLinkDisconnectReq(void)
{
    switch (appScoFwdGetState())
    {
        case SFWD_STATE_CONNECTED_ACTIVE_SEND:
        case SFWD_STATE_CONNECTED_ACTIVE_RECEIVE:
        case SFWD_STATE_CONNECTED:
            appScoFwdSetState(SFWD_STATE_DISCONNECTING);
            break;

        default:
            break;
    }
}


static void appScoFwdHandleL2capRegisterCfm(const CL_L2CAP_REGISTER_CFM_T *cfm)
{
    DEBUG_LOGF("appScoFwdHandleL2capRegisterCfm, status %u, psm %u", cfm->status, cfm->psm);
    PanicFalse(appScoFwdGetState() == SFWD_STATE_INITIALISING);

    /* We have registered the PSM used for SCO forwarding links with
       connection manager, now need to wait for requests to process 
       an incoming connection or make an outgoing connection. */
    if (success == cfm->status)
    {
        scoFwdTaskData *theScoFwd = appGetScoFwd();

        /* Keep a copy of the registered L2CAP PSM, maybe useful later */
        theScoFwd->local_psm = cfm->psm;

        /* Copy and update SDP record */
        uint8 *record = PanicUnlessMalloc(appSdpGetScoFwdServiceRecordSize());
        memcpy(record, appSdpGetScoFwdServiceRecord(), appSdpGetScoFwdServiceRecordSize());

        /* Write L2CAP PSM into service record */
        appSdpSetScoFwdPsm(record, cfm->psm);

        /* Set supported attributes if MIC forwarding is enabled */
        appSdpSetScoFwdFeatures(record, SFWD_FEATURE_RING_FWD | (appConfigMicForwardingEnabled() ? SFWD_FEATURE_MIC_FWD : 0));

        /* Register service record */
        ConnectionRegisterServiceRecord(appGetScoFwdTask(), appSdpGetScoFwdServiceRecordSize(), record);
    }
    else
    {
        DEBUG_LOG("appScoFwdHandleL2capRegisterCfm, failed to register L2CAP PSM");
        Panic();
    }
}

static void appScoFwdHandleClSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm)
{
    DEBUG_LOGF("appScoFwdHandleClSdpRegisterCfm, status %d", cfm->status);
    PanicFalse(appScoFwdGetState() == SFWD_STATE_INITIALISING);

    if (cfm->status == sds_status_success)
    {
        /* Move to 'idle' state */
        appScoFwdSetState(SFWD_STATE_IDLE);
    }
    else
        Panic();
}

static bool appScoFwdGetL2capPSM(const uint8 *begin, const uint8 *end, uint16 *psm, uint16 id)
{
    ServiceDataType type;
    Region record, protocols, protocol, value;
    record.begin = begin;
    record.end   = end;

    while (ServiceFindAttribute(&record, id, &type, &protocols))
        if (type == sdtSequence)
            while (ServiceGetValue(&protocols, &type, &protocol))
            if (type == sdtSequence
               && ServiceGetValue(&protocol, &type, &value)
               && type == sdtUUID
               && RegionMatchesUUID32(&value, (uint32)0x0100)
               && ServiceGetValue(&protocol, &type, &value)
               && type == sdtUnsignedInteger)
            {
                *psm = (uint16)RegionReadUnsigned(&value);
                return TRUE;
            }

    return FALSE;
}


static void appScoFwdHandleClSdpServiceSearchAttributeCfm(const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOGF("appScoFwdHandleClSdpServiceSearchAttributeCfm, status %d", cfm->status);

    switch (appScoFwdGetState())
    {
        case SFWD_STATE_SDP_SEARCH:
        {
            /* Find the PSM in the returned attributes */
            if (cfm->status == sdp_response_success)
            {
                if (appScoFwdGetL2capPSM(cfm->attributes, cfm->attributes + cfm->size_attributes,
                                         &theScoFwd->remote_psm, saProtocolDescriptorList))
                {
                    DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, peer psm %u", theScoFwd->remote_psm);

                    appScoFwdConnectL2cap();
                    appScoFwdSetState(SFWD_STATE_CONNECTING);
                }
                else
                {
                    /* No PSM found, malformed SDP record on peer? */
                    appScoFwdSetState(SFWD_STATE_IDLE);
                }
            }
            else if (cfm->status == sdp_no_response_data)
            {
                /* Peer Earbud doesn't support SCO forwarding service */
                appScoFwdSetState(SFWD_STATE_IDLE);
            }
            else
            {
                /* SDP seach failed, retry? */
                appScoFwdSetState(SFWD_STATE_SDP_SEARCH);
            }
        }
        break;

        default:
        {
            /* Silently ignore, not the end of the world */
        }
        break;
    }
}

static void appScoFwdHandleL2capConnectInd(const CL_L2CAP_CONNECT_IND_T *ind)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOGF("appScoFwdHandleL2capConnectInd, state %u, psm %u", appScoFwdGetState(), ind->psm);
    PanicFalse(ind->psm == theScoFwd->local_psm);
    bool accept = FALSE;

    static const uint16 l2cap_conftab[] =
    {
        /* Configuration Table must start with a separator. */
        L2CAP_AUTOPT_SEPARATOR, 
        /* Local Flush Timeout  - Accept Non-default Timeout*/
        L2CAP_AUTOPT_FLUSH_OUT,
            BKV_UINT32R(SFWD_FLUSH_MIN_US,SFWD_FLUSH_MAX_US),
        L2CAP_AUTOPT_FLUSH_IN,
            BKV_UINT32R(SFWD_FLUSH_MIN_US,SFWD_FLUSH_MAX_US),
        L2CAP_AUTOPT_TERMINATOR
    };    

    /* Only accept connection if it's from the peer and we're in the idle state */
    if (appDeviceIsPeer(&ind->bd_addr))
    {
        switch (appScoFwdGetState())
        {
            case SFWD_STATE_IDLE:
            {
                accept = TRUE;
                DEBUG_LOG("appScoFwdHandleL2capConnectInd, idle, accept");
                appScoFwdSetState(SFWD_STATE_CONNECTING);
            }
            break;

            case SFWD_STATE_CONNECTING:
            {                
                /* Connection crossover, accept the connection if we're the left Earbud */
                accept = appConfigIsLeft();
                DEBUG_LOGF("appScoFwdHandleL2capConnectInd, crossover, accept %u", accept);
            }
            break;

            default:
            {
                /* Incorrect state, reject the connection */
                DEBUG_LOG("appScoFwdHandleL2capConnectInd, reject");
                accept = FALSE;
            }
            break;
        }
    }

    /* Keep track of this connection */
    theScoFwd->pending_connects++;

    /* Send a response accepting or rejcting the connection. */
    ConnectionL2capConnectResponse(appGetScoFwdTask(),     /* The client task. */
                                   accept,                 /* Accept/reject the connection. */
                                   ind->psm,               /* The local PSM. */
                                   ind->connection_id,     /* The L2CAP connection ID.*/
                                   ind->identifier,        /* The L2CAP signal identifier. */
                                   CONFTAB_LEN(l2cap_conftab),
                                   l2cap_conftab);          /* The configuration table. */
}

static void appScoFwdHandleL2capConnectCfm(const CL_L2CAP_CONNECT_CFM_T *cfm)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOGF("appScoFwdHandleL2capConnectCfm, status %u, pending %u", cfm->status, theScoFwd->pending_connects);

    /* Pending connection, return, will get another message in a bit */
    if (l2cap_connect_pending == cfm->status)
    {
        DEBUG_LOG("appScoFwdHandleL2capConnectCfm, connect pending, wait");
        return;
    }

    /* Decrement number of pending connect confirms, panic if 0 */
    PanicFalse(theScoFwd->pending_connects > 0);
    theScoFwd->pending_connects--;

    switch (appScoFwdGetState())
    {
        case SFWD_STATE_CONNECTING:
        {
            /* If connection was succesful, get sink, attempt to enable wallclock and move
             * to connected state */
            if (l2cap_connect_success == cfm->status)
            {
                DEBUG_LOGF("appScoFwdHandleL2capConnectCfm, connected, conn ID %u, flush remote %u", cfm->connection_id, cfm->flush_timeout_remote);

                PanicNull(cfm->sink);
                theScoFwd->link_sink = cfm->sink;
                theScoFwd->link_source = StreamSourceFromSink(cfm->sink);
                appScoFwdSetState(SFWD_STATE_CONNECTED);
            }
            else
            {
                /* Connection failed, if no more pending connections, return to idle state */
                if (theScoFwd->pending_connects == 0)
                {
                    DEBUG_LOG("appScoFwdHandleL2capConnectCfm, failed, go to idle state");
                    appScoFwdSetState(SFWD_STATE_IDLE);
                }
                else
                {
                    DEBUG_LOG("appScoFwdHandleL2capConnectCfm, failed, wait");
                }
            }
        }
        break;

        default:
        {
            /* Connect confirm receive not in connecting state, connection must have failed */
            PanicFalse(l2cap_connect_success != cfm->status);
            DEBUG_LOGF("appScoFwdHandleL2capConnectCfm, failed, pending %u", theScoFwd->pending_connects);
        }
        break;
    }
}


static void appScoFwdHandleL2capDisconnectInd(const CL_L2CAP_DISCONNECT_IND_T *ind)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOGF("appScoFwdHandleL2capDisconnectInd, status %u", ind->status);

    /* Always send reponse */
    ConnectionL2capDisconnectResponse(ind->identifier, ind->sink);

    /* Only change state if sink matches */
    if (ind->sink == theScoFwd->link_sink)
        appScoFwdSetState(SFWD_STATE_IDLE);
}


static void appScoFwdHandleL2capDisconnectCfm(const CL_L2CAP_DISCONNECT_CFM_T *cfm)
{
    UNUSED(cfm);
    DEBUG_LOGF("appScoFwdHandleL2capDisconnectCfm, status %u", cfm->status);

    /* Move to idle state if we're in the disconnecting state */
    if (appScoFwdGetState() == SFWD_STATE_DISCONNECTING)
        appScoFwdSetState(SFWD_STATE_IDLE);
}


static void appScoFwdHandleMMD(const MessageMoreData *mmd)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    if (mmd->source == theScoFwd->forwarding_source)
    {
        sfwd_tx_queue_next_packet();
    }
    else if (mmd->source == theScoFwd->link_source)
    {
        appScoFwdProcessForwardedAudio();
    }
    else
    {
        DEBUG_LOG("MMD received that doesn't match a link");
        if (ALL_SOURCES_CONFIGURED(theScoFwd))
        {
            Panic();
        }
    }
}

/*! \brief Handle messages about the interface between the outside world
           (L2CAP) and the audio chain.
           
     We treat space in the target buffer in the same way as more data in the
     send buffer. May need to assess if we need this, but neccessary for 
     making sure stall situations recover (if possible).
 */
static void appScoFwdHandleMMS(const MessageMoreSpace *mms)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    if (mms->sink == theScoFwd->link_sink)
    {
        sfwd_tx_queue_next_packet();
    }
    else if (mms->sink == theScoFwd->forwarded_sink)
    {
        appScoFwdProcessForwardedAudio();
    }
    else
    {
        DEBUG_LOG("MMS received that doesn't match a link");
        if (ALL_SINKS_CONFIGURED(theScoFwd))
        {
            Panic();
        }
    }
}

static void appScoFwdHandleStartReceiveChain(void)
{
    if (!appScoFwdIsStreaming() && appScoFwdIsConnected())
    {
        /* Move to receive state */
        appScoFwdSetState(SFWD_STATE_CONNECTED_ACTIVE_RECEIVE);
    }
    else
    {
        DEBUG_LOG("appScoFwdHandleStartChain Asked to start when already active");
    }
}

static void appScoFwdHandleStopReceiveChain(void)
{
    if (appScoFwdIsStreaming())
    {
        appScoFwdSetState(SFWD_STATE_CONNECTED);
    }
    else
    {
        DEBUG_LOGF("appScoFwdHandleStopChain Asked to stop when not active. State %d",appScoFwdGetState());
    }
}

static void appScoFwdHandleKickProcessing(void)
{
    if (appScoFwdInActiveState())
    {
        appScoFwdProcessForwardedAudio();
        sfwd_tx_queue_next_packet();
    }
}

/*  We have stopped receiving packets, but the link etc. is still up.

    Stopping the late packet timer will mean we do no more work on the 
    speaker path (which will have muted by now anyway). Clearing the ttp
    means that when audio restarts we just forward the first packet.
 */
static void appScoFwdHandleRxAudioMissing(void)
{
    scoFwdReceivedPacketStats *stats = appGetScoFwdStats();

    /* Make sure audio still missing */
    if (stats->audio_missing)
    {
        cancel_late_packet_timer();
        clear_last_received_ttp();
    }
}

/*! Get forwarding started.
    Only works if we have an active SCO already on the master */
static void appScoFwdHandleEnableForwarding(void)
{
    DEBUG_LOGF("appScoFwdHandleEnableForwarding, state %u", appScoFwdGetState());

    /* Move to sending state if in connected active state, this will start forwarding */
    if (appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE)
    {
        /* Check role first, once done move to active send state */
        appScoFwdSetState(SFWD_STATE_CONNECTED_ACTIVE_SEND_PENDING_ROLE_IND);
    }
}


/* Stop forwarding.
   Only works if we're currently a master forwarding SCO. */
static void appScoFwdHandleDisableForwarding(void)
{
    DEBUG_LOGF("appScoFwdHandleDisableForwarding, state %u", appScoFwdGetState());

    /* Move to back to connected active state if sending, this will stop forwarding */
    if ((appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND) || 
        (appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND_PENDING_ROLE_IND))
        appScoFwdSetState(SFWD_STATE_CONNECTED_ACTIVE);
}


static void appScoFwdHandlePhyStateChangedInd(const PHY_STATE_CHANGED_IND_T *ind)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOGF("appScoFwdHandlePhyStateChangedInd new state %u", ind->new_state);

    switch (ind->new_state)
    {
        case PHY_STATE_OUT_OF_EAR:
        case PHY_STATE_OUT_OF_EAR_AT_REST:
        case PHY_STATE_IN_CASE:
        {
            /* Mute local audio if we're streaming and earbud no longer
             * in the ear */
            if ((appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND) || 
                (appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND_PENDING_ROLE_IND))
                appKymeraScoSetVolume(0);
 
            /* always mark local audio as muted, for correct handling
             * during any subsequent start of forwarding, whilst still
             * out of the ear */
            theScoFwd->vol_muted = TRUE;
        }
        break;
        
        case PHY_STATE_IN_EAR:
        {
            if ((appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND) ||
                (appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND_PENDING_ROLE_IND) ||
                (appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE))
            {
                appKymeraScoSetVolume(appHfpGetVolume());
            }
            theScoFwd->vol_muted = FALSE;
        }
        break;

        default:
        break;
    }
}

static void appScoFwdHandleMicChainDetails(const SFWD_INTERNAL_CHAIN_DETAILS_T *details)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    theScoFwd->forwarded_sink = details->sink;
    PanicFalse(SinkMapInit(theScoFwd->forwarded_sink, STREAM_TIMESTAMPED, AUDIO_FRAME_METADATA_LENGTH));
}


static void appScoFwdHandleHfpScoIncomingRingInd(void)
{
    DEBUG_LOG("appScoFwdHandleHfpScoIncomingRingInd");
    SendOTAControlMessage(SFWD_OTA_MSG_INCOMING_CALL);
}

static void appScoFwdHandleHfpScoIncomingCallEndedInd(void)
{
    DEBUG_LOG("appScoFwdHandleHfpScoIncomingCallEndedInd");
    SendOTAControlMessage(SFWD_OTA_MSG_INCOMING_ENDED);
}

static void appScoFwdSendHfpVolumeToSlave(uint8 volume)
{
    SendOTAControlMessageWithPayload(SFWD_OTA_MSG_SET_VOLUME, &volume, sizeof(volume));
}

static void appScoFwdHandleHfpVolumeInd(const APP_HFP_VOLUME_IND_T *ind)
{
    if (appHfpIsScoFwdHandlingVolume())
    {
        DEBUG_LOGF("appScoFwdHandleHfpVolumeInd volume %u", ind->volume);

        appScoFwdSendHfpVolumeToSlave(ind->volume);

        /* always set the volume unless SCOFWD has decided to mute it, due to being
         * an out of ear master with slave still in ear */
        if (!appGetScoFwd()->vol_muted)
        {
            appKymeraScoSetVolume(ind->volume);
        }
    }
    else
    {
        DEBUG_LOG("appScoFwdHandleHfpVolumeInd. Not handled. TWS+");
    }
}

/*! Handle OTA signalling from peer */
static void appScoFwdHandlePeerSignallingMessage(const PEER_SIG_MSG_CHANNEL_RX_IND_T *ind)
{
    DEBUG_LOGF("appScoFwdHandlePeerSignallingMessage. Channel 0x%x, len %d, content %x",
                ind->channel,ind->msg_size,ind->msg[0]);

    /* parse and handle message, only a single channel in use
     * if just a single byte command message, then NULL the msg parameters */
    ProcessOTAControlMessage(ind->msg[0],
                             ind->msg_size > 1 ? &ind->msg[1] : NULL,
                             ind->msg_size - 1);
}

/* Handle a confirm message.

    This is only used for debug purposes, but may need to deal with any error code
    in future. */
static void appScoFwdHandlePeerSignallingMessageTxConfirm(const PEER_SIG_MSG_CHANNEL_TX_CFM_T *cfm)
{
    peerSigStatus status = cfm->status;

    DEBUG_LOGF("appScoFwdHandlePeerSignallingMessageTxConfirm. Channel 0x%x", cfm->channel);

    if (peerSigStatusSuccess != status)
    {
        DEBUG_LOGF("appScoFwdHandlePeerSignallingMessageTxConfirm reports failure code 0x%x(%d)",status,status);
    }
}

static void appScoFwdHandlePeerSignallingConnectionInd(const PEER_SIG_CONNECTION_IND_T *ind)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOGF("appScoFwdHandlePeerSignallingConnectionInd, status %u", ind->status);

    /* Peer signalling has disconnected, therefore we don't know if peer has
     * incoming call or not */
    if (ind->status == peerSigStatusDisconnected)
        theScoFwd->peer_incoming_call = FALSE;
}

static void appScoFwdHandleScoConnected(const APP_HFP_CONNECTED_IND_T *connected)
{
    bool tws_plus_device = appDeviceIsTwsPlusHandset(&connected->bd_addr);

    DEBUG_LOGF("appScoFwdHandleScoConnected: SCO Fwd handling volume %d",!tws_plus_device);

    appHfpScoFwdHandlingVolume(!tws_plus_device);
}

static void appScoFwdHandleScoDisconnected(void)
{
    /* Default to SCO Fwd handling volume when SCO Forwarding is enabled */
    appHfpScoFwdHandlingVolume(TRUE);
}

static void appScoFwdHandleRoleNotify(SFWD_INTERNAL_ROLE_NOTIFY_T *role)
{
    DEBUG_LOGF("appScoFwdHandleRoleNotify, status %u, role %u", appScoFwdGetState(), role->role);
 
    if (appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND_PENDING_ROLE_IND)
    {
        if (role->role == hci_role_master)
            appScoFwdSetState(SFWD_STATE_CONNECTED_ACTIVE_SEND);
    }
}



/*! \brief Message Handler

    This function is the main message handler for SCO forwarding, every
    message is handled in it's own seperate handler function.  

    The different groups of messages are separated in the switch statement
    by a comment like this ----
*/
static void appScoFwdHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);

#ifdef SCOFWD_EXTRA_DEBUG
    if (   id != MESSAGE_MORE_DATA
        && id != MESSAGE_MORE_SPACE)
    {
        DEBUG_LOGF("**** appScoFwdHandleMessage: 0x%x (%d)",id,id);
    }
#endif

    switch (id)
    {
        case PEER_SIG_MSG_CHANNEL_RX_IND:
            appScoFwdHandlePeerSignallingMessage((const PEER_SIG_MSG_CHANNEL_RX_IND_T *)message);
            break;

        case PEER_SIG_MSG_CHANNEL_TX_CFM:
            appScoFwdHandlePeerSignallingMessageTxConfirm((const PEER_SIG_MSG_CHANNEL_TX_CFM_T *)message);
            break;

        case PEER_SIG_CONNECTION_IND:
            appScoFwdHandlePeerSignallingConnectionInd((const PEER_SIG_CONNECTION_IND_T *)message);
            break;

        /*----*/

        case CL_L2CAP_REGISTER_CFM:
            appScoFwdHandleL2capRegisterCfm((const CL_L2CAP_REGISTER_CFM_T *)message);
            break;

        case CL_SDP_REGISTER_CFM:
            appScoFwdHandleClSdpRegisterCfm((const CL_SDP_REGISTER_CFM_T *)message);
            break;

        case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
            appScoFwdHandleClSdpServiceSearchAttributeCfm((const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *)message);
            return;

        case CL_L2CAP_CONNECT_IND:
            appScoFwdHandleL2capConnectInd((const CL_L2CAP_CONNECT_IND_T *)message);
            break;

        case CL_L2CAP_CONNECT_CFM:
            appScoFwdHandleL2capConnectCfm((const CL_L2CAP_CONNECT_CFM_T *)message);
            break;

        case CL_L2CAP_DISCONNECT_IND:
            appScoFwdHandleL2capDisconnectInd((const CL_L2CAP_DISCONNECT_IND_T *)message);
            break;

        case CL_L2CAP_DISCONNECT_CFM:
            appScoFwdHandleL2capDisconnectCfm((const CL_L2CAP_DISCONNECT_CFM_T *)message);
            break;

        /*----*/

        case APP_HFP_CONNECTED_IND:
            appScoFwdHandleScoConnected((const APP_HFP_CONNECTED_IND_T *)message);
            break;

        case APP_HFP_DISCONNECTED_IND:
            appScoFwdHandleScoDisconnected();
            break;

        case APP_HFP_SCO_INCOMING_RING_IND:
            appScoFwdHandleHfpScoIncomingRingInd();
            break;

        case APP_HFP_SCO_INCOMING_ENDED_IND:
            appScoFwdHandleHfpScoIncomingCallEndedInd();
            break;

        case APP_HFP_VOLUME_IND:
            appScoFwdHandleHfpVolumeInd((const APP_HFP_VOLUME_IND_T *)message);
            break;

        /*----*/

        case MESSAGE_MORE_DATA:
            appScoFwdHandleMMD((const MessageMoreData*)message);
            break;

        case MESSAGE_MORE_SPACE:
            appScoFwdHandleMMS((const MessageMoreSpace*)message);
            break;

        case MESSAGE_SOURCE_EMPTY:
            break;

        /*----*/

        case SFWD_INTERNAL_LINK_CONNECT_REQ:
            appScoFwdHandleLinkConnectReq();
            break;

        case SFWD_INTERNAL_LINK_DISCONNECT_REQ:
            appScoFwdHandleLinkDisconnectReq();
            break;

        case SFWD_INTERNAL_START_RX_CHAIN:
            appScoFwdHandleStartReceiveChain();
            break;

        case SFWD_INTERNAL_STOP_RX_CHAIN:
            appScoFwdHandleStopReceiveChain();
            break;

        case SFWD_INTERNAL_KICK_PROCESSING:
            appScoFwdHandleKickProcessing();
            break;

        case SFWD_INTERNAL_RX_AUDIO_MISSING:
            appScoFwdHandleRxAudioMissing();
			break;

        case SFWD_INTERNAL_ENABLE_FORWARDING:
            appScoFwdHandleEnableForwarding();
            break;

        case SFWD_INTERNAL_DISABLE_FORWARDING:
            appScoFwdHandleDisableForwarding();
            break;

        case SFWD_INTERNAL_ROLE_NOTIFY:
            appScoFwdHandleRoleNotify((SFWD_INTERNAL_ROLE_NOTIFY_T *)message);
            break;
            
        case SFWD_INTERNAL_PLAY_RING:
            appUiHfpRing(0);
            break;


        /*----*/

        case SFWD_INTERNAL_MIC_CHAIN_DETAILS:
            appScoFwdHandleMicChainDetails((const SFWD_INTERNAL_MIC_CHAIN_DETAILS_T *)message);
            return;

        /*----*/

        case SFWD_TIMER_LATE_PACKET:
            handle_late_packet_timer();
            break;

        /*----*/

        case PHY_STATE_CHANGED_IND:
            appScoFwdHandlePhyStateChangedInd((const PHY_STATE_CHANGED_IND_T *)message);
            break;

        /*----*/

        default:
            DEBUG_LOGF("appScoFwdHandleMessage. UNHANDLED Message id=x%x (%d). State %d", id, id, appScoFwdGetState());
            break;
    }
}

#ifdef INCLUDE_SWB
static appKymeraScoMode appScoFwdGetQceScoMode(uint16 codec_mode_id)
{
    switch (codec_mode_id)
    {
        case aptx_adaptive_64_2_EV3:
        case aptx_adaptive_64_2_EV3_QHS3:
        case aptx_adaptive_64_QHS3:
            return SCO_SWB;
            
        case aptx_adaptive_128_QHS3:
            return SCO_UWB;
            
        default:
            Panic();
            return NO_SCO;
    }
}
#endif

static void appScoFwdNotifyAudioDisappeared(void)
{
    MessageSend(appGetScoFwdTask(), SFWD_INTERNAL_RX_AUDIO_MISSING, NULL);
}








bool appScoFwdIsStreaming(void)
{
    return appScoFwdInActiveState();
}


bool appScoFwdIsReceiving(void)
{
    return (appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_RECEIVE);
}


bool appScoFwdIsSending(void)
{
    return (appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND);
}


bool appScoFwdIsConnected(void)
{
    return (appScoFwdGetState() == SFWD_STATE_CONNECTED ||
            appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND ||
            appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_SEND_PENDING_ROLE_IND ||
            appScoFwdGetState() == SFWD_STATE_CONNECTED_ACTIVE_RECEIVE);
}


bool appScoFwdIsDisconnected(void)
{
    return (appScoFwdGetState() == SFWD_STATE_IDLE);
}


/*! \brief Initialise SCO Forwarding task

    Called at start up to initialise the SCO forwarding task
*/
void appScoFwdInit(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();

    /* Set up task handler */
    theScoFwd->task.handler = appScoFwdHandleMessage;

    /* Register a Protocol/Service Multiplexor (PSM) that will be 
       used for this application. The same PSM is used at both
       ends. */
    ConnectionL2capRegisterRequest(appGetScoFwdTask(), L2CA_PSM_INVALID, 0);

    /* Initialise state */
    theScoFwd->state = SFWD_STATE_NULL;
    appScoFwdSetState(SFWD_STATE_INITIALISING);

    /* Want to know about HFP calls and will handle setting volume in
     * kymera. */
    appHfpScoFwdHandlingVolume(TRUE);
    appHfpStatusClientRegister(appGetScoFwdTask());

    /* Register a channel for peer signalling */
    appPeerSigMsgChannelTaskRegister(appGetScoFwdTask(), PEER_SIG_MSG_CHANNEL_SCOFWD);

    /* Register for peer signaling notifications */
    appPeerSigClientRegister(appGetScoFwdTask());

    /* Register for physical state changes */
    appPhyStateRegisterClient(appGetScoFwdTask());

    /* Clear wallclock */
    appScoFwdSetWallclock((Sink)NULL);
}

/*! \brief Set-up an L2CAP connection between the devices for forwarding SCO audio */
void appScoFwdConnectPeer(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdConnectPeer");
    MessageSendConditionally(appGetScoFwdTask(), SFWD_INTERNAL_LINK_CONNECT_REQ, NULL, &theScoFwd->lock);
}

/*! \brief Disconect L2CAP connection berween the device for forwarding SCO audio */
void appScoFwdDisconnectPeer(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdDisconnectPeer");
    MessageSendConditionally(appGetScoFwdTask(), SFWD_INTERNAL_LINK_DISCONNECT_REQ, NULL, &theScoFwd->lock);
}


bool appScoFwdIsCallIncoming(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOGF("appScoFwdIsCallIncoming, incoming call %u", theScoFwd->peer_incoming_call);
    return theScoFwd->peer_incoming_call;
}

void appScoFwdCallAccept(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdCallAccept");

    SendOTAControlMessage(SFWD_OTA_MSG_CALL_ANSWER);
    theScoFwd->peer_incoming_call = FALSE;

    /* Cancel the ring tone now to prevent any delay waiting for the
     * SFWD_OTA_MSG_INCOMING_ENDED back */
    appScoFwdRingCancel();
}

void appScoFwdCallReject(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdCallReject");

    SendOTAControlMessage(SFWD_OTA_MSG_CALL_REJECT);    
    theScoFwd->peer_incoming_call = FALSE;

    /* Cancel the ring tone now to prevent any delay waiting for the
     * SFWD_OTA_MSG_INCOMING_ENDED back */
    appScoFwdRingCancel();
}

void appScoFwdCallHangup(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdCallHangup");

    SendOTAControlMessage(SFWD_OTA_MSG_CALL_HANGUP);
    theScoFwd->peer_incoming_call = FALSE;
}

void appScoFwdCallVoice(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdCallVoice");

    SendOTAControlMessage(SFWD_OTA_MSG_CALL_VOICE);
    theScoFwd->peer_incoming_call = FALSE;
}

/*! \brief Forward ring indication to peer Earbud with time-to-play */
void appScoFwdRing(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    rtime_t wallclock, sync_time;
    wallclock_state_t wc_state;
    uint8 buff[3];

    DEBUG_LOGF("appScoFwdRing, wc_sink %u", theScoFwd->wallclock_sink);

    if (RtimeWallClockGetStateForSink(&wc_state, theScoFwd->wallclock_sink) &&
        RtimeLocalToWallClock24(&wc_state, VmGetTimerTime(), &wallclock))
    {
        DEBUG_LOGF("appScoFwdRing, wc_state %u", wc_state);

        sync_time = rtime_add(wallclock, appConfigScoFwdRingMs() * 1000);

        sfwd_tx_help_write_ttp(buff,sync_time);

        /* Send the SCOFWD OTA message */
        SendOTAControlMessageWithPayload(SFWD_OTA_MSG_RING, buff, sizeof(buff));

        /* Also, play the ring locally: for this we will use the same function that will
           be called on the peer to handle the msg SFWD_OTA_MSG_RING */
        appScoFwdPlayRingAtWcTime(sync_time);

        theScoFwd->peer_incoming_call = FALSE;
    }
}


/*! \brief Cancel any pending local ring indication */
void appScoFwdRingCancel(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    if (MessageCancelAll(&theScoFwd->task, SFWD_INTERNAL_PLAY_RING))
        DEBUG_LOG("appScoFwdRingCancel, pending message to RING cancelled");
}


void appScoFwdVolumeStart(int16 step)
{
    uint8 step_msg[2];
    appScoFwdOTAPayloadWriteInt16(step_msg, step);
    SendOTAControlMessageWithPayload(SFWD_OTA_MSG_VOLUME_START, step_msg, sizeof(step));
}


void appScoFwdVolumeStop(int16 step)
{
    uint8 step_msg[2];
    appScoFwdOTAPayloadWriteInt16(step_msg, step);
    SendOTAControlMessageWithPayload(SFWD_OTA_MSG_VOLUME_STOP, step_msg, sizeof(step));
}


void appScoFwdEnableForwarding(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    MessageSendConditionally(appGetScoFwdTask(), SFWD_INTERNAL_ENABLE_FORWARDING, NULL, &theScoFwd->lock);
}


void appScoFwdDisableForwarding(void)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    MessageSendConditionally(appGetScoFwdTask(), SFWD_INTERNAL_DISABLE_FORWARDING, NULL, &theScoFwd->lock);
}


/*! \brief SCO forwarding handling of HFP_AUDIO_CONNECT_CFM.
    \param cfm The confirmation message.
    \note Without SCO forwarding just start Kymera SCO chain.
*/
void appScoFwdHandleHfpAudioConnectConfirmation(const HFP_AUDIO_CONNECT_CFM_T *cfm)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    uint16 volume = theScoFwd->vol_muted ? 0 : appHfpGetVolume();
    uint8 delay = 0;
    bdaddr hfp_bdaddr;
    bool allow_scofwd, allow_micfwd;

    DEBUG_LOGF("appScoFwdHandleHfpAudioConnectConfirmation, status %u", cfm->status);

    /* Query HFP for the Bluetooth Device Address, check if device is TWS+   */
    bool tws_plus_handset = HfpLinkGetBdaddr(cfm->priority, &hfp_bdaddr) && 
                            appDeviceIsTwsPlusHandset(&hfp_bdaddr);


    /* SCO forwarding enabled only if enabled in configuration and connected to standard handset */
    allow_scofwd = !tws_plus_handset && appConfigScoForwardingEnabled();
    
    /* MIC forwarding enabled only SCO forwarding enabled, enabled in configuration and
     * both EBs support MIC forwarding */
    allow_micfwd = allow_scofwd && appDeviceIsPeerMicForwardSupported();
    
    /* TODO: This would be a good place to disable SCO forwarding if the SCO parameters
     * are not compatible with forwarding.  Unfortunately the parameters in 
     * HFP_AUDIO_CONNECT_CFM currently doesn't include SCO packet type or TeSCO. */
        
    /* Calculate the message delay, if we'll be starting forwarding immediately */
    if (appScoFwdIsConnected() && appPeerSyncIsPeerInEar())
    {
        if (!tws_plus_handset)
            delay = SFWD_SCO_START_MSG_DELAY - 1;
    }        

    appKymeraScoMode sco_mode = SCO_NB;
    
#ifdef INCLUDE_SWB
    /* Get SCO mode when Qualcomm to Qualcomm CODEC selected, otherwise
       check for wideband mSBC */
    if (cfm->qce_codec_mode_id != CODEC_MODE_ID_UNSUPPORTED)
        sco_mode = appScoFwdGetQceScoMode(cfm->qce_codec_mode_id);
    else
        sco_mode = (cfm->codec == hfp_wbs_codec_mask_msbc) ? SCO_WB : SCO_NB;        
#else
    sco_mode = (cfm->codec == hfp_wbs_codec_mask_msbc) ? SCO_WB : SCO_NB;        
#endif
    
    /* Always start kymera for the new SCO, start muted if vol_muted flag
     * is set indicating this earbud is out of the ear */
    if (appKymeraScoStart(cfm->audio_sink, sco_mode, &allow_scofwd, &allow_micfwd,
                          cfm->wesco, volume, delay))
    {        
        /* Check if SCO forwarding was enabled in chain */
        if (allow_scofwd)
        {
            /* If L2CAP connection is up then move into an active state, otherwise do nothing
             * and when the L2CAP comes up we'll handle a transition */
            if (appScoFwdGetState() == SFWD_STATE_CONNECTED)
            {
                /* Move to active state now that the audio chain can support SCO forwarding */
                appScoFwdSetState(SFWD_STATE_CONNECTED_ACTIVE);
            }
        }
    }
}


/*! \brief SCO forwarding handling of HFP_AUDIO_DISCONNECT_IND.
    \param ind The indication message.
    \note Without SCO forwarding just start Kymera SCO chain.
*/
void appScoFwdHandleHfpAudioDisconnectIndication(const HFP_AUDIO_DISCONNECT_IND_T *ind)
{
    UNUSED(ind);
    DEBUG_LOGF("appScoFwdHandleHfpAudioDisconnectIndication, status %u", ind->status);

    switch (appScoFwdGetState())
    {
        case SFWD_STATE_CONNECTED_ACTIVE:
        case SFWD_STATE_CONNECTED_ACTIVE_SEND:
        case SFWD_STATE_CONNECTED_ACTIVE_SEND_PENDING_ROLE_IND:
        {   
            /* Move to back connected state, state exit function will handle cleanup */
            appScoFwdSetState(SFWD_STATE_CONNECTED);
        }
        break;

        case SFWD_STATE_CONNECTED:
        {
            /* Handset ended call already, or was a TWS+ handset that does
               not support SCO forwarding */
        }
        break;

        case SFWD_STATE_IDLE:
        case SFWD_STATE_SDP_SEARCH:
        case SFWD_STATE_CONNECTING:        
        case SFWD_STATE_DISCONNECTING:
        {
            /* Nothing to do, SCO forwarding is not active, the L2CAP went down before
             * notification SCO going down arrived */
        }
        break;

        case SFWD_STATE_CONNECTED_ACTIVE_RECEIVE:
        {
            /* In active receive mode there should be no SCO to handset */
            DEBUG_LOG("appScoFwdHandleHfpAudioDisconnectIndication, bad state SFWD_STATE_CONNECTED_ACTIVE_RECEIVE");
            
            /* TODO: Should we panic here? */
            Panic();
        }
        break;

        default:
        {
            DEBUG_LOGF("appScoFwdHandleHfpAudioDisconnectIndication, unexpected state %u", appScoFwdGetState());
        }
        break;
    }

    /* SCO has gone, always stop Kymera */
    appKymeraScoStop();
}

/*! \brief Set the time base for the SCOFWD to match a sink

    Stores the supplied sink for the RING synchronization.

    \param sink The sink to be used as a time base.
*/
void appScoFwdSetWallclock(Sink sink)
{
    scoFwdTaskData *theScoFwd = appGetScoFwd();
    DEBUG_LOG("appScoFwdSetWallclock, sink %u", sink);
    theScoFwd->wallclock_sink = sink;
}

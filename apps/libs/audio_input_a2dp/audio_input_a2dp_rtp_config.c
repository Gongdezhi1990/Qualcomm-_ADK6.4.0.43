/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_a2dp_rtp_config.c

DESCRIPTION
    RTP decode operator configuration
*/

#include "audio_input_a2dp_rtp_config.h"
#include "audio_input_a2dp_chain_config.h"
#include "audio_input_a2dp_context.h"
#include "audio_input_a2dp_ttp_latency.h"
#include <audio_config.h>
#include <print.h>
#include <panic.h>

/* Make sure buffer is large enough to hold number of samples corresponding to the latency */
#define RTP_BUFFER_SIZE (8000U)
#define BA_RTP_BUFFER_SIZE (1000U)

#define APTX_LL_SAMPLES_PER_CODEWORD        (4U)
#define APTX_LL_CHANNELS                    (2U)
#define APTX_LL_RTP_BUFFER_RATIO            (2U)
#define APTX_LL_MINIMUM_LATENCY_IN_MS       (0U)
#define APTX_LL_MAXIMUM_LATENCY_DELTA_IN_MS (10U)

#define MS_PER_SECOND (1000U)
#define CONVERTION_FACTOR_2MS_TO_1MS (2U)

/* MTU? (895) - max tws packet header (5) = 890, and then round down to a multiple of 4 := 888 */
#define RTP_PACKET_LIMITED_LENGTH           (888U)
/* MTU? (895) - max share me packet header (13) = 882, and then round down to a multiple of 4 := 880 */
#define RTP_PACKET_LIMITED_LENGTH_SHAREME   (880U)


static uint16 aptx_ll_target_ttp_level;

static void AudioPluginA2dpAptxAdModeNotificationMessageHandler(Task task, MessageId id, Message message);

TaskData aptx_ad_mode_notification = { AudioPluginA2dpAptxAdModeNotificationMessageHandler };

static void AudioPluginA2dpAptxAdModeNotificationMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    if(id == MESSAGE_FROM_OPERATOR)
    {
        aptx_ad_mode_notification_t aptx_ad_mode_info;
        uint16 target_latency_tenths_of_ms;

        audio_input_context_t * ctx = audioInputA2dpGetContextData();
        aptx_ad_mode_info = OperatorsRtpGetAptxAdModeNotificationInfo((const MessageFromOperator *)message);
        target_latency_tenths_of_ms = (uint16)((aptx_ad_mode_info.target_latency / US_PER_MS) * 10);

        audioInputA2dpSendLatencyUpdateToAppTask(ctx, target_latency_tenths_of_ms);       
     }
}

static uint16 getRtpPacketLength(void)
{
    if(AudioConfigGetPeerMode() == peer_mode_shareme)
        return RTP_PACKET_LIMITED_LENGTH_SHAREME;
    else
        return RTP_PACKET_LIMITED_LENGTH;
}

static rtp_codec_type_t getRtpCodecType(audio_codec_t decoder)
{
    rtp_codec_type_t rtp_codec_type = rtp_codec_type_sbc;

    switch(decoder)
    {
        case audio_codec_sbc:
            rtp_codec_type = rtp_codec_type_sbc;
            break;
        case audio_codec_aptx:
        case audio_codec_aptx_ll:
            rtp_codec_type = rtp_codec_type_aptx;
            break;
        case audio_codec_aptx_hd:
            rtp_codec_type = rtp_codec_type_aptx_hd;
            break;
        case audio_codec_aptx_adaptive:
            rtp_codec_type = rtp_codec_type_aptx_ad;
            break;
        case audio_codec_aac:
            rtp_codec_type = rtp_codec_type_aac;
            break;
        default:
            Panic();
    }

    return rtp_codec_type;
}

static bool isCodecNotUsingRtpHeaders(audio_codec_t decoder, bool content_protection)
{
    if ((decoder == audio_codec_aptx) || (decoder == audio_codec_aptx_ll))
        if (content_protection == FALSE)
            return TRUE;

    return FALSE;
}

static rtp_working_mode_t getRtpWorkingMode(audio_codec_t decoder, A2dpPluginConnectParams *a2dp_connect_params)
{
    rtp_working_mode_t rtp_mode;

    if (isCodecNotUsingRtpHeaders(decoder, a2dp_connect_params->content_protection))
        rtp_mode = rtp_ttp_only;
    else if (audioInputA2dpisTimeToPlayControlledBySource(decoder, a2dp_connect_params))
        rtp_mode = rtp_passthrough;
    else
        rtp_mode = rtp_decode;

    return rtp_mode;
}

static aptx_adaptive_ttp_in_ms_t convertAptxAdaptiveTtpToOperatorsFormat(aptx_adaptive_ttp_latencies_t ttp_in_non_q2q_mode)
{
    aptx_adaptive_ttp_in_ms_t aptx_ad_ttp;

    aptx_ad_ttp.low_latency_0 = ttp_in_non_q2q_mode.low_latency_0_in_ms;
    aptx_ad_ttp.low_latency_1 = ttp_in_non_q2q_mode.low_latency_1_in_ms;
    aptx_ad_ttp.high_quality  = (uint16) (CONVERTION_FACTOR_2MS_TO_1MS * ttp_in_non_q2q_mode.high_quality_in_2ms);
    aptx_ad_ttp.tws_legacy    = (uint16) (CONVERTION_FACTOR_2MS_TO_1MS * ttp_in_non_q2q_mode.tws_legacy_in_2ms);

    return aptx_ad_ttp;
}

static bool hadToAdjustTtpLatency(uint16 *ttp_latency, ttp_latency_t ttp_supported)
{
    uint16 original_ttp_latency = *ttp_latency;

    *ttp_latency = MIN(*ttp_latency, ttp_supported.max_in_ms);
    *ttp_latency = MAX(*ttp_latency, ttp_supported.min_in_ms);

    if (*ttp_latency == original_ttp_latency)
        return FALSE;
    else
        return TRUE;
}

static aptx_adaptive_ttp_in_ms_t getAdjustedAptxAdaptiveTtpLatencies(aptx_adaptive_ttp_in_ms_t aptx_ad_ttp)
{
    ttp_latency_t ttp_supported = AudioConfigGetA2DPTtpLatency();

    if (hadToAdjustTtpLatency(&aptx_ad_ttp.low_latency_0, ttp_supported))
    {
        PRINT(("audio_input_a2dp: aptX Adaptive Non-Q2Q LL-0 mode TTP adjusted to %u ms\n", aptx_ad_ttp.low_latency_0));
    }

    if (hadToAdjustTtpLatency(&aptx_ad_ttp.low_latency_1, ttp_supported))
    {
        PRINT(("audio_input_a2dp: aptX Adaptive Non-Q2Q LL-1 mode TTP adjusted to %u ms\n", aptx_ad_ttp.low_latency_1));
    }

    if (hadToAdjustTtpLatency(&aptx_ad_ttp.high_quality, ttp_supported))
    {
        PRINT(("audio_input_a2dp: aptX Adaptive Non-Q2Q HQ mode TTP adjusted to %u ms\n", aptx_ad_ttp.high_quality));
    }

    ttp_supported = AudioConfigGetTWSTtpLatency();

    if (hadToAdjustTtpLatency(&aptx_ad_ttp.tws_legacy, ttp_supported))
    {
        PRINT(("audio_input_a2dp: aptX Adaptive Non-Q2Q TWS-Legacy mode TTP adjusted to %u ms\n", aptx_ad_ttp.tws_legacy));
    }

    return aptx_ad_ttp;
}

static void setTtpLatencyForAptxAdaptiveInNonQ2qMode(Operator rtp_op, aptx_adaptive_ttp_latencies_t ttp_in_non_q2q_mode)
{
    aptx_adaptive_ttp_in_ms_t aptx_ad_ttp = convertAptxAdaptiveTtpToOperatorsFormat(ttp_in_non_q2q_mode);

    aptx_ad_ttp = getAdjustedAptxAdaptiveTtpLatencies(aptx_ad_ttp);

    OperatorsRtpSetAptxAdaptiveTTPLatency(rtp_op, aptx_ad_ttp);
    OperatorsRtpEnableAptxAdModeNotifications(rtp_op, (Task)&aptx_ad_mode_notification);
}

static ttp_latency_t getTtpLatencyForAptxLowLatency(uint32 sample_rate)
{
    ttp_latency_t ttp_latency;

    ttp_latency.target_in_ms = (uint16) ((aptx_ll_target_ttp_level * APTX_LL_SAMPLES_PER_CODEWORD * APTX_LL_CHANNELS * MS_PER_SECOND) / sample_rate);
    ttp_latency.min_in_ms = APTX_LL_MINIMUM_LATENCY_IN_MS;
    ttp_latency.max_in_ms = (uint16) (ttp_latency.target_in_ms + APTX_LL_MAXIMUM_LATENCY_DELTA_IN_MS);

    return ttp_latency;
}

static void setTtpLatencyForBroadcaster(Operator rtp_op)
{
    OperatorsStandardSetTimeToPlayLatency(rtp_op, TTP_BA_LATENCY_IN_US);
    OperatorsStandardSetLatencyLimits(rtp_op, TTP_BA_MIN_LATENCY_LIMIT_US, TTP_BA_MAX_LATENCY_LIMIT_US);
}

static void setTtpLatency(Operator rtp_op, audio_codec_t decoder, uint32 sample_rate, A2dpPluginConnectParams *a2dp_connect_params)
{
    if (a2dp_connect_params->ba_output_plugin)
        setTtpLatencyForBroadcaster(rtp_op);
    else if (decoder == audio_codec_aptx_adaptive)
        setTtpLatencyForAptxAdaptiveInNonQ2qMode(rtp_op, a2dp_connect_params->aptx_ad_params.nq2q_ttp);
    else
    {
        ttp_latency_t ttp_latency = audioInputA2dpGetA2dpTtpLatency(decoder, sample_rate);
        audioInputConfigureRtpLatency(rtp_op, &ttp_latency);
    }
}

static void setBufferSize(Operator rtp_op, audio_codec_t decoder, A2dpPluginConnectParams *a2dp_connect_params)
{
    unsigned buffer_size;

    if (a2dp_connect_params->ba_output_plugin)
        buffer_size = BA_RTP_BUFFER_SIZE;
    else if (decoder == audio_codec_aptx_ll)
        buffer_size = a2dp_connect_params->aptx_sprint_params.initial_codec_level * APTX_LL_RTP_BUFFER_RATIO;
    else
        buffer_size = RTP_BUFFER_SIZE;

    if ((!AudioConfigGetUseSramForA2dp()) || (a2dp_connect_params->ba_output_plugin))
    {
        OperatorsStandardSetBufferSizeWithFormat(rtp_op, buffer_size, operator_data_format_encoded);
    }
}

void audioInputA2dpConfigureRtpOperator(Operator rtp_op, kymera_chain_handle_t chain, audio_codec_t decoder,
                                        uint32 sample_rate, A2dpPluginConnectParams *a2dp_connect_params)
{
    rtp_working_mode_t rtp_mode = getRtpWorkingMode(decoder, a2dp_connect_params);
    OperatorsRtpSetWorkingMode(rtp_op, rtp_mode);

    aptx_ll_target_ttp_level = a2dp_connect_params->aptx_sprint_params.target_codec_level;

    if (rtp_mode != rtp_passthrough)
    {
        OperatorsRtpSetCodecType(rtp_op, getRtpCodecType(decoder));
        OperatorsRtpSetContentProtection(rtp_op, a2dp_connect_params->content_protection);
        OperatorsStandardSetSampleRate(rtp_op, sample_rate);

        setTtpLatency(rtp_op, decoder, sample_rate, a2dp_connect_params);

        if (decoder == audio_codec_aac)
            OperatorsRtpSetAacCodec(rtp_op, ChainGetOperatorByRole(chain, decoder_role));

        if (decoder == audio_codec_aptx)
            OperatorsRtpSetMaximumPacketLength(rtp_op, getRtpPacketLength());
    }

    setBufferSize(rtp_op, decoder, a2dp_connect_params);
}

ttp_latency_t audioInputA2dpGetTwsTtpLatency(audio_codec_t decoder)
{
    ttp_latency_t ttp_latency = AudioConfigGetTWSTtpLatency();

    switch(decoder)
    {
        case audio_codec_aptx_adaptive:
            /*
             * There are multiple target TTP latencies for this codec, the plugin does not need to change the TTP latencies after connect
             * Return invalid target TTP latency
             */
            ttp_latency.target_in_ms = INVALID_TARGET_TTP_LATENCY;
            break;
        default:
            break;
    }

    return ttp_latency;
}

ttp_latency_t audioInputA2dpGetA2dpTtpLatency(audio_codec_t decoder, uint32 sample_rate)
{
    ttp_latency_t ttp_latency = AudioConfigGetA2DPTtpLatency();

    switch(decoder)
    {
        case audio_codec_aptx_ll:
            ttp_latency = getTtpLatencyForAptxLowLatency(sample_rate);
            break;
        case audio_codec_aptx_adaptive:
            /*
             * There are multiple target TTP latencies for this codec, the plugin does not need to change the TTP latencies after connect
             * Return invalid target TTP latency
             */
            ttp_latency.target_in_ms = INVALID_TARGET_TTP_LATENCY;
            break;
        default:
            break;
    }

    return ttp_latency;
}

void audioInputConfigureRtpLatency(Operator rtp_op, ttp_latency_t *ttp_latency)
{
    OperatorsStandardSetTimeToPlayLatency(rtp_op, TTP_LATENCY_IN_US(ttp_latency->target_in_ms));
    OperatorsStandardSetLatencyLimits(rtp_op, TTP_LATENCY_IN_US(ttp_latency->min_in_ms), TTP_LATENCY_IN_US(ttp_latency->max_in_ms));
}

bool audioInputA2dpisTimeToPlayControlledBySource(audio_codec_t decoder, A2dpPluginConnectParams *a2dp_connect_params)
{
    if (decoder == audio_codec_aptx_adaptive)
        if (a2dp_connect_params->aptx_ad_params.q2q_enabled)
            return TRUE;

    return FALSE;
}

uint16 audioInputA2dpGetLatency(audio_codec_t decoder, audio_input_context_t *ctx, bool *estimated)
{
    *estimated = FALSE;

    switch(decoder)
    {
        case audio_codec_aptx_ll:
            if(ctx)
                return getTtpLatencyForAptxLowLatency(ctx->sample_rate).target_in_ms;
            else
            {
                *estimated = TRUE;
                return TTP_APTX_LL_ESTIMATED_LATENCY_IN_MS;
            }
        default:
            return AudioConfigGetA2DPTtpLatency().target_in_ms;
    }
}

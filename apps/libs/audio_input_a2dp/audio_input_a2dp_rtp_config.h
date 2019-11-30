/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_rtp_config.h

DESCRIPTION
    RTP decode operator configuration
*/

#ifndef AUDIO_INPUT_A2DP_RTP_CONFIG_H_
#define AUDIO_INPUT_A2DP_RTP_CONFIG_H_

#include <audio_input_common.h>

#define INVALID_TARGET_TTP_LATENCY (0)

/****************************************************************************
DESCRIPTION
    Create the chain with the appropriate configuration filters.
*/
void audioInputA2dpConfigureRtpOperator(Operator rtp_op, kymera_chain_handle_t chain, audio_codec_t decoder,
                                        uint32 sample_rate, A2dpPluginConnectParams *a2dp_connect_params);

/****************************************************************************
DESCRIPTION
    Configures the RTP time to play latency
*/
void audioInputConfigureRtpLatency(Operator rtp_op, ttp_latency_t *ttp_latency);

/****************************************************************************
DESCRIPTION
    Get the TTP latency used for A2DP
*/
uint16 audioInputA2dpGetLatency(audio_codec_t decoder, audio_input_context_t *ctx, bool *estimated);

/****************************************************************************
DESCRIPTION
    Returns TRUE if the source/encoder controls TTP by adjusting the timestamp in the RTP header
*/
bool audioInputA2dpisTimeToPlayControlledBySource(audio_codec_t decoder, A2dpPluginConnectParams *a2dp_connect_params);

ttp_latency_t audioInputA2dpGetTwsTtpLatency(audio_codec_t decoder);

ttp_latency_t audioInputA2dpGetA2dpTtpLatency(audio_codec_t decoder, uint32 sample_rate);

#endif /* AUDIO_INPUT_A2DP_RTP_CONFIG_H_ */

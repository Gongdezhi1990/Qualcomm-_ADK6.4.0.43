/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_spdif_driver.c

DESCRIPTION
    Implementation of spdif driver which handles messages send
    by spdif decoder operator.
*/


#include <audio_mixer.h>
#include <operators.h>
#include <rtime.h>
#include <print.h>

#include "audio_input_spdif_private.h"
#include "audio_input_spdif_driver.h"
#include "audio_input_spdif_chain_config.h"
#include <audio_config.h>


#define ADAPT_TO_NEW_RATE       (0x1000)
#define NEW_DECODER_REQUIRED    (0x1001)
#define NEW_DATA_TYPE           (0x1002)
#define INPUT_RATE_VALID        (0x1003)
#define NEW_CHANNEL_STATUS      (0x1004)


#define OPERATOR_MSG_ID                         (2)
#define ADAPT_TO_NEW_RATE_MSG_SAMPLE_RATE       (4)

#define MS_TO_US(x) (x * US_PER_MS)
#define BUFFER_SIZE(latency_ms, sample_rate) (((latency_ms) * (sample_rate)) / MS_PER_SEC)


static void driverMessageHandler(Task task, MessageId id, Message message);
static void adaptToNewRate(uint32 sample_rate);

static TaskData DriverHandlerTask = {driverMessageHandler};
static uint32 TargetLatencyMs;


Task AudioInputSpdifGetDriverHandlerTask(void)
{
    return &DriverHandlerTask;
}

void AudioInputSpdifSetTargetLatency(void)
{
    audio_input_context_t *ctx = AudioInputSpdifGetContext();
    ttp_latency_t  ttp_latency = AudioConfigGetWiredTtpLatency();
    Operator spdif_ttp_passthrough = ChainGetOperatorByRole(ctx->chain, spdif_ttp_passthrough_role);

    OperatorsStandardSetTimeToPlayLatency(spdif_ttp_passthrough, MS_TO_US(ttp_latency.target_in_ms));
    OperatorsStandardSetLatencyLimits(spdif_ttp_passthrough, TTP_LATENCY_IN_US(ttp_latency.min_in_ms),
                                          TTP_LATENCY_IN_US(ttp_latency.max_in_ms));
    TargetLatencyMs = ttp_latency.target_in_ms;
}

static void driverMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(id);

    const uint16 *spdif_message = (const uint16*)message;
    uint16 msg_id = spdif_message[OPERATOR_MSG_ID];

    switch(msg_id)
    {
        case NEW_CHANNEL_STATUS:
        {
            PRINT(("SPDIF DRIVER: [0x%4x] NEW_CHANNEL_STATUS\n", msg_id));
            break;
        }

        case ADAPT_TO_NEW_RATE:
        {
            uint32 sample_rate = (uint32)(spdif_message[ADAPT_TO_NEW_RATE_MSG_SAMPLE_RATE] * 25);

            PRINT(("SPDIF DRIVER: [0x%4x] ADAPT_TO_NEW_RATE, sample rate: %d\n", msg_id, sample_rate));
            adaptToNewRate(sample_rate);

            break;
        }

        case INPUT_RATE_VALID:
        {
            PRINT(("SPDIF DRIVER: [0x%4x] INPUT_RATE_VALID\n", msg_id));
            break;
        }

        case NEW_DECODER_REQUIRED:
        {
            PRINT(("SPDIF DRIVER: [0x%4x] NEW_DECODER_REQUIRED\n", msg_id));
            break;
        }

        case NEW_DATA_TYPE:
        {
            PRINT(("SPDIF DRIVER: [0x%4x] NEW_DATA_TYPE\n", msg_id));
            break;
        }

        default:
        {
            PRINT(("SPDIF DRIVER: [0x%4x] Unsupported message!\n", msg_id));
            break;
        }
    }
}

static void adaptToNewRate(uint32 sample_rate)
{
    audio_input_context_t *ctx = AudioInputSpdifGetContext();
    Operator spdif_decoder = ChainGetOperatorByRole(ctx->chain, spdif_decoder_role);
    Operator spdif_ttp_passthrough = ChainGetOperatorByRole(ctx->chain, spdif_ttp_passthrough_role);

    OperatorsSpdifSetOutputSampleRate(spdif_decoder, sample_rate);

    OperatorsStandardSetSampleRate(spdif_ttp_passthrough, sample_rate);

    AudioMixerSetInputSampleRate(ctx->mixer_input, sample_rate);
}

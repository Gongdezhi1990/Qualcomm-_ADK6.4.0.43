/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_a2dp_chain_config.c

DESCRIPTION
    Create appropriate chain for audio_input_a2dps
*/

#include "audio_input_a2dp_chain_config.h"
#include "audio_input_a2dp_chain_config_stereo.h"
#include "audio_input_a2dp_chain_config_mono_sbc.h"
#include "audio_input_a2dp_chain_config_mono_aac.h"
#include "audio_input_a2dp_chain_config_mono_aptx.h"
#include <panic.h>


static kymera_chain_handle_t getMonoChain(audio_codec_t decoder)
{
    switch(decoder)
    {
        case audio_codec_sbc:
            return audioInputA2dpCreateMonoSbcChain();
        case audio_codec_aac:
            return audioInputA2dpCreateMonoAacChain();
        case audio_codec_aptx:
        case audio_codec_aptx_adaptive:
            return audioInputA2dpCreateMonoAptxChain(decoder);
        default:
            Panic();
            return NULL;
    }
}

kymera_chain_handle_t audioInputA2dpCreateChain(audio_codec_t decoder)
{
    kymera_chain_handle_t chain = NULL;

    if (audioInputA2dpIsSinkInMonoMode())
        chain = getMonoChain(decoder);
    else
        chain = audioInputA2dpCreateStereoChain(decoder);

    return chain;
}

kymera_chain_handle_t audioInputA2dpCreateBroadcasterChain(audio_codec_t decoder)
{
    kymera_chain_handle_t chain = NULL;

    if (audioInputA2dpIsSinkInMonoMode())
        Panic();
    else
        chain = audioInputA2dpCreateStereoChainWithoutSplitter(decoder);

    return chain;
}



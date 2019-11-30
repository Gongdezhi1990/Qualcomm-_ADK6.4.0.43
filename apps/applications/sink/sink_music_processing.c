/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_music_processing.c
 
DESCRIPTION
    Implementation of music processing settings handling.
*/

#include <audio_plugin_music_params.h>
#include <byte_utils.h>

#include "sink_a2dp.h"
#include "sink_music_processing.h"

A2DP_MUSIC_PROCESSING_T sinkMusicProcessingEqBankToProcessingMode(unsigned eq_bank)
{
    return eq_bank + A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK0;
}

unsigned sinkMusicProcessingEqProcessingModeToBankNum(A2DP_MUSIC_PROCESSING_T mode)
{
    return mode - A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK0;
}

void sinkMusicProcessingSetCurrentEqBank(unsigned eq_bank)
{
    setMusicProcessingMode(sinkMusicProcessingEqBankToProcessingMode(eq_bank));
}

unsigned sinkMusicProcessingGetCurrentEqBank(void)
{
    return sinkMusicProcessingEqProcessingModeToBankNum(getMusicProcessingMode());
}

bool sinkMusicProcessingUpdateEqBank(unsigned eq_bank)
{
    bool was_updated = FALSE;
    unsigned current_eq_bank = sinkMusicProcessingGetCurrentEqBank();

    if(current_eq_bank != eq_bank)
    {
        sinkMusicProcessingSetCurrentEqBank(eq_bank);
        was_updated = TRUE;
    }

    return was_updated;
}

void sinkMusicProcessingWriteToMsgPayload(uint8 *payload)
{
    uint16 eq_bank = sinkMusicProcessingEqProcessingModeToBankNum(getMusicProcessingMode());
    uint16 bypasses = getMusicProcessingBypassFlags();

    uint16 settings = bypasses | eq_bank;

    payload[0] = LOBYTE(settings);
    payload[1] = HIBYTE(settings);
}

void sinkMusicProcessingStoreFromMsgPayload(const uint8 *payload)
{
    uint16 settings = MAKEWORD_HI_LO(payload[1], payload[0]);

    uint16 eq_bank = settings & A2DP_MUSIC_CONFIG_USER_EQ_SELECT;
    uint16 bypasses = settings & ~A2DP_MUSIC_CONFIG_USER_EQ_SELECT;

    setMusicProcessingMode(sinkMusicProcessingEqBankToProcessingMode(eq_bank));
    setMusicProcessingBypassFlags(bypasses);
}

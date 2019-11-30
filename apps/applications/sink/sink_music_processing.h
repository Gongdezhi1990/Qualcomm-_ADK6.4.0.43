/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_music_processing.h
 
DESCRIPTION
    Module handling music processing settings.
    Music processing settings consist of music processing mode (see A2DP_MUSIC_PROCESSING_T)
    and music processing bypass flags.

    Music processing mode is stored internally in the sink application in different format
    than it is sent over the air for peers synchronisation purpose.

    When sent over the air both music processing bypass flags and selected EQ bank are packed together
    in one 16 bit word. Three least significant bits are selected EQ bank (0 to 6).
    The most significant bit is a flag indicating that bypass flags should be applied.
    Bits in between are used to carry bypass music enhancement flags.

    In the sink application bypass flags and processing mode are stored in two separate variables.
    Bit positions are not changed, thus they are still not overlapping.
    However processing mode is full A2DP_MUSIC_PROCESSING_T type instead of just EQ bank number.
    In effect processing mode is using 4 bits as oppose 3 bits used by EQ bank number.

*/

#ifndef SINK_MUSIC_PROCESSING_H_
#define SINK_MUSIC_PROCESSING_H_

#include <csrtypes.h>
#include <audio_plugin_if.h>

/* Number of bytes required to sent music processing settings over the air */
#define MUSIC_PROCESSING_PAYLOAD_SIZE 2

/*************************************************************************
NAME
    sinkMusicProcessingEqBankToProcessingMode

DESCRIPTION
    Converts EQ bank number to music processing mode.

**************************************************************************/
A2DP_MUSIC_PROCESSING_T sinkMusicProcessingEqBankToProcessingMode(unsigned eq_bank);

/*************************************************************************
NAME
    sinkMusicProcessingEqProcessingModeToBankNum

DESCRIPTION
    Converts music processing mode to EQ bank number.

**************************************************************************/
unsigned sinkMusicProcessingEqProcessingModeToBankNum(A2DP_MUSIC_PROCESSING_T mode);

/*************************************************************************
NAME
    sinkMusicProcessingSetCurrentEqBank

DESCRIPTION
    Writes EQ bank number to the sink application internal state.

**************************************************************************/
void sinkMusicProcessingSetCurrentEqBank(unsigned eq_bank);

/*************************************************************************
NAME
    sinkMusicProcessingGetCurrentEqBank

DESCRIPTION
    Reads EQ bank number from the sink application internal state.

**************************************************************************/
unsigned sinkMusicProcessingGetCurrentEqBank(void);

/*************************************************************************
NAME
    sinkMusicProcessingUpdateEqBank

DESCRIPTION
    If EQ bank number stored in the sink application state is different
    than eq_bank then EQ bank is updated.

RETURN
    TRUE if EQ bank in the sink application was updated, FALSE otherwise.

**************************************************************************/
bool sinkMusicProcessingUpdateEqBank(unsigned eq_bank);

/*************************************************************************
NAME
    sinkMusicProcessingWriteToMsgPayload

DESCRIPTION
    Populates payload with both music processing bypass flags and selected EQ bank.

**************************************************************************/
void sinkMusicProcessingWriteToMsgPayload(uint8 *payload);

/*************************************************************************
NAME
    sinkMusicProcessingWriteToMsgPayload

DESCRIPTION
    Updates the sink application internal state from the payload.
    Both music processing bypass flags and selected EQ bank are updated.

**************************************************************************/
void sinkMusicProcessingStoreFromMsgPayload(const uint8 *payload);

#endif /* SINK_MUSIC_PROCESSING_H_ */

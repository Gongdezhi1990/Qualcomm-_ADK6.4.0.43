/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup stream Stream Subsystem
 *
 * \file  stream_audio_data_format.h
 *
 * stream public header file. <br>
 * This file contains public stream types that may be parsed by the structure
 * sharing tool. <br>
 */


#ifndef _STREAM_AUDIO_DATA_FORMAT_H_
#define _STREAM_AUDIO_DATA_FORMAT_H_

#include "limits.h"

/****************************************************************************
Public Type Declarations
*/
/**
 * \brief  Enumeration of audio data formats. Endpoints that are to be
 *         connected together are expected to be set to the same data format
 *         unless hardware conversion between the two formats is supported.
 */
typedef enum
{
    /** Conventional 16 bit 2s complement data */
    AUDIO_DATA_FORMAT_16_BIT = 0,
    /** Generic data type - for now also ID = 0. To be used for cases
     * where generic data is vehiculated without any specific meaning or
     * assumption on its representation
     */
    AUDIO_DATA_FORMAT_GENERIC_DATA = AUDIO_DATA_FORMAT_16_BIT,
    /** fixed point format used by operators */
    AUDIO_DATA_FORMAT_FIXP = 1,
    /** Conventional 16 bit 2s complement with metadata */
    AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA = 2,
    /** fixed point format with metadata */
    AUDIO_DATA_FORMAT_FIXP_WITH_METADATA = 3,
    /** 16 bit stream with bytes swapped */
    AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP = 4,
    /** 16 bit stream with bytes swapped and metadata */
    AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA = 5,
    /** Conventional 13 bit 2s complement data with 3 bits of
     *  padding at the MSB */
    AUDIO_DATA_FORMAT_13_BIT = 6,
    /** Conventional 8 bit 2s complement data with 8 bits of
     *  padding at the MSB */
    AUDIO_DATA_FORMAT_8_BIT = 7,
    /** 8 bit mu-law encoded data with 8 bits of padding at
     * the MSB */
    AUDIO_DATA_FORMAT_MU_LAW = 8,
    /** 8 bit a-law encoded data with 8 bits of padding at
     * the MSB */
    AUDIO_DATA_FORMAT_A_LAW = 9,

    /* SPDIF input data type */
    SPDIF_INPUT_DATA_FORMAT = 10,

    /** Conventional 24 bit 2s complement data */
    AUDIO_DATA_FORMAT_24_BIT = 11,
    /* Others to be added here */    

    /* USB audio data type */
    USB_AUDIO_DATA_FORMAT = 12,

    /* Make sure the enum is a whole word for asm code compatibility */
    AUDIO_DATA_FORMAT_MAX = INT_MAX
} AUDIO_DATA_FORMAT;

/* Returns the used octets in a word for a given format. */
static inline unsigned get_octets_per_word(AUDIO_DATA_FORMAT format)
{
    if (format == AUDIO_DATA_FORMAT_24_BIT)
    {
        return 3;
    }
    else if ((format == AUDIO_DATA_FORMAT_FIXP) ||
             (format == AUDIO_DATA_FORMAT_FIXP_WITH_METADATA) ||
             (format == SPDIF_INPUT_DATA_FORMAT) ||
             (format == USB_AUDIO_DATA_FORMAT))
    {
       return (DAWTH / 8);
    }
    else /* AUDIO_DATA_FORMAT_16_BIT, AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA,
            AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP, AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA,
            AUDIO_DATA_FORMAT_13_BIT, AUDIO_DATA_FORMAT_8_BIT, AUDIO_DATA_FORMAT_MU_LAW,
            AUDIO_DATA_FORMAT_A_LAW*/
    {
        return 2;
    }
}

#endif /* _STREAM_AUDIO_DATA_FORMAT_H_ */

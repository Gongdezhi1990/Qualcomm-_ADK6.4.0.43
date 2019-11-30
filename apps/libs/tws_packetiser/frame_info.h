/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    frame_info.h

DESCRIPTION
    Codec frame information.

*/

#ifndef FRAME_INFO_H_
#define FRAME_INFO_H_

/*! Information regarding a compressed audio frame */
typedef struct __frame_info
{
    /*! The length of the frame in octets */
    uint32 length;
    /*! The number of stereo samples the frame represents once decoded */
    uint32 samples;
} frame_info_t;

/*! Store previous SBC header and resulting frame_info to avoid performing the same
    calculations on repeated frames having the same header */
typedef struct __frame_info_history_sbc
{
    /*! The previous value of the SBC header[0] */
    uint8 last_header1;
    /*! The previous value of the SBC header[1] */
    uint8 last_header2;
    /*! The previously calculated frame_info based on the header */
    frame_info_t frame_info;
} frame_info_history_sbc_t;

/*!
    @brief Calculate the frame info for a SBC frame.
    @param sbc Address of the SBC frame.
    @param len Either the length of the frame (if known), or the remaining
               data in the buffer from which frames are being read.
    @param frame_info The frame info is written here.
    @param history (optional) The history from the previous call.
    @return TRUE if the frame info was successfully read from the frame.
    The SBC frame header CRC is not calculated and checked.
*/
bool frameInfoSBC(const uint8 *sbc, uint32 len, frame_info_t *frame_info, frame_info_history_sbc_t *history);

/*!
    @brief Calculate the frame info for a aptX frame.
    @param len The length of the frame.
    @param frame_info The frame info is written here.
    @return TRUE if the frame info was successfully read from the frame.
*/
bool frameInfoAptx(uint32 len, frame_info_t *frame_info);

#endif

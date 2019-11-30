/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    extended_audio_header.h

DESCRIPTION
    The extended audio header describes the length of frames in a packet.
    The eah is an array of 16-bit words. The most significant bit of the word
    set indicates the final word in the array. The remaining bits of the word
    define the number of frames of a particular length.
*/

#ifndef EXTENDED_AUDIO_HEADER_H_
#define EXTENDED_AUDIO_HEADER_H_

/*! State used to read the extended audio header */
typedef struct __eah_reader_state
{
    /*! Number of frames in the packet */
    uint32 frames;

    /*! Sum of the length of all the frames in the packet */
    uint32 sum;

    /*! The length of the eah in bytes */
    uint32 eahlen;

    /*! Points to an array of the length of the frames in a packet. */
    uint16 *lengths;
} eah_reader_state_t;

/*!
  @brief Initialises and parses the extended audio header.
  @param state The extended audio header reader state.
  @param base Points to the start of the extended audio header.
  @param maxlen The maximum length of the extended audio header in bytes.
  @return Pointer to reader instance if the reader was successfully initialised,
  otherwise NULL.
*/
eah_reader_state_t* eahReaderInit(const uint8 *base, uint32 maxlen);

/*!
  @brief Destroy the extended audio header state.
  @param state The extended audio header reader state to destroy.
*/
void eahReaderDestroy(eah_reader_state_t *state);

/*!
  @brief Get the length of the indexed frame.
  @param state The extended audio header reader state.
  @param index The frame index to retrieve.
  @return The indexed frame length, or zero if the indexed frame
  is not described by the extended audio header.
  The frames can be indexed in any order.
*/
uint32 eahReaderGetFrameLen(eah_reader_state_t *state, uint32 index);

/*!
  @brief Get the length of all the frames described by the eah.
  @param state The extended audio header reader state.
  @per_frame_overhead The number of octets overhead per frame.
  @return The frames length.
*/
uint32 eahReaderGetFramesLen(eah_reader_state_t *state, uint32 per_frame_overhead);

/*!
  @brief Get the number of bytes in the eah.
  @param state The extended audio header reader state.
  @return The eah length.
*/
uint32 eahReaderGetEahLen(eah_reader_state_t *state);


/*!
  @brief Determine if there is a mismatch between the number of unread bytes in
  a packet and the number of bytes described by the extended audio header.
  @param state The extended audio header reader state.
  @param unread_packet_bytes The number of unread bytes in the packet.
  @param per_frame_overhead The number of octets overhead per frame.
  @return TRUE is there is a mismatch, otherwise FALSE.
*/
bool eahReaderPacketLengthMismatch(eah_reader_state_t *state,
                                   uint32 unread_packet_bytes,
                                   uint32 per_frame_overhead);


#endif

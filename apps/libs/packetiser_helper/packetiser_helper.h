/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    packetiser_helper.h

DESCRIPTION
    Definitions and helper/utility functions used by the packetiser libraries.
*/

#ifndef PACKETISER_HELPER_H_
#define PACKETISER_HELPER_H_

#include <csrtypes.h>
#include <source.h>
#include <sink.h>
#include <rtime.h>

/*********************************************************************
    Definitions related to the tag-data structure used in broadcast
    and TWS packets.
*********************************************************************/

/*! The length of the tag in octets */
#define TAG_LENGTH 1

/*! Bit 7 of the tag defines if the tag data is audio or non-audio */
#define TAG_AUDIO_TYPE_MASK ( 1 << 7)

/*! TRUE if the tag data type is audio */
#define TAG_IS_AUDIO(t) (((t) & TAG_AUDIO_TYPE_MASK) != 0)
/*! TRUE if the tag data type is non-audio */
#define TAG_IS_NON_AUDIO(t) (((t) & TAG_AUDIO_TYPE_MASK) == 0)

/*! Mask to obtain the scmst type from a audio data tag */
#define TAG_AUDIO_SCMST_MASK (1 << 5 | 1 << 6)
/*! Right shift to obtain the scmst type from the masked audio data tag */
#define TAG_AUDIO_SCMST_SHIFT 5

/*! For audio data tags, gets the scmst type */
#define TAG_GET_AUDIO_SCMST_TYPE(t) (((t) & TAG_AUDIO_SCMST_MASK) >> TAG_AUDIO_SCMST_SHIFT)
/*! For audio data tags, sets the scmst type */
#define TAG_SET_AUDIO_SCMST_TYPE(s) (((s) << TAG_AUDIO_SCMST_SHIFT) & TAG_AUDIO_SCMST_MASK)

/*! Mask to obtain the extended audio header from a audio data tag. If set,
  the extended audio header is present. */
#define TAG_AUDIO_EXTENDED_AUDIO_HEADER_MASK (1 << 4)

/*! TRUE if the extended audio header is present */
#define TAG_AUDIO_EXTENDED_HEADER_IS_PRESENT(t) (((t) & TAG_AUDIO_EXTENDED_AUDIO_HEADER_MASK) != 0)

/*! Mask to obtain the audio frame incomplete field from a audio data tag.
    If clear the audio data contains a complete audio frame or the final fragment of an audio frame. */
#define TAG_AUDIO_FRAME_INCOMPLETE_MASK (1 << 3)

/*! TRUE if the audio data contains a complete audio frame or the final fragment of an audio frame. */
#define TAG_AUDIO_FRAME_COMPLETE(t) (((t) & TAG_AUDIO_FRAME_INCOMPLETE_MASK) == 0)

/*! Mask to obtain the data type for non-audio data tags */
#define TAG_NON_AUDIO_TYPE_MASK (1 << 5 | 1 << 6 | 1 << 7)
/*! Right shift to obtain the data type from the masked non-audio data tag */
#define TAG_NON_AUDIO_TYPE_SHIFT 5

/*! For non-audio data tags, gets the non-audio tag type */
#define TAG_GET_NON_AUDIO_TYPE(t) (((t) & TAG_NON_AUDIO_TYPE_MASK) >> TAG_NON_AUDIO_TYPE_SHIFT)

/*! Mask to obtain the data length from a non-audio data tags. Note, for audio
  data tags, there is no length - the data fills the remaining space in the
  packet. */
#define TAG_NON_AUDIO_LENGTH_MASK (0x1f)

/*! For non-audio data tags, get the non-audio tag length */
#define TAG_GET_NON_AUDIO_LENGTH(t) ((t) & TAG_NON_AUDIO_LENGTH_MASK)

/*!  Enumerate the audio data tag scmst types. */
typedef enum __packetiser_helper_scmst
{
    packetiser_helper_scmst_copy_allowed = 0,
    packetiser_helper_scmst_copy_once = 1,
    packetiser_helper_scmst_copy_prohibited = 3,
    packetiser_helper_scmst_reserved = 2
} packetiser_helper_scmst_t;

/*! Enumerate the non-audio tag data types. */
typedef enum __packetiser_helper_non_audio_types
{
    /*! SCM here is sub-channel message, which is unrelated to scmst */
    packetiser_helper_non_audio_type_scm = 0,
    packetiser_helper_non_audio_type_reserved1 = 1,
    packetiser_helper_non_audio_type_reserved2 = 2,
    packetiser_helper_non_audio_type_reserved3 = 3
} packetiser_helper_non_audio_types_t;

/*********************************************************************
    Definitions related to the extended audio header used in broadcast
    and TWS packets.
*********************************************************************/

/*! The length of each entry in the extended audio data header */
#define EXTENDED_AUDIO_HEADER_LENGTH 2
/*! Mask to obtain the end (final word) from the extended audio header */
#define EXTENDED_AUDIO_HEADER_END_MASK 0x8000
/*! Right shift to obtain the end (final word) from the masked extended audio
  header */
#define EXTENDED_AUDIO_HEADER_END_SHIFT 15
/*! Mask to obtain number of frames from extended audio header */
#define EXTENDED_AUDIO_HEADER_NFRMS_MASK 0x7C00
/*! Right shift to obtain number of frames from masked extended audio header */
#define EXTENDED_AUDIO_HEADER_NFRMS_SHIFT 10
/*! Mask to obtain frame length from extended audio header */
#define EXTENDED_AUDIO_HEADER_FLEN_MASK 0x003FF
/*! Right shift to obtain frame length from masked extended audio header */
#define EXTENDED_AUDIO_HEADER_FLEN_SHIFT 0
/*! Extract the number of frames from an extended audio header */
#define EXTENDED_AUDIO_HEADER_GET_NFRMS(e)                              \
    (((e) & EXTENDED_AUDIO_HEADER_NFRMS_MASK) >> EXTENDED_AUDIO_HEADER_NFRMS_SHIFT)
/*! Extract the frame length from an extended audio header */
#define EXTENDED_AUDIO_HEADER_GET_FLEN(e)                               \
    (((e) & EXTENDED_AUDIO_HEADER_FLEN_MASK) >> EXTENDED_AUDIO_HEADER_FLEN_SHIFT)
/*! Set the the number of frames in an extended audio header */
#define EXTENDED_AUDIO_HEADER_SET_NFRMS(n)                              \
    (((n) << EXTENDED_AUDIO_HEADER_NFRMS_SHIFT) & EXTENDED_AUDIO_HEADER_NFRMS_MASK)
/*! Set the frame length in an extended audio header */
#define EXTENDED_AUDIO_HEADER_SET_FLEN(f)                               \
    (((f) << EXTENDED_AUDIO_HEADER_FLEN_SHIFT) & EXTENDED_AUDIO_HEADER_FLEN_MASK)

/*********************************************************************
    Definitions / functions related to the audio frame metadata used
    to describe the audio frames read/written between the apps subsystem
    and the audio subsystem.
*********************************************************************/

/*! The length of the frame metadata in octets */
#define AUDIO_FRAME_METADATA_LENGTH 9

typedef struct __audio_frame_metadata
{
    /*! The time-to-play the frame. The time base is the local time clock which
      is common between the audio sub-system and the application sub-system. */
    rtime_t ttp;
    /*! The sample period adjustment. A signed fraction in the range ~1.0 to
      -1.0 that specifies the adjustment to the nominal sample period to give
    the actual sample period at the time-to-play instant. If no sample period
    is available this field should be set to 0.0. The actual sample period is
    calculated as:
      sample_period = nominal_sample_period * (1 + sample_period_adjustment)
    */
    rtime_spadj_t sample_period_adjustment;
    /*! If asserted this frame is the start of a new stream or the audio
      frame configuration has changed. */
    bool start_of_stream;
} audio_frame_metadata_t;

/*!
  @brief Write the header with the fields in the md structure.
  @param md The audio frame metadata.
  @param hdr The header where the metadata will be written.
*/
void PacketiserHelperAudioFrameMetadataSet(const audio_frame_metadata_t *md, uint8 *hdr);

/*!
  @brief Parse the header to populate the md structure.
  @param hdr The header from where the metadata will be read.
  @param md The audio frame metadata to be populated.
 */
void PacketiserHelperAudioFrameMetadataGet(const uint8 *hdr, audio_frame_metadata_t *md);

/*!
  @brief Read the header from a source then parse the header to populate the
         md structure
  @param source The source from where the header will be read.
  @param md The audio frame metadata to be populated.
  @return TRUE on success, FALSE otherwise.
*/
bool PacketiserHelperAudioFrameMetadataGetFromSource(Source source, audio_frame_metadata_t *md);

#endif

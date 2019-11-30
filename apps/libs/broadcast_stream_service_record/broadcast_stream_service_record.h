/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    broadcast_stream_service_record.h

DESCRIPTION
    This library defines the Broadcast Stream Service Record (BSSR) and
    provides functions to parse a BSSR.

  The BSSR is an array of octets that defines the properties of one or more
  streams used by a broadcaster. The broadcaster shares the BSSR with a receiver
  during association. The receiver parses the BSSR to determine whether it
  supports the streams used by a broadcaster.

  The stream ID tag must be the first tag defined in a BSSR and is used to
  delimit the properties associated with each broadcast stream. The properties
  of a stream may be defined in any order. For example:
  STREAM_ID_0
  PROPERTY_OF_STREAM_ID_0_A
  PROPERTY_OF_STREAM_ID_0_B
  PROPERTY_OF_STREAM_ID_0_C
  STREAM_ID_1
  PROPERTY_OF_STREAM_ID_1_C
  PROPERTY_OF_STREAM_ID_1_A
  PROPERTY_OF_STREAM_ID_1_B

  Each entry in a BSSR is defined using a TAG|LENGTH|DATA structure. In this
  way a receiver can parse the BSSR, even if it does not understand all the tags
  defined.
*/

#ifndef BSSR_H_
#define BSSR_H_

#include <stdlib.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

/*! This stream ID must _not_ be used */
#define BSSR_STREAM_ID_INVALID 0

/*! The size of the tag type field (in octets).
  The tag type field defines the property type. */
#define BSSR_TAG_TYPE_SIZE                      0x01
/*! The size of the tag length field (in octets).
  The tag length field defines the number of octets of data that follow. */
#define BSSR_TAG_LEN_SIZE                       0x01
/*! The size of the header for each entry in the BSSR (in octets).
  The header comprises the tag type field and the tag length field. */
#define BSSR_TAG_HEADER_SIZE                    (BSSR_TAG_TYPE_SIZE + BSSR_TAG_LEN_SIZE)

/*! The tag type defining the stream ID of a broadcast stream. */
#define BSSR_TYPE_STREAM_ID                     0x00
/*! The tag type defining the security used by a broadcast stream.
  One instance must be defined per stream. */
#define BSSR_TYPE_SECURITY                      0x01
/*! The tag type defining the codec type used by a broadcast stream.
  One instance must be defined per stream. */
#define BSSR_TYPE_CODEC_TYPE                    0x02
/*! The tag type defining the CELT codec configuration used by a broadcast
  stream. CELT codec frames do not have a frame header defining the sample rate
  and the number of samples represented by each frame. Therefore these
  parameters are shared between the Broadcaster and Receiver using the BSSR.
  At least one instance must be defined if the codec type is set to CELT.
  More than one instance may be defined per stream if the codec configuration
  differs between sample rates supported by the stream.*/
#define BSSR_TYPE_CODEC_CONFIG_CELT             0x03
/*! The tag type defining the SBC codec configuration used by a broadcast
  stream. One instance must be defined per stream if the codec type is set to
  SBC. This differs from the CELT configuration, because SBC frames have a
  header that defines its contents, whereas CELT frames do not have a header.
  The SBC configuration is used to define the sample rates supported by the
  stream. */
#define BSSR_TYPE_CODEC_CONFIG_SBC              0x04
/*! The tag type defining the broadcast stream SCMS-T configuration.
 This tag is optional, with its omission meaning SCMS-T is disabled. */
#define BSSR_TYPE_SCMST_SUPPORT                 0x05
/*! The tag type defining the erasure coding scheme used by a broadcast stream.
  One instance must be defined per stream. */
#define BSSR_TYPE_ERASURE_CODE                  0x06
/*! The tag type defining the channel configuration used by a broadcast stream.
  One instance must be defined per stream. */
#define BSSR_TYPE_CHANNELS                      0x07
/*! The tag type defining the sample size used by a broadcast stream.
  One instance must be defined per stream. */
#define BSSR_TYPE_SAMPLE_SIZE                   0x08
/*! The tag type defining the AFH channel map update method.
 One instance must be defined per stream */
#define BSSR_TYPE_AFH_CHANNEL_MAP_UPDATE_METHOD 0x09
/*! All other tag types are reserved */
#define BSSR_TYPE_RESERVED                      0xF0

/*! The size of the stream ID data (in octets). */
#define BSSR_STREAM_ID_LEN                      0x01
/*! The size of the stream security data (in octets). */
#define BSSR_SECURITY_LEN                       0x02
/*! The size of the stream channels data (in octets). */
#define BSSR_CHANNELS_LEN                       0x02
/*! The size of the stream sample size data (in octets). */
#define BSSR_SAMPLE_SIZE_LEN                    0x01
/*! The size of the stream codec type data (in octets). */
#define BSSR_CODEC_TYPE_LEN                     0x01
/*! The size of the AFH channel map update method type data (in octets). */
#define BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_LEN  0x01

/*! The size of the stream codec configuration frequency sub-field (in octets).
 The frequency sub-field is a bitfield of the sample frequencies for which the
 codec configuration applies.*/
#define BSSR_CODEC_FREQ_LEN                     0x02
/*! The size of the stream codec configuration frame size sub-field (in octets).
 The frame size sub-field defines the codec frame size (in octets). */
#define BSSR_CODEC_FRAME_SIZE_LEN               0x02
/*! The size of the stream codec configuration frame samples sub-field (in octets).
 The frame samples sub-field defines the number of samples represented by each
 codec frame. */
#define BSSR_CODEC_FRAME_SAMPLES_LEN            0x02

/*! The size of the stream CELT codec configuration data (in octets). */
#define BSSR_CODEC_CONFIG_CELT_LEN              (BSSR_CODEC_FREQ_LEN + \
                                                 BSSR_CODEC_FRAME_SIZE_LEN + \
                                                 BSSR_CODEC_FRAME_SAMPLES_LEN)
/*! The size of the stream SBC codec configuration data (in octets). */
#define BSSR_CODEC_CONFIG_SBC_LEN               (BSSR_CODEC_FREQ_LEN)
/*! The size of the stream SCMST support configuraion data (in octets). */
#define BSSR_SCMST_SUPPORT_LEN                  0x01
/*! The size of the stream erasure code configuration data (in octets). */
#define BSSR_ERASURE_CODE_LEN                   0x01

/*! Stream security: key type mask. */
#define BSSR_SECURITY_KEY_TYPE_MASK             0x0003
/*! Stream security: the key is private. */
#define BSSR_SECURITY_KEY_TYPE_PRIVATE          0x0001
/*! Stream security: the key is temporary. */
#define BSSR_SECURITY_KEY_TYPE_TEMP             0x0002
#define BSSR_SECURITY_KEY_TYPE_RESERVED         0x007C
/*! Stream security: forward mask. */
#define BSSR_SECURITY_FORWARD_MASK              0x0080
/*! Stream security: the key may be forwarded to other devices. */
#define BSSR_SECURITY_FORWARD_ENABLED           0x0080
/*! Stream security: encryption type mask. */
#define BSSR_SECURITY_ENCRYPT_TYPE_MASK         0x0300
/*! Stream security: the broadcast is not encrypted. */
#define BSSR_SECURITY_ENCRYPT_TYPE_NONE         0x0100
/*! Stream security: the broadcast is encrypted using AES-CCM. */
#define BSSR_SECURITY_ENCRYPT_TYPE_AESCCM       0x0200
#define BSSR_SECURITY_ENCRYPT_TYPE_RESERVED     0xFC00
#define BSSR_SECURITY_TYPE_INVALID              0xFFFF

/*! Codec type: the supported broadcast codec is SBC. */
#define BSSR_CODEC_TYPE_SBC                     0x00
/*! Codec type: the supported broadcast codec is CELT. */
#define BSSR_CODEC_TYPE_CELT                    0x01
#define BSSR_CODEC_TYPE_INVALID                 0xFF

/*! Codec config frequency sub-field: data mask. */
#define BSSR_CODEC_FREQ_MASK                    0x7FFF
/*! Codec config frequency sub-field: 8kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_8KHZ                    0x0001
/*! Codec config frequency sub-field: 11.025kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_11025HZ                 0x0002
/*! Codec config frequency sub-field: 12kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_12KHZ                   0x0004
/*! Codec config frequency sub-field: 16kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_16KHZ                   0x0008
/*! Codec config frequency sub-field: 22.05kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_22050HZ                 0x0010
/*! Codec config frequency sub-field: 24kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_24KHZ                   0x0020
/*! Codec config frequency sub-field: 32kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_32KHZ                   0x0040
/*! Codec config frequency sub-field: 44.1kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_44100HZ                 0x0080
/*! Codec config frequency sub-field: 48kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_48KHZ                   0x0100
/*! Codec config frequency sub-field: 64kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_64KHZ                   0x0200
/*! Codec config frequency sub-field: 88.2kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_88200HZ                 0x0400
/*! Codec config frequency sub-field: 96kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_96KHZ                   0x0800
/*! Codec config frequency sub-field: 128kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_128KHZ                  0x1000
/*! Codec config frequency sub-field: 176.4kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_176400HZ                0x2000
/*! Codec config frequency sub-field: 192kHz is a supported frequency. */
#define BSSR_CODEC_FREQ_192KHZ                  0x4000
#define BSSR_CODEC_FREQ_RESERVED                0x8000

/*! Channels configuration: data mask. */
#define BSSR_CHANNELS_MASK                      0x000F
/*! Channels configuration: the stream is mono. */
#define BSSR_CHANNELS_MONO                      0x0001
/*! Channels configuration: the stream is stereo. */
#define BSSR_CHANNELS_STEREO                    0x0004
#define BSSR_CHANNELS_RESERVED                  0xFFF0

/*! Sample size configuration: data mask. */
#define BSSR_SAMPLE_SIZE_MASK                   0x07
/*! Sample size configuration: the sample size is 8 bits. */
#define BSSR_SAMPLE_SIZE_8_BIT                  0x01
/*! Sample size configuration: the sample size is 16 bits. */
#define BSSR_SAMPLE_SIZE_16_BIT                 0x02
/*! Sample size configuration: the sample size is 24 bits. */
#define BSSR_SAMPLE_SIZE_24_BIT                 0x04
#define BSSR_SAMPLE_SIZE_RESERVED               0xF8

/*! SCMS-T configuration: data mask. */
#define BSSR_SCMST_SUPPORT_MASK                 0x03
/*! SCMS-T configuration: The recipient must not copy the data. */
#define BSSR_SCMST_SUPPORT_COPY                 0x01
/*! SCMS-T configuration: The recipient must not forward the data. */
#define BSSR_SCMST_SUPPORT_FORWARD              0x02
#define BSSR_SCMST_SUPPORT_RESERVED             0xFC

/*! Erasure code scheme: data mask. */
#define BSSR_ERASURE_CODE_MASK                  0xff
/*! Erasure code scheme: no code. */
#define BSSR_ERASURE_CODE_NONE                  0x00
#define BSSR_ERASURE_CODE_2_5                   0x01
#define BSSR_ERASURE_CODE_3_7                   0x02
#define BSSR_ERASURE_CODE_3_8                   0x03
#define BSSR_ERASURE_CODE_3_9                   0x04
#define BSSR_ERASURE_CODE_INVALID               0xFF

/*! The stream has no AFH channel map update method */
#define BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_NONE 0x00
/*! The stream uses SCM to transport AFH channel map updates from broadcaster
    to receivers. */
#define BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_SCM 0x01
/*! The stream uses the triggered CSB sync train method to transport SCM
    channel map updates from broadcaster to receivers.
*/
#define BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_TRIGGERED_SYNC_TRAIN 0x02
#define BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_RESERVED 0xFC

/****************************************************************************
 * Typedefs and Enumerations
 ****************************************************************************/
/*! The CELT codec configuration. */
typedef struct
{
    /*! Bitfield defining the sample rates to which this config applies. */
    uint16 frequencies;
    /*! The frame size in octets of each CELT frame. */
    uint16 frame_size;
    /*! The number of audio samples represented by each CELT frame. */
    uint16 frame_samples;
} codec_config_celt;

/*! The SBC codec configuration. */
typedef struct
{
    /*! Bitfield defining the sample rates supported by the SBC codec. */
    uint16 frequencies;
} codec_config_sbc;


/****************************************************************************
 * Macros
 ****************************************************************************/

/*! The most significant octet of a 16-bit word. */
#define BSSR_MSB_16(x)   (((x) >> 8) & 0xFF)
/*! The least significant octet of a 16-bit word. */
#define BSSR_LSB_16(x)   ((x) & 0xFF)

/*! Macro to split a uint16 as two uint8s.
    Use this to initialise an array of uint8s with 16bit data. */
#define BSSR_UINT16_TO_UINT8S(x) BSSR_MSB_16((x)), BSSR_LSB_16((x))

/*! Macro to combine two uint8s into a uint16.
    Use this to retrieve uint16 data from an array of uint8s */
#define BSSR_UINT8S_TO_UINT16(x) ((*(x) << 8) + *((x) + 1))

/* Macro to create an entry in a BSSR. Use this to initialise an array of uint8s
   with a series of BSSR entries. This macro requires that a consistent naming
   scheme is used for the type and length definitions. Unfortunately variadic
   macros are not supported with this compiler.
*/
#define BSSR_ENTRY_TYPE_LEN(base_name) BSSR_TYPE_ ## base_name, BSSR_ ## base_name ## _LEN
/*! Create a BSSR entry with a single value argument. */
#define BSSR_ENTRY1(base_name, value1) BSSR_ENTRY_TYPE_LEN(base_name), value1
/*! Create a BSSR entry with two value arguments. */
#define BSSR_ENTRY2(base_name, value1, value2) BSSR_ENTRY_TYPE_LEN(base_name), value1, value2
/*! Create a BSSR entry with three value arguments. */
#define BSSR_ENTRY3(base_name, value1, value2, value3) BSSR_ENTRY_TYPE_LEN(base_name), value1, value2, value3

/****************************************************************************
 * Functions
 ****************************************************************************/

/*! @brief Get the codec_type for a given stream id.

    @param stream_records [IN] Pointer to the stream records array.
    @param stream_records_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream ID for which the codec_type should be retrieved.
    @param codec_type [OUT] The retrieved codec_type.

    @returns bool TRUE if the codec_type was found in the record.
                  FALSE if the codec_type was not found in the record.
*/
bool bssrGetCodecType(const uint8* stream_records, uint16 stream_records_len,
                      uint8 stream_id, uint8 *codec_type);

/*! @brief Get the security for a given stream id.

    @param stream_records [IN] Pointer to the stream records array.
    @param stream_records_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream ID for which the security should be retrieved.
    @param security [OUT] The retrieved security.

    @returns bool TRUE if the security was found in the record.
                  FALSE if the security was not found in the record.
*/
bool bssrGetSecurity(const uint8* stream_records, uint16 stream_records_len,
                         uint8 stream_id, uint16 *security);

/*! @brief Get the erasure_code for a given stream id.

    @param stream_records [IN] Pointer to the stream records array.
    @param stream_records_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream ID for which the erasure_code should be retrieved.
    @param erasure_code [OUT] The retrieved erasure_code.

    @returns bool TRUE if the erasure_code was found in the record.
                  FALSE if the erasure_code was not found in the record.
*/
bool bssrGetErasureCode(const uint8* stream_records, uint16 stream_records_len,
                        uint8 stream_id, uint8 *erasure_code);

/*! @brief Get the sample_size for a given stream id.

    @param stream_records [IN] Pointer to the stream records array.
    @param stream_records_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream ID for which the erasure_code should be retrieved.
    @param sample_size [OUT] The retrieved sample_size.

    @returns bool TRUE if the sample_size was found in the record.
                  FALSE if the sample_size was not found in the record.
*/
bool bssrGetSampleSize(const uint8* stream_records, uint16 stream_records_len,
                       uint8 stream_id, uint8 *sample_size);

/*! @brief Get the channels for a given stream id.

    @param stream_records [IN] Pointer to the stream records array.
    @param stream_records_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream ID for which the channels should be retrieved.
    @param channels [OUT] The retrieved channels.

    @returns bool TRUE if the channels was found in the record.
                  FALSE if the channels was not found in the record.
*/
bool bssrGetChannels(const uint8* stream_records, uint16 stream_records_len,
                     uint8 stream_id, uint16 *channels);

/*! @brief Get the nth instance of the CELT codec config for a given stream id.

    @param stream_records [IN] Pointer to the stream records array.
    @param stream_records_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream ID for which the channels should be retrieved.
    @param n [IN] The zero based nth instance. Set to 0 to retrieve the first instance.
    @param codec_config [OUT] The retrieved codec_config.

    @returns bool TRUE if the nth codec_config was found in the record.
                  FALSE if the nth codec_config was not found in the record.
*/
bool bssrGetNthCodecConfigCelt(const uint8* stream_records, uint16 stream_records_len,
                               uint8 stream_id, uint8 n, codec_config_celt *codec_config);

/*! @brief Get the nth instance of the SBC codec config for a given stream id.

    @param stream_records [IN] Pointer to the stream records array.
    @param stream_records_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream ID for which the channels should be retrieved.
    @param n [IN] The zero based nth instance. Set to 0 to retrieve the first instance.
    @param codec_config [OUT] The retrieved codec_config.

    @returns bool TRUE if the nth codec_config was found in the record.
                  FALSE if the nth codec_config was not found in the record.
*/
bool bssrGetNthCodecConfigSBC(const uint8* stream_records, uint16 stream_records_len,
                              uint8 stream_id, uint8 n, codec_config_sbc *codec_config);


/*! @brief Get the AFH channel map update method for a given stream_id.

    @param stream_records [IN] Pointer to the stream records array.
    @param stream_records_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream ID for which the AFH channel map update method should be retrieved.
    @param afh_channel_map_update_method [OUT] The retrieved AFH channel map update method.

    @returns bool TRUE if the method was found in the record.
                  FALSE if the method was not found in the record.
*/
bool bssrGetAFHChannelMapUpdateMethod(const uint8 *stream_records, uint16 stream_records_len,
                                      uint8 stream_id, uint8 *afh_channel_map_update_method);

/*! @brief Get the CELT codec config for a given stream_id and sample rate. 
 
    @param stream_records [IN] Pointer to the stream records array.
    @param stream_records_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream ID for which the codec config should be retrieved.
    @param rate [IN] BSSR bit definition of the sample rate to match, e.g. BSSR_CODEC_FREQ_44100HZ.
    @param codec_config [OUT] The retrieved codec_config.
 
    @returns bool TRUE if a matching codec_config was found in the record.
                  FALSE if a matching codec_config was not found in the record.
 */
bool bssrGetCodecConfigCeltByRate(const uint8* stream_records, uint16 stream_records_len,
                                  uint8 stream_id, uint16 rate,
                                  codec_config_celt *codec_config);
#endif /* BSSR_H_ */

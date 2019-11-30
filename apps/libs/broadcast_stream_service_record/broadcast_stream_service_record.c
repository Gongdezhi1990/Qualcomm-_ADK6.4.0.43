/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    broadcast_stream_service_record.c

DESCRIPTION
    Broadcast Stream Service Record Parsing.
*/

#include "broadcast_stream_service_record.h"

/*! @brief Return the length field of the current tag. */
static uint8 bssr_current_tag_len(const uint8 *stream_records)
{
    return *(stream_records + BSSR_TAG_TYPE_SIZE);
}

/*! @brief Return the uint8 data field of the current tag. */
static uint8 bssr_current_uint8_data(const uint8 *stream_records)
{
    return *(stream_records + BSSR_TAG_HEADER_SIZE);
}

/*! @brief Return the uint16 data field of the current tag. */
static uint16 bssr_current_uint16_data(const uint8 *stream_records)
{
    const uint8 *dptr = stream_records + BSSR_TAG_HEADER_SIZE;
    return BSSR_UINT8S_TO_UINT16(dptr);
}

/*! @brief Advance the pointer past the current tag to the next one. */
static bool bssr_move_to_next_tag(const uint8 **stream_records_pptr, uint16 *stream_records_len)
{
    const uint8 *end = *stream_records_pptr + *stream_records_len - 1;
    uint16 tag_len = BSSR_TAG_HEADER_SIZE + bssr_current_tag_len(*stream_records_pptr);
    /* Ensure there is enough space to advance to the next tag */
    if (*stream_records_pptr + tag_len <= end)
    {
        /* Advance the pointer, past this tag */
        *stream_records_pptr += tag_len;
        *stream_records_len -= tag_len;
        return TRUE;
    }
    /* The BSSR seems to be corrupt, as the buffer ends prior to the completion
       of the last complete record. Ignore the problem and jump to the end of
       the buffer. */
    *stream_records_pptr = end;
    *stream_records_len = 0;
    return FALSE;
}

/*! @brief Move to the start of the next stream id record. */
static bool bssr_move_to_next_stream_id(const uint8 **stream_records_pptr,
                                        uint16* stream_records_len_ptr)
{
    /* Ensure we have some stream record data to work with */
    if (*stream_records_len_ptr > (BSSR_TAG_HEADER_SIZE + BSSR_STREAM_ID_LEN))
    {
        /* While the current item is not a STREAM_ID tag type */
        while (**stream_records_pptr != BSSR_TYPE_STREAM_ID)
        {
            if (!bssr_move_to_next_tag(stream_records_pptr, stream_records_len_ptr))
            {
                /* Ran out of data, stream id not found */
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

/*! @brief Move to the start of a given stream id record. */
static bool bssr_move_to_stream_id(const uint8 **stream_records_pptr,
                                   uint16* stream_records_len_ptr,
                                   uint8 stream_id)
{
    while(bssr_move_to_next_stream_id(stream_records_pptr,
                                      stream_records_len_ptr))
    {
        /* Validate the length is as expected */
        if (bssr_current_tag_len(*stream_records_pptr) == BSSR_STREAM_ID_LEN)
        {
            /* Determine if the requested stream_id has been found */
            if (bssr_current_uint8_data(*stream_records_pptr) == stream_id)
            {
                return TRUE;
            }
            /* The current stream id is not the requested one, so skip over this
               record and then look for the start of the next stream id */
            if (!bssr_move_to_next_tag(stream_records_pptr, stream_records_len_ptr))
            {
                /* Ran out of data, stream id not found */
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}

/*! @brief Move to the start of the nth instance of a data type in a stream
  record for a given stream_id and data_type. The instance is zero-based, so
  to find the first instance, set instance to 0.
*/
static bool bssr_move_to_nth_instance_of_data_type_in_stream_id(const uint8 **stream_records_pptr,
                                                                uint16* stream_records_len_ptr,
                                                                uint8 data_type,
                                                                uint8 stream_id,
                                                                uint8 instance)
{
    if (stream_records_pptr && *stream_records_pptr && stream_records_len_ptr &&
        bssr_move_to_stream_id(stream_records_pptr, stream_records_len_ptr, stream_id))
    {
        int16 discovered = -1;
        /* While the end of the buffer is not reached and the end of the
           records associated with this stream id is not reached, iterate
           over the records until the nth instance of the data_type is found. */
        while (bssr_move_to_next_tag(stream_records_pptr, stream_records_len_ptr) &&
               **stream_records_pptr != BSSR_TYPE_STREAM_ID)
        {
            if (**stream_records_pptr == data_type && ++discovered == (int16)instance)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*! @brief Get the codec type for a given stream_id. */
bool bssrGetCodecType(const uint8 *stream_records, uint16 stream_records_len,
                      uint8 stream_id, uint8 *codec_type)
{
    if (codec_type &&
        bssr_move_to_nth_instance_of_data_type_in_stream_id(&stream_records,
                                                            &stream_records_len,
                                                            BSSR_TYPE_CODEC_TYPE,
                                                            stream_id,
                                                            0))
    {
        /* Found codec type tag in this stream record */
        /* Validate the length is as expected */
        if (bssr_current_tag_len(stream_records) == BSSR_CODEC_TYPE_LEN)
        {
            *codec_type = bssr_current_uint8_data(stream_records);
            return TRUE;
        }
    }
    return FALSE;
}

static bool bssrGetUint16(const uint8 *stream_records, uint16 stream_records_len,
                          uint8 stream_id, uint8 data_type, uint16 *data)
{
    if (data &&
        bssr_move_to_nth_instance_of_data_type_in_stream_id(&stream_records,
                                                            &stream_records_len,
                                                            data_type,
                                                            stream_id,
                                                            0))
    {
        /* Found data type tag in this stream record */
        /* Validate the length is as expected */
        if (bssr_current_tag_len(stream_records) == 2)
        {
            *data = bssr_current_uint16_data(stream_records);
            return TRUE;
        }
    }
    return FALSE;
}

static bool bssrGetUint8(const uint8 *stream_records, uint16 stream_records_len,
                         uint8 stream_id, uint8 data_type, uint8 *data)
{
    if (data &&
        bssr_move_to_nth_instance_of_data_type_in_stream_id(&stream_records,
                                                            &stream_records_len,
                                                            data_type,
                                                            stream_id,
                                                            0))
    {
        /* Found data type tag in this stream record */
        /* Validate the length is as expected */
        if (bssr_current_tag_len(stream_records) == 1)
        {
            *data = bssr_current_uint8_data(stream_records);
            return TRUE;
        }
    }
    return FALSE;
}

/*! @brief Get the security for a given stream_id. */
bool bssrGetSecurity(const uint8 *stream_records, uint16 stream_records_len,
                     uint8 stream_id, uint16 *security)
{
    return bssrGetUint16(stream_records, stream_records_len, stream_id,
                         BSSR_TYPE_SECURITY, security);
}

/*! @brief Get the Erasure Code for a given stream_id. */
bool bssrGetErasureCode(const uint8 *stream_records, uint16 stream_records_len,
                        uint8 stream_id, uint8 *erasure_code)
{
    return bssrGetUint8(stream_records, stream_records_len, stream_id,
                        BSSR_TYPE_ERASURE_CODE, erasure_code);
}

/*! @brief Get the sample size for a given stream_id. */
bool bssrGetSampleSize(const uint8 *stream_records, uint16 stream_records_len,
                       uint8 stream_id, uint8 *sample_size)
{
    return bssrGetUint8(stream_records, stream_records_len, stream_id,
                        BSSR_TYPE_SAMPLE_SIZE, sample_size);
}

/*! @brief Get the channels for a given stream_id. */
bool bssrGetChannels(const uint8 *stream_records, uint16 stream_records_len,
                     uint8 stream_id, uint16 *channels)
{
    return bssrGetUint16(stream_records, stream_records_len, stream_id,
                         BSSR_TYPE_CHANNELS, channels);
}

/*! @brief Get the nth instance of the Celt Codec Config for a given stream_id. */
bool bssrGetNthCodecConfigCelt(const uint8 *stream_records, uint16 stream_records_len,
                               uint8 stream_id, uint8 n, codec_config_celt *codec_config)
{
    if (codec_config &&
        bssr_move_to_nth_instance_of_data_type_in_stream_id(&stream_records,
                                                            &stream_records_len,
                                                            BSSR_TYPE_CODEC_CONFIG_CELT,
                                                            stream_id, n))
    {
        /* Found codec_config tag in this stream record */
        /* Validate the length is as expected */
        if (bssr_current_tag_len(stream_records) == BSSR_CODEC_CONFIG_CELT_LEN)
        {
            /* Jump over the header then store the data */
            stream_records += BSSR_TAG_HEADER_SIZE;
            codec_config->frequencies = BSSR_UINT8S_TO_UINT16(stream_records);
            stream_records += BSSR_CODEC_FREQ_LEN;
            codec_config->frame_size = BSSR_UINT8S_TO_UINT16(stream_records);
            stream_records += BSSR_CODEC_FRAME_SIZE_LEN;
            codec_config->frame_samples = BSSR_UINT8S_TO_UINT16(stream_records);
            return TRUE;
        }
    }
    return FALSE;
}

/*! @brief Get the nth instance of the SBC Codec Config for a given stream_id. */
bool bssrGetNthCodecConfigSBC(const uint8 *stream_records, uint16 stream_records_len,
                              uint8 stream_id, uint8 n, codec_config_sbc *codec_config)
{
    if (codec_config &&
        bssr_move_to_nth_instance_of_data_type_in_stream_id(&stream_records,
                                                            &stream_records_len,
                                                            BSSR_TYPE_CODEC_CONFIG_SBC,
                                                            stream_id, n))
    {
        /* Found codec_config tag in this stream record */
        /* Validate the length is as expected */
        if (bssr_current_tag_len(stream_records) == BSSR_CODEC_CONFIG_SBC_LEN)
        {
            /* Jump over the header then store the data */
            stream_records += BSSR_TAG_HEADER_SIZE;
            codec_config->frequencies = BSSR_UINT8S_TO_UINT16(stream_records);
            return TRUE;
        }
    }
    return FALSE;
}

/*! @brief Get the AFH channel map update method for a given stream_id. */
bool bssrGetAFHChannelMapUpdateMethod(const uint8 *stream_records, uint16 stream_records_len,
                                      uint8 stream_id, uint8 *afh_channel_map_update_method)
{
    return bssrGetUint8(stream_records, stream_records_len, stream_id,
                        BSSR_TYPE_AFH_CHANNEL_MAP_UPDATE_METHOD, afh_channel_map_update_method);
}

/*! @brief Get the CELT codec config for a given stream_id and sample rate. */
bool bssrGetCodecConfigCeltByRate(const uint8* stream_records,
                                  uint16 stream_records_len,
                                  uint8 stream_id, uint16 rate,
                                  codec_config_celt *codec_config)
{
    uint16 instance = 0;

    /* loop through all the CELT codec configs for the stream ID */
    for (instance = 0;
         bssrGetNthCodecConfigCelt(stream_records, stream_records_len, stream_id, instance, codec_config);
         instance++)
    {
        /* is this config for the requested rate? */
        if (codec_config->frequencies & rate)
            return TRUE;
    }

    return FALSE;
}


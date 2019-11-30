/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "rtp_decode_private.h"
#include "rtp_decode_struct.h"
#include "opmgr/opmgr_for_ops.h"

#ifdef INSTALL_OPERATOR_AAC_DECODE
#include "aac_c.h"

/*
 * get_AAC_codec
 */
void* get_AAC_codec(unsigned ext_op_id)
{
    OPERATOR_DATA *op_data = get_op_data_from_id(EXT_TO_INT_OPID(ext_op_id));
    if (op_data != NULL)
    {
        if (op_data->cap_data->id == CAP_ID_AAC_DECODER)
        {
            /* This is the aac decode capability we hoped for. Return the decoder which
             * happens to be the extra op data because the decoder is the first field
             * in the structure. More details: AAC_DEC_OP_DATA first field is decoder_data
             * which first filed is codec of the type DECODER. The decoder is used to
             * decode the aac frame so if the hierarchy is not true any more this
             * function need changes. */
            return op_data->extra_op_data;
        }
        else
        {
            return NULL;
        }
    }
    return NULL;
}


/**
 * Decodes the AAC frame and populates the frame data structure with the necessary
 * information to generate a timestamped tags.
 *
 * \param opx_data Pointer to the RTP operator data.
 * \param payload_size Pointer to the RTP operator data.
 * \param frame_data Pointer to the frame data structure.
 */
static void aac_sample_count(RTP_DECODE_OP_DATA *opx_data, unsigned payload_size,
        RTP_FRAME_DECODE_DATA *frame_decode_data)
{
    DECODER *aac_codec_struc = (DECODER *)opx_data->aac_codec;
    dummy_decoder aac_decoder;

    /* Initialise the input parameters for the codec.*/
    /* Set the  main decoder's structure.*/
    aac_decoder.codec_struc = aac_codec_struc;
    /* Set the return structure. */
    aac_decoder.frame_dec_struc = frame_decode_data;

    /* Set the payload size which is all the available data. */
    aac_decoder.payload_size = payload_size;

    /* Set the input buffer. */
    aac_decoder.in_cbuffer = opx_data->clone_op_buffer->buffer;

    /* Set the bit position. */
    if (opx_data->clone_op_buffer->read_offset == 1)
    {
        /* Second octet from the word. */
        aac_decoder.bit_position = 8;
    }
    else
    {
        /* First octet from the word. */
        aac_decoder.bit_position = 16;
    }

    if (opx_data->aac_utility_select == AAC_LC_OVER_LATM)
    {
        aacdec_samples_in_packet_lc(&aac_decoder);
    }
    else
    {
        aacdec_samples_in_packet(&aac_decoder);
    }

}
#else

void* get_AAC_codec(unsigned ext_op_id)
{
    return NULL;
}


static void aac_sample_count(RTP_DECODE_OP_DATA *opx_data, unsigned payload_size,
        RTP_FRAME_DECODE_DATA *frame_decode_data)
{

}
#endif

/**
 * Decodes the sbc frame header and populates the frame data structure with the necessary
 * information to generate a timestamped tags.
 *
 * \param opx_data Pointer to the RTP operator data.
 * \param payload_size Pointer to the RTP operator data.
 * \param frame_data Pointer to the frame data structure.
 */
static void sbc_sample_count(RTP_DECODE_OP_DATA* opx_data, unsigned payload_size,
        RTP_FRAME_DECODE_DATA* frame_data)
{
    unsigned sbc_header[SBC_SAMPLE_COUNT_HEADER_SIZE];
    /* Read and unpack the sbc header. */
    obuffer_unpack_from_offset((int*) &sbc_header, opx_data->clone_op_buffer, 0,
            SBC_SAMPLE_COUNT_HEADER_SIZE);
    if (sbc_header[0] == SBC_HEADER0_SYNC)
    {
        unsigned mode = sbc_header[1] & SBC_HEADER1_CHANNEL_MASK;
        unsigned nrof_blocks = SBC_HEADER1_GET_NROF_BLOCKS(sbc_header[1]);
        unsigned nrof_subbands = SBC_HEADER1_GET_NROF_SUBBANDS(sbc_header[1]);
        unsigned bitpool = SBC_HEADER2_BITPOOL(sbc_header[2]);
        unsigned frame_length = 0;
        unsigned sbc_frames_in_payload = 0;

        switch (mode)
        {
            case SBC_HEADER1_CHANNEL_MONO:
            {
                /* nrof_channels = 1;
                 * frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +  ceil((nrof_blocks * nrof_channels * bitpool) / 8); */
                frame_length = 4 + (nrof_subbands) / 2
                        + ((nrof_blocks * bitpool) + 7) / 8;
                break;
            }
            case SBC_HEADER1_CHANNEL_DUAL:
            {
                /* nrof_channels = 2;
                 * frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +  ceil((nrof_blocks * nrof_channels * bitpool) / 8); */
                frame_length = 4 + nrof_subbands
                        + ((nrof_blocks * 2 * bitpool) + 7) / 8;
                break;
            }
            case SBC_HEADER1_CHANNEL_STEREO:
            {
                /* nrof_channels = 2;
                 * frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +  ceil((nrof_blocks * bitpool) / 8); */
                frame_length = 4 + nrof_subbands
                        + ((nrof_blocks * bitpool) + 7) / 8;
                break;
            }
            case SBC_HEADER1_CHANNEL_JOINT:
            {
                /* nrof_channels = 2;
                 * frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +  ceil((nrof_subbands + nrof_blocks *  bitpool)) / 8); */
                frame_length = 4 + nrof_subbands
                        + ((nrof_subbands + nrof_blocks * bitpool) + 7) / 8;
                break;
            }
            default:
            {
                break;
            }
        }
        if (frame_length != 0)
        {
            if (payload_size % frame_length == 0)
            {
                frame_data->frame_length = frame_length;
                sbc_frames_in_payload = payload_size / frame_length;
            }
            else
            {
                /* Probably the frame length calculation is wrong or the
                 * frames in the packet have different sizes. */
                sbc_frames_in_payload = 0;
            }
        }

        if (sbc_frames_in_payload != 0)
        {
            frame_data->valid = TRUE;
            frame_data->nr_of_frames = sbc_frames_in_payload;
            /* frame_samples =  nrof_subbands * nrof_blocks. */
            frame_data->frame_samples = nrof_subbands * nrof_blocks;
        }
    }
    else
    {
        fault_diatribe(FAULT_RTP_SBC_SYNC_LOST, sbc_header[0]);
    }
}

void get_samples_in_packet(RTP_DECODE_OP_DATA *opx_data, RTP_FRAME_DECODE_DATA* frame_data)
{
    unsigned payload_size = obuffer_calc_amount_data_in_octets(opx_data->clone_op_buffer);
    frame_data->valid = FALSE;

    switch (opx_data->codec_type)
    {
        case SBC:
        {
            sbc_sample_count(opx_data, payload_size, frame_data);
            break;
        }

        case ATRAC:
        {
            break;
        }
        case MP3:
        {
            break;
        }
        case APTX:
        {
            frame_data->valid = TRUE;
            /* For aptX, the generated samples is the same as the number of encoded octets,
             * as long as the input number of octets is a multiple of 4.
             * This is assumed to always be the case here. */
            payload_size = payload_size >> 2;
            payload_size = payload_size << 2;
            if (payload_size != 0)
            {
                frame_data->frame_length = payload_size;
                frame_data->nr_of_frames = 1;
                frame_data->frame_samples = payload_size;
            }
            else
            {
                L4_DBG_MSG("RTP decode APTX classic: Not enough data to create a tag.");
                frame_data->nr_of_frames = 0;
            }
            break;
        }
        case APTXHD:
        {
            frame_data->valid = TRUE;
            /* For aptXHD, every 6 input octets generates 4 output samples. */
            frame_data->frame_samples = payload_size / 3;
            frame_data->frame_samples >>= 1;
            if (frame_data->frame_samples != 0)
            {
                frame_data->frame_length = frame_data->frame_samples * 6;
                frame_data->frame_samples <<= 2;
                frame_data->nr_of_frames = 1;
                /* If the size of the payload received is not a multiple of 6 the remaining
                 * octets will be part of the next payload. */
            }
            else
            {
                L4_DBG_MSG("RTP decode APTX hd: Not enough data to create a tag.");
                frame_data->nr_of_frames = 0;
            }
            break;
        }
        case AAC:
        {
            aac_sample_count(opx_data, payload_size, frame_data);
            break;
        }
        default:
        {
            /*mode not supported yet*/
            panic(PANIC_AUDIO_RTP_UNSUPPORTED_CODEC);
            break;
        }
    }
}

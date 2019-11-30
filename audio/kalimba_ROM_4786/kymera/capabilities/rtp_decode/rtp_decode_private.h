/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup rtp_decode
 * \file  rtp_decode_private.h
 * \ingroup capabilities
 *
 * Rtp decode operator private header file. <br>
 *
 */

#ifndef RTP_DECODE_PRIVATE_H
#define RTP_DECODE_PRIVATE_H
/*****************************************************************************
Include Files
*/
#include "capabilities.h"
#include "rtp_decode/rtp_decode.h"
#include "platform/pl_intrinsics.h"
#include "platform/pl_assert.h"
#include "rtp_decode_struct.h"
#include "op_msg_helpers.h"
#include "buffer/buffer_metadata.h"
#include "hal/hal_time.h"
#include "panic/panic.h"
#include "fault/fault.h"
#include "audio_log/audio_log.h"
#include "ttp_utilities.h"

/****************************************************************************
 * Private Constant Definitions
 */
/* this would be global somewhere */
#define RTP_DECODE_VERSION_LENGTH                       2

/** default buffer size for this operator */
#define RTP_DECODE_DEFAULT_BUFFER_SIZE                  512

/** default block size for this operator's terminals */
#define RTP_DECODE_DEFAULT_BLOCK_SIZE                   1

/** Active channel mask. */
#define CHANNEL_MASK(ID) (1<<(ID))

/****************************************************************************
 *  The RTP fixed header is 12 octets log. The RTP decode capability decodes the header
 *  in three parts. The table below shows how the parts are separated.
 *
 *
 *  |  V   |  P  |  X  |   C   |  M  | payload type | sequence |     timestamp      |       source      |
 *  |  2   |  1  |  1  |   4   |  1  |       7      |    16    |        32          |         32        |
 *  | bits | bit | bit |  bits | bit |      bits    |   bits   |       bits         |        bits       |
 *  |                                                          |                    |                   |
 *  |                  HEADER FIRST PART                       | HEADER SECOND PART | HEADER THIRD PART |
 *  |                                                          |  (TIMESTAMP PART)  |   (SOURCE PART)   |
 *  |                      4 octets                            |       4 octets     |      4 octets     |
 */

/**
 * The first part of the fixed RTP header contains the RTP version (V), padding field
 * which is set when the payload has padding part (P), extension header present field (x),
 * number of contributing sources (C), marker bit(M), payload type and sequence number.
 *
 * The length of the first part is 4 octets.
 */
#define RTP_FIXED_HEADER_FIRST_PART_LENGTH (4)

/**
 * The second part of the fixed RTP header contains timestamp
 *
 * The length of the second part is 4 octets.
 */
#define RTP_FIXED_HEADER_TIMESTAMP_PART_LENGTH (4)

/**
 * The third part of the fixed RTP header contains source (who is sending the packet).
 *
 * The length of the third part is 4 octets.
 */
#define RTP_FIXED_HEADER_SOURCE_PART_LENGTH (4)


/****************************************************************************
 * Constants from bc\main\dev\src\csr\stream\transform_rtp.c
 */

/*
 * RTP Header definitions
 */
#define RTP_MINIMUM_HEADER_SIZE         12
#define RTP0_VERSION_2                  0x80
#define RTP0_VERSION_MASK               0xc0
#define RTP0_PADDING                    0x20
#define RTP0_EXTENSION                  0x10
#define RTP0_CSRC_COUNT_MASK            0x0f
#define RTP1_MARKER                     0x80
#define RTP1_PAYLOAD_TYPE_MASK          0x7f
#define RTP1_PAYLOAD_TYPE_FIRST_DYNAMIC 96
#define RTP_SEQ_INVALID                 0

/*
 * SBC Header definitions
 */
#define SBC_MAX_FRAMES              15
#define SBC_PAYLOAD_HEADER_SIZE     1
#define SBC_PAYLOAD_FRAGMENT        0x80
#define SBC_PAYLOAD_START           0x40
#define SBC_PAYLOAD_LAST            0x20
#define SBC_HEADER_SIZE             4

/* Header size which enough to calculate the sample count in the packet. */
#define SBC_SAMPLE_COUNT_HEADER_SIZE   3

#define SBC_HEADER0_SYNC            0x9C
#define SBC_HEADER1_FREQUENCY_MASK  0xc0
#define SBC_HEADER1_FREQUENCY_SHIFT 6
#define SBC_HEADER1_BLOCKS_MASK     0x30
#define SBC_HEADER1_BLOCKS_SHIFT    4
#define SBC_HEADER1_BLOCKS(X)       (((X) & SBC_HEADER1_BLOCKS_MASK) >> SBC_HEADER1_BLOCKS_SHIFT)
#define SBC_HEADER1_CHANNEL_MASK    0x0c
#define SBC_HEADER1_CHANNEL_MONO    0x00
#define SBC_HEADER1_CHANNEL_DUAL    0x04
#define SBC_HEADER1_CHANNEL_STEREO  0x08
#define SBC_HEADER1_CHANNEL_JOINT   0x0c
#define SBC_HEADER1_ALLOCATION_SNR  0x02
#define SBC_HEADER1_SUBBANDS_8      0x01
#define SBC_HEADER1_SUBBANDS(X)     ((X) & SBC_HEADER1_SUBBANDS_8)
#define SBC_HEADER2_BITPOOL_MASK    0xff
#define SBC_HEADER2_BITPOOL(X)      ((X) & SBC_HEADER2_BITPOOL_MASK)

#define SBC_HEADER1_GET_NROF_BLOCKS(X)      (SBC_HEADER1_BLOCKS(X) * 4 + 4)
#define SBC_HEADER1_GET_NROF_SUBBANDS(X)    (SBC_HEADER1_SUBBANDS(X) * 4 + 4 )

/**
 * Returns the number of samples in the sbc frame providing the second word from the
 * SBC header.
 */
#define SBC_HEADER1_GET_SAMPLE_COUNT(X)     (SBC_HEADER1_GET_NROF_BLOCKS(X) * SBC_HEADER1_GET_NROF_SUBBANDS(X))

/**
 * Returns the SBC frame length.
 */
#define SBC_HEADER1_GET_SAMPLE_COUNT(X)     (SBC_HEADER1_GET_NROF_BLOCKS(X) * SBC_HEADER1_GET_NROF_SUBBANDS(X))




/*
 * MP3 Header definitions
 */
#define MP3_PAYLOAD_HEADER_SIZE     4
#define MP3_HEADER_SIZE             4
#define MP3_HEADER0_SYNC            0xff
#define MP3_HEADER1_SYNC_MASK       0xe0
#define MP3_HEADER1_VERSION_MASK    0x18
#define MP3_HEADER1_VERSION_1       0x18
#define MP3_HEADER1_VERSION_2       0x10
#define MP3_HEADER1_VERSION_2_5     0x00
#define MP3_HEADER1_LAYER_MASK      0x06
#define MP3_HEADER1_LAYER_3         0x02
#define MP3_HEADER1_PROTECTION_MASK 0x01
#define MP3_HEADER2_BITRATE_MASK    0xf0
#define MP3_HEADER2_BITRATE_SHIFT   4
#define MP3_HEADER2_FREQUENCY_MASK  0x0c
#define MP3_HEADER2_FREQUENCY_SHIFT 2
#define MP3_HEADER2_PADDING_MASK    0x02
#define MP3_HEADER3_CHANNEL_MASK    0xc0
#define MP3_HEADER3_CHANNEL_MONO    0xc0
#define MP3_HEADER3_CHANNEL_DUAL    0x80
#define MP3_HEADER3_CHANNEL_STEREO  0x00
#define MP3_HEADER3_CHANNEL_JOINT   0x40
#define MP3_MPEG1_SAMPLESPERFRAME   1152
#define MP3_MPEG2_SAMPLESPERFRAME   576


/*
 * APTX Header definition
 */
/* Some says that the APTX payload header size is 0*/
#define APTX_PAYLOAD_HEADER_SIZE     0

/*
 * APTXHD Header definition
 */
/* Some says that the APTXHD payload header size is 0*/
#define APTXHD_PAYLOAD_HEADER_SIZE     0

/*
 * AAC Header definition
 */
/* There are loads and loads of AAC headers, but seemingly no spurious one 
 * from encapsulation in the RTP packet.
 */
#define AAC_PAYLOAD_HEADER_SIZE     0


/* MP3 bitrates table - shifted 3 bits to fit into uint8 */
#ifdef INSTALL_MP3
static const uint8 mp3_bitrate[2][16] = {
    { 0, 32>>3, 40>>3, 48>>3, 56>>3, 64>>3, 80>>3, 96>>3, 112>>3, 128>>3, 160>>3, 192>>3, 224>>3, 256>>3, 320>>3, 0 }, /* MPEG-1 */
    { 0, 8>>3, 16>>3, 24>>3, 32>>3, 40>>3, 48>>3, 56>>3, 64>>3, 80>>3, 96>>3, 112>>3, 128>>3, 144>>3, 160>>3, 0 }      /* MPEG-2, MPEG-2.5 */
};
#endif

#define ATRAC_PAYLOAD_HEADER_SIZE 1

#define SCMS_HEADER_SIZE 1

/*
 * We need to round frame length up to the nearest byte,
 * so we add the number below before dividing by 8.
 */
#define FRAME_LENGTH_ROUNDING   7

#define STATS_PACKET_OFFSET 0
#define STATS_LATE_OFFSET   1
#define STATS_LOST_OFFSET   2



/*****************************************************************************
 * Private Function Definitions
 */

/**
 * Calculates the samples in the packet. If the codec header cannot be decoded the
 * sample count provided from the RTP header is returned.
 *
 * \param opx_data Pointer to the RTP operator data.
 */
void get_samples_in_packet(RTP_DECODE_OP_DATA *opx_data, RTP_FRAME_DECODE_DATA* frame_data);

/**
 * Returns the AAC coded given the operator id.
 *
 * \param ext_op_id Operator id.
 * \returns AAC codec needed to decode and AAC frame.
 */
void* get_AAC_codec(unsigned ext_op_id);

/* Message handlers */
extern bool rtp_decode_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool rtp_decode_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool rtp_decode_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool rtp_decode_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool rtp_decode_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool rtp_decode_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool rtp_decode_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool rtp_decode_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool rtp_decode_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
extern bool rtp_decode_opmsg_set_working_mode(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_codec_type(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_content_protection(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_AAC_decoder(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_AAC_utility(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_max_packet_length(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_play_pointer(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_ttp_latency(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_latency_limits(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_ttp_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool rtp_decode_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Data processing function */
extern void rtp_decode_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

#endif /* RTP_DECODE_PRIVATE_H */

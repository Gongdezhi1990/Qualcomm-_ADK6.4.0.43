/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup sco_fw Sco
 * \file  sco_fw_c.h
 *
 * SCO fw header
 *
 */

#ifndef SCO_FW_C_H
#define SCO_FW_C_H

/****************************************************************************
Include Files
*/
#include "sco_struct.h" 
#ifdef SCO_DEBUG
#include "audio_log/audio_log.h"
#endif /* SCO_DEBUG */

/****************************************************************************
Public Type Declarations
*/
typedef enum{
    SCO_NB = 0,
    WBS
}CONNECTION_TYPE;

/****************************************************************************
Public Constant Declarations
*/

/****************************************************************************
Public Macro Declarations
*/
/**
 * Sco debug messages.
 */
#if defined(SCO_DEBUG) && defined(SCO_DEBUG_PRINTF)
#include <stdio.h>         
#define SCO_DBG_MSG(x)                 printf(x);printf("\n")
#define SCO_DBG_MSG1(x, a)             printf(x, a);printf("\n")
#define SCO_DBG_MSG2(x, a, b)          printf(x, a, b);printf("\n")
#define SCO_DBG_MSG3(x, a, b, c)       printf(x, a, b, c);printf("\n")
#define SCO_DBG_MSG4(x, a, b, c, d)    printf(x, a, b, c, d);printf("\n")
#define SCO_DBG_MSG5(x, a, b, c, d, e) printf(x, a, b, c, d, e);printf("\n")
#elif defined(SCO_DEBUG) && defined(SCO_DEBUG_LOG)
#define SCO_DBG_MSG(x)                 L0_DBG_MSG(x)
#define SCO_DBG_MSG1(x, a)             L0_DBG_MSG1(x, a)
#define SCO_DBG_MSG2(x, a, b)          L0_DBG_MSG2(x, a, b)
#define SCO_DBG_MSG3(x, a, b, c)       L0_DBG_MSG3(x, a, b, c)
#define SCO_DBG_MSG4(x, a, b, c, d)    L0_DBG_MSG4(x, a, b, c, d)
#define SCO_DBG_MSG5(x, a, b, c, d, e) L0_DBG_MSG5(x, a, b, c, d, e)
#else  /* SCO_DEBUG && SCO_DEBUG_LOG */
#define SCO_DBG_MSG(x)                 ((void)0)
#define SCO_DBG_MSG1(x, a)             ((void)0)
#define SCO_DBG_MSG2(x, a, b)          ((void)0)
#define SCO_DBG_MSG3(x, a, b, c)       ((void)0)
#define SCO_DBG_MSG4(x, a, b, c, d)    ((void)0)
#define SCO_DBG_MSG5(x, a, b, c, d, e) ((void)0)
#endif /* SCO_DEBUG */

/****************************************************************************
Public Variable Declarations
*/

/**
 * Convert a quantity in octets (as seen by the BTSS) to samples/words
 * (as seen by the audio subsystem), assuming the buffer mode is 16-bit
 * unpacked. If the length in octets is uneven the sample number will
 * be rounded up.
 */
#define CONVERT_OCTETS_TO_SAMPLES(x)  (((x) + 1)  / 2)

/**
 * Convert a quantity in samples back to octets.
 */
#define CONVERT_SAMPLES_TO_OCTETS(x) ((x) * 2)

/****************************************************************************
Public Constant Declarations
*/
/**
 * Constant string used for displaying a packet
 */
#define PACKET_STRING \
" ==> Packet    time_stamp: %5d; length: %5d; status: %5d;"

/**
 * Constant string used for displaying the sco state
 */
#define SCO_STATE_STRING \
"SCO_rcv_state  exp_time_stamp: %5d; exp_pkts: %5d; sco_pkt_size: %5d; t_esco: %5d;"

/****************************************************************************
Public Macro Declarations
*/

/*
These values apply transformations to the sco metadata. The format of the data
should be the same on all platforms if the endpoints are doing there job
correctly it may be useful during development of a new platform to modify
these values.
*/

/* Byte swap */
#define SCO_NB_METADATA_BYTE_SWAP(x)    ((x<<8) & 0xFF00) |((x>>8) & 0x00FF)

/* Right shift 8 */
#define SCO_NB_METADATA_SHIFT(x)        ((x)>> (DAWTH- 16))

/* Value used for masking the first 16 bit.*/
#define MASK_16_BIT_VAL                 (0xFFFF)

/* Mask the last two octets */
#define MASK_16_BIT(x)                  ((x) & MASK_16_BIT_VAL)

/**
 * Transform macro used in case of a sco nb.
 */
#ifdef CVSD_CODEC_SOFTWARE
#define SCO_NB_METADATA_TRANSFORM(x) MASK_16_BIT(x)
#else
#define SCO_NB_METADATA_TRANSFORM(x) MASK_16_BIT(SCO_NB_METADATA_SHIFT(x))
#endif

/**
 * Transform macro used for wbs.
 */
#define WBS_METADATA_TRANSFORM(x)    SCO_NB_METADATA_BYTE_SWAP(x)

/****************************************************************************
Public Function Declarations
*/
/* Metadata related functions. */
extern bool enough_data_to_run(SCO_COMMON_RCV_OP_DATA* sco_data, unsigned data_size);
extern bool enough_space_to_run(SCO_COMMON_RCV_OP_DATA* sco_data, unsigned output_packet_size);
extern stream_sco_metadata_status read_packet_metadata(SCO_COMMON_RCV_OP_DATA* sco_data, stream_sco_metadata *in_packet, CONNECTION_TYPE type);
extern stream_sco_metadata_status analyse_sco_metadata(SCO_COMMON_RCV_OP_DATA* sco_data, stream_sco_metadata *in_packet);

/* Common packet handling functions. */
extern unsigned fake_packet(SCO_COMMON_RCV_OP_DATA* sco_data, unsigned packet_size, CONNECTION_TYPE type);
extern void discard_packet(SCO_COMMON_RCV_OP_DATA* sco_data, stream_sco_metadata* discard_packet);

/* Sco state controlling functions. */
extern void sco_fw_update_expected_timestamp(SCO_COMMON_RCV_OP_DATA* sco_data);
extern void sco_fw_check_bad_kick_threshold(SCO_COMMON_RCV_OP_DATA* sco_data);
extern unsigned sco_rcv_get_packet_size(SCO_COMMON_RCV_OP_DATA* sco_data);
extern unsigned sco_rcv_get_output_size_words(SCO_COMMON_RCV_OP_DATA* sco_data);


#ifdef SCO_RX_OP_GENERATE_METADATA
/**
 * \brief transport metadata to output PCM buffer
 * \param sco_data - Pointer to the common SCO rcv operator data
 * \param input_processed - amount of input processed (in words)
 * \param output_generated - amount of output generated (in samples)
 * \param type - connection type (SCO_NB or WBS) only used to determine sample rate
 */
extern void sco_rcv_transport_metadata(SCO_COMMON_RCV_OP_DATA* sco_data,
                                  unsigned input_processed,
                                  unsigned output_generated,
                                  CONNECTION_TYPE type);
#endif /* SCO_RX_OP_GENERATE_METADATA */
/**
 * sco_rcv_flush_input_buffer
 * \brief clearing sco input buffer with metadata
 * \param Pointer to the sco common data.
 */
extern void sco_rcv_flush_input_buffer(SCO_COMMON_RCV_OP_DATA* sco_data);

/****************************************************************************
Public Function Definition
*/

/**
 * \brief Returns the stream sco packet payload length in words.
 *
 * \param  sco_data  Pointer to the stream sco packet.
 */
static inline unsigned get_packet_payload_length(stream_sco_metadata* sco_metadata)
{
    return CONVERT_OCTETS_TO_SAMPLES(sco_metadata->packet_length);
}

/**
 * \brief If SCO_DEBUG is enabled this function prints out the sco rcv state.
 *
 * \param  sco_data  Pointer to the sco common data.
 */
static inline void print_SCO_state(SCO_COMMON_RCV_OP_DATA* sco_data)
{
    SCO_DBG_MSG4(SCO_STATE_STRING, sco_data->sco_rcv_parameters.expected_time_stamp,
            sco_data->sco_rcv_parameters.exp_pkts,
            sco_data->sco_rcv_parameters.sco_pkt_size,
            sco_data->sco_rcv_parameters.t_esco);

}

/**
 * \brief If SCO_DEBUG is enabled this function prints out the packet read from the
 *        input buffer.
 *
 * \param  sco_packet  Pointer to the stream sco packet (must be different than NULL).
 */
static inline void print_sco_metadata(stream_sco_metadata *sco_metadata)
{
    SCO_DBG_MSG3(PACKET_STRING, sco_metadata->time_stamp,
            sco_metadata->packet_length,
            sco_metadata->status);

}

#endif /* SCO_FW_C_H */

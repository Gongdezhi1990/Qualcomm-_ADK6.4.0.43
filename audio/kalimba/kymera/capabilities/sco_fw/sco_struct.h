/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup sco
 * \file  sco_struct.h
 * \ingroup capabilities
 *
 * SCO operators public header file.
 *
 */

#ifndef SCO_OP_H
#define SCO_OP_H

#include "buffer/cbuffer_c.h"
#include "audio_fadeout.h"
#include "op_msg_utilities.h"
#ifdef INSTALL_PLC100
#include "plc100_c.h"
#endif
#ifdef CVSD_CODEC_SOFTWARE
#include "cvsd.h"
#endif
#include "stream/stream.h"
#include "stream/stream_for_sco_operators.h"

/** Minimum block size in words used in SCO */
#define SCO_MIN_BLOCK_SIZE                            15
#define SCO_DEFAULT_BLOCK_SIZE                        30
/** Default buffer size TODO in words that of the sco side buffer */
#define SCO_DEFAULT_SCO_BUFFER_SIZE                       256

/** Metadata header size in 16 bit words. Because the metadata in the input buffer is in 16 bit unpacked words we can consider them as words. */
#define METADATA_HEADER_SIZE                         5

/** The terminal ID of the input terminal */
#define INPUT_TERMINAL_ID (0 | TERMINAL_SINK_MASK)
/** The terminal ID of the output terminal */
#define OUTPUT_TERMINAL_ID (0)


/** Default buffer size, minimum and default block size for sco receive  TODO in words. */
#define SCO_RCV_INPUT_BUFFER_SIZE                  (SCO_DEFAULT_SCO_BUFFER_SIZE)
#define SCO_RCV_OUTPUT_BUFFER_SIZE                 (SCO_DEFAULT_SCO_BUFFER_SIZE)
#define SCO_RCV_MIN_BLOCK_SIZE                       (SCO_MIN_BLOCK_SIZE)
#define SCO_RCV_DEFAULT_BLOCK_SIZE                   (SCO_DEFAULT_BLOCK_SIZE)

/* Enabling metadata generation (transport from input) */
#ifdef INSTALL_SCO_RX_TOA_METADATA_GENERATE
#if defined(INSTALL_TTP) && defined(INSTALL_METADATA)
#define SCO_RX_OP_GENERATE_METADATA
#endif /* defined(INSTALL_TTP) && defined(INSTALL_METADATA) */
#endif /* #ifdef INSTALL_SCO_RX_TOA_METADATA_GENERATE */

/** Generic storage for the input and output buffers. */
typedef struct SCO_TERMINAL_BUFFERS
{
    /** The buffer at the input terminal */
    tCbuffer *ip_buffer;

    /** The buffer at the output terminal */
    tCbuffer *op_buffer;
} SCO_TERMINAL_BUFFERS;

/** Common parameters for SCO RCV operators - any calls to SCO FW need to work
 *  with same indexes to get to these parameters located inside various
 *  operator data structures.
 */
typedef struct SCO_RCV_PARAMS
{
    /** SCO packet and timing-related parameters */
    unsigned sco_pkt_size;
    unsigned t_esco;
    unsigned out_of_time_pkt_cnt;
    unsigned exp_pkts;
    unsigned expected_time_stamp;

    /** Debug data fields */
    unsigned frame_count;
    unsigned frame_error_count;

    /** Force PLC off - NOTE this flag is currently the only capability-specific
     *  parameter that is controllable from OBPM. If any other such parameters are
     *  added, they must be contiguous after this field - unless then all of them
     *  are moved into a dedicated structure, which is another option.
     */
#ifdef INSTALL_PLC100
    unsigned force_plc_off;

    /** PLC structure pointer if PLC installed */
    PLC100_STRUC* plc100_struc;
#endif /* INSTALL_PLC100 */

    /** Some counters and variables used for debug mainly, not conditioned for only
     *  debug builds.
     */
    unsigned md_num_kicks;
    unsigned md_process_errors;
    unsigned md_pkt_size_changed;
    unsigned md_late_pkts;
    unsigned md_early_pkts;
    unsigned md_out_of_time_reset;
    unsigned num_bad_kicks;
    unsigned num_good_pkts_per_kick;
    unsigned packet_log[4];

    /* output buffer size */
    unsigned output_buffer_size;

#ifdef INSTALL_PLC100
    unsigned md_pkt_faked;
#endif /* INSTALL_PLC100 */

#ifdef CVSD_CODEC_SOFTWARE
	sCvsdState_t cvsd_struct;
	int* ptScratch;				// pointer to scratch memory
#endif

#ifdef SCO_RX_OP_GENERATE_METADATA
    /* if enabled TTP will be generated instead of ToA */
    bool generate_timestamp;
    unsigned timestamp_latency;
#endif
} SCO_RCV_PARAMS;

#ifdef SCO_RX_OP_GENERATE_METADATA
typedef struct SCO_COMMON_RCV_METADATA_HIST
{
   /* store the last tags written to
    * help when PLC fakes a packet */
   unsigned nrof_samples;
   unsigned timestamp;
   int      sp_adjust;
   bool     valid;
   int      time_offset;
}SCO_COMMON_RCV_METADATA_HIST;
#endif /* SCO_RX_OP_GENERATE_METADATA */

/** Common part of SCO RCV operator data - WB SCO uses extra information.
 *  In order for common functionality across NB and WB SCO to work safely,
 *  care must be taken to use this common definition when making pointer casts to refer to below
 *  part of the WB SCO operator(s).
 */
typedef struct SCO_COMMON_RCV_OP_DATA
{
    /** Terminal buffers */
    SCO_TERMINAL_BUFFERS buffers;

    /** Fade-out parameters */
    FADEOUT_PARAMS fadeout_parameters;

    /** SCO parameters that are common between different sco receive capabilities */
    SCO_RCV_PARAMS sco_rcv_parameters;

    CPS_PARAM_DEF  parms_def;

#ifdef SCO_RX_OP_GENERATE_METADATA
    SCO_COMMON_RCV_METADATA_HIST last_tag;
    /* connected endpoint */
    ENDPOINT *sco_source_ep;
#endif

} SCO_COMMON_RCV_OP_DATA;



/**************************************************************************************
 * Stream SCO metadata related types.
 *
 */
/**
 * For cases 2 and 3, the packet_length value may still be non-zero, in which case the
 * for contents are junk; the packet length may even legitimately exceed the length of
 * the buffer.
 */
typedef enum{
    OK = 0,
    CRC_ERROR = 1,
    NOTHING_RECEIVED =2,
    NEVER_SCHEDULED = 3     /* this corresponds to the scos_time_missed debug variable
                             * in the BlueCore firmware. */
}metadata_status;

/**
 * The sco stream metadata consist of generic header and a sco specific header.
 */
typedef struct sco_metadata
{

    /**
     * Generic sco stream metadata header.
     */
    /**
     * This is fixed resynchronisation word: 0x5C5C. This word is not presented
     * in this structure.
     */

    /**
     * The length of the metadata, in words. This includes the generic header, so the
     * minimum value is 4. For SCO nb and WBS the metadata length is 5.
     */
    unsigned metadata_length;
    /**
     *  The length of the associated data packet, in octets. If this is odd, then an
     *  octet of padding will be added to the end of the packet so that the transferred
     *  data is an integral number of words.
     */
    unsigned packet_length;
    /**
     * Sco specific metadata header
     */
    /**
     * Status represents the state of the sco packet (see enumeration before).
     */
    metadata_status status;
    /**
     * The least 16 bits of the Bluetooth clock for the master's reserved slot.
     */
    unsigned time_stamp;
} stream_sco_metadata;


/**
 * During the process of a sco packet first the sco metadata is read.
 * After reading the sco metadata it is analysed and the packet is processed. These
 * stages are represented by the sco packet status.
 */
typedef enum
{
    /**
     * Initial state. Packet is not read yet.
     */
    SCO_METADATA_NOT_READ,
    /**
     * Sco metadata is read and is ready for further analyses.
     */
    SCO_METADATA_READY,
    /**
     * Sco metadata was analysed and the SCO packet is late.
     */
    SCO_METADATA_LATE,
    /**
     * Sco metadata was analysed and the SCO packet is on time.
     */
    SCO_METADATA_ON_TIME,
    /**
     * Sco metadata was analysed and the SCO packet is early.
     */
    SCO_METADATA_EARLY,
    /**
     * The sco metadata was read and is invalid.
     */
    SCO_METADATA_INVALID

} stream_sco_metadata_status;


#endif /* SCO_OP_H */

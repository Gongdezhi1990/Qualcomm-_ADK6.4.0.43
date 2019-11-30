/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup wbs
 * \file  wbs_struct.h
 * \ingroup capabilities
 *
 * Wide Band Speech type header. <br>
 *
 */

#ifndef WBS_TYPES_H
#define WBS_TYPES_H
/*****************************************************************************
Include Files
*/
#include "sco_struct.h"
#include "sbc_c.h"


/** capability-specific extra operator data for WBS ENC */
typedef struct WBS_ENC_OP_DATA
{
    /** Fade-out parameters */
    FADEOUT_PARAMS fadeout_parameters;

    /** Terminal buffers */
    SCO_TERMINAL_BUFFERS buffers;

    /* This is where additional WBS-specific information can start */

    /* The codec deta structure - in the old days, it is what value at $caps.sco.CODEC_DATA_STRUC_PTR_FIELD used to point to */
    sbc_codec* codec_data;

    /** Amount of data to process each time */
    unsigned frame_size;
} WBS_ENC_OP_DATA;


/** capability-specific extra operator data for WBS DEC */
typedef struct WBS_DEC_OP_DATA
{
    /** Common part of SCO Rcv operators' extra data - it packages the buffer size,
     *  fadeout parameters and SCO parameters so that common code can safely reference same fields.
     */
    SCO_COMMON_RCV_OP_DATA sco_rcv_op_data;

    /** The codec data structure - in the old days, it is what value at $caps.sco.CODEC_DATA_STRUC_PTR_FIELD used to point to */
    sbc_codec* codec_data;

    /** First valid packet flag */
    unsigned received_first_valid_pkt;

    /** Address of next packet value */
    unsigned* next_packet_value;

    /**  num of times no data at kick and we needed to fake, ie, not priming */
    unsigned md_bad_kick_attmpt_fake;
    /**  num of times no data at kick and we had to fake, ie, validate returned NZ */
    unsigned md_bad_kick_faked;
    /** number of good packets found during one kick period */
    unsigned good_pkts_per_kick;

#ifdef SCO_DEBUG
    /** decoder statistics */
    unsigned wbs_dec_dbg_stats_enable; /* flag for debug stats enabling */

    unsigned wbs_dec_stats_count;    /* Seq number of stats message */
    unsigned wbs_dec_stats_last_ts;  /* last time stamp */
    unsigned wbs_dec_num_kicks;   /* Num of kicks in period */
    unsigned wbs_dec_num_pkts_searched; /* num of times metadata read was called in period */
    unsigned wbs_dec_metadata_found;    /* num of times metadata read returned success in period */
    unsigned wbs_dec_error_pkts;  /* num of packets with error status in metadata hdr in period */
    unsigned wbs_dec_validate_ret_nz;   /* num of times validate returned non zero values: zero means no space in op buffer */
    unsigned wbs_dec_validate_ret_good; /* num of times validate returned values >= 120. */
#endif /* SCO_DEBUG */

    unsigned wbs_dec_no_output;   /*  num of times decoder process returned no_output status code */
    unsigned wbs_dec_fake_pkt;   /*  num of times decoder process returned no_output code and PLC output a fake packet */
    unsigned wbs_dec_good_output; /*  num of times decoder process returned good output status code */
    unsigned wbs_dec_bad_output;  /*  num of times decoder process returned bad output status code */

    /** flags for touched terminals, bit0 is input, bit12 is output terminal touched flag */
    unsigned terminals_touched;
    /** number of samples resulting from WBS validate - this will match, if PLC is installed, the PLC packet length eventually */
    unsigned wbs_dec_output_samples;

} WBS_DEC_OP_DATA;

#endif /* WBS_TYPES_H */

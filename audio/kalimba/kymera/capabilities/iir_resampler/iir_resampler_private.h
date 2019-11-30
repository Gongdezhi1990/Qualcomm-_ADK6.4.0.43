/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#ifndef _IIR_RESAMPLER_PRIVATE_H_
#define _IIR_RESAMPLER_PRIVATE_H_

/**
 * Include Files
 */
#include "iir_resamplerv2_common.h"
#include "op_channel_list.h"
#include "opmgr/opmgr_for_ops.h"

/****************************************************************************
Private Constant Declarations
*/

#define IIR_RESAMPLER_MAX_CHANNELS                      (8)
/* config message bit field values bits 0-15, bits 16-23 are used for internal operator state */
#define IIR_RESAMPLER_CONFIG_DBL_PRECISION              (0x0001)
#define IIR_RESAMPLER_CONFIG_LOW_MIPS                   (0x0002)
/** Flag indicating if the operator is running in stream_based mode or not. */
#define IIR_RESAMPLER_CONFIG_STREAM_BASED               (0x010000)
#define IIR_RESAMPLER_EX_CONFIG_MASK                    (IIR_RESAMPLER_CONFIG_DBL_PRECISION | IIR_RESAMPLER_CONFIG_LOW_MIPS)
#define IIR_RESAMPLER_INT_CONFIG_MASK                   (~IIR_RESAMPLER_EX_CONFIG_MASK)

#define IIR_RESAMPLER_NO_SAMPLE_RATE                    0

#define IIR_RESAMPLER_BUFFER_SIZE                       256

/* sample (not frame) based processing */
#define IIR_RESAMPLER_DEFAULT_BLOCK_SIZE                1

/**
 * TTP debug messages and macros.
 */
#ifdef IIR_RESAMPLER_DEBUG

#define IIR_DBG_MSG(x)                 L0_DBG_MSG(x)
#define IIR_DBG_MSG1(x, a)             L0_DBG_MSG1(x, a)
#define IIR_DBG_MSG2(x, a, b)          L0_DBG_MSG2(x, a, b)
#define IIR_DBG_MSG3(x, a, b, c)       L0_DBG_MSG3(x, a, b, c)
#define IIR_DBG_MSG4(x, a, b, c, d)    L0_DBG_MSG4(x, a, b, c, d)
#define IIR_DBG_MSG5(x, a, b, c, d, e) L0_DBG_MSG5(x, a, b, c, d, e)

#define IIR_DEBUG_MACRO(x) (x)

#else  /* IIR_METADATA_DEBUG */

#define IIR_DBG_MSG(x)                 ((void)0)
#define IIR_DBG_MSG1(x, a)             ((void)0)
#define IIR_DBG_MSG2(x, a, b)          ((void)0)
#define IIR_DBG_MSG3(x, a, b, c)       ((void)0)
#define IIR_DBG_MSG4(x, a, b, c, d)    ((void)0)
#define IIR_DBG_MSG5(x, a, b, c, d, e) ((void)0)

#define IIR_DEBUG_MACRO(x) ((void)0)

#endif /* IIR_METADATA_DEBUG */

/**
 * IIR Resampler internal data structure
 */
typedef struct iir_resampler_internal
{
    /** iir_resamplerv2 common parameter data */
    iir_resamplerv2_common    common;

    /**
     * pointer to working data blocks
     * dynamically allocated at end of structure (following channel array)
     */
    unsigned* working;

    /** 
     * channel-specific parameter data array
     * variable length depending on number of active channels
     */
    iir_resamplerv2_channel   channel[];

} iir_resampler_internal;

/**
 * Type of tags supported by the operator.
 */
typedef enum
{
    UNSUPPORTED_TAG = 0,
    TIMESTAMPED_TAG,
    TIME_OF_ARRIVAL_TAG,
    VOID_TAG,
    EMPTY_TAG
} tag_types;

/**
 *  Structure to store information from a tag.
 */
typedef struct
{
    /* Type of the last tag. */
    tag_types type;

    /** Timestamp or time of arrival from the last metadata tag processed */
    unsigned timestamp;

    /** Sample period adjustment value from the last metadata tag */
    unsigned spa;

    /** Samples read since the last metadata tag */
    unsigned samples_after;

    /* Error offset ID from last tag */
    unsigned err_offset_id;
} tag_data;

/**
 * Capability-specific extra operator data
 */
typedef struct IIR_RESAMPLER_OP_DATA
{
    /** input sample rate in Hz */
    unsigned in_rate;
    
    /** output sample rate in Hz */
    unsigned out_rate;
    
    /** Conversion fraction used to relate number of input samples to output samples
     * This is encoded as a fraction with components representing integer and 
     * fractional parts */
    unsigned conv_fact;

    /** linked-list containing I/O channel data */
    OP_CHANNEL_LIST* channel_list;
    
    /** The list of channels to consider during amount_to_use calculation.
      * This is used when the capability operates in STREAM_BASED mode so
      * only a single channel is considered in amount_to_use calculations. */
    OP_CHANNEL_LIST* amt_to_use_channel_list;
    
    /** number of active channels */
    unsigned num_channels;
    
    /** iir_resamplerv2 internal data pointer */
    iir_resampler_internal* iir_resamplerv2;
    
    /** iir_resamplerv2 shared filter memory pointer */
    void* lpconfig;
    
    /** amount of scratch memory that has been reserved in addrs */
    unsigned scratch_reserved;
    
    /** size of intermediate data buffer in words */
    unsigned temp_buffer_size;
    
    /** touched terminals mask for active channels */
    unsigned touched_mask;
    
    /** iir_resampler configuration flags */
    unsigned config;

#ifdef INSTALL_METADATA
    /** The input buffer with metadata to transport from */
    tCbuffer *metadata_ip_buffer;

    /** The output buffer with metadata to transport to */
    tCbuffer *metadata_op_buffer;

    /** The number of octets delay that the processing introduces */
    unsigned metadata_delay;

    /* Store information about the last tag. */
    tag_data last_tag;

#ifdef IIR_METADATA_DEBUG
    unsigned output_buff_before_index;
    unsigned output_buff_after_index;

    unsigned samples_to_tag;
    unsigned calculated_produced;
    unsigned input_buff_before_index;
    unsigned input_buff_after_index;
#endif
#endif /* INSTALL_METADATA */
} IIR_RESAMPLER_OP_DATA;

/****************************************************************************
Private Function Definitions
*/
/* opcmd handlers */
extern bool iir_resampler_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool iir_resampler_stop(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool iir_resampler_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool iir_resampler_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool iir_resampler_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool iir_resampler_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool iir_resampler_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool iir_resampler_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool iir_resampler_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* opmsg handlers */
extern bool iir_resampler_opmsg_set_sample_rates(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool iir_resampler_opmsg_set_conversion_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool iir_resampler_opmsg_set_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool iir_resampler_data_stream_based(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#ifdef INSTALL_METADATA
extern bool iir_resampler_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif

// iir resampler obpm support
extern bool iir_resampler_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool iir_resampler_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool iir_resampler_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool iir_resampler_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool iir_resampler_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);


/* process data function */
extern void iir_resampler_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/***************************************************************************
Internal Function Declarations
*/
extern bool set_rates_iir_resampler_internal(OPERATOR_DATA *op_data, unsigned in_rate, unsigned out_rate);
extern bool init_iir_resampler_internal(IIR_RESAMPLER_OP_DATA* op_extra_data);
extern void free_iir_resampler_internal(IIR_RESAMPLER_OP_DATA* op_extra_data);
extern bool iir_resampler_stop_reset(OPERATOR_DATA *op_data,void **response_data);

#endif  // _IIR_RESAMPLER_PRIVATE_H_

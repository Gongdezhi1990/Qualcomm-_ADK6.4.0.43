/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup mixer
 * \file  mixer_private.h
 * \ingroup capabilities
 *
 * Mixer operator private header file. <br>
 *
 */

#ifndef _MIXER_PRIVATE_H_
#define _MIXER_PRIVATE_H_

/*****************************************************************************
Include Files
*/
#include "mixer.h"
#include "mixer_struct.h"
#include "platform/pl_intrinsics.h"
#include "platform/pl_fractional.h"
#include "common_conversions.h"
#include "op_msg_helpers.h"

#define CHANNEL_MASK(ID) (1<<(ID))
/****************************************************************************
Private Constant Definitions
*/
/** buffer sizes for this operator */
#define GEN_MIXER_DEFAULT_BUFFER_SIZE                  128
#define GEN_MIXER_LARGE_BUFFER_SIZE                    256
/** default block size for this operator's terminals */
#define GEN_MIXER_DEFAULT_BLOCK_SIZE                   1

/*****************************************************************************
Private Function Definitions
*/

extern bool mixer_start(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
extern bool mixer_destroy(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
extern bool mixer_create(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
extern bool mixer_connect(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
extern bool mixer_buffer_details(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);
extern bool mixer_disconnect(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);
extern bool mixer_get_sched_info(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);
                                    
extern void mixer_deallocate_channels(GEN_MIXER_OP_DATA *mixer_data);
extern bool mixer_allocate_channels(GEN_MIXER_OP_DATA *mixer_data,unsigned num_sinks,unsigned num_sources,unsigned num_streams);
extern GEN_MIXER_SOURCE_INFO* mixer_allocate_source(GEN_MIXER_OP_DATA *mixer_data,unsigned src_idx,unsigned sink_mask,bool bInitGains);
extern GEN_MIXER_MIX_INFO *setup_mixes(GEN_MIXER_SOURCE_INFO *src_ptr,unsigned sink_mask,GEN_MIXER_SINK_INFO *sink_pptr,GEN_MIXER_OP_DATA *mixer_data);
extern void mixer_update_processing(OPERATOR_DATA *op_data);
extern void setup_mixers(GEN_MIXER_OP_DATA *mixer_data);
extern void setup_mixers_gain_change(GEN_MIXER_OP_DATA *mixer_data);

#ifdef MIXER_SUPPORTS_STALLS
static bool mixer_link_streams(GEN_MIXER_OP_DATA *mixer_data);
#endif

#ifdef INSTALL_METADATA
extern void set_metadata(GEN_MIXER_OP_DATA *mixer_data,unsigned term_id,tCbuffer *buffer);
extern void clr_metadata(GEN_MIXER_OP_DATA *mixer_data,unsigned term_id);
extern tCbuffer *get_metadata(GEN_MIXER_OP_DATA *mixer_data,unsigned term_id);
extern void handle_metadata(GEN_MIXER_OP_DATA *mixer_data, unsigned proc_amount);
#endif

/* ASM Functions */
extern void gen_mixer_process_channels(GEN_MIXER_OP_DATA *mixer_data,unsigned num_samples);
extern void gen_mixer_set_gain(GEN_MIXER_OP_DATA *mixer_data,GEN_MIXER_GAIN_DEF *gain_def,unsigned inv_transition);


/****************************************************************************
Public Function Declarations
*/
extern void mixer_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS*);

extern bool mixer_set_stream_gains(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_set_stream_channels(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_set_ramp_num_samples(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_set_primary_stream(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_set_sample_rate(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_set_channel_gain(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool mixer_set_stream_channels(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

#ifdef INSTALL_METADATA
extern bool mixer_set_metadata_stream(OPERATOR_DATA *op_data, void *message_data,
                                       unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif /* INSTALL_METADATA */

extern bool channel_mixer_set_stream_parameters(OPERATOR_DATA *op_data,
        void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD  **response_data);


/* mixer obpm support */
extern bool mixer_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool mixer_opmsg_get_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern void mixer_get_status(OPERATOR_DATA *op_data);

#ifdef MIXER_SUPPORTS_STALLS
int  gen_mixer_compute_transfer(GEN_MIXER_OP_DATA *mixer_data,unsigned *bProcess_all_input);
extern unsigned gen_mixer_samples_to_time(unsigned samples,unsigned inv_sample_rate);
#endif

#endif /* _MIXER_PRIVATE_H_ */

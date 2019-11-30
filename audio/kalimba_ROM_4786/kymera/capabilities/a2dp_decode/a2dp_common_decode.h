/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
*    \file a2dp_common_decode.h
*
*    A2DP common header file. Contains common A2DP function headers.
*/

/****************************************************************************
Include Files
*/
#include "a2dp_common_decode_struct.h"
#include "opmgr/opmgr_for_ops.h"
#include "audio_proc/sra_c.h" /* All decoders have to interact with the SRA algorithm */
#include "mem_utils/scratch_memory.h"

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Public Constant Declarations
*/

/****************************************************************************
Public Constant Definitions
*/

extern const opmsg_handler_lookup_table_entry a2dp_decode_opmsg_handler_table[];

/****************************************************************************
Public Function Declarations
*/
/* Message handlers */
extern bool a2dp_decode_connect(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
extern bool a2dp_decode_connect_core(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data , unsigned out_bufsize);
extern bool a2dp_decode_disconnect(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);
extern bool a2dp_decode_start(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);
extern bool a2dp_decode_buffer_details(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);
extern bool a2dp_decode_buffer_details_core(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data,
									unsigned inp_bufsize, unsigned out_bufsize);
extern bool a2dp_decode_get_data_format(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
extern bool a2dp_decode_get_sched_info(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);

extern void a2dp_decoder_decode(DECODER *stream_decode_obj, void (*decode_frame)(void),
                                int stream_mode, A2DP_HEADER_PARAMS* a2dp_header);

/* Operator Message handlers */
extern bool a2dp_dec_opmsg_enable_fadeout(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool a2dp_dec_opmsg_disable_fadeout(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
bool a2dp_dec_assign_buffering(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#ifndef TIMED_PLAYBACK_MODE
extern bool a2dp_dec_opmsg_ratematch_enacting(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool a2dp_dec_opmsg_ratematch_enacting_core(OPERATOR_DATA *op_data,
                    void *message_data, unsigned int bsize);
#endif

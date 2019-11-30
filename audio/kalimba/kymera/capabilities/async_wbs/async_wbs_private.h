/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup wbs
 * \file  async_wbs_private.h
 * \ingroup capabilities
 *
 * Wide Band Speech operators private header file. <br>
 *
 */

#ifndef ASYNC_WBS_PRIVATE_H
#define ASYNC_WBS_PRIVATE_H
/*****************************************************************************
Include Files
*/
#include "async_wbs_struct.h"
#include "wbs_private.h"

/****************************************************************************
Private Const Declarations
*/

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define ASYNC_WBS_ENC_CAP_ID CAP_ID_DOWNLOAD_ASYNC_WBS_ENC
#define ASYNC_WBS_DEC_CAP_ID CAP_ID_DOWNLOAD_ASYNC_WBS_DEC
#else
#define ASYNC_WBS_ENC_CAP_ID CAP_ID_ASYNC_WBS_ENC
#define ASYNC_WBS_DEC_CAP_ID CAP_ID_ASYNC_WBS_DEC
#endif

#define ASYNC_WBS_DEFAULT_TESCO (12)

#define ASYNC_WBS_MAX_BITPOOL_VALUE (26)
#define ASYNC_WBS_MIN_BITPOOL_VALUE (10) /* Below this value audio is not understandable*/

/****************************************************************************
Private Type Definitions
*/

/*****************************************************************************
Private Function Declarations
*/
extern unsigned awbs_encode_frame_size(sbc_codec *codec_data);
extern bool awbs_enc_opmsg_set_encoding_params(OPERATOR_DATA *op_data, void *message_data, unsigned int *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool awbs_enc_opmsg_set_frames_per_packet(OPERATOR_DATA *op_data, void *message_data, unsigned int *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool awbs_dec_opmsg_set_encoding_params(OPERATOR_DATA *op_data, void *message_data, unsigned int *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Message handlers */
extern bool async_wbs_enc_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool async_wbs_enc_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool async_wbs_enc_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool async_wbs_enc_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Data processing function */
extern void async_wbs_enc_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/* Message handlers */
extern bool async_wbs_dec_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool async_wbs_dec_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool async_wbs_dec_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool async_wbs_dec_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Data processing function */
extern void async_wbs_dec_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/* get current encoder/decoder buffer size */
extern bool async_wbs_enc_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool async_wbs_dec_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
#endif /* ASYNC_WBS_PRIVATE_H */

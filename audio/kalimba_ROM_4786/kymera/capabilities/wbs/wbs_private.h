/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup wbs
 * \file  wbs_private.h
 * \ingroup capabilities
 *
 * Wide Band Speech operators private header file. <br>
 *
 */

#ifndef WBS_PRIVATE_H
#define WBS_PRIVATE_H
/*****************************************************************************
Include Files
*/
#include "types.h"
#include "wbs_struct.h"
#include "capabilities.h"
#include "sco_common_funcs.h"
#include "sco_fw_c.h"
#include "mem_utils/shared_memory_ids.h"
#include "mem_utils/scratch_memory.h"
#include "mem_utils/memory_table.h"
#include "fault/fault.h"

/****************************************************************************
Private Const Declarations
*/
#define WBS_DEC_MALLOC_TABLE_LENGTH 2
#define WBS_ENC_MALLOC_TABLE_LENGTH 2
#define WBS_DM1_SCRATCH_TABLE_LENGTH (2)


#define WBS_DEFAULT_ENCODED_BLOCK_SIZE  30

/** Default buffer size, minimum and default block size for wbs_enc */
#define WBS_ENC_DEFAULT_OUTPUT_BLOCK_SIZE        WBS_DEFAULT_ENCODED_BLOCK_SIZE
#define WBS_ENC_DEFAULT_INPUT_BLOCK_SIZE         120
#define WBS_ENC_INPUT_BUFFER_SIZE (256) /* Big enough for 2 frames of data */
#define WBS_ENC_OUTPUT_BUFFER_SIZE              (SCO_DEFAULT_SCO_BUFFER_SIZE)

/** Default buffer sizes, minimum and default block size for wbs_dec */
#define WBS_DEC_DEFAULT_INPUT_BLOCK_SIZE             0
#define WBS_DEC_DEFAULT_OUTPUT_BLOCK_SIZE            120
#define WBS_DEC_INPUT_BUFFER_SIZE                  (SCO_DEFAULT_SCO_BUFFER_SIZE)
#define WBS_DEC_OUTPUT_BUFFER_SIZE (256) /* Big enough to decode 2 frames */

/* WBS shared table lengths */
#define WBS_SBC_SHARED_TABLE_LENGTH     3
#define WBS_SBC_ENC_SHARED_TABLE_LENGTH 2
#define WBS_SBC_DEC_SHARED_TABLE_LENGTH 3

#define WBS_AUDIO_SAMPLE_BUFF_SIZE  120

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define WBS_ENC_CAP_ID CAP_ID_DOWNLOAD_WBS_ENC
#define WBS_DEC_CAP_ID CAP_ID_DOWNLOAD_WBS_DEC
#else
#define WBS_ENC_CAP_ID CAP_ID_WBS_ENC
#define WBS_DEC_CAP_ID CAP_ID_WBS_DEC
#endif

#define WBS_DEC_PROCESS_OK 0
#define WBS_DEC_PROCESS_NO_OUTPUT -1
#define WBS_DEC_PROCESS_FAKE_FRAME -2

/****************************************************************************
Private Type Definitions
*/
/*****************************************************************************
WBS shared tables
*/
/** memory shared between WBS enc and dec */
extern const share_malloc_t_entry wbs_sbc_shared_malloc_table[];

/** memory shared between WBS encoders */
extern const share_malloc_t_entry wbs_sbc_enc_shared_malloc_table[];

/** memory shared between WBS decoders */
extern const share_malloc_t_entry wbs_sbc_dec_shared_malloc_table[];

/** Scratch memory used by an encoder and decoder instance */
extern const scratch_malloc_t_entry wbs_scratch_table_dm1[];

/*****************************************************************************
Private Function Declarations
*/
/* Reset for SBC codec data */
extern void wbs_dec_reset_sbc_data(OPERATOR_DATA* op_data);
extern void wbs_enc_reset_sbc_data(OPERATOR_DATA* op_data);

/* Decode processing function */
extern unsigned wbs_dec_processing(OPERATOR_DATA *op_data);

/* Encoder ASM defined functions and C stubs to SBC encoder/decoder*/
extern void wbsenc_init_encoder(OPERATOR_DATA *op_data);
extern void wbsdec_init_dec_param(OPERATOR_DATA *op_data);
extern void wbsenc_process_frame(OPERATOR_DATA *op_data);

/* Message handlers */
extern bool wbs_enc_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_enc_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_enc_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_enc_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_enc_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_enc_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_enc_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
extern bool wbs_enc_opmsg_enable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool wbs_enc_opmsg_disable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool wbs_enc_opmsg_set_to_air_info(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Data processing function */
extern void wbs_enc_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/* Message handlers */
extern bool wbs_dec_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_dec_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_dec_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_dec_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_dec_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_dec_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_dec_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool wbs_dec_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
extern bool wbs_dec_opmsg_enable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool wbs_dec_opmsg_disable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#ifdef INSTALL_PLC100
extern bool wbs_dec_opmsg_force_plc_off(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif /* INSTALL_PLC100 */

extern bool wbs_dec_opmsg_frame_counts(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool wbs_dec_opmsg_set_from_air_info(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool wbs_dec_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool wbs_dec_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool wbs_dec_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool wbs_dec_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool wbs_dec_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Data processing function */
extern void wbs_dec_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);
/**
 * \brief Validates an incoming WBS packet.
 *
 * \param opdata WBS operator data.
 * \param payload_length Sco packet payload length.
 * \param wbs_packet_length pointer which will be populated with the WBS packet length.
 * \param amount_advanced pointer which will be populated with the amount the read pointer advanced.
 *
 * \return Validate returns in r1 either 120 or 240 (1 or 2 frames worth of data) if
 *         everything is alright. If it returns 0, 1 or 2, then there is no data
 *           0 means there is not enough space in output buffer
 *           1 means there is not enough data in the input buffer
 *           2 means there is some data in input buffer, but unable to identify the WBS
 *             frame sync.
 */
extern unsigned sco_decoder_wbs_validate(void* opdata, unsigned payload_length, unsigned *wbs_packet_length, unsigned *amount_advanced);

/**
 * \brief Process an incoming WBS packet.
 *
 * \param opdata WBS operator data.
 * \param packet The incoming SCO stream packet.
 * \param validate_retval Return value from sco_decoder_wbs_validate.
 * \param wbs_packet_length WBS packet length in the payload.
 *
 * \return output packet status:
 *          -2 output bad, GENERATE_FAKE_FRAME     BFI
 *          -1 no output                           RETCODE_NO_OUTPUT
 *           0 output good no compensation         BFI
 *           1 output bad, needs compensation      BFI
 *           2 output bad, needs compensation      BFI
 */
extern int sco_decoder_wbs_process(void* opdata, stream_sco_metadata *packet, unsigned validate_retval, unsigned wbs_packet_length);
/* get current encoder/decoder buffer size */
extern bool wbs_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
#endif /* WBS_PRIVATE_H */

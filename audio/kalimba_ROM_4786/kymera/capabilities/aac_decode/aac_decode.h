/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  aac_decode.h
 * \ingroup capabilities
 *
 * AAC Decode Capability public header file. <br>
 *
 */

#ifndef AAC_DECODE_H
#define AAC_DECODE_H


/** The capability data structure for sbc decode */
#ifdef INSTALL_OPERATOR_AAC_DECODE
extern const CAPABILITY_DATA aac_decode_cap_data;
#endif

#ifdef INSTALL_OPERATOR_AAC_SHUNT_DECODER
extern const CAPABILITY_DATA aac_a2dp_decoder_cap_data;
#endif

/****************************************************************************
Private Function Definitions
*/
/* Message handlers */
extern bool aac_decode_create(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
extern bool aac_decode_destroy(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);
extern bool aac_decode_reset(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);

extern void aac_decode_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched);

extern bool aac_dec_opmsg_content_protection_enable(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool aac_decode_buffer_details(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data );
extern bool aac_decode_connect(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
#ifndef TIMED_PLAYBACK_MODE
extern bool aac_dec_opmsg_ratematch_enacting(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif
extern bool aac_set_frame_type(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);


/****************************************************************************
OBPM Function Definitions
*/
extern bool aac_dec_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aac_dec_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aac_dec_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aac_dec_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool aac_dec_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

#endif /* AAC_DECODE_H */

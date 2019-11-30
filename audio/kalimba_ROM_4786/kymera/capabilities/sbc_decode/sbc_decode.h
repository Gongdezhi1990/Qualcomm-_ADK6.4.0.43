/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  sbc_decode.h
 * \ingroup capabilities
 *
 * SBC Decode Capability public header file. <br>
 *
 */

#ifndef SBC_DECODE_H
#define SBC_DECODE_H


/** The capability data structure for sbc decode */
#ifdef INSTALL_OPERATOR_SBC_DECODE
extern const CAPABILITY_DATA sbc_decode_cap_data;
#endif

#ifdef INSTALL_OPERATOR_SBC_SHUNT_DECODER
extern const CAPABILITY_DATA sbc_a2dp_decoder_cap_data;
#endif

/****************************************************************************
Private Function Definitions
*/
/* Message handlers */
extern bool sbc_decode_create(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
extern bool sbc_decode_destroy(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);
extern bool sbc_decode_reset(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);

extern void sbc_decode_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched);

extern bool sbc_dec_opmsg_content_protection_enable(OPERATOR_DATA *op_data,
                       void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

					   
/****************************************************************************
OBPM Function Definitions
*/
extern bool sbc_dec_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sbc_dec_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sbc_dec_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sbc_dec_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sbc_dec_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

#endif /* SBC_DECODE_H */

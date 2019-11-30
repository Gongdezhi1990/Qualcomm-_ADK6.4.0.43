/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  sbc_encode.h
 * \ingroup capabilities
 *
 * SBC Encode Capability public header file. <br>
 *
 */

#ifndef SBC_ENCODE_H
#define SBC_ENCODE_H


/** The capability data structure for sbc decode */
#ifdef INSTALL_OPERATOR_SBC_ENCODE
extern const CAPABILITY_DATA sbc_encode_cap_data;
#endif

/****************************************************************************
Private Function Definitions
*/
/* Message handlers */
extern bool sbc_encode_create(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);

/* Operator message handlers */
extern bool sbc_enc_opmsg_set_encoding_params(OPERATOR_DATA *op_data, void *message_data, unsigned int *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Misc internal functions that need prototypes */
extern void sbc_encode_free_data(OPERATOR_DATA *op_data);
extern bool sbc_encode_reset(OPERATOR_DATA *op_data);
extern bool sbc_encode_frame_sizes(OPERATOR_DATA *op_data, unsigned *in_size_samples, unsigned *out_size_octets);

/****************************************************************************
OBPM Function Definitions
*/
extern bool sbc_enc_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sbc_enc_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sbc_enc_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sbc_enc_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sbc_enc_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

#endif /* SBC_ENCODE_H */

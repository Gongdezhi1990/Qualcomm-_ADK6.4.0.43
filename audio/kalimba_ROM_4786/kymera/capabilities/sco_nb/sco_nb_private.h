/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup sco_nb
 * \file  sco_nb_private.h
 * \ingroup opmgr
 *
 * Sco NNS operators private header file. <br>
 *
 */

#ifndef _SCO_NB_PRIVATE_H_
#define _SCO_NB_PRIVATE_H_
/*****************************************************************************
Include Files
*/
#include "capabilities.h"
#include "sco_struct.h"
#include "sco_fw_c.h"
#include "fault/fault.h"

/****************************************************************************
Public Constant Definitions
*/

/* Capability Version */
#define SCO_RCV_NB_VERSION_MINOR            1

/****************************************************************************
Public Type Declarations
*/

/*****************************************************************************
Private Function Definitions
*/

/* Receive ASM processing function */
extern unsigned sco_rcv_processing(OPERATOR_DATA *op_data);

/* Message handlers */
extern bool sco_send_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_send_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_send_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_send_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_send_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_send_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_send_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_send_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_send_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
extern bool sco_send_opmsg_enable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sco_send_opmsg_disable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sco_send_opmsg_set_to_air_info(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Data processing function */
extern void sco_send_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/* Message handlers */
extern bool sco_rcv_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_rcv_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_rcv_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_rcv_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_rcv_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_rcv_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_rcv_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_rcv_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool sco_rcv_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
extern bool sco_rcv_opmsg_enable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sco_rcv_opmsg_disable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#ifdef INSTALL_PLC100
extern bool sco_rcv_opmsg_force_plc_off(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif /* INSTALL_PLC100 */
extern bool sco_rcv_opmsg_frame_counts(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sco_rcv_opmsg_set_from_air_info(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sco_rcv_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sco_rcv_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sco_rcv_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sco_rcv_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool sco_rcv_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Data processing function */
extern void sco_rcv_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

#endif /* _SCO_NB_PRIVATE_H_ */

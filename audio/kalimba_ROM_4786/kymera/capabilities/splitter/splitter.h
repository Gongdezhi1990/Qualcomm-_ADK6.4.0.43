/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup splitter
 * \file  splitter.h
 * \ingroup capabilities
 *
 * Splitter operator public header file. <br>
 *
 */

#ifndef SPLITTER_H
#define SPLITTER_H

#include "capabilities.h"

/****************************************************************************
Public Variable Definitions
*/
/** The capability data structure for splitter */
extern const CAPABILITY_DATA splitter_cap_data;

/****************************************************************************
Private Function Declarations
*/
extern bool splitter_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool splitter_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool splitter_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool splitter_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool splitter_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool splitter_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool splitter_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool splitter_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

extern void splitter_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched);

extern bool splitter_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool splitter_set_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool splitter_set_running_streams(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

#endif /* SPLITTER_H */

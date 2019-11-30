/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup capabilities Capabilities 
 * \file  capabilities.h
 * \ingroup capabilities
 *
 * Capabilities public header file. <br>
 * This file contains access to the Kymera API required by all capabilities. <br>
 *
 */

#ifndef CAPABILITIES_H
#define CAPABILITIES_H

/****************************************************************************
Include Files
*/
#include "types.h"
#include "cap_id_prim.h"
#include "pmalloc/pl_malloc.h"
#include "sched_oxygen/sched_oxygen.h"
#include "opmgr/opmgr.h"
#include "opmgr/opmgr_for_ops.h"
#include "audio_log/audio_log.h"
#include "base_op.h"
#include "panic/panic.h"
#include "audio_fadeout.h"
#include "opmsg_prim.h"
/****************************************************************************
Private Type Definitions
*/


/****************************************************************************
Public Function Declarations
*/

/**
 * \brief Common operator message handler for changing output data type of single-output operator.
 *
 * \param  op_data  Pointer to the operator structure.
 * \param  message_data  Pointer to message structure.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool common_opmsg_change_single_output_data_type(OPERATOR_DATA *op_data, void *message_data);

/**
 * \brief Common function for setting fadeout state (and other fadeout parameters in relation to that, if there is a
 *        state transition).
 *
 * \param  fadeout_parameters  Pointer to the FADEOUT_PARAMS structure.
 * \param  state  Fadeout state to be set.
 *
 * \return None
 */
extern void common_set_fadeout_state(FADEOUT_PARAMS* fadeout_parameters, FADEOUT_STATE state);

/**
 * \brief Common function for sending simple unsolicited message to OpManager, message has no real payload,
 *        just the unsolicited message ID.
 *
 * \param  op_data  Pointer to the operator data structure for the operator that is sending the message.
 * \param  msg_id  ID of the unsolicited message.
 */
extern void common_send_simple_unsolicited_message(OPERATOR_DATA *op_data, unsigned msg_id);

/**
 * \brief Common function for sending unsolicited message to OpManager
 * 
 * \param  op_data  Pointer to the operator data structure for the operator that is sending the message.
 * \param  msg_id  ID of the unsolicited message.
 * \param  length  length of the payload, this is nuber of unsigned words in the payload message but
 *         note that only lower 16 bits of the message words will be sent
 * \param  payload payload message
 * \return FALSE if immediately failed to send the message, TRUE means that the message has been put
 *         in the queue for sending.
 */
extern bool common_send_unsolicited_message(OPERATOR_DATA *op_data, unsigned msg_id, unsigned length, const unsigned *payload);

#endif /* CAPABILITIES_H */

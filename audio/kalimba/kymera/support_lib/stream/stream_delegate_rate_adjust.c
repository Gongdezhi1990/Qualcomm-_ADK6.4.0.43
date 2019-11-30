/****************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 ****************************************************************************/
/**
 * \file  stream_delegate_rate_adjust.c
 * \ingroup stream
 *
 * This file contains API functions for sending messages to a standlone
 * rate adjust operator. <br>
 *
 * \section sec1 Contains:
 * stream_delegate_rate_adjust_set_target_rate
 * stream_delegate_rate_adjust_set_passthrough_mode
 * stream_delegate_rate_adjust_set_current_rate
 */

/****************************************************************************
Include Files
*/
#include "adaptor/adaptor.h"
#include "opmsg_prim.h"
#include "audio_log/audio_log.h"
#include "opmgr/opmgr.h"
#include "stream_delegate_rate_adjust.h"

/****************************************************************************
Private Function Definitions
*/

/**
 * rate_adjust_message_callback
 *
 * \brief call back for sending message to this operator
 */
static bool stream_delegate_rate_adjust_message_callback(unsigned con_id,
                                                         unsigned status,
                                                         unsigned op_id,
                                                         unsigned num_resp_params,
                                                         unsigned *resp_params)
{
    /* No action, just log the response if message if it failed */
    if(status != STATUS_OK)
    {
        L2_DBG_MSG4("Message to standalone rate adjust op failed: conid=%d, opid=%d, status=%d, messageID=%d",
                    con_id,
                    op_id,
                    status,
                    num_resp_params == 0?-1:resp_params[0]);
    }

    return TRUE;
}

/****************************************************************************
Public Function Definitions
*/

/**
 * \brief send message to a standalone rate adjust op to set the target rate address
 *
 * \param opid operator id for the standalone rate adjust operator
 * \param target_rate_addr target rate
 */
void stream_delegate_rate_adjust_set_target_rate(unsigned opid, unsigned target_rate)
{
    unsigned params[3];

    /* send SET_TARGET_RATE message to the operator */
    params[0] = OPMSG_COMMON_SET_RATE_ADJUST_TARGET_RATE;
    params[1] = (uint16) (target_rate >> 16);
    params[2] = (uint16) (target_rate & 0xFFFF);
    opmgr_operator_message(RESPOND_TO_OBPM,
                           opid,
                           sizeof(params)/sizeof(unsigned),
                           params,
                           stream_delegate_rate_adjust_message_callback);
}

/**
 * \brief sends message to a standalone rate adjust op to enable/disable
 *        passthrough mode
 *
 * \param opid operator id for the standalone rate adjust operator
 * \param enable if TRUE enables pass-through mode else disables it
 */
void stream_delegate_rate_adjust_set_passthrough_mode(unsigned opid, bool enable)
{
    unsigned params[2];

    /* send PASSTHROUGH_MODE message to the operator */
    params[0] = OPMSG_COMMON_SET_RATE_ADJUST_PASSTHROUGH_MODE;
    params[1] = (uint16) enable;
    opmgr_operator_message(RESPOND_TO_OBPM,
                           opid,
                           sizeof(params)/sizeof(unsigned),
                           params,
                           stream_delegate_rate_adjust_message_callback);
}

/**
 * \brief send message to a standalone rate adjust op to set the current
 *             rate of the rate adjust operator
 *
 * \param opid operator id for the standalone rate adjust operator
 * \param rate the current rate to be used
 *
 * Note: This message when delivered will directly set the current sra rate,
 *       suitable for TTP-type rate adjustment.
 */
void stream_delegate_rate_adjust_set_current_rate(unsigned opid, unsigned rate)
{
    unsigned params[3];

    /* send SET_CURRENT_RATE message to the operator */
    params[0] = OPMSG_COMMON_SET_RATE_ADJUST_CURRENT_RATE;
    params[1] = (uint16) (rate >> 16);
    params[2] = (uint16) (rate & 0xFFFF);
    opmgr_operator_message(RESPOND_TO_OBPM,
                           opid,
                           sizeof(params)/sizeof(unsigned),
                           params,
                           stream_delegate_rate_adjust_message_callback);
}



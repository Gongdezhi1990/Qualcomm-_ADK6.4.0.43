/*************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/

#ifdef INSTALL_AOV
#include "opmgr/opmgr_for_ops.h"
#include "types.h"
#ifndef AOV_FOR_OPS_H
#define AOV_FOR_OPS_H

/*
 * \brief Sends a trigger notification to the always on voice module
 *        A voice activity detector would usually only send
 *        positive triggers (trigerred = TRUE)
 *        A trigger phrase detector may send both positive and negative
 *        (trigerred = FALSE) triggers
 * \param op_data operator instance
 * \param triggerred TRUE if triggerred, FALSE if not
 *
 * \return None
 */

void aov_send_trigger_notification(OPERATOR_DATA * op_data, bool triggerred);

#endif

#endif // INSTALL_AOV

/****************************************************************************
* Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file opmgr_for_ops_dsl.h
 * \ingroup opmgr
 *
 * New opmgr macros missing from opmgr_for_ops.h
 */
#ifndef _OPMGR_FOR_OPS_DSL_H_
#define _OPMGR_FOR_OPS_DSL_H_

#include "opmgr/opmgr_for_ops.h"
/* macro for retrieving a pointer to a word from opmsg with an offset from a named field */
#define OPMSG_FIELD_POINTER_GET_FROM_OFFSET(msg, msgtype, field, offset) (&((unsigned int *)msg)[msgtype##_##field##_WORD_OFFSET + OPCMD_MSG_HEADER_SIZE + offset])

#endif /* _OPMGR_FOR_OPS_DSL_H_ */
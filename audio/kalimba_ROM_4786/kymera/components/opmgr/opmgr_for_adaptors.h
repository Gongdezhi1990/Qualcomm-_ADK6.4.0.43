/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_for_adaptors.h
 * \ingroup opmgr
 *
 * Operator Manager header file used by adaptor(s). <br>
 *
 */

#ifndef _OPMGR_FOR_ADAPTORS_H_
#define _OPMGR_FOR_ADAPTORS_H_

#include "opmgr/opmgr_types.h"
#include "opmgr/opmgr_for_ops.h"
#include "platform/profiler_c.h"

#if defined(INSTALL_DUAL_CORE_SUPPORT)
extern OPERATOR_DATA* get_remote_op_data_from_id(unsigned int id);
extern OPERATOR_DATA* get_multicore_op_data_from_id(unsigned int id);
#endif /* INSTALL_DUAL_CORE_SUPPORT */

extern const CAPABILITY_DATA* opmgr_lookup_cap_data_for_cap_id(unsigned cap_id);

#ifdef PROFILER_ON
/**
 * Function to get the operator mips usage.
 * @param op_id Operatora ID
 * @return Returns the operator mips usage in permille. -1 if the operator doesn't exist.
 */
extern unsigned opmgr_get_operator_mips(unsigned op_id);
#endif
#endif /* _OPMGR_FOR_ADAPTORS_H_ */

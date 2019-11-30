/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup secure_basic_passthrough
 * \file  secure_basic_passthrough.h
 * \ingroup capabilities
 *
 * Basic passthrough operator public header file. <br>
 *
 */

#ifndef _SECURE_BASIC_PASSTHROUGH_OP_H_
#define _SECURE_BASIC_PASSTHROUGH_OP_H_

#include "capabilities.h"

/****************************************************************************
Public Constant Definitions
*/
#define INSTALL_LICENSE_CHECK                                  1

/* Use this to support early versions of 'secure basic passthrough'
 * that did not make use of 'get_bt_address' in patch B-271257.
 */
/*
#define LEGACY_LICENSING                                       1
*/

/****************************************************************************
Public Variable Definitions
*/
/** The capability data structure for basic passthrough */
extern const CAPABILITY_DATA secure_basic_passthrough_cap_data;

/** The capability data structure for passthrough with time-to-play */
extern const CAPABILITY_DATA secure_ttp_passthrough_cap_data;

#endif /* _SECURE_BASIC_PASSTHROUGH_OP_H_ */

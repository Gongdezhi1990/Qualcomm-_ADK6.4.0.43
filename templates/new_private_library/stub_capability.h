/****************************************************************************
 * Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd 
****************************************************************************/
/**
 * \file  @@@cap_name@@@.h
 * \ingroup capabilities
 *
 * Stub Capability public header file. <br>
 *
 */

#ifndef @@@cap_name^U@@@_H
#define @@@cap_name^U@@@_H

#include "@@@cap_name@@@_struct.h"

/** The capability data structure for stub capability */
extern const CAPABILITY_DATA @@@cap_name@@@_cap_data;

/** Assembly processing function */
extern void @@@cap_name@@@_proc_func(@@@cap_name^U@@@_OP_DATA *op_data);

/** Assembly private library entry-point function (API) */
extern int @@@plib_name@@@_entry(int num);

#endif /* @@@cap_name^U@@@_H */

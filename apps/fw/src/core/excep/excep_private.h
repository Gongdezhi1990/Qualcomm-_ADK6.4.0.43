/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef EXCEP_PRIVATE_H_
#define EXCEP_PRIVATE_H_

#include "hydra/hydra.h"
#define IO_DEFS_MODULE_APPS_BANKED_EXCEPTIONS_P1
#define IO_DEFS_MODULE_APPS_SYS_CPU1_VM

#define IO_DEFS_MODULE_APPS_SYS_CPU0_VM
#define IO_DEFS_MODULE_K32_CORE
#define IO_DEFS_MODULE_K32_TRACE
#define IO_DEFS_MODULE_K32_MONITOR
#define IO_DEFS_MODULE_K32_TRACE_MONITOR
#define IO_DEFS_MODULE_APPS_SYS_EXCEPTIONS

#include "hal/hal.h"
#include "io/io_defs.h"
#include "int/int.h"
#include "panic/panic.h"
#include "appcmd/appcmd.h"
#include "hydra_log/hydra_log.h"
#include "excep/excep.h"
#include "assert.h"

/**
 * Register the appcmd handler for use by memory_exception_test.py
 *
 * \ingroup excep_test
 */
extern void excep_register_test_handler(void);


#endif /* EXCEP_PRIVATE_H_ */
/*@}*/

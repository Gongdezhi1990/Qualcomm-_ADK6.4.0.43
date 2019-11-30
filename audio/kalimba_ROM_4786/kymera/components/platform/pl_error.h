/**************************************************************************** 
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//** 
 * \file pl_error.h
 * \ingroup platform
 *
 * Header file for the kalimba error handler
 *
 ****************************************************************************/

#ifndef PL_ERROR_H
#define PL_ERROR_H

#include "types.h"

/****************************************************************************
Include Files
*/

/****************************************************************************
Public Macro Declarations
*/

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Global Variable Definitions
*/

/****************************************************************************
Public Function Prototypes
*/

/* General-purpose error handler for Kalimba libs */
extern void pl_lib_error_handler(unsigned data);

/* Handler for un-registered interrupt */
extern void pl_unknown_interrupt_handler(void);

/* Handler for memory-access exception */
void pl_sw_exception_handler(unsigned code_address);

/* Handler for stack-overflow exception */
void pl_stack_exception_handler(unsigned data);

#ifdef CHIP_BASE_A7DA_KAS
void pl_apb_slv_error(void);
#endif /* CHIP_BASE_A7DA_KAS */

/* Enable handler functions for memory exceptions.
 * We do this because we may handle an exception early in the boot sequence,
 * before we're ready to deal with it (e.g. the sssm has finished and panic is 
 * ready) */
extern void error_enable_exception_handlers(bool enable);

#endif   /* PL_ERROR_H */

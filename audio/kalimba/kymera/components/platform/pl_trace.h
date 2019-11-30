/**************************************************************************** 
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//** 
 * \file pl_trace.h
 * \ingroup platform
 *
 * Definition of trace macro
 *
 ****************************************************************************/

#if !defined(PL_TRACE_H)
#define PL_TRACE_H

/****************************************************************************
Include Files
*/

#include "types.h"
#include "pl_trace_masks.h"

/****************************************************************************
Public Macro Declarations
*/

/**
 * \brief  Macro used in PL_PRINT_Px macros to check if the mask if defined
 */
#define PL_CHECK_MASK(mask) if ((PlTraceMaskBitField >> (mask)) & 1)

/*
 * Trace macros to be used globally
 */

#ifdef PL_TRACE_PRINTF

/* For non kalimba builds both the PL_PRINT_PX and PL_DEV_PRINT_PX macros map to conditional calls to printf */
#include <stdio.h>
#define PL_PRINT_P0(mask, string) PL_CHECK_MASK(mask) printf(string)
#define PL_PRINT_P1(mask, string, a) PL_CHECK_MASK(mask) printf(string, a)
#define PL_PRINT_P2(mask, string, a, b) PL_CHECK_MASK(mask) printf(string, a, b)
#define PL_PRINT_P3(mask, string, a, b, c) PL_CHECK_MASK(mask) printf(string, a, b, c)
#define PL_PRINT_P4(mask, string, a, b, c, d) PL_CHECK_MASK(mask) printf(string, a, b, c, d)
#define PL_PRINT_P5(mask, string, a, b, c, d, e) PL_CHECK_MASK(mask) printf(string, a, b, c, d, e)

/* Use of this macro requires linking against the buffer module */
#define PL_PRINT_BUFFER(mask, buffer_p) PL_CHECK_MASK(mask) cbuffer_print(buffer_p)

#else /* PL_TRACE_PRINTF */

#ifdef PL_TRACE_HYDRA

#include "audio_log/audio_log.h"

#define PL_PRINT_P0(mask, string) PL_CHECK_MASK(mask) L0_DBG_MSG(string)
#define PL_PRINT_P1(mask, string, a) PL_CHECK_MASK(mask) L0_DBG_MSG1(string, a)
#define PL_PRINT_P2(mask, string, a, b) PL_CHECK_MASK(mask) L0_DBG_MSG2(string, a, b)
#define PL_PRINT_P3(mask, string, a, b, c) PL_CHECK_MASK(mask) L0_DBG_MSG3(string, a, b, c)
#define PL_PRINT_P4(mask, string, a, b, c, d) PL_CHECK_MASK(mask) L0_DBG_MSG4(string, a, b, c, d)
#define PL_PRINT_P5(mask, string, a, b, c, d, e) PL_CHECK_MASK(mask) L0_DBG_MSG5(string, a, b, c, d, e)

#define PL_PRINT_BUFFER(mask, buffer)

#else /* PL_TRACE_HYDRA */
 /* If neither PL_TRACE_PRINTF or PL_TRACE_HYDRA are defined for this module,
  *  macros compile to nothing 
  */
#define PL_PRINT_P0(mask, string)
#define PL_PRINT_P1(mask, string, ...)
#define PL_PRINT_P2(mask, string, ...)
#define PL_PRINT_P3(mask, string, ...)
#define PL_PRINT_P4(mask, string, ...)
#define PL_PRINT_P5(mask, string, ...)

#define PL_PRINT_BUFFER(mask, buffer)
#endif /* PL_TRACE_HYDRA */
#endif /* PL_TRACE_PRINTF */

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Global Variable Definitions
*/

/****************************************************************************
Public Function Prototypes
*/

#endif /* PL_TRACE_H */

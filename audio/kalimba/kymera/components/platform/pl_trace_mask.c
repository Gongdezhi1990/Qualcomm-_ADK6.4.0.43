/**************************************************************************** 
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//** 
 * \file  pl_trace_mask.c
 * \ingroup platform
 *
 * Global variables used for trace by PL module
 *
 ****************************************************************************/

/* Nothing defined in this file is needed if trace is not on */
#if defined(PL_TRACE_PRINTF) || defined(PL_TRACE_HYDRA)

#include "pl_trace_masks.h"

/* GLOBAL VARIABLE DECLARATIONS *********************************************/

#ifdef TR_PL_TRACE_MASK
unsigned int PlTraceMaskBitField = TR_PL_TRACE_MASK;
#else
unsigned int PlTraceMaskBitField = TR_PL_TRACE_MASK_DEFAULT;
#endif

#endif /* PL_TRACE_PRINTF || PL_TRACE_HYDRA  */

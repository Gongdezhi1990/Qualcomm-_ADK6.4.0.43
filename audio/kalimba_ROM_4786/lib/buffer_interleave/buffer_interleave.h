/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer_interleave
 * \file  buffer_interleave.h
 * \ingroup common
 *
 * Header file for buffer_interleave <br>
 *
 */

#ifndef _BUFFER_INTERLEAVE_H_
#define _BUFFER_INTERLEAVE_H_

/*****************************************************************************
Include Files
*/
#include "buffer/cbuffer_c.h"
#include "types.h"

/*****************************************************************************
Public Function Definitions
*/

/**
 * \brief copy multiple buffers into a single interleaved buffer
 *
 * \param output_buffer output interleaved buffer
 * \param input_buffers array of input buffers
 * \param nrof_buffers number of buffers
 * \param max_samples maximum number of samples to interleave
 *
 * \return number of words interleaved from each buffer
 */
extern unsigned interleave_buffers(tCbuffer* output_buffer, tCbuffer** input_buffers, unsigned nrof_buffers, unsigned max_samples);

/**
 * \brief deinterleave a single buffer to multiple buffers
 *
 * \param input_buffer input interleaved buffer
 * \param output_buffers array of output buffers
 * \param nrof_buffers number of buffers
 * \param max_samples maximum number of samples to interleave
 *
 * \return number of words de interleaved to each buffer
 */
extern unsigned deinterleave_buffers(tCbuffer* input_buffer, tCbuffer** output_buffers, unsigned nrof_buffers, unsigned max_samples);

#endif /* _BUFFER_INTERLEAVE_H_ */

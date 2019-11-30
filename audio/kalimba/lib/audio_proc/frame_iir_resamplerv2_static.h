/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#ifndef IIR_RESAMPLER_FRAME_HDR_STATIC
#define IIR_RESAMPLER_FRAME_HDR_STATIC

#include "iir_resamplerv2_common_static.h"

// iir_resampler_op.struct
// NOTE: input and output ptr fields are NOT in use for cbops case. All indexes cascaded and using size definitions, in case any component struct changes separately
.CONST 	$frame_iir_resamplerv2.iir_resampler_op_struct.INPUT_FRAME_PTR_FIELD	 0*ADDR_PER_WORD;
.CONST 	$frame_iir_resamplerv2.iir_resampler_op_struct.OUTPUT_FRAME_PTR_FIELD	 $frame_iir_resamplerv2.iir_resampler_op_struct.INPUT_FRAME_PTR_FIELD + ADDR_PER_WORD;
// NOTE: common params here are not to be confused with similar field in cbops struct. When resampler
// is utilised by cbops, then common params start right after a common cbops multichannel info table,
// hence indexing there is different.
.CONST 	$frame_iir_resamplerv2.iir_resampler_op_struct.COMMON_FIELD 	         $frame_iir_resamplerv2.iir_resampler_op_struct.OUTPUT_FRAME_PTR_FIELD + ADDR_PER_WORD;
// Channel-specific params, only one channel in this case.
.CONST 	$frame_iir_resamplerv2.iir_resampler_op_struct.CHANNEL_FIELD	         $frame_iir_resamplerv2.iir_resampler_op_struct.COMMON_FIELD + $iir_resamplerv2_common.iir_resampler_common_struct.STRUC_SIZE*ADDR_PER_WORD;
// Working buffers - only one channel, allocated dynamically with variable size
.CONST 	$frame_iir_resamplerv2.iir_resampler_op_struct.WORKING_FIELD	         $frame_iir_resamplerv2.iir_resampler_op_struct.CHANNEL_FIELD + $iir_resamplerv2_common.iir_resampler_channel_struct.STRUC_SIZE*ADDR_PER_WORD;

// Size in number of fields (it only includes fields up to the working data - latter is allocated as addendum to the struct
.CONST	$frame_iir_resamplerv2.iir_resampler_op_struct.STRUC_SIZE 	             ($frame_iir_resamplerv2.iir_resampler_op_struct.WORKING_FIELD >> LOG2_ADDR_PER_WORD) + 1;

#endif 

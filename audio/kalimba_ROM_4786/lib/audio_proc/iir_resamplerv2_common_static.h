/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#ifndef IIR_RESAMPLER_COMMON_STATIC_HDR
#define IIR_RESAMPLER_COMMON_STATIC_HDR

#include "portability_macros.h"

.CONST	$iir_resamplerv2_common.iir_19_s2	0;
.CONST	$iir_resamplerv2_common.iir_19_s3	1;
.CONST	$iir_resamplerv2_common.iir_19_s4	2;
.CONST	$iir_resamplerv2_common.iir_19_s5	3;
.CONST	$iir_resamplerv2_common.iir_15_s3	4;
.CONST	$iir_resamplerv2_common.iir_15_s2	5;
.CONST	$iir_resamplerv2_common.iir_9_s2	   6;

.CONST	$iir_resamplerv2_common.iir_1stStage_none	      0;
.CONST	$iir_resamplerv2_common.iir_1stStage_upsample	1;
.CONST	$iir_resamplerv2_common.iir_2ndStage_upsample	2;
.CONST	$iir_resamplerv2_common.iir_2ndStage_downsample	3;


// iir_resampler_stage_def.struct 
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.FUNC_PTR1_FIELD	0*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.FIR_SIZE_FIELD	1*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.IIR_SIZE_FIELD	2*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.ROUT_FIELD	3*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.INPUT_SCALE_FIELD	4*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.OUTPUT_SCALE_FIELD	5*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.FIRFILTER_FIELD	6*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.FRACRATIO_FIELD	7*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.INTRATIO_FIELD	8*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.IIRFUNCTION_FIELD	9*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_stage_def_struct.IIR_COEFFS_FIELD	10*ADDR_PER_WORD;
.CONST	$iir_resamplerv2_common.iir_resampler_stage_def_struct.STRUC_SIZE 	10;
// iir_resampler_def.struct 
.CONST 	$iir_resamplerv2_common.iir_resampler_def_struct.INT_RATIO_FIELD	0*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_def_struct.FRAC_RATIO_FIELD	1*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_def_struct.INT_RATIO_S1_FIELD	2*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_def_struct.FRAC_RATIO_S1_FIELD	3*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_def_struct.STAGE1_FIELD	4*ADDR_PER_WORD;
.CONST	$iir_resamplerv2_common.iir_resampler_def_struct.STRUC_SIZE 	5;
// iir_resampler_common.struct 
.CONST 	$iir_resamplerv2_common.iir_resampler_common_struct.FILTER_FIELD	0*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_common_struct.INPUT_SCALE_FIELD	1*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_common_struct.OUTPUT_SCALE_FIELD	2*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_common_struct.INTERMEDIATE_PTR_FIELD	3*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_common_struct.INTERMEDIATE_SIZE_FIELD	4*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_common_struct.RESET_FLAG_FIELD	5*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_common_struct.DBL_PRECISSION_FIELD	6*ADDR_PER_WORD;
.CONST	$iir_resamplerv2_common.iir_resampler_common_struct.STRUC_SIZE 	7;
// iir_resampler_channel.struct 
.CONST 	$iir_resamplerv2_common.iir_resampler_channel_struct.PARTIAL1_FIELD	0*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_channel_struct.SMPL_COUNT1_FIELD	1*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_channel_struct.FIR1_PTR_FIELD	2*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_channel_struct.IIR1_PTR_FIELD	3*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_channel_struct.PARTIAL2_FIELD	4*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_channel_struct.SMPL_COUNT2_FIELD	5*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_channel_struct.FIR2_PTR_FIELD	6*ADDR_PER_WORD;
.CONST 	$iir_resamplerv2_common.iir_resampler_channel_struct.IIR2_PTR_FIELD	7*ADDR_PER_WORD;
.CONST	$iir_resamplerv2_common.iir_resampler_channel_struct.STRUC_SIZE 	8;

#endif
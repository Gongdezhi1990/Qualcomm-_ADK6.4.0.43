// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __IIR_RESAMPLER_GEN_ASM_H__
#define __IIR_RESAMPLER_GEN_ASM_H__

// CodeBase IDs
.CONST $M.IIR_RESAMPLER_IIRV2_CAP_ID       	0x0094;
.CONST $M.IIR_RESAMPLER_IIRV2_ALT_CAP_ID_0       	0x4053;
.CONST $M.IIR_RESAMPLER_IIRV2_SAMPLE_RATE       	0;
.CONST $M.IIR_RESAMPLER_IIRV2_VERSION_MAJOR       	0;

// Constant Values


// Piecewise Disables


// Statistic Block
.CONST $M.IIR_RESAMPLER.STATUS.IN_RATE 		0*ADDR_PER_WORD;
.CONST $M.IIR_RESAMPLER.STATUS.OUT_RATE		1*ADDR_PER_WORD;
.CONST $M.IIR_RESAMPLER.STATUS.BLOCK_SIZE   	2;

// Parameter Block
.CONST $M.IIR_RESAMPLER.PARAMETERS.OFFSET_CONFIG		0*ADDR_PER_WORD;
.CONST $M.IIR_RESAMPLER.PARAMETERS.STRUCT_SIZE 		1;


#endif // __IIR_RESAMPLER_GEN_ASM_H__

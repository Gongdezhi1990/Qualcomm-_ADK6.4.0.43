// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __MIXER_GEN_ASM_H__
#define __MIXER_GEN_ASM_H__

// CodeBase IDs
.CONST $M.MIXER_MX_CAP_ID       	0x000A;
.CONST $M.MIXER_MX_ALT_CAP_ID_0       	0x405B;
.CONST $M.MIXER_MX_SAMPLE_RATE       	0;
.CONST $M.MIXER_MX_VERSION_MAJOR       	2;

.CONST $M.MIXER_CMX_CAP_ID       	0x0097;
.CONST $M.MIXER_CMX_SAMPLE_RATE       	0;
.CONST $M.MIXER_CMX_VERSION_MAJOR       	2;

// Constant Values


// Piecewise Disables


// Statistic Block
.CONST $M.MIXER.STATUS.SOURCE_MASK		0*ADDR_PER_WORD;
.CONST $M.MIXER.STATUS.SINK_MASK  		1*ADDR_PER_WORD;
.CONST $M.MIXER.STATUS.MIXER_STATE		2*ADDR_PER_WORD;
.CONST $M.MIXER.STATUS.BLOCK_SIZE      	3;

// Parameter Block
.CONST $M.MIXER.PARAMETERS.OFFSET_CONFIG		0*ADDR_PER_WORD;
.CONST $M.MIXER.PARAMETERS.STRUCT_SIZE 		1;


#endif // __MIXER_GEN_ASM_H__

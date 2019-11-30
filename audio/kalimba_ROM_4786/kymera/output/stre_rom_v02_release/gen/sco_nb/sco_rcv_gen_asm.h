// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __SCO_RCV_GEN_ASM_H__
#define __SCO_RCV_GEN_ASM_H__

// CodeBase IDs
.CONST $M.SCO_RCV_NB_CAP_ID       	0x0004;
.CONST $M.SCO_RCV_NB_ALT_CAP_ID_0       	0x4062;
.CONST $M.SCO_RCV_NB_SAMPLE_RATE       	8000;
.CONST $M.SCO_RCV_NB_VERSION_MAJOR       	1;

// Constant Values


// Piecewise Disables


// Statistic Block
.CONST $M.SCO_RCV.STATUS.SCO_PKT_SIZE        		0*ADDR_PER_WORD;
.CONST $M.SCO_RCV.STATUS.T_ESCO              		1*ADDR_PER_WORD;
.CONST $M.SCO_RCV.STATUS.FRAME_COUNT         		2*ADDR_PER_WORD;
.CONST $M.SCO_RCV.STATUS.FRAME_ERROR_COUNT   		3*ADDR_PER_WORD;
.CONST $M.SCO_RCV.STATUS.MD_LATE_PKTS        		4*ADDR_PER_WORD;
.CONST $M.SCO_RCV.STATUS.MD_EARLY_PKTS       		5*ADDR_PER_WORD;
.CONST $M.SCO_RCV.STATUS.OUT_OF_TIME_PKT_CNT 		6*ADDR_PER_WORD;
.CONST $M.SCO_RCV.STATUS.MD_OUT_OF_TIME_RESET		7*ADDR_PER_WORD;
.CONST $M.SCO_RCV.STATUS.BLOCK_SIZE               	8;

// Parameter Block
.CONST $M.SCO_RCV.PARAMETERS.OFFSET_FORCE_PLC_OFF		0*ADDR_PER_WORD;
.CONST $M.SCO_RCV.PARAMETERS.STRUCT_SIZE        		1;


#endif // __SCO_RCV_GEN_ASM_H__

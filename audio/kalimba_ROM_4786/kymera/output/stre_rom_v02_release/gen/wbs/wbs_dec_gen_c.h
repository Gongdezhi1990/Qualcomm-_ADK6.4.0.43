// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __WBS_DEC_GEN_C_H__
#define __WBS_DEC_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define WBS_DEC_WB_CAP_ID	0x0006
#define WBS_DEC_WB_ALT_CAP_ID_0	0x4064
#define WBS_DEC_WB_SAMPLE_RATE	16000
#define WBS_DEC_WB_VERSION_MAJOR	1

// Constant Definitions


// Runtime Config Parameter Bitfields


// Statistics Block
typedef struct _tag_WBS_DEC_STATISTICS
{
	ParamType OFFSET_SCO_PKT_SIZE;
	ParamType OFFSET_T_ESCO;
	ParamType OFFSET_FRAME_COUNT;
	ParamType OFFSET_FRAME_ERROR_COUNT;
	ParamType OFFSET_MD_LATE_PKTS;
	ParamType OFFSET_MD_EARLY_PKTS;
	ParamType OFFSET_OUT_OF_TIME_PKT_CNT;
	ParamType OFFSET_MD_OUT_OF_TIME_RESET;
	ParamType OFFSET_WBS_DEC_NO_OUTPUT;
	ParamType OFFSET_WBS_DEC_FAKE_PKT;
	ParamType OFFSET_WBS_DEC_GOOD_OUTPUT;
	ParamType OFFSET_WBS_DEC_BAD_OUTPUT;
}WBS_DEC_STATISTICS;

typedef WBS_DEC_STATISTICS* LP_WBS_DEC_STATISTICS;

// Parameters Block
typedef struct _tag_WBS_DEC_PARAMETERS
{
	ParamType OFFSET_FORCE_PLC_OFF;
}WBS_DEC_PARAMETERS;

typedef WBS_DEC_PARAMETERS* LP_WBS_DEC_PARAMETERS;

unsigned *WBS_DEC_GetDefaults(unsigned capid);

#endif // __WBS_DEC_GEN_C_H__

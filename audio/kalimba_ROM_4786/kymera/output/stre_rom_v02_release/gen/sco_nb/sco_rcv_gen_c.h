// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __SCO_RCV_GEN_C_H__
#define __SCO_RCV_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define SCO_RCV_NB_CAP_ID	0x0004
#define SCO_RCV_NB_ALT_CAP_ID_0	0x4062
#define SCO_RCV_NB_SAMPLE_RATE	8000
#define SCO_RCV_NB_VERSION_MAJOR	1

// Constant Definitions


// Runtime Config Parameter Bitfields


// Statistics Block
typedef struct _tag_SCO_RCV_STATISTICS
{
	ParamType OFFSET_SCO_PKT_SIZE;
	ParamType OFFSET_T_ESCO;
	ParamType OFFSET_FRAME_COUNT;
	ParamType OFFSET_FRAME_ERROR_COUNT;
	ParamType OFFSET_MD_LATE_PKTS;
	ParamType OFFSET_MD_EARLY_PKTS;
	ParamType OFFSET_OUT_OF_TIME_PKT_CNT;
	ParamType OFFSET_MD_OUT_OF_TIME_RESET;
}SCO_RCV_STATISTICS;

typedef SCO_RCV_STATISTICS* LP_SCO_RCV_STATISTICS;

// Parameters Block
typedef struct _tag_SCO_RCV_PARAMETERS
{
	ParamType OFFSET_FORCE_PLC_OFF;
}SCO_RCV_PARAMETERS;

typedef SCO_RCV_PARAMETERS* LP_SCO_RCV_PARAMETERS;

unsigned *SCO_RCV_GetDefaults(unsigned capid);

#endif // __SCO_RCV_GEN_C_H__

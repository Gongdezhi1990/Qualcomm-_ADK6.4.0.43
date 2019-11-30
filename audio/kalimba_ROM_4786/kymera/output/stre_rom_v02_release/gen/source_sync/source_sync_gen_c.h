// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __SOURCE_SYNC_GEN_C_H__
#define __SOURCE_SYNC_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define SOURCE_SYNC_SRC_SYNC_CAP_ID	0x0099
#define SOURCE_SYNC_SRC_SYNC_SAMPLE_RATE	0
#define SOURCE_SYNC_SRC_SYNC_VERSION_MAJOR	3

// Constant Definitions
#define SOURCE_SYNC_CONSTANT_COMP_METADATA   		0x00000001
#define SOURCE_SYNC_CONSTANT_COMP_SPLIT_ROUTE		0x00000002


// Runtime Config Parameter Bitfields


// System Mode
#define SOURCE_SYNC_SYSMODE_STATIC		0
#define SOURCE_SYNC_SYSMODE_FULL  		2
#define SOURCE_SYNC_SYSMODE_MAX_MODES		2

// Statistics Block
typedef struct _tag_SOURCE_SYNC_STATISTICS
{
	ParamType OFFSET_CUR_MODE;
	ParamType OFFSET_COMPILED_CONFIG;
	ParamType OFFSET_DIRTY;
	ParamType OFFSET_STALLED;
	ParamType OFFSET_STALL_OCCURRED;
}SOURCE_SYNC_STATISTICS;

typedef SOURCE_SYNC_STATISTICS* LP_SOURCE_SYNC_STATISTICS;

// Parameters Block
typedef struct _tag_SOURCE_SYNC_PARAMETERS
{
	ParamType OFFSET_SS_PERIOD;
	ParamType OFFSET_SS_MAX_PERIOD;
	ParamType OFFSET_SS_MAX_LATENCY;
	ParamType OFFSET_SS_P3;
	ParamType OFFSET_SS_STALL_RECOVERY_DEFAULT_FILL;
	ParamType OFFSET_SS_STALL_RECOVERY_CATCHUP_LIMIT;
}SOURCE_SYNC_PARAMETERS;

typedef SOURCE_SYNC_PARAMETERS* LP_SOURCE_SYNC_PARAMETERS;

unsigned *SOURCE_SYNC_GetDefaults(unsigned capid);

#endif // __SOURCE_SYNC_GEN_C_H__

// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __DBE_GEN_C_H__
#define __DBE_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define DBE_DBE_CAP_ID	0x002f
#define DBE_DBE_ALT_CAP_ID_0	0x4058
#define DBE_DBE_SAMPLE_RATE	0
#define DBE_DBE_VERSION_MAJOR	1

#define DBE_DBE_FULLBAND_CAP_ID	0x0090
#define DBE_DBE_FULLBAND_ALT_CAP_ID_0	0x4059
#define DBE_DBE_FULLBAND_SAMPLE_RATE	0
#define DBE_DBE_FULLBAND_VERSION_MAJOR	1

#define DBE_DBE_FULLBAND_BASSOUT_CAP_ID	0x0091
#define DBE_DBE_FULLBAND_BASSOUT_ALT_CAP_ID_0	0x405A
#define DBE_DBE_FULLBAND_BASSOUT_SAMPLE_RATE	0
#define DBE_DBE_FULLBAND_BASSOUT_VERSION_MAJOR	1

// Constant Definitions


// Runtime Config Parameter Bitfields
#define DBE_CONFIG_BYPASS                		0x00000001
#define DBE_CONFIG_BYPASS_XOVER          		0x00000002
#define DBE_CONFIG_BYPASS_BASS_OUTPUT_MIX		0x00000004


// System Mode
#define DBE_SYSMODE_STATIC   		0
#define DBE_SYSMODE_MUTE     		1
#define DBE_SYSMODE_FULL     		2
#define DBE_SYSMODE_PASS_THRU		3
#define DBE_SYSMODE_MAX_MODES		4

// System Control
#define DBE_CONTROL_MODE_OVERRIDE		0x2000

// Statistics Block
typedef struct _tag_DBE_STATISTICS
{
	ParamType OFFSET_CUR_MODE;
	ParamType OFFSET_OVR_CONTROL;
	ParamType OFFSET_COMPILED_CONFIG;
	ParamType OFFSET_OP_STATE;
	ParamType OFFSET_OP_INTERNAL_STATE;
}DBE_STATISTICS;

typedef DBE_STATISTICS* LP_DBE_STATISTICS;

// Parameters Block
typedef struct _tag_DBE_PARAMETERS
{
	ParamType OFFSET_DBE_CONFIG;
	ParamType OFFSET_EFFECT_STRENGTH;
	ParamType OFFSET_AMP_LIMIT;
	ParamType OFFSET_FC_LP;
	ParamType OFFSET_FC_HP;
	ParamType OFFSET_HARM_CONTENT;
	ParamType OFFSET_XOVER_FC;
	ParamType OFFSET_MIX_BALANCE;
}DBE_PARAMETERS;

typedef DBE_PARAMETERS* LP_DBE_PARAMETERS;

unsigned *DBE_GetDefaults(unsigned capid);

#endif // __DBE_GEN_C_H__

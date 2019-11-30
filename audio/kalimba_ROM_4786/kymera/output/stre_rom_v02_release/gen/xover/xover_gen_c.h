// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __XOVER_GEN_C_H__
#define __XOVER_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define XOVER_XOVER_CAP_ID	0x000033
#define XOVER_XOVER_ALT_CAP_ID_0	0x406B
#define XOVER_XOVER_SAMPLE_RATE	0
#define XOVER_XOVER_VERSION_MAJOR	1

// Constant Definitions


// Runtime Config Parameter Bitfields
#define XOVER_CONFIG_BYPXFAD  		0x00000001
#define XOVER_CONFIG_INV_BAND1		0x00000002
#define XOVER_CONFIG_INV_BAND2		0x00000004


// System Mode
#define XOVER_SYSMODE_STATIC   		0
#define XOVER_SYSMODE_MUTE     		1
#define XOVER_SYSMODE_FULL     		2
#define XOVER_SYSMODE_PASS_THRU		3
#define XOVER_SYSMODE_MAX_MODES		4

// System Control
#define XOVER_CONTROL_MODE_OVERRIDE		0x2000

// Statistics Block
typedef struct _tag_XOVER_STATISTICS
{
	ParamType OFFSET_CUR_MODE;
	ParamType OFFSET_OVR_CONTROL;
	ParamType OFFSET_COMPILED_CONFIG;
	ParamType OFFSET_OP_STATE;
	ParamType OFFSET_OP_INTERNAL_STATE;
}XOVER_STATISTICS;

typedef XOVER_STATISTICS* LP_XOVER_STATISTICS;

// Parameters Block
typedef struct _tag_XOVER_PARAMETERS
{
	ParamType OFFSET_XOVER_CONFIG;
	ParamType OFFSET_NUM_BANDS;
	ParamType OFFSET_XOVER_FILTER_TYPE;
	ParamType OFFSET_LP_TYPE;
	ParamType OFFSET_LP_FC;
	ParamType OFFSET_LP_ORDER;
	ParamType OFFSET_HP_TYPE;
	ParamType OFFSET_HP_FC;
	ParamType OFFSET_HP_ORDER;
}XOVER_PARAMETERS;

typedef XOVER_PARAMETERS* LP_XOVER_PARAMETERS;

unsigned *XOVER_GetDefaults(unsigned capid);

#endif // __XOVER_GEN_C_H__

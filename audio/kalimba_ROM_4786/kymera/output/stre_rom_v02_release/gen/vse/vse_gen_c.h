// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __VSE_GEN_C_H__
#define __VSE_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define VSE_VSE_CAP_ID	0x00004A
#define VSE_VSE_ALT_CAP_ID_0	0x406A
#define VSE_VSE_SAMPLE_RATE	0
#define VSE_VSE_VERSION_MAJOR	1

// Constant Definitions


// Runtime Config Parameter Bitfields
#define VSE_CONFIG_BYPASS		0x00000001


// System Mode
#define VSE_SYSMODE_STATIC   		0
#define VSE_SYSMODE_MUTE     		1
#define VSE_SYSMODE_FULL     		2
#define VSE_SYSMODE_PASS_THRU		3
#define VSE_SYSMODE_MAX_MODES		4

// System Control
#define VSE_CONTROL_MODE_OVERRIDE		0x2000

// Statistics Block
typedef struct _tag_VSE_STATISTICS
{
	ParamType OFFSET_CUR_MODE;
	ParamType OFFSET_OVR_CONTROL;
	ParamType OFFSET_COMPILED_CONFIG;
	ParamType OFFSET_OP_STATE;
	ParamType OFFSET_OP_INTERNAL_STATE;
}VSE_STATISTICS;

typedef VSE_STATISTICS* LP_VSE_STATISTICS;

// Parameters Block
typedef struct _tag_VSE_PARAMETERS
{
	ParamType OFFSET_VSE_CONFIG;
	ParamType OFFSET_BINAURAL_FLAG;
	ParamType OFFSET_SPEAKER_SPACING;
	ParamType OFFSET_VIRTUAL_ANGLE;
}VSE_PARAMETERS;

typedef VSE_PARAMETERS* LP_VSE_PARAMETERS;

unsigned *VSE_GetDefaults(unsigned capid);

#endif // __VSE_GEN_C_H__

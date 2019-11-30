// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __PEQ_GEN_C_H__
#define __PEQ_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define PEQ_PEQ_CAP_ID	0x0049
#define PEQ_PEQ_ALT_CAP_ID_0	0x405C
#define PEQ_PEQ_SAMPLE_RATE	0
#define PEQ_PEQ_VERSION_MAJOR	1

// Constant Definitions


// Runtime Config Parameter Bitfields
#define PEQ_CONFIG_BYPASS		0x00000001


// System Mode
#define PEQ_SYSMODE_STATIC   		0
#define PEQ_SYSMODE_MUTE     		1
#define PEQ_SYSMODE_FULL     		2
#define PEQ_SYSMODE_PASS_THRU		3
#define PEQ_SYSMODE_MAX_MODES		4

// System Control
#define PEQ_CONTROL_MODE_OVERRIDE		0x2000

// Coefficients override mode indicator
#define PEQ_OvrCoeff_ON		1

// Statistics Block
typedef struct _tag_PEQ_STATISTICS
{
	ParamType OFFSET_CUR_MODE;
	ParamType OFFSET_OVR_CONTROL;
	ParamType OFFSET_OVR_COEFF_MODE;
	ParamType OFFSET_COMPILED_CONFIG;
	ParamType OFFSET_OP_STATE;
	ParamType OFFSET_OP_INTERNAL_STATE;
}PEQ_STATISTICS;

typedef PEQ_STATISTICS* LP_PEQ_STATISTICS;

// Parameters Block
typedef struct _tag_PEQ_PARAMETERS
{
	ParamType OFFSET_PEQ_CONFIG;
	ParamType OFFSET_CORE_TYPE;
	ParamType OFFSET_NUM_BANDS;
	ParamType OFFSET_MASTER_GAIN;
	ParamType OFFSET_STAGE1_TYPE;
	ParamType OFFSET_STAGE1_FC;
	ParamType OFFSET_STAGE1_GAIN;
	ParamType OFFSET_STAGE1_Q;
	ParamType OFFSET_STAGE2_TYPE;
	ParamType OFFSET_STAGE2_FC;
	ParamType OFFSET_STAGE2_GAIN;
	ParamType OFFSET_STAGE2_Q;
	ParamType OFFSET_STAGE3_TYPE;
	ParamType OFFSET_STAGE3_FC;
	ParamType OFFSET_STAGE3_GAIN;
	ParamType OFFSET_STAGE3_Q;
	ParamType OFFSET_STAGE4_TYPE;
	ParamType OFFSET_STAGE4_FC;
	ParamType OFFSET_STAGE4_GAIN;
	ParamType OFFSET_STAGE4_Q;
	ParamType OFFSET_STAGE5_TYPE;
	ParamType OFFSET_STAGE5_FC;
	ParamType OFFSET_STAGE5_GAIN;
	ParamType OFFSET_STAGE5_Q;
	ParamType OFFSET_STAGE6_TYPE;
	ParamType OFFSET_STAGE6_FC;
	ParamType OFFSET_STAGE6_GAIN;
	ParamType OFFSET_STAGE6_Q;
	ParamType OFFSET_STAGE7_TYPE;
	ParamType OFFSET_STAGE7_FC;
	ParamType OFFSET_STAGE7_GAIN;
	ParamType OFFSET_STAGE7_Q;
	ParamType OFFSET_STAGE8_TYPE;
	ParamType OFFSET_STAGE8_FC;
	ParamType OFFSET_STAGE8_GAIN;
	ParamType OFFSET_STAGE8_Q;
	ParamType OFFSET_STAGE9_TYPE;
	ParamType OFFSET_STAGE9_FC;
	ParamType OFFSET_STAGE9_GAIN;
	ParamType OFFSET_STAGE9_Q;
	ParamType OFFSET_STAGE10_TYPE;
	ParamType OFFSET_STAGE10_FC;
	ParamType OFFSET_STAGE10_GAIN;
	ParamType OFFSET_STAGE10_Q;
}PEQ_PARAMETERS;

typedef PEQ_PARAMETERS* LP_PEQ_PARAMETERS;

unsigned *PEQ_GetDefaults(unsigned capid);

#endif // __PEQ_GEN_C_H__

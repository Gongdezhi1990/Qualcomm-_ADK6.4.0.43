// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __MIXER_GEN_C_H__
#define __MIXER_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define MIXER_MX_CAP_ID	0x000A
#define MIXER_MX_ALT_CAP_ID_0	0x405B
#define MIXER_MX_SAMPLE_RATE	0
#define MIXER_MX_VERSION_MAJOR	2

#define MIXER_CMX_CAP_ID	0x0097
#define MIXER_CMX_SAMPLE_RATE	0
#define MIXER_CMX_VERSION_MAJOR	2

// Constant Definitions


// Runtime Config Parameter Bitfields


// Statistics Block
typedef struct _tag_MIXER_STATISTICS
{
	ParamType OFFSET_SOURCE_MASK;
	ParamType OFFSET_SINK_MASK;
	ParamType OFFSET_MIXER_STATE;
}MIXER_STATISTICS;

typedef MIXER_STATISTICS* LP_MIXER_STATISTICS;

// Parameters Block
typedef struct _tag_MIXER_PARAMETERS
{
	ParamType OFFSET_CONFIG;
}MIXER_PARAMETERS;

typedef MIXER_PARAMETERS* LP_MIXER_PARAMETERS;

unsigned *MIXER_GetDefaults(unsigned capid);

#endif // __MIXER_GEN_C_H__

// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __AAC_DECODE_GEN_C_H__
#define __AAC_DECODE_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define AAC_DECODE_AACD_CAP_ID	0x0018
#define AAC_DECODE_AACD_ALT_CAP_ID_0	0x404F
#define AAC_DECODE_AACD_SAMPLE_RATE	0
#define AAC_DECODE_AACD_VERSION_MAJOR	0

// Constant Definitions


// Runtime Config Parameter Bitfields


// Statistics Block
typedef struct _tag_AAC_DECODE_STATISTICS
{
	ParamType OFFSET_SF_INDEX;
	ParamType OFFSET_CHANNEL_CONFIG;
	ParamType OFFSET_AUDIO_OBJ_TYPE;
	ParamType OFFSET_EXT_AUDIO_OBJ_TYPE;
	ParamType OFFSET_SBR_PRESENT;
	ParamType OFFSET_MP4_FRAME_COUNT;
}AAC_DECODE_STATISTICS;

typedef AAC_DECODE_STATISTICS* LP_AAC_DECODE_STATISTICS;

// Parameters Block
typedef struct _tag_AAC_DECODE_PARAMETERS
{
	ParamType OFFSET_CONFIG;
}AAC_DECODE_PARAMETERS;

typedef AAC_DECODE_PARAMETERS* LP_AAC_DECODE_PARAMETERS;

unsigned *AAC_DECODE_GetDefaults(unsigned capid);

#endif // __AAC_DECODE_GEN_C_H__

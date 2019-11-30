// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __SBC_ENCODE_GEN_C_H__
#define __SBC_ENCODE_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define SBC_ENCODE_SBCENC_CAP_ID	0x0014
#define SBC_ENCODE_SBCENC_ALT_CAP_ID_0	0x4060
#define SBC_ENCODE_SBCENC_SAMPLE_RATE	0
#define SBC_ENCODE_SBCENC_VERSION_MAJOR	0

// Constant Definitions


// Runtime Config Parameter Bitfields


// Statistics Block
typedef struct _tag_SBC_ENCODE_STATISTICS
{
	ParamType OFFSET_SAMPLING_FREQUENCY;
	ParamType OFFSET_CHANNEL_MODE;
	ParamType OFFSET_BITPOOL;
	ParamType OFFSET_NUM_BLOCKS;
	ParamType OFFSET_NUM_CHANNELS;
	ParamType OFFSET_NUM_SUBBANDS;
	ParamType OFFSET_ALLOC_METHOD;
}SBC_ENCODE_STATISTICS;

typedef SBC_ENCODE_STATISTICS* LP_SBC_ENCODE_STATISTICS;

// Parameters Block
typedef struct _tag_SBC_ENCODE_PARAMETERS
{
	ParamType OFFSET_CONFIG;
}SBC_ENCODE_PARAMETERS;

typedef SBC_ENCODE_PARAMETERS* LP_SBC_ENCODE_PARAMETERS;

unsigned *SBC_ENCODE_GetDefaults(unsigned capid);

#endif // __SBC_ENCODE_GEN_C_H__

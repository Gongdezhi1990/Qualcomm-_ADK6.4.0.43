// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __SBC_DECODE_GEN_C_H__
#define __SBC_DECODE_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define SBC_DECODE_SBCDEC_CAP_ID	0x0016
#define SBC_DECODE_SBCDEC_ALT_CAP_ID_0	0x405F
#define SBC_DECODE_SBCDEC_SAMPLE_RATE	0
#define SBC_DECODE_SBCDEC_VERSION_MAJOR	0

// Constant Definitions


// Runtime Config Parameter Bitfields


// Statistics Block
typedef struct _tag_SBC_DECODE_STATISTICS
{
	ParamType OFFSET_SAMPLING_FREQUENCY;
	ParamType OFFSET_CHANNEL_MODE;
	ParamType OFFSET_BITPOOL;
	ParamType OFFSET_NUM_BLOCKS;
	ParamType OFFSET_NUM_CHANNELS;
	ParamType OFFSET_NUM_SUBBANDS;
	ParamType OFFSET_ALLOC_METHOD;
}SBC_DECODE_STATISTICS;

typedef SBC_DECODE_STATISTICS* LP_SBC_DECODE_STATISTICS;

// Parameters Block
typedef struct _tag_SBC_DECODE_PARAMETERS
{
	ParamType OFFSET_CONFIG;
}SBC_DECODE_PARAMETERS;

typedef SBC_DECODE_PARAMETERS* LP_SBC_DECODE_PARAMETERS;

unsigned *SBC_DECODE_GetDefaults(unsigned capid);

#endif // __SBC_DECODE_GEN_C_H__

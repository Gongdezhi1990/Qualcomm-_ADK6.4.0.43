// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __IIR_RESAMPLER_GEN_C_H__
#define __IIR_RESAMPLER_GEN_C_H__

#ifndef ParamType
typedef unsigned ParamType;
#endif

// CodeBase IDs
#define IIR_RESAMPLER_IIRV2_CAP_ID	0x0094
#define IIR_RESAMPLER_IIRV2_ALT_CAP_ID_0	0x4053
#define IIR_RESAMPLER_IIRV2_SAMPLE_RATE	0
#define IIR_RESAMPLER_IIRV2_VERSION_MAJOR	0

// Constant Definitions


// Runtime Config Parameter Bitfields


// Statistics Block
typedef struct _tag_IIR_RESAMPLER_STATISTICS
{
	ParamType OFFSET_IN_RATE;
	ParamType OFFSET_OUT_RATE;
}IIR_RESAMPLER_STATISTICS;

typedef IIR_RESAMPLER_STATISTICS* LP_IIR_RESAMPLER_STATISTICS;

// Parameters Block
typedef struct _tag_IIR_RESAMPLER_PARAMETERS
{
	ParamType OFFSET_CONFIG;
}IIR_RESAMPLER_PARAMETERS;

typedef IIR_RESAMPLER_PARAMETERS* LP_IIR_RESAMPLER_PARAMETERS;

unsigned *IIR_RESAMPLER_GetDefaults(unsigned capid);

#endif // __IIR_RESAMPLER_GEN_C_H__

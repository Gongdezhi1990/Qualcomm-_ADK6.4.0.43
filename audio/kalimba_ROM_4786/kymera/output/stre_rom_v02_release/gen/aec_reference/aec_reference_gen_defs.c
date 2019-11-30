// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "aec_reference_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_aec_referenceAECREF[] = {
   0x00002080u,			// CONFIG
   0x000000CFu,			// ADC_GAIN1
   0x000000CFu,			// ADC_GAIN2
   0x000000CFu,			// ADC_GAIN3
   0x000000CFu,			// ADC_GAIN4
   0x000000CFu,			// ADC_GAIN5
   0x000000CFu,			// ADC_GAIN6
   0x000000CFu,			// ADC_GAIN7
   0x000000CFu,			// ADC_GAIN8
   0x7FFFFFFFu,			// ST_CLIP_POINT
   0x00800000u,			// ST_ADJUST_LIMIT
   0x00000000u,			// STF_SWITCH
   0x00000000u,			// STF_NOISE_LOW_THRES
   0x00000000u,			// STF_NOISE_HIGH_THRES
   0x00000001u,			// STF_GAIN_EXP
   0x40000000u,			// STF_GAIN_MANTISSA
   0x00000003u,			// ST_PEQ_CONFIG
   0x00000001u,			// ST_PEQ_GAIN_EXP
   0x40000000u,			// ST_PEQ_GAIN_MANT
   0x373CC800u,			// ST_PEQ_STAGE1_B2
   0x91866F00u,			// ST_PEQ_STAGE1_B1
   0x373CC800u,			// ST_PEQ_STAGE1_B0
   0x2FAE2600u,			// ST_PEQ_STAGE1_A2
   0x92BB0300u,			// ST_PEQ_STAGE1_A1
   0x373CC800u,			// ST_PEQ_STAGE2_B2
   0x91866F00u,			// ST_PEQ_STAGE2_B1
   0x373CC800u,			// ST_PEQ_STAGE2_B0
   0x2FAE2600u,			// ST_PEQ_STAGE2_A2
   0x92BB0300u,			// ST_PEQ_STAGE2_A1
   0x373CC800u,			// ST_PEQ_STAGE3_B2
   0x91866F00u,			// ST_PEQ_STAGE3_B1
   0x373CC800u,			// ST_PEQ_STAGE3_B0
   0x2FAE2600u,			// ST_PEQ_STAGE3_A2
   0x92BB0300u,			// ST_PEQ_STAGE3_A1
   0x00000001u,			// ST_PEQ_SCALE1
   0x00000001u,			// ST_PEQ_SCALE2
   0x00000001u			// ST_PEQ_SCALE3
};

unsigned *AEC_REFERENCE_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0043: return defaults_aec_referenceAECREF;
		case 0x4007: return defaults_aec_referenceAECREF;
	}
	return((unsigned *)0);
}

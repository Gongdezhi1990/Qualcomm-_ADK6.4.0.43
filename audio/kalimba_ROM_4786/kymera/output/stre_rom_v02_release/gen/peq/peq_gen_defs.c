// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "peq_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_peqPEQ[] = {
   0x00000000u,			// PEQ_CONFIG
   0x00000000u,			// CORE_TYPE
   0x00000000u,			// NUM_BANDS
   0x00000000u,			// MASTER_GAIN
   0x0000000Du,			// STAGE1_TYPE
   0x00020000u,			// STAGE1_FC
   0x00000000u,			// STAGE1_GAIN
   0x00B504F3u,			// STAGE1_Q
   0x0000000Du,			// STAGE2_TYPE
   0x00040000u,			// STAGE2_FC
   0x00000000u,			// STAGE2_GAIN
   0x00B504F3u,			// STAGE2_Q
   0x0000000Du,			// STAGE3_TYPE
   0x0007D000u,			// STAGE3_FC
   0x00000000u,			// STAGE3_GAIN
   0x00B504F3u,			// STAGE3_Q
   0x0000000Du,			// STAGE4_TYPE
   0x000FA000u,			// STAGE4_FC
   0x00000000u,			// STAGE4_GAIN
   0x00B504F3u,			// STAGE4_Q
   0x0000000Du,			// STAGE5_TYPE
   0x001F4000u,			// STAGE5_FC
   0x00000000u,			// STAGE5_GAIN
   0x00B504F3u,			// STAGE5_Q
   0x0000000Du,			// STAGE6_TYPE
   0x003E8000u,			// STAGE6_FC
   0x00000000u,			// STAGE6_GAIN
   0x00B504F3u,			// STAGE6_Q
   0x0000000Du,			// STAGE7_TYPE
   0x007D0000u,			// STAGE7_FC
   0x00000000u,			// STAGE7_GAIN
   0x00B504F3u,			// STAGE7_Q
   0x0000000Du,			// STAGE8_TYPE
   0x00FA0000u,			// STAGE8_FC
   0x00000000u,			// STAGE8_GAIN
   0x00B504F3u,			// STAGE8_Q
   0x0000000Du,			// STAGE9_TYPE
   0x01F40000u,			// STAGE9_FC
   0x00000000u,			// STAGE9_GAIN
   0x00B504F3u,			// STAGE9_Q
   0x0000000Du,			// STAGE10_TYPE
   0x03E80000u,			// STAGE10_FC
   0x00000000u,			// STAGE10_GAIN
   0x00B504F3u			// STAGE10_Q
};

unsigned *PEQ_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0049: return defaults_peqPEQ;
		case 0x405C: return defaults_peqPEQ;
	}
	return((unsigned *)0);
}

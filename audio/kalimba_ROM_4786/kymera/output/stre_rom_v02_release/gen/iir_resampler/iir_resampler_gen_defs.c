// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "iir_resampler_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_iir_resamplerIIRV2[] = {
   0x00002080u			// CONFIG
};

unsigned *IIR_RESAMPLER_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0094: return defaults_iir_resamplerIIRV2;
		case 0x4053: return defaults_iir_resamplerIIRV2;
	}
	return((unsigned *)0);
}

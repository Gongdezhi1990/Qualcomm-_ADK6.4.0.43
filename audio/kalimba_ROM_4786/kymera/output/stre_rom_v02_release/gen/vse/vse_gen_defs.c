// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "vse_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_vseVSE[] = {
   0x00000000u,			// VSE_CONFIG
   0x00000000u,			// BINAURAL_FLAG
   0x0CCCCCCDu,			// SPEAKER_SPACING
   0x0000003Cu			// VIRTUAL_ANGLE
};

unsigned *VSE_GetDefaults(unsigned capid){
	switch(capid){
		case 0x00004A: return defaults_vseVSE;
		case 0x406A: return defaults_vseVSE;
	}
	return((unsigned *)0);
}

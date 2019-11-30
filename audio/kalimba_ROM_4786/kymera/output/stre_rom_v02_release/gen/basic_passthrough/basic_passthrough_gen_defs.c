// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "basic_passthrough_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_basic_passthroughTTP[] = {
   0x00002080u,			// CONFIG
   0x00000000u			// GAIN
};

unsigned *BASIC_PASSTHROUGH_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0001: return defaults_basic_passthroughTTP;
		case 0x4002: return defaults_basic_passthroughTTP;
		case 0x003C: return defaults_basic_passthroughTTP;
		case 0x4067: return defaults_basic_passthroughTTP;
	}
	return((unsigned *)0);
}

// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "mixer_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_mixerCMX[] = {
   0x00002080u			// CONFIG
};

unsigned *MIXER_GetDefaults(unsigned capid){
	switch(capid){
		case 0x000A: return defaults_mixerCMX;
		case 0x405B: return defaults_mixerCMX;
		case 0x0097: return defaults_mixerCMX;
	}
	return((unsigned *)0);
}

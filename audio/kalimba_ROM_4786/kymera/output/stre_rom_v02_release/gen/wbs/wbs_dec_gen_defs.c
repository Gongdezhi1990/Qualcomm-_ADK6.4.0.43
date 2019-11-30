// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "wbs_dec_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_wbs_decWB[] = {
   0x00000000u			// FORCE_PLC_OFF
};

unsigned *WBS_DEC_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0006: return defaults_wbs_decWB;
		case 0x4064: return defaults_wbs_decWB;
	}
	return((unsigned *)0);
}

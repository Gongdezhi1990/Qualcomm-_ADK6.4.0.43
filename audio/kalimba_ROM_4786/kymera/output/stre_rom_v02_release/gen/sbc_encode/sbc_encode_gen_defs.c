// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "sbc_encode_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_sbc_encodeSBCENC[] = {
   0x00002080u			// CONFIG
};

unsigned *SBC_ENCODE_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0014: return defaults_sbc_encodeSBCENC;
		case 0x4060: return defaults_sbc_encodeSBCENC;
	}
	return((unsigned *)0);
}

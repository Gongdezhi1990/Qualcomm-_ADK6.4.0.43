// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "sbc_decode_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_sbc_decodeSBCDEC[] = {
   0x00002080u			// CONFIG
};

unsigned *SBC_DECODE_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0016: return defaults_sbc_decodeSBCDEC;
		case 0x405F: return defaults_sbc_decodeSBCDEC;
	}
	return((unsigned *)0);
}

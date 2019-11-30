// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "aac_decode_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_aac_decodeAACD[] = {
   0x00002080u			// CONFIG
};

unsigned *AAC_DECODE_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0018: return defaults_aac_decodeAACD;
		case 0x404F: return defaults_aac_decodeAACD;
	}
	return((unsigned *)0);
}

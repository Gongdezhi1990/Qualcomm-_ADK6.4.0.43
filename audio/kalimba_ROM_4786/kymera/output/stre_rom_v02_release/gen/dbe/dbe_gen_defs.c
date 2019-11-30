// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "dbe_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_dbeDBE[] = {
   0x00000006u,			// DBE_CONFIG
   0x00000032u,			// EFFECT_STRENGTH
   0x00000000u,			// AMP_LIMIT
   0x00064000u,			// FC_LP
   0x00064000u,			// FC_HP
   0x00000064u,			// HARM_CONTENT
   0x000C8000u,			// XOVER_FC
   0x00000032u			// MIX_BALANCE
};
#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_dbeDBE_FULLBAND[] = {
   0x00000000u,			// DBE_CONFIG
   0x00000032u,			// EFFECT_STRENGTH
   0x00000000u,			// AMP_LIMIT
   0x00064000u,			// FC_LP
   0x00064000u,			// FC_HP
   0x00000064u,			// HARM_CONTENT
   0x000C8000u,			// XOVER_FC
   0x00000032u			// MIX_BALANCE
};
#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_dbeDBE_FULLBAND_BASSOUT[] = {
   0x00000004u,			// DBE_CONFIG
   0x00000032u,			// EFFECT_STRENGTH
   0x00000000u,			// AMP_LIMIT
   0x00064000u,			// FC_LP
   0x00064000u,			// FC_HP
   0x00000064u,			// HARM_CONTENT
   0x000C8000u,			// XOVER_FC
   0x00000032u			// MIX_BALANCE
};

unsigned *DBE_GetDefaults(unsigned capid){
	switch(capid){
		case 0x002f: return defaults_dbeDBE;
		case 0x4058: return defaults_dbeDBE;
		case 0x0090: return defaults_dbeDBE_FULLBAND;
		case 0x4059: return defaults_dbeDBE_FULLBAND;
		case 0x0091: return defaults_dbeDBE_FULLBAND_BASSOUT;
		case 0x405A: return defaults_dbeDBE_FULLBAND_BASSOUT;
	}
	return((unsigned *)0);
}

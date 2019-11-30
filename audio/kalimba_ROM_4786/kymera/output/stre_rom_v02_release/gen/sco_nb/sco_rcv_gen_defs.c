// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#include "sco_rcv_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_sco_rcvNB[] = {
   0x00000000u			// FORCE_PLC_OFF
};

unsigned *SCO_RCV_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0004: return defaults_sco_rcvNB;
		case 0x4062: return defaults_sco_rcvNB;
	}
	return((unsigned *)0);
}

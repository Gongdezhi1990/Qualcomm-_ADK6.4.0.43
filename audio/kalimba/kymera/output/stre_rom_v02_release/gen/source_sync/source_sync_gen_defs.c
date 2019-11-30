// -----------------------------------------------------------------------------
// Copyright (c) 2019                  Qualcomm Technologies International, Ltd.
//
#include "source_sync_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_source_syncSRC_SYNC[] = {
   0x40000000u,			// SS_PERIOD
   0x08000000u,			// SS_MAX_PERIOD
   0x14000000u,			// SS_MAX_LATENCY
   0x08000000u,			// SS_OUTPUT_SPACE
   0x08000000u,			// SS_STALL_RECOVERY_DEFAULT_FILL
   0x0CCCCCCDu			// SS_STALL_RECOVERY_CATCHUP_LIMIT
};

unsigned *SOURCE_SYNC_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0099: return defaults_source_syncSRC_SYNC;
		case 0x408C: return defaults_source_syncSRC_SYNC;
	}
	return((unsigned *)0);
}

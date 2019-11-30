/*
Copyright (c) 2016 Qualcomm Technologies International, Ltd.
*/

#include "sink_watchdog.h"
#include "sink_main_task.h"
#include "sink_events.h"

#include <vm.h>

#ifdef ENABLE_WATCHDOG
void watchdogReset(void)
{
	VmSoftwareWdKick(WD_TIMEOUT_SECONDS);
	MessageSendLater( &theSink.task , EventSysResetWatchdog , 0 ,D_SEC(WD_KICK_TIME_SECONDS));
}
#endif

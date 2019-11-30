/*
Copyright (c) 2016 Qualcomm Technologies International, Ltd.
*/

#ifndef _SINK_WATCHDOG_H_
#define _SINK_WATCHDOG_H_

#ifdef ENABLE_WATCHDOG
void watchdogReset(void);
#else
#define watchdogReset() {}
#endif

#define WD_TIMEOUT_SECONDS        40
#define WD_KICK_TIME_SECONDS      10


#endif /* _SINK_WATCHDOG_H_ */

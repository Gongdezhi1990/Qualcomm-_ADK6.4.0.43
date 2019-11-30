/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

*/
/*!
@file system_clock.h
@brief Interface to the system clock.

*/

#ifndef SYSTEM_CLOCK_H_
#define SYSTEM_CLOCK_H_

#include <rtime.h>
/****************************************************************************
DESCRIPTION
    Returns the local 1us system clock (TIMER_TIME).
*/
rtime_t SystemClockGetTimerTime(void);

#endif

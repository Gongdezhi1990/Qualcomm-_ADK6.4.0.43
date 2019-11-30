/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    system_clock.c
*/

#include <system_clock.h>
#include <vm.h>

rtime_t SystemClockGetTimerTime(void)
{
    return (rtime_t)VmGetTimerTime();
}

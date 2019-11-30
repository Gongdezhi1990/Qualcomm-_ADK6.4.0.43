/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_synchronised_control.c
*/

#include <stdlib.h>
#include <rtime.h>
#include <panic.h>
#include <system_clock.h>

#include "tws_synchronised_control.h"

#define SYSTEM_CLOCKS_PER_MILLISECOND      ((rtime_t)1000)

static wallclock_state_t * avrcp_wallclock = NULL;

static rtime_t millisecondsToSystemTime(uint16 milliseconds)
{
    return (milliseconds * SYSTEM_CLOCKS_PER_MILLISECOND);
}

static int32 systemTimeToMilliseconds(int32 system_time)
{
    return (system_time / (int32)SYSTEM_CLOCKS_PER_MILLISECOND);
}

void twsSynchronisedControlEnable(Sink avrcp_sink)
{
    if(avrcp_wallclock || !avrcp_sink)
    {
        Panic();
    }
    avrcp_wallclock = PanicUnlessMalloc(sizeof(wallclock_state_t));
    
    RtimeWallClockEnable(avrcp_wallclock, avrcp_sink);
}

void twsSynchronisedControlDisable(void)
{
    if(avrcp_wallclock)
    {
        RtimeWallClockDisable(avrcp_wallclock);
        free(avrcp_wallclock);
        avrcp_wallclock = NULL;
    }
}

bool twsSynchronisedControlIsEnabled(void)
{
    if(avrcp_wallclock)
    {
        return TRUE;
    }
    return FALSE;
}

tws_timestamp_t twsSynchronisedControlGetFutureTimestamp(uint16 milliseconds_in_future)
{
    rtime_t timestamp;
    if(RtimeLocalToWallClock(avrcp_wallclock, SystemClockGetTimerTime(), &timestamp))
    {
        return (tws_timestamp_t)rtime_add(timestamp, millisecondsToSystemTime(milliseconds_in_future));
    }
    return 0;
}

int32 twsSynchronisedControlConvertTimeStampToMilliseconds(tws_timestamp_t timestamp)
{
    rtime_t localtime = 0;
    if(RtimeWallClockToLocal(avrcp_wallclock, timestamp, &localtime))
    {
        return systemTimeToMilliseconds(RtimeTimeBeforeTTP(localtime));
    }
    return 0;
}

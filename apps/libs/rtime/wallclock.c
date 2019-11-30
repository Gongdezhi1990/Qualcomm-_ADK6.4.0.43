/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    wallclock.c
*/

#include <stream.h>
#include <sink.h>
#include <panic.h>
#include <vmtypes.h>
#include <system_clock.h>
#include "rtime.h"

/* The number of milli-seconds between each update of the wallclock */
#define WALLCLOCK_UPDATE_MS 50

#define WALLCLOCK_UPDATE_MSG_ID 0

static void wallclockSendUpdateMsg(Task task)
{
    MessageSendLater(task, WALLCLOCK_UPDATE_MSG_ID, NULL, WALLCLOCK_UPDATE_MS);
}

/* Convert BT clock (in 1/2 slots) to microseconds */
static rtime_t btclock_to_rtime(uint32 btclock)
{
    rtime_t btclock_us = (btclock / 2) * US_PER_SLOT;
    return (btclock & 1) ? (btclock_us + HALF_SLOT_US) : btclock_us;
}

/* Update the offset between wall clock and local clock */
static bool wallclockUpdate(wallclock_state_t *state)
{
    if (state)
    {
        bt_wallclock_info btwci;
        if (SinkGetWallclock(state->sink, &btwci))
        {
            rtime_t wallclock = btclock_to_rtime(btwci.btclock);
            state->offset = rtime_sub(wallclock, btwci.timestamp);
            return TRUE;
        }
    }
    return FALSE;
}

static void wallclockMessageHandler(Task task, MessageId id, Message message)
{
    wallclock_state_t *state = (wallclock_state_t *)task;
    UNUSED(message);
    switch (id)
    {
        case WALLCLOCK_UPDATE_MSG_ID:
            if (wallclockUpdate(state))
            {
                wallclockSendUpdateMsg(&state->task);
            }
            else
            {
                /* Invalidate sink so further conversions fail */
                state->sink = NULL;
            }
            break;

        default:
            break;
    }
}

bool RtimeWallClockEnable(wallclock_state_t *state, Sink sink)
{
    if (state && SinkIsValid(sink))
    {
        state->sink = sink;
        state->task.handler = wallclockMessageHandler;
        SinkConfigure(sink, STREAM_WALLCLOCK_ENABLE, 1);
        if (wallclockUpdate(state))
        {
            wallclockSendUpdateMsg(&state->task);
            return TRUE;
        }
        SinkConfigure(sink, STREAM_WALLCLOCK_ENABLE, 0);
    }
    return FALSE;
}

bool RtimeWallClockGetStateForSink(wallclock_state_t *state, Sink sink)
{
    bool result = FALSE;
    if (state)
    {
        state->sink = sink;
        result = wallclockUpdate(state);
        if (!result)
        {
            if (SinkConfigure(sink, STREAM_WALLCLOCK_ENABLE, 1))
            {
                result = wallclockUpdate(state);
                /* Leave the wallclock enabled for this sink to avoid starting
                   and stopping the wallclock service each time this function
                   is called. P0 will clean up for us when the sink is gone */
            }
        }
    }
    return result;
}

bool RtimeWallClockDisable(wallclock_state_t *state)
{
    if (state)
    {
        /* The P0 will automatically disable wall clock when a sink is gone.
           Thus SinkConfigure shouldn't be used here because it can result in
           attempt to configure not existing sink.
           Checking SinkIsValid() or SinkGetWallclock() won't help here.*/

        state->sink = NULL;
        state->task.handler = NULL;
        MessageCancelAll(&state->task, WALLCLOCK_UPDATE_MSG_ID);
        return TRUE;
    }
    return FALSE;
}

/* This function may be called from high priority task handler and must only call
   fast or non blocking traps */
bool RtimeLocalToWallClock(wallclock_state_t *state, rtime_t local, rtime_t *wallclock)
{
    if (state && state->sink)
    {
        *wallclock = rtime_add(local, state->offset);
        return TRUE;
    }
    return FALSE;
}

/* This function may be called from high priority task handler and must only call
   fast or non blocking traps */
bool RtimeLocalToWallClock24(wallclock_state_t *state, rtime_t local, rtime24_t *wallclock)
{
    rtime_t wallclock32;
    if (RtimeLocalToWallClock(state, local, &wallclock32))
    {
        *wallclock = rtime_to_rtime24(wallclock32);
        return TRUE;
    }
    return FALSE;
}


/* This function may be called from high priority task handler and must only call
   fast or non blocking traps */
bool RtimeWallClockToLocal(wallclock_state_t *state, rtime_t wallclock, rtime_t *local)
{
    if (state && state->sink)
    {
        *local = rtime_sub(wallclock, state->offset);
        return TRUE;
    }
    return FALSE;
}

/* This function may be called from high priority task handler and must only call
   fast or non blocking traps */
bool RtimeWallClock24ToLocal(wallclock_state_t *state, rtime24_t wallclock24, rtime_t *local)
{
    /* Include error check here, so no need to check return values */
    if (state && state->sink)
    {
        rtime_t wallclock32;
        rtime_t wallclock_current;
        RtimeLocalToWallClock(state, SystemClockGetTimerTime(), &wallclock_current);
        RtimeWallClockExtend(state, (rtime_t)wallclock24, 24, wallclock_current, &wallclock32);
        RtimeWallClockToLocal(state, wallclock32, local);
        return TRUE;
    }
    return FALSE;
}


bool RtimeWallClockExtend(wallclock_state_t *state, rtime_t wallclock,
                          uint32 wallclock_bits, rtime_t wallclock_current,
                          rtime_t *extended)
{
    if (state && state->sink)
    {
        uint32 shift = 32 - wallclock_bits;
        rtime_t mask = (~0UL) >> shift;

        /* Calculate the difference between the given wallclock and
           the masked wallclock, then signed extend the result to 32-bits */
        int32 diff = rtime_sub(wallclock, wallclock_current & mask);
        diff *= (1 << shift);
        diff /= (1 << shift);

        /* Calculate the extended wallclock */
        *extended = rtime_add(wallclock_current, diff);

        return TRUE;
    }
    return FALSE;
}

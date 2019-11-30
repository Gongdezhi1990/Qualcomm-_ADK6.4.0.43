/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    rtime.c
*/

#include <sink.h>
#include <vm.h>
#include <vmtypes.h>
#include <panic.h>
#include <stream.h>
#include <system_clock.h>
#include "rtime.h"

/* The sample period in us. The sample period is represented as
   Q32.32 in a signed 64 bit integer */
typedef signed long long sp_t;
#define S_TO_US 1000000ULL
#define ONE 1LL

/* The number of bits shift to the decimal point in the sp_t */
#define QSHIFT (sizeof(sp_t) * CHAR_BIT / 2)
/* Divide a sp_t by QDIV to obtain the integer part of the type */
#define QDIV (ONE << QSHIFT)
/* Mask a sp_t by QMASK to obtain the fractional part of the type */
#define QMASK (QDIV - ONE)

/* Convert a sample rate to a sample period in us */
#define SR_TO_SP(sr) ((S_TO_US << QSHIFT) / (sr))

/* Sample period lookup table */
typedef struct __sr_sp_lut
{
    /* The sample rate, e.g. 44100, 48000 */
    uint32 sr;
    /* The equivalent sample period in us */
    sp_t sp;
} sr_sp_lut_t;

#define SR_SP_LUT_ITEM(sr) {(sr), SR_TO_SP((sr))}

static const sr_sp_lut_t sr_sp_lut[] = {SR_SP_LUT_ITEM(44100),
                                        SR_SP_LUT_ITEM(48000)};

/* Number of bits in the full spadj */
#define SPADJ_BITS (sizeof(rtime_spadj_t) * CHAR_BIT)
/* Number of bits in the mini spadj */
#define SPADJ_MINI_BITS 8
/* Bit position of start of slice (down from msbit) */
#define SPADJ_SLICE 7
/* Division / multiplication factor used to convert between full/mini spadj */
#define SPADJ_CONVERSION (1 << (SPADJ_BITS - SPADJ_SLICE - SPADJ_MINI_BITS))

rtime_spadj_mini_t RtimeSpadjFullToMini(rtime_spadj_t s)
{
    return (s / SPADJ_CONVERSION);
}

rtime_spadj_t RtimeSpadjMiniToFull(rtime_spadj_mini_t s)
{
    /* Sign extend and convert */
    return (rtime_spadj_t)s * SPADJ_CONVERSION;
}

/* Multiply a sample period by a sample period adjustment, returning
   a new adjusted sample period: s + (s * adj). */
static sp_t adjustedSamplePeriod(rtime_sample_rate_t sr, rtime_spadj_t adj)
{
    sp_t s, tmp1, tmp2, res;

    PanicFalse(sr < rtime_sample_rate_end);

    /* Read the sample period from the LUT */
    s = sr_sp_lut[sr].sp;

    /* Multiply adj by lower half of s, retaining only the upper half of
       the result through division by QDIV */
    tmp1 = ((s & QMASK) * adj) / QDIV;

    /* Multiply adj by upper half of s */
    tmp2 = (s / QDIV) * adj;

    /* Result is twice their sum. Twice, because adj is a signed value - the
       sign bit makes the adj fraction half the size. */
    res = (tmp1 + tmp2) * 2;

    return s + res;
}

rtime_t RtimeSamplesToTime(int32 samples, rtime_sample_rate_t sr, rtime_spadj_t spadj)
{
    /* Multiply the number of samples by the adjusted sample period to get the
       time taken to play those samples.
       The QDIV retrieves the upper half, which is the integer part */
    return (samples * adjustedSamplePeriod(sr, spadj)) / QDIV;
}

int32 RtimeTimeBeforeTTP(rtime_t ttp)
{
    rtime_t timer_time = SystemClockGetTimerTime();
    int32 ttottp = rtime_sub(ttp, timer_time);
    return ttottp;
}

int32 RtimeTimeBeforeTx(rtime_t ttp, rtime_t time_before_ttp_to_tx)
{
    int32 ttottp = RtimeTimeBeforeTTP(ttp);
    int32 ttotx = rtime_sub(ttottp, time_before_ttp_to_tx);
    return ttotx;
}

rtime_sample_rate_t RtimeSampleRateConvert(uint32 sample_rate)
{
    switch (sample_rate)
    {
        case 44100:
            return rtime_sample_rate_44100;
        case 48000:
            return rtime_sample_rate_48000;
        default:
            return rtime_sample_rate_unknown;
    }
}

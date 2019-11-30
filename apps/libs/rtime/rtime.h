/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

*/
/*!
@file 
@brief Interface to the rtime library.

A library that provides functionality to help other modules understand and
utilise time, wall clocks, sample rates, sample periods, sample period
adjustments etc.

@b wallclock

The wallclock provided by this module is a representation of time that can
be used at both ends of a connection, with a resolution of &mu;seconds.
Each end of the link will be synchronised within a few &mu;seconds. The
exact accuracy depends on the rate of clock drift between the two devices. 
If connected to multiple devices, multiple wallclocks must be used.

Operations on wallclock times should always use the rtime_ macros in this
file, which correctly deal with any wrapping of values.

Any time values sent between devices should be converted as follow
@code
    // Each end of the link should call this function after
    // the link is established.
    RtimeWallClockEnable(&wallc, my_sink);

    // Convert a local time to a value to send
    RtimeLocalToWallClock(&wallc, localtime_to_send, &time_over_air);

    ....send the time

    // Convert the received value to the local time at this end
    RtimeWallClockToLocal(&wallc, time_from_air, &localtime_received);
@endcode

*/

#ifndef RTIME_H__
#define RTIME_H__

#include <csrtypes.h>

/** System time, in microseconds.
    The fundamental type "rtime_t" is really a uint32. rtime_t values will
    *wrap* after around 4295 seconds, so functions using rtime_ts must be
    cautious. */
typedef uint32 rtime_t;

/** 24 bit value that may be used to represent time to play information
    being transmitted between two devices.
    Function are provided to convert this to and from local time using
    a wallclock.
    In most situations \ref rtime_t should be used. This is 32 bits. */
typedef uint32 rtime24_t;


/** Enumerate the sample rates used in the system (in particular by the
  packetisation libraries. */
typedef enum __rtime_sample_rate
{
    rtime_sample_rate_44100,
    rtime_sample_rate_48000,
    /*! End of valid sample rates */
    rtime_sample_rate_end,
    /*! The sample rate is unknown */
    rtime_sample_rate_unknown
} rtime_sample_rate_t;

/** The mini sample period adjustment is a signed 8-bit slice of the full sample
  period adjustment. */
typedef int8 rtime_spadj_mini_t;

/** The full sample period adjustment is a signed fraction represented in Q31 format. */
typedef int32 rtime_spadj_t;

/** Wallclock state. Create and enable a wallclock state for each Sink that
    requires conversion to/from wallclock/local time. */
typedef struct __wallclock_state
{
    /** The task which periodically reads the wallclock */
    TaskData task;
    /** The sink for which the wallclock is read */
    Sink sink;
    /** The offset between the local time and the wallclock */
    int32 offset;
} wallclock_state_t;

/** The number of microseconds per millisecond. */
#define US_PER_MS         ((rtime_t) 1000)
/** The number of milliseconds per second. */
#define MS_PER_SEC        ((rtime_t) 1000)
/** The number of microseconds per second. */
#define US_PER_SEC        (US_PER_MS * MS_PER_SEC)

/** The number of microseconds per Bluetooth slot */
#define US_PER_SLOT       ((rtime_t) 625)

/** The number of microseconds in half a Bluetooth slot.
    Rounded up because it's not an integral number of us. */
#define HALF_SLOT_US ((US_PER_SLOT + 1) / 2)

/**
 * \brief  Add two time values
 *
 * \returns The sum of "t1 and t2".
 *
 * NOTES
 *  Implemented as a macro, because it's trivial.
 *
 *  Adding the numbers can overflow the range of a rtime_t, so the user must
 *  be cautious.
 */
#define rtime_add(t1, t2) ((t1) + (t2))


/**
 * \brief  Subtract two time values
 *
 * \returns t1 - t2.
 *
 * Implemented as a macro, because it's trivial.
 *
 * Subtracting the numbers can provoke an underflow.   This returns
 * a signed number for correct use in comparisons.
 *
 * If you want to know whether the time since last timestamp has exceeded
 * some threshold value, don't be tempted to use this:
 *
 * if((uint32) rtime_sub(current_time, last_time) > some_threshold);
 *
 * or any other such variant. This may give wrong result.
 *
 * The correct way to express the above is using rtime_gt macro:
 *
 * if (rtime_gt(rtime_sub(current_time, last_time), threshold_time));
 * or, equivalently:
 * if (rtime_gt(current_time, rtime_add(last_time, threshold_time));
 */
#define rtime_sub(t1, t2) ((int32) (t1) - (int32) (t2))


/**
 * \brief  Compare two time values
 *
 * \returns TRUE if "t1" equals "t2", else FALSE.
 *
 * Compares the two time values "t1" and "t2".
 * Implemented as a macro, because it's trivial.
 *
 */
#define rtime_eq(t1, t2) ((t1) == (t2))


/**
 * \brief  Compare two time values
 *
 * \returns FALSE if "t1" equals "t2", else TRUE.
 *
 * Compares the two time values "t1" and "t2".
 * Implemented as a macro, because it's trivial.
 *
 */
#define rtime_ne(t1, t2) ((t1) != (t2))


/**
 * \brief  Compare two time values
 *
 * \returns TRUE if "t1" is greater than "t2", else FALSE.
 *
 * Compares the time values "t1" and "t2".
 *
 * Because time values wrap, "t1" and "t2" must differ by less than half
 * the range of the clock apart.
 * Implemented as a macro, because it's trivial.
 *
 */
#define rtime_gt(t1, t2) (rtime_sub((t1), (t2)) > 0)


/**
 * \brief  Compare two time values
 *
 * \returns TRUE if "t1" is greater than, or equal to, "t2", else FALSE.
 *
 * Compares the time values "t1" and "t2".
 *
 * Because time values wrap, "t1" and "t2" must differ by less than half
 * the range of the clock apart.
 * Implemented as a macro, because it's trivial.
 *
 */
#define rtime_ge(t1, t2) (rtime_sub((t1), (t2)) >= 0)


/**
 * \brief  Compare two time values
 *
 * \returns TRUE if "t1" is less than "t2", else FALSE.
 *
 * Compares the time values "t1" and "t2".
 *
 * Because time values wrap "t1" and "t2" must be less than half the
 * range of the clock apart.
 * Implemented as a macro, because it's trivial.
 *
 */
#define rtime_lt(t1, t2) (rtime_sub((t1), (t2)) < 0)


/**
 * \brief  Compare two time values
 *
 * \returns TRUE if "t1" is less than, or equal to, "t2", else FALSE.
 *
 * Compares the time values "t1" and "t2".
 *
 * Because time values wrap "t1" and "t2" must be less than half the
 * range of the clock apart.
 * Implemented as a macro, because it's trivial.
 *
 */
#define rtime_le(t1, t2) (rtime_sub((t1), (t2)) <= 0)

/**
 * \brief  Minimum of two time values
 *
 * \returns Minimum of t1 and t2
 *
 * Implemented as a macro, because it's trivial.
 */
#define rtime_min(t1, t2) (rtime_lt(t1, t2) ? (t1) : (t2))

/**
 * \brief  Maximum of two time values
 *
 * \returns Maximum of t1 and t2
 *
 * Implemented as a macro, because it's trivial.
 */
#define rtime_max(t1, t2) (rtime_gt(t1, t2) ? (t1) : (t2))

/**
 * \brief  Convert an \ref rtime_t value to \ref rtime24_t
 *
 * \returns The \ref rtime24_t variant of the supplied value
 *
 * Implemented as a macro, because it's trivial.
 */
#define rtime_to_rtime24(t) ((rtime24_t)(t & 0xFFFFFF))

/**
 * \brief Calculate the time in microseconds before the time to play.
 * \param ttp The local-clock based time-to-play (in microseconds).
 * \return The time before the ttp, positive if the TTP is in the future,
 * negative if the TTP is in the past.
 */
int32 RtimeTimeBeforeTTP(rtime_t ttp);

/**
 * \brief Calculate the time in microseconds before the frame must be
 * transmitted.
 * \param ttp The frame's local-clock based time-to-play (in microseconds).
 * \param time_before_ttp_to_tx The time before the frame's ttp to transmit the
 * frame (in microseconds).
 * \return The time before the time to transmit. Positive if the time to transmit
 * is in the future, negative if in the past.
 */
int32 RtimeTimeBeforeTx(rtime_t ttp, rtime_t time_before_ttp_to_tx);

/**
 * \brief Enable the wallclock for the sink.
 * \param state The wallclock state.
 * \param sink The sink.
 * \return TRUE if the wallclock was enabled for the sink, otherwise FALSE.
 * Enabling the wallclock starts a background task that periodically calculates
 * the offset between the local system clock and the wallclock. Once the
 * wallclock is enabled for a sink, the functions #RtimeLocalToWallClock and
 * #RtimeWallClockToLocal may be called.
 */
bool RtimeWallClockEnable(wallclock_state_t *state, Sink sink);

/**
 * \brief Disable the wallclock for the sink.
 * \param state The wallclock state.
 * \return TRUE if the wallclock was disabled, otherwise FALSE.
 * The functions #RtimeLocalToWallClock and #RtimeWallClockToLocal may not be
 * called after the wallclock is disabled.
 */
bool RtimeWallClockDisable(wallclock_state_t *state);

/**
 * \brief Populate the state for a sink.
 * \param state The wallclock state.
 * \param sink The sink.
 * \return TRUE if the state was correctly populated, otherwise FALSE.
 * Does not start a background task. The state can be used for timestamp
 * conversion using the functions #RtimeLocalToWallClock and
 * #RtimeWallClockToLocal.
 */
bool RtimeWallClockGetStateForSink(wallclock_state_t *state, Sink sink);

/**
 * \brief Convert from local time to wall-clock time.
 * \param state The wall-clock state.
 * \param local The local time.
 * \param wallclock [OUT] The converted wall-clock time.
 * \return TRUE if the conversion was successful, FALSE otherwise.
 */
bool RtimeLocalToWallClock(wallclock_state_t *state, rtime_t local, rtime_t *wallclock);

/**
 * \brief Convert from local time to a 24 bit wall-clock time.
 * \param state The wall-clock state.
 * \param local The local time.
 * \param wallclock [OUT] The converted wall-clock time.
 * \return TRUE if the conversion was successful, FALSE otherwise.
 */
bool RtimeLocalToWallClock24(wallclock_state_t *state, rtime_t local, rtime24_t *wallclock);

/**
 * \brief Convert from wall-clock time to local time.
 * \param state The wall-clock state.
 * \param wallclock The 32-bit wall-clock time.
 * \param local [OUT] The converted local time.
 * \return TRUE if the conversion was successful, FALSE otherwise.
 */
bool RtimeWallClockToLocal(wallclock_state_t *state, rtime_t wallclock, rtime_t *local);

/**
 * \brief Convert from a 24 bit wall-clock time to local time.
 * \param state The wall-clock state.
 * \param wallclock The 24-bit wall-clock time.
 * \param local [OUT] The converted local time.
 * \return TRUE if the conversion was successful, FALSE otherwise.
 */
bool RtimeWallClock24ToLocal(wallclock_state_t *state, rtime24_t wallclock, rtime_t *local);


/**
 * \brief Extend a wall-clock to a full (32-bit) rtime_t.
 * \param state The wall-clock state.
 * \param wallclock The wall-clock time to be extended.
 * \param wallclock_bits The nunber of bits in the wallclock.
 * \param wallclock_current The current value of the wallclock.
 * \param extended [OUT] The extended local time.
 * \return TRUE if the conversion was successful, FALSE otherwise.
 * The wallclock will be extended from wallclock_bits length to 32-bits
 * using the wallclock_current as a reference.
 */
bool RtimeWallClockExtend(wallclock_state_t *state,
                          rtime_t wallclock,
                          uint32 wallclock_bits,
                          rtime_t wallclock_current,
                          rtime_t *extended);

/**
 * \brief Convert a mini sample period adjustment to a full sample period
 * adjustment.
 */
rtime_spadj_t RtimeSpadjMiniToFull(rtime_spadj_mini_t s);

/**
 * \brief Convert a full sample period adjustment to a mini sample period
 * adjustment. Note: this conversion results in a loss of precision.
 */
rtime_spadj_mini_t RtimeSpadjFullToMini(rtime_spadj_t s);

/**
 * \brief Convert a number of samples at the defined sample rate and sample
 * period adjustment, to time.
 */
rtime_t RtimeSamplesToTime(int32 samples, rtime_sample_rate_t sr, rtime_spadj_t spadj);

/**
 * \brief Convert a integer sample rate e.g. 44100, 48000, to a rtime_sample_rate_t
 * \param sample_rate The integer sample rate.
 * \return The converted rate, or rtime_sample_rate_unknown if unknown.
 */
rtime_sample_rate_t RtimeSampleRateConvert(uint32 sample_rate);

#endif  /* RTIME_H__ */

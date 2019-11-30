/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Hardware abstraction layer for the timer hardware
 *
 * Control of the timer hardware.
*/

#ifndef HALTIME_H
#define HALTIME_H

#include "hydra/hydra_types.h"
#ifndef DESKTOP_TEST_BUILD
/* We don't want to pull io_defs into the namespace for desktop tests if we
 * don't have to */
#ifndef IO_DEFS_MODULE_K32_TIMERS
#define IO_DEFS_MODULE_K32_TIMERS
#endif
#include "io/io.h"
#endif

#define MICROSECOND (1L)
#define MILLISECOND (1000L * MICROSECOND)
#define SECOND      (1000L * MILLISECOND)
#define MINUTE      (60L   * SECOND)
/*@}*/
/**
 * Initialise the timer event generation
 *
 * Initialises the event timer and cancel any scheduled events.
 *
 * This function must be called by each processor following a software
 * boot.
 */
extern void init_hal_local_timer(void);

/**
 * Read the current system time
 *
 * Returns the current value of the system's 32 bit 1MHz clock. The
 * resolution of the returned int32 is one microsecond, so the system's
 * clock wraps after approximately 71 minutes.
 *
 * This clock is the basis of all timed events in the chip's hardware,
 * notably other functions declared in this file.
 *
 * The function can be called from the machine's background or from
 * interrupt routines.
 *
 * This essentially just reads TIMER_TIME. However, this requires two
 * separate 16 bit word reads, and there is a risk of a carry occuring
 * between the two reads. This function encapsulates the complexity of
 * obtaining a consistent value.
 *
 * Returns the current system time.
 */
extern uint32 hal_get_time(void);

/**
 * Read the least significant word of current system time
 *
 * Returns the least significant word of the current value of the
 * system's 32 bit 1MHz clock. The resolution of the returned uint16
 * is one microsecond, so this wraps after approximately 65
 * milliseconds.
 *
 * The macro can be called from the machine's background or from
 * interrupt routines.
 *
 * Returns the least significant word of the current system time.
 */
extern uint16 hal_get_time_lsw(void);

/**
 * Schedule a timer event
 *
 * This function sets the chip's timer to
 * provoke a "timer event" interrupt after "at" microseconds.
 *
 * The hardware treats "at" as a signed 32 bit number.
 * If the hardware finds "at" to be in the past
 * then the event fires immediately.
 */
extern void hal_post_timer1_event(uint32 at);
extern void hal_post_timer2_event(uint32 at);
extern void hal_post_timer3_event(uint32 at);

/**
 * Cancel a scheduled timer event
 *
 * Cancels a timer event.
 */
extern void hal_cancel_timer1_event(void);
extern void hal_cancel_timer2_event(void);
extern void hal_cancel_timer3_event(void);



/**
 * hal_connect_timer_pioX  -  configure timer driven PIO event
 *
 * Configure the timer driven PIO to drive a pulse starting at "start" of
 * duration "period". If "period" is 0 then the pulse does not end, i.e.
 * only a single transition is generated. If "invert" is FALSE then a
 * positive going pulse will be produced, otherwise a negative going pulse
 * will be produced.
 *
 * The state of the PIO is derived from the configuration and current time;
 * there is no hyseresis or state. This means that the PIO should be
 * disconnected from the timer if no further pulses are required, otherwise
 * an additional pulse will be generated each time the timer wraps.
 *
 * Note that if "start" is being changed from the last value set then this
 * temporarily disconnects the PIO from the timer. To avoid glitches on
 * the output, the state of the PIO should be set appropriately using
 * hal_set_output_pio() before calling this function.
 */
extern void hal_connect_timer_pio1(int32 start, uint16 period, bool invert);
extern void hal_connect_timer_pio2(int32 start, uint16 period, bool invert);
extern void hal_connect_timer_pio3(int32 start, uint16 period, bool invert);

/**
 * Disconnect PIO from timer harware
 *
 * Disconnect the PIO from the timer hardware. The PIO will return to the
 * state configured via hal_set_output_pio().
 */
extern void hal_disconnect_timer_pio1(void);
extern void hal_disconnect_timer_pio2(void);
extern void hal_disconnect_timer_pio3(void);

/**
 * Runs a delay loop
 *
 * A delay loop generally used to allow the LO/ADCs to settle during
 * calibration/trimming.  Likely to be used by various trimming
 * routines.  The time that this function takes depends on the
 * current clokc speed and is therefore variable.  hal_delay_us is a
 * better choice.
 */
extern void hal_delay(volatile uint16 loops);


/**
 * Pause for the given number of micro-seconds
 *
 * This function will busy wait for the given number of
 * micro-seconds.  This is a more useful function than 'hal_delay'
 * because it does not depend on what speed the CPU is running at,
 * and if we are interrupted while we are in the delay the delay will
 * be only as long as the interrupt.
 */
extern void hal_delay_us(uint16 n);

#ifndef DESKTOP_TEST_BUILD
/* Implement simpler functions as macros to avoid function call overhead */

#define hal_cancel_timer1_event() \
    ((void) (TIMER_ENABLES &= ~TIMER_SW1_EVENT_EN_MASK))

#define hal_cancel_timer2_event() \
    ((void) (TIMER_ENABLES &= ~TIMER_SW2_EVENT_EN_MASK))

#define hal_cancel_timer3_event() \
    ((void) (TIMER_ENABLES &= ~TIMER_SW3_EVENT_EN_MASK))

#define hal_disconnect_timer_pio1() \
    ((void) (TIMER_ENABLES &= ~TIMER_PIO1_EVENT_EN_MASK))

#define hal_disconnect_timer_pio2() \
    ((void) (TIMER_ENABLES &= ~TIMER_PIO2_EVENT_EN_MASK))

#define hal_disconnect_timer_pio3() \
    ((void) (TIMER_ENABLES &= ~TIMER_PIO2_EVENT_EN_MASK))

#define hal_get_time()          (TIMER_TIME)

#define hal_get_time_lsw()      ((volatile uint16) TIMER_TIME)
#endif

#endif /* HALTIME_H */

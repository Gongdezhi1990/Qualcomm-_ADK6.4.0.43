/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Internal header for the interrupt manager
 * 
 */

#ifndef SWINT_PRIVATE_H
#define SWINT_PRIVATE_H

#include "int/swint.h"
#include "int/int.h"
#include "hydra/hydra_macros.h"
#include "panic/panic.h"

/** Number of software interrupt sources supported by hardware */
#define SWINT_HW_SOURCES (CHIP_SW_INT_COUNT)    /* (hardware constraint) */
/** Number of software clients sharing hardware interrupt sources */
#define SWINT_SW_SOURCES (4)
#define SWINT_SOURCES (SWINT_HW_SOURCES + SWINT_SW_SOURCES)

/** Range of interrupt levels to multiplex */
#define SWINT_MUX_LEVEL_MIN (INT_LEVEL_FG)
#define SWINT_MUX_LEVEL_MAX (INT_LEVEL_FG)
#define SWINT_MUX_LEVELS (SWINT_MUX_MAX - SWINT_MUX_MIN + 1)

#define SWINT_TO_INT_SOURCE(sw)                                 \
    ((int_source) (INT_SOURCE_SW0 + (sw)))

/** Information for each hardware interrupt used for multiplexing */
typedef struct swint_hw_source
{
    /** Number of multiplexed clients */
    uint16 mux_count;

    /** Configured interrupt level */
    int_level level;
} swint_hw_source;
extern swint_hw_source swint_hw_sources[SWINT_HW_SOURCES];

/** Information for each software multiplexed interrupt */
#if SWINT_SW_SOURCES
typedef struct swint_sw_source
{
    /** Hardware event being shared */
    swint_id hw_id;

    /** Client handler function */
    void (* handler)(void);

    /** Is there an outstanding request? */
    bool req;
} swint_sw_source;
extern swint_sw_source swint_sw_sources[SWINT_SW_SOURCES];
#endif /* SWINT_SW_SOURCES */

/** Mask of currently used software interrupt sources */
extern uint16f swint_configured;


/**
 * Configure a software interrupt source
 *
 * Configure a software interrupt to call "handler" at priority "level".
 * It is not necessary to enable the source separately.
 *
 * If no software interrupt sources are available then this function will
 * panic.
 * This function is similar to configure_sw_interrupt(), except that it
 * always attempts to allocate a hardware event rather than using a software
 * multiplex for
 *
 * Returns
 * An identifier for this software interrupt. This should be passed to
 * generate_sw_interrupt() to trigger the interrupt, and may be passed to
 * unconfigure_sw_interrupt() when this interrupt is no longer required.
 */
extern swint_id configure_sw_interrupt_raw(int_level level,
                                           void (* handler)(void));

/**
 * Handler for a multiplexed software interrupt
 *
 * Call any pending client handler functions that are multiplexed on
 * hardware event "id".
 */
#if SWINT_SW_SOURCES
extern void swint_demux(swint_id id);
#endif


/**
 * Handler for a multiplexed software interrupt
 *
 * Handler functions called for the corresponding hardware event when it is
 * being used to multiplex multiple clients at the same interrupt level. All
 * of these functions forward to swint_demux() to perform the dispatch.
 */
#if SWINT_SW_SOURCES
extern void swint_demux_0(void);
#if (SWINT_HW_SOURCES > 1)
extern void swint_demux_1(void);
#endif
#if (SWINT_HW_SOURCES > 2)
extern void swint_demux_2(void);
extern void swint_demux_3(void);
#endif
#if (SWINT_HW_SOURCES > 4)
extern void swint_demux_4(void);
extern void swint_demux_5(void);
#endif
#if (SWINT_HW_SOURCES > 6)
extern void swint_demux_6(void);
#endif
#endif /* SWINT_SW_SOURCES */

#endif /* SWINT_PRIVATE_H */

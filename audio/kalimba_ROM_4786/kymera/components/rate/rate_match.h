/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_match.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_MATCH_H
#define RATE_RATE_MATCH_H

#include "rate_types.h"
#include "rate_measure.h"
#include "rate_compare.h"
#include "rate_pid.h"

/** Define this to enable normal trace output; do not define
 * for unit tests to avoid target dependencies of audio_log
 */
#if !defined(UNIT_TEST_BUILD) && !defined(DESKTOP_TEST_BUILD)
#define RATE_MATCH_LOG
#endif

/** Define this to enable trace output of state variables; do not define
 * for unit tests to avoid target dependencies of audio_log */
#if !defined(UNIT_TEST_BUILD) && !defined(DESKTOP_TEST_BUILD)
/* #define RATE_MATCH_DEBUG */
#endif

/** For fast settling (~1s), the values are Kp=0.5
 * and Ki=1.5 (to be multiplied with period i.e. 0.05).
 */
#define RATE_MATCH_PID_KP_DEFAULT       (0.5)
#define RATE_MATCH_PID_KI_DEFAULT       (1.5)

/** The rate match update period, this value is based
 * on the default behaviour of the stream_ratematch_mgr.
 */
#define RATE_MATCH_PERIOD_DEFAULT       (0.05)

/** Overloaded RATE_COMPARE_RESULT value to tell the adaptive
 * rate matching implementation to start
 */
#define RATE_MATCH_ADAPT_START_MASK (((RATE_COMPARE_RESULT)(RATE_COMPARE_START | RATE_COMPARE_VALID)))
#define RATE_MATCH_ADAPT_START (RATE_COMPARE_START)
#define RATE_MATCH_ADAPT_RESET ((RATE_COMPARE_RESULT)(RATE_COMPARE_START | RATE_COMPARE_UNRELIABLE))

/****************************************************************************
 * Public Type Definitions
 */

typedef struct RATE_MATCH_CONTROL RATE_MATCH_CONTROL;

/** Coefficients of rate match controller */
typedef struct
{
    /** kp coefficient of PID controller, fractional */
    int                                 pid_kp;

    /** ki coefficient x period of PID controller, fractional */
    int                                 pid_ki;

    /** Adaptive responsiveness implementation */
    int                                 (*adapt_fn)(RATE_MATCH_CONTROL*, int, RATE_COMPARE_RESULT);

    /** Default adaptive responsiveness enable
     * \note Declaring an 8 bit packed field results in byte access */
    bool                                enable_adapt    : 8;

    /** Adaptive responsiveness configuration
     * \note Declaring an 8 bit packed field results in byte access */
    unsigned                            adapt_cfg_u8_1  : 8;
    unsigned                            adapt_cfg_u8_2  : 8;
    unsigned                            adapt_cfg_u8_3  : 8;
    int                                 adapt_cfg_int_1;
    int                                 adapt_cfg_int_2;
    int                                 adapt_cfg_int_3;

} RATE_MATCH_CONFIG;

/** Context/state for rate match controller */
struct RATE_MATCH_CONTROL
{
    /** Pointer to the feedback rate measurement context */
    RATE_MEASURE*                       fb_measure;

    /** Pointer to measurement validity parameters */
    const RATE_MEASUREMENT_VALIDITY*    valid_criteria;

    /** Rate comparison context */
    RATE_COMPARE                        cmp;

    /** PID controller */
    RATE_PID                            pid;

    /** PID update speed */
    int                                 pid_speed;

    /** Coefficients */
    const RATE_MATCH_CONFIG*            config;

    /** Enable adaptive change of responsiveness.
     * \note Declaring an 8 bit packed field results in byte access */
    bool                                enable_adapt    : 8;

    /** State field for use by adaptive responsiveness implementations.
     * \note Declaring an 8 bit packed field results in byte access */
    unsigned                            adapt_state     : 8;

    /** State field for use by adaptive responsiveness implementations.
     * \note Declaring an 8 bit packed field results in byte access */
    unsigned                            adapt_state2    : 8;

    /** State field for use by adaptive responsiveness implementations.
     * \note Declaring an 8 bit packed field results in byte access */
    unsigned                            adapt_state3    : 8;

    /** State field for use by adaptive responsiveness implementations. */
    int                                 adapt_state4;

    /** State field for use by adaptive responsiveness implementations. */
    int                                 adapt_state5;

    int                                 adapt_accum;

#ifdef RATE_MATCH_DEBUG
    /** Flag to turn on data traces
     * \note Declaring an 8 bit packed field results in byte access */
    bool                                enable_trace    : 8;

    /** Log counter */
    unsigned                            log_serial;
#endif /* RATE_MATCH_DEBUG */
};

/****************************************************************************
 * Public DataDeclarations
 */

extern const RATE_MATCH_CONFIG rate_match_default_config;

/****************************************************************************
 * Public Function Declarations
 */

/** \brief Set up a RATE_MATCH_CONTROL context and connect it
 *         to the related objects
 *  \param rmc The RATE_MATCH_CONTROL instance
 *  \param fb_measure Pointer to a RATE_MEASURE instance used for feedback
 *  \param config Configuration (i.e. coefficients). If NULL, default
 *                coefficients will be used.
 *  \param valid_criteria Measurement criteria
 *  \param sample_rate Feedback path sample rate (either Hz or Hz/25)
 *  \return False if invalid parameters (NULLs) were passed, true otherwise
 */
bool rate_match_init(RATE_MATCH_CONTROL* rmc, RATE_MEASURE* fb_measure,
                     const RATE_MATCH_CONFIG* config,
                     const RATE_MEASUREMENT_VALIDITY* valid_criteria,
                     unsigned sample_rate);

/** \brief Update and calculate a new correction
 *
 */
bool rate_match_update(RATE_MATCH_CONTROL* rmc, const RATE_RELATIVE_RATE* ref,
                       int* correction, TIME now);

/** \brief Reset rate match state
 *
 */
void rate_match_reset(RATE_MATCH_CONTROL* rmc);

#ifdef RATE_MATCH_DEBUG
/** \brief Enable/disable debug data trace output */
void rate_match_trace_enable(RATE_MATCH_CONTROL* rmc, bool enable);

/** \brief Accessor for the trace enable flag */
bool rate_match_is_trace_enabled(const RATE_MATCH_CONTROL* rmc);

/** \brief Trace with caller defined data */
void rate_match_trace_client(RATE_MATCH_CONTROL* rmc, int p1, int p2, int p3);
#endif /* RATE_MATCH_DEBUG */

#endif /* RATE_RATE_MATCH_H */

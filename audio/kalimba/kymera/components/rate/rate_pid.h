/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_match.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_PID_H
#define RATE_RATE_PID_H

#include "rate_types.h"

/****************************************************************************
 * Public Type Definitions
 */

/** State and coefficients for the PID controller */
typedef struct
{
    /** PID controller integrator state */
    int                                 pid_int;

    /** PID Kp coefficient */
    int                                 pid_kp;

    /** PID Ki coefficient */
    int                                 pid_ki;

} RATE_PID;

/****************************************************************************
 * Public Function Declarations
 */

/** \brief Set PID controller coefficients
 *
 */
void rate_pid_set_coeff(RATE_PID* rpm, int kp, int ki);

/** \brief Update the PID controller
 *
 */
int rate_pid_update(RATE_PID* rmp, int deviation);

/** \brief Update the PID controller with dampening factor,
 *         i.e. scale kp with speed and ki with speed^2.
 *         This approximately retains the "shape" (overshoot)
 *         and stability.
 */
int rate_pid_update_scaled(RATE_PID* rmp, int deviation, int speed);

/** \brief Reset rate match state
 *
 */
void rate_pid_reset(RATE_PID* rmp);


#endif /* RATE_RATE_PID_H */

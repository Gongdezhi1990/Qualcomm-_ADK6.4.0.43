/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_match.c
 * \ingroup rate_lib
 *
 * \note The main value in encapsulating this code is to hide the
 *       details of using saturating addition
 */

#include "rate_private.h"

/****************************************************************************
 * Public Function Implementations
 */


/** \brief Set PID controller coefficients
 *
 */
void rate_pid_set_coeff(RATE_PID* rp, int kp, int ki)
{
    if (rp != NULL)
    {
        rp->pid_kp = kp;
        rp->pid_ki = ki;
    }
}

/** \brief Update the PID controller
 *
 */
int rate_pid_update(RATE_PID* rp, int deviation)
{
    int correction;

    if (rp == NULL)
    {
        return 0;
    }

    RATE_ENABLE_SATURATING(save_arith_mode);

    rp->pid_int = RATE_ADDS(rp->pid_int, deviation);
    correction = RATE_ADDS( frac_mult(rp->pid_kp, deviation),
                         frac_mult(rp->pid_ki, rp->pid_int) );

    RATE_RESTORE_SATURATING(save_arith_mode);

    return correction;
}

/** \brief Update the PID controller with dampening factor,
 *         i.e. scale kp with speed and ki with speed^2.
 *         This approximately retains the "shape" (overshoot)
 *         and stability.
 *         Applying the scaling to ki before integrating
 *         rather than after, allows changing the speed
 *         dynamically.
 */
int rate_pid_update_scaled(RATE_PID* rp, int deviation, int speed)
{
    int correction;

    if (rp == NULL)
    {
        return 0;
    }

    RATE_ENABLE_SATURATING(save_arith_mode);

    int deviation_scaled = frac_mult(deviation, speed);
    rp->pid_int = RATE_ADDS( rp->pid_int,
                             frac_mult(deviation_scaled, speed));
    correction = RATE_ADDS( frac_mult(rp->pid_kp, deviation_scaled),
                            frac_mult(rp->pid_ki, rp->pid_int) );

    RATE_RESTORE_SATURATING(save_arith_mode);

    return correction;
}

/** \brief Reset PID state
 *
 */
void rate_pid_reset(RATE_PID* rp)
{
    if (rp != NULL)
    {
        rp->pid_int = 0;
    }
}


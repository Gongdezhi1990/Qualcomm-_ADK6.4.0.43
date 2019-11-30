/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file
 * Header file for the kalimba scheduler
 *
 * NOTES:
 * This header file is for use by dorm only as it needs to know whether tasks
 * have been scheduled to run whilst entering deep sleep.
 */

#ifndef SCHED_OXYGEN_FOR_DORM_H_
#define SCHED_OXYGEN_FOR_DORM_H_

/* Expose the TotalNumMessages global outside the scheduler to allow dorm to
 * interrogate it. */
extern volatile uint16f TotalNumMessages;

/**
 * \brief Finds the deadline of the next timer (strict or casual) that is set to expire.
 *
 * \param[out] next_time Holds the expiry time of the first event to expire, if there
 * are any.
 *
 * \return Returns TRUE if a valid timer was found, else FALSE
 */
extern bool timers_get_next_event_time(TIME *next_time);

#endif /* SCHED_OXYGEN_FOR_DORM_H_ */

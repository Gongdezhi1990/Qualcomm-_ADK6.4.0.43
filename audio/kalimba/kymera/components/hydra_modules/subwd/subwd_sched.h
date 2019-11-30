/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file
 *
 * Scheduler header for subwd
 */

#ifndef SUBWD_SCHED_H
#define SUBWD_SCHED_H

#define SUBWD_SCHED_TASK(m)

#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)
#define SUBWD_BG_INT(m) 
#define subwd_bg_int_id NO_BG_INT_ID

#else

#define SUBWD_BG_INT(m)                             \
    BG_INT(m, (subwd, subwd_bg_int_handler))

#endif 

#endif /* SUBWD_SCHED_H */

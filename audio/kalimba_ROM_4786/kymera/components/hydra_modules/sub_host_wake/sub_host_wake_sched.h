/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file
 */

#ifndef SUB_HOST_WAKE_SCHED_H_
#define SUB_HOST_WAKE_SCHED_H_

#define SUB_HOST_WAKE_SCHED_TASK(m)


#if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)

#define SUB_HOST_WAKE_BG_INT(m)
#define sub_host_wake_bg_int_id NO_BG_INT_ID

void sub_host_wake_bg_int_handler(void);


#else

#define SUB_HOST_WAKE_BG_INT(m)\
    BG_INT(m, (sub_host_wake, sub_host_wake_bg_int_handler))

#endif

#endif /* SUB_HOST_WAKE_SCHED_H_ */

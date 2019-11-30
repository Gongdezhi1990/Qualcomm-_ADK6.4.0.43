/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file ps_router_sched.h
 * \ingroup ps
 *
 * Definition of oxygen tasks for ps_router.
 */
#ifndef PS_ROUTER_SCHED_H
    #define PS_ROUTER_SCHED_H

    #if defined(AUDIO_SECOND_CORE) && !defined(INSTALL_DUAL_CORE_SUPPORT)
        /* Disable PS_ROUTER on secondary cores. */

        #define PS_ROUTER_SCHED_TASK(m)
        #define PS_ROUTER_TASK_QUEUE_ID NO_QID
		#define PS_ROUTER_BG_INT(m)

        /* Silence compiler warnings. */
        void ps_router_task_init(void **t);
        void ps_router_handler(void **t);
    #else
        /* Enable PS_ROUTER on primary processor. */
        #define PS_ROUTER_SCHED_TASK(m)                                 \
            SCHED_TASK_START(m, (ps_router, ps_router_task_init,        \
                                 ps_router_handler, RUNLEVEL_BASIC))    \
            SCHED_TASK_QUEUE(m, (ps_router, PS_ROUTER_TASK_QUEUE_ID))   \
            SCHED_TASK_END(m, (ps_router))
        #define PS_ROUTER_BG_INT(m)
    #endif
#endif /* PS_ROUTER_SCHED_H */
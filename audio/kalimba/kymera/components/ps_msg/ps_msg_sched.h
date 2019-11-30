/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file ps_msg_sched.h
 * \ingroup ps_msg
 *
 * Definition of oxygen tasks for ps_msg.
 */
#ifndef PS_MSG_SCHED_H
    #define PS_MSG_SCHED_H

    #ifdef INSTALL_PS_FOR_ACCMD
        /* Disable PS_MSG on Hydra. */

        #define PS_MSG_SCHED_TASK(m)
        #define PS_MSG_BG_INT(m) 
        #define PS_MSG_TASK_QUEUE_ID NO_QID
        #define ps_msg_bg_int_id NO_BG_INT_ID

        /* Silence compiler warnings. */
        void ps_msg_task_init(void **t);
        void ps_msg_handler(void **t);
        void ps_msg_bg_int_handler(void);
    #else
        /* Bluecore, Atlas, Hydra MIB. */
        #define PS_MSG_SCHED_TASK(m)                                 \
            SCHED_TASK_START(m, (ps_msg, ps_msg_task_init,           \
                                 ps_msg_handler, RUNLEVEL_BASIC))    \
            SCHED_TASK_QUEUE(m, (ps_msg, PS_MSG_TASK_QUEUE_ID))      \
            SCHED_TASK_END(m, (ps_msg))

        #define PS_MSG_BG_INT(m)                                     \
            BG_INT(m, (ps_msg, ps_msg_bg_int_handler))
    #endif /* INSTALL_PS_FOR_ACCMD */
#endif /* PS_MSG_SCHED_H */
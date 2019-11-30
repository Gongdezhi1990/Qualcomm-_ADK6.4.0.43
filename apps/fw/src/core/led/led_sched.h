/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Scheduler header for led module.
 * This is used only for led appcmd.
 */

#ifndef LED_SCHED_H
#define LED_SCHED_H

#define CORE_LED_SCHED_TASK(m) \
    SCHED_TASK_START_PRIORITY(m, (led_task, led_appcmd_init,        \
                                  led_msg_handler, RUNLEVEL_FINAL,  \
                                  DEFAULT_PRIORITY))                \
    SCHED_TASK_QUEUE(m, (led_task, led_queue_id))                   \
    SCHED_TASK_END(m, (led_task))

#define CORE_LED_BG_INT(m)                                          \
                                           BG_INT(m, (led, led_bg_int_handler))

#endif /* LED_SCHED_H */

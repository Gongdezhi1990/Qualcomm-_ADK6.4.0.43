/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef TRAP_API_SCHED_H_
#define TRAP_API_SCHED_H_

#define CUSTOMER_CORE_TRAP_API_SCHED_TASK(m)                                             \
    SCHED_TASK_START(m, (trap_api_sched_task, trap_api_sched_init,       \
                                  trap_api_sched_msg_handler, RUNLEVEL_FINAL)) \
    SCHED_TASK_QUEUE(m, (trap_api_sched_task, trap_api_sched_queue_id))       \
    SCHED_TASK_END(m, (trap_api_sched_task))

#define CUSTOMER_CORE_TRAP_API_BG_INT(m)



#endif /* TRAP_API_SCHED_H_ */

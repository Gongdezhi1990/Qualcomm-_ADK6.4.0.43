/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef SCHED_OXYGEN_PRIORITY_H_
#define SCHED_OXYGEN_PRIORITY_H_

/**
 * PRIORITY - A priority level used by the scheduler.
 *
 * Used to decide whether a put_message call from one task to another should
 * interrupt the calling task.
 * NOTE - before adding priority levels, ensure there are at least
 * NUM_PRIORITIES bits in CurrentPriorityMask
 */
typedef enum
{
    LOWEST_PRIORITY = 0,/*!< LOWEST_PRIORITY  */
    LOW_PRIORITY,       /*!< LOW_PRIORITY     */
    MID_PRIORITY,       /*!< MID_PRIORITY     */
    HIGH_PRIORITY,      /*!< HIGH_PRIORITY    */
    HIGHEST_PRIORITY,   /*!< HIGHEST_PRIORITY */
    NUM_PRIORITIES      /*!< NUM_PRIORITIES   */
} PRIORITY;


#define DEFAULT_PRIORITY MID_PRIORITY
/*@}*/



#endif /* SCHED_OXYGEN_PRIORITY_H_ */

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
        longtimer_private.h  -  internal header for the longtimer library

CONTAINS
    get_long_milli_time  -  get a 64 bit millisecond time
*/

#ifndef    __LONGTIMER_PRIVATE_H__
#define    __LONGTIMER_PRIVATE_H__

 
/* BCHS_EXPORT_POINT_START */

#include "longtimer/longtimer.h"
#include "sched/sched.h"
#include "bigint/bigint.h"

typedef uint64 longtimer;

/****************************************************************************
NAME
    get_long_milli_time  -  get a 64 bit millisecond time

FUNCTION
    Fills in the current millisecond time in the 64 bit integer pointed
    to by t.
*/

void get_long_milli_time(longtimer *t);

/* BCHS_EXPORT_POINT_END */

#endif    /* __LONGTIMER_PRIVATE_H__ */

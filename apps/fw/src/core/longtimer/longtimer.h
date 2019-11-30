/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
        longtimer.h  -  advertise the services of the longtimer library

CONTAINS
    get_milli_time  -  get a 32 bit millisecond time
    get_deci_time  -  get a 32 bit decisecond time
    get_second_time  -  get a 32 bit second time
    millitime_to_time  -  convert a millisecond time into a system time
    decitime_to_time  -  convert a decisecond time into a system time
    secondtime_to_time  -  convert a second time into a system time
*/

#ifndef    __LONGTIMER_H__
#define    __LONGTIMER_H__
 
/* BCHS_EXPORT_POINT_START */

#include "sched/sched.h"

/*
 * Typedefs for times in the different units. That way, when you put the
 * time into a structure you can give a hint about what the units are.
 */

typedef TIME MILLITIME;
typedef TIME DECITIME;
typedef TIME SECONDTIME;

/****************************************************************************
NAME
    get_milli_time  -  get a 32 bit millisecond time

RETURNS
    A 32 bit millisecond time.
*/

extern MILLITIME get_milli_time(void);


/****************************************************************************
NAME
    get_deci_time  -  get a 32 bit decisecond time

RETURNS
    A 32 bit decisecond (100 ms) time.
*/

extern DECITIME get_deci_time(void);


/****************************************************************************
NAME
    get_second_time  -  get a 32 bit second time

RETURNS
    A 32 bit second time.
*/

extern SECONDTIME get_second_time(void);


/****************************************************************************
NAME
    millitime_to_time  -  convert a millisecond time into a system time

RETURNS
    A 32 bit microsecond time that will be valid within half an hour of
    the time that was passed in.
*/

#define millitime_to_time(mt)  ((TIME) ((mt) * MILLISECOND))


/****************************************************************************
NAME
    decitime_to_time  -  convert a decisecond time into a system time

RETURNS
    A 32 bit microsecond time that will be valid within half an hour of
    the time that was passed in.
*/

#define decitime_to_time(mt)   ((TIME) ((mt) * (100 * MILLISECOND)))


/****************************************************************************
NAME
    secondtime_to_time  -  convert a second time into a system time

RETURNS
    A 32 bit microsecond time that will be valid within half an hour of
    the time that was passed in.
*/

#define secondtime_to_time(mt) ((TIME) ((mt) * SECOND))

/* BCHS_EXPORT_POINT_END */

#endif    /* __LONGTIMER_H__ */

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    get_milli_time.c  -  get a 32 bit millisecond time.

CONTAINS
    get_milli_time  -  get a 32 bit millisecond time.
*/
  
#include "longtimer/longtimer_private.h"

/****************************************************************************
NAME
    get_milli_time  -  get a 32 bit millisecond time
*/
MILLITIME get_milli_time(void)
{
    longtimer t;

    get_long_milli_time(&t);

    return (MILLITIME) uint64_lo32(t);
}

/* BCHS_EXPORT_POINT_END */

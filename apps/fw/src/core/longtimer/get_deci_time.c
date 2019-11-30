/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    get_deci_time.c  -  get a 32 bit decisecond time.

CONTAINS
    get_deci_time  -  get a 32 bit decisecond time.
*/

#include "longtimer/longtimer_private.h"

/****************************************************************************
NAME
    get_deci_time  -  get a 32 bit decisecond time
*/
DECITIME get_deci_time(void)
{
    longtimer t;

    get_long_milli_time(&t);

    return (DECITIME) uint64_lo32(uint64_div16(t,100));
}

/* BCHS_EXPORT_POINT_END */

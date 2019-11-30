/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    power_monitor.h

DESCRIPTION
    This file contains the ADC monitoring functionality

NOTES

*/

#define POWER_INIT_COMPLETE   powerUtilInitCompleteMask()

#define POWER_INIT_GET(x)        (power && (power->init_mask & (x)))
#define POWER_INIT_SET(x)        (power->init_mask |= (x))
#define POWER_INIT_IS_COMPLETE() (power && (power->init_mask == POWER_INIT_COMPLETE))


/****************************************************************************
NAME
    PowerInitComplete
    
DESCRIPTION
    Power library initialisation completed
*/
void PowerInitComplete(Task clientTask, bool success);

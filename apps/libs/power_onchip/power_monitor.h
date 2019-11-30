/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    power_monitor.h

DESCRIPTION
    This file contains the ADC monitoring functionality

NOTES

*/


/****************************************************************************
NAME
    powerMonitorRestart
    
DESCRIPTION
    Restart monitoring all values.
*/
void powerMonitorRestart(void);


/****************************************************************************
NAME
    powerMonitorInit
    
DESCRIPTION
    Initialise power monitoring
*/
void powerMonitorInit(void);


/****************************************************************************
NAME
    powerMonitorReadAdc

DESCRIPTION
    Queue an ADC reading
*/
void powerMonitorReadAdc(Task task, MessageId id, uint32 period);

/****************************************************************************
NAME
    powerMonitorReadAdcNow

DESCRIPTION
    Attempt to take an ADC reading, if this fails queue a retry
*/
bool powerMonitorReadAdcNow(Task task, MessageId id, power_adc* adc, power_init_mask mask);

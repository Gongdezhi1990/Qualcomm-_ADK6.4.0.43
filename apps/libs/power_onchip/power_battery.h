/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    power_battery.h

DESCRIPTION
    This file contains the battery voltage monitoring specific functionality

NOTES
    Certain assumptions have been made on the assignment of the analog input
    signals:
    
    VBAT = AIO *   Ra
                 -------
                 Ra + Rb
    
    Where the divisor ratio of Ra and Rb is configured from persistent store
*/


#ifndef POWER_BATTERY_H_
#define POWER_BATTERY_H_


/****************************************************************************
NAME    
    powerBatterySendVoltageInd
    
DESCRIPTION
    Call to send the battery voltage and its level to App.
*/
void powerBatterySendVoltageInd(void);


/****************************************************************************
NAME    
    powerBatteryHandleVoltageReading
    
DESCRIPTION
    Calculate current battery voltage and check to determine the battery level.
    Both battery voltage and level are sent to App and the App decides the 
    further operation base on the reported results.
*/
void powerBatteryHandleVoltageReading(uint16 reading);


/****************************************************************************
NAME    
    powerBatteryHandleTemperatureReading
    
DESCRIPTION
    Calculate the current battery temperature
*/
void powerBatteryHandleTemperatureReading(uint16 reading);


#endif /* POWER_BATTERY_H_ */



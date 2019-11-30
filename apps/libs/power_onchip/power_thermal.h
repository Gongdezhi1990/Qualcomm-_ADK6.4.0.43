/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    power_thermal.h

DESCRIPTION
    This file contains the thermal safety handling for the battery charging
    functionality.

    Once the battery charger is connected, die temperature is monitored and
    the charger is managed accordingly.

NOTES

**************************************************************************/


#ifndef POWER_THERMAL_H_
#define POWER_THERMAL_H_


/****************************************************************************
NAME
    powerThermalInit
    
DESCRIPTION
    Initialise the thermal power handler.
    
RETURNS
    void
*/
void powerThermalInit(void);

/****************************************************************************
NAME
    powerThermalHandleMonitorReq

DESCRIPTION
    Handle thermal power monitoring requests.

RETURNS
    void
*/
void powerThermalHandleMonitorReq(void);

/****************************************************************************
NAME
    powerThermalUpdateEnable

DESCRIPTION
    Updates charger enable request with respect to thermal state.

RETURNS
    bool
*/
bool powerThermalUpdateEnable(bool enable);

/****************************************************************************
NAME
    powerThermalUpdateCurrent

DESCRIPTION
    Updates charger current with respect to thermal state.

RETURNS
    uint16
*/
uint16 powerThermalUpdateCurrent(uint16 current);

#endif /* POWER_THERMAL_H_ */



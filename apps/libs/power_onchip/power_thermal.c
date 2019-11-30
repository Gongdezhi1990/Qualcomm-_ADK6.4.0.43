/****************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_thermal.c

DESCRIPTION
    This file contains the thermal safety handling for the battery charging
    functionality.

    Once the battery charger is connected, die temperature is monitored and
    the charger is managed accordingly.

NOTES

**************************************************************************/


/****************************************************************************
    Header files
*/

#include "power_onchip.h"
#include "power_private.h"
#include "power_thermal.h"
#include "power_charger.h"
#include "power_utils.h"

/****************************************************************************
NAME
    powerThermalInit

DESCRIPTION
    Initialise the thermal power handler.

RETURNS
    void
*/
void powerThermalInit(void)
{
    int16   temperature;

    PRINT(("POWER: Thermal Monitoring Init\n"));

    powerUtilGetThermalConfig();

    PRINT(("POWER: Thermal Shutdown Period = %ums\n", power->thermal.config.shutdown_period));
    PRINT(("POWER: Thermal Shutdown Long Period = %ums\n", power->thermal.config.shutdown_long_period));
    PRINT(("POWER: Shutdown Temperature = %u degC\n", power->thermal.config.shutdown_temperature));
    PRINT(("POWER: Rampdown Temperature = %u degC\n", power->thermal.config.rampdown_temperature));
    PRINT(("POWER: Re-enable Temperature = %u degC\n", power->thermal.config.reenable_temperature));

    temperature = powerUtilGetThermalDieTemperature();

    power->thermal.state = thermal_normal;
    if ((temperature == INVALID_SENSOR) || (temperature == INVALID_TEMPERATURE))
    {
        PRINT(("POWER: ERROR Could not read die temperature\n"));
    }
    else
    {
        PRINT(("POWER: Thermal Monitoring Temperature: %u\n", temperature));

        if (temperature >= power->thermal.config.rampdown_temperature)
        {
            power->thermal.state = thermal_rampdown;
        }

        if (temperature >= power->thermal.config.shutdown_temperature)
        {
            power->thermal.state = thermal_shutdown;
            ChargerConfigure(CHARGER_ENABLE, FALSE);
            power->charger_enabled = FALSE;
        }
     }

    PRINT(("POWER: Thermal Monitoring State: %u\n", power->thermal.state));
}

/****************************************************************************
NAME
    powerThermalHandleMonitorReq

DESCRIPTION
    Handle thermal power monitoring requests.

RETURNS
    void
*/
void powerThermalHandleMonitorReq(void)
{
    int16               temperature;
    uint16              period;
    uint16              enable;
    power_thermal_state new_state = power->thermal.state;

    temperature = powerUtilGetThermalDieTemperature();

    if ((temperature == INVALID_SENSOR) || (temperature == INVALID_TEMPERATURE))
    {
        PRINT(("POWER: ERROR Could not read die temperature\n"));
    }
    else
    {
        switch (power->thermal.state)
        {
        case thermal_normal:
        {
            if (temperature >= power->thermal.config.rampdown_temperature)
            {
                new_state = thermal_rampdown;
            }
            if (temperature >= power->thermal.config.shutdown_temperature)
            {
                new_state = thermal_shutdown;
            }
            break;
        }
        case thermal_rampdown:
        {
            if (temperature < power->thermal.config.rampdown_temperature)
            {
                new_state = thermal_normal;
            }
            if (temperature >= power->thermal.config.shutdown_temperature)
            {
                new_state = thermal_shutdown;
            }
            break;
        }
        case thermal_shutdown:
        {
            if (temperature <= power->thermal.config.reenable_temperature)
            {
                new_state = thermal_normal;
            }
            break;
        }
        default:
        {
            Panic();
        }
        }

        if (new_state != power->thermal.state)
        {
            power->thermal.state = new_state;

            switch (power->thermal.state)
            {
            case thermal_shutdown:
            {
                PowerOnChipChargerEnable(FALSE);
                break;
            }
            case thermal_rampdown:
                /* Fall-through, same behaviour for ramp-down and normal. */
            case thermal_normal:
            {
                enable = power->charger_enabled;
                PowerOnChipChargerEnable(FALSE);
                powerUtilSetChargerCurrents(power->charger_i_target);
                PowerOnChipChargerEnable(enable);
                break;
            }
            }
        }

        THERMAL_PRINT(("POWER: Thermal Monitoring Temperature: %u degC, State: %u\n", temperature, power->thermal.state));
        if (power->chg_state != power_charger_disconnected)
        {
            period = (power->chg_state == power_charger_complete)
                       ? power->thermal.config.shutdown_long_period : power->thermal.config.shutdown_period;
            THERMAL_PRINT(("POWER: Que POWER_INTERNAL_THERMAL_REQ in %u ms\n", period));
            MessageSendLater(&power->task, POWER_INTERNAL_THERMAL_REQ, 0, period);
        }
    }
}

/****************************************************************************
NAME
    powerThermalUpdateEnable

DESCRIPTION
    Updates charger enable request with respect to thermal state.

RETURNS
    void
*/
bool powerThermalUpdateEnable(bool enable)
{
    if (power->thermal.state == thermal_shutdown)
    {
        PRINT(("POWER: Charger NOT enabled due to thermal shutdown\n"));
        return FALSE;
    }

    return enable;
}

/****************************************************************************
NAME
    powerThermalUpdateCurrent

DESCRIPTION
    Updates charger current with respect to thermal state.

RETURNS
    void
*/
uint16 powerThermalUpdateCurrent(uint16 current)
{
    if (power->thermal.state == thermal_rampdown)
    {
        PRINT(("POWER: Charger current reduced due to thermal rampdown\n"));
        return current/4;
    }

    if (power->thermal.state == thermal_shutdown)
    {
        PRINT(("POWER: Charger current zeroed due to thermal shutdown\n"));
        return 0;
    }

    return current;
}

/****************************************************************************
Copyright (c) 2005-2018 Qualcomm Technologies International, Ltd.
*/


/*!
@file    power_adapter.h

@brief   Declares the API that all power library adapters must implement.

         Power library adapters are selected at link-time. A power library
         adapter uses libraries that are specific to the charger hardware.

         This file provides documentation for the Power API.
*/


#ifndef POWER_ADAPTER_H_
#define POWER_ADAPTER_H_

#include <power.h>

/*!
    @brief This function will initialise the battery monitoring and charging
           sub-system. The application will receive a POWER_INIT_CFM message
           from the library indicating the initialisation status.

    @param clientTask The task that will handle power messages.

    @param power_data Configuration data for the power library.

    @param pmu_mon_config PMU monitoring configuration data.
*/
void PowerAdapterInit(Task clientTask, const power_config *power_data, const power_pmu_temp_mon_config *pmu_mon_config);


/*!
    @brief This function will close the battery monitoring and charging sub-system.
*/
void PowerAdapterClose(void);


/*!
    @brief Get the battery voltage and gas gauge level.

    @param vbat Container for battery voltage and gas gauge level.
 
    @return bool TRUE if reading available, FALSE otherwise.
*/
bool PowerAdapterBatteryGetVoltage(voltage_reading* vbat);


/*!
    @brief Get the battery thermistor voltage and temperature region.

    @param vthm Container for thermistor voltage and temperature region.

    @return bool TRUE if reading available, FALSE otherwise.
*/
bool PowerAdapterBatteryGetTemperature(voltage_reading *vthm);


/*!
    @brief Get the charger voltage and level.

    @param vchg Container for charger voltage and level.

    @return bool TRUE if reading available, FALSE otherwise.
*/
bool PowerAdapterChargerGetVoltage(voltage_reading* vchg);


/*!
    @brief Get the charger state.

    @param vbat Container for the charger state.
*/
void PowerAdapterChargerGetState(power_charger_state* state);


/*!
    @brief Enable/disable the charger status monitoring. Monitoring is enabled by
           default but is disabled when charger detach is detected. It is up
           to the application to re-enable charger monitoring when the charger
           is attached.
*/
void PowerAdapterChargerMonitor(void);


/*!
    @brief Enable/disable the charger.

    @param enable Requested charger enable state.
*/
void PowerAdapterChargerEnable(bool enable);


/*!
    @brief Set the fast charge current.

    @param current Requested fast charge current.
*/
void PowerAdapterChargerSetCurrent(uint16 current);


/*!
    @brief Configure boost settings for BlueCore. By default this is disabled,
           but can be configured to draw boost current from internal/external
           transistor. Boost charge will be enabled on entering fast charge
           unless the boost cycle has already completed. The boost cycle is
           reset on charger attach.

    @param boost Requested boost setting.
*/
void PowerAdapterChargerSetBoost(power_boost_enable boost);


/*!
    @brief Set the termination voltage.

    @param vterm Requested termination voltage.

    @return bool TRUE, except on BlueCore this will return FALSE if requested
                 termination voltage is higher than the configured termination
                 voltage.
*/
bool PowerAdapterChargerSetVterm(uint16 vterm);

/*!
    @brief Gets the voltage scalar for the charger version.

    @return uint16 The voltage scalar.
*/
uint16 PowerAdapterChargerGetVoltageScale(void);


/*!
    @brief Set suppress LED0 whilst charging.

    @param suppress Requested suppress state.
*/
void PowerAdapterChargerSuppressLed0(bool suppress);


/*!
    @brief Configure the charger with initial configuration.
*/
void PowerAdapterChargerConfigure(void);


/*!
    @brief Get charger enabled state.

    @return bool TRUE if charging, FALSE otherwise.
*/
bool PowerAdapterChargerEnabled(void);

/*!
    @brief Determines charger connected status from underlying hardware. Can be called
    before Power library is initialised.

    @return bool True if the charger is connected, false otherwise.
*/
bool PowerAdapterChargerConnected(void);

/*!
    @brief Determines if the charger is (potentially) drawing full current from underlying hardware.
    Can be called before Power library is initialised.

    @return bool True if the charger is drawing full current, false otherwise.
*/
bool PowerAdapterChargerFullCurrent(void);

/*!
    @brief Determines if the charger is fast charging from underlying hardware.
    Can be called before Power library is initialised.

    @return bool True if the charger fast charging, false otherwise.
*/
bool PowerAdapterChargerFastCharge(void);

/*!
    @brief Determines if the charger is connected but disabled from underlying hardware.
    Can be called before Power library is initialised.

    @return bool True if the charger is connected but disabled, false otherwise.
*/
bool PowerAdapterChargerDisabled(void);

/*!
    @brief Register a task to handle messages from the charger hardware.

    @param task This task will receive messages with charger hardware state changes.

*/
void PowerAdapterRegisterChargerTask(Task task);

/*!
    @brief Start monitoring charging cycle progress.
*/
void PowerAdapterMonitorStartChargerProgressMonitor(void);

/*!
    @brief Stop monitoring charging cycle progress.
*/
void PowerAdapterMonitorStopChargerProgressMonitor(void);


/*!
    @brief Monitor charger voltage whilst current limiting as per USB Battery
           Charging v1.2.

    @param vchg_reading Current charger voltage reading.
*/
void PowerAdapterCurrentLimitingHandleMonitorReq(uint16 vchg_reading);


/*!
    @brief Start monitoring charger voltage for current limiting as per USB Battery
           Charging v1.2.
*/
void PowerAdapterCurrentLimitingStart(void);

/*!
    @brief Stop monitoring charger voltage for current limiting as per USB Battery
           Charging v1.2.
*/
void PowerAdapterCurrentLimitingStop(void);

/*!
    @brief Handle charger headroom error whilst monitoring charger voltage for
           current limiting as per USB Battery Charging v1.2.
*/
void PowerAdapterCurrentLimitingHandleHeadroomError(void);

/*!
    @brief Check if battery is in over-voltage condition.

    @return bool TRUE is battery is in over-voltage condition, FALSE otherwise.
*/
bool PowerAdapterBatteryIsOverVoltage(void);


/*!
    @brief Check if battery has recovered from over-voltage condition.

    @param The current battery voltage.

    @return bool TRUE if the battery is in the over-voltage condition and the
            the battery voltage is now less than the termination voltage,
            FALSE otherwise.
*/
bool PowerAdapterBatteryIsRecoveredFromOverVoltage(voltage_reading vbat);

/*!
    @brief Check the battery status at boot time.

    @return charger_battery_status.
*/
charger_battery_status PowerAdapterBatteryGetStatusAtBoot(void);

/*!
    @brief Service the Quick Charge interrupt.
*/
void PowerAdapterQuickChargeServiceInterrupt(void);


/*!
    @brief Register a task to handle messages from the Quick Charge hardware.

    @param task This task will receive messages when parts of the Quick Charger
           hardware changes.
*/
void PowerAdapterQuickChargeRegisterTask(Task task);

#endif     /* POWER_ADAPTER_H_ */

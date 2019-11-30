/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    charger_if.h

CONTAINS
    Definitions for the charger subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef __APP_CHARGER_IF_H__
#define __APP_CHARGER_IF_H__

/*! @brief Status of the charger hardware as returned by the ChargerStatus()
    trap.
*/
typedef enum
{
    /*! The charger has no input power. */       
    NO_POWER,

    /*! The battery is flat and being charged slowly. */             
    TRICKLE_CHARGE,

    /*! The battery is in pre charge state. */
    PRE_CHARGE,

    /*! The battery is being charged quickly. */
    FAST_CHARGE,

    /*! Charger voltage is not high enough to be able to charge the battery.
     * Once triggered the error can be cleared by disabling and
     * then re-enabling the charger. */
    HEADROOM_ERROR,

    /*! The charger enters overvoltage mode if the VBAT exceeds 4.7V (typ).
     * Once triggered the error can be cleared by disabling and
     * then re-enabling the charger. */
    VBAT_OVERVOLT_ERROR,

    /*! The battery is full and not being charged. */
    STANDBY,

    /*! The charger is disabled or in an unreliable state. */
    DISABLED_ERROR,

    /*! Charger could not be configured: Unknown reason. */
    CONFIG_FAIL_UNKNOWN,

    /*! Charger could not be configured: Charger is currently enabled. */
    CONFIG_FAIL_CHARGER_ENABLED,

    /*! Charger could not be configured: Invalid primary eFuse CRC. */ 
    CONFIG_FAIL_EFUSE_CRC_INVALID, 
    
    /*! Charger could not be configured: One or more charger eFuse trims are zero. */
    CONFIG_FAIL_EFUSE_TRIMS_ZERO,
    
    /*! Charger could not be configured: One or more requested charge currents are zero. */
    CONFIG_FAIL_CURRENTS_ZERO, 
    
    /*! Charger could not be configured: One or more requested values or eFuse trims are out of range. */
    CONFIG_FAIL_VALUES_OUT_OF_RANGE, 
    
    /*! Charger could not be enabled/disabled: Unknown reason. */
    ENABLE_FAIL_UNKNOWN,

    /*! Charger could not be enabled: Invalid primary eFuse CRC. */
    ENABLE_FAIL_EFUSE_CRC_INVALID,

    /*! Internal current source LED configure failed */
    INTERNAL_CURRENT_SOURCE_CONFIG_FAIL
} charger_status;

/*! @brief Charger event enables that can be passed to ChargerDebounce() */
typedef enum
{
    CHARGER_VREG_EVENT    = (1 << 0),
    CHARGER_CONNECT_EVENT = (1 << 1)
}charger_events;

/*! @brief Charger config keys - used to configure the charger.

For Qualcomm Voice and Music chips which include battery charger hardware,
the data sheet will include a section which gives electrical
characteristics of the charger.

The CSRA6810x, QCC302x/3x, and QCC512x chargers differ from previous 
Qualcomm BlueCore chargers, in that the previous Trickle charge phase 
is now split into two phases Trickle charge and Pre-charge.

 */
typedef enum
{

    /*!
When using this key, a zero value will disable the charger and
a non zero value will enable the charger.

All the required charger configuration keys below should be configured before
the charger is enabled as once enabled only the three currents can be modified
once the charger is enabled. 
     */
    CHARGER_ENABLE,

    /*!
Only supported on BlueCore chips.
     */
    CHARGER_TRIM,

    /*!
Only supported on BlueCore chips.
     */
    CHARGER_CURRENT,

    /*!
Only supported on BlueCore chips.
     */
    CHARGER_SUPPRESS_LED0,

    /*!
Only supported on BlueCore chips.
     */
    CHARGER_ENABLE_BOOST,

    /*!
Only supported on BlueCore chips.
     */
    CHARGER_USE_EXT_TRAN,

    /*!
Only supported on BlueCore chips. 

On CSRA6810x, QCC302x/3x, and QCC512x devices the MIBKEY ResetOnChargerAttach 
is used to reset the device on a charger-attach event.
     */
    CHARGER_ATTACH_RESET_ENABLE,

    /*!
Used to set the termination voltage. The value passed to this function is in
millivolts and the supported range is device dependent. Any unsupported values
are rounded down. 
    */
    CHARGER_TERM_VOLTAGE,

    /*!
Only supported on BlueCore chips.

On CSRA6810x, QCC302x/3x, and QCC512x devices see CHARGER_EXTERNAL_RESISTOR
    */
    CHARGER_ENABLE_HIGH_CURRENT_EXTERNAL_MODE,

    /*!
Only supported on BlueCore chips.
    */
    CHARGER_SET_EXTERNAL_TRICKLE_CURRENT,

    /*!
When the battery has become fully charged, the charger will enter its 
STANDBY phase. During this phase it will monitor the battery's voltage. 
When it drops below a threshold, it will restart fast charging. 

The parameter is in millivolts and describes the threshold below VFLOAT
when the charger will begin charging again.

The values are chip dependent.
On CSRA6810x, QCC302x/3x, and QCC512x devices, the possible values are:
100mV, 150mV, 200mV and 250mV below VFLOAT. 
Attempting to set a value outside the supported range will clip to the limit. 
Values between these will round to next highest value from the list.
    */
    CHARGER_STANDBY_FAST_HYSTERESIS,

    /*!
Used to set the current for trickle charger mode in milliamps.

This parameter can be modified whilst the charger is enabled. 

The limits are chip dependent.

On CSRA6810x, QCC302x/3x, and QCC512x devices the maximum possible trickle 
current is 50mA.

     */
    CHARGER_TRICKLE_CURRENT,

    /*!
Used to set the current for pre charger mode in milliamps. 

This parameter can be modified whilst the charger is enabled. 

The limits are chip dependent.

On CSRA6810x, QCC302x/3x, and QCC512x devices the maximum current is: 

Internal max 200mA,
External min/max varies with resistor.

 Max current equates to 100mV across the external resistor.
   Eg. 100mOhms gives External max 1000mA
        56mOhms gives External max 1785mA
     */
    CHARGER_PRE_CURRENT,

    /*!
Used to set the current for fast charger mode in milliamps. 

This parameter can be modified whilst the charger is enabled. 

The limits are chip dependent.

On CSRA6810x, QCC302x/3x, and QCC512x devices the maximum current is: 

Internal max 200mA,
External min/max varies with resistor.

 Max current equates to 100mV across the external resistor.
   Eg. 100mOhms gives External max 1000mA
        56mOhms gives External max 1785mA
     */
    CHARGER_FAST_CURRENT,
    
    /*!
Used to set the external series resistor value in milliohms. 

A non-zero value is needed to enable the external charger. 
     */
    CHARGER_EXTERNAL_RESISTOR,

    /*!
Used to select the external charger in pre charge mode. 

Boolean: Any non-zero value will enable the external charger, set this to zero 
for internal charger. 
     */
    CHARGER_USE_EXTERNAL_RESISTOR_FOR_PRE_CHARGE,

    /*!
Used to select the external charger in fast charge mode. 

Boolean: Any non-zero value will enable the external charger, set this to zero 
for internal charger. 
     */
    CHARGER_USE_EXTERNAL_RESISTOR_FOR_FAST_CHARGE,

    /*!
Used to set the threshold to switch from Pre to Fast charge in millivolts. 

The values are chip dependent.
On CSRA6810x, QCC302x/3x, and QCC512x devices, the possible values are:
2500mV, 2900mV, 3000mV and 3100mV. 
Attempting to set a value outside the supported range will clip to the limit. 
Values between these will round to next highest value from the list.
     */
    CHARGER_PRE_FAST_THRESHOLD,

    /*!
Used to set the current termination in constant voltage mode as a percentage
of the fast charge current.

The values are chip dependent.
On CSRA6810x, QCC302x/3x, and QCC512x devices, the possible values are:
10%, 20%, 30% and 40%.
Attempting to set a value outside the supported range will clip to the limit. 
Values between these will round to next highest value from the list.
     */
    CHARGER_ITERM_CTRL,

    /*!
Used to set the debounce timer as the charger changes states eg. Pre to Fast, 
Fast to Standby based on current and voltage changes in ms.

The values are chip dependent.
On CSRA6810x, QCC302x/3x, and QCC512x devices, the possible values are:
0(off), 4ms, 32ms and 128ms.
Attempting to set a value more than this will cause the debounce to be set to
maximum value. Other values will cause the next highest value from the list 
to be selected.
     */
    CHARGER_STATE_CHANGE_DEBOUNCE
}charger_config_key;


/*! @brief Battery status keys - used to configure the charger.
*/
typedef enum
{
    CHARGER_BATTERY_UNKNOWN = 0,  /*!< The battery voltage was not checked  */
    CHARGER_BATTERY_OK = 1,       /*!< Battery voltage sufficient to use */
    CHARGER_BATTERY_DEAD = 2      /*!< Battery voltage too low at start (dead) */
} charger_battery_status;

/*! @brief PMU power source
*/
typedef enum
{
   PMU_EXTERNAL_SUPPLY = 0,
   PMU_BATTERY = 1
}pmu_power_source;

/*! @brief Hardware supported values for CHARGER_TERM_VOLTAGE
*/
typedef enum
{
   CHARGER_TERM_MILLIVOLT_3650 = 3650,
   CHARGER_TERM_MILLIVOLT_3700 = 3700,
   CHARGER_TERM_MILLIVOLT_3750 = 3750,
   CHARGER_TERM_MILLIVOLT_3800 = 3800,
   CHARGER_TERM_MILLIVOLT_3850 = 3850,
   CHARGER_TERM_MILLIVOLT_3900 = 3900,
   CHARGER_TERM_MILLIVOLT_3950 = 3950,
   CHARGER_TERM_MILLIVOLT_4000 = 4000,
   CHARGER_TERM_MILLIVOLT_4050 = 4050,
   CHARGER_TERM_MILLIVOLT_4100 = 4100,
   CHARGER_TERM_MILLIVOLT_4150 = 4150,
   CHARGER_TERM_MILLIVOLT_4200 = 4200,
   CHARGER_TERM_MILLIVOLT_4250 = 4250,
   CHARGER_TERM_MILLIVOLT_4300 = 4300,
   CHARGER_TERM_MILLIVOLT_4350 = 4350,
   CHARGER_TERM_MILLIVOLT_4400 = 4400
}charger_term_voltage;

/*! @brief Hardware supported values for CHARGER_STANDBY_FAST_HYSTERESIS
*/
typedef enum
{
   CHARGER_STANDBY_FAST_HYSTERESIS_MILLIVOLT_100 = 100,
   CHARGER_STANDBY_FAST_HYSTERESIS_MILLIVOLT_150 = 150,
   CHARGER_STANDBY_FAST_HYSTERESIS_MILLIVOLT_200 = 200,
   CHARGER_STANDBY_FAST_HYSTERESIS_MILLIVOLT_250 = 250
}charger_standby_fast_hysteresis;

/*! @brief Hardware supported values for CHARGER_PRE_FAST_THRESHOLD
*/
typedef enum
{
   CHARGER_PRE_FAST_THRESHOLD_MILLIVOLT_2500 = 2500,
   CHARGER_PRE_FAST_THRESHOLD_MILLIVOLT_2900 = 2900,
   CHARGER_PRE_FAST_THRESHOLD_MILLIVOLT_3000 = 3000,
   CHARGER_PRE_FAST_THRESHOLD_MILLIVOLT_3100 = 3100
}charger_pre_fast_threshold;


/*! @brief Hardware supported values for CHARGER_ITERM_CTRL
*/
typedef enum
{
   CHARGER_ITERM_CTRL_PERCENT_10 = 10,
   CHARGER_ITERM_CTRL_PERCENT_20 = 20,
   CHARGER_ITERM_CTRL_PERCENT_30 = 30,
   CHARGER_ITERM_CTRL_PERCENT_40 = 40
}charger_iterm_ctrl;

/*! @brief Hardware supported values for CHARGER_STATE_CHANGE_DEBOUNCE
*/
typedef enum
{
   CHARGER_STATE_CHANGE_DEBOUNCE_MILLISEC_0 = 0,
   CHARGER_STATE_CHANGE_DEBOUNCE_MILLISEC_4 = 4,
   CHARGER_STATE_CHANGE_DEBOUNCE_MILLISEC_32 = 32,
   CHARGER_STATE_CHANGE_DEBOUNCE_MILLISEC_128 = 128
}charger_state_change_debounce;

#endif /* __APP_CHARGER_IF_H__  */

/****************************************************************************
Copyright (c) 2005-2018 Qualcomm Technologies International, Ltd.
*/


/*!
@file    power.h

@brief   These functions are used to monitor the reference, battery and charger
         voltages; monitor the battery temperature; and configure the charger.
*/


#ifndef POWER_H_
#define POWER_H_

#include <adc.h>
#include <charger.h>
#include <library.h>

#define POWER_MAX_VTHM_LIMITS   0x07
#define POWER_MAX_VBAT_LIMITS   0x05
#define POWER_MAX_VTHM_REGIONS  (POWER_MAX_VTHM_LIMITS + 1)
#define POWER_MAX_VBAT_REGIONS  (POWER_MAX_VBAT_LIMITS + 1)
#define POWER_VBAT_LIMIT_END    0xFF
#define POWER_VTHM_LIMIT_END    0xFFFF
#define POWER_VSCALE            PowerChargerGetVoltageScale()
#define POWER_PERIOD_SCALE      0x0A
#define MAX_TRICKLE_CURRENT       50
#define MAX_INTERNAL_CURRENT     200
#define MAX_EXTERNAL_CURRENT    1800
#define MAX_SENSE_RESISTOR_MV    100
#define ONE_HUNDRED_PERCENT      100

/*!
    @brief Define the types for the upstream messages sent from the Power
    library to the application.
*/
typedef enum
{
/* 0x7800 */    POWER_INIT_CFM = POWER_MESSAGE_BASE,
/* 0x7801 */    POWER_BATTERY_VOLTAGE_IND, 
/* 0x7802 */    POWER_BATTERY_TEMPERATURE_IND,
/* 0x7803 */    POWER_CHARGER_VOLTAGE_IND,
/* 0x7804 */    POWER_CHARGER_CURRENT_LIMITING_IND,
/* 0x7805 */    POWER_CHARGER_STATE_IND,

/* Library message limit */
/* 0x7806 */    POWER_MESSAGE_TOP
}PowerMessageId ;

/*! 
    @brief Charger state
*/
typedef enum
{
    power_charger_disconnected,
    power_charger_disabled,
    power_charger_trickle,
    power_charger_just_connected,
    power_charger_fast, /* DO NOT reorder this enum without checking where it is used! */
    power_charger_boost_internal,
    power_charger_boost_external,
    power_charger_complete
} power_charger_state;

/*! 
    @brief Boost setting
*/
typedef enum
{
    power_boost_disabled,
    power_boost_internal_enabled,
    power_boost_external_enabled
} power_boost_enable;

/*! 
    @brief ADC monitoring info
*/
typedef struct
{
    vm_adc_source_type source:8;         /* The ADC source */
    unsigned           unused:8;
    unsigned           period_chg:16;    /* Interval (in milli-seconds) between measurements when charging */
    unsigned           period_no_chg:16; /* Interval (in milli-seconds) between measurements when not charging */
} power_adc;

/*! 
    @brief VREF Configuration
*/
typedef struct
{
    power_adc           adc;                /* VREF ADC */
} power_vref_config;

/*! 
    @brief Battery Limit Setting
*/
typedef struct
{
    unsigned notify_period:8;       /* Interval (seconds/POWER_PERIOD_SCALE) between notifications (0 to disable) */
    uint16   limit;                 /* Battery Voltage Threshold / Voltage Scale */
} power_vbat_limit;

/*! 
    @brief Battery Configuration
*/
typedef struct
{
    power_adc           adc;                            /* Battery Voltage ADC */
    power_vbat_limit    limits[POWER_MAX_VBAT_REGIONS]; /* Battery Voltage Thresholds Settings */
    unsigned            ignore_increase_no_chg:1;       /* Do not report increasing voltage if charger not connected */
    unsigned            :15;
} power_vbat_config;

/*!
    @brief Thermistor Configuration. VTHM is read from an external 
    AIO. Limits can be calculated from your thermistor's data sheet, 
    where T is the resistance of the thermistor for given temperature 
    and R is a fixed resistor:
    
    VSRC -------
                |
                R                VSRC(mV) x T
                |---- VTHM(mV) = ------------
                T                   T + R
                |
                _
                -
    
    Where VSRC is the same source as the ADC we can configure limits 
    in terms of raw ADC counts in place of the calculated voltage 
    for improved accuracy:
    
                       VSRC(ADC Counts) x T
    VTHM(ADC Counts) = --------------------
                              T + R
    
    VSRC(ADC Counts) = VADC(ADC Counts) = Max ADC Counts.
    
    NOTES:
    - Thermistor may be driven from a constant source or via a PIO
    - ADC readings are 10 bits: Max ADC Counts = 1023.
    - ADC readings will saturate above ADC source voltage, on BC5/BC6 
    this is 1.5v, on BC7 1.35v. No limit should be set above this level.
*/
typedef struct
{
    power_adc           adc;                           /* Thermistor ADC */
    unsigned            delay:4;                       /* Delay(ms) between setting PIO and reading ADC */
    unsigned            unused1:1;                     /* Unused */
    unsigned            raw_limits:1;                  /* Set limits in ADC counts rather than voltage */
    unsigned            drive_ics:1;                   /* Drive thermistor from internal current source */
    unsigned            drive_pio:1;                   /* Drive thermistor from a PIO */
    unsigned            pio:8;                         /* PIO to use */
    unsigned            ics:8;                         /* Internal Current Source to use */
    unsigned            unused2:8;                     /* Padding */
    uint16              limits[POWER_MAX_VTHM_LIMITS]; /* Voltage Thresholds (mV/ADC Counts) */
} power_vthm_config;

/*! 
    @brief Charger Configuration
*/
typedef struct
{
    power_adc           adc;                        /* VCHG ADC */
    
    unsigned            current_limit_period_chg:16;     /* Vchg current limit read period in ms. */
    unsigned            non_current_limit_period_chg:16; /* Vchg non current limit read period in ms. */
    	
    uint16              limit;                      /* VCHG Threshold / Voltage Scale */

    unsigned            ext_fast:1;                 /* Use external resistor for pre-charge. */
    unsigned            ext_resistor_value:10;      /* Resistor value for external series resistor */
    unsigned            unused1:5;                  /* Unused */

    unsigned            trickle_charge_current:8;   /* Trickle charge current*/
    unsigned            precharge_current:8;        /* Pre-charge current*/

    uint16              default_vfloat;             /* Default terminal voltage */
    uint16              debounce_time;              /* State change debounce timer */
    uint16              pre_fast_threshold;         /* Pre/fast voltage threshold */
    uint16              iterm_ctrl;                 /* Termination current control */
    uint16              standby_fast_hysteresis;    /* Standby/fast voltage hysteresis */

    unsigned            current_limit_threshold:13;     /* Vchg current limit threshold in mV */
    unsigned            unused2:3;                      /* Unused */
    unsigned            current_limit_recovery_threshold:13;  /* Vchg current recovery threshold in mV */
    unsigned            unused3:3;                      /* Unused */

    unsigned            positive_current_step:6;    /* Current increment step size in mA */
    unsigned            unused4:2;                  /* Unused */
    unsigned            negative_current_step:6;    /* Current decrement step size in mA */
    unsigned            unused5:2;                  /* Unused */


} power_vchg_config;

typedef struct
{
    power_adc           adc;                        /* Charger Progress Monitor ADC */
} power_chg_prg_mon_config;

/*!
    @brief  PMU temperature based charging control configuration
*/
typedef struct
{
    uint16              min_charge_i;           /*!< Minimum charger current */
    uint16              max_charge_i;           /*!< Maximum charger current */
    uint16              monitor_period_active;  /*!< Period (ms) to monitor PMU temperature when charger is active */
    uint16              monitor_period_idle;    /*!< Period (ms) to monitor PMU temperature when charger is idle */
    uint16              monitor_period_nopwr;   /*!< Period (ms) to monitor PMU temperature when there is no charger power */
    unsigned            no_incr_i_temp:8;       /*!< Temeprature (degC) where current will not be increased */
    unsigned            charger_i_step:8;       /*!< Charger current step-size */
    unsigned            unused:8;               /*!< Not used (set to 0) */
    unsigned            decr_i_temp:8;          /*!< Temeprature (degC) where current will be decreased */
} power_pmu_temp_mon_config;

/*! 
    @brief Battery Level 
*/
typedef enum
{
    POWER_BATT_CRITICAL,           /* when voltage (mV) < level0 * PowerChargerGetVoltageScale() */
    POWER_BATT_LOW,                /* when voltage (mV) < level1 * PowerChargerGetVoltageScale() */
    POWER_BATT_LEVEL0,             /* when voltage (mV) < level2 * PowerChargerGetVoltageScale() */
    POWER_BATT_LEVEL1,             /* when voltage (mV) < level3 * PowerChargerGetVoltageScale() */
    POWER_BATT_LEVEL2,             /* when voltage (mV) < level4 * PowerChargerGetVoltageScale() */
    POWER_BATT_LEVEL3              /* when voltage (mV) < level5 * PowerChargerGetVoltageScale() */
}power_battery_level;

/*!
    @brief Power Library Configuration
*/
typedef struct
{
    power_vref_config       vref;       /* VREF Config */
    power_vbat_config       vbat;       /* Battery config */
    power_vthm_config       vthm;       /* Thermistor config */
    power_vchg_config       vchg;       /* Charger config */
    power_chg_prg_mon_config   chg_prg_mon;   /* Charger progress monitor config */
} power_config;

/*!
    @brief Contains voltage and level
*/
typedef struct
{
    uint16 voltage;
    uint8  level;
} voltage_reading;

/*!
    @brief This message is returned when the battery and charger monitoring 
    subsystem has been initialised.
*/
typedef struct
{
    bool            success;
    voltage_reading vthm;
    voltage_reading vbat;
    voltage_reading vchg;
    power_charger_state state;
} POWER_INIT_CFM_T;

/*!
    @brief This message is sent to App to indicate the battery voltage level and its value.
*/
typedef struct
{
    voltage_reading vbat;
} POWER_BATTERY_VOLTAGE_IND_T;

/*!
    @brief This message is sent to App to indicate the temperature level and its value.
*/
typedef struct
{
    voltage_reading vthm;
} POWER_BATTERY_TEMPERATURE_IND_T;

/*!
    @brief This message is sent to App to indicate the charger voltage level and its value.
*/
typedef struct
{
    voltage_reading vchg;
} POWER_CHARGER_VOLTAGE_IND_T;

/*!
    @brief This message is sent to App to indicate the charger voltage level for dynamic current limiting.
*/
typedef struct
{
    uint16 voltage;
} POWER_CHARGER_CURRENT_LIMITING_IND_T;

/*!
    @brief This message is sent to App to indicate the charging state.
*/
typedef struct
{
    power_charger_state   state;
} POWER_CHARGER_STATE_IND_T;

/*!
    @brief This function will initialise the battery monitoring and charging
           sub-system. The application will receive a POWER_INIT_CFM message
           from the library indicating the initialisation status.

    @param clientTask The task that will handle power messages.

    @param power_data Configuration data for the power library.

    @param pmu_mon_config PMU monitoring configuration data.
*/
void PowerInit(Task clientTask, const power_config *power_data, const power_pmu_temp_mon_config *pmu_mon_config);


/*!
    @brief This function will close the battery monitoring and charging sub-system.
*/
void PowerClose(void);


/*!
    @brief Get the battery voltage and gas gauge level.

    @param vbat Container for battery voltage and gas gauge level.
 
    @return bool TRUE if reading available, FALSE otherwise.
*/
bool PowerBatteryGetVoltage(voltage_reading* vbat);


/*!
    @brief Get the battery thermistor voltage and temperature region.

    @param vthm Container for thermistor voltage and temperature region.

    @return bool TRUE if reading available, FALSE otherwise.
*/
bool PowerBatteryGetTemperature(voltage_reading *vthm);


/*!
    @brief Get the charger voltage and level.

    @param vchg Container for charger voltage and level.

    @return bool TRUE if reading available, FALSE otherwise.
*/
bool PowerChargerGetVoltage(voltage_reading* vchg);


/*!
    @brief Enable/disable the charger status monitoring. Monitoring is enabled by
           default but is disabled when charger detach is detected. It is up
           to the application to re-enable charger monitoring when the charger
           is attached.
*/
void PowerChargerMonitor(void);


/*!
    @brief Enable/disable the charger.

    @param enable Requested charger enable state.
*/
void PowerChargerEnable(bool enable);


/*!
    @brief Set the fast charge current.

    @param current Requested fast charge current.
*/
void PowerChargerSetCurrent(uint16 current);


/*!
    @brief Configure boost settings for BlueCore. By default this is disabled,
           but can be configured to draw boost current from internal/external
           transistor. Boost charge will be enabled on entering fast charge
           unless the boost cycle has already completed. The boost cycle is
           reset on charger attach.

    Not available on Hydracore devices.

    @param boost Requested boost setting.
*/
void PowerChargerSetBoost(power_boost_enable boost);


/*!
    @brief Set the termination voltage.

    @param vterm Requested termination voltage.

    @return bool TRUE, except on BlueCore this will return FALSE if requested
                 termination voltage is higher than the configured termination
                 voltage.
*/
bool PowerChargerSetVterm(uint16 vterm);

/*!
    @brief Gets the voltage scalar for the charger version.

    @return uint16 The voltage scalar.
*/
uint16 PowerChargerGetVoltageScale(void);


/*!
    @brief Set suppress LED0 whilst charging.

    @param suppress Requested suppress state.
*/
void PowerChargerSuppressLed0(bool suppress);


/*!
    @brief Configure the charger with initial configuration.
*/
void PowerChargerConfigure(void);


/*!
    @brief Get charger enabled state.

    @return bool TRUE if charging, FALSE otherwise.
*/
bool PowerChargerEnabled(void);

/*!
    @brief Determines charger connected status from underlying hardware. Can be called
    before Power library is initialised.

    @return bool True if the charger is connected, false otherwise.
*/
bool PowerChargerConnected(void);

/*!
    @brief Determines if the charger is (potentially) drawing full current from underlying hardware.
    Can be called before Power library is initialised.

    @return bool True if the charger is drawing full current, false otherwise.
*/
bool PowerChargerFullCurrent(void);

/*!
    @brief Determines if the charger is fast charging from underlying hardware.
    Can be called before Power library is initialised.

    @return bool True if the charger is fast charging, false otherwise.
*/
bool PowerChargerFastCharge(void);

/*!
    @brief Determines if the charger is connected but disabled from underlying hardware.
    Can be called before Power library is initialised.

    @return bool True if the charger is connected but disabled, false otherwise.
*/
bool PowerChargerDisabled(void);

/*!
    @brief Register a task to handle messages from the charger hardware.

    @param task This task will receive messages with charger hardware state changes.

*/
void PowerChargerRegisterTask(Task task);

/*!
    @brief Start monitoring charging cycle progress.
*/
void PowerMonitorStartChargerProgressMonitor(void);

/*!
    @brief Stop monitoring charging cycle progress.
*/
void PowerMonitorStopChargerProgressMonitor(void);


/*!
    @brief Monitor charger voltage whilst current limiting as per USB Battery
           Charging v1.2.

    @param vchg_reading Current charger voltage reading.
*/
void PowerCurrentLimitingHandleMonitorReq(uint16 vchg_reading);


/*!
    @brief Start monitoring charger voltage for current limiting as per USB Battery
           Charging v1.2.
*/
void PowerCurrentLimitingStart(void);

/*!
    @brief Stop monitoring charger voltage for current limiting as per USB Battery
           Charging v1.2.
*/
void PowerCurrentLimitingStop(void);

/*!
    @brief Handle charger headroom error whilst monitoring charger voltage for
           current limiting as per USB Battery Charging v1.2.
*/
void PowerCurrentLimitingHandleHeadroomError(void);

/*!
    @brief Check if battery is in over-voltage condition.

    @return bool TRUE is battery is in over-voltage condition, FALSE otherwise.
*/
bool PowerBatteryIsOverVoltage(void);


/*!
    @brief Check if battery has recovered from over-voltage condition.

    @param The current battery voltage.

    @return bool TRUE if the battery is in the over-voltage condition and the
            the battery voltage is now less than the termination voltage,
            FALSE otherwise.
*/
bool PowerBatteryIsRecoveredFromOverVoltage(voltage_reading vbat);

/*!
    @brief Check the battery status at boot time.

    @return charger_battery_status.
*/
charger_battery_status PowerBatteryGetStatusAtBoot(void);

/*!
    @brief Service the Quick Charge interrupt.
*/
void PowerQuickChargeServiceInterrupt(void);

#endif     /* POWER_H_ */

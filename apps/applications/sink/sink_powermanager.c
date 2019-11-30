/*
Copyright (c) 2005 - 2019 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    Module responsible for managing the battery monitoring and battery 
    charging functionaility
************************************************************************/
#include "sink_private_data.h"
#include "sink_main_task.h"
#ifdef ENABLE_BISTO
#include "sink2bisto.h"
#endif
#include "sink_powermanager.h"
#include "sink_statemanager.h"
#include "sink_led_manager.h"
#include "sink_audio.h"
#include "sink_pio.h"
#include "sink_csr_features.h"
#include "sink_led_manager.h"
#include "sink_led_err.h"
#include "sink_leds.h"
#include "sink_debug.h"
#include "sink_display.h"
#include "sink_config.h"
#include "sink_audio_routing.h"
#include "gain_utils.h"
#include "sink_volume.h"
#include "sink_usb.h"
#include "sink_bootmode.h"

#include <app/charger/charger_if.h>
#include <psu.h>
#include <vm.h>
#include <boot.h>
#include <ps.h>
#include <config_store.h>
#include <adc.h>
#include <audio.h>

#ifdef DEBUG_POWER
    #define PM_DEBUG(x) {printf x;}
#else
    #define PM_DEBUG(x) 
#endif

#ifdef ENABLE_BATTERY_OPERATION

#define FAST_CHARGE_RATIO    ONE_HUNDRED_PERCENT
#define MV_IN_V                             1000
#define NOT_DRIVEN           255

/* Sink Power Global data requirement referance */

typedef struct __sink_power_global_data_t
{
    unsigned                battery_state:3;
    unsigned                lbipmEnable:1; /* enable Low Battery Intelligent Power Management feature */
    unsigned                vsel_fast_charge:1;
    unsigned                peer_slave_low_bat:1;
    unsigned                low_batt_limit_wait_for_charger:1; 
    unsigned                have_charger_error:1;
    unsigned                unused1:8;
#ifdef ENABLE_CHARGER_V2
    unsigned                ext_fast:1;
    unsigned                ext_resistor_value:10;
    unsigned                charger_pin_state:1;
    unsigned                unused2:4;

    uint16                  trickle_charge_timeout;
    uint16                  precharge_timeout;
    uint16                  fast_charge_timeout;
    uint16                  default_vfloat;
#endif

    unsigned                unused3:8;
    unsigned                current_external_devices:8;
}sink_power_global_data_t;

#include "sink_power_manager_config_def.h"
#include "sink_power_manager_vref_config_def.h"
#include "sink_power_manager_vbat_config_def.h"
#include "sink_power_manager_vthm_config_def.h"
#include "sink_power_manager_vchg_config_def.h"
#include "sink_power_manager_pmu_monitor_config_def.h"
#include "sink_power_manager_charger_configuration_config_def.h"
#include "sink_power_manager_charger_bc12_config_def.h"

static const sink_charge_setting default_charge_setting =
            {{FALSE, power_boost_disabled, vsel_chg, FALSE, FALSE, 0, 0},
            {vterm_default, 0, 0, 0}};

static void powerManagerHandleVbat(voltage_reading vbat, battery_level_source source);
static void powerManagerGetMinandMaxBatteryLimit(sink_battery_limits *bLimits);
static void powerManagerGetChargerSetting(uint16 level,sink_charge_setting *cSettings);
static void powerManagerGetBatterySetting(uint16 level,sink_battery_setting *bSetting);
static void powerManagerSetBatteryLevel(power_battery_level level);
#ifdef ENABLE_PEER
static void powerManagerUpdatePeerDeviceLowBatt(unsigned limit);
#endif /* ENABLE_PEER*/
#ifdef ENABLE_CHARGER_V2
static void powerManagerStopChargerStuckTimeout(void);
#else
#define powerManagerStopChargerStuckTimeout()  ((void)(0))
#endif
static sink_power_global_data_t power_gdata;
#define GPOWER power_gdata

/****************************************************************************
NAME
    powerManagerSetChargerPin

DESCRIPTION
    Store state of charger connected/disconnected pin.

RETURNS
    void
*/
static void powerManagerSetChargerPinState(bool enabled)
{
#ifdef ENABLE_CHARGER_V2
    GPOWER.charger_pin_state = enabled;
#else
    UNUSED(enabled);
#endif
}

/****************************************************************************
NAME
    powerManagerChargerPin

DESCRIPTION
    Return state of charger connected/disconnected pin.

RETURNS
    TRUE if charger pin connected, false otherwise.
*/
static bool powerManagerChargerPinState(void)
{
#ifdef ENABLE_CHARGER_V2
    return GPOWER.charger_pin_state;
#else
    return TRUE;
#endif
}

/****************************************************************************
NAME    
    powerManagerPowerOff
    
DESCRIPTION
    Power off due to critical battery or temperature outside operational
    limits
    
RETURNS
    void
*/
static void powerManagerPowerOff(void)
{
    if(stateManagerGetState() == deviceLimbo)
        stateManagerUpdateLimboState();
    else
        MessageSend(&theSink.task, EventUsrPowerOff, 0);
}

/****************************************************************************
NAME    
    powerManagerGetMinandMaxBatteryLimit
    
DESCRIPTION
   Get min and max battery limit
    
RETURNS
    void
*/
static void  powerManagerGetMinandMaxBatteryLimit(sink_battery_limits *bLimits)
{
    uint16 level = 0;
    sink_battery_limits *battery_limits = bLimits;
    sink_power_settings_vbat_config_def_t *ps_config = NULL;
    
    if (configManagerGetReadOnlyConfig(SINK_POWER_SETTINGS_VBAT_CONFIG_BLK_ID, (const void **)&ps_config))
    {
#ifdef ENABLE_CHARGER_V2
        bLimits->max_battery_v = GPOWER.default_vfloat;
#else
        PsFullRetrieve(PSKEY_CHARGER_CALC_VTERM, &(bLimits->max_battery_v),
                PS_SIZE_ADJ(sizeof(bLimits->max_battery_v)));
#endif

        for (level = 0; level < POWER_MAX_VBAT_LIMITS; level++)
        {

            if (( ps_config->bat_events[level].event) == EventSysBatteryCritical)
            {
                battery_limits->min_battery_v = ps_config->bat_events[level].limit * POWER_VSCALE;
                break;
            }
        }
        configManagerReleaseConfig(SINK_POWER_SETTINGS_VBAT_CONFIG_BLK_ID);
    }
}

#ifdef ENABLE_CHARGER_V2

/****************************************************************************
NAME
    powerManagerStopChargerStuckTimeout

DESCRIPTION
      Handle change in charger status

RETURNS
    void
*/
static void powerManagerStopChargerStuckTimeout(void)
{
    PM_DEBUG(("PM: Stop charger stuck timeout\n"));
    MessageCancelAll(&theSink.task, EventSysChargerError);
}

/****************************************************************************
NAME
    powerManagerConvertTerminationVoltage

DESCRIPTION
    Convert the termination voltage from an enumerated value,
    as specified in the configuration tool to physical values specified in
    charger_if.h.

RETURNS
    Termination voltage.

*/
static uint16 powerManagerConvertTerminationVoltage(unsigned short config_termination_voltage)
{
    switch (config_termination_voltage)
    {
        case 0:     return CHARGER_TERM_MILLIVOLT_3650;
        case 1:     return CHARGER_TERM_MILLIVOLT_3700;
        case 2:     return CHARGER_TERM_MILLIVOLT_3750;
        case 3:     return CHARGER_TERM_MILLIVOLT_3800;
        case 4:     return CHARGER_TERM_MILLIVOLT_3850;
        case 5:     return CHARGER_TERM_MILLIVOLT_3900;
        case 6:     return CHARGER_TERM_MILLIVOLT_3950;
        case 7:     return CHARGER_TERM_MILLIVOLT_4000;
        case 8:     return CHARGER_TERM_MILLIVOLT_4050;
        case 9:     return CHARGER_TERM_MILLIVOLT_4100;
        case 10:    return CHARGER_TERM_MILLIVOLT_4150;
        case 11:    return CHARGER_TERM_MILLIVOLT_4200;
        case 12:    return CHARGER_TERM_MILLIVOLT_4250;
        case 13:    return CHARGER_TERM_MILLIVOLT_4300;
        case 14:    return CHARGER_TERM_MILLIVOLT_4350;
        case 15:    return CHARGER_TERM_MILLIVOLT_4400;
        case 31:    return CHARGER_TERM_MILLIVOLT_3650;
        default:
            Panic();
            return CHARGER_TERM_MILLIVOLT_3650;
    }
}

/****************************************************************************
NAME
    powerManagerConvertStandbyFastHysteresis

DESCRIPTION
    Convert the standby fast hysteresis from an enumerated value,
    as specified in the configuration tool to physical values specified in
    charger_if.h.

RETURNS
    Standby fast hysteresis.

*/
static uint16 powerManagerConvertStandbyFastHysteresis(unsigned short config_standby_fast_hysteresis)
{
    switch (config_standby_fast_hysteresis)
    {
        case 0:     return CHARGER_STANDBY_FAST_HYSTERESIS_MILLIVOLT_100;
        case 1:     return CHARGER_STANDBY_FAST_HYSTERESIS_MILLIVOLT_150;
        case 2:     return CHARGER_STANDBY_FAST_HYSTERESIS_MILLIVOLT_200;
        case 3:     return CHARGER_STANDBY_FAST_HYSTERESIS_MILLIVOLT_250;
        default:
            Panic();
            return CHARGER_STANDBY_FAST_HYSTERESIS_MILLIVOLT_100;
    }
}

/****************************************************************************
NAME
    powerManagerConvertPreFastChargeThreshold

DESCRIPTION
    Convert the pre fast charge threshold from an enumerated value,
    as specified in the configuration tool to physical values specified in
    charger_if.h.

RETURNS
    Pre fast charge threshold.

*/
static uint16 powerManagerConvertPreFastChargeThreshold(unsigned short config_pre_charge_threshold)
{
    switch (config_pre_charge_threshold)
    {
        case 0:     return CHARGER_PRE_FAST_THRESHOLD_MILLIVOLT_2500;
        case 1:     return CHARGER_PRE_FAST_THRESHOLD_MILLIVOLT_2900;
        case 2:     return CHARGER_PRE_FAST_THRESHOLD_MILLIVOLT_3000;
        case 3:     return CHARGER_PRE_FAST_THRESHOLD_MILLIVOLT_3100;
        default:
            Panic();
            return CHARGER_PRE_FAST_THRESHOLD_MILLIVOLT_2500;
    }
}

/****************************************************************************
NAME
    powerManagerConvertCurrentTerminationPercentage

DESCRIPTION
    Convert the current termination percentage from an enumerated value,
    as specified in the configuration tool to physical values specified in
    charger_if.h.

RETURNS
    Current terminations percentage.

*/
static uint16 powerManagerConvertCurrentTerminationPercentage(unsigned short config_current_termination_percentage)
{
    switch (config_current_termination_percentage)
    {
        case 0:     return CHARGER_ITERM_CTRL_PERCENT_10;
        case 1:     return CHARGER_ITERM_CTRL_PERCENT_20;
        case 2:     return CHARGER_ITERM_CTRL_PERCENT_30;
        case 3:     return CHARGER_ITERM_CTRL_PERCENT_40;
        default:
            Panic();
            return CHARGER_ITERM_CTRL_PERCENT_10;
    }
}

/****************************************************************************
NAME
    powerManagerConvertChargerStateChangeDebounce

DESCRIPTION
    Convert the charger state change debounce time from an enumerated value,
    as specified in the configuration tool to physical values specified in
    charger_if.h.

RETURNS
    Charger state change debounce.

*/
static uint16 powerManagerConvertChargerStateChangeDebounce(unsigned short config_charger_state_change_debounce)
{
    switch (config_charger_state_change_debounce)
    {
        case 0:     return CHARGER_STATE_CHANGE_DEBOUNCE_MILLISEC_0;
        case 1:     return CHARGER_STATE_CHANGE_DEBOUNCE_MILLISEC_4;
        case 2:     return CHARGER_STATE_CHANGE_DEBOUNCE_MILLISEC_32;
        case 3:     return CHARGER_STATE_CHANGE_DEBOUNCE_MILLISEC_128;
        default:
            Panic();
            return CHARGER_STATE_CHANGE_DEBOUNCE_MILLISEC_0;
    }
}
#endif

/****************************************************************************
NAME    
    powerManagerGetChargerSetting
    
DESCRIPTION
   Retrieve Charger settings for a given level
    
RETURNS
   void

*/
static void powerManagerGetChargerSetting(uint16 level,sink_charge_setting *cSettings)
{
    charger_control_config_def_t *read_config = NULL;

    /* Get the Config Data */
    if (configManagerGetReadOnlyConfig(CHARGER_CONTROL_CONFIG_BLK_ID, (const void **)&read_config))
    {
        if(cSettings)
        {
            cSettings->current.charge = read_config->temperature_regions[level].charge;
#ifdef ENABLE_CHARGER_V2
            cSettings->current.boost = power_boost_disabled;
            cSettings->termination.voltage = powerManagerConvertTerminationVoltage(read_config->temperature_regions[level].termination_voltage);
#else
            cSettings->current.boost = read_config->temperature_regions[level].boost;
            cSettings->termination.voltage = read_config->temperature_regions[level].termination_voltage;
#endif
            cSettings->current.vsel = read_config->temperature_regions[level].vsel;
            cSettings->current.power_off = read_config->temperature_regions[level].power_off;
            cSettings->current.disable_leds = read_config->temperature_regions[level].disable_leds;
            cSettings->current.current = read_config->temperature_regions[level].current;
            cSettings->termination.type = read_config->temperature_regions[level].termination_type;
        }
        configManagerReleaseConfig(CHARGER_CONTROL_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME    
    powerManagerGetBatterySetting
    
DESCRIPTION
   Retrieve Battery settings for a given level
    
RETURNS
    void
*/
static void powerManagerGetBatterySetting(uint16 level,sink_battery_setting *bSetting)
{
    sink_power_settings_vbat_config_def_t *read_config = NULL;
   
    /* Get the Config Data */
    if (configManagerGetReadOnlyConfig(SINK_POWER_SETTINGS_VBAT_CONFIG_BLK_ID, (const void **)&read_config))
    {
        if(bSetting)
        {
            bSetting->event = read_config->bat_events[level].event;
            bSetting->sources= read_config->bat_events[level].sources;
        }
        configManagerReleaseConfig(SINK_POWER_SETTINGS_VBAT_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME    
    powerManagerConfigInit
    
DESCRIPTION
   Retrieve Config Data and updates the global data
    
RETURNS
    bool
    Note:This function need to be removed once config LIB API is in place
*/
static bool powerManagerConfigInit(sink_pmu_mon_config_config_def_t **pmu_config, uint16 *pmu_read_size)
{  
    *pmu_read_size = 0;
    
    if (configManagerGetReadOnlyConfig(SINK_PMU_MON_CONFIG_CONFIG_BLK_ID, (const void **)pmu_config) == (sizeof(sink_pmu_mon_config_config_def_t)/sizeof(uint16)))
    {
        *pmu_read_size =(uint16)sizeof(sink_pmu_mon_config_config_def_t);
    }
    return TRUE;
}

/****************************************************************************
NAME
    updateVrefSettings

DESCRIPTION
    This function manipulates the Vref Config structure into power_config structure
    
RETURNS
    void
*/
static void updateVrefSettings(power_config* config, sink_power_settings_vref_config_def_t *vref_data)
{
    if(config && vref_data)
    {
        config->vref.adc.period_chg = D_SEC(vref_data->config_p_vref.p_config_vref.vref.adc.period_chg);
        config->vref.adc.period_no_chg = D_SEC(vref_data->config_p_vref.p_config_vref.vref.adc.period_no_chg);
        config->vref.adc.source = vref_data->config_p_vref.p_config_vref.vref.adc.source;
    }
}

/****************************************************************************
NAME
    updateVbatSettings

DESCRIPTION
    This function manipulates the Vbat Config structure into power_config structure

RETURNS
    void
*/
static void updateVbatSettings(power_config* config, sink_power_settings_vbat_config_def_t *vbat_data)
{
    uint16 level = 0;

    if(config && vbat_data)
    {
        config->vbat.adc.period_chg = D_SEC(vbat_data->config_p_vbat.p_config_vbat.vbat.adc.period_chg);
        config->vbat.adc.period_no_chg = D_SEC(vbat_data->config_p_vbat.p_config_vbat.vbat.adc.period_no_chg);
        config->vbat.adc.source = vbat_data->config_p_vbat.p_config_vbat.vbat.adc.source;

        /* store the critical voltage as the min battery voltage */
        for (level = 0; level <= POWER_MAX_VBAT_LIMITS; level++)
        {
             config->vbat.limits[level].limit = vbat_data->bat_events[level].limit;
             config->vbat.limits[level].notify_period = vbat_data->bat_events[level].notify_period;
        }

        config->vbat.ignore_increase_no_chg = vbat_data->config_p_vbat.p_config_vbat.vbat.ignore_increase_no_chg;
    }
}

/****************************************************************************
NAME
    updateVthmSettings

DESCRIPTION
    This function manipulates the Vthm Config structure into power_config structure

RETURNS
    void
*/
static void updateVthmSettings(power_config* config, sink_power_settings_vthm_config_def_t *vthm_data)
{
    if(config && vthm_data)
    {
        config->vthm.adc.period_chg = D_SEC(vthm_data->config_p_vthm.p_config_vthm.vthm.adc.period_chg);
        config->vthm.adc.period_no_chg = D_SEC(vthm_data->config_p_vthm.p_config_vthm.vthm.adc.period_no_chg);
        config->vthm.adc.source = vthm_data->config_p_vthm.p_config_vthm.vthm.adc.source;

        /*Set the flags about how to drive the thermistor */
        config->vthm.drive_ics = (vthm_data->config_p_vthm.p_config_vthm.vthm.ics != NOT_DRIVEN);
        config->vthm.drive_pio = (vthm_data->config_p_vthm.p_config_vthm.vthm.pio != NOT_DRIVEN);

        /* Can't drive thermistor with both PIO and internal current source at the same time! */
        PanicFalse(!(config->vthm.drive_ics && config->vthm.drive_pio));

        config->vthm.pio = vthm_data->config_p_vthm.p_config_vthm.vthm.pio;
        config->vthm.ics = vthm_data->config_p_vthm.p_config_vthm.vthm.ics;

        config->vthm.delay = vthm_data->config_p_vthm.p_config_vthm.vthm.delay;

        config->vthm.raw_limits = vthm_data->config_p_vthm.p_config_vthm.vthm.raw_limits;
        memcpy(&config->vthm.limits,&vthm_data->config_p_vthm.p_config_vthm.vthm.limits, POWER_MAX_VTHM_LIMITS*sizeof(config->vthm.limits[0]));
    }
}

/****************************************************************************
NAME
    updateVchgSettings

DESCRIPTION
    This function manipulates the Vchg Config structure into power_config structure

RETURNS
    void
*/
static void updateVchgSettings(power_config* config, sink_power_settings_vchg_config_def_t *vchg_data)
{
    if(config && vchg_data)
    {
        config->vchg.adc.period_chg = D_SEC(vchg_data->config_p_vchg.p_config_vchg.vchg.adc.period_chg);
        config->vchg.adc.period_no_chg = D_SEC(vchg_data->config_p_vchg.p_config_vchg.vchg.adc.period_no_chg);
        config->vchg.adc.source = vchg_data->config_p_vchg.p_config_vchg.vchg.adc.source;

        config->vchg.non_current_limit_period_chg = D_SEC(vchg_data->config_p_vchg.p_config_vchg.vchg.adc.period_chg);
    }
}

/****************************************************************************
NAME
updateChargerSafetySettings

DESCRIPTION
    This function manipulates the Charger settings into the power_config structure

RETURNS
    void
*/
#ifdef ENABLE_CHARGER_V2
static void updateChargerSafetySettings(void)
{
    charger_safety_structure_t *data = NULL;
    uint16 size = 0;

    size = configManagerGetReadOnlyConfig(CHARGER_SAFETY_CONFIG_BLK_ID, (const void **)&data);

    if (size)
    {
        GPOWER.trickle_charge_timeout           = data->trickle_charge_timeout;
        GPOWER.precharge_timeout                = data->precharge_timeout;
        GPOWER.fast_charge_timeout              = data->fast_charge_timeout;
        configManagerReleaseConfig(CHARGER_SAFETY_CONFIG_BLK_ID);
    }
}
#else
#define updateChargerSafetySettings()
#endif

/****************************************************************************
NAME
updateChargerModeVoltageSelectorSettings

DESCRIPTION
    This function manipulates the Charger Mode Voltage Selector settings into
    the power manager data structure

RETURNS
    void
*/
#ifdef ENABLE_CHARGER_V2
static void updateChargerModeVoltageSelectorSettings(void)
{
    voltage_selectors_structure_t *data = NULL;
    uint16 size = 0;

    size = configManagerGetReadOnlyConfig(VOLTAGE_SELECT_CONFIG_BLK_ID, (const void **)&data);

    if (size)
    {
        GPOWER.vsel_fast_charge    = data->vsel_fast_charge;
        configManagerReleaseConfig(VOLTAGE_SELECT_CONFIG_BLK_ID);
    }
}
#else
static void updateChargerModeVoltageSelectorSettings(void)
{
    GPOWER.vsel_fast_charge    = vsel_chg;
}
#endif

/****************************************************************************
NAME
updateChargerExternalResistorSettings

DESCRIPTION
    This function manipulates the Charger settings into the power_config structure

RETURNS
    void
*/
#ifdef ENABLE_CHARGER_V2
static void updateChargerExternalResistorSettings(power_config* config)
{
    external_charger_structure_t *data = NULL;
    uint16 size = 0;

    if(config)
    {
        size = configManagerGetReadOnlyConfig(EXTERNAL_CHARGER_CONFIG_BLK_ID, (const void **)&data);

        if (size)
        {
            config->vchg.ext_fast               = data->ext_fast;
            config->vchg.ext_resistor_value     = data->ext_resistor_value;
            configManagerReleaseConfig(EXTERNAL_CHARGER_CONFIG_BLK_ID);
        }
    }
}
#else
#define updateChargerExternalResistorSettings(config)
#endif

/****************************************************************************
NAME
updateChargerCurrentSettings

DESCRIPTION
    This function manipulates the Charger settings into the power_config structure

RETURNS
    void
*/
#ifdef ENABLE_CHARGER_V2
static void updateChargerCurrentSettings(power_config* config)
{
    charger_currents_config_def_t *data = NULL;
    uint16 size = 0;

    if(config)
    {
        size = configManagerGetReadOnlyConfig(CHARGER_CURRENTS_CONFIG_BLK_ID, (const void **)&data);

        if (size)
        {
            config->vchg.trickle_charge_current    = data->mode_currents.trickle_charge_current;
            config->vchg.precharge_current         = data->mode_currents.precharge_current;

            configManagerReleaseConfig(CHARGER_CURRENTS_CONFIG_BLK_ID);
        }
    }
}
#else
#define updateChargerCurrentSettings(config)
#endif

/****************************************************************************
NAME
updateChargerCurrentDeterminationSettings

DESCRIPTION
    This function manipulates the Charger Determination settings into the power_config structure

RETURNS
    void
*/
#ifdef ENABLE_CHARGER_V2
static void updateChargerCurrentDeterminationSettings(power_config* config)
{
    bc_1_2_config_def_t *data = NULL;
    uint16 size = 0;

    if(config)
    {
        size = configManagerGetReadOnlyConfig(BC_1_2_CONFIG_BLK_ID, (const void **)&data);

        if (size)
        {
            PanicFalse(data->charger_current_determination.current_limit_threshold < data->charger_current_determination.current_recovery_threshold);

            config->vchg.current_limit_threshold    = data->charger_current_determination.current_limit_threshold;
            config->vchg.current_limit_recovery_threshold = data->charger_current_determination.current_recovery_threshold;
            config->vchg.positive_current_step      = data->charger_current_determination.positive_current_step;
            config->vchg.negative_current_step      = data->charger_current_determination.negative_current_step;

            config->vchg.current_limit_period_chg   = data->charger_current_determination.current_limit_read_period;

            configManagerReleaseConfig(CHARGER_CURRENTS_CONFIG_BLK_ID);
        }
    }
}
#else
#define updateChargerCurrentDeterminationSettings(config)
#endif

/****************************************************************************
NAME
updateChargerProgressSettings

DESCRIPTION
    This function manipulates the Charger settings into the power_config structure

RETURNS
    void
*/
static void updateChargerProgressSettings(power_config* config)
{
    charger_progress_structure_t *data = NULL;
    uint16 size = 0;

    if(config)
    {
        size = configManagerGetReadOnlyConfig(CHARGER_PROGRESS_CONFIG_BLK_ID, (const void **)&data);

        if (size)
        {
#ifdef ENABLE_CHARGER_V2
            GPOWER.default_vfloat                   = powerManagerConvertTerminationVoltage(data->default_vfloat);
            config->vchg.standby_fast_hysteresis    = powerManagerConvertStandbyFastHysteresis(data->standby_fast_hysteresis);
            config->vchg.debounce_time              = powerManagerConvertChargerStateChangeDebounce(data->debounce_time);
            config->vchg.iterm_ctrl                 = powerManagerConvertCurrentTerminationPercentage(data->iterm_ctrl);
            config->vchg.pre_fast_threshold         = powerManagerConvertPreFastChargeThreshold(data->pre_fast_threshold);
            config->chg_prg_mon.adc.period_chg      = D_SEC(data->period_charger_progress);
            config->chg_prg_mon.adc.period_no_chg   = 0;
            config->chg_prg_mon.adc.source          = adcsel_chg_mon;
#else
            config->vchg.limit                      = data->limit;
#endif
            GPOWER.current_external_devices         = data->current_external_devices;
            configManagerReleaseConfig(CHARGER_PROGRESS_CONFIG_BLK_ID);
        }
    }
}

/****************************************************************************
NAME    
updatePmuMonSettings    
DESCRIPTION
    This function manipulates the PMUMon Config structure into power_pmu_temp_mon_config structure 
    
RETURNS
    void
*/
static void updatePmuMonSettings(power_pmu_temp_mon_config* config, sink_pmu_mon_config_config_def_t* data)
{
    config->charger_i_step = data->charger_i_step;
    config->decr_i_temp = data->decr_i_temp;
    config->max_charge_i = data->max_charge_i;
    config->min_charge_i = data->min_charge_i;
    config->monitor_period_active = data->monitor_period_active;
    config->monitor_period_idle = data->monitor_period_idle;
    config->monitor_period_nopwr = data->monitor_period_nopwr;
    config->no_incr_i_temp = data->no_incr_i_temp;
}

/****************************************************************************
NAME
validateChargerConfig
DESCRIPTION
    This function ensures that the charger cannot be used inappropriately.

RETURNS
    void
*/
static void validateChargerConfig(power_config* config)
{
    int i;
    uint16 max_configured_charge_current, max_charger_mode_current;
    int num_regions;

    num_regions = sizeof(charger_control_config_def_t) / sizeof(charger_configuration_settings_t);
    max_configured_charge_current = 0;
    for (i = 0; i < num_regions; ++i)
    {
        sink_charge_setting setting;
        memset(&setting,0,sizeof(sink_charge_setting));

        powerManagerGetChargerSetting(i,&setting);
        if (max_configured_charge_current < setting.current.current)
        {
            max_configured_charge_current = setting.current.current;
        }
    }

    PanicZero(config);

#ifdef ENABLE_CHARGER_V2
    max_charger_mode_current = (MAX_SENSE_RESISTOR_MV * MV_IN_V) / config->vchg.ext_resistor_value;
    max_charger_mode_current = MIN(max_charger_mode_current, MAX_EXTERNAL_CURRENT);
    max_charger_mode_current = config->vchg.ext_fast ? max_charger_mode_current : MAX_INTERNAL_CURRENT;

    PanicFalse(config->vchg.trickle_charge_current <= max_charger_mode_current);
    PanicFalse(config->vchg.precharge_current <= max_charger_mode_current);
    PanicFalse(config->vchg.trickle_charge_current <= config->vchg.precharge_current);

    PanicFalse(max_configured_charge_current <= max_charger_mode_current);

#else
    max_charger_mode_current = MAX_INTERNAL_CURRENT;

    PanicFalse(max_configured_charge_current <= max_charger_mode_current);
#endif
}

/****************************************************************************
NAME    
powerManagerInit    
DESCRIPTION
    Configure power management
    
RETURNS
    TRUE: Success ,FALSE: Failure
*/
bool powerManagerInit(void)
{
    uint16 vref_size = 0;
    uint16 vbat_size = 0;
    uint16 vthm_size = 0;
    uint16 vchg_size = 0;
    power_config config;
    power_pmu_temp_mon_config pmu;
    sink_power_settings_vref_config_def_t *ps_vref_config = NULL;
    sink_power_settings_vbat_config_def_t *ps_vbat_config = NULL;
    sink_power_settings_vthm_config_def_t *ps_vthm_config = NULL;
    sink_power_settings_vchg_config_def_t *ps_vchg_config = NULL;
    sink_pmu_mon_config_config_def_t *pmu_config = NULL;
    uint16 pmu_read_size = 0;
    PM_DEBUG(("PM: Power Library Init\n"));

    /* Tidy up the Sink Power Manager Global Data Structure */
    memset(&GPOWER, 0, sizeof(GPOWER));
        
    /* Read configuration data */
    vref_size = configManagerGetReadOnlyConfig(SINK_POWER_SETTINGS_VREF_CONFIG_BLK_ID, (const void **)&ps_vref_config);
    vbat_size = configManagerGetReadOnlyConfig(SINK_POWER_SETTINGS_VBAT_CONFIG_BLK_ID, (const void **)&ps_vbat_config);
    vthm_size = configManagerGetReadOnlyConfig(SINK_POWER_SETTINGS_VTHM_CONFIG_BLK_ID, (const void **)&ps_vthm_config);
    vchg_size = configManagerGetReadOnlyConfig(SINK_POWER_SETTINGS_VCHG_CONFIG_BLK_ID, (const void **)&ps_vchg_config);
    
    /* Initialize config data  */
    powerManagerConfigInit(&pmu_config, &pmu_read_size);
    
    powerManagerSetBatteryLevel(POWER_BATT_CRITICAL);

    updateVrefSettings(&config, ps_vref_config);
    updateVbatSettings(&config, ps_vbat_config);
    updateVthmSettings(&config, ps_vthm_config);
    updateVchgSettings(&config, ps_vchg_config);

    updateChargerProgressSettings(&config);
    updateChargerSafetySettings();
    updateChargerCurrentSettings(&config);
    updateChargerCurrentDeterminationSettings(&config);
    updateChargerExternalResistorSettings(&config);
    updateChargerModeVoltageSelectorSettings();
    updatePmuMonSettings(&pmu, pmu_config);
    validateChargerConfig(&config);

    if(pmu_read_size)
        PowerInit(&theSink.task,&config,&pmu);
    else
        PowerInit(&theSink.task,&config,NULL);
    
    if(vref_size)
        configManagerReleaseConfig(SINK_POWER_SETTINGS_VREF_CONFIG_BLK_ID);
    if(vbat_size)
        configManagerReleaseConfig(SINK_POWER_SETTINGS_VBAT_CONFIG_BLK_ID);
    if(vthm_size)
        configManagerReleaseConfig(SINK_POWER_SETTINGS_VTHM_CONFIG_BLK_ID);
    if(vchg_size)
        configManagerReleaseConfig(SINK_POWER_SETTINGS_VCHG_CONFIG_BLK_ID);

    configManagerReleaseConfig(SINK_PMU_MON_CONFIG_CONFIG_BLK_ID);
    
    PowerChargerConfigure();

    return TRUE;
}


/****************************************************************************
NAME    
    powerManagerChargerConnected
    
DESCRIPTION
    This function is called when the charger is plugged into the device
    
RETURNS
    void
*/
void powerManagerChargerConnected( void )
{
    PM_DEBUG(("PM: Charger has been connected\n"));
    /* Monitor charger state */
    PowerChargerMonitor();
    /* notify the audio plugin of the new power state */
    AudioSetPower(POWER_BATT_LEVEL3);
    /* reset flag to indicate that the vol cap limit can now be removed if applicable */
    GPOWER.low_batt_limit_wait_for_charger = FALSE;
    powerManagerSetChargerPinState(TRUE);
}


/****************************************************************************
NAME    
    powerManagerChargerDisconnected
    
DESCRIPTION
    This function is called when the charger is unplugged from the device
    
RETURNS
    void
*/
void powerManagerChargerDisconnected( void )
{
    PM_DEBUG(("PM: Charger has been disconnected\n"));  
    /* Immediately update charger */
    PowerChargerMonitor();
    powerManagerUpdateChargeCurrent();
    /* Cancel current LED indication */
    MessageSend(&theSink.task, EventSysCancelLedIndication, 0);
    /* Restore default bootmode */
    usbSetBootMode(BOOTMODE_DEFAULT);
    powerManagerSetChargerPinState(FALSE);
    powerManagerStopChargerStuckTimeout();
}


/****************************************************************************
NAME    
    powerManagerIsChargerConnected
    
DESCRIPTION
    This function is called by applications to check whether the charger has 
    been plugged into the device
    
RETURNS
    bool
*/
bool powerManagerIsChargerConnected(void)
{
    return PowerChargerConnected();
}

/****************************************************************************
NAME
    powerManagerIsChargerFullCurrent
    
DESCRIPTION
    This function is called by applications to check whether the charger is
    in (potentially) drawing full configured current.
    
RETURNS
    bool
*/
bool powerManagerIsChargerFullCurrent(void)
{
    return PowerChargerFullCurrent();
}


/****************************************************************************
NAME
    powerManagerIsVthmCritical
    
DESCRIPTION
    This function is called by applications to check whether the battery 
    temperature has reached critical level where the device must be powered
    off
    
RETURNS
    bool
*/
bool powerManagerIsVthmCritical(void)
{
    voltage_reading vthm;
    sink_charge_setting setting;
    memset(&setting,0,sizeof(sink_charge_setting));
    
    if(PowerBatteryGetTemperature(&vthm))
    {
         powerManagerGetChargerSetting(vthm.level,&setting);        
        /* Check the power_off setting for this temperature */
        PM_DEBUG(("PM: VTHM %s\n", setting.current.power_off ? "Critical" : "Okay"));
        return setting.current.power_off;
    }
    /* No temperature reading, wait until one available */
    PM_DEBUG(("PM: VTHM Unknown\n"));
    return FALSE;
}


/****************************************************************************
NAME    
    powerManagerReadVbat
    
DESCRIPTION
    Call this function to take an immediate battery reading and sent to AG.
    
RETURNS
    void
*/
void powerManagerReadVbat(battery_level_source source)
{
    voltage_reading reading;
    if(PowerBatteryGetVoltage(&reading))
        powerManagerHandleVbat(reading, source);
}


/****************************************************************************
NAME    
    powerManagerHandleVbatCritical
    
DESCRIPTION
    Called when the battery voltage is detected to be in critical state
*/
static void powerManagerHandleVbatCritical( void )
{
    PM_DEBUG(("PM: Battery Critical\n"));
    /* Reset low batt warning */
    powerManagerSetBatteryLevel(POWER_BATT_CRITICAL);
    /* Power Off */
    if(!powerManagerIsChargerConnected() || sinkDataAllowAutomaticPowerOffWhenCharging())
    {
        powerManagerPowerOff();
    }
}


/****************************************************************************
NAME    
    powerManagerHandleVbatLow
    
DESCRIPTION
    Called when the battery voltage is detected to be in Battery Low state
*/
static void powerManagerHandleVbatLow( void )
{
    sinkState lSinkState = stateManagerGetState ();
    bool batt_was_low = powerManagerIsVbatLow();

    PM_DEBUG(("PM: Battery Low\n"));
    if(powerManagerIsChargerConnected() || lSinkState == deviceLimbo)
    {
        powerManagerSetBatteryLevel(POWER_BATT_LEVEL0);
    }
    else
    {
        powerManagerSetBatteryLevel(POWER_BATT_LOW);
    }

    if(!batt_was_low || !powerManagerIsVbatLow())
    {
        /* update state indication to indicate low batt state change */
        LedManagerIndicateExtendedState();
    }
    
    AudioSetPower(powerManagerGetLBIPM());
}


/****************************************************************************
NAME    
    batteryNormal
    
DESCRIPTION
    Called when the battery voltage is detected to be in a Normal state
*/
static void powerManagerHandleVbatNormal(uint8 level)
{
    bool low_batt = powerManagerIsVbatLow();
    PM_DEBUG(("PM: Battery Normal %u\n", level));
    MessageSend(&theSink.task, EventSysBatteryOk, 0);
    
    /* If charger connected send a charger gas gauge message (these don't have any functional use but can be associated with LEDs/tones) */
    if (powerManagerIsChargerConnected())
        MessageSend(&theSink.task, (EventSysChargerGasGauge0+level), 0);
    
    /* reset any low battery warning that may be in place */
    powerManagerSetBatteryLevel(POWER_BATT_LEVEL0 + level);
    csr2csrHandleAgBatteryRequestRes(level);

    /* when changing from low battery state to a normal state, refresh the led state pattern
       to replace the low battery pattern should it have been shown */
    if(low_batt) LedManagerIndicateExtendedState();

    AudioSetPower(powerManagerGetLBIPM());
}


/****************************************************************************
NAME    
    powerManagerHandleVbat
    
DESCRIPTION
    Called when the battery voltage is detected to be in a Normal state
*/
static void powerManagerHandleVbat(voltage_reading vbat, battery_level_source source)
{
    /* Get the event associated with this battery level */
    sink_battery_setting setting;
    sinkEvents_t event = 0; 

    PM_DEBUG(("PM: Battery Voltage 0x%02X (%umV)\n", vbat.level, vbat.voltage));

    if (PowerBatteryIsRecoveredFromOverVoltage(vbat))
    {
        PM_DEBUG(("PM: Vbat Overvoltage finished\n"))
        if (!stateManagerIsShuttingDown())
            PowerChargerEnable(FALSE);
    }

    powerManagerGetBatterySetting(vbat.level,&setting);
    event = setting.event;  

    displayUpdateBatteryLevel(powerManagerIsChargerConnected());
#ifdef ENABLE_BISTO    
    BistoTargetBatteryUpdate();
#endif
    /* Send indication if not charging, not in limbo state and indication enabled for this source */
    if(!powerManagerIsChargerConnected() && (stateManagerGetState() != deviceLimbo) && (setting.sources & source))
    {
        PM_DEBUG(("PM: Sending Event 0x%X\n", event));
        MessageSend(&theSink.task, event, NULL);
    }
    
    switch(event)
    {
        case EventSysBatteryCritical:
            /* Always indicate critical battery */
            powerManagerHandleVbatCritical();
            usbSetVbatDead(TRUE);
            break;

        case EventSysBatteryLow:
            powerManagerHandleVbatLow();
            usbSetVbatDead(FALSE);
            break;

        case EventSysGasGauge0 :
        case EventSysGasGauge1 :
        case EventSysGasGauge2 :
        case EventSysGasGauge3 :
            powerManagerHandleVbatNormal(event - EventSysGasGauge0);
            usbSetVbatDead(FALSE);
            break;

        default:
            break;
    }
    /* determine if any action is required to reduce power consumption whilst charging */
      powerManagerLowGasGaugeAction(event);
}


/****************************************************************************
NAME
    powerManagerHandleChargeState
    
DESCRIPTION
    Called when the charger state changes
*/
static void powerManagerHandleChargeState(power_charger_state state)
{
    if(!LedManagerChargerTermLedOveride()) 
    {
        /* Generate new message based on the reported charger state */
        switch(state)
        {
            case power_charger_trickle:
            case power_charger_fast:
            case power_charger_boost_internal:
            case power_charger_boost_external:
                PM_DEBUG(("PM: Charge In Progress\n"));
                MessageSend(&theSink.task, EventSysChargeInProgress, 0);
            break;
            case power_charger_complete:
                PM_DEBUG(("PM: Charge Complete\n"));
                MessageSend(&theSink.task, EventSysChargeComplete, 0);
            break;
            case power_charger_disconnected:
                PM_DEBUG(("PM: Charger Disconnected\n"));
            break;
            case power_charger_disabled:
                PM_DEBUG(("PM: Charger Disabled\n"));
                MessageSend(&theSink.task, EventSysChargeDisabled, 0);
            break;            
            case power_charger_just_connected:
                PM_DEBUG(("PM: Charger just connected\n"));
                /* do nothing, wait until the next monitoring period to update LEDs etc. */
            break;
            default:
                PM_DEBUG(("PM: Charger Unhandled!\n"));
            break;
        }
    }
}


/*************************************************************************
NAME    
    handlePowerMessage
    
DESCRIPTION
    handles the Battery/Charger Monitoring Messages

RETURNS
    
*/

void handlePowerMessage( Task task, MessageId id, Message message )
{
    UNUSED(task);

    switch(id)
    {
        case POWER_INIT_CFM:
        {
            POWER_INIT_CFM_T* cfm = (POWER_INIT_CFM_T*)message;
            PM_DEBUG(("PM: Rcv POWER_INIT_CFM with success %u, state %u\n", cfm->success, cfm->state));
            if(!cfm->success) Panic();
            /* Update VBUS level */
            usbSetVbusLevel(cfm->vchg);
            /* Only indicate if low/critical (or intial reading feature enabled) */
            powerManagerHandleVbat(cfm->vbat, battery_level_initial_reading);
            /* Update charge current based on battery temperature */
            if (!powerManagerChargerSetup(&cfm->vthm))
            {
                /* and if the charger was left disabled then handle initial charge state 
                 * to notify the app that charger is disabled
                 */
                powerManagerHandleChargeState(cfm->state);
            }
        }
        break;

        case POWER_BATTERY_VOLTAGE_IND:
        {
            POWER_BATTERY_VOLTAGE_IND_T* ind = (POWER_BATTERY_VOLTAGE_IND_T*)message;
            PM_DEBUG(("PM: Rcv POWER_BATTERY_VOLTAGE_IND with voltage %umV, level %u\n", ind->vbat.voltage, ind->vbat.level));
            powerManagerHandleVbat(ind->vbat, battery_level_automatic);
            usbUpdateChargeCurrent();
        }
        break ;

        case POWER_CHARGER_VOLTAGE_IND:
        {
            PM_DEBUG(("PM: Rcv POWER_CHARGER_VOLTAGE_IND with voltage %umV, level %u\n", ((POWER_CHARGER_VOLTAGE_IND_T*)message)->vchg.voltage, ((POWER_CHARGER_VOLTAGE_IND_T*)message)->vchg.level));
            usbSetVbusLevel(((POWER_CHARGER_VOLTAGE_IND_T*)message)->vchg);
            usbUpdateChargeCurrent();
        }
        break;

        case POWER_CHARGER_CURRENT_LIMITING_IND:
        {
            PM_DEBUG(("PM: Rcv POWER_CHARGER_CURRENT_LIMITING_IND with voltage %umV\n", ((POWER_CHARGER_CURRENT_LIMITING_IND_T*)message)->voltage));    
            PowerCurrentLimitingHandleMonitorReq(((POWER_CHARGER_CURRENT_LIMITING_IND_T*)message)->voltage);
        }
        break;

        case POWER_BATTERY_TEMPERATURE_IND:
        {
            POWER_BATTERY_TEMPERATURE_IND_T* ind = (POWER_BATTERY_TEMPERATURE_IND_T*)message;
            PM_DEBUG(("PM: Rcv POWER_BATTERY_TEMPERATURE_IND with voltage %umV, level %u\n", ind->vthm.voltage, ind->vthm.level));
            powerManagerChargerSetup(&ind->vthm);
        }
        break;

        case POWER_CHARGER_STATE_IND:
        {
            POWER_CHARGER_STATE_IND_T* ind = (POWER_CHARGER_STATE_IND_T*)message;
            /* Send event if not using LED overide feature or state is currently not trickle charge */
            PM_DEBUG(("PM: Rcv POWER_CHARGER_STATE_IND with state %u\n", ind->state));
            powerManagerHandleChargeState(ind->state);
            /* NB. Charger state will not indicate a change from complete to fast. We assume this
            doesn't matter as we should run from VBUS when complete so battery should not run down */
            powerManagerUpdateChargeCurrent();
        }
        break ;
        
        default :
            PM_DEBUG(("PM: Rcv UNHANDLED BATTERY MSG [%x]\n", id));
        break ;
    }
}

#ifdef ENABLE_CHARGER_V2
/****************************************************************************
NAME
    powerManagerStartChargerStuckTimeout

DESCRIPTION
      Handle change in charger status

RETURNS
    void
*/
static void powerManagerStartChargerStuckTimeout(uint16 timeout)
{
    if (timeout > 0)
    {
        PM_DEBUG(("PM: Start charger stuck timeout\n"));
        MessageSendLater(&theSink.task, EventSysChargerError, NULL, D_MIN(timeout));
    }
}


/****************************************************************************
NAME
    powerManagerHandleEmergencyMode

DESCRIPTION
      Handle emergency mode indication

RETURNS
    void
*/
void powerManagerHandleEmergencyMode(void)
{
    /* The charger should have been disabled, but make sure that our state is
     * consistent.
     */
    PM_DEBUG(("PM: Handle Emergency Mode\n"));
    powerManagerStopChargerStuckTimeout();
    PowerMonitorStopChargerProgressMonitor();
    PowerCurrentLimitingStop();
    PowerChargerEnable(FALSE);
}

/****************************************************************************
NAME
    powerManagerHandleChargerStatus

DESCRIPTION
      Handle change in charger status

RETURNS
    Success of Failure
*/
void powerManagerHandleChargerStatus(charger_status chg_status)
{
    powerManagerStopChargerStuckTimeout();
    PowerMonitorStopChargerProgressMonitor();
    PowerCurrentLimitingStop();

    switch (chg_status)
    {
        case NO_POWER:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS with NO_POWER\n"));
            break;
        }
        case TRICKLE_CHARGE:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS with TRICKLE_CHARGE\n"));
            powerManagerStartChargerStuckTimeout(GPOWER.trickle_charge_timeout);
            break;
        }
        case PRE_CHARGE:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS with PRE_CHARGE\n"));
            powerManagerStartChargerStuckTimeout(GPOWER.precharge_timeout);
            PowerMonitorStartChargerProgressMonitor();
            break;
        }
        case FAST_CHARGE:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS with FAST_CHARGE\n"));
            powerManagerStartChargerStuckTimeout(GPOWER.fast_charge_timeout);
            PowerMonitorStartChargerProgressMonitor();
            PowerCurrentLimitingStart();
            break;
        }
        case HEADROOM_ERROR:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS with HEADROOM_ERROR\n"));
            PowerCurrentLimitingHandleHeadroomError();
            break;
        }
        case VBAT_OVERVOLT_ERROR:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS with VBAT_OVERVOLT_ERROR\n"));
            break;
        }
        case STANDBY:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS with STANDBY\n"));
            if ((stateManagerIsWakingUp()) &&
                (VmGetResetSource() == RESET_SOURCE_DORMANT_WAKEUP))
            {
                stateManagerEnterLimboState(limboShutDown);
            }
            break;
        }
        case DISABLED_ERROR:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS with DISABLED_ERROR\n"));
            break;
        }
        case CONFIG_FAIL_UNKNOWN:
        case CONFIG_FAIL_CHARGER_ENABLED:
        case CONFIG_FAIL_EFUSE_CRC_INVALID:
        case CONFIG_FAIL_EFUSE_TRIMS_ZERO:
        case CONFIG_FAIL_CURRENTS_ZERO:
        case CONFIG_FAIL_VALUES_OUT_OF_RANGE:
        case ENABLE_FAIL_UNKNOWN:
        case ENABLE_FAIL_EFUSE_CRC_INVALID:
        case INTERNAL_CURRENT_SOURCE_CONFIG_FAIL:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS with FAIL status %u\n", chg_status));
            MessageSend(&theSink.task, EventSysChargerError, NULL);
            PowerChargerEnable(FALSE);
            break;
        }
        default:
        {
            PM_DEBUG(("PM: Rcv MESSAGE_CHARGER_STATUS: UNKNOWN\n"));
            Panic();
        }
    }
}
#endif /* ENABLE_CHARGER_V2 */

/****************************************************************************
NAME    
    powerManagerGetLBIPM
    
DESCRIPTION
    Returns the Power level to use for Low Battery Intelligent Power 
    Management (LBIPM).
    Note will always return high level if this feature is disabled.
    
RETURNS
    void
*/
power_battery_level powerManagerGetLBIPM( void )                             
{
    PM_DEBUG(("PM: Battery Management %s\n", powerMangerIsLbipmEnabled() ? "Enabled" : "Disabled"));
    PM_DEBUG(("PM: Using %s Level ", powerManagerIsChargerConnected() ? "Chg" : "Batt"));
    PM_DEBUG(("%u\n", powerManagerIsChargerConnected() ? POWER_BATT_LEVEL3 : powerManagerGetBatteryLevel()));

    /* Get current battery level */
    if(powerMangerIsLbipmEnabled() && !powerManagerIsChargerConnected())
        return powerManagerGetBatteryLevel();
    
    /* LBIPM disabled or charger is connected so use full power level */
    return POWER_BATT_LEVEL3;
}

/****************************************************************************
NAME
    powerManagerChargerModeVoltageSelect

DESCRIPTION
    Update the charger voltage select based on charging state.

RETURNS
    sink_vsel
*/
#ifdef ENABLE_USB
static sink_vsel powerManagerChargerModeVoltageSelect(void)
{
    return (PowerChargerFastCharge() ? GPOWER.vsel_fast_charge : vsel_chg);
}
#endif

/****************************************************************************
NAME
    powerManagerSetDefaultVterm

DESCRIPTION
    Sets Vterm to the default setting.

RETURNS
    void
*/

#ifdef ENABLE_CHARGER_V2
static void powerManagerSetDefaultVterm(void)
{
    PM_DEBUG(("PM: Setting Default Termination Voltage Mapping %u\n", GPOWER.default_vfloat));
    PowerChargerSetVterm(GPOWER.default_vfloat);
}
#else
static void powerManagerSetDefaultVterm(void)
{
    PM_DEBUG(("PM: Termination Voltage Unchanged\n"));
}
#endif

/****************************************************************************
NAME
    powerManagerSelectVoltageSource

DESCRIPTION
    Selects the voltage source for the IC.

RETURNS
    void
*/

#ifdef ENABLE_CHARGER_V2
static void powerManagerSelectVoltageSource(sink_vsel vsel)
{
    PsuConfigure(PSU_ALL, PSU_SMPS_INPUT_SEL_VBAT, ((vsel == vsel_bat) ? TRUE : FALSE));
}
#else
static void powerManagerSelectVoltageSource(sink_vsel vsel)
{
    PsuConfigure(PSU_VBAT_SWITCH, PSU_SMPS_INPUT_SEL_VBAT, ((vsel == vsel_bat) ? TRUE : FALSE));
}
#endif

/****************************************************************************
NAME    
    powerManagerChargerSetup
    
DESCRIPTION
    Update the charger settings based on USB limits and provided temperature
    reading. Updates current, trim, boost charge settings and enables or 
    disables the charger. On BC7 this can also be used to select whether the
    chip draws power from VBAT or VBYP.
    
RETURNS
    bool
*/
bool powerManagerChargerSetup(voltage_reading* vthm)
{
    if (PowerBatteryIsOverVoltage())
    {
        PM_DEBUG(("PM: Charger is overvoltage, don't reconfigure the charger\n"));
        return FALSE;
    }
    else
    {
        /* Get temperature limits */
        sink_charge_setting setting;
#ifdef ENABLE_USB
        usb_charge_current_setting usb_limits;
        memset(&usb_limits,0,sizeof(usb_charge_current_setting));
#endif
        memset(&setting,0,sizeof(sink_charge_setting));

        if(vthm)
        {
            powerManagerGetChargerSetting(vthm->level,&setting);
        }
        else
        {
            setting = default_charge_setting;
        }

#ifdef ENABLE_USB
        if(usbGetChargeCurrent(&usb_limits))
        {
            /* Apply minimum settings from combined limits */
            setting.current.charge = usb_limits.charge & setting.current.charge;
            if(sinkUsbGetBoost(&usb_limits) < setting.current.boost)
                setting.current.boost = sinkUsbGetBoost(&usb_limits);
            setting.current.vsel = usb_limits.vsel | setting.current.vsel | powerManagerChargerModeVoltageSelect();
            setting.current.disable_leds = usb_limits.disable_leds | setting.current.disable_leds;
            if(usb_limits.current < setting.current.current)
                setting.current.current = usb_limits.current;
        }
#endif

        if(!(powerManagerIsChargerConnected() && powerManagerChargerPinState()))
        {
            /* Must apply these settings when charger removed */
            setting.current.boost   = power_boost_disabled;
            setting.current.charge  = FALSE;
            setting.current.current = 0;
#ifdef HAVE_VBAT_SEL
            if (!powerManagerIsVbatCritical())
                setting.current.vsel = vsel_bat;
        }
        else if(setting.current.power_off)
        {
            /* If outside operating temp cannot run from battery */
            setting.current.vsel = vsel_chg;
#endif
        }

        if (GPOWER.have_charger_error)
        {
            setting.current.charge = FALSE;
        }

        switch(setting.termination.type)
        {
            case vterm_voltage:
                /* BC7 allows us to set absolute termination voltage */
                if(!PowerChargerSetVterm(setting.termination.voltage))
                {
                    PM_DEBUG(("PM: Failed, disabling charger\n"));
                    setting.current.charge = FALSE;
                }
            break;
            default:
                powerManagerSetDefaultVterm();
            break;
        }

        /* Disable LEDs if required */
        LedManagerForceDisable(setting.current.disable_leds);

        if(GPOWER.current_external_devices < setting.current.current)
        {
            setting.current.current -= GPOWER.current_external_devices;
        }
        else
        {
            setting.current.current = 0;
            setting.current.charge = FALSE;
        }

        /* With VBAT_SEL we can wait for temp reading before enabling charger.
           Without we enable charger by default and may need to turn it off. */
        if(vthm)
        {
            /* Set charge current */
            PowerChargerSetCurrent(setting.current.current);
            PowerChargerSetBoost(setting.current.boost);

#ifdef HAVE_VBAT_SEL
        }

        powerManagerSelectVoltageSource(setting.current.vsel);

        if(vthm)
        {
#endif

            PM_DEBUG (("PM settings: current %u, boost %u, charge %u, vsel %s, power_off %u, disable_leds %u\n",
                       setting.current.current, setting.current.boost, setting.current.charge, setting.current.vsel == 1 ? "VBAT" : "VBYP",
                       setting.current.power_off, setting.current.disable_leds));

            /* Enable/disable charger */
            if (!stateManagerIsShuttingDown())
                PowerChargerEnable(setting.current.charge);

            /* Power off */
            if(setting.current.power_off)
            {
                powerManagerPowerOff();
            }
        }
        return((vthm) && (setting.current.charge));
    }
}


/****************************************************************************
NAME    
    powerManagerUpdateChargeCurrent
    
DESCRIPTION
    Update the charger settings based on latest reading of VTHM
    
RETURNS
    void
*/
void powerManagerUpdateChargeCurrent(void)
{
    voltage_reading vthm;
    /* If power library intialised setup charger */
    if(PowerBatteryGetTemperature(&vthm))
        powerManagerChargerSetup(&vthm);
    else
        powerManagerChargerSetup(NULL);
}

/****************************************************************************
NAME    
    powerManagerReadVbatVoltage
    
DESCRIPTION
    Call this function to take an immediate battery reading and retrieve the voltage.
    
RETURNS
    The current battery voltage
*/
uint16 powerManagerReadVbatVoltage(void)
{
    voltage_reading reading;
    
    if(PowerBatteryGetVoltage(&reading))
        return reading.voltage;
    
    return 0;
}

/*******************************************************************************
FUNCTION
    powerManagerBatteryLevelAsPercentage

DESCRIPTION
    Call this function to get battery level in percentage.
RETURNS
    BATTERY_LEVEL_INVALID on invalid battery configuration
*/
uint16 powerManagerBatteryLevelAsPercentage(void)
{
    
    uint16 battery_level = BATTERY_LEVEL_INVALID;
    uint16 voltage = powerManagerReadVbatVoltage();
    sink_battery_limits battery_limits ;
    powerManagerGetMinandMaxBatteryLimit(&battery_limits);
    /* calculate % battery level using: (currentV - minV)/(maxV - minV)*100 */
    if (battery_limits.max_battery_v > battery_limits.min_battery_v)
    {
        if (voltage < battery_limits.min_battery_v)
        {
            battery_level = 0;
        }
        else if (voltage > battery_limits.max_battery_v)
        {
            battery_level = 100;
        }
        else
        {
            battery_level = (uint16)(((uint32)(voltage - battery_limits.min_battery_v)  * (uint32)100) / (uint32)(battery_limits.max_battery_v - battery_limits.min_battery_v));
        }
    }
    return battery_level;
}

/****************************************************************************
NAME    
    powerManagerGetBatteryLevel
    
DESCRIPTION
      Get the Bettery Level
    
RETURNS
    power_battery_level
*/
power_battery_level powerManagerGetBatteryLevel(void)
{
      return GPOWER.battery_state;
}

/****************************************************************************
NAME    
    powerManagerSetBatteryLevel
    
DESCRIPTION
      Set the Bettery Level
    
RETURNS
    void
*/
static void powerManagerSetBatteryLevel(power_battery_level level)
{
     GPOWER.battery_state = level;
}

/****************************************************************************
NAME    
    powerManagerGetSessionData
    
DESCRIPTION
      Set power data
    
RETURNS
    void
*/
 void powerManagerGetSessionData(void)
 {
     sink_power_manager_writeable_config_def_t *config_data = NULL;
    /* Read session data */
    configManagerGetReadOnlyConfig(SINK_POWER_MANAGER_WRITEABLE_CONFIG_BLK_ID, (const void **)&config_data);
    GPOWER.lbipmEnable = config_data->lbipmenable;
    configManagerReleaseConfig(SINK_POWER_MANAGER_WRITEABLE_CONFIG_BLK_ID);
 }

/****************************************************************************
NAME    
    powerManagerSetSessionData
    
DESCRIPTION
      Set session data
    
RETURNS
    void
*/
void powerManagerSetSessionData(void)
{            
    sink_power_manager_writeable_config_def_t *config_data = NULL;
    /* Write session data */
    configManagerGetWriteableConfig(SINK_POWER_MANAGER_WRITEABLE_CONFIG_BLK_ID, (void **)&config_data, 0);
    config_data->lbipmenable = GPOWER.lbipmEnable;
    configManagerUpdateWriteableConfig(SINK_POWER_MANAGER_WRITEABLE_CONFIG_BLK_ID);
}

/****************************************************************************
NAME    
    powerManagerEnableLbipm
    
DESCRIPTION
      Enable/Disbale low battery Inteligent Power Manager 
    
RETURNS
    void
*/
void powerManagerEnableLbipm(bool enable)
{
    GPOWER.lbipmEnable = enable;
}

/****************************************************************************
NAME    
    powerMangerIsLbipmEnabled
    
DESCRIPTION
      Check low battery Inteligent Power Manager enabled
    
RETURNS
    Success of Failure
*/
bool powerMangerIsLbipmEnabled(void)
{
    return (GPOWER.lbipmEnable) ? TRUE : FALSE;
}

/****************************************************************************
NAME
    powerManagerIsReadyForDormant

DESCRIPTION
      Prepare for dormant mode, if required
      Return TRUE if ready for dormant mode

RETURNS
    bool
*/
bool powerManagerIsReadyForDormant(void)
{
    if (powerManagerIsChargerConnected())
    {
        if (!PowerChargerEnabled())
        {
            /* Not trickle, pre-charge or fast charge, but make sure charger is
             * disabled as it may be in standby or an error mode.
             */
            powerManagerDisableCharger();
            return PowerChargerDisabled();
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return TRUE;
    }
}

/****************************************************************************
NAME
    powerManagerDisableCharger

DESCRIPTION
    Disable the charger

RETURNS
    void
*/
void powerManagerDisableCharger(void)
{
    PowerChargerEnable(FALSE);
}


/****************************************************************************
NAME
    powerManagerLowGasGaugeAction

DESCRIPTION
    To ensure that speaker parts cannot flatten the battery whilst being charged,
    determine if any action needs to be taken when battery is almost at the low
    battery indication level, three options are available:

        1) do nothing, device will not power off and may discharge battery beyond
           operational levels.
        2) power off device when reaching low battery level even if charger is attached
           to prevent operation with a discharged battery causing performance issues.
        3) limit the maximum volume to reduce the current drain to a level lower than
           that of the charger device to allow the device to contine being used whilst charging.

RETURNS
    void
*/
void powerManagerLowGasGaugeAction(sinkEvents_t event)
{
    PM_DEBUG(("PM: LowGasGaugeCheck\n"));
    /* determine the required low power action from configuration */

    switch(powerManagerGetChargingDeficitAction())
    {
        /* when reaching the low battery level, if charger is connected,
           issue a power off request */
        case battery_low_output_action_power_off_at_low_batt_warning:

            /* if routing audio with the charger connected and at gas gauge 0 or below, power off */
            if(sinkAudioIsAudioRouted() && (powerManagerIsChargerConnected() == TRUE) &&
               ((event == EventSysChargerGasGauge0) || (event == EventSysGasGauge0) || (event == EventSysBatteryLow)))
            {
                PM_DEBUG(("PM: LowGasGaugeCheck->PowerOff\n"));
                /* power off device to allow charge to complete */
                MessageSend(&theSink.task, EventUsrPowerOff, 0);
            }

        break;

        /* if at low battery or gas_gauge 0, set a volume limit, clear the
           limit at gas_gauge 1 or above if charger is disconnected */
        case battery_low_output_action_limit_max_volume:
        {
            unsigned cdl_percentage;
            unsigned cdl_as_step_level;

            volume_group_config_t gain_utils_vol;
            sinkVolumeGetGroupConfig(audio_output_group_main, &gain_utils_vol);

            cdl_percentage = powerManagerGetVolumeLimitForChargingDeficit();
            cdl_as_step_level = (gain_utils_vol.no_of_steps * cdl_percentage) / 100;

            /* if at gas gauge 0 or below, SET max volume LIMIT, also set max volume limit if a remote peer
               device indicates a low battery condition */
            if((sinkAudioIsAudioRouted() || sinkAudioIsVoiceRouted()) &&
               (sinkVolumeGetChargeDeficitMaxVolumeLimit() != cdl_as_step_level) &&
               ((event == EventSysChargerGasGauge0) || (event == EventSysGasGauge0) || (event == EventSysBatteryLow)
#ifdef ENABLE_PEER
               || (GPOWER.peer_slave_low_bat)
#endif
              ))
            {
                PM_DEBUG(("PM: LowGasGaugeCheck->ApplyLimit\n"));
                /* set volume limit step level calculated from cdl_percentage */
                sinkVolumeSetChargeDeficitMaxVolumeLimit(cdl_as_step_level);
                
                /* force update of volume level to enforce limit to current connection,  
                   but do not unmute if muted. */       
                modifyAndUpdateRoutedAudioVolume(same_volume,audio_output_group_main, FALSE); 

#ifdef ENABLE_PEER
                /* if this is a peer slave device, send the volume limit applied status via avrcp
                   cmd to the peer master. the peer master device will limit the volume applied to
                   the slave device since volume is controlled via embedded info in the relay
                   data stream */
                powerManagerUpdatePeerDeviceLowBatt(sinkVolumeGetChargeDeficitMaxVolumeLimit());
#endif
                /* test if we got here due to a local low battery event (i.e. not a peer_slave_low_bat situation)*/
                if((event == EventSysChargerGasGauge0) || (event == EventSysGasGauge0) || (event == EventSysBatteryLow))
                {
                    /* set flag to indicate that the vol cap limit cannot be removed until charger has been inserted */
                    GPOWER.low_batt_limit_wait_for_charger = TRUE;
                }
            }
            /*   IF :
             We are routing audio                                                                           AND
             (Either the local charger has been disconnected OR we are processing change of limitation
                            event as generated by the slave (i.e as indicated by event == EventInvalid) )   AND
             A non zero charge deficit limit has been specified                                             AND
             The battery is above gas gauge 0 etc,
                 THEN :
             RESET THE LIMIT if we waiting for the charger has been removed AND the remote peer device if
                         present indicates no low battery condition */
            else if((sinkAudioIsAudioRouted() || sinkAudioIsVoiceRouted()) &&
                    ((powerManagerIsChargerConnected() == FALSE) || event == EventInvalid) &&
                    (sinkVolumeGetChargeDeficitMaxVolumeLimit() != 0) &&
                    (event != EventSysChargerGasGauge0) && (event != EventSysGasGauge0) && (event != EventSysBatteryLow)
                   )
            {
#ifdef ENABLE_PEER
                {
                    uint16 peerA2dpId;

                    /* check whether peer is connected/available */
                    if(a2dpGetPeerIndex(&peerA2dpId))
                    {
                        /* have already confirmed that audio is streaming,
                           determine if this device is a slave peer device? 
                           */
                        if(peerIsThisDeviceTwsMaster() == FALSE)
                        {
                            /* check if charger has been inserted 
                               if so max limit can be removed, notify peer master device which
                               will remove the volume limit applied to the relay data stream
                               which controls the slave device volume */
                            if(GPOWER.low_batt_limit_wait_for_charger == FALSE)
                                powerManagerUpdatePeerDeviceLowBatt(0);
                            
                        }                        
                        else
                        {
                            /* This device is therefore a master device, check if slave is in vol limit mode */
                            if(GPOWER.peer_slave_low_bat)
                                return; /* peer slave is still in volume limit condition, don't remove limit */
                        }
                    }
                }
#endif
                /* to prevent the limit from triggering and resetting when near the low batt
                   limit voltage, ensure the charger has been inserted and then removed before removing
                   the limit */
                if(GPOWER.low_batt_limit_wait_for_charger == FALSE)
                {
                    /* limit both locally and from remote slave peer device indication now removed */
                    PM_DEBUG(("PM: LowGasGaugeCheck->RemoveLimit\n"));
                    sinkVolumeSetChargeDeficitMaxVolumeLimit(0);
                    /* force update of volume level to remove limit, but do not unmute if muted. */
                    modifyAndUpdateRoutedAudioVolume(same_volume, audio_output_group_main, FALSE);
                }
            }
        }
        break;

        /* do nothing option */
        default:
        break;
    }
}

/****************************************************************************
NAME
    powerManagerGetChargingDeficitAction

DESCRIPTION
    Get the charging deficit limitations action as uint

RETURNS
    battery_low_output_action enum value 
*/

battery_low_output_action powerManagerGetChargingDeficitAction(void)
{
    sink_gas_gauge_actions_config_def_t *data = NULL;
    uint16 size = 0;
     /* default to "Take no action" value */
    battery_low_output_action result = battery_low_output_action_none;

    size = configManagerGetReadOnlyConfig(SINK_GAS_GAUGE_ACTIONS_CONFIG_BLK_ID, (const void **)&data);

    if (size)
    {
        result = (battery_low_output_action)data->action_when_battery_less_than_gas_gauge0;
        configManagerReleaseConfig(SINK_GAS_GAUGE_ACTIONS_CONFIG_BLK_ID);
    }
    
    return result;
}

/****************************************************************************
NAME
    getVolumeLimitForChargingDeficit

DESCRIPTION
    Get the value to be used as the volume limit for charging deficit limitation
    when "Limit maximum volume level until charger is disconnected and level is 
    above Gas Gauge 0" is action.

RETURNS
    uint 
*/

unsigned powerManagerGetVolumeLimitForChargingDeficit(void)
{
    sink_gas_gauge_actions_config_def_t *data = NULL;
    uint16 size = 0;
    unsigned result = 0; /* default to "Take no action" value */

    size = configManagerGetReadOnlyConfig(SINK_GAS_GAUGE_ACTIONS_CONFIG_BLK_ID, (const void **)&data);

    if (size)
    {
        result = data->max_volume_limit;
        configManagerReleaseConfig(SINK_GAS_GAUGE_ACTIONS_CONFIG_BLK_ID);
    }

    return result;
}

/****************************************************************************
NAME
    powerManagerUpdatePeerDeviceLowBatt

DESCRIPTION
    low power volume limiting information is to be sent from a slave PEER device to
    the master peer device when audio is streaming, this allows a peer slave device with a
    low battery condition to limit the volume on the peer master device

RETURNS
    void
*/
#ifdef ENABLE_PEER
void powerManagerUpdatePeerDeviceLowBatt(unsigned limit)
{


    /* if this is a peer slave device, send volume limit applied status to peer master when
       audio is streaming */
    if(sinkAudioIsAudioRouted())
    {
        uint16 peerA2dpId;

        /* check whether peer is connected/available */
        if(a2dpGetPeerIndex(&peerA2dpId))
        {
            /* only PEER slave device notifies the master PEER device of volume limit being applied
               since volume limit applied on the master due to low battery condition is automatically applied
               to the slave */
            if(peerIsThisDeviceTwsMaster() == FALSE)
            {
                /* default to max limit is not applied*/
                bool applyLimitation = FALSE;
                
                /* check if volume limit is to be applied */
                if(limit != 0)
                {
                    /* max limit is applied, notify peer master device */
                    applyLimitation = TRUE;
                }
                avrcpUpdatePeerLowBatteryStatus(applyLimitation);
           }
        }
    }
}
#endif
/****************************************************************************
NAME
    powerManagerGetWaitingForChargerFlag

DESCRIPTION
   Gets the flag which indicates if we are waiting for the charger to be disconnected.
   This indication is used to signal that we are ready to restore the volume after
   limiting as part of the charge deficit limitation/mitigation.
RETURNS
    bool
*/
bool powerManagerGetWaitingForChargerFlag(void)
{
    return GPOWER.low_batt_limit_wait_for_charger;
}

/****************************************************************************
NAME
    powerManagerSetWaitingForChargerFlag

DESCRIPTION
   Sets the flag which indicates if we are waiting for the charger to be disconnected.

RETURNS
    void
*/
void powerManagerSetWaitingForChargerFlag(bool newFlagStatus)
{
    GPOWER.low_batt_limit_wait_for_charger = newFlagStatus;
}


/****************************************************************************
NAME
    powerManagerGetPeerSlaveLowBat

DESCRIPTION
   Gets the flag which indicates if the slave end of a TWS pairing has 
   been placed into the low battery limitation mode of operation or not.
RETURNS
    bool
*/
bool powerManagerGetPeerSlaveLowBat(void)
{
    return GPOWER.peer_slave_low_bat;
}

/****************************************************************************
NAME
    powerManagerSetPeerSlaveLowBat

DESCRIPTION
   Sets the flag which indicates that the slave end of a TWS pairing has 
   been placed into the low battery limitation mode of operation.

RETURNS
    void
*/
void powerManagerSetPeerSlaveLowBat(bool newFlagStatus)
{
    GPOWER.peer_slave_low_bat = newFlagStatus;
}

/****************************************************************************
NAME
    powerManagerServiceQuickChargeStatInterrupt

DESCRIPTION
   Service the Quick Charge STAT interrupt.

RETURNS
    void
*/
void powerManagerServiceQuickChargeStatInterrupt(void)
{
    PowerQuickChargeServiceInterrupt();
}

/****************************************************************************
NAME
    powerManagerChargerError

DESCRIPTION
   Handler for Charger Error

RETURNS
    void
*/
void powerManagerChargerError(bool haveChargerError)
{
    PM_DEBUG(("PM: powerManagerChargerError(%u)\n", haveChargerError));
    GPOWER.have_charger_error = haveChargerError;
}

/****************************************************************************
NAME
    powerManagerHasChargerError

DESCRIPTION
   Accessor for whether there has been a Charger Error

RETURNS
    bool: TRUE if there has been a Charger Error, else FALSE
*/
bool powerManagerHasChargerError(void)
{
    return GPOWER.have_charger_error;
}

#endif /* battery operation disabled */

/****************************************************************************
NAME    
    powerManagerCheckPanic
    
DESCRIPTION
    Check if the last reset was caused by Panic/ESD
    
RETURNS
    void
*/
#ifdef INSTALL_PANIC_CHECK
void powerManagerCheckPanic(void)
{
    vm_reset_source rst_src = VmGetResetSource();
    vm_power_enabler en_src = (vm_power_enabler)VmGetPowerSource();

    sinkDataSetPanicReconnect(FALSE);
    /* Is panic check enabled */
    if(sinkDataIsReconnectOnPanic())
        if(rst_src == RESET_SOURCE_FIRMWARE || rst_src == UNEXPECTED_RESET || en_src == POWER_ENABLER_RESET_PROTECTION)
            sinkDataSetPanicReconnect(TRUE);

    PM_DEBUG(("PM: Power %u, Reset %u,  %sReconnect\n", en_src, rst_src, sinkDataGetPanicReconnect() ? "":"Don't "));
}
#endif


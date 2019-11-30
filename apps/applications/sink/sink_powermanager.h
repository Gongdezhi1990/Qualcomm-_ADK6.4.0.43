/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    Configuration manager for the device - resoponsible for extracting user information out of the 
    PSKEYs and initialising the configurable nature of the device components
    
*/
#ifndef SINK_POWER_MANAGER_H
#define SINK_POWER_MANAGER_H

#include <csrtypes.h>
#include <power.h>
#include "sink_debug.h"
#include "sink_events.h"
#include "sink_configmanager.h"

#define PSKEY_CHARGER_CALC_VTERM 0x25b6

/* Battery level to return if the configuration is invalid and cannot read the internal battery */
#define BATTERY_LEVEL_INVALID   ((uint16) -1)

typedef enum
{
    battery_level_automatic         = (1<<0),
    battery_level_user              = (1<<1),
    battery_level_initial_reading   = (1<<2)
} battery_level_source;

typedef enum
{
    vsel_chg,
    vsel_bat
} sink_vsel;

typedef struct
{
    unsigned            charge:1;
    power_boost_enable  boost:2;
    sink_vsel           vsel:1;
    unsigned            power_off:1;
    unsigned            disable_leds:1;
    unsigned            unused:10;
    unsigned            current:16;
} sink_charge_current;

typedef enum
{
    vterm_default,  /* Use the default termination voltage */
    vterm_voltage,  /* Use voltage as specified in mV (BC7) */
    vterm_trim      /* Reduce trim by specified amount (pre-BC7) */
} sink_vterm;

typedef struct
{
    uint16 max_battery_v;
    uint16 min_battery_v;
}sink_battery_limits;

/* action to take when battery is at battery low or between battery low and gas gauge 0 levels */
typedef enum 
{
    battery_low_output_action_none = 0,                            /* no action */
    battery_low_output_action_power_off_at_low_batt_warning = 1,   /* power off device */
    battery_low_output_action_limit_max_volume = 2                 /* set a maximum volume level to allow battery to charge */      
}battery_low_output_action;



typedef struct
{
    sink_vterm   type:4;   
    unsigned     trim:4;
    unsigned     unused:8;
    uint16       voltage;
} sink_charge_termination;

typedef struct
{
    sink_charge_current      current;
    sink_charge_termination  termination;
} sink_charge_setting;

typedef struct
{
    unsigned                unused:13;
    battery_level_source    sources:3;
    uint16                  event;        /* Event to generate */
} sink_battery_setting;

typedef struct
{
    sink_battery_setting bat_events[POWER_MAX_VBAT_REGIONS];
    sink_charge_setting  chg_settings[POWER_MAX_VTHM_REGIONS];
} sink_power_settings;


/****************************************************************************
NAME    
    powerManagerInit
    
DESCRIPTION
      Configure power management
    
RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
bool powerManagerInit(void);
#else
#define powerManagerInit() ((void)(0))
#endif

/****************************************************************************
NAME    
    powerManagerChargerConnected
    
DESCRIPTION
      This function is called when the charger is plugged into the device
    
RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerChargerConnected(void);
#else
#define powerManagerChargerConnected() ((void)(0))
#endif


/****************************************************************************
NAME    
    powerManagerChargerDisconnected
    
DESCRIPTION
      This function is called when the charger is unplugged from the device
    
RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerChargerDisconnected(void);
#else
#define powerManagerChargerDisconnected() ((void)(0))
#endif


/*************************************************************************
NAME    
    handlePowerMessage
    
DESCRIPTION
    handles the Battery/Charger Monitoring Messages

RETURNS
    
*/
#ifdef ENABLE_BATTERY_OPERATION
void handlePowerMessage( Task task, MessageId id, Message message );
#else
#define handlePowerMessage(task, id, message) ((void)(0))
#endif

/****************************************************************************
NAME
    powerManagerIsVbatLow

DESCRIPTION
      Call this function to check the low battery warning state

RETURNS
    TRUE or FALSE
*/
#ifdef ENABLE_BATTERY_OPERATION
#define powerManagerIsVbatLow() (powerManagerGetBatteryLevel() == POWER_BATT_LOW)
#else
#define powerManagerIsVbatLow() FALSE
#endif

/****************************************************************************
NAME
    powerManagerIsVbatCritical

DESCRIPTION
      Call this function to check the critical battery warning state

RETURNS
    TRUE or FALSE
*/
#ifdef ENABLE_BATTERY_OPERATION
#define powerManagerIsVbatCritical() (powerManagerGetBatteryLevel() == POWER_BATT_CRITICAL)
#else
#define powerManagerIsVbatCritical() FALSE
#endif

/****************************************************************************
NAME    
    powerManagerIsChargerConnected
    
DESCRIPTION
    This function is called by applications to check whether the charger has 
    been plugged into the device
    
RETURNS
    bool
*/
#ifdef ENABLE_BATTERY_OPERATION
bool powerManagerIsChargerConnected(void);
#else
#define powerManagerIsChargerConnected()  (FALSE)
#endif


/****************************************************************************
NAME    
    powerManagerIsChargerFullCurrent
    
DESCRIPTION
    This function is called by applications to check whether the charger is
    in (potentially) drawing full configured current.
    
RETURNS
    bool
*/
#ifdef ENABLE_BATTERY_OPERATION
bool powerManagerIsChargerFullCurrent(void);
#else
#define powerManagerIsChargerFullCurrent()  (FALSE)
#endif


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
#ifdef ENABLE_BATTERY_OPERATION
bool powerManagerIsVthmCritical(void);
#else
#define powerManagerIsVthmCritical()  (FALSE)
#endif

/****************************************************************************
NAME    
    powerManagerReadVbat
    
DESCRIPTION
      Call this function to take an immediate battery reading and sent to AG.
    
RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerReadVbat(battery_level_source source);
#else
#define powerManagerReadVbat(x) ((void)(0))
#endif

/****************************************************************************
NAME    
    powerManagerGetLBIPM
    
DESCRIPTION
      Returns the Power level to use for Low Battery Intelligent Power Management (LBIPM)
    Note will always return high level if this feature is disabled.
    
RETURNS
    void
*/

#ifdef ENABLE_BATTERY_OPERATION
power_battery_level powerManagerGetLBIPM( void );
#else
#define powerManagerGetLBIPM() (POWER_BATT_LEVEL3)
#endif

/****************************************************************************
NAME    
    powerManagerChargerSetup
    
DESCRIPTION
    Update the charger settings based on USB limits and latest temperature
    reading. Updates current, trim, boost charge settings and enables or 
    disables the charger. On BC7 this can also be used to select whether the
    chip draws power from VBAT or VBYP. 
    
RETURNS
    bool
*/
#ifdef ENABLE_BATTERY_OPERATION
bool powerManagerChargerSetup(voltage_reading* vthm);
#else
#define powerManagerChargerSetup(x) (FALSE)
#endif

/****************************************************************************
NAME    
    powerManagerChargerSetup
    
DESCRIPTION
    Update the charger settings based on latest reading of VTHM
    
RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerUpdateChargeCurrent(void);
#else
#define powerManagerUpdateChargeCurrent() ((void)(0))
#endif

/****************************************************************************
NAME    
    powerManagerReadVbatVoltage
    
DESCRIPTION
    Call this function to take an immediate battery reading and retrieve the voltage.
    
RETURNS
    The current battery voltage
*/
#ifdef ENABLE_BATTERY_OPERATION
uint16 powerManagerReadVbatVoltage(void);
#else
#define powerManagerReadVbatVoltage() (0)
#endif

/*******************************************************************************
NAME
    powerManagerBatteryLevelAsPercentage

DESCRIPTION
    Call this function to get battery level in percentage.

RETURNS
    The current battery level in percentage.
    BATTERY_LEVEL_INVALID on invalid battery configuration.
*/
#ifdef ENABLE_BATTERY_OPERATION
uint16 powerManagerBatteryLevelAsPercentage(void);
#else
#define powerManagerBatteryLevelAsPercentage() (BATTERY_LEVEL_INVALID)
#endif

/****************************************************************************
NAME    
    powerManagerGetBatteryLevel
    
DESCRIPTION
      Get the Bettery Level
    
RETURNS
    power_battery_level
*/
#ifdef ENABLE_BATTERY_OPERATION
power_battery_level powerManagerGetBatteryLevel(void);
#else
#define powerManagerGetBatteryLevel() (0)
#endif
/****************************************************************************
NAME    
    powerManagerSetSessionData
    
DESCRIPTION
      Set session data
    
RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerSetSessionData(void);
#else
#define powerManagerSetSessionData() ((void)(0))
#endif

/****************************************************************************
NAME    
    powerManagerGetSessionData
    
DESCRIPTION
      Set power data
    
RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerGetSessionData(void);
#else
#define  powerManagerGetSessionData() ((void)(0))
#endif

/****************************************************************************
NAME    
    powerManagerEnableLbipm
    
DESCRIPTION
      Enable/Disbale low battery Inteligent Power Manager 
    
RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerEnableLbipm(bool enable);
#else
#define powerManagerEnableLbipm(enable)  ((void)(0))
#endif

/****************************************************************************
NAME    
    powerMangerIsLbipmEnabled
    
DESCRIPTION
      Check low battery Inteligent Power Manager enabled
    
RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
bool powerMangerIsLbipmEnabled(void);
#else
#define powerMangerIsLbipmEnabled() (0)
#endif

/****************************************************************************
NAME
    powerManagerIsReadyForDormant

DESCRIPTION
      Return TRUE if charger status is suitable for dormant mode

RETURNS
    bool
*/
#ifdef ENABLE_BATTERY_OPERATION
bool powerManagerIsReadyForDormant(void);
#else
#define powerManagerIsReadyForDormant() (0)
#endif

/****************************************************************************
NAME
    powerManagerDisableCharger

DESCRIPTION
    Disable the charger

RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerDisableCharger(void);
#else
#define powerManagerDisableCharger() ((void)(0))
#endif

/****************************************************************************
NAME
    powerManagerHandleEmergencyMode

DESCRIPTION
      Handle emergency mode indication

RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerHandleEmergencyMode(void);
#else
#define powerManagerHandleEmergencyMode() ((void)(0))
#endif

/****************************************************************************
NAME
    powerManagerHandleChargerStatus

DESCRIPTION
      Handle change in battery charger status

RETURNS
    void
*/
#ifdef ENABLE_CHARGER_V2
void powerManagerHandleChargerStatus(charger_status chg_status);
#else
#define powerManagerHandleChargerStatus(chg_status) ((void)(0))
#endif


/****************************************************************************
NAME    
    powerManagerCheckPanic
    
DESCRIPTION
    Check if the last reset was caused by Panic/ESD
    
RETURNS
    void
*/
#ifdef INSTALL_PANIC_CHECK
void powerManagerCheckPanic(void);
#else
#define powerManagerCheckPanic() do { \
			sinkDataSetPanicReconnect(sinkDataIsReconnectOnPanic());\
            } while(0)
#endif



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
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerLowGasGaugeAction(sinkEvents_t event);
#else
#define powerManagerLowGasGaugeAction(x) ((void)(0))
#endif




/****************************************************************************
NAME
    powerManagerLowGasGaugeAction

DESCRIPTION
    Gets the current action setting for Charging Deficit Limitation behaviour

RETURNS
    battery_low_output_action
*/
#ifdef ENABLE_BATTERY_OPERATION
battery_low_output_action powerManagerGetChargingDeficitAction(void);
#else
#define powerManagerGetChargingDeficitAction() (battery_low_output_action_none)
#endif




/****************************************************************************
NAME
    powerManagerGetVolumeLimitForChargingDeficit

DESCRIPTION
   Gets the volume limit associated with the battery_low_output_action_limit_max_volume action.
   This read-only value is modified via configuration tool only.
RETURNS
    unsigned
*/
#ifdef ENABLE_BATTERY_OPERATION
unsigned powerManagerGetVolumeLimitForChargingDeficit(void);
#else
#define powerManagerGetVolumeLimitForChargingDeficit() (0)
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
#ifdef ENABLE_BATTERY_OPERATION
bool powerManagerGetWaitingForChargerFlag(void);
#else
#define powerManagerGetWaitingForChargerFlag() FALSE
#endif


/****************************************************************************
NAME
    powerManagerSetWaitingForChargerFlag

DESCRIPTION
   Sets the flag which indicates if we are waiting for the charger to be disconnected.

RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerSetWaitingForChargerFlag(bool);
#else
#define powerManagerSetWaitingForChargerFlag(x) ((void) 0)
#endif



/****************************************************************************
NAME
    powerManagerGetPeerSlaveLowBat

DESCRIPTION
   Gets the flag which indicates if the slave end of a TWS pairing has 
   been placed into the low battery limitation mode of operation or not.
RETURNS
    bool
*/
#ifdef ENABLE_BATTERY_OPERATION
bool powerManagerGetPeerSlaveLowBat(void);
#else
#define powerManagerGetPeerSlaveLowBat() (FALSE)
#endif


/****************************************************************************
NAME
    powerManagerSetPeerSlaveLowBat

DESCRIPTION
   Sets the flag which indicates that the slave end of a TWS pairing has 
   been placed into the low battery limitation mode of operation.

RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerSetPeerSlaveLowBat(bool newFlagStatus);
#else
#define powerManagerSetPeerSlaveLowBat(x) (UNUSED(x))
#endif

/****************************************************************************
NAME
    powerManagerServiceQuickChargeStatInterrupt

DESCRIPTION
   Service the Quick Charge STAT interrupt.

RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerServiceQuickChargeStatInterrupt(void);
#else
#define powerManagerServiceQuickChargeStatInterrupt() ((void) (0))
#endif

/****************************************************************************
NAME
    powerManagerChargerError

DESCRIPTION
   Handler for Charger Error

RETURNS
    void
*/
#ifdef ENABLE_BATTERY_OPERATION
void powerManagerChargerError(bool haveChargerError);
#else
#define powerManagerChargerError(x) ((void) (0))
#endif

/****************************************************************************
NAME
    powerManagerHasChargerError

DESCRIPTION
   Accessor for whether there has been a Charger Error

RETURNS
    bool: TRUE if there has been a Charger Error, else FALSE
*/
#ifdef ENABLE_BATTERY_OPERATION
bool powerManagerHasChargerError(void);
#else
#define powerManagerHasChargerError() (FALSE)
#endif

#endif /* SINK_POWER_MANAGER_H */

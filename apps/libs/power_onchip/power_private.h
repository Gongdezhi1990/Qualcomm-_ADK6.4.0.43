/*************************************************************************
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
 

    FILE : 
                power_private.h

    CONTAINS:
                Internal information for the Battery Power Monitoring and 
                and charging management 

**************************************************************************/


#ifndef POWER_PRIVATE_H_
#define POWER_PRIVATE_H_

#include "power_onchip.h"

#include <message.h> 
#include <panic.h>
#include <print.h>

#ifndef MESSAGE_MAKE
/* 
   Normally picked up from message.h, but might not be present pre
   4.1.1h, so fall back to the old behaviour.
*/
#define MESSAGE_MAKE(N,T) T *N = PanicUnlessNew(T)
#endif

#define MAKE_POWER_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T);

#define POWER_DEFAULT_VOLTAGE     0xFFFF

#define INITIAL_FAST_DYNAMIC_CURRENT_IN_MA (500)

#ifdef DEBUG_THERMAL
#define THERMAL_PRINT(x)    PRINT(x)
#else
#define THERMAL_PRINT(x)
#endif

#ifdef DEBUG_CHG_MONITOR
#define CHG_MONITOR_PRINT(x)    PRINT(x)
#else
#define CHG_MONITOR_PRINT(x)
#endif

#ifdef DEBUG_ADC_MESSAGES
#define ADC_PRINT(x)    PRINT(x)
#else
#define ADC_PRINT(x)
#endif

/* Internal messages */
enum
{
    POWER_INTERNAL_VREF_REQ,
    POWER_INTERNAL_VBAT_REQ,
    POWER_INTERNAL_VCHG_REQ,
    POWER_INTERNAL_VTHM_REQ,
    POWER_INTERNAL_CHG_MONITOR_REQ,
    POWER_INTERNAL_VBAT_NOTIFY_REQ,
    POWER_INTERNAL_VTHM_SETUP,
    POWER_INTERNAL_THERMAL_REQ,
    POWER_INTERNAL_CHG_PRG_MON_REQ,
    POWER_INTERNAL_TOP              /* Must come last */
};

/* Init mask */
typedef enum
{
    power_init_start    = 0,
    power_init_vref     = 1 << 0,
    power_init_vbat     = 1 << 1,
    power_init_vchg     = 1 << 2,
    power_init_vthm     = 1 << 3,
    power_init_chg_prg_mon = 1 << 4,
    power_init_cfm_sent = 1 << 5
} power_init_mask;

typedef enum {
    chg_cycle_incomplete =0,
    chg_cycle_complete =1,
    chg_cycle_notified  =2
} power_chg_cycle;

typedef enum {
    /*!
        @brief Thermal state
    */
    thermal_normal      =0,
    thermal_rampdown    =1,
    thermal_shutdown    =2
} power_thermal_state;

typedef struct {
    uint16              shutdown_period;        /* Thermal shutdown period when not charging. */
    uint16              shutdown_long_period;   /* Thermal shutdown period when charging. */
    int16               shutdown_temperature;   /* Die temperature at which charger is disabled. */
    int16               rampdown_temperature;   /* Die temperature at which charger is put to 1/4 current. */
    int16               reenable_temperature;   /* Die temperature at which charger is re-enabled. */
} power_thermal_config;

typedef struct {
    power_thermal_state     state;
    power_thermal_config    config;
} power_thermal;

/* Definition of the power type, a global structure used in library */
typedef struct
{
    TaskData            task;                   /* Power library task */
    Task                clientTask;             /* Client */
    power_config        config;                 /* Configuration */
    power_pmu_temp_mon_config   pmu_mon_config; /* Configuration for PMU based charger control */
    power_init_mask     init_mask:6;            /* Library initialisation mask */
    power_charger_state chg_state:3;            /* Charger state */
    power_boost_enable  chg_boost:2;            /* Boost internal/external/disable */
    power_chg_cycle     chg_cycle_complete:2;   /* Boost cycle complete */
    unsigned            pmu_mon_enabled:1;      /* Enable PMU temperature monitoring */
    unsigned            unused:1;               /* Unused */
    unsigned            charger_enabled:1;      /* Has charger been enabled? */
    uint16              vref;                   /* Most recent VREF measurement */
    uint16              vbat;                   /* Most recent VBAT measurement */
    int16               vbat_trend;             /* Current trend of VBAT measurements */
    uint16              vchg;                   /* Most recent VCHG measurement */
    uint16              vthm;                   /* Most recent VTHM measurement */
    uint8               ps_chg_trim;            /* Charger trim from PS */
    uint16              chg_prg_mon;
    uint16              charger_i_target;       /* Requested charger current */
    uint16              charger_i_setting;      /* Current value that has been set */
    uint16              charger_i_safe_fast;    /* Maximum safe external fast current */
    uint16              charger_i_actual_fast;  /* Limited external fast current value that has been set */
    power_thermal       thermal;                /* Die temperature monitoring information. */
    uint16              vterm;                  /* Most recent termination voltage setting. */
    uint16              cumulative_non_current_limit_period_chg;        /* Accumulate short periods used for current determination into long reporting period. */
} power_type;

extern power_type*      power;

#endif /* POWER_PRIVATE_H_ */



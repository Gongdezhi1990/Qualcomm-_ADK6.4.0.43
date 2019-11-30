/****************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_monitor.c

DESCRIPTION
    This file contains the ADC monitoring functionality

NOTES

*/


/****************************************************************************
    Header files
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "power_onchip.h"
#include "power_private.h"
#include "power_init.h"
#include "power_monitor.h"
#include "power_charger.h"


/****************************************************************************
NAME
    PowerInitComplete
    
DESCRIPTION
    Power library initialisation completed
*/
void PowerInitComplete(Task clientTask, bool success)
{
    MAKE_POWER_MESSAGE(POWER_INIT_CFM);
    message->success = success;
    
    if(success)
    {
        /* Set initial readings */
        PowerOnChipChargerGetVoltage(&message->vchg);
        PowerOnChipBatteryGetVoltage(&message->vbat);
        PowerOnChipBatteryGetTemperature(&message->vthm);
        powerOnChipChargerGetState(&message->state);
        /* Record CFM sent so we don't send > 1 success */
        POWER_INIT_SET(power_init_cfm_sent);
        /* Charger connected during init, restart monitoring */
        if(power->chg_state != power_charger_disconnected)
            powerMonitorRestart();
    }
    
    PRINT(("POWER: Snd POWER_INIT_CFM(success = %u, ..., state = %u) to app\n", success, message->state));
    MessageSend(clientTask, POWER_INIT_CFM, message);
}


/****************************************************************************
NAME
    PowerInit
    
DESCRIPTION
    This function will initialise the battery and its charging sub-system.
    The sub-systems manages the reading and calulation of the battery voltage
    and temperature, and the monitoring the charger status. 
*/
void PowerOnChipInit(Task clientTask, const power_config *config, const power_pmu_temp_mon_config *pmu_mon_config)
{
    if(config && !power)
    {
        /* Set up power task */
        power = PanicUnlessNew(power_type);
        
        /* ensure memory is initialised */
        memset(power,0,sizeof(power_type));
        
        power->clientTask   = clientTask;
        power->config       = *config;
        power->vref         = POWER_DEFAULT_VOLTAGE;
        power->vbat         = POWER_DEFAULT_VOLTAGE;
        power->vthm         = POWER_DEFAULT_VOLTAGE;
        power->vchg         = POWER_DEFAULT_VOLTAGE;
        power->chg_cycle_complete = chg_cycle_incomplete;
        power->chg_state    = power_charger_disconnected;
        if (pmu_mon_config != NULL)
        {
            power->pmu_mon_config = *pmu_mon_config;
            
            power->chg_prg_mon = MAX_CHG_PRG_MON;
            
            power->charger_i_target = 0;
            power->charger_i_setting = 0;
            power->charger_i_safe_fast = 0;
            power->charger_i_actual_fast = 0;

            /* if the monitoring periods have been set then enable the pmu charger control */
            if(power->pmu_mon_config.monitor_period_active || power->pmu_mon_config.monitor_period_idle || power->pmu_mon_config.monitor_period_nopwr)
                power->pmu_mon_enabled = 1;
            /* otherwise disable it */
            else
                power->pmu_mon_enabled = 0;
        }
        else
        {
            power->pmu_mon_enabled = 0;
        }
        
        /*    Can't have both...   */
        PanicFalse(!(power->pmu_mon_enabled && power->config.vchg.ext_fast));

        /*    Ensure that VCHG monitoring is called immediately.   */
        power->cumulative_non_current_limit_period_chg = power->config.vchg.non_current_limit_period_chg;
        
        POWER_INIT_SET(power_init_start);
        powerChargerGetTrim();
        
        /* Start monitoring */
        powerUtilInit();
        powerMonitorInit();
        PowerOnChipChargerMonitor();
    }
    else
    {
        PowerInitComplete(clientTask, FALSE);
    }
}


/****************************************************************************
NAME
    PowerOnChipClose

DESCRIPTION
    This function will close the battery and battery charging sub-system

RETURNS
    void
*/
void PowerOnChipClose(void)
{
    free(power);
    power = NULL;
}

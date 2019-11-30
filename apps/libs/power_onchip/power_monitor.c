/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    power_monitor.c

DESCRIPTION
    This file contains the ADC monitoring functionality

NOTES

*/


/****************************************************************************
    Header files
*/
#include "power_onchip.h"
#include "power_private.h"
#include "power_init.h"
#include "power_monitor.h"
#include "power_battery.h"
#include "power_charger.h"
#include "power_thermal.h"
#include "power_current_limiting.h"
#include "power_utils.h"

#include <pio_common.h>
#include <adc.h>
#include <pio.h>
#include <stdlib.h>
#include <vm.h>

#define PIO_MASK(pio) (((uint32)1) << pio)

power_type* power;

/****************************************************************************
NAME
    powerMonitorSetThermistorPio
    
DESCRIPTION
    Drive the thermistor with a PIO
*/
static void powerMonitorSetThermistorPio(bool drive)
{
    if (power->config.vthm.drive_pio)
    {
        PRINT(("POWER: Trp PioCommonSetPio(PIO = %u, pio_drive = %u, drive = %u) for Thermistor\n", power->config.vthm.pio, pio_drive, drive));
        PioCommonSetPio(power->config.vthm.pio, pio_drive, drive);
    }
}


/****************************************************************************
NAME
    powerMonitorConfigureThermistorPio
    
DESCRIPTION
    Configure the thermistor PIO
*/
static bool powerMonitorConfigureThermistorPio(void)
{
    if (power->config.vthm.drive_pio)
    {
        PRINT(("POWER: Trp PioCommonSetPio(PIO = %u, pio_drive = %u, drive = %u) for Thermistor\n", power->config.vthm.pio, pio_drive, FALSE));
        return PioCommonSetPio(power->config.vthm.pio, pio_drive, FALSE);
    }
    return TRUE;
}


/****************************************************************************
NAME
    powerMonitorReadAdc
    
DESCRIPTION
    Queue an ADC reading
*/
void powerMonitorReadAdc(Task task, MessageId id, uint32 period)
{
    switch (id)
    {
    case POWER_INTERNAL_VREF_REQ:
        ADC_PRINT(("POWER: Que POWER_INTERNAL_VREF_REQ in %ums\n", (unsigned int)period));
        break;

    case POWER_INTERNAL_VBAT_REQ:
        ADC_PRINT(("POWER: Que POWER_INTERNAL_VBAT_REQ in %ums\n", (unsigned int)period));
        break;

    case POWER_INTERNAL_VCHG_REQ:
        ADC_PRINT(("POWER: Que POWER_INTERNAL_VCHG_REQ in %ums\n", (unsigned int)period));
        break;

    case POWER_INTERNAL_VTHM_REQ:
        ADC_PRINT(("POWER: Que POWER_INTERNAL_VTHM_REQ in %ums\n", (unsigned int)period));
        break;

    case POWER_INTERNAL_VTHM_SETUP:
        ADC_PRINT(("POWER: Que POWER_INTERNAL_VTHM_SETUP in %ums\n", (unsigned int)period));
        break;

    case POWER_INTERNAL_CHG_PRG_MON_REQ:
        ADC_PRINT(("POWER: Que POWER_INTERNAL_CHG_PRG_MON_REQ in %ums\n", (unsigned int)period));
        break;

    default:
        ADC_PRINT(("POWER: Unexpected ADC request\n"));
        break;
    }

    MessageCancelAll(task, id);
    MessageSendLater(task, id, NULL, period);
}


/****************************************************************************
NAME
    powerMonitorReadAdcNow
    
DESCRIPTION
    Attempt to take an ADC reading, if this fails queue a retry
*/
bool powerMonitorReadAdcNow(Task task, MessageId id, power_adc* adc, power_init_mask mask)
{
    uint32 period = powerUtilGetAdcReadPeriod(adc);
    
    /* Make sure init doesn't stall */
    if(!period)
    {
        POWER_INIT_SET(mask);
        return TRUE;
    }
    
    /* Don't allow any additional readings until VREF has been measured */
    if(mask == power_init_vref || POWER_INIT_GET(power_init_vref))
    {
        /* Request ADC Reading */
        if(powerUtilAdcRequest(task, adc->source, (mask == power_init_vthm), power->config.vthm.drive_ics, power->config.vthm.delay))
        {
            /* Successful request, queue next monitor */
            powerMonitorReadAdc(task, id, period);
            return TRUE;
        }
    }
    
    /* Failed */
    return FALSE;
}


/****************************************************************************
NAME
    powerMonitorHandler
    
DESCRIPTION
    Power library message handler.
*/
static void powerMonitorHandler(Task task, MessageId id, Message message)
{
    switch(id)
    {
        case POWER_INTERNAL_VREF_REQ:
            ADC_PRINT(("POWER: Rcv POWER_INTERNAL_VREF_REQ\n"));
            /* Queue retry if ADC read fails */
            if(!powerMonitorReadAdcNow(task, id, &power->config.vref.adc, power_init_vref))
                powerMonitorReadAdc(task, id, 0);
        break;
        
        case POWER_INTERNAL_VBAT_REQ:
            ADC_PRINT(("POWER: Rcv POWER_INTERNAL_VBAT_REQ\n"));
            /* Queue retry if ADC read fails */
            if(!powerMonitorReadAdcNow(task, id, &power->config.vbat.adc, power_init_vbat))
                powerMonitorReadAdc(task, id, 0);
        break;
        
        case POWER_INTERNAL_VCHG_REQ:
            ADC_PRINT(("POWER: Rcv POWER_INTERNAL_VCHG_REQ\n"));
            if(!powerMonitorReadAdcNow(task, id, &power->config.vchg.adc, power_init_vchg))
                powerMonitorReadAdc(task, id, 0);
        break;
        
        case POWER_INTERNAL_VTHM_SETUP:
            ADC_PRINT(("POWER: Rcv POWER_INTERNAL_VTHM_SETUP\n"));
            /* Set PIO and queue ADC read */
            powerMonitorSetThermistorPio(TRUE);
            powerMonitorReadAdc(task, POWER_INTERNAL_VTHM_REQ, power->config.vthm.delay);
        break;
        
        case POWER_INTERNAL_VTHM_REQ:
            ADC_PRINT(("POWER: Rcv POWER_INTERNAL_VTHM_REQ\n"));
            /* Queue next setup if successful, otherwise queue immediate retry of ADC read */
            if(!powerMonitorReadAdcNow(task, POWER_INTERNAL_VTHM_SETUP, &power->config.vthm.adc, power_init_vthm))
                powerMonitorReadAdc(task, POWER_INTERNAL_VTHM_REQ, 0);
        break;
        
        case POWER_INTERNAL_CHG_MONITOR_REQ:
            CHG_MONITOR_PRINT(("POWER: Rcv POWER_INTERNAL_CHG_MONITOR_REQ\n"));
            powerChargerHandleMonitorReq();
        break;
        
        case POWER_INTERNAL_VBAT_NOTIFY_REQ:
            ADC_PRINT(("POWER: Rcv POWER_INTERNAL_VBAT_NOTIFY_REQ\n"));
            powerBatterySendVoltageInd();
        break;
        
        case POWER_INTERNAL_THERMAL_REQ:
            THERMAL_PRINT(("POWER: Rcv POWER_INTERNAL_THERMAL_REQ\n"));
            powerThermalHandleMonitorReq();
        break;
		
        case POWER_INTERNAL_CHG_PRG_MON_REQ:
            ADC_PRINT(("POWER: Rcv POWER_INTERNAL_CHG_PRG_MON_REQ\n"));
            powerUtilHandleChargerProgressMonitorReq(task);
        break;

        case MESSAGE_ADC_RESULT:
        {
            const MessageAdcResult* const result  = (const MessageAdcResult*)message;
            uint16            reading = result->reading;

            ADC_PRINT(("POWER: Rcv MESSAGE_ADC_RESULT with adc_source %u, reading %u\n", result->adc_source, result->reading));

            if(powerUtilAdcSourceIsVref(result->adc_source))
            {
                /* Store reading */
                power->vref = reading;
                POWER_INIT_SET(power_init_vref);
            }
            else if(powerUtilAdcSourceIsChargerProgressMonitor(result->adc_source))
            {
                /* Do not report charger progress to application. */
            }
            else
            {
                uint16 res = powerUtilGetVoltage(result, power->vref);
                ADC_PRINT(("POWER: ADC voltage: %umV\n", res));

                /* Processing the reading results */
                if(powerUtilAdcSourceIsVthm(result->adc_source))
                {
                    powerMonitorSetThermistorPio(FALSE);
                    powerBatteryHandleTemperatureReading(power->config.vthm.raw_limits ? reading : res);
                }
                else if(powerUtilAdcSourceIsVbat(result->adc_source))
                {
                    powerBatteryHandleVoltageReading(res);
                }
                else if(powerUtilAdcSourceIsVchg(result->adc_source))
                {
                    powerChargerHandleVoltageReading(res);
                }
                else
                {
                    Panic();
                }
            }
            
            if(POWER_INIT_IS_COMPLETE())
            {
                /* We have all our initial readings */
                PRINT(("POWER: INIT COMPLETE\n"));
                PowerInitComplete(power->clientTask, TRUE);
            }
        }
        break;
        
        default:
            Panic();
        break;
    }
}


/****************************************************************************
NAME
    powerMonitorStart
    
DESCRIPTION
    Start monitoring all values.
*/
static void powerMonitorStart(void)
{
    /* Start monitoring by taking VREF measurement*/
    powerMonitorReadAdc(&power->task, POWER_INTERNAL_VREF_REQ, 0);
    powerMonitorReadAdc(&power->task, POWER_INTERNAL_VTHM_SETUP, 0);
    powerMonitorReadAdc(&power->task, POWER_INTERNAL_VCHG_REQ, 0);
    powerMonitorReadAdc(&power->task, POWER_INTERNAL_VBAT_REQ, 0);
    powerMonitorReadAdc(&power->task, POWER_INTERNAL_CHG_PRG_MON_REQ, 0);
}


/****************************************************************************
NAME
    powerMonitorRestart
    
DESCRIPTION
    Restart monitoring all values.
*/
void powerMonitorRestart(void)
{
    /* Cancel all pending monitor requests */
    ADC_PRINT(("POWER: Ccl POWER_INTERNAL_VREF_REQ\n"));
    MessageCancelAll(&power->task, POWER_INTERNAL_VREF_REQ);
    ADC_PRINT(("POWER: Ccl POWER_INTERNAL_VTHM_SETUP\n"));
    MessageCancelAll(&power->task, POWER_INTERNAL_VTHM_SETUP);
    ADC_PRINT(("POWER: Ccl POWER_INTERNAL_VTHM_REQ\n"));
    MessageCancelAll(&power->task, POWER_INTERNAL_VTHM_REQ);
    ADC_PRINT(("POWER: Ccl POWER_INTERNAL_VCHG_REQ\n"));
    MessageCancelAll(&power->task, POWER_INTERNAL_VCHG_REQ);
    ADC_PRINT(("POWER: Ccl POWER_INTERNAL_VBAT_REQ\n"));
    MessageCancelAll(&power->task, POWER_INTERNAL_VBAT_REQ);
    ADC_PRINT(("POWER: Ccl POWER_INTERNAL_CHG_PRG_MON_REQ\n"));
    MessageCancelAll(&power->task, POWER_INTERNAL_CHG_PRG_MON_REQ);
    /* Restart monitoring */
    powerMonitorStart();
}

/****************************************************************************
NAME
    powerMonitorInit
    
DESCRIPTION
    Initialise power monitoring
*/
void powerMonitorInit(void)
{
    power->task.handler = powerMonitorHandler;
    PanicFalse(powerMonitorConfigureThermistorPio());
    
    powerCurrentLimitingInit();
    powerMonitorStart();

    powerThermalInit();
}

/****************************************************************************
NAME
    PowerOnChipMonitorStartChargerProgressMonitor

DESCRIPTION
    Start monitoring Charger Progress Monitor.

RETURNS
    void
*/
void PowerOnChipMonitorStartChargerProgressMonitor(void)
{
    powerUtilHandleChargerProgressMonitorReq(&power->task);
}

/****************************************************************************
NAME
    PowerOnChipMonitorStopChargerProgressMonitor

DESCRIPTION
    Stop monitoring Charger Progress Monitor.

RETURNS
    void
*/
void PowerOnChipMonitorStopChargerProgressMonitor(void)
{
    ADC_PRINT(("POWER: Ccl POWER_INTERNAL_CHG_PRG_MON_REQ\n"));
    MessageCancelAll(&power->task, POWER_INTERNAL_CHG_PRG_MON_REQ);
}

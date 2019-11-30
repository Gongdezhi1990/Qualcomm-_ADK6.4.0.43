/*
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

*/
/** 
\file 
\ingroup sink_app
\brief Support for Dormant Mode. 

*/

#include "sink_dormant.h"
#include "sink_debug.h"

#ifdef DEBUG_DORMANT
/* Ensure that we enable the development macros for a debug build */
#define DEVELOPMENT_BUILD
#endif /* DEBUG_DORMANT */

#include "sink_configmanager.h"
#include "sink_development.h"


#ifdef ENABLE_DORMANT_SUPPORT


#include "sink_low_power_mode_config_def.h"
#include <dormant.h>
#include <system_clock.h>

#undef SHORT_DORMANT_WAKEUP_PERIOD

#ifdef DEBUG_DORMANT

#define DORMANT_INFO(x) DEBUG(x)
#define DORMANT_ERROR(x) TOLERATED_ERROR(x)

#else /* DEBUG_DORMANT */

#define DORMANT_INFO(x)
#define DORMANT_ERROR(x)

#endif /* DEBUG_DORMANT */

static void enableDebugLed(void)
{
#ifdef DEBUG_DORMANT
    /* Enable LED2 when dormant. */
    DormantConfigure(LED_OUTPUT_DRIVE_MASK, 0x0004);
    DormantConfigure(LED_OUTPUT_LEVEL_MASK, 0x0000);
#endif
}

static rtime_t dormantDeadline(void)
{
    rtime_t current_time, deadline_time;
    current_time = SystemClockGetTimerTime();
#ifdef SHORT_DORMANT_WAKEUP_PERIOD
    rtime_t period_time = D_SEC(sinkDataGetLimboTimeout()) * US_PER_MS;
    deadline_time = rtime_add(current_time, period_time);
#else
#define MAX_DEADLINE_TIME ((rtime_t)(-1))
#define MAX_PERIOD        ((rtime_t)(0x7FFFFFFF))
    if (rtime_gt(current_time, MAX_PERIOD))
    {
        deadline_time = MAX_DEADLINE_TIME;
    }
    else
    {
        deadline_time = rtime_add(current_time, MAX_PERIOD);
    }
#endif
    return deadline_time;
}

bool sinkDormantIsWakeUpFromNfcConfigured(void)
{
    bool wake_up_from_nfc = FALSE;

    sink_low_power_mode_readonly_config_def_t *dormant_read_config = NULL;

    if (configManagerGetReadOnlyConfig(SINK_LOW_POWER_MODE_READONLY_CONFIG_BLK_ID, (const void **)&dormant_read_config))
    {
        wake_up_from_nfc = dormant_read_config->dormant_nfc;

        configManagerReleaseConfig(SINK_LOW_POWER_MODE_READONLY_CONFIG_BLK_ID);
    }

    return wake_up_from_nfc;
}


void sinkDormantEnable(bool periodic_wakeup_required, bool match_power_off_wakeup_sources)
{
    rtime_t deadline_time;
    bool deadline_valid = FALSE;

    sink_low_power_mode_readonly_config_def_t *dormant_read_config = NULL;
    
    if (configManagerGetReadOnlyConfig(SINK_LOW_POWER_MODE_READONLY_CONFIG_BLK_ID, (const void **)&dormant_read_config))
    {              
        if (match_power_off_wakeup_sources)
        {
            DORMANT_INFO(("Dormant: SYS_CTRL and VCHG wake up sources only, same as off state\n"));
        }
        else
        {
            DORMANT_INFO(("Dormant: SYS_CTRL and VCHG plus configured wake up sources\n"));

            if (dormant_read_config->dormant_xio)
            {
                /* Wake from dormant on an XIO event */
                DORMANT_INFO(("Dormant: XIO wake up [0x%x]\n", dormant_read_config->dormant_xio));
                if (!DormantConfigure(XIO_WAKE_MASK, dormant_read_config->dormant_xio))
                {
                    DORMANT_ERROR(("Dormant: Failed to configure XIO wake\n"));
                }
            }

            if (dormant_read_config->dormant_nfc)
            {
                /* Wake from dormant on an NFC field detect */
                DORMANT_INFO(("Dormant: NFC wake up [0x%x]\n", dormant_read_config->dormant_nfc));
                if (!DormantConfigure(NFC_WAKE_MASK, dormant_read_config->dormant_nfc))
                {
                    DORMANT_ERROR(("Dormant: Failed to configure NFC wake\n"));
                }
            }

            if (periodic_wakeup_required)
            {
                deadline_time = dormantDeadline();
                deadline_valid = TRUE;

                DORMANT_INFO(("Dormant: deadline [%uus]\n", deadline_time));
                if (!DormantConfigure(DEADLINE, deadline_time))
                {
                    DORMANT_ERROR(("Dormant: Failed to configure deadline\n"));
                }
            }
        }

        DORMANT_INFO(("Dormant: deadline valid [%s]\n", deadline_valid ? "TRUE" : "FALSE"));
        if (!DormantConfigure(DEADLINE_VALID, deadline_valid))
        {
            DORMANT_ERROR(("Dormant: Failed to configure deadline valid\n"));
        }

        enableDebugLed();

        /* Enter dormant mode */
        DORMANT_INFO(("Dormant: Enter Dormant Mode\n"));
        if (!DormantConfigure(DORMANT_ENABLE, 0))
        {
            DORMANT_INFO(("Dormant: Failed to enter dormant\n"));
        }
        
        configManagerReleaseConfig(SINK_LOW_POWER_MODE_READONLY_CONFIG_BLK_ID);
    }
}


#endif /* ENABLE_DORMANT_SUPPORT */


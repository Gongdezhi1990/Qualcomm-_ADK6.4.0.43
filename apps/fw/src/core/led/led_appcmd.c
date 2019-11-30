/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Implementation for led appcmd interface.
 */

#ifdef APPCMD_MODULE_PRESENT

#include "led/led_appcmd.h"
#include "led/led.h"
#include "led/led_private.h"

/**
 * Helper macro for testing parameters.
 */
#define param_assert(c) if(!(c)) return APPCMD_RESPONSE_INVALID_PARAMETERS

/**
 * Holds the led settings for passing to the driver. This can be accessed from 
 * pydbg: apps0.fw.env.struct("led_conf")
 */
static led_config led_conf;

/* Pydbg may want to use this enum */
PRESERVE_TYPE_FOR_DEBUGGING(led_appcmd_fields)

/**
 * Called by the scheduler when a background interrupt is generated. Not used.
 */
void led_bg_int_handler(void)
{
    
}

/**
 * Called by the scheduler. Not used.
 */
void led_msg_handler(void **context)
{
    UNUSED(context);
}

/**
 * Called by scheduler.
 */
void led_appcmd_init(void **context)
{
    UNUSED(context);
    if (!led_install_appcmd_handler())
    {
        L0_DBG_MSG("LED APPCMD: Could not register AppCmd handler");
        return;
    }
}

bool led_install_appcmd_handler(void)
{
    return appcmd_add_test_handler(APPCMD_TEST_ID_LED, led_appcmd_handler);
}

APPCMD_RESPONSE led_appcmd_handler (APPCMD_TEST_ID test_id,
                                          uint32 *params, 
                                          uint32 *results)
{
    led_instance_mask instances;
    
    UNUSED(test_id);
    UNUSED(results);
    
    instances = (led_instance_mask)appcmd_get_value(LED, PARAMS_INSTANCES, 
                                                                       params);
    
    switch (appcmd_get_value(LED, PARAMS_COMMAND, params))
    {
        case LED_APPCMD_INIT:
            led_init((appcmd_get_value(LED, PARAMS_INIT, params) == FALSE)? 
                                                                  FALSE : TRUE,
                     (led_clk_mux)
                              (appcmd_get_value(LED, PARAMS_CLK_MUX, params)));
            return APPCMD_RESPONSE_SUCCESS;
        case LED_APPCMD_CONFIGURE:
            param_assert((instances & LED_ALL) == instances);
            led_conf.initial_state = (led_state)(
                              appcmd_get_value(LED, PARAMS_INITSTATE, params));
            led_conf.pin_conf = (led_pin_config)(
                                appcmd_get_value(LED, PARAMS_PINCONF, params));
            led_conf.single_shot_mode = 
                        (appcmd_get_value(LED, PARAMS_SSEN, params) == FALSE)? 
                                                                 FALSE : TRUE; 
            led_conf.logarithmic_mode = 
                       (appcmd_get_value(LED, PARAMS_LOGEN, params) == FALSE)? 
                                                                 FALSE : TRUE; 
            led_conf.logarithmic_offset = (uint8)(
                       appcmd_get_value(LED, PARAMS_LOG_OFFSET_LO_HI, params));
            led_conf.min.lo = (uint16)(
                                 appcmd_get_value(LED, PARAMS_MIN_LO, params));
            led_conf.min.hi = (uint16)(
                                 appcmd_get_value(LED, PARAMS_MIN_HI, params));
            led_conf.max.lo = (uint16)(
                                 appcmd_get_value(LED, PARAMS_MAX_LO, params));
            led_conf.max.hi = (uint16)(
                                 appcmd_get_value(LED, PARAMS_MAX_HI, params));
            led_conf.hold.lo = (uint16)(
                                appcmd_get_value(LED, PARAMS_HOLD_LO, params));
            led_conf.hold.hi = (uint16)(
                                appcmd_get_value(LED, PARAMS_HOLD_HI, params));
            led_conf.ramp_current.lo = (uint16)(
                            appcmd_get_value(LED, PARAMS_RAMP_CUR_LO, params));
            led_conf.ramp_current.hi = (uint16)(
                            appcmd_get_value(LED, PARAMS_RAMP_CUR_HI, params));
            led_conf.single_shot.lo = (uint16)(
                                  appcmd_get_value(LED, PARAMS_SS_LO, params));
            led_conf.single_shot.hi = (uint16)(
                                  appcmd_get_value(LED, PARAMS_SS_HI, params));
            led_conf.ramp = (uint16)appcmd_get_value(LED, PARAMS_RAMP, params);
            led_conf.counthold_value = 
                      (uint16)appcmd_get_value(LED, PARAMS_COUNT_HOLD, params);
            led_configure(instances, &led_conf);
            return APPCMD_RESPONSE_SUCCESS;
        case LED_APPCMD_ENABLE:
            param_assert((instances & LED_ALL) == instances);
            led_enable(instances, (bool)
                                     appcmd_get_value(LED, PARAMS_EN, params));
            return APPCMD_RESPONSE_SUCCESS;
        case LED_APPCMD_RESET:
            led_soft_reset();
            return APPCMD_RESPONSE_SUCCESS;
        case LED_APPCMD_MAX_INSTANCES:
            appcmd_set_value(LED, RESULTS_MAX_INSTANCES, results, LED_ALL);
            return APPCMD_RESPONSE_SUCCESS;
        case LED_APPCMD_GET_CONFIG:
            return APPCMD_RESPONSE_UNIMPLEMENTED;
        case LED_APPCMD_SYNC:
            param_assert((instances & LED_ALL) == instances);
            led_sync(instances);
            return APPCMD_RESPONSE_SUCCESS;
        default:
            return APPCMD_RESPONSE_UNKNOWN_COMMAND;
    }
}

#endif /* APPCMD_MODULE_PRESENT */

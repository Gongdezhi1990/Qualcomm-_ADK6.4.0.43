/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implementation of LED configure module.
 *
 * This module gets built for both P1 and P0. For trap call the code runs on P1,
 * whereas for Production test code runs on P0. Trap call and production test
 * execution are mutually exclusive.
 */

#include "led_cfg/led_cfg_private.h"


#ifndef DESKTOP_TEST_BUILD


static const uint8 led_id_to_ctrl_inst[NUM_LED_CTRL_INST] =
    {0, 1, 2, 3, 4, 5};


static client_led_config led_conf[NUM_OF_LED_CTRLS];
static led_instance_mask enabled_leds = 0;

/**
 * @brief Updates PWM duty cycle and period. The PWM period varies according to
 * the LED_PERIOD configuration.
 * Assuming a 32 MHz clock the resulting PWM period is:
 *
 *     pwm_period = (LED_PERIOD + 1) * ~130us
 *
 * @param led Index of the LED driver to update.
 */
static void led_update_pwm(led_id led);

/**
 * @brief Enables LED flashing mode.
 * @param led Index of the LED driver to update.
 */
static void led_flash_enable(led_id led);

/**
 * @brief Synchronises all LEDs in the given group.
 * @param group Group ID.
 * @return TRUE if synchronisation was sucessful, FALSE otherwise.
 *
 * Disable driver for each led
 * Add led to group
 * Configure led
 * Synchronise (will enable led drivers)
 */
static bool synchronise_group(uint8 group);


bool led_cfg_initialise_led(led_id led)
{
    static bool led_hw_inited = FALSE;

    led_instance_mask led_mask =
        (led_instance_mask)(1 << led_id_to_ctrl_inst[led]);
    led_config *cfg = &led_conf[led].driver_config;

    if (!led_cfg_is_clocked())
    {
        return FALSE;
    }

    if(!led_hw_inited)
    {
        led_init(TRUE, CLK_MUX_PMU_FOSC);
        led_hw_inited = TRUE;
    }

    /* Initialise LED parameters */
    led_conf[led].state = LED_STATE_NORMAL;
    led_conf[led].duty_cycle = LED_MAX_DUTY_CYCLE_VALUE;
    led_conf[led].low_duty_cycle = 0;
    led_conf[led].period = LED_MAX_PERIOD_VALUE;
    led_conf[led].invert = FALSE;
    led_conf[led].groups = 0;
    cfg->initial_state = S_COUNT_HIGH_HOLD_MAX;
    cfg->pin_conf = PIN_PP;
    cfg->single_shot_mode = 1;
    cfg->logarithmic_mode = 0;
    cfg->logarithmic_offset = 0;
    cfg->min.lo = LED_MAX_PWM_PERIOD;
    cfg->min.hi = 0;
    cfg->max.lo = 0;
    cfg->max.hi = LED_MAX_PWM_PERIOD;
    cfg->hold.lo = 0;
    cfg->hold.hi = 0;
    cfg->ramp_current.lo = 0;
    cfg->ramp_current.hi = 0;
    cfg->single_shot.lo = LED_MAX_PWM_PERIOD;
    cfg->single_shot.hi = 0;
    cfg->ramp = 1;
    cfg->counthold_value = 0;

    led_configure(led_mask, cfg);

    L2_DBG_MSG1("LED %d initialised", led);

    return TRUE;
}

bool led_cfg_set_config(led_id led, led_config_key key, uint16 value)
{
    bool rtn = TRUE;
    bool update_config = TRUE;
    led_instance_mask led_mask =
                        (led_instance_mask)(1 << led_id_to_ctrl_inst[led]);
    led_config *cfg;

    if (led_cfg_is_clocked())
    {
        switch(key)
        {
            case LED_ENABLE:
                if (value)
                {
                    enabled_leds |= led_mask;
                    led_update_pwm(led);
                }
                else
                {
                    enabled_leds &= (led_instance_mask)(~led_mask);
                    led_enable(led_mask, FALSE);
                    update_config = FALSE;
                }
                break;
            case LED_DUTY_CYCLE:
                led_conf[led].duty_cycle =
                    (uint16)(value > LED_MAX_DUTY_CYCLE_VALUE ?
                            LED_MAX_DUTY_CYCLE_VALUE : value);
                led_update_pwm(led);
                break;
            case LED_PERIOD:
                led_conf[led].period = (uint8)(value > LED_MAX_PERIOD_VALUE ?
                    LED_MAX_PERIOD_VALUE : value);
                led_update_pwm(led);
                break;
            case LED_FLASH_ENABLE:
                if (value)
                {
                    led_conf[led].state = LED_STATE_FLASHING;
                    led_flash_enable(led);
                }
                else
                {
                    led_conf[led].state = LED_STATE_NORMAL;
                    led_update_pwm(led);
                }
                break;
            case LED_FLASH_RATE:
                /*
                 * Flash rate increases exponentially
                 * Ramp time = (2^(FLASH_RATE+1)-1) * min_state_duty_cycle *
                 *     pwm period / 0xFFF   (0..~128s)
                 */
                led_conf[led].driver_config.ramp =
                    (uint16)((1 << ((value > LED_MAX_FLASH_RATE ?
                        LED_MAX_FLASH_RATE : value) + 1)) - 1);
                break;
            case LED_FLASH_MAX_HOLD:
                if (led_conf[led].invert)
                {
                    led_conf[led].driver_config.hold.hi = value;
                }
                else
                {
                    led_conf[led].driver_config.hold.lo = value;
                }
                break;
            case LED_FLASH_MIN_HOLD:
                if (led_conf[led].invert)
                {
                    led_conf[led].driver_config.hold.lo = value;
                }
                else
                {
                    led_conf[led].driver_config.hold.hi = value;
                }
                break;
            case LED_FLASH_SEL_INVERT:
                if (((led_conf[led].invert == FALSE) && (value != 0)) ||
                    ((led_conf[led].invert != FALSE) && (value == 0)))
                {
                    uint16 hi;
                    led_conf[led].invert = (value ? TRUE : FALSE);
                    /* we need to swap the hold high/low values */
                    hi = led_conf[led].driver_config.hold.hi;
                    led_conf[led].driver_config.hold.hi =
                        led_conf[led].driver_config.hold.lo;
                    led_conf[led].driver_config.hold.lo = hi;
                    led_update_pwm(led);
                }
                break;
            case LED_FLASH_SEL_DRIVE:
                L2_DBG_MSG2(
                    "LedConfigure(%d, LED_FLASH_SEL_DRIVE, %u): "
                    "Not implemented", led, value);
                rtn = FALSE;
                break;
            case LED_FLASH_SEL_TRISTATE:
                L2_DBG_MSG2("LedConfigure(%d, LED_FLASH_SEL_TRISTATE, %u): "
                            "Not implemented", led, value);
                rtn = FALSE;
                break;
            case LED_FLASH_LOW_DUTY_CYCLE:
                led_conf[led].low_duty_cycle =
                    (uint16)(value > LED_MAX_DUTY_CYCLE_VALUE ?
                            LED_MAX_DUTY_CYCLE_VALUE : value);
                led_update_pwm(led);
                if (LED_STATE_FLASHING == led_conf[led].state)
                {
                    led_conf[led].driver_config.single_shot_mode = 0;
                }
                break;
            case LED_LOGARITHMIC_MODE_ENABLE:
                led_conf[led].driver_config.logarithmic_mode = value ? 1 : 0;
                break;
            case LED_LOGARITHMIC_MODE_OFFSET_HIGH:
                if (value <= MAX_LOGARITHMIC_MODE_OFFSET)
                {
                    led_conf[led].driver_config.logarithmic_offset =
                        (led_conf[led].driver_config.logarithmic_offset &
                         0x0F) |
                        (uint8)(value << 4);
                }
                else
                {
                    L2_DBG_MSG2(
                        "LedConfigure(%d, LED_LOGARITHMIC_MODE_OFFSET_HIGH, %u)"
                        ": Value out of range", led, value);
                    rtn = FALSE;
                }
                break;
            case LED_LOGARITHMIC_MODE_OFFSET_LOW:
                if (value <= MAX_LOGARITHMIC_MODE_OFFSET)
                {
                    led_conf[led].driver_config.logarithmic_offset =
                        (led_conf[led].driver_config.logarithmic_offset &
                         0xF0) |
                        (uint8)value;
                }
                else
                {
                    L2_DBG_MSG2(
                        "LedConfigure(%d, LED_LOGARITHMIC_MODE_OFFSET_LOW, %u)"
                        ": Value out of range", led, value);
                    rtn = FALSE;
                }
                break;
            case LED_ADD_TO_GROUP:
                if (value < 32)
                {
                    led_conf[led].groups |= (1 << value);
                }
                else
                {
                    L2_DBG_MSG2("LedConfigure(%d, LED_ADD_TO_GROUP, %u): "
                                "Invalid group", led, value);
                    rtn = FALSE;
                }
                update_config = FALSE;
                break;
            case LED_REMOVE_FROM_GROUP:
                if (value < 32)
                {
                    led_conf[led].groups &= ~(1 << value);
                }
                else
                {
                    L2_DBG_MSG2("LedConfigure(%d, LED_REMOVE_FROM_GROUP, %u): "
                                "Invalid group", led, value);
                    rtn = FALSE;
                }
                update_config = FALSE;
                break;
            case LED_SYNCHRONISE_GROUP:
                if (value >= 32)
                {
                    L2_DBG_MSG2("LedConfigure(%d, LED_SYNCHRONISE_GROUP, %u): "
                                "Invalid group", led, value);
                    rtn = FALSE;
                }
                else if (0 == (led_conf[led].groups & (1 << value)))
                {
                    L2_DBG_MSG3("LedConfigure(%d, LED_SYNCHRONISE_GROUP, %u): "
                                "LED not in group %d", led, value, value);
                    rtn = FALSE;
                }
                else
                {
                    if (synchronise_group((uint8)value))
                    {
                        update_config = FALSE;
                    }
                    else
                    {
                        rtn = FALSE;
                    }
                }
                break;
            case LED_INITIAL_STATE:
                cfg = &led_conf[led].driver_config;
                if (!led_conf[led].invert)
                {
                    switch(value)
                    {
                        case 0: /* Hold minimum brightness (low) */
                            cfg->initial_state = S_COUNT_LOW_HOLD_MAX;
                            break;
                        case 1: /* Hold minimum brightness (high) */
                            cfg->initial_state = S_COUNT_HIGH_HOLD_MAX;
                            break;
                        case 2: /* Ramp up to maximum brightness (low) */
                            cfg->initial_state = S_COUNT_LOW_RAMP_DOWN;
                            cfg->ramp_current.lo = cfg->max.lo;
                            cfg->ramp_current.hi = cfg->max.hi;
                            break;
                        case 3: /* Ramp up to maximum brightness (high) */
                            cfg->initial_state = S_COUNT_HIGH_RAMP_DOWN;
                            cfg->ramp_current.lo = cfg->max.lo;
                            cfg->ramp_current.hi = cfg->max.hi;
                            break;
                        case 4: /* Hold maximum brightness (low) */
                            cfg->initial_state = S_COUNT_LOW_HOLD_MIN;
                            break;
                        case 5: /* Hold maximum brightness (high) */
                            cfg->initial_state = S_COUNT_HIGH_HOLD_MIN;
                            break;
                        case 6: /* Ramp down to minimum brightness (low) */
                            cfg->initial_state = S_COUNT_LOW_RAMP_UP;
                            cfg->ramp_current.lo = cfg->min.lo;
                            cfg->ramp_current.hi = cfg->min.hi;
                            break;
                        case 7: /* Ramp down to minimum brightness (high) */
                            cfg->initial_state = S_COUNT_HIGH_RAMP_UP;
                            cfg->ramp_current.lo = cfg->min.lo;
                            cfg->ramp_current.hi = cfg->min.hi;
                            break;
                        default:
                            L2_DBG_MSG2("LedConfigure(%d, LED_START_UP_STATE,"
                                        " %u): Invalid value", led, value);
                            rtn = FALSE;
                            break;
                    }
                }
                else
                {
                    switch(value)
                    {
                        case 0: /* Hold minimum brightness (low) */
                            cfg->initial_state = S_COUNT_LOW_HOLD_MIN;
                            break;
                        case 1: /* Hold minimum brightness (high) */
                            cfg->initial_state = S_COUNT_HIGH_HOLD_MIN;
                            break;
                        case 2: /* Ramp up to maximum brightness (low) */
                            cfg->initial_state = S_COUNT_LOW_RAMP_UP;
                            cfg->ramp_current.lo = cfg->min.lo;
                            cfg->ramp_current.hi = cfg->min.hi;
                            break;
                        case 3: /* Ramp up to maximum brightness (high) */
                            cfg->initial_state = S_COUNT_HIGH_RAMP_UP;
                            cfg->ramp_current.lo = cfg->min.lo;
                            cfg->ramp_current.hi = cfg->min.hi;
                            break;
                        case 4: /* Hold maximum brightness (low) */
                            cfg->initial_state = S_COUNT_LOW_HOLD_MAX;
                            break;
                        case 5: /* Hold maximum brightness (high) */
                            cfg->initial_state = S_COUNT_HIGH_HOLD_MAX;
                            break;
                        case 6: /* Ramp down to minimum brightness (low) */
                            cfg->initial_state = S_COUNT_LOW_RAMP_DOWN;
                            cfg->ramp_current.lo = cfg->max.lo;
                            cfg->ramp_current.hi = cfg->max.hi;
                            break;
                        case 7: /* Ramp down to minimum brightness (high) */
                            cfg->initial_state = S_COUNT_HIGH_RAMP_DOWN;
                            cfg->ramp_current.lo = cfg->max.lo;
                            cfg->ramp_current.hi = cfg->max.hi;
                            break;
                        default:
                            L2_DBG_MSG2("LedConfigure(%d, LED_START_UP_STATE,"
                                        " %u): Invalid value", led, value);
                            rtn = FALSE;
                            break;
                    }
                }
                break;
            case LED_COUNTHOLD:
                led_conf[led].driver_config.counthold_value = value;
                break;
            case LED_PIN_DRIVE_CONFIG:
                if (value < 4)
                {
                    led_conf[led].driver_config.pin_conf =
                                                         (led_pin_config)value;
                }
                else
                {
                    L2_DBG_MSG2("LedConfigure(%d, LED_PIN_CONFIG, %u): "
                                "Invalid value", led, value);
                    rtn = FALSE;
                }
                break;
            default:
                L2_DBG_MSG3("LedConfigure(%d, %u, %u): Invalid config key",
                            led, key, value);
                rtn = FALSE;
        }
    }
    else
    {
        rtn = FALSE;
    }

    if (rtn && update_config)
    {
        led_cfg_log_config(led, led_conf);
        if (enabled_leds & led_mask)
        {
            led_enable(led_mask, FALSE);
            led_configure(led_mask, &led_conf[led].driver_config);
            led_enable(led_mask, TRUE);
        }
        else
        {
            led_configure(led_mask, &led_conf[led].driver_config);
        }
    }

    return rtn;
}

static void led_update_pwm(led_id led)
{
    led_config *cfg = &led_conf[led].driver_config;
    uint16 pwm_period = (uint16)((led_conf[led].period + 1) *
        LED_MAX_PWM_PERIOD / (LED_MAX_PERIOD_VALUE + 1));

    cfg->max.lo =
        (uint16)((led_conf[led].low_duty_cycle * pwm_period) /
                 LED_MAX_DUTY_CYCLE_VALUE);
    cfg->max.hi = (uint16)(pwm_period - cfg->max.lo);

    cfg->min.lo =
        (uint16)((led_conf[led].duty_cycle * pwm_period) /
                 LED_MAX_DUTY_CYCLE_VALUE);
    cfg->min.hi = (uint16)(pwm_period - cfg->min.lo);

    if (LED_STATE_NORMAL == led_conf[led].state)
    {
        cfg->single_shot_mode = 1;
        if (led_conf[led].invert)
        {
            cfg->initial_state = S_COUNT_HIGH_RAMP_UP;
            cfg->ramp_current.lo = (uint16)(cfg->max.lo + 1);
            cfg->ramp_current.hi = (uint16)(cfg->max.hi - 1);

            cfg->single_shot.lo = cfg->max.lo;
            cfg->single_shot.hi = cfg->max.hi;
        }
        else
        {
            cfg->initial_state = S_COUNT_HIGH_RAMP_DOWN;
            cfg->ramp_current.lo = (uint16)(cfg->min.lo - 1);
            cfg->ramp_current.hi = (uint16)(cfg->min.hi + 1);

            cfg->single_shot.lo = cfg->min.lo;
            cfg->single_shot.hi = cfg->min.hi;
        }
    }
}

static void led_flash_enable(led_id led)
{
    led_config *cfg = &led_conf[led].driver_config;
    cfg->single_shot_mode = 0;
}

static bool synchronise_group(uint8 group)
{
    uint32 group_mask = 1 << group;
    led_instance_mask led_mask = 0;
    uint8 led;

    for (led = 0; led < NUM_OF_LED_CTRLS; led++)
    {
        if (led_conf[led].groups & group_mask)
        {
            led_mask |= (led_instance_mask)(1 << led_id_to_ctrl_inst[led]);
        }
    }
    if (led_cfg_is_clocked())
    {
        L2_DBG_MSG1("Synchronising LEDs: 0x%X", led_mask);
        led_sync(led_mask);
        led_enable(led_mask, TRUE);
        enabled_leds |= led_mask;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


#endif /* DESKTOP_TEST_BUILD */

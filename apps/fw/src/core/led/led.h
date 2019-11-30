/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Bitserial public header file.
 * This file contains public interfaces.
 */
#ifndef LED_H
#define LED_H

#include "hydra/hydra_types.h"
#include "hal/hal_led.h"

/**
 * Type definition for the instance mask.
 */
typedef uint16 led_instance_mask;

/**
 * Type definition for the pin configuration.
 */
typedef hal_led_pin_config led_pin_config;

/**
 * Type definition for the led state. Used mainly for signalling the initial 
 * state.
 */
typedef hal_led_state led_state;

/**
 * Type definition for the clock muxing options. This is used for 
 * initialisation.
 */
typedef hal_led_clk_mux led_clk_mux;

/**
 * Type definition for led configuration.
 */
typedef struct led_config
{
    led_state initial_state;
    led_pin_config pin_conf;
    bool single_shot_mode;
    bool logarithmic_mode;
    uint8 logarithmic_offset;
    struct
    {
        uint16 lo;
        uint16 hi;
    }min;
    struct
    {
        uint16 lo;
        uint16 hi;
    }max;
    struct
    {
        uint16 lo;
        uint16 hi;
    }hold;
    struct
    {
        uint16 lo;
        uint16 hi;
    }ramp_current;
    struct
    {
        uint16 lo;
        uint16 hi;
    }single_shot;
    uint16 ramp;
    uint16 counthold_value;
}led_config;

/**
 * Initialises or deinitialises the hardware. This controls the lock muxing and
 * turns on/off the main clock for the peripheral.
 * \param en Signals if initialisation(TRUE) or deinitialisation(FALSE) is 
 *           required.
 * \param clk_mux Signals which clock muxing option is required.
 */
void led_init(bool en, led_clk_mux clk_mux);

/**
 * Configures one or more led controllers.
 * \param instances Instance mask indicating to which led controller the 
 *                  configuration is to be passed.
 * \param led_conf Pointer to a led_config structure holding the configuration 
 *                 data
 */
void led_configure(led_instance_mask instances, led_config *led_conf);

/**
 * Soft resets all led controllers. The hardware register interface remains 
 * intact.
 */
void led_soft_reset(void);

/**
 * Enables or disables the main clock deglitching filter. This is enabled by 
 * default and should not be disabled for normal use cases.
 * \param en Signals if enable or disable is required.
 */
void led_clk_gate_dont_deglitch(bool en);

/**
 * Synchronises multiple led controllers. This essentially means they are set 
 * to their starting state at the same time.
 * \param instances Instance mask indicating to led controller to update and
 *                  enable.
 */
void led_sync(led_instance_mask instances);

/**
 * Enables/disables the indicated led controllers.
 * \param instances Instance mask indicating to led controller to update and
 *                  enable.
 * \param en Signals if enable or disable is required.
 */
void led_enable(led_instance_mask instances, bool en);

#endif /* LED_H */

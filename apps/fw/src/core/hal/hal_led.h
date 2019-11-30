/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Led hardware abstraction layer header file.
 * This file contains the interface abstraction to the led hardware. 
*/

#ifndef HAL_LED_H
#define HAL_LED_H

#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"
#define IO_DEFS_MODULE_LED_CTRL
#define IO_DEFS_MODULE_APPS_SYS_CLKGEN
#include "io/io_map.h"
#include "hal/hal_macros.h"
#include "io/io_defs.h"

/* Macros for abstracting the lower hardware interface. */
#define hal_led_set_ctrl_soft_reset(x) hal_set_reg_led_ctrl_soft_reset(x)

#define hal_led_set_ctrl_clk_mux_sel(x) hal_set_led_ctrl_clk_mux_sel(x)

#define hal_led_set_ctrl_clk_enable(x) hal_set_reg_led_ctrl_clk_enable(x)

#define hal_led_set_debug_select(x) hal_set_reg_led_debug_select(x)

#define hal_led_set_ctrl_clk_gate_dont_deglitch(x) \
                                 hal_set_reg_led_ctrl_clk_gate_dont_deglitch(x)

#define hal_led_set_update(x) hal_set_reg_led_update(x)

#define hal_led_set_en(x) hal_set_reg_led_en(x)
#define hal_led_get_en() hal_get_reg_led_en()

#define hal_led_set_pin_config(x) hal_set_reg_led_pin_config(x)

#define hal_led_set_index(x) hal_set_reg_led_index(x)

#define hal_led_set_configure(x) hal_set_reg_led_configure(x)

#define hal_led_set_ramp_config(x) hal_set_reg_led_ramp_config(x)

#define hal_led_set_start_up_state(x) hal_set_reg_led_start_up_state(x)

#define hal_led_set_counthold_value(x) hal_set_reg_led_counthold_value(x)

#define hal_led_set_logarithmic_en(x) hal_set_reg_led_logarithmic_en(x)

/**
 * Clock option type definition. This is used to choose which clock source to 
 * hook up to the led module.
 */
typedef enum hal_led_clk_mux
{
    CLK_MUX_PMU_FOSC = 0x00,
    CLK_MUX_80MHZ_AUX = 0x01
}hal_led_clk_mux;


/**
 * Mask which refers all leds.
 */
#define LED_ALL ((1u<<NUM_OF_LED_CTRLS)-1)

/**
 * Type definition for pin operation mode.
 */
typedef enum hal_led_pin_config
{
    PIN_PP = 0x00, /* PUSH-PULL */
    PIN_OD = 0x01, /* OPEN DRAIN */
    PIN_OS = 0x10, /* OPEN-SOURCE */
    PIN_NPP = 0x11 /* PUSH-PULL INVERTED */
}hal_led_pin_config;

/**
 * Type definition for led state. This is used to set the initial led states.
 * This is preserved for the following cycled and thus can be used to control 
 * the phase of leds relative to each other.
 */
typedef enum hal_led_state
{
    S_COUNT_LOW_HOLD_MIN = 0x00,
    S_COUNT_HIGH_HOLD_MIN = 0x01,
    S_COUNT_LOW_RAMP_UP = 0x02,
    S_COUNT_HIGH_RAMP_UP = 0x03,
    S_COUNT_LOW_HOLD_MAX = 0x04,
    S_COUNT_HIGH_HOLD_MAX = 0x05,
    S_COUNT_LOW_RAMP_DOWN = 0x06,
    S_COUNT_HIGH_RAMP_DOWN = 0x07
}hal_led_state;


/**
 * Sets the configuration for the minimum brightness.
 * \param low The duration of the low period in units of ~30us assuming 32KHz 
 *            clock.
 * \param high The duration of the high period in units of ~30us assuming 32KHz
 *            clock.
 * \note We Must always have max low <= min low and max high >= min high
 */
void hal_led_set_min_config(uint16 low, uint16 high);

/**
 * Sets the configuration for the maximum brightness.
 * \param low The duration of the low period in units of ~30us assuming 32KHz 
 *            clock.
 * \param high The duration of the high period in units of ~30us assuming 32KHz
 *            clock.
 * \note We Must always have max low <= min low and max high >= min high
 */
void hal_led_set_max_config(uint16 low, uint16 high);

/**
 * Sets the configuration for the hold state.
 * \param low The duration for which the led brightness is held at min levels 
 *            in units of ~16ms assuming 32KHz clock.
 * \param high The duration for which the led brightness is held at max levels 
 *             in units of ~16ms assuming 32KHz clock.
 */
void hal_led_set_hold_config(uint16 low, uint16 high);

/**
 * Sets the initial duty cycle for the ramp states.
 * \param low The duration of the low period in units of ~30ms assuming 32KHz 
 *            clock.
 * \param high The duration of the high period in units of ~30ms assuming 32KHz
 *             clock.
 */
void hal_led_set_ramp_current(uint16 low, uint16 high);

/**
 * Sets the intermediate duty cycle used for single shot mode.
 * \param low The duration of the low period in units of ~30us assuming 32KHz 
 *            clock.
 * \param high The duration of the high period in units of ~30us assuming 32KHz
 *            clock.
 * \note Must always have low period between range max low min low and high 
 * period between range max high, min high.
 */
void hal_led_set_single_shot_config(uint16 low, uint16 high);

/**
 * Sets the offset into the logarithmic function when ramping.
 * \param offset The least significant 4 bits set the offset into the 
 *               logarithmic function when ramping down and the most 
 *               significant 4 bits set the offset into the logarithmic 
 *               function when ramping up
 */
void hal_led_set_logarithmic_offset(uint8 offset);


/**
 * Enables or Disables single shot mode for several LEDs at once.
 * \param mask Mask representing the LEDs to alter.
 * \param en TRUE if single shot needs to be enabled, FALSE otherwise.
 */
void hal_led_set_single_shot_en(uint16 mask, bool en);



#endif /* HAL_LED_H */

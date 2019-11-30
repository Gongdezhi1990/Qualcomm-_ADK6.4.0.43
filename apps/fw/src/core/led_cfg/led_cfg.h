/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Led control public header file.
 */
#ifndef LED_CFG_H
#define LED_CFG_H

#include "hydra/hydra_types.h"
#include <app/led/led_if.h>
#include "led/led.h"

#ifndef DESKTOP_TEST_BUILD


/**
 * Initialises an LED driver. When this function is called the LED Controller
 * is initialised enabling the clock.
 * \param led LED index.
 * \return TRUE if LED controller was successfully initialised, FALSE otherwise.
 */
bool led_cfg_initialise_led(led_id led);

/**
 * Configures one or more led controllers.
 * \param led The LED to configure.
 * \param key The LED parameter to configure. Refer to \c led_config_key for
 * details on each key.
 * \param value The value of the parameter specified by "key".
 * \return TRUE if parameters are valid, otherwise FALSE.
 */
bool led_cfg_set_config(led_id led, led_config_key key, uint16 value);

/**
 * Updates the mask of the PIOs muxed to the LED controller.
 * \param bank Bank ID.
 * \param mask Mask of PIOs to change.
 * \param is_muxed_to_led TRUE if selected PIOs have been muxed to the LED
 * controller, FALSE if they have been muxed to something else.
 */
void led_cfg_pio_mux_mask_update(uint16 bank,
                                 uint32 mask,
                                 bool is_muxed_to_led);

/**
 * Checks if given led ID has PIO assignments.
 * \param led Led ID to check for PIO assignments.
 * \return TRUE if led ID has a PIO assigned, FALSE otherwise.
 */
bool led_cfg_led_has_pio(led_id led);


#else /* DESKTOP_TEST_BUILD */


/** Stub out led_cfg_pio_mux_mask_update. */
#define led_cfg_pio_mux_mask_update(bank, mask, is_muxed_to_led)


#endif /* DESKTOP_TEST_BUILD */
#endif /* LED_CFG_H */

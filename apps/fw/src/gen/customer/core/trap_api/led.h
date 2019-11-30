#ifndef __LED_H__
#define __LED_H__
#include <app/led/led_if.h>

/*! file  @brief Traps to control the LED hardware present on some BlueCore variants */

#if TRAPSET_LED

/**
 *  \brief Control the LED hardware present on some BlueCore variants
 *     More detailed information on the keys and values can be found in led_if.h
 *     See the data sheet for accurate information on which LED hardware a given
 *  BlueCore
 *     variant supports.
 *     Before using this trap at least one PIO needs to be assigned to the LED
 *  controller.
 *     This is done by using PioSetMapPins32Bank to put the PIO under HW control
 *  and then
 *     PioSetFunction to set the function to LED. If there is an LED pad that can
 *  be used
 *     by an LED controller instance and that LED controller instance does not
 *  have any PIO
 *     assigned to it, then the LED pad will be automatically configured for the
 *  LED function
 *     on the first call to this trap.
 *  \param led The LED to configure 
 *  \param key Which LED parameter to configure. See led_config_key for the documentation of
 *  each key. 
 *  \param value The value the parameter specified by "key" should be set to 
 *  \return TRUE if the \#led_id is valid and the input value is in valid range and there is
 *  at least one PIO assigned to the LED controller using PioSetFunction, else
 *  FALSE.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_led
 */
bool LedConfigure(led_id led, led_config_key key, uint16 value);
#endif /* TRAPSET_LED */
#endif

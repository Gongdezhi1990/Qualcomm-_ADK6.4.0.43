#ifndef __LED_IF_H__
#define __LED_IF_H__

/* Copyright (c) 2016, 2019 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    led_if.h

CONTAINS
    Definitions for the led subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/


/* Note that code in bccmd_process.c and vm_trap_led.c relies on 
 * LED_0 == 0, LED_1 == 1, etc */

/*! @brief IDs used to configure the LED pads with the LedConfigure() trap. Some
BlueCore variants support no LED pads at all, some support two LED pads
(LED_0 and LED_1) and some support three or more LED pads.*/
typedef enum 
{
    LED_0,
    LED_1,
    LED_2,
    LED_3,
    LED_4,
    LED_5,
    LED_6,
    LED_7,
    LED_8,
    LED_9,
    LED_10,
    LED_11,
    LED_ID_N
}led_id;

/*! @brief LED config keys.

#LED_DUTY_CYCLE and #LED_PERIOD are used to dim the LED brightness. In order to use
these keys, an LED must be enabled by calling LedConfigure(led_id, LED_ENABLE, 1);

#LED_FLASH_ENABLE and #LED_FLASH_RATE are used to flash the LEDs.
In order to use these keys, an LED must be enabled.
 */
typedef enum 
{
    /*!
When using this key, a zero value will disable the LED pad and a
non zero value will enable the LED pad.

Examples:

\code
LedConfigure(LED_0, LED_ENABLE, 1);  Turn on LED 0

LedConfigure(LED_0, LED_ENABLE, 0);  Turn off LED 0
\endcode
     */
    LED_ENABLE,
    
    /*!
LED_DUTY_CYCLE controls the the PWM duty cycle. Valid values with this key
range from 0..0xFFF. 0x0 turns the LED off fully, 0xFFF turns the LED on fully. All other
values set the duty cycle to the value /4096.
     */
    LED_DUTY_CYCLE,
    
    /*!
LED_PERIOD controls the PWM period. Valid values with this key range from 0..0xF.
pwm_period is calculated as follows:

pwm_period = clock_period * 4096 * 2^LED_PERIOD

Where clock_period is 1/26MHz for CSR8670/CSR8610 and 1/16MHz for earlier.

CSR8670/CSR8610:  The resultant range of values is 157uS...5.162S at all times.
There is a normalisation adjustment that checks that
LED_FLASH_RATE >= LED_PERIOD. The normalisation check will increase
LED_FLASH_RATE if necessary, and occurs whenever LED_PERIOD is configured.
The normalisation adjustment can be circumvented by configuring LED_PERIOD
before configuring LED_FLASH_RATE.

Pre-CSR8670/CSR8610:  The resultant range of values is 256us...8.39s when
BlueCore is running at full speed. The clock period will be longer when
BlueCore is in shallow or deep sleep.

In CSRA6810x is not possible to achieve the same period as other chips above.
Assuming a 32MHz clock pwm_period is calculated as follows:

    pwm_period = (LED_PERIOD + 1) * ~130us

     */
    LED_PERIOD,

    /*!
When using this key, a zero value will disable the LED
flashing hardware and a non zero value will enable the flashing hardware.
     */
    LED_FLASH_ENABLE,

    /*!
Set the rate to flash/pulse LED. Valid values to use with
this key range from 0..0xF.

LED_FLASH_RATE sets the flash pulse period as a multiple of pwm_period.

Led Flash Rate is calculated as follows:

Led Flash Period = pwm_period * 96 * 2^LED_FLASH_RATE

Where pwm_period is as calculated above.

And so, LED_FLASH_RATE maps to a multiplier as follows.

LED_FLASH_RATE = 0:     Led Flash Period = pwm_period * 96

LED_FLASH_RATE = 1:     Led Flash Period = pwm_period * 192

LED_FLASH_RATE = 2:     Led Flash Period = pwm_period * 384

LED_FLASH_RATE = 3:     Led Flash Period = pwm_period * 768

LED_FLASH_RATE = 4:     Led Flash Period = pwm_period * 1536

...

LED_FLASH_RATE = 15:    Led Flash Period = pwm_period * 3145728



In CSRA6810x or QCCxxxx the flash rate has changed. The LED_FLASH_RATE value has to be used
together with LED_FLASH_MAX_HOLD and LED_FLASH_MIN_HOLD. The resulting flashing period
will be:

    ramp_time = (2^(LED_FLASH_RATE + 1) - 1) * (duty_cycle - low_duty_cycle) * pwm_period / 4095   (0..~128s)
    Led Flash Period = 2 * ramp_time + LED_FLASH_MAX_HOLD + LED_FLASH_MIN_HOLD

     */
    LED_FLASH_RATE,

    /*!
Sets hold time for max PWM when flashing. This is represented in units of ~16us
assuming a 32MHz clock.
     */
    LED_FLASH_MAX_HOLD,

    /*!
Sets hold time for min PWM when flashing. This is represented in units of ~16us
assuming a 32MHz clock.
     */
    LED_FLASH_MIN_HOLD,

    /*!
Inverts the PWM output. When using this key, a zero
value will disable the feature and a non zero value will enable the feature.
     */
    LED_FLASH_SEL_INVERT,

    /*!
Selects the status of DRIVE pad if Drive EnableB is
used for LED output.  When using this key, a zero value will disable the feature
and a non zero value will enable the feature.
This configuration key is not supported by CSRA6810x LED controller
hardware.
     */
    LED_FLASH_SEL_DRIVE,

    /*!
Selects that the LED PWM output controls the Pad drive
enableB rather than output. When using this key, a zero value will disable the 
feature and a non zero value will enable the feature.

This configuration key is not supported by CSRA6810x LED controller
hardware.
     */
    LED_FLASH_SEL_TRISTATE,

    /*!
LED_FLASH_LOW_DUTY_CYCLE controls the the PWM duty cycle when flashing is in low
state. Valid values with this key range from 0..0xFFF. 0x0 turns the LED off fully,
0xFFF turns the LED on fully. All other values set the duty cycle to the value /4096.
    */
    LED_FLASH_LOW_DUTY_CYCLE,

    /*!
When using this key a zero value will disable logarithmic fading. A non zero value
will enable logarithmic fading.
    */
    LED_LOGARITHMIC_MODE_ENABLE,

    /*!
Sets the offset into the logarithmic function when ramping up.
Valid offset values have to be between 0 and 15.
    */
    LED_LOGARITHMIC_MODE_OFFSET_HIGH,

    /*!
Sets the offset into the logarithmic function when ramping down.
Valid offset values have to be between 0 and 15.
    */
    LED_LOGARITHMIC_MODE_OFFSET_LOW,

    /*!
Adds the LED to a synchronisation group. Valid group values are
between 0 and 31.
    */
    LED_ADD_TO_GROUP,

    /*!
Removes the LED from a synchronisation group. Valid group values are
between 0 and 31.
    */
    LED_REMOVE_FROM_GROUP,

    /*!
Synchronises all LEDs in a synchronisation group. An LED can belong to
multiple groups and it can be synchronised to any of them. The group
is identified using the value parameter.If the selected LED is not in
the group the trap returns FALSE without synchronising the group.
    */
    LED_SYNCHRONISE_GROUP,

    /*!
Sets the initial state in the flashing state machine.
    LED_INITIAL_STATE = 0:  Hold minimum brightness (low)
    LED_INITIAL_STATE = 1:  Hold minimum brightness (high)
    LED_INITIAL_STATE = 2:  Ramp up to maximum brightness (low)
    LED_INITIAL_STATE = 3:  Ramp up to maximum brightness (high)
    LED_INITIAL_STATE = 4:  Hold maximum brightness (low)
    LED_INITIAL_STATE = 5:  Hold maximum brightness (high)
    LED_INITIAL_STATE = 6:  Ramp down to minimum brightness (low)
    LED_INITIAL_STATE = 7:  Ramp down to minimum brightness (high)
Please note that the initial state is taken into account only if
\c LED_FLASH_ENABLE is enabled beforehand.
    */
    LED_INITIAL_STATE,

    /*!
Sets the counthold counter in the bright or dull hold state (in units of ~16us
assuming a 32MHz clock). To take effect the initial state has to be either a
hold max or a hold min state. Also, \c LED_FLASH_MAX_HOLD and/or
\c LED_FLASH_MIN_HOLD must be set. Assuming the initial state is "hold maximum
brightness" and LED_FLASH_MAX_HOLD is 0xF000, setting LED_COUNTHOLD to 0x4000
will cause the inital hold period to be 0xF000 - 0x4000 = 0xB000. Further hold
periods will use the full ammount (0xF000).
    */
    LED_COUNTHOLD,

    /*!
Sets the Pin drive config for LED chosen using LED_INDEX when
running in LED mode, encoded as:

    0 = push-pull
    1 = open-drain
    2 = open-source
    3 = push-pull inverted

For normal PIO pad it should be push-pull mode and LED pad it should be open-drain mode
    */
    LED_PIN_DRIVE_CONFIG

}led_config_key;

#endif /* __LED_IF_H__  */


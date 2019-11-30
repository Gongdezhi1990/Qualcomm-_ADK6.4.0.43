/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Implementation of led HAL layer. This file contains functions which are 
 * interfaced directly to the HW, hence this should be the only place for 
 * changes when porting the led FW on another platform.
 */

#include "hal/hal_led.h"


void hal_led_set_min_config(uint16 low, uint16 high)
{
    hal_set_reg_led_min_low_config(low);
    hal_set_reg_led_min_high_config(high);
}

void hal_led_set_max_config(uint16 low, uint16 high)
{
    hal_set_reg_led_max_low_config(low);
    hal_set_reg_led_max_high_config(high);
}

void hal_led_set_hold_config(uint16 low, uint16 high)
{
    hal_set_reg_led_hold_low_config(low);
    hal_set_reg_led_hold_high_config(high);
}

void hal_led_set_ramp_current(uint16 low, uint16 high)
{
    hal_set_reg_led_ramp_current_low_config(low);
    hal_set_reg_led_ramp_current_high_config(high);
}

void hal_led_set_single_shot_config(uint16 low, uint16 high)
{
    hal_set_reg_led_single_shot_low_config(low);
    hal_set_reg_led_single_shot_high_config(high);
}

void hal_led_set_logarithmic_offset(uint8 offset)
{
    hal_set_reg_led_logarithmic_offset_low(offset & 0xf);
    hal_set_reg_led_logarithmic_offset_high((offset >> 4) & 0xf);
}

void hal_led_set_single_shot_en(uint16 mask, bool en)
{
    hal_set_reg_led_single_shot_mode(hal_get_reg_led_single_shot_mode() &
                                     (~mask) | (en?mask:0));
}

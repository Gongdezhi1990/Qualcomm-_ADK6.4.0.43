/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Implementation of led driver. This file implements functions used in led 
 * control.
 */

#include "led/led.h"

void led_init(bool en, led_clk_mux clk_mux)
{
    uint32 i;
    UNUSED(clk_mux);

    if (en == FALSE)
    {
        hal_led_set_ctrl_clk_enable(FALSE);
    }
    else
    {
        hal_led_set_en(0);
        hal_led_set_update(0);
        hal_led_set_single_shot_en(0xffff, FALSE);
        hal_led_set_ctrl_soft_reset(0);
        hal_led_set_debug_select(0);
        hal_led_set_min_config(0, 0);
        hal_led_set_max_config(0, 0);
        hal_led_set_ramp_config(0);
        hal_led_set_hold_config(0, 0);
        hal_led_set_pin_config(0);
        hal_led_set_start_up_state(0);
        hal_led_set_counthold_value(0);
        hal_led_set_ramp_current(0, 0);
        hal_led_set_single_shot_config(0, 0);
        hal_led_set_logarithmic_en(0);
        hal_led_set_logarithmic_offset(0);

        for(i = 0; i < NUM_OF_LED_CTRLS; i++)
        {
            hal_led_set_index(i);
            hal_led_set_configure(TRUE);
        }
        hal_led_set_index(0);
        hal_led_set_configure(FALSE);

        led_soft_reset();

        hal_led_set_ctrl_clk_enable(TRUE);
    }
}

void led_configure(led_instance_mask instances, led_config *led_conf)
{
    uint16 instance_id = 0;
    led_instance_mask instances_tmp = instances;

    while (instances_tmp)
    {
        if (instances_tmp & 0x01)
        {
            /* It turns out single shot mode cannot be entered immediately. We 
               need to configure the led in normal mode, update it and then we
               can enter single shot mode. */

            hal_led_set_index(instance_id);

            /* set the non single shot parameters */
            hal_led_set_start_up_state(led_conf->initial_state);
            hal_led_set_pin_config(led_conf->pin_conf);
            hal_led_set_logarithmic_en(led_conf->logarithmic_mode);
            hal_led_set_logarithmic_offset(led_conf->logarithmic_offset);
            hal_led_set_min_config(led_conf->min.lo, led_conf->min.hi);
            hal_led_set_max_config(led_conf->max.lo, led_conf->max.hi);
            hal_led_set_hold_config(led_conf->hold.lo, led_conf->hold.hi);
            hal_led_set_ramp_current(led_conf->ramp_current.lo, 
                                                    led_conf->ramp_current.hi);
            hal_led_set_single_shot_config(led_conf->single_shot.lo, 
                                                     led_conf->single_shot.hi);
            hal_led_set_ramp_config(led_conf->ramp);
            hal_led_set_counthold_value(led_conf->counthold_value);
            hal_led_set_configure(TRUE);
            led_sync((led_instance_mask)(1 << instance_id));
            
        }
        instances_tmp = instances_tmp >> 1;
        instance_id++;
    }
    /* A rising edge is needed to update single shot parameters and keep high
       to keep single shot mode enabled */
    hal_led_set_single_shot_en(instances, FALSE);
    if (led_conf->single_shot_mode)
    {
        hal_led_set_single_shot_en(instances, TRUE);
    }
}

void led_sync(led_instance_mask instances)
{
    /* this needs one led clock before clearing */
    hal_led_set_update(instances);

    /* Clear */
    hal_led_set_update(0);
}

void led_enable(led_instance_mask instances, bool en)
{
    if (en == TRUE)
    {
        hal_led_set_en(hal_led_get_en() | instances);
    }
    else
    {
        hal_led_set_en(hal_led_get_en() & (~instances));
    }
}

void led_soft_reset(void)
{
    hal_led_set_ctrl_soft_reset(TRUE);
    hal_led_set_ctrl_soft_reset(FALSE);
}


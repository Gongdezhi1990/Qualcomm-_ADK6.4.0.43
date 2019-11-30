/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implementation of LED configure module.
 */

#include "led_cfg/led_cfg_private.h"

#ifndef DESKTOP_TEST_BUILD


static uint32 led_cfg_pio_mux_mask[NUMBER_OF_PIO_BANKS];

bool led_cfg_is_clocked(void)
{
    uint32 i;
    bool ret = FALSE;

    ATOMIC_BLOCK_START {
        for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
        {
            if (led_cfg_pio_mux_mask[i])
            {
                ret = TRUE;
            }
        }
    } ATOMIC_BLOCK_END;

    return ret;
}

void led_cfg_pio_mux_mask_update(uint16 bank, uint32 mask, bool is_muxed_to_led)
{
    ATOMIC_BLOCK_START {
        led_cfg_pio_mux_mask[bank] = BIT_WRITE_MASK(led_cfg_pio_mux_mask[bank],
                                                    mask,
                                                    is_muxed_to_led?mask:0);
    } ATOMIC_BLOCK_END;
}

bool led_cfg_led_has_pio(led_id led)
{
    bool ret = FALSE;
    uint32 bank, a = 0;
    uint32 masks[NUMBER_OF_PIO_BANKS];

    for(bank = 0; bank < NUMBER_OF_PIO_BANKS; bank++)
    {
        masks[bank] = 0;
    }
    for(bank = 0; ; )
    {
        uint32 global_offset =
                   ((DEFAULT_LED0_PIO + (led - (uint32)LED_0)) %
                     NUM_LED_CTRL_INST) +
                   (NUM_LED_CTRL_INST * a++);
        if (global_offset >= ((bank + 1) * PIOS_PER_BANK))
        {
            bank++;
            if (bank >= NUMBER_OF_PIO_BANKS)
            {
                break;
            }
        }
        masks[bank] += 1 << (global_offset - (bank * PIOS_PER_BANK));
    }

    ATOMIC_BLOCK_START {
        for(bank = 0; bank < NUMBER_OF_PIO_BANKS; bank++)
        {
            if (masks[bank] & led_cfg_pio_mux_mask[bank])
            {
                ret = TRUE;
            }
        }
    } ATOMIC_BLOCK_END;

    return ret;
}

void led_cfg_log_config(led_id led, client_led_config *led_conf)
{
    led_config *cfg = &led_conf[led].driver_config;
    L4_DBG_MSG1("LED Index: %d", led);
    L4_DBG_MSG1("    Duty Cycle: %d", led_conf[led].duty_cycle);
    L4_DBG_MSG1("    Low Duty Cycle: %d", led_conf[led].low_duty_cycle);
    L4_DBG_MSG1("    Invert: %d", led_conf[led].invert);
    L4_DBG_MSG1("    initial_state: %d", cfg->initial_state);
    L4_DBG_MSG1("    pin_conf: %d", cfg->pin_conf);
    L4_DBG_MSG1("    single_shot_mode: %d", cfg->single_shot_mode);
    L4_DBG_MSG1("    logarithmic_mode: %d", cfg->logarithmic_mode);
    L4_DBG_MSG1("    logarithmic_offset: %d", cfg->logarithmic_offset);
    L4_DBG_MSG1("    min.lo: %d", cfg->min.lo);
    L4_DBG_MSG1("    min.hi: %d", cfg->min.hi);
    L4_DBG_MSG1("    max.lo: %d", cfg->max.lo);
    L4_DBG_MSG1("    max.hi: %d", cfg->max.hi);
    L4_DBG_MSG1("    hold.lo: %d", cfg->hold.lo);
    L4_DBG_MSG1("    hold.hi: %d", cfg->hold.hi);
    L4_DBG_MSG1("    ramp_current.lo: %d", cfg->ramp_current.lo);
    L4_DBG_MSG1("    ramp_current.hi: %d", cfg->ramp_current.hi);
    L4_DBG_MSG1("    single_shot.lo: %d", cfg->single_shot.lo);
    L4_DBG_MSG1("    single_shot.hi: %d", cfg->single_shot.hi);
    L4_DBG_MSG1("    ramp: %d", cfg->ramp);
    L4_DBG_MSG1("    counthold_value: %d", cfg->counthold_value);
}


#endif /* DESKTOP_TEST_BUILD */


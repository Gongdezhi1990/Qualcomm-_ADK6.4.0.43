/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implementation of LED hardware control.
 */

#include <csrtypes.h>
#include <led.h>
#include <pio.h>
#include "led_cfg/led_cfg.h"
#include "hydra_log/hydra_log.h"

#ifndef DESKTOP_TEST_BUILD


/** PIO index of the first LED */
#define PIO_CFG_FIRST_LED_PIO_IDX DEFAULT_LED0_PIO
/** PIO index of the last LED */
#define PIO_CFG_LAST_LED_PIO_IDX (PIO_CFG_FIRST_LED_PIO_IDX + NUM_OF_LED_PADS)

static led_instance_mask initialised_leds = 0;
static led_instance_mask autoconf_pios_leds = 0;

bool LedConfigure(led_id led, led_config_key key, uint16 value)
{
    uint32 mask, status;
    uint16 pio = NUMBER_OF_PIOS, bank;
    bool pin_acquired = FALSE;
    bool ret;

    if (led >= NUM_OF_LED_CTRLS)
    {
        L2_DBG_MSG1("LedConfigure: Invalid LED ID %d", led);
        return FALSE;
    }

    /* In the future we'd like the app to configure these. Until then we
       default LED pads to the LED function to maintain backwards
       compatibility. */
    if ((!led_cfg_led_has_pio(led)) &&
        (PIO_CFG_FIRST_LED_PIO_IDX + (led - (uint32)LED_0) <=
         PIO_CFG_LAST_LED_PIO_IDX))
    {
        pio = (uint16)(PIO_CFG_FIRST_LED_PIO_IDX + (led - (uint32)LED_0));
        bank = (uint16)(pio / 32);
        mask = 1 << (pio % 32);
        
        status = PioSetMapPins32Bank(bank, mask, 0);
        L3_DBG_MSG1("LED defaults: PioSetMapPins32Bank status 0x%08x",
                    status);
        if (status)
        {
            return FALSE;
        }
        status = PioSetFunction(pio, LED);
        L3_DBG_MSG2("LED defaults: PioSetFunction to LED for pio %d, "
                    "status %d", pio, status);
        if (status == FALSE)
        {
            return FALSE;
        }
        pin_acquired = TRUE;
        autoconf_pios_leds |= (led_instance_mask)(1 << led);
    }

    if (0 == (initialised_leds & (1 << led)))
    {
        if (led_cfg_initialise_led(led))
        {
            initialised_leds |= (led_instance_mask)(1 << led);
        }
        else
        {
            if (pin_acquired)
            {
                /* release newly acquired PIO */
                status = PioSetFunction(pio, OTHER);
                L3_DBG_MSG2("LED defaults: PioSetFunction to OTHER for pio %d, "
                            "status %d", pio, status);
                autoconf_pios_leds = (led_instance_mask)(autoconf_pios_leds &
                                                         (~(1 << led)));
            }
            return FALSE;
        }
    }

    ret = led_cfg_set_config(led, key, value);

    if (ret && (key == LED_ENABLE) && (!value) &&
        (autoconf_pios_leds & (1 << led)))
    {
        /* If the app disables the LED controller and it's LED pad was
           autoconfigured then make sure the LED pad is not left as digital
           input (the Curator does that). */
        pio = (uint16)(PIO_CFG_FIRST_LED_PIO_IDX + (led - (uint32)LED_0));
        status = PioSetFunction(pio, OTHER);
        L3_DBG_MSG2("LED defaults: controller disable, PioSetFunction to OTHER "
                    "for pio %d, status %d", pio, status);
        autoconf_pios_leds = (led_instance_mask)(autoconf_pios_leds &
                                                 (~(1 << led)));
    }

    return ret;
}


#endif /* DESKTOP_TEST_BUILD */

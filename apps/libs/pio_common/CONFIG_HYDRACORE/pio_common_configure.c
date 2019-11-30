/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    pio_common_configure.c

DESCRIPTION
    Common PIO operations used by libraries and applications
    Enable pins for special functions

NOTES

*/

/****************************************************************************
    Header files
*/
#include <pio.h>
#include <adc.h>
#include <led.h>
#include <print.h>
#include <panic.h>
#include <vmtypes.h>
#include <vmal.h>

#include "pio_common.h"
#include "pio_common_private.h"

#define ACTIVE_LOW_LEVEL 0

/****************************************************************************
    Public functions
*/
void PioCommonConfigureSpdifInput(uint8 spdif_input_pin)
{
    UNUSED(spdif_input_pin);
}

void PioCommonConfigureSpdifOutput(uint8 spdif_output_pin)
{
    UNUSED(spdif_output_pin);
}

void PioCommonSetStrongBias(pio_common_allbits mask, pio_common_allbits bits)
{
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    for (bank = 0; bank < number_of_banks; bank ++)
    {   /* only call bias trap if there are PIO bits masked */
        if(bits.mask[bank])
            PioSetStrongBias32Bank(bank, mask.mask[bank], bits.mask[bank]);
    }
}

static void configureActiveLowInterruptPin(uint16 pin)
{
    uint16 bank =              (pin / NUMBER_OF_PIOS_PER_BANK);
    uint32 mask = (uint32)1 << (pin % NUMBER_OF_PIOS_PER_BANK);

    PioSetDeepSleepEitherLevelBank(bank, mask, mask);
    PioSetWakeupStateBank(bank, mask, ACTIVE_LOW_LEVEL);
}

bool pioCommonSetConvertedFunction(uint16 pin, pio_common_pin_function_id function)
{
    if (pin < (MAX_NUMBER_OF_PIO_BANKS * NUMBER_OF_PIOS_PER_BANK))
    {
        switch (function)
        {
            /* PIO functions provided through pio.h. */
            case pin_function_uart_rx:
                return PioSetFunction(pin, UART_RX);
            case pin_function_uart_tx:
                return PioSetFunction(pin, UART_TX);
            case pin_function_uart_rts:
                return PioSetFunction(pin, UART_RTS);
            case pin_function_uart_cts:
                return PioSetFunction(pin, UART_CTS);
            case pin_function_pcm_in:
                return PioSetFunction(pin, PCM_IN);
            case pin_function_pcm_out:
                return PioSetFunction(pin, PCM_OUT);
            case pin_function_pcm_sync:
                return PioSetFunction(pin, PCM_SYNC);
            case pin_function_pcm_clk:
                return PioSetFunction(pin, PCM_CLK);
            case pin_function_sqif:
                return PioSetFunction(pin, SQIF);
            case pin_function_led:
                return PioSetFunction(pin, LED);
            case pin_function_lcd_segment:
                return PioSetFunction(pin, LCD_SEGMENT);
            case pin_function_lcd_common:
                return PioSetFunction(pin, LCD_COMMON);
            case pin_function_pio:
                return PioSetFunction(pin, PIO);
            case pin_function_spdif_rx:
                return PioSetFunction(pin, SPDIF_RX);
            case pin_function_spdif_tx:
                return PioSetFunction(pin, SPDIF_TX);
            case pin_function_bitserial_0_clock_in:
                return PioSetFunction(pin, BITSERIAL_0_CLOCK_IN);
            case pin_function_bitserial_0_clock_out:
                return PioSetFunction(pin, BITSERIAL_0_CLOCK_OUT);
            case pin_function_bitserial_0_data_in:
                return PioSetFunction(pin, BITSERIAL_0_DATA_IN);
            case pin_function_bitserial_0_data_out:
                return PioSetFunction(pin, BITSERIAL_0_DATA_OUT);
            case pin_function_bitserial_0_sel_in:
                return PioSetFunction(pin, BITSERIAL_0_SEL_IN);
            case pin_function_bitserial_0_sel_out:
                return PioSetFunction(pin, BITSERIAL_0_SEL_OUT);
            case pin_function_bitserial_1_clock_in:
                return PioSetFunction(pin, BITSERIAL_1_CLOCK_IN);
            case pin_function_bitserial_1_clock_out:
                return PioSetFunction(pin, BITSERIAL_1_CLOCK_OUT);
            case pin_function_bitserial_1_data_in:
                return PioSetFunction(pin, BITSERIAL_1_DATA_IN);
            case pin_function_bitserial_1_data_out:
                return PioSetFunction(pin, BITSERIAL_1_CLOCK_OUT);
            case pin_function_bitserial_1_sel_in:
                return PioSetFunction(pin, BITSERIAL_1_SEL_IN);
            case pin_function_bitserial_1_sel_out:
                return PioSetFunction(pin, BITSERIAL_1_CLOCK_OUT);
            case pin_function_analogue:
                return PioSetFunction(pin, ANALOGUE);
            case pin_function_other:
                return PioSetFunction(pin, OTHER);
            /* Other PIO functions defined by ADK config rather than through pio.h. */
            case pin_function_interrupt_active_low:
                configureActiveLowInterruptPin(pin);
                return TRUE;
            default:
                return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

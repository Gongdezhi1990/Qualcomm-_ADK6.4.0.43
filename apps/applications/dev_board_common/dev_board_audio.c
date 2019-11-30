/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2018

@file

    Functions that provide additional control of the audio hardware
    on the development board.

*/

#include "dev_board_audio.h"

#define SW_PIO_MASK(bank,pio)   (1ul << ((pio)-((bank)*PIOS_PER_BANK)))
#define SW_PIO_BANK(pio)        ((pio)/PIOS_PER_BANK)

#define AMP_ENABLE_PIO 32

/* @brief Enable the output amplifier.
 * On some base- and daughter-board combinations, the base-board output amplifier is
 * disabled by default owing to the MUTE line being held high by the daughter-board.
 * Depending on the type of the device on the daughter-board, MUTE on the SODIMM connector
 * may be connected to either:
 *  - Ground, in which case the amplifier will be enabled unless overridden by a jumper
 *    setting on the base board;
 *  - A device PIO line, in which case the power-on state of that PIO line determines
 *    whether or not the amplifier is enabled or disabled.
 * For the latter case, this routine provides a simple example of how to drive MUTE low via
 * the PIO output - define ENABLE_EXAMPLE_CODE to include it in your build.
 * At the time of writing, PIO-32 was most commonly used to drive the MUTE signal - please
 * check your device documentation and development board schematics to confirm this for
 * your hardware.
 */
static void enable_audio_amplifier(void)
{
#if ENABLE_EXAMPLE_CODE
    const uint16 amp_enable_bank = SW_PIO_BANK(AMP_ENABLE_PIO);
    const uint32 amp_enable_pin = SW_PIO_MASK(amp_enable_bank,AMP_ENABLE_PIO);

    PioSetMapPins32Bank(amp_enable_bank, amp_enable_pin, amp_enable_pin);
    PioSetDir32Bank(amp_enable_bank, amp_enable_pin, amp_enable_pin);
    PioSet32Bank(amp_enable_bank, amp_enable_pin, 0);
#endif
}

/* @brief Perform any necessary audio hardware initialisation
 *
 */
void dev_board_audio_initialise(void)
{
	enable_audio_amplifier();
}


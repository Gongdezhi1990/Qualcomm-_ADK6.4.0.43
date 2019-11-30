/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * hardware abstraction layer for auxiliary input and output
 *
 * Functions to control and monitor auxiliary IO pins.
 * Up to 32 PIOs (programmable input/outputs) can be controlled, although
 * they may not all be accessible on a particular hardware platform.
 *
 * See pio.h for a detailed explanation of the PIO hardware on a Hydra chip.
 */

#ifndef HALAUXIO_H
#define HALAUXIO_H

#include "hydra/hydra_types.h"
#include "int/int.h"
#define IO_DEFS_MODULE_APPS_SYS_PIO
#define IO_DEFS_MODULE_KALIMBA_PIO_INT
#define IO_DEFS_MODULE_CHIP
#define IO_DEFS_MODULE_APPS_SYS_SYS
#include "io/io.h"
#include "hal/hal_macros.h"
#include "utils/utils_bit.h"


/** Constant define for the number of PIO banks */
#define NUMBER_OF_PIO_BANKS ((NUMBER_OF_PIOS+PIOS_PER_BANK-1) / (PIOS_PER_BANK))

/** Constant definition for indicating all PIOs. */
#define ALL_PIOS 0xFFFFFFFFUL 

/** Helper macro for generating register names by combining 3 elements */
#define PIO_CONC3(a,b,c) PIO_CONC2(PIO_CONC2(a,b),c) 
/** Helper macro for generating register names by combining 2 elements */
#define PIO_CONC2(a,b) PIO_CONC2_(a,b)
/** Second level for helper macro */
#define PIO_CONC2_(a,b) a##b
/** Helper macro for calculating the PIO bank */
#define PBANK(pio) ((uint16)((pio) / PIOS_PER_BANK))
/** Helper macro for calculating the PIO offset within the bank */
#define POFF(pio) ((pio) % PIOS_PER_BANK)
/** Helper macro for calculating the PIO offset mask */
#define POFFM(pio) (1U << POFF(pio))
/** Helper macro for calculating the PIO index having the bank and the offset */
#define PIDX(bank, offset) ((uint8)(((bank) * PIOS_PER_BANK) + (offset)))

/** PIO interrupt abstraction */
#define PIO_INT_SOURCE(n) PIO_CONC2(INT_SOURCE_PIO_INT_EVENT_, n)

/** Type definition for the PIO mux selection options */
typedef enum pio_select
{
    PIO_SELECT_P0 = 0x0,
    PIO_SELECT_P1 = 0x1
} pio_select;

/** Type definition for the PIO masks used in the API */
typedef uint32 pio_size_bits;

/**
 * Set the ownership within the Apps block for "pio" to "mode".
 * \param pio PIO number.
 * \param owner PIO owner.  
 */
/* extern void hal_set_pio_select(uint16 pio_number, pio_select owner); */
#define hal_set_pio_select(pio, owner) \
    hal_set_reg_apps_sys_pio_mux(PBANK(pio), \
                                 BIT_WRITE_MASK( \
                                     hal_get_reg_apps_sys_pio_mux(PBANK(pio)),\
                                     POFFM(pio), \
                                     (owner)?POFFM(pio):0))
/**
 * Get the ownership for an auxiliary pin within the Apps block.
 * \param pio PIO number.
 * \returns Owner of the PIO.
 */
/* extern pio_select hal_get_pio_select(uint16 pio_number); */
#define hal_get_pio_select(pio) \
        (hal_get_reg_apps_sys_pio_mux(PBANK(pio)) >> POFF(pio))

/**
 * Set the ownership within the Apps block for several PIOs indicated by a
 * mask.
 * \param bank Bank index.
 * \param owners Ownership mask.
 */
/* extern void hal_set_pio_select_mask(uint16 bank, pio_size_bits owners); */
#define hal_set_pio_select_mask(bank, owners) \
    hal_set_reg_apps_sys_pio_mux(bank, owners)

/**
 * Get the ownership for PIOs within the Apps block.
 * \param bank Bank index.
 * \returns Ownership mask.
 */
/* extern pio_size_bits hal_get_pio_select(uint16 bank); */
#define hal_get_pio_select_mask(bank) \
    hal_get_reg_apps_sys_pio_mux(bank)

/**
 * Set drive enable for several PIOs.
 * \param bank Bank index.
 * \param mask Mask indicating the direction (1 for output, 0 for input).
 */
/* extern void hal_set_pio_directions(uint16 bank, pio_size_bits dir_mask); */
#define hal_set_pio_directions(bank, mask) \
        hal_set_reg_apps_sys_pio_drive_enable(bank, mask)

/**
 * Get drive direction for several PIOs.
 * \param bank Bank index.
 * \returns Direction mask (1 for output, 0 for input).
 */
/* extern pio_size_bits hal_get_pio_directions(uint16 bank); */
#define hal_get_pio_directions(bank) \
        hal_get_reg_apps_sys_pio_drive_enable(bank)

/**
 * Set the direction of the given PIO.
 * \param pio PIO number.
 * \param value Direction value as a boolean, True for output and False for
 * input.
 */
/* extern void hal_set_pio_direction(uint16 pio_number, bool value); */
#define hal_set_pio_direction(pio, value) \
    hal_set_pio_directions(PBANK(pio), \
                           BIT_WRITE_MASK(hal_get_pio_directions(PBANK(pio)), \
                                          POFFM(pio), \
                                          (value)?POFFM(pio):0))

/**
 * Sets the values of all the PIOs to "value". For pins configured as inputs
 * this controls whether a pull-down or a pull-up is applied.
 * \param bank Bank index.
 * \param value_mask Mask indicating the output values or pull direction.
 */
/* extern void hal_set_output_pios(uint16 bank, pio_size_bits value_mask); */
#define hal_set_output_pios(bank, value_mask) \
        hal_set_reg_apps_sys_pio_drive(bank, value_mask)

/**
 * Returns a bitmask indicating how PIOs are being driven. For pins configured
 * as inputs this indicates whether a pull-down or a pull-up is applied.
 * \param bank Bank index.
 * \returns Mask of output values or pull directions.
 */
/* extern pio_size_bits hal_get_output_pios(uint16 bank); */
#define hal_get_output_pios(bank) \
        hal_get_reg_apps_sys_pio_drive(bank)

/**
 * Set value of selected auxiliary output pin
 * \param pio PIO number.
 * \param value Output value as a boolean.
 */
/* extern void hal_set_output_pio(uint16 pio, bool value); */
#define hal_set_output_pio(pio, value) \
    hal_set_output_pios(PBANK(pio), \
                        BIT_WRITE_MASK(hal_get_output_pios(PBANK(pio)), \
                                       POFFM(pio), \
                                       (value)?POFFM(pio):0))

/**
 * Get value of selected PIO.
 * \param pio Pio number.
 * \returns Output value as a boolean.
 */
/* extern bool hal_get_input_pio(uint16 pio_number); */
#define hal_get_input_pio(pio) \
        ((hal_get_reg_apps_sys_pio_status(PBANK(pio)) >> POFF(pio)) & 1U)

/**
 * Get value of several PIOs.
 * \param bank Bank index.
 * \returns Mask of input values.
 */
/* extern pio_size_bits hal_get_input_pios(uint16 bank); */
#define hal_get_input_pios(bank) \
        hal_get_reg_apps_sys_pio_status(bank)

/**
 * Set the mask indicating on which PIOs a rising edge will trigger the
 * INT_SOURCE_PIOn_EVENT if enabled.
 * \param int_no Interrupt number.
 * \param bank Bank index.
 * \param mask Mask indicating the enabled rising edge triggers.
 */
/* extern void hal_set_pio_rising_int_triggers(uint16 int_no, 
                                               uint16 bank, 
                                               pio_size_bits mask); */
#define hal_set_pio_rising_int_triggers(int_no, bank, mask) \
    PIO_CONC3(hal_set_reg_kalimba_pio_int_pio, int_no, \
              _event_rising_enable)(bank, mask)

/**
 * Returns the mask indicating on which PIOs a rising edge will trigger the
 * INT_SOURCE_PIOn_EVENT if enabled.
 * \param int_no Interrupt number.
 * \param bank Bank index.
 * \returns Mask indicating the enabled rising edge triggers.
 */
/* extern pio_size_bits hal_get_pio_rising_int_triggers(uint16 int_no, 
                                                        uint16 bank); */
#define hal_get_pio_rising_int_triggers(int_no, bank) \
    PIO_CONC3(hal_get_reg_kalimba_pio_int_pio, int_no, \
              _event_rising_enable)(bank)

/**
 * Set the mask indicating on which PIOs a falling edge will trigger the
 * INT_SOURCE_PIOn_EVENT if enabled.
 * \param int_no Interrupt number.
 * \param bank Bank index.
 * \param mask Mask indicating the enabled falling edge triggers.
 */
/* extern void hal_set_pio_falling_int_triggers(uint16 int_no, 
                                                uint16 bank, 
                                                pio_size_bits mask); */
#define hal_set_pio_falling_int_triggers(int_no, bank, mask) \
    PIO_CONC3(hal_set_reg_kalimba_pio_int_pio, int_no, \
              _event_falling_enable)(bank, mask)

/**
 * Returns the mask indicating on which PIOs a falling edge will trigger the
 * INT_SOURCE_PIOn_EVENT if enabled.
 * \param int_no Interrupt number.
 * \param bank Bank index.
 * \returns Mask indicating the enabled falling edge triggers.
 */
/* extern pio_size_bits hal_get_pio_falling_int_triggers(uint16 int_no, 
                                                         uint16 bank); */
#define hal_get_pio_falling_int_triggers(int_no, bank) \
    PIO_CONC3(hal_get_reg_kalimba_pio_int_pio, int_no, \
              _event_falling_enable)(bank)

/**
 * Returns the mask indicating on which PIOs have triggered an interrupt event.
 * \param int_no Interrupt number.
 * \param bank Bank index.
 * \returns Mask indicating the triggered interrupts.
 */
/* extern pio_size_bits hal_get_pio_event_cause(uint16 int_no, uint16 bank); */
#define hal_get_pio_event_cause(int_no, bank) \
    PIO_CONC3(hal_get_reg_kalimba_pio_int_pio, int_no, _event_cause)(bank)

/**
 * Clears the events which raised the interrupt.
 * \param int_no Interrupt number.
 * \param bank Bank index.
 * \param mask Mask indicating the events to clear.
 */
/* extern void hal_clear_pio_event_cause(uint16 int_no, 
                                         uint16 bank, 
                                         pio_size_bits mask); */
#define hal_clear_pio_event_cause(int_no, bank, mask) \
    PIO_CONC3(hal_set_reg_kalimba_pio_int_pio, int_no, \
              _event_cause_clear_data)(bank, mask)


#endif /* HALAUXIO_H */

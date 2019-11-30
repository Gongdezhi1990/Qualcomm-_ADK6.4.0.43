/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Internal header for the PIO interrupt gneeration module.
 */

#ifndef PIOINT_PRIVATE_H
#define PIOINT_PRIVATE_H

#include "pioint/pioint.h"
#include "pmalloc/pmalloc.h"
#include "hal/halauxio.h"
#include "int/int.h"
#include "assert.h"


/** Helper macro for the interrupt source. */
#define PIOINT_SOURCE PIO_INT_SOURCE(PIOINT_NO)

/** Helper macro for the interrupt priority */
#define PIOINT_PRIORITY INT_LEVEL_FG

/**
 * The PIO interrupt configuration structure. This defines the structure of a
 * database entry.
 */
typedef struct pioint_config_entry_s
{
    /** Links to next list element. */
    struct pioint_config_entry_s *next;
    /** Bank of PIOs which trigger the interrupt. */
    uint16 bank;
    /** Mask of PIOs within bank. */
    pio_size_bits mask;
    /** Handler to call on level changes. */
    void (*handler)(void);
}pioint_config_entry;

/** Abstraction macro for detaching from the PIO interrupt number. */
#define pio_set_rising_int_triggers(bank, mask) \
        hal_set_pio_rising_int_triggers(PIOINT_NO, bank, mask)
/** Abstraction macro for detaching from the PIO interrupt number. */
#define pio_get_rising_int_triggers(bank) \
        hal_get_pio_rising_int_triggers(PIOINT_NO, bank)
/** Abstraction macro for detaching from the PIO interrupt number. */
#define pio_set_falling_int_triggers(bank, mask) \
        hal_set_pio_falling_int_triggers(PIOINT_NO, bank, mask)
/** Abstraction macro for detaching from the PIO interrupt number. */
#define pio_get_falling_int_triggers(bank) \
        hal_get_pio_falling_int_triggers(PIOINT_NO, bank)
/** Abstraction macro for detaching from the PIO interrupt number. */
#define pio_get_event_cause(bank) \
        hal_get_pio_event_cause(PIOINT_NO, bank)
/** Abstraction macro for detaching from the PIO interrupt number. */
#define pio_clear_event_cause(bank, mask) \
        hal_clear_pio_event_cause(PIOINT_NO, bank, mask)


#endif /* PIOINT_PRIVATE_H */


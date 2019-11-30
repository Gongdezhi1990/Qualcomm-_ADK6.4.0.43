/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef PIOINT_H
#define PIOINT_H

#include "hydra/hydra_macros.h"
#include "hydra/hydra_types.h"
#include "hal/halauxio.h"


/** Compile time definition for the interrupt used by pioint. */
#define PIOINT_NO 2

/**
 * Initialise the pioint subsystem.
 */
extern void pioint_init(void);

/**
 * Configure interrupts on PIO pins.
 * \param bank PIO bank index.
 * \param mask PIOs allowed trigger the interrupt. These are indicated by a 
 * mask.
 * \param handler Function pointer. This will be called in interrupt context
 * when an event is detected.
 */
extern void pioint_configure(uint16 bank, pio_size_bits mask, void (*handler)(void));


#endif /* PIOINT_H */


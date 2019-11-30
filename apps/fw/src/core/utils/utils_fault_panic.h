/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Utilities for fault and panic.
 */

#ifndef UTILS_FAULT_PANIC_H
#define UTILS_FAULT_PANIC_H

#include "hydra/hydra_types.h"

/**
 * We have to divide a DIATRIBE_TYPE into an array of 16-bit integers. In
 * principle, uint16 has the semantics "uint_least16_t" so may actually have
 * more than 16 bits (e.g. on a 24-bit platform).  This means that the number
 * of 16-bit "pieces" in a DIATRIBE_TYPE is not simply
 * sizeof(DIATRIBE_TYPE)/sizeof(uint16). This calculates the bit size of the
 * diatribe type and is used to calculate the length of a uint16 array that can
 * hold the diatribe.
 */
#define DIATRIBE_TYPE_BIT ((sizeof(DIATRIBE_TYPE)/sizeof(char))*CHAR_BIT)

/**
 * Calculates the length of a uint16 array that can hold the diatribe rounding up.
 */
#define ARRAY_LENGTH ((DIATRIBE_TYPE_BIT+15)/16)

/**
 * Converts a fault or panic diatribe to a uint16 array.
 * The buffer is provided by the caller.
 * \param arg Fault or panic diatribe.
 * \param array16 Pointer to buffer to store the uint16 array.
 */
void diatribe_to_array(DIATRIBE_TYPE arg, uint16 *array16);

#endif /* UTILS_FAULT_PANIC_H */

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * memory map
 */

#ifndef MEMMAP_H
#define MEMMAP_H

#include <limits.h>
#include "hydra/hydra_types.h"

/** Scale factor to convert Xap2 addresses appropriately for this target */
#if CHAR_BIT == 8
#define MEMMAP_ADDRESS_SHIFT (0)
#elif CHAR_BIT == 16
#define MEMMAP_ADDRESS_SHIFT (1)
#else /* ! CHAR_BIT == 16 */
#error "Unsupported platform ('char' is not 8 or 16 bits)"
#endif /* ! CHAR_BIT == 16 */
#define MEMMAP_ADDRESS(a) ((a) >> MEMMAP_ADDRESS_SHIFT)

/** Many standard (or pseudo standard) C functions take sizes that are
 * in multiples of 'chars' (most importantly this includes sizeof,
 * memcpy and ?malloc).  The token and mmu functions take sizes that
 * are in multiples of octets.  On the XAP (and possibly other
 * architectures) these are not the same, so a way of converting
 * between the two is needed.
 */
#define CHARS_TO_OCTETS(n)                      \
    ((n) * (1 << MEMMAP_ADDRESS_SHIFT))
#define OCTETS_TO_CHARS(n)                                              \
    (((n) + ((1 << MEMMAP_ADDRESS_SHIFT) - 1)) / (1 << MEMMAP_ADDRESS_SHIFT))

/** Step size for all sliding windows (in bytes) */
#define BANK_SHIFT_BYTES (12)
#define BANK_SIZE_BYTES (1 << BANK_SHIFT_BYTES)

/** Step size for all sliding windows (in addressable location size) */
#define BANK_SHIFT (BANK_SHIFT_BYTES - MEMMAP_ADDRESS_SHIFT)
#define BANK_SIZE (1 << BANK_SHIFT)


/* The following are normally defined in the firmware mem config file,
 * but the linux tests don't use these config files so we provide default
 * values here. This is gross and should be fixed.
 * TODO */

/**
 * Size of MMU Processor Port Window
 */
#ifndef MMU_PROC_BUF_WIN_SIZE
#define MMU_PROC_BUF_WIN_SIZE 0x2000
#endif /* MMU_PROC_BUF_WIN_SIZE */

/**
 * log_2 of the MMU buffer page size in bytes (octets)
 */
#ifndef MMU_LOG_PAGE_BYTES
#define MMU_LOG_PAGE_BYTES      (7)
#endif /* MMU_LOG_PAGE_BYTES */

/**
 * MMU buffer page size (in bytes)
 */
#ifndef MMU_PAGE_BYTES
#define MMU_PAGE_BYTES (1U << MMU_LOG_PAGE_BYTES)
#endif /* MMU_PAGE_BYTES */

#define MMU_PAGE_WORDS  (MMU_PAGE_BYTES << 1)

#ifdef HAVE_32BIT_DATA_WIDTH
/** Kalimba 32-bit addresses memory in bytes */
#define MMU_PAGE_ADDRESSES      MMU_PAGE_BYTES
#else
/** XAPs address memory in words */
#define MMU_PAGE_ADDRESSES      MMU_PAGE_WORDS
#endif /* HAVE_32BIT_DATA_WIDTH */

#endif /* MEMMAP_H */

/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup util Kalimba Utils
 * \file util.h
 * \ingroup util
 *
 * Utility macros for Kalimba. <br>
 */

#ifndef UTIL_H
#define UTIL_H

#include <limits.h>

/****************************************************************************
Public Macro Declarations
*/

/* Inlined minimum, maximum and absolute (may evaluate expressions twice) */
#ifndef MAX
#define MAX(a,b)        (((a) < (b)) ? (b) : (a))
#endif

#ifndef MIN
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)          (((a) < 0) ? -(a) : (a))
#endif

/**
 * Compile time assertions with some (hopefully) useful messages.
 * Unlike gcc the Kalimba compiler seems to be less verbose when
 * it comes to reporting a negative array size error. However, the
 * error when traced back to the source should provide a reasonable
 * clue to help fix the compile time assertion.
 *
 * NOTE: msg cannot contain spaces. Use underscore.
 * Use like: STATIC_ASSERT(CONFIG_BITS > 1, config_bits_should_be_greater_than_1)
 */

#define STATIC_ASSERT(cond, msg) typedef char static_assert_ ## msg ## __COUNTER__ [2*!!((cond)) - 1]

/**
 * Finds the number of chars, given the number of octets.
 * Rounded up if the number of octets do not fit exactly in
 * a char.
 */
#define OCTETS_TO_CHARS(octets) (((octets * 8) + (CHAR_BIT - 8))/ CHAR_BIT)

/**
 * Rounds the given number to the nearest multiple of native word size
 */
#define ROUND_UP_TO_WHOLE_WORDS(num) (((num)+(ADDR_PER_WORD-1))&(~(ADDR_PER_WORD-1)))

#define ROUND_UP_TO_WORD_ALIGNED_ADDR(addr) ((unsigned int*) ROUND_UP_TO_WHOLE_WORDS(((uintptr_t)(addr))))

/*
 * These macros can be used to ensure that a type that is not explicitly
 * referenced is not elided from the DWARF. They achieve this by declaring a
 * global of that type but putting it in a special section that contains only
 * these unreferenced symbols. (kld appears to elide a section that is wholly
 * unreferenced, whereas an individual unreferenced symbol in a section
 * also containing referenced symbols is not eliminated.)
 *
 * These macros currently do nothing for tool chains other than kcc and
 * architectures other than KAL_ARCH4.
 */
#if !defined(__GNUC__)
#define UNREFERENCED_ATTR _Pragma("datasection DEBUG_TRACE_STRINGS")
#define PRESERVE_TYPE_FOR_DEBUGGING_(typename, tag) \
    /*lint --e{19} --e{129} Useless declarations */\
    UNREFERENCED_ATTR tag typename preserve_##tag##_##typename##_for_debugging;
#else
#define PRESERVE_TYPE_FOR_DEBUGGING_(typename, tag)
#endif /* __KALIMBA__ */

#define PRESERVE_ENUM_FOR_DEBUGGING(typename) \
        PRESERVE_TYPE_FOR_DEBUGGING_(typename, enum)

#define PRESERVE_STRUCT_FOR_DEBUGGING(typename) \
        PRESERVE_TYPE_FOR_DEBUGGING_(typename, struct)

#define PRESERVE_TYPE_FOR_DEBUGGING(typename) \
        PRESERVE_TYPE_FOR_DEBUGGING_(typename,)

#endif /* UTIL_H */

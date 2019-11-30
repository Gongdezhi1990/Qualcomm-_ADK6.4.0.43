/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 *
 ************************************************************************//**
 * \file malloc_pm.h
 * Interface for memory allocation/free functions
 *
 * MODULE : malloc_pm
 *
 * \ingroup malloc_pm
 *
 ****************************************************************************/

#if !defined(MALLOC_PM_H)
#define MALLOC_PM_H

/****************************************************************************
Include Files
*/

/****************************************************************************
Public Macro Declarations
*/

/* Attribute macros that can be used to put specific items in Kalimba PM
 * Defined to nothing for GCC builds
 */

#if !defined(__GNUC__)
#define PM_MALLOC _Pragma("codesection CODE_HEAP")
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
#define PM_MALLOC_P1 _Pragma("codesection CODE_HEAP_P1")
#endif
#else
#define PM_MALLOC
#ifdef INSTALL_DUAL_CORE_SUPPORT
#define PM_MALLOC_P1
#endif
#endif

#define MALLOC_PM_PREFERENCE_CORE_0 0
#define MALLOC_PM_PREFERENCE_CORE_1 1

/**
 * Use PM memory heap based malloc and free functions, defined in malloc_pm.c
 *
 * xpmalloc_pm: a version of malloc similar to standard C library, that does not panic if it has not memory but instead returns a Null pointer.
 * pmalloc_pm: a version of malloc that panics if it has no memory.
 *
 * free_pm: a version of free (does not panic if passed a Null pointer).
 *
 * Note init_malloc_pm must be called before any version of malloc_pm or free_pm is called
 */


/****************************************************************************
Public Type Declarations
*/

/* Void function pointer type.
 *
 * This is the underlying type for the data handled by the malloc_pm module;
 * the allocation addresses are code pointers.
 * This type is not intended for general use away from this module.
 */
typedef void (*void_func_ptr)(void);

/****************************************************************************
Global Variable Definitions
*/

/****************************************************************************
Public Function Prototypes
*/
extern void_func_ptr xpmalloc_pm(unsigned int numBytes, unsigned int preference_core);
extern void_func_ptr pmalloc_pm(unsigned int numBytes, unsigned int preference_core);
extern void free_pm(void_func_ptr pMemory);
extern int sizeof_pm(void_func_ptr pMemory);
extern void init_malloc_pm(unsigned int reservedSize);

#endif /* MALLOC_PM_H */

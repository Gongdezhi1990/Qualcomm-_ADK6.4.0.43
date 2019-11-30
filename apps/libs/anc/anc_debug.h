/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_debug.h

DESCRIPTION
    Definition of macros used to help debug the ANC VM library
*/

#ifndef ANC_DEBUG_H_
#define ANC_DEBUG_H_

#include <panic.h>
#include <assert.h>

/* Macro used to generate debug version of this library */
#ifdef ANC_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <print.h>
#include <stdio.h>

#define ANC_DEBUG_RAW(x) PRINT(x)
#define ANC_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define ANC_DEBUG_PANIC(x) {ANC_DEBUG_INFO(x); ANC_PANIC();}
#define ANC_DEBUG_ASSERT(x) ANC_ASSERT(x)

#else /* ANC_DEBUG_LIB */

#define ANC_DEBUG_RAW(x)
#define ANC_DEBUG_INFO(x)
#define ANC_DEBUG_PANIC(x)
#define ANC_DEBUG_ASSERT(x)

#endif /* ANC_DEBUG_LIB */

/* Macros defined in all variants of the ANC VM library */
#define ANC_PANIC() Panic()
#define ANC_ASSERT(x) {if (!(x)) Panic();}

#endif

/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_debug.h

DESCRIPTION
    AMA debug macros
*/

#ifndef AMA_DEBUG_H_
#define AMA_DEBUG_H_

#include <panic.h>
#include <print.h>

/* Macro used to generate debug version of this library */
#ifdef DEBUG_AMA_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif


#define AMA_DEBUG(x) {PRINT(x);}
#define AMA_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define AMA_DEBUG_PANIC(x) {AMA_DEBUG_INFO(x); Panic();}

#else /*DEBUG_AMA_LIB */

#define AMA_DEBUG(x)
#define AMA_DEBUG_PANIC(x)

#endif /* DEBUG_AMA_LIB */

#endif /* AMA_DEBUG_H_ */


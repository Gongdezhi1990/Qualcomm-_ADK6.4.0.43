/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_debug.h
DESCRIPTION
    Controls debug output

*/
#ifndef __IAP2_DEBUG_H__
#define __IAP2_DEBUG_H__

#ifdef IAP2_DEBUG_LIB
#include <stdio.h>

/* Common debug print macro */
#define PRINT(x) do { printf("%s[%d]: ", __FILE__, __LINE__); printf x; } while(0)

/* Debug flags */
#define IAP2_DEBUG_GENERAL
#define IAP2_DEBUG_CPx
#define IAP2_DEBUG_INITx
#define IAP2_DEBUG_CONNECT
#define IAP2_DEBUG_LINKx
#define IAP2_DEBUG_PACKET
#define IAP2_DEBUG_MALLOCx
#else
#define DEBUG_PRINT(x)
#endif /* IAP2_DEBUG_LIB */

/* Debug print macros */
#ifdef IAP2_DEBUG_GENERAL
#define DEBUG_PRINT(x) PRINT(x)
#else
#define DEBUG_PRINT(x)
#endif /* IAP2_DEBUG_GENERAL */

#ifdef IAP2_DEBUG_CP
#define DEBUG_PRINT_CP(x) PRINT(x)
#else
#define DEBUG_PRINT_CP(x)
#endif /* IAP2_DEBUG_CP */

#ifdef IAP2_DEBUG_INIT
#define DEBUG_PRINT_INIT(x) PRINT(x)
#else
#define DEBUG_PRINT_INIT(x)
#endif /* IAP2_DEBUG_INIT */

#ifdef IAP2_DEBUG_CONNECT
#define DEBUG_PRINT_CONNECT(x) PRINT(x)
#else
#define DEBUG_PRINT_CONNECT(x)
#endif /* IAP2_DEBUG_CONNECT */

#ifdef IAP2_DEBUG_LINK
#define DEBUG_PRINT_LINK(x) PRINT(x)
#else
#define DEBUG_PRINT_LINK(x)
#endif /* IAP2_DEBUG_LINK */

#ifdef IAP2_DEBUG_PACKET
#define DEBUG_PRINT_PACKET(x) PRINT(x)
#else
#define DEBUG_PRINT_PACKET(x)
#endif /* IAP2_DEBUG_PACKET */

#ifdef IAP2_DEBUG_MALLOC
#define MALLOC(x) PanicUnlessMalloc(x)
#else
#define MALLOC(x) malloc(x)
#endif /* IAP2_DEBUG_MALLOC */

#endif /* __IAP2_DEBUG_H__ */

/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    hydra_dev.h

DESCRIPTION
    Header file for the defines required for csra68100 development.
*/

/* This file contains various defines needed to get the old VM code building 
   whilst parts of the production code remain incomplete. 
   It is included with all of the VM libs with -preinclude.
   Remove this file when the project is complete.
*/


#ifndef _HYDRA_DEV_H
#define _HYDRA_DEV_H


#include <trapsets.h>
#include <psu.h>
#include <vm.h>
#include <message.h>
#include <charger.h>
#include <file.h>
#include <micbias.h>
#include <transform.h>
#include <kalimba.h>
#include <pio.h>


/* These should be in ps_if.h */
typedef uint16 pskey;


#define checkDUTKeyRelease(x, y) ((void)0)
#define checkDUTKeyPress(x) ((void)0)


/* Needed for VM library builds */
#define PIOS_PER_BANK 32

/* memcmp8 is specific to the XAP processor, comparing just 
   8 bits of each element in a "uint8" (which is 16 bits 
   on the XAP processor).  As we only have 8 bits in each element,
   we can use the standard memcmp */
#define memcmp8(x, y, z) memcmp((x),(y),(z))

/* Within CSRA68100 and friends source we want these functions to resolve to the 'byte' version */
#define UtilCompare(A, B, C) UtilCompareByte((const uint8*)(A), (const uint8*)(B), (C))
#define UtilFind(A, B, C, D, E, F) UtilFindByte((uint8)(A), (uint8)(B), (const uint8*)(C), (D), (E), (F))

#endif /*_HYDRA_DEV_H*/

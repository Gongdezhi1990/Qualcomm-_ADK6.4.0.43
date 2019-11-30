/****************************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd

FILE NAME
    imu_ppg_private.h

DESCRIPTION
    This file contains data private to the IMU and PPG device library.

NOTES

****************************************************************************/
//#ifdef ACTIVITY_MONITORING
#ifndef IMU_PPG_PRIVATE_H
#define IMU_PPG_PRIVATE_H

#include "message.h"
#include <panic.h>

/** Macros for creating messages */
#define MAKE_IMUPPG_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_IMUPPG_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN - 1);

/** Macro used to generate debug lib printfs. */
#ifdef IMUPPG_DEBUG_LIB
#define DEBUG_PRINT_ENABLED
#include <panic.h>
#include <stdio.h>
#include <print.h>
#define IMUPPG_FATAL_IN_DEBUG(x)   {printf x; Panic();}
#else
#define IMUPPG_FATAL_IN_DEBUG(x)
#endif


#endif /* IMU_PPG_PRIVATE_H*/
//#endif /* ACTIVITY_MONITORING */


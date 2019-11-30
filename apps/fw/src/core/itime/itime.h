/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef ITIME_H_
#define ITIME_H_

#if defined(__KALIMBA__) || \
    (defined (SUBSYSTEM_APPS) && defined(DESKTOP_TEST_BUILD) && !defined(SUBSYSTEM_APPS_OFF_CHIP))
#include "itime_kal/itime_kal.h"
#else
#include "itime_xap/itime_xap.h"
#endif

#endif /* ITIME_H_ */

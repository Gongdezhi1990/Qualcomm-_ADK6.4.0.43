/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
#ifndef TIMED_EVENT_H_
#define TIMED_EVENT_H_

/**
 * \file
 * Shim layer redirecting clients to the appropriate implementation module
 */

#ifdef OS_OXYGOS
#include "types.h"
#include "timed_event_oxygen/timed_event_oxygen.h"
#else
#include "timed_event_carlos/timed_event_carlos.h"
#endif

#endif /* TIMED_EVENT_H_ */

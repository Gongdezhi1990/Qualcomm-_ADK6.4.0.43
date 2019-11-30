/*
Copyright (c) 2015 Qualcomm Technologies International, Ltd.

*/

/*!
\file    
\ingroup sink_app
\brief   Interface to the Auto power off functionality. 
*/

#ifndef _SINK_AUTO_POWER_OFF_H_
#define _SINK_AUTO_POWER_OFF_H_

#include <csrtypes.h>
#include <message.h>

#include "sink_states.h"

/*************************************************************************
NAME
    sinkHandleAutoPowerOff

DESCRIPTION
    This function determines whether to send auto power off immediately or need to reset the timer again.

RETURNS
    TRUE for postponed the auto power off, FALSE otherwise

**************************************************************************/
bool sinkHandleAutoPowerOff(Task task, sinkState lState);

/*************************************************************************
NAME
    sinkStartAutoPowerOffTimer

DESCRIPTION
    This function starts the auto power OFF timer

RETURNS
    VOID

**************************************************************************/

void sinkStartAutoPowerOffTimer(void);

/*************************************************************************
NAME
    sinkStopAutoPowerOffTimer

DESCRIPTION
    This function stops the auto power OFF timer

RETURNS
    VOID

**************************************************************************/

void sinkStopAutoPowerOffTimer(void);

#endif /* _SINK_AUTO_POWER_OFF_H_ */


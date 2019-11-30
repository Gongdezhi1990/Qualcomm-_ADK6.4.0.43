/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    subsystem_if.h

CONTAINS
    Definitions for types used by subsystem messages to system task.

DESCRIPTION
    This file is seen by the stack/CDA devices, and customer applications, and
    contains things that are common between them.
*/

/*!
 @file subsystem_if.h
 @brief Types used by the subsystem messages to system task.

 See #MessageSubsystemEventReport() for documentation on receiving
 messages on subsystem events.
*/
#ifndef __APP_SUBSYSTEM_IF_H__
#define __APP_SUBSYSTEM_IF_H__

typedef enum
{
    SUBSYSTEM_EVENT_PANIC,
    SUBSYSTEM_EVENT_FAULT,
    SUBSYSTEM_EVENT_WATCHDOG
}subsystem_event_level;

#endif /*__APP_SUBSYSTEM_IF_H__*/

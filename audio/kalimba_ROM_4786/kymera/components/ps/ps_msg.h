/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file ps_msg.h
 * \ingroup ps
 *
 * Persistent Storage private API between "ps" and "ps_msg".
 */

#ifndef PS_MSG_H
#define PS_MSG_H

#include "ps_common.h"
#include "sched_oxygen/sched_oxygen.h"

/** Message IDs used in messages to the platform-specific PS tasks. */
#define PS_WRITE_REQ                0x4000
#define PS_READ_REQ                 0x4001
#define PS_ENTRY_DELETE_REQ         0x4002
#define PS_DELETE_REQ               0x4003

/** Message name & ID for the shutdown notification that comes from
 *  application layer.
 */
#define PS_SYSTEM_SHUTDOWN_MSG      0x0001
#define PS_READ_RESP                0x0002
#define PS_WRITE_RESP               0x0003
#define PS_REGISTER                 0x0004

/** Structure of message payload sent to the platform-specific PS message handler. */
/* TODO: PS_MSG_DATA should be split in different messages. */
typedef struct
{
    /** callback function pointer - for responding */
    void* callback;

    /** The PS key value */
    PS_KEY_TYPE key;

    /** PS rank */
    PERSISTENCE_RANK rank;

    /** ACCMD: Indicate whether the request was successful */	
    bool success;

    /** ACCMD: Size of value associated with a key. */
    uint16 total_length;
	
    /** Length of data vehiculated for the PS key */
    uint16 data_length;

    /** ACCMD: Register. BC should use that too. */
    unsigned conn_id;

    /** Placeholder for data for the PS key */
    uint16 data[];
} PS_MSG_DATA;

void ps_msg_queue(MSG *pmsg);

#endif /* PS_MSG_H */


/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#ifndef _TEST_SINK_H_
#define _TEST_SINK_H_

#include <message.h>
#include "sink_states.h"
#include "sink_events.h"

/* Register the main task  */
void test_init(void);

#define SINK_TEST_MESSAGE_BASE   0x2000

/**************************************************
   VM2HOST
 **************************************************/
typedef enum {
    SINK_TEST_STATE = SINK_TEST_MESSAGE_BASE,
    SINK_TEST_EVENT
} vm2host_sink;

typedef struct {
    uint16 state;    /*!< The Sink app state. */
} SINK_TEST_STATE_T;

typedef struct {
    uint16 event;   /*!< The Sink app event. */
} SINK_TEST_EVENT_T;

/* HS State notification */
void vm2host_send_state(sinkState state);

/* HS Event notification */
void vm2host_send_event(sinkEvents_t event);

/**************************************************
   HOST2VM
 **************************************************/
typedef enum {
    SINK_TEST_EVENT_MSG = SINK_TEST_MESSAGE_BASE + 0x80
} host2vm_sink;

typedef struct {
    uint16 event;
} SINK_TEST_EVENT_MSG_T;

typedef struct {
    uint16 length;
    uint16 bcspType;
    uint16 funcId;

    union {
        SINK_TEST_EVENT_MSG_T SINK_TEST_EVENT_MSG;
    } sink_from_host_msg;
} sink_from_host_msg_T;

/* Sink app host messages handler */
void handle_msg_from_host(Task task, MessageId id, Message message);

#endif

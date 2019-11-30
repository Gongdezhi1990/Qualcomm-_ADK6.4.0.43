/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include <app/message/system_message.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>

#include "sink_main_task.h"
#include "test_sink.h"
#include "test_utils.h"

static const TaskData testTask = {handle_msg_from_host};

/* Register the test task  */
void test_init(void) {
    if (MessageHostCommsTask((TaskData*)&testTask)) {Panic();}
}

/**************************************************
   VM2HOST
 **************************************************/

/* HS State notification */
void vm2host_send_state(sinkState state) {
    SINK_TEST_STATE_T message;
    message.state = state;
    test_send_message(SINK_TEST_STATE, (Message)&message, sizeof(SINK_TEST_STATE_T), 0, NULL);
}

/* HS Event Notification */
void vm2host_send_event(sinkEvents_t event) {
    SINK_TEST_EVENT_T message;
    message.event = event;
    test_send_message(SINK_TEST_EVENT, (Message)&message, sizeof(SINK_TEST_EVENT_T), 0, NULL);
}

/**************************************************
   HOST2VM
 **************************************************/

/* HS host messages handler */
void handle_msg_from_host(Task task, MessageId id, Message message) {
    sink_from_host_msg_T *tmsg = (sink_from_host_msg_T *)message;

    switch (tmsg->funcId) {
        case SINK_TEST_EVENT_MSG:
            MessageSend(
                &theSink.task,
                tmsg->sink_from_host_msg.SINK_TEST_EVENT_MSG.event,
                NULL
            );
            break;
    }
}

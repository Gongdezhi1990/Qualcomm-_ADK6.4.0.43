/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 */

#include "trap_api/trap_api_private.h"
#include "macros.h"
#include "app/message/system_message.h"
#include "assert.h"

/**
 * Linked list structure for storing trap API messages for retrieval via
 * (Python) calls to \c trap_api_test_get_next()
 */
typedef struct TEST_MESSAGE_LIST
{
    struct TEST_MESSAGE_LIST *next;
    Task t;
    MessageId id;
    void *m;
} TEST_MESSAGE_LIST;

/**
 * The list of received messages
 */
static TEST_MESSAGE_LIST *list = NULL;

/**
 * The maximum size of the bodies of received messages.
 */
static uint32 max_message_body_bytes = 32;

uint32 trap_api_test_reset_max_message_body_bytes(uint32 new_size)
{
    uint32 old_size = max_message_body_bytes;
    max_message_body_bytes = new_size;
    return old_size;
}

void trap_api_test_task_handler(Task t, MessageId id, Message m)
{
    /* Just put the message on a linked list and wait for the receiver function
     * to be called */

    TEST_MESSAGE_LIST **pnext = &list;
    void *m_copy = NULL;

    while(*pnext != NULL)
    {
        pnext = &((*pnext)->next);
    }

    /* If this is a Bluestack primitive, we must take a copy because the
     * firmware will cause it to be freed as soon as we return from here.  In
     * other cases we can probably get away with it, but since these messages
     * could sit here for arbitrarily long periods of time, we should copy them.
     * Unfortunately, we don't have any idea how big the messages are. Since
     * this is test code, we use the crude solution of a reset-able flag that
     * tells us how many bytes will always be enough. */

    if (m)
    {
        m_copy = pmalloc(max_message_body_bytes);
        memcpy(m_copy, m, max_message_body_bytes); /* It doesn't matter if we
                                                    * copy off the end of m */
    }
   *pnext = pnew(TEST_MESSAGE_LIST);
   (*pnext)->t = t;
   (*pnext)->id = id;
   (*pnext)->m = m_copy;
   (*pnext)->next = NULL;
}

TEST_MESSAGE_LIST *trap_api_test_get_next(void)
{
    if (list)
    {
        TEST_MESSAGE_LIST *next = list;
        list = list->next;
        return next;
    }
    return NULL;
}

void trap_api_test_map_page_at(uint32 offset)
{
    assert(offset < (MAP_VM_BUFFER_UPPER - MAP_VM_BUFFER_LOWER));

    *(uint8 *)(MAP_VM_BUFFER_LOWER + offset) = 0;
}

PRESERVE_TYPE_FOR_DEBUGGING(MessageMoreData)
PRESERVE_TYPE_FOR_DEBUGGING(MessageMoreSpace)
PRESERVE_TYPE_FOR_DEBUGGING(MessagePioChanged)
PRESERVE_TYPE_FOR_DEBUGGING(MessageAdcResult)
PRESERVE_TYPE_FOR_DEBUGGING(MessageStreamDisconnect)
PRESERVE_TYPE_FOR_DEBUGGING(MessageEnergyChanged)
PRESERVE_TYPE_FOR_DEBUGGING(MessageSourceEmpty)
PRESERVE_TYPE_FOR_DEBUGGING(MessageUsbConfigValue)
PRESERVE_TYPE_FOR_DEBUGGING(MessageUsbSuspended)
PRESERVE_TYPE_FOR_DEBUGGING(MessageUsbAltInterface)
PRESERVE_TYPE_FOR_DEBUGGING(MessageChargerChanged)
PRESERVE_TYPE_FOR_DEBUGGING(MessageCapacitiveSensorChanged)
PRESERVE_TYPE_FOR_DEBUGGING(MessageXioVoltageMeasured)
PRESERVE_TYPE_FOR_DEBUGGING(MessageStreamSetDigest)
PRESERVE_TYPE_FOR_DEBUGGING(MessageFromOperator)
PRESERVE_TYPE_FOR_DEBUGGING(MessageImageUpgradeEraseStatus)
PRESERVE_TYPE_FOR_DEBUGGING(MessageChargerDetected)
PRESERVE_TYPE_FOR_DEBUGGING(MessageChargerStatus)
PRESERVE_TYPE_FOR_DEBUGGING(MessageImageUpgradeCopyStatus)
PRESERVE_TYPE_FOR_DEBUGGING(MessageBitserialEvent)
PRESERVE_TYPE_FOR_DEBUGGING(MessageImageUpgradeAudioStatus)
PRESERVE_TYPE_FOR_DEBUGGING(MessageImageUpgradeHashAllSectionsUpdateStatus)


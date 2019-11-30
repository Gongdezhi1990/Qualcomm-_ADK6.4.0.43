/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\file       message_broker.c
\brief      The Message Broker allows client Application Modules to register interest
            in System Messages by Message Group. The Message Broker sniffs Messages
            sent in the system, and if they belong to the interested Group, forwards
            a copy to the interested client component.
*/

#include "message_broker.h"

#include <message.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <vmtypes.h>

#define REGISTERED_CLIENT_ARRAY_LEN   5U

#ifdef MESSAGE_BROKER_DEBUG_LIB
#include <logging.h>
#define MB_DEBUG(x)  DEBUG_LOG(x)
#else
#define MB_DEBUG(x)
#endif

/*! \brief An individual registered client task
 */
typedef struct registered_client
{
    Task client_task;
    struct registered_client *next;

} registered_client_t;

/*! \brief A record of registered client tasks
 */
typedef struct clients_record
{
    registered_client_t *clients[REGISTERED_CLIENT_ARRAY_LEN];
    unsigned num_clients;
    struct clients_record *continuation;

} registered_clients_t;

/*! \brief An instance of an interested message group.
    This denotes at least one client is interested in this message group.
 */
typedef struct interested_msg_group
{
    unsigned msg_group;
    registered_clients_t registered_clients;
    struct interested_msg_group *next;

} interested_msg_group_t;

/*! \brief Head of interested message group linked list.
 */
interested_msg_group_t *head_msg_group_list = NULL;

/*! \brief Head of registered client linked list.
 */
registered_client_t *head_client_list = NULL;

/******************************************************************************
 * Internal functions
 ******************************************************************************/
static void deallocate_client_list(void)
{
    while (head_client_list != NULL)
    {
        registered_client_t * tmp = head_client_list->next;
        free(head_client_list);
        head_client_list = tmp;
    }
}

static void deallocate_registered_client_list(registered_clients_t * rc)
{
    while (rc != NULL)
    {
        registered_clients_t * tmp = rc->continuation;
        free(rc);
        rc = tmp;
    }
}

static void deallocate_msg_group_list(void)
{
    while (head_msg_group_list != NULL)
    {
        interested_msg_group_t * tmp = head_msg_group_list->next;
        deallocate_registered_client_list(head_msg_group_list->registered_clients.continuation);
        free(head_msg_group_list);
        head_msg_group_list = tmp;
    }
}

static registered_client_t * createClient(Task task)
{
    registered_client_t *client_instance = PanicUnlessMalloc(sizeof(registered_client_t));

    client_instance->client_task = task;
    client_instance->next = head_client_list;
    head_client_list = client_instance;

    return head_client_list;
}

static registered_client_t * getClient(Task task)
{
    registered_client_t *current = head_client_list;
    while (current != NULL)
    {
        if (current->client_task == task)
        {
            break;
        }
        current = current->next;
    }
    return current;
}

static interested_msg_group_t * createInterestedMsgGroup(unsigned msg_group)
{
    interested_msg_group_t *new_img = PanicUnlessMalloc(sizeof(interested_msg_group_t));
    memset(new_img, 0, sizeof(interested_msg_group_t));

    new_img->msg_group = msg_group;
    new_img->next = head_msg_group_list;
    head_msg_group_list = new_img;

    return head_msg_group_list;
}

static interested_msg_group_t * getInterestedMsgGroup(unsigned msg_group)
{
    interested_msg_group_t *current = head_msg_group_list;
    while (current != NULL)
    {
        if (current->msg_group == msg_group)
        {
            break;
        }
        current = current->next;
    }
    return current;
}

static void addClient(registered_clients_t *rc, registered_client_t *client_instance)
{
    rc->clients[rc->num_clients] = client_instance;
    rc->num_clients += 1;
}

static registered_clients_t * createRegisteredClientsContinuation(registered_client_t *client_instance)
{
    registered_clients_t * rc = PanicUnlessMalloc(sizeof(registered_clients_t));
    memset(rc, 0, sizeof(registered_clients_t));
    addClient(rc, client_instance);
    return rc;
}

static void registerClient(registered_client_t *client_instance, registered_clients_t *rc)
{
    if (rc->num_clients < REGISTERED_CLIENT_ARRAY_LEN)
    {
        MB_DEBUG(("registerClient: client %08x at index %d\n", client_instance->client_task, rc->num_clients));

        addClient(rc,client_instance);
    }
    else
    {
        if (rc->continuation == NULL)
        {
            MB_DEBUG(("registerClient: create continuation client %08x\n", client_instance->client_task));

            rc->continuation = createRegisteredClientsContinuation(client_instance);
        }
        else
        {
            registerClient(client_instance, rc->continuation);
        }
    }
}

static void spawnDuplicateMsgAndSend(Task client_task, MessageId id, void *data, size_t size_data)
{
    void * msg = NULL;
    if (size_data && data != NULL)
    {
        msg = PanicUnlessMalloc(size_data);
        memcpy(msg, data, size_data);
    }
    MessageSend(client_task, id, msg);
}

static void createAndSendNotifications(registered_clients_t *rc, MessageId id, void *data, size_t size_data)
{
    unsigned client_index = 0;

    MB_DEBUG(("notifyHelper: num_clients %d\n", rc->num_clients));

    for (client_index = 0; client_index < rc->num_clients; client_index++)
    {
        MB_DEBUG(("notifyHelper: client %d is %08x\n", client_index, rc->clients[client_index]));

        spawnDuplicateMsgAndSend(rc->clients[client_index]->client_task, id, data, size_data);
    }
}

static void notifyRegisteredClients(registered_clients_t *rc, MessageId id, void *data, size_t size_data)
{
    do
    {
        createAndSendNotifications(rc, id, data, size_data);
        rc = rc->continuation;
    }
    while(rc != NULL);
}

/******************************************************************************
 * External API functions
 ******************************************************************************/
void MessageBroker_Init(void)
{
    deallocate_client_list();
    deallocate_msg_group_list();
}

void MessageBroker_SniffMessage(MessageId id, void *data, size_t size_data)
{
    uint16 msg_group = ID_TO_MSG_GRP(id);
    interested_msg_group_t *img = NULL;

    MB_DEBUG(("SniffMessage: msg_group = %d, data = %08x size = %04x\n", msg_group, data, size_data));

    img = getInterestedMsgGroup(msg_group);
    if (img)
    {
        notifyRegisteredClients(&img->registered_clients, id, data, size_data);
    }
}

void MessageBroker_RegisterInterestInMsgGroups(Task task, uint16 *msg_groups, unsigned num_groups)
{
    uint16 msg_group_index = 0;
    registered_client_t *client = NULL;

    if (task == NULL || msg_groups == NULL)
    {
        Panic();
    }

    client = getClient(task);
    if (!client)
    {
        client = createClient(task);
    }

    for (msg_group_index = 0; msg_group_index < num_groups; msg_group_index++)
    {
        interested_msg_group_t *img;

        MB_DEBUG(("RegisterInterestInMsgGroups: msg_group = %d\n", msg_groups[msg_group_index]));

        img = getInterestedMsgGroup(msg_groups[msg_group_index]);
        if (!img)
        {
            img = createInterestedMsgGroup(msg_groups[msg_group_index]);
        }

        MB_DEBUG(("RegisterInterestInMsgGroups: img = %08x\n", img));

        registerClient(client, &img->registered_clients);
    }
}

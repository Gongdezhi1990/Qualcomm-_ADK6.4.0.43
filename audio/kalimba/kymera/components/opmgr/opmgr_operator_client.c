/****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_operator_client.c
 * \ingroup  opmgr
 *
 * Operator Manager for Operator Clients. <br>
 * This file contains the operator manager functionalities for operator clients.
 * This contains both APIs used by the Framework (and thus not exposed to the
 * customers) and APIs exposed to the operator client developer, which can be
 * the customer itself. <br>
 */

/****************************************************************************
Include Files
*/

#include "opmgr_private.h"
#include "stream/stream_endpoint.h"
#include "opmgr_op_client_framework.h"
#include "opmgr_op_client_interface.h"
#include "opmsg_prim.h"

/****************************************************************************
Private Type Declarations
*/

typedef struct OP_CLIENT
{
    bool valid;
    unsigned ext_op_id;
    unsigned con_id;
    unsigned num_owned_ops;
    unsigned num_owned_eps;
    unsigned *owned_ops;
    ENDPOINT **owned_eps;
} OP_CLIENT;

#define OPMGR_ANY_SIZE 1

typedef struct OP_CLIENT_UNSOLICITED_MESSAGE
{
    OPMSG_HEADER header;
    EXT_OP_ID src_op_id;
    unsigned payload[OPMGR_ANY_SIZE];
} OP_CLIENT_UNSOLICITED_MESSAGE;

typedef struct OP_CLIENT_MSG_RESPONSE
{
    OPMSG_HEADER header;
    EXT_OP_ID src_op_id;
    unsigned status;
    unsigned payload[OPMGR_ANY_SIZE];
} OP_CLIENT_MSG_RESPONSE;

typedef struct OP_CLIENT_CMD_RESPONSE
{
    OPMSG_HEADER header;
    unsigned count;
    unsigned status;
    unsigned err_code;
} OP_CLIENT_CMD_RESPONSE;

/****************************************************************************
Private Constant Declarations
*/
#define MAX_NUM_OP_CLIENT    4
/****************************************************************************
Private Macro Declarations
*/
/* New patchpoints cannot be added in stre_rom */
#undef patch_fn_shared
#define patch_fn_shared(op_client)
/****************************************************************************
Private Variable Definitions
*/

/** Table of all the created mappings between a client operator and a creator id
 */
static OP_CLIENT *op_client_db[MAX_NUM_OP_CLIENT];

/****************************************************************************
Private Function Declarations
*/

static bool op_client_create_owned_ops(OP_CLIENT *client, unsigned con_id,
                                       unsigned num_owned_ops, unsigned *owned_op_array);
static bool op_client_create_owned_eps(OP_CLIENT *client, unsigned con_id,
                                       unsigned num_owned_eps, unsigned *owned_ep_array);

static OP_CLIENT *op_client_with_op_id(unsigned ext_op_id);
static OP_CLIENT *op_client_with_con_id(unsigned con_id);
static OP_CLIENT *op_client_new(unsigned ext_op_id);
static void op_client_delete(OP_CLIENT *client);
static void op_client_notify_promotion(unsigned con_id, OP_CLIENT *client);
static bool op_client_empty_cback(unsigned con_id, unsigned status,
        unsigned op_id, unsigned num_resp_params, unsigned *resp_params);
static bool op_client_send_message_cback(unsigned con_id, unsigned status,
        unsigned ext_op_id, unsigned num_resp_params, unsigned *resp_params);
static bool op_client_operator_action_cback(unsigned con_id, unsigned status,
        unsigned count, unsigned err_code);
/****************************************************************************
Public Function Definitions
To be used within the Framework. Must not be exposed to the operator client
codespace.
*/

/**
 * \brief Promotes an operator to be an operator client.
 *        This API will be called to serve a request from the application client.
 *        Effectively creates a new entry in the operator client DB, generating
 *        a new client id, saved in the con_id. This will be used to set this
 *        operator as owner of the operators specified in owned_op_array and the
 *        endpoints in owned_ep_array.
 *        This API also checks that the client has the rights to perform this
 *        operation, i.e. it is the owner of the operator to promote as well as
 *        the operators and endpoints it is delegating.
 *
 *        Note: To check the ownership, we consider that this API should be
 *        called to handle a request from Apps (or OBPM). At this point, input
 *        con_id equals client_id, since there will be no info on the rcv id or
 *        processor id.
 *        Note2: OBPM will skip these tests.
 *
 * \param con_id Connection ID of this call.
 * \param ext_op_id External operator ID, as seen by the application client.
 * \param num_owned_ops Number of operators owned by this operator client.
 * \param owned_op_array Array of the external operator ids to take ownership of.
 * \param num_owned_eps Number of endpoints owned by this operator client.
 * \param owned_ep_array Array of the external endpoint ids to take ownership of.
 *
 * \return TRUE if the client has successfully been promoted.
 */
bool opmgr_op_client_promote(unsigned con_id, unsigned ext_op_id,
                             unsigned num_owned_ops, unsigned * owned_op_array,
                             unsigned num_owned_eps, unsigned * owned_ep_array)
{
    OP_CLIENT *client;
    OPERATOR_DATA *op_data;

    patch_fn_shared(op_client);

    if (num_owned_ops == 0)
    {
        /* We should delegate at least one operator. */
        return FALSE;
    }

    op_data = get_op_data_from_id(EXT_TO_INT_OPID(ext_op_id));
    if (op_data == NULL)
    {
        /* Operator not found. */
        return FALSE;
    }

    /* Client con_id should be the very same as the con_id saved in the
     * operator's op_data. Note that this means that only operators in P0 will
     * pass this test and thus be promoted.
     */
    if ((con_id != op_data->con_id) && (con_id != RESPOND_TO_OBPM))
    {
        /* The client is trying to promote an operator that someone else owns.
         * Not allowed. */
        return FALSE;
    }

    client = op_client_new(ext_op_id);
    if (client == NULL)
    {
        /* Operator client already exists or no space left. */
        return FALSE;
    }

    /*
     * Create list with the (OPERATOR_DATA *) for each operator.
     */
    if (!op_client_create_owned_ops(client, con_id, num_owned_ops, owned_op_array))
    {
        /* Release all memory and fail. */
        op_client_delete(client);
        return FALSE;
    }

    if (num_owned_eps > 0)
    {
        /**
         * Create list with the (ENDPOINT *) for each owned endpoint.
         */
        if (!op_client_create_owned_eps(client, con_id, num_owned_eps, owned_ep_array))
        {
            /* Release all memory and fail. */
            op_client_delete(client);
            return FALSE;
        }
    }

    /* Set ownership of delegated operators to the operator client */
    opmgr_set_creator_id(client->owned_ops, client->num_owned_ops, client->con_id);

    /* Set ownership of delegated endpoints (real endpoints and operator
     * endpoints related to delegated operators), to the operator client */
    stream_set_endpoint_connection_id(client->owned_eps, client->num_owned_eps,
                                      client->con_id);

    /* Notify client of the delegated operators. */
    op_client_notify_promotion(con_id, client);

    return TRUE;
}


/**
 * \brief Cancels promotion of an operator to operator client.
 *        This API will be called to serve a request from the application client.
 *        Effectively resets the ownership of the operators and endpoints and
 *        destroys the entry in the operator client DB.
 *        This API also checks that the client has the rights to perform this
 *        operation, i.e. it is the owner of the promoted operator.
 *
 * \param con_id Connection ID of this call.
 * \param ext_op_id External operator ID, as seen by the application client.
 *
 * \return TRUE if the client has successfully been un-promoted.
 *         FALSE if there is an error. Error causes can be:
 *         - operator doesn't exist or is not a client
 *         - requesting client doesn't own the operator
 */
bool opmgr_op_client_unpromote(unsigned con_id, unsigned ext_op_id)
{
    OP_CLIENT *client = op_client_with_op_id(ext_op_id);
    OPERATOR_DATA *op_data;

    patch_fn_shared(op_client);

    if (client == NULL)
    {
        /* Operator is not an operator client. */
        return FALSE;
    }

    op_data = get_op_data_from_id(EXT_TO_INT_OPID(client->ext_op_id));
    if (op_data == NULL)
    {
        L2_DBG_MSG1("Client with op id 0x%04x is not a valid operator", client->ext_op_id);
#ifdef OP_CLIENT_DEBUG
        panic_diatribe(PANIC_AUDIO_OP_CLIENT_CORRUPTED, client->ext_op_id);
#endif
    }

    if ((con_id != op_data->con_id) && (con_id != RESPOND_TO_OBPM))
    {
        /* The client is trying to unpromote an operator that someone else owns.
         * Not allowed. */
        return FALSE;
    }
    opmgr_set_creator_id((const unsigned *) client->owned_ops, client->num_owned_ops,
                            op_data->con_id);

    stream_set_endpoint_connection_id(client->owned_eps, client->num_owned_eps,
                                        op_data->con_id);

    op_client_delete(client);
    return TRUE;
}

/**
 * \brief Forward an unsolicited message to the operator client that owns the
 *        sender operator. The sender's connection id actually identifies the
 *        operator client.
 *
 * \param reversed_con_id Connection ID for the sender operator (reversed).
 * \param msg_length Length of the unsolicited message.
 * \param msg Unsolicited message.
 *
 * \return TRUE if the message was successfully forwarded.
 */
bool opmgr_op_client_unsolicited_message(unsigned reversed_con_id,
                                         unsigned msg_length, unsigned * msg)
{
    /* The operator client is the receiver of this message. */
    unsigned client_id = GET_CON_ID_RECV_ID(reversed_con_id);
    OP_CLIENT *client = op_client_with_con_id(client_id);
    OPERATOR_DATA *dst_op;
    tRoutingInfo new_rinfo;
    OP_UNSOLICITED_MSG *message = (OP_UNSOLICITED_MSG *)msg;
    OP_CLIENT_UNSOLICITED_MESSAGE *client_msg;
    unsigned len;

    patch_fn_shared(op_client);

    if (client == NULL)
    {
        /* Invalid connection id. The client might have been unpromoted.
         * Fail and ignore */
        return FALSE;
    }

    /* Check if sender is owned by this operator client. */
    if (message->client_id != client_id)
    {
        return FALSE;
    }

    /* Lookup the operators */
    dst_op = get_op_data_from_id(EXT_TO_INT_OPID(client->ext_op_id));
    if (dst_op == NULL)
    {
        L2_DBG_MSG1("Client with op id 0x%04x is not a valid operator",client->ext_op_id);
#ifdef OP_CLIENT_DEBUG
        panic_diatribe(PANIC_AUDIO_OP_CLIENT_CORRUPTED, client->ext_op_id);
#endif
    }

    new_rinfo.src_id = client->con_id;

    /* The destination id is the opid (op client is always in P0) */
    new_rinfo.dest_id = dst_op->id;

    /* Allocate space for the client_msg.
     * Memory freed once the receiver's opmgr_task_handler processes the msg. */
    len = message->length + CLIENT_UNSOLICITED_MESSAGE_SIZE;
    client_msg = (OP_CLIENT_UNSOLICITED_MESSAGE *) xpnewn(len, unsigned);
    if (client_msg == NULL)
    {
        return FALSE;
    }

    /* fill in the client_msg fields */
    len = message->length + CLIENT_UNSOLICITED_MESSAGE_SIZE_EXTRA;
    client_msg->header.cmd_header.client_id = client_id;
    client_msg->header.cmd_header.length = len;
    client_msg->header.msg_id = message->msg_id | OPMSG_OP_CLIENT_REPLY_ID_MASK;
    /* The operator id will have been switched to an external op id from the
     * adaptor. No need to convert it here. */
    client_msg->src_op_id = message->op_id;

    if (message->length != 0)
    {
        memcpy(client_msg->payload, message->payload, message->length*sizeof(unsigned));
    }

    /* Save the callback so we can use it when we get the response - make sure it is saved with the
    * conid we received here, not the one stored at creation. If anyone other than creator or special
    * super-user like OBPM is talking to us, we don't reach this point.
    */

    if (!opmgr_store_in_progress_task(new_rinfo.src_id, dst_op->id,
                                        (void *)op_client_empty_cback))
    {
        /* If we couldn't save the in progress task data then panic */
        panic_diatribe(PANIC_AUDIO_OP_CLIENT_CALLBACK_FAILED, client->ext_op_id);
    }

    put_message_with_routing(mkqid(dst_op->task_id, 1), OPCMD_MESSAGE, client_msg, &new_rinfo);

    return TRUE;
}

/****************************************************************************
Public Function Definitions
To be used for the development of an operator client.
*/

/**
 * \brief Send a message to one of the owned operators.
 *        All of the target operators must be owned by this client otherwise
 *        the function will fail.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 * \param target_op_id External operator id of the target operator
 * \param msg_length Length of the message to send
 * \param msg Contents of the message to send
 *
 * \return TRUE if the message has been successfully sent
 */
bool opmgr_op_client_send_message(OPERATOR_DATA *op_data, OPERATOR_ID target_op_id,
                                  unsigned msg_length, unsigned * msg)
{
    OP_CLIENT *client = op_client_with_op_id(INT_TO_EXT_OPID(op_data->id));

    if (client == NULL)
    {
        return FALSE;
    }

    opmgr_operator_message(client->con_id, target_op_id,
            msg_length, msg, op_client_send_message_cback);

    return TRUE;
}

/**
 * \brief Send a "start operator" request to a list of the owned operators.
 *        This will fail if the target operators are not owned by this client.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 * \param num_ops Number of operators to start
 * \param op_list List of operators to start
 *
 * \return TRUE if the request has been successfully forwarded to opmgr
 */
bool opmgr_op_client_start_operator(OPERATOR_DATA *op_data,
                                    unsigned num_ops, OPERATOR_ID *op_list)
{
    OP_CLIENT *client = op_client_with_op_id(INT_TO_EXT_OPID(op_data->id));

    if (client == NULL)
    {
        return FALSE;
    }

    opmgr_start_operator(client->con_id, num_ops, op_list, op_client_operator_action_cback);

    return TRUE;
}


/**
 * \brief Send a "stop operator" request to a list of the owned operators.
 *        This will fail if the target operators are not owned by this client.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 * \param num_ops Number of operators to stop
 * \param op_list List of operators to stop
 *
 * \return TRUE if the request has been successfully forwarded to opmgr
 */
bool opmgr_op_client_stop_operator(OPERATOR_DATA *op_data,
                                   unsigned num_ops, OPERATOR_ID *op_list)
{
    OP_CLIENT *client =  op_client_with_op_id(INT_TO_EXT_OPID(op_data->id));

    if (client == NULL)
    {
        return FALSE;
    }

    opmgr_stop_operator(client->con_id, num_ops, op_list, op_client_operator_action_cback);

    return TRUE;
}


/**
 * \brief Send a "reset operator" request to a list of the owned operators.
 *        This will fail if the target operators are not owned by this client.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 * \param num_ops Number of operators to reset
 * \param op_list List of operators to reset
 *
 * \return TRUE if the request has been successfully forwarded to opmgr
 */
bool opmgr_op_client_reset_operator(OPERATOR_DATA *op_data,
                                    unsigned num_ops, OPERATOR_ID *op_list)
{
    OP_CLIENT *client = op_client_with_op_id(INT_TO_EXT_OPID(op_data->id));

    if (client == NULL)
    {
        return FALSE;
    }

    opmgr_reset_operator(client->con_id, num_ops, op_list, op_client_operator_action_cback);

    return TRUE;
}


/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Create array with the (OPERATOR_DATA *) for each owned operator.
 *
 * \param client Operator client.
 * \param con_id Connection ID of this call.
 * \param num_owned_ops Number of operators owned by this operator client.
 * \param owned_op_array Array of the external operator ids to take ownership of.
 *
 * \return TRUE if the array has successfully been created.
 */
static bool op_client_create_owned_ops(OP_CLIENT *client, unsigned con_id,
                                       unsigned num_owned_ops, unsigned *owned_op_array)
{
    unsigned i;
    OPERATOR_DATA *op_data;

    client->num_owned_ops = num_owned_ops;
    /* Allocate necessary memory. Freed in op_client_destoy. */
    client->owned_ops = xzpnewn(num_owned_ops, unsigned);
    if (client->owned_ops == NULL)
    {
        /* No memory. */
        return FALSE;
    }

    for (i = 0; i < num_owned_ops; i++)
    {
        op_data = get_anycore_op_data_from_id(EXT_TO_INT_OPID(owned_op_array[i]));
        if (op_data == NULL)
        {
            /* An operator from the list was not found. */
            return FALSE;
        }
        /* Note that the operator may be running in the other processor. */
        if ((con_id != GET_CON_ID_OWNER_CLIENT_ID(op_data->con_id)) &&
            (con_id != RESPOND_TO_OBPM))
        {
            /* The client is trying to delegate ownership of an operator it
             * doesn't own. */
            return FALSE;
        }
        client->owned_ops[i] = owned_op_array[i];
    }
    return TRUE;
}

/**
 * \brief Create array with the (ENDPOINT *) for each owned endpoint.
 *
 * \param client Operator client.
 * \param con_id Connection ID of this call.
 * \param num_owned_eps Number of endpoints owned by this operator client.
 * \param owned_ep_array Array of the external endpoint ids to take ownership of.
 *
 * \return TRUE if the array has successfully been created.
 */
static bool op_client_create_owned_eps(OP_CLIENT *client, unsigned con_id,
                                       unsigned num_owned_eps, unsigned *owned_ep_array)
{
    ENDPOINT * ep;
    unsigned i;

    client->num_owned_eps = num_owned_eps;
    /* Allocate necessary memory. Freed in op_client_destoy. */
    client->owned_eps = xzpnewn(num_owned_eps, ENDPOINT *);
    if (client->owned_eps == NULL)
    {
        /* No memory. */
        return FALSE;
    }

    for (i = 0; i < num_owned_eps; i++)
    {
        ep = stream_endpoint_from_extern_id(owned_ep_array[i]);
        if (ep == NULL)
        {
            /* An endpoint from the list was not found. */
            return FALSE;
        }
        /* Client con_id should be the very same as the con_id saved in the
         * endpoint's data. Note that this means that only endpoints in P0 will
         * pass this test and thus be delegated. Indeed, we need this only to
         * prevent the endpoints from being closed if the ACCMD service is torn
         * down.
         * Note: Currently when we tear down the ACCMD service, we first destroy
         * all operators all cores and assume that this will also close all
         * operator endpoints. Then we need to go through the remaining
         * endpoints to close the real ones, which can only be in P0.
         */
        if ((ep->con_id != con_id) && (con_id != RESPOND_TO_OBPM))
        {
            /* The client is trying to delegate ownership of an endpoint it
             * doesn't own. */
            return FALSE;
        }

        client->owned_eps[i] = ep;
    }
    return TRUE;
}

/**
 * \brief Finds an operator client entry from the DB with specified
 *        external op id.
 *
 * \param ext_op_id External Operator ID
 *
 * \return Pointer to operator client entry.
 */
static OP_CLIENT *op_client_with_op_id(unsigned ext_op_id)
{
    unsigned i;

    /* This patch point can also be used to patch interface APIs. */
    patch_fn_shared(op_client);

    /* Go through all the mappings and search con_id */
    for (i = 0; i < MAX_NUM_OP_CLIENT; i++)
    {
       if ((op_client_db[i] != NULL) && (op_client_db[i]->ext_op_id == ext_op_id))
       {
           if (op_client_db[i]->valid)
           {
               return op_client_db[i];
           }
           else
           {
               /*
                * Some race condition. The element is being created,
                * but is not ready yet. Can't really use it yet.
                */
               return NULL;
           }
       }
    }
    return NULL;
}

/**
 * \brief Finds an operator client entry from the DB with specified
 *        operator client con_id.
 *
 * \param con_id Connection ID
 *
 * \return Pointer to operator client entry.
 */
static OP_CLIENT *op_client_with_con_id(unsigned con_id)
{
    /* This patch point can also be used to patch callbacks. */
    patch_fn_shared(op_client);

    if (GET_SEND_RECV_ID_IS_SPECIAL_CLIENT(con_id))
    {
        unsigned op_client_id = GET_SEND_RECV_ID_CLIENT_INDEX(con_id);
        /* This is actually the index of the entry in the db. */
        if (op_client_id < MAX_NUM_OP_CLIENT)
        {
            OP_CLIENT *client = op_client_db[op_client_id];
            if ((client != NULL) && client->valid)
            {
                return client;
            }
        }
    }
    return NULL;
}

/**
 * \brief Create new entry in the operator client DB for "ext_op_id".
 *        Fail if exists.
 *
 * \param ext_op_id External Operator ID
 *
 * \return Pointer to operator client entry.
 */
static OP_CLIENT *op_client_new(unsigned ext_op_id)
{
    unsigned idx = MAX_NUM_OP_CLIENT;
    unsigned i;

    for (i = 0; i < MAX_NUM_OP_CLIENT; i++)
    {
        /* Find an empty slot for a new operator client. */
        if (op_client_db[i] == NULL)
        {
            /* Found one. Save it if we haven't already. */
            if (idx == MAX_NUM_OP_CLIENT)
            {
                idx = i;
            }
        }
        else if (op_client_db[i]->ext_op_id == ext_op_id)
        {
            /* op_client already created. Must be destroyed and created again
             * if needed.
             */
            return NULL;
        }
    }

    if (idx >= MAX_NUM_OP_CLIENT)
    {
        /* Too many operator clients. */
        return NULL;
    }

    op_client_db[idx] = xzpnew(OP_CLIENT);
    if (op_client_db[idx] == NULL)
    {
        /* Malloc failed! */
        return NULL;
    }
    op_client_db[idx]->con_id = MAKE_SPECIAL_CLIENT_ID(idx);
    op_client_db[idx]->ext_op_id = ext_op_id;
    /* Any other field should be initialized before this line. */
    op_client_db[idx]->valid = TRUE;

    return op_client_db[idx];
}

/**
 * \brief Delete entry from the operator client DB.
 *
 * \param client Pointer to operator client entry.
 */
static void op_client_delete(OP_CLIENT *client)
{
    unsigned i = 0;

    if (client != NULL)
    {
        for (i = 0; i < MAX_NUM_OP_CLIENT; i++)
        {
            if (op_client_db[i] == client)
            {
                op_client_db[i] = NULL;
                pdelete(client->owned_eps);
                pdelete(client->owned_ops);
                /* Any other de-initialization should be done before this line. */
                pdelete(client);
                /* Client found and deleted. */
                return;
            }
        }
        /* client had just been found in op_client_db before this call.
         * If don't find it now, something has gone wrong. */
        L2_DBG_MSG1("Pointer to client addr 0x%x is not valid.", client);
#ifdef OP_CLIENT_DEBUG
        panic_diatribe(PANIC_AUDIO_OP_CLIENT_CORRUPTED, client);
#endif
    }
}

/**
 * \brief Send a message to the promoted operator with the list of the
 *        delegated operators.
 *
 * \param con_id Connection ID of this call.
 * \param client Pointer to operator client entry.
 */
static void op_client_notify_promotion(unsigned con_id, OP_CLIENT *client)
{
    unsigned i;
    unsigned *operators;
    unsigned len = client->num_owned_ops + OPMSG_OP_CLIENT_DELEGATED_OPERATORS_OPERATORS_WORD_OFFSET;
    /* Memory freed once the receiver's opmgr_task_handler processes the msg.
     * Too late now to fail the command, panic if no mem. */
    unsigned *msg = pnewn(len, unsigned);

    OP_CLIENT_MSG_FIELD_SET(msg, OPMSG_OP_CLIENT_DELEGATED_OPERATORS,
                            MESSAGE_ID, OPMSG_OP_CLIENT_REPLY_ID_DELEGATED_OPERATORS);

    operators = OP_CLIENT_MSG_FIELD_POINTER_GET(msg, OPMSG_OP_CLIENT_DELEGATED_OPERATORS,
                                                OPERATORS);
    for (i=0; i < client->num_owned_ops; i++)
    {
        operators[i] = client->owned_ops[i];
    }
    opmgr_operator_message(con_id, client->ext_op_id,
                            len, (unsigned *)msg,
                            op_client_empty_cback);
    pfree(msg);
}

/**
 * \brief Empty callback for messages we send to an operator client.
 *        The callback is empty because no one should handle any response.
 */
static bool op_client_empty_cback(unsigned con_id, unsigned status,
                                  unsigned ext_op_id,
                                  unsigned num_resp_params, unsigned *resp_params)
{
    return TRUE;
}

/**
 * \brief Callback to notify operator client about the result of an operator
 *        message it sent to one of its owned operators. This callback is
 *        actually called by the opmgr task: to notify the operator client,
 *        we will send to it a special operator message.
 */
static bool op_client_send_message_cback(unsigned reversed_con_id, unsigned status,
                                         unsigned ext_op_id,
                                         unsigned length, unsigned *resp_payload)
{
    OP_CLIENT *client = op_client_with_con_id(
                            GET_EXT_CON_ID_RECV_ID(reversed_con_id));
    OPERATOR_DATA *dst_op;
    tRoutingInfo new_rinfo;
    OP_CLIENT_MSG_RESPONSE *client_msg;
    unsigned len;

    if (client == NULL)
    {
        /* Invalid connection id. The client might have been unpromoted.
         * Fail and ignore */
        return FALSE;
    }

    /* Lookup the operators */
    dst_op = get_op_data_from_id(EXT_TO_INT_OPID(client->ext_op_id));
    if (dst_op == NULL)
    {
        L2_DBG_MSG1("Client with op id 0x%04x is not a valid operator", client->ext_op_id);
#ifdef OP_CLIENT_DEBUG
        panic_diatribe(PANIC_AUDIO_OP_CLIENT_CORRUPTED, client->ext_op_id);
#endif
    }

    new_rinfo.src_id = client->con_id;

    /* The destination id is the opid (op client is always in P0) */
    new_rinfo.dest_id = dst_op->id;

    /* Allocate space for the client_msg, panic if no memory.
     * Memory freed once the receiver's opmgr_task_handler processes the msg.*/
    len = length + CLIENT_MSG_RESPONSE_SIZE;
    client_msg = (OP_CLIENT_MSG_RESPONSE *) pnewn(len, unsigned);

    /* fill in the client_msg fields */
    len = length + CLIENT_MSG_RESPONSE_SIZE_EXTRA;
    client_msg->header.cmd_header.client_id = client->con_id;
    client_msg->header.cmd_header.length = len;
    client_msg->header.msg_id = OPMSG_OP_CLIENT_REPLY_ID_MESSAGE_RESPONSE;
    client_msg->src_op_id = ext_op_id;
    client_msg->status = status;

    if (length != 0)
    {
        memcpy(client_msg->payload, resp_payload, length*sizeof(unsigned));
    }

    /* Save the callback so we can use it when we get the response - make sure it is saved with the
    * conid we received here, not the one stored at creation. If anyone other than creator or special
    * super-user like OBPM is talking to us, we don't reach this point.
    */

    if (!opmgr_store_in_progress_task(new_rinfo.src_id, dst_op->id,
                                        (void *)op_client_empty_cback))
    {
        /* If we couldn't save the in progress task data then panic */
        panic_diatribe(PANIC_AUDIO_OP_CLIENT_CALLBACK_FAILED, client->ext_op_id);
    }

    put_message_with_routing(mkqid(dst_op->task_id, 1), OPCMD_MESSAGE, client_msg, &new_rinfo);

    return TRUE;
}

/**
 * \brief Callback to notify operator client about the result of an operator
 *        start/stop it sent to one of its owned operators. This callback is
 *        actually called by the opmgr task: to notify the operator client,
 *        we will send to it a special operator message.
 */
static bool op_client_operator_action_cback(unsigned reversed_con_id, unsigned status,
                                            unsigned count, unsigned err_code)
{
    OP_CLIENT *client = op_client_with_con_id(
                            GET_EXT_CON_ID_RECV_ID(reversed_con_id));
    OPERATOR_DATA *dst_op;
    tRoutingInfo new_rinfo;
    OP_CLIENT_CMD_RESPONSE *client_msg;
    unsigned len;

    if (client == NULL)
    {
        /* Invalid connection id. The client might have been unpromoted.
         * Fail and ignore */
        return FALSE;
    }

    /* Lookup the operators */
    dst_op = get_op_data_from_id(EXT_TO_INT_OPID(client->ext_op_id));
    if (dst_op == NULL)
    {
        L2_DBG_MSG1("Client with op id 0x%04x is not a valid operator", client->ext_op_id);
#ifdef OP_CLIENT_DEBUG
        panic_diatribe(PANIC_AUDIO_OP_CLIENT_CORRUPTED, client->ext_op_id);
#endif
    }

    new_rinfo.src_id = client->con_id;

    /* The destination id is the opid (op client is always in P0) */
    new_rinfo.dest_id = dst_op->id;

    /* Allocate space for the client_msg, panic if no memory.
     * Memory freed once the receiver's opmgr_task_handler processes the msg. */
    client_msg = (OP_CLIENT_CMD_RESPONSE *) pnewn(CLIENT_CMD_RESPONSE_SIZE, unsigned);

    /* fill in the client_msg fields */
    len = OPMSG_OP_CLIENT_COMMAND_RESPONSE_WORD_SIZE;
    client_msg->header.cmd_header.client_id = client->con_id;
    client_msg->header.cmd_header.length = len;
    client_msg->header.msg_id = OPMSG_OP_CLIENT_REPLY_ID_COMMAND_RESPONSE;
    client_msg->count = count;
    client_msg->status = status;
    client_msg->err_code = err_code;

    /* Save the callback so we can use it when we get the response - make sure it is saved with the
    * conid we received here, not the one stored at creation. If anyone other than creator or special
    * super-user like OBPM is talking to us, we don't reach this point.
    */

    if (!opmgr_store_in_progress_task(new_rinfo.src_id, dst_op->id,
                                        (void *)op_client_empty_cback))
    {
        /* If we couldn't save the in progress task data then panic */
        panic_diatribe(PANIC_AUDIO_OP_CLIENT_CALLBACK_FAILED, client->ext_op_id);
    }

    put_message_with_routing(mkqid(dst_op->task_id, 1), OPCMD_MESSAGE, client_msg, &new_rinfo);

    return TRUE;
}


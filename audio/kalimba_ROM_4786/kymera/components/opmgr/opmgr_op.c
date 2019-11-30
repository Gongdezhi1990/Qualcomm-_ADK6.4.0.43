/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_op.c
 * \ingroup  opmgr
 *
 * Operator Manager Operator generic task file. <br>
 * This file contains the Operator generic task operations that are not
 * capability specific.<br>
 *
 * This is also the location of the table of capability function handler tables.<br>
 */

/****************************************************************************
Include Files
*/
#include "opmgr_private.h"

/****************************************************************************
Private type definitions
*/
/** A structure for storing kick propagation data. This structure represents an
 * element in a list, stored as an array. */
typedef struct
{
    /** The touched terminal mask for this element */
    unsigned t_mask;

    /** The second element depends on the type of thing being kicked. Operators
     * taskIDs are stored, Endpoints the endpoint pointer is stored.
     */
    union
    {
        /** The bg int task of the operator to kick */
        BGINT_TASK op_bgint_task;
        /** The endpoint to kick */
        ENDPOINT *ep;
    }kt;
} KP_ELEMENT;

/** A structure for storing kick propagation data.*/
struct KPT
{
    /** Length of table array */
    unsigned table_length;
    /** Number of operators connected to source terminals */
    unsigned num_op_sources;
    /** Number of operators connected to sink terminals */
    unsigned num_op_sinks;
    /** Number of endpoints connected to source terminals */
    unsigned num_ep_sources;
    /** Number of endpoints connected to sink terminals */
    unsigned num_ep_sinks;
    /** Table of entities to kick */
    KP_ELEMENT table[];
};

/****************************************************************************
Private constant definitions
*/
/** The kick propagation table section identifiers */
#define KP_TABLE_OP_SOURCES_SECTION 1
#define KP_TABLE_OP_SINKS_SECTION 2
#define KP_TABLE_EP_SOURCES_SECTION 3
#define KP_TABLE_EP_SINKS_SECTION 4


/****************************************************************************
Private variable definitions
*/

AUDIO_LOG_STRING(opcmd_create_str, "OPCMD_CREATE");
AUDIO_LOG_STRING(opcmd_destroy_str, "OPCMD_DESTROY");
AUDIO_LOG_STRING(opcmd_start_str, "OPCMD_START");
AUDIO_LOG_STRING(opcmd_stop_str, "OPCMD_STOP");
AUDIO_LOG_STRING(opcmd_reset_str, "OPCMD_RESET");
AUDIO_LOG_STRING(opcmd_connect_str, "OPCMD_CONNECT");
AUDIO_LOG_STRING(opcmd_disconnect_str, "OPCMD_DISCONNECT");
AUDIO_LOG_STRING(opcmd_buff_details_str, "OPCMD_BUFFER_DETAILS");
AUDIO_LOG_STRING(opcmd_data_format_str, "OPCMD_DATA_FORMAT");
AUDIO_LOG_STRING(opcmd_sched_info_str, "OPCMD_SCHED_INFO");
AUDIO_LOG_STRING(opcmd_test_str, "OPCMD_TEST");

const char const *opcmd_debug_strings[OPCMD_ARRAY_SIZE] = 
{
    opcmd_create_str,
    opcmd_destroy_str,
    opcmd_start_str,
    opcmd_stop_str,
    opcmd_reset_str,
    opcmd_connect_str,
    opcmd_disconnect_str,
    opcmd_sched_info_str,
    opcmd_buff_details_str,
    opcmd_data_format_str,
    opcmd_test_str
};

/****************************************************************************
Public variable definitions
*/


/****************************************************************************
Private function definitions
*/
#ifdef UNIT_TEST_BUILD

#include "stream/stream_endpoint.h"
#include "platform/pl_assert.h"

/*
 * check_propagation_table
 */
void check_propagation_table(unsigned ep_id_1, unsigned ep_id_2, bool kicks)
{
    if(ep_id_1 & STREAM_EP_OP_BIT)
    {

        unsigned  table_idx_start, table_idx_end,i;
        OPERATOR_DATA *cur_op = get_op_data_from_id(get_opid_from_opidep(ep_id_1));
        KP_TABLE *kpt;
        KP_ELEMENT *kp;

        if (!cur_op)
        {
            PL_ASSERT(FALSE);
        }

        kpt = cur_op->kick_propagation_table;


        if (!kpt)
        {
            PL_ASSERT(FALSE == kicks);
            return;
        }



        /* check in the table. */
        if(ep_id_2 & STREAM_EP_OP_BIT)
        {
            BGINT_TASK task;

            OPERATOR_DATA *connected_op = get_op_data_from_id(get_opid_from_opidep(ep_id_2));

            if (!sched_find_bgint(connected_op->task_id, &task))
            {
                /* The bgint task couldn't be found for the operator task, this
                 * is rather catastrophic. */
                panic_diatribe(PANIC_AUDIO_OPERATOR_HAS_NO_TASK, connected_op->task_id);
            }

            /* Go through the table and see if the entry already exists */
            /* The first entry is the malloced size of the table so skip this. This
             * becomes important when trying to add a new entry.
             */
            /* OP Sources come first in the table so is this endpoint a source of
             * this operator? If it's not then move on to the operator sinks.
             */
            if (!get_is_sink_from_opidep(ep_id_1))
            {
                table_idx_start = 0;
                table_idx_end = kpt->num_op_sources;
            }
            else
            {
                table_idx_start = kpt->num_op_sources;
                table_idx_end = table_idx_start + kpt->num_op_sinks;
            }

            kp = kpt->table;

            for (i = table_idx_start; i < table_idx_end; i++)
            {
                if (kp[i].kt.op_bgint_task == task)
                {
                    PL_ASSERT(kicks == TRUE);
                    return ;
                }
            }

            /* Kick not found. */
            PL_ASSERT(kicks == FALSE);


        }
        else
        {
            ENDPOINT *ep_2 = stream_endpoint_from_extern_id(ep_id_2);

            /* Step index to op_sinks length field */
            table_idx_start = kpt->num_op_sources + kpt->num_op_sinks;

            /* If this is sink on the operator then this goes in the next table section */
            if (get_is_sink_from_opidep(ep_id_1))
            {
                /* Step index to op_sources_kicking_eps length field */
                table_idx_start += kpt->num_ep_sources;
                table_idx_end = table_idx_start + kpt->num_ep_sinks;
            }
            else
            {
                table_idx_end = table_idx_start + kpt->num_ep_sources;
            }

            kp = kpt->table;

            for (i = table_idx_start; i < table_idx_end; i++)
            {
                if (kp[i].kt.ep == ep_2)
                {
                    PL_ASSERT(kicks == TRUE);
                    return ;
                }
            }

            /* Kick not found. */
            PL_ASSERT(kicks == FALSE);

        }
    }
    else
    {
        ENDPOINT *ep_1 = stream_endpoint_from_extern_id(ep_id_1);
        ENDPOINT *ep_2 = stream_endpoint_from_extern_id(ep_id_2);

        PL_ASSERT( kicks ==  (ep_1->ep_to_kick == ep_2));
    }

}

#endif

/**
 * \brief Function to return the terminal to which the opertor endpoint is connected
 *
 * \param  endpoint_id Operator endpoint ID.
 *
 * \return  Terminal Id of the associated operator.
 */
static unsigned int  get_op_ep_terminal_id(unsigned int endpoint_id)
{
    unsigned int terminal_id;

    terminal_id = get_terminal_from_opidep(endpoint_id);

    /* formulate the terminal id + direction */
    if(!get_is_source_from_opidep(endpoint_id))
    {
        /* Set an upper bit to indicate whether the terminal_id is sink */
        terminal_id |= TERMINAL_SINK_MASK;
    }

    return terminal_id;
}

/* look up handler in the operator message handler function table based on op msg ID / key ID */
static opmsg_handler_function lookup_opmsg_handler(const opmsg_handler_lookup_table_entry* func_table, unsigned opmsg_id)
{
    const opmsg_handler_lookup_table_entry* entry = func_table;

    while ((entry->id != opmsg_id) && (entry->handler != NULL))
    {
        entry++;
    }

    if(entry != NULL)
    {
        return entry->handler;
    }
    else
    {
        return NULL;
    }
}

/**
 * \brief Function to handle an operator message
 *
 * \param  op_data Pointer to operator data.
 * \param  message_data Pointer to the operator command message.
 * \param  response_id Pointer to response id which will be populated by the handler.
 * \param  response_data Double pointer which will hold the pointer to the response.
 *
 * \return TRUE if there is any kind of response returned.
 */
static bool handle_operator_message(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    OP_OPMSG_RSP_PAYLOAD *resp_data = NULL;
    OP_OPMSG_RSP *resp = NULL;
    unsigned int resp_length = 0;
    opmsg_handler_function op_msg_handler;
    unsigned status = STATUS_CMD_FAILED;
    unsigned i;
    unsigned message_id;
    /* The C Standard, 6.7.2.1: There may be unnamed padding within a structure object,
     * but not at its beginning. Therefore the header which is the first field can be
     * directly casted from the message pointer. */
    OPMSG_HEADER* message_header = (OPMSG_HEADER*)message_data;

    patch_fn_shared(opmgr);

    *response_id = OPCMD_MESSAGE;
    message_id = OPMGR_GET_OPCMD_MESSAGE_MSG_ID(message_header);

    L4_DBG_MSG("handle_operator_message  \n");

    /* Client ID is first field, opmsg ID / key ID is the second field in msg_data. */

    if((op_data->cap_data != NULL) && (op_data->cap_data->opmsg_handler_table != NULL))
    {
        /* Find the handler based on opmsgID/keyID in 2nd field of the message data */
        op_msg_handler = lookup_opmsg_handler((op_data->cap_data)->opmsg_handler_table, message_id);
        /* if found one, then call the handler and get the response data and length back */
        /* Incoming message data is passed "as is", so handler, if needs to, can make use of the client ID in first field */
        if(op_msg_handler != NULL)
        {
            L5_DBG_MSG1("handle_operator_message: Found operator handler for opmsg ID %d \n", message_id);
            if((*op_msg_handler)(op_data, message_data, &resp_length, &resp_data))
            {
                status = STATUS_OK;
            }
            else
            {
                L4_DBG_MSG2("Operator message ID 0x%04X sent to Operator ID 0x%04X failed", message_id, INT_TO_EXT_OPID(op_data->id));
            }
        }
        else
        {
            status = STATUS_UNSUPPORTED_COMMAND;
        }
    }

    if(status == STATUS_OK)
    {
        L5_DBG_MSG1("handle_operator_message: opmsg handler of the op executed OK, resp_length = %d\n", resp_length);
        /* if all OK, check if there is some payload in response. If not, make a default one */
        if (resp_data == NULL)
        {
            resp_length = 1;
        }

        resp = (OP_OPMSG_RSP*)xzpmalloc(sizeof(OP_OPMSG_RSP) + resp_length*sizeof(unsigned));
        if(resp == NULL)
        {
            /* Free the resp_data (could be NULL, but that's OK).*/
            pfree(resp_data);
            /* Try to send and error message. */
            status = STATUS_CMD_FAILED;
        }
        else
        {
            resp->op_id = op_data->id;
            resp->length = resp_length; /* 1 or more, as the opmsgID/keyID is echoed even if no other payload */
            resp->status = status;
            if (resp_data == NULL)
            {
                resp->payload[0] = message_id;
            }
            else
            {
                /* copy the payload, first field is the opmsgID/keyID */
                for(i=0; i<resp_length; i++)
                {
                    resp->payload[i] = ((unsigned*)resp_data)[i];
                }

                /* free the stuff passed back to us by the opmsg handler */
                pfree(resp_data);
            }
        }
    }
    /* If there is not enough memory for the response the status is changed to STATUS_CMD_FAILED.
     * Because of this reason the status has to be checked again.*/
    if(status != STATUS_OK)
    {
        /* send an error message to OpMgr */
        resp = (OP_OPMSG_RSP*)xzpmalloc(sizeof(OP_OPMSG_RSP) + sizeof(unsigned)); /* We have exactly one word of payload */
        if(resp != NULL)
        {
            resp->op_id = op_data->id;
            resp->length = 1;
            resp->status = status;

            /* echo the msgID / keyID */
            resp->payload[0] = message_id;
            resp_length = 1;
        }
    }

    *response_data = resp;
    /* At this level, we either provide a valid response
     * (which could be success or failure) and return TRUE, or
     * a NULL response (because it couldn't be allocated) and return FALSE.
     */
    return (resp != NULL);
}


/**
 * \brief Searches an operators function handler table for the handler for message_id
 *
 * \param  op_data Pointer to the operator data for the operator.
 * \param  message_id The id of the message a handler is needed for.
 *
 * \return  A function pointer to a handler, if not found NULL is returned.
 */
static handler_function lookup_handler_for_operator_commands(OPERATOR_DATA *op_data, unsigned int message_id)
{
    patch_fn_shared(opmgr);
    /* Use equivalence between handler_lookup_struct and handler_function[OPCMD_ARRAY_SIZE] */
    const handler_function *entry = (const handler_function *) op_data->cap_data->handler_table;
    if (message_id == OPCMD_MESSAGE)
    {
        /* Operator messages are a special case, handled locally in opmgr */
        return handle_operator_message;
    }
    PL_ASSERT(message_id < OPCMD_ARRAY_SIZE);

    return entry[message_id];
}

/**
 * \brief Function for sending operator commands to an operator by operator
 *        endpoint id.
 *
 * \param  endpoint_id Operator endpoint ID.
 * \param  command_id Operator command ID.
 * \param  message_data Pointer to the operator command message.
 * \param  resp_id Pointer to response id which will be populated by the command handler.
 * \param  resp Double pointer which will hold the pointer to the response.
 *
 * \return  Status of the operator command.
 */
static unsigned send_cmd_from_op_ep_to_op(unsigned int endpoint_id, unsigned int command_id, void *message_data, unsigned int *resp_id, void **resp )
{
    OPERATOR_DATA *cur_op;

    patch_fn_shared(opmgr);
    /* Lookup the operator */
    /* Look in local oplist. This function sends cmd from op ep to op,
     * therefore it is always local to the OpMgr and Stream entities
     * that make use of this.
     */
    cur_op = get_op_data_from_id(get_opid_from_opidep(endpoint_id));

    if (cur_op == NULL)
    {
        /* Streams has asked opmgr to get the data type for an endpoint that
         * opmgr cannot find. This should be impossible because we can only have
         * one ACCMD in progress at a given time (see ACCMD spec) and the act of
         * creating or detroying an operator will cause the stream endpoints to be
         * created or destroyed. This code should run at stream_connect time only.
         */
        panic_diatribe(PANIC_AUDIO_OPERATOR_NOT_FOUND, endpoint_id);
    }
    else
    {
        handler_function handler_func = lookup_handler_for_operator_commands(cur_op, command_id);

        if ((NULL == handler_func) || (!handler_func(cur_op, message_data, resp_id,resp)) || (*resp == NULL))
        {
            /* Something went wrong.
             */
            return STATUS_CMD_FAILED;
        }

        /* Check the response message contains the operator ID and extract the status
         * from it.
         */
        if ( cur_op->id == ((unsigned int *)*resp)[0])
        {
            unsigned status = ((unsigned int *)*resp)[1];
            return status;
        }

    }

    return STATUS_CMD_FAILED;
}


/**
 * \brief Helper function to determine whether a kick propagation table has
 * enough free space for a new entry to be added.
 *
 * \param The memory containing the kick propagation table
 *
 * \return TRUE if enough free space exists, FALSE if the table is full.
 */
static bool is_space_for_kick_prop_entry(KP_TABLE *table)
{
    /* Account for used length of all 4 sections:
     * Space used by op source connections
     * Space used by op sink connections
     * Space used by ep source connections
     * Space used by ep sink connections
     */
    unsigned used_space = table->num_op_sources + table->num_op_sinks +
            table->num_ep_sources + table->num_ep_sinks;

    if (used_space >= table->table_length)
    {
        return FALSE;
    }
    return TRUE;
}

/**
 * \brief Helper function that increases the size of an operators kick propagation
 * if needed to accommodate a new KP_ELEMENT entry. The table size is increased
 * by mallocing new memory and copying the old table contents into the new
 * memory.
 *
 * \param cur_op The operator to increase the kick propagation table of.
 *
 * \return TRUE The table now contains space for a new entry. FALSE No more space
 * due to out of RAM.
 */
static bool expand_kick_prop_table(OPERATOR_DATA *cur_op)
{
    KP_TABLE *old_table = cur_op->kick_propagation_table;
    if (!is_space_for_kick_prop_entry(old_table))
    {
        unsigned table_size;
        KP_TABLE *new_table;

        /* Malloc space for an extra element */
        table_size = ((old_table->table_length + 1) * sizeof(KP_ELEMENT)) + sizeof(KP_TABLE);

        new_table = (KP_TABLE *)xzpmalloc(table_size);
        if (NULL == new_table)
        {
            return FALSE;
        }
        /* Copy the existing data into the new table and then update the new size */
        memcpy(new_table, old_table, table_size - sizeof(KP_ELEMENT));
        new_table->table_length = old_table->table_length + 1;

        cur_op->kick_propagation_table = new_table;
        pfree(old_table);
        return TRUE;
    }
    return TRUE;
}

/**
 * \brief Shuffles the kick propagation table around and inserts a new KP_ELEMENT
 * in the requested table section.
 *
 * \param cur_op The operator to add a KP_ELEMENT to
 * \param table_idx The index into the table (viewed as an unsigned array) to insert the new element.
 * \param section_idx The index (viewed as an unsigned array) of the start of the
 * table section the new element is to be inserted into.
 * \param kick_item The thing to kick (either an operator task id or an endpoint)
 * \param terminal_mask The terminal mask to apply to touched terminals bit field
 *
 * \return TRUE the new element was added to the operators table. FALSE Insufficient
 * RAM prevented the operation from succeeding.
 */
static bool add_new_kick_prop_table_entry(OPERATOR_DATA *cur_op, unsigned table_idx, unsigned section, void *kick_item, unsigned terminal_mask)
{
    /* The process is make the table big enough, shuffle everything else
     * up to make this section bigger, then add the new element
     * to the list with interrupts blocked so that kick propagation
     * doesn't go screwy if a kick occurs */
    unsigned i;
    KP_TABLE *kpt;

    if (!expand_kick_prop_table(cur_op))
    {
        return FALSE;
    }

    kpt = cur_op->kick_propagation_table;

    LOCK_INTERRUPTS;
    for (i = kpt->table_length - 1; i > table_idx; i--)
    {
        kpt->table[i] =
                kpt->table[i - 1];
    }

    kpt->table[table_idx].kt.op_bgint_task = (BGINT_TASK)kick_item;
    kpt->table[table_idx].t_mask = terminal_mask;

    switch(section)
    {
        case KP_TABLE_OP_SOURCES_SECTION:
            kpt->num_op_sources++;
            break;
        case KP_TABLE_OP_SINKS_SECTION:
            kpt->num_op_sinks++;
            break;
        case KP_TABLE_EP_SOURCES_SECTION:
            kpt->num_ep_sources++;
            break;
        case KP_TABLE_EP_SINKS_SECTION:
            kpt->num_ep_sinks++;
            break;
        default:
            /* Never happens but shuts up the compiler */
            break;
    }
    UNLOCK_INTERRUPTS;
    return TRUE;
}

/**
 * \brief Shuffles the kick propagation table around and removes an existing KP_ELEMENT
 * from the requested table section.
 *
 * \param cur_op The operator to remove a KP_ELEMENT from
 * \param table_idx The index into the table (viewed as an unsigned array) of the element to remove.
 * \param section_idx The index (viewed as an unsigned array) of the start of the
 * table section the element is to be removed from.
 */
static void remove_new_kick_prop_table_entry(OPERATOR_DATA *cur_op, unsigned table_idx, unsigned section)
{
    /* The process is shuffle everything down overwriting this element
     * in the list with interrupts blocked so that kick propagation
     * doesn't go screwy if a kick occurs */
    KP_TABLE *temp = cur_op->kick_propagation_table;
    unsigned length = temp->table_length;


    LOCK_INTERRUPTS;
    for ( ; table_idx < length - 1; table_idx++)
    {
        temp->table[table_idx] =
                temp->table[table_idx + 1];
    }

    switch(section)
    {
        case KP_TABLE_OP_SOURCES_SECTION:
            temp->num_op_sources--;
            break;
        case KP_TABLE_OP_SINKS_SECTION:
            temp->num_op_sinks--;
            break;
        case KP_TABLE_EP_SOURCES_SECTION:
            temp->num_ep_sources--;
            break;
        case KP_TABLE_EP_SINKS_SECTION:
            temp->num_ep_sinks--;
            break;
        default:
            /* Never happens but shuts up the compiler */
            break;
    }
    UNLOCK_INTERRUPTS;
    /* If all sections are empty the table can be freed. */
    if (0 == temp->num_op_sources &&
            0 == temp->num_op_sinks &&
            0 == temp->num_ep_sources &&
            0 == temp->num_ep_sinks)
    {
        /* NULL the table before freeing the actual memory to prevent a race with
         * the operator executing if it is running.
         */
        cur_op->kick_propagation_table = NULL;
        pfree(temp);
    }

    return;
}


/*
 * opmgr_kick_prop_table_add
 */
bool opmgr_kick_prop_table_add(unsigned endpoint_id, unsigned ep_id_to_kick)
{
    unsigned table_size, table_idx_start, table_idx_end;
    /* Lookup the operator */
    OPERATOR_DATA *op_to_kick;

    patch_fn_shared(opmgr);

    /* Look in local oplist, as kick propagation happens in graph local
     * to OpMgr and Stream that calls this func.
     * Shadowed 'real' EPs, not OP EPs will cross the IPC barrier.
     */
    OPERATOR_DATA *cur_op = get_op_data_from_id(get_opid_from_opidep(endpoint_id));
    KP_TABLE *kpt;

    if (NULL == cur_op->kick_propagation_table)
    {
        if (cur_op->cap_data->max_sinks == cur_op->cap_data->max_sources)
        {
            table_size = 2 * sizeof(KP_ELEMENT) + sizeof(KP_TABLE);
        }
        else
        {
            table_size = 3 * sizeof(KP_ELEMENT) + sizeof(KP_TABLE);
        }
        cur_op->kick_propagation_table = xzpmalloc(table_size);
        if (NULL == cur_op->kick_propagation_table)
        {
            return FALSE;
        }

        cur_op->kick_propagation_table->table_length =
                            (table_size - sizeof(KP_TABLE)) / sizeof(KP_ELEMENT);
    }

    kpt = cur_op->kick_propagation_table;

    if (ep_id_to_kick & STREAM_EP_OP_BIT)
    {
        unsigned terminal, terminal_mask, i, section;
        KP_ELEMENT *kp;
        BGINT_TASK task;


        /* Look in local oplist, as kick propagation happens in graph
         * local to OpMgr and Stream that calls this func.
         * Shadowed 'real' EPs, not OP EPs will cross the IPC barrier.
         */
         op_to_kick = get_op_data_from_id(get_opid_from_opidep(ep_id_to_kick));

        if (!sched_find_bgint(op_to_kick->task_id, &task))
        {
            /* The bgint task couldn't be found for the operator task, this
             * is rather catastrophic. */
            panic_diatribe(PANIC_AUDIO_OPERATOR_HAS_NO_TASK, op_to_kick->task_id);
        }

        terminal = get_terminal_from_opidep(endpoint_id);
        /* Go through the table and see if the entry already exists */
        /* The first entry is the malloced size of the table so skip this. This
         * becomes important when trying to add a new entry.
         */
        /* OP Sources come first in the table so is this endpoint a source of
         * this operator? If it's not then move on to the operator sinks.
         */
        if (!get_is_sink_from_opidep(endpoint_id))
        {
            table_idx_start = 0;
            table_idx_end = kpt->num_op_sources;
            section = KP_TABLE_OP_SOURCES_SECTION;
            terminal_mask = TOUCHED_SOURCE_0 << terminal;
        }
        else
        {
            table_idx_start = kpt->num_op_sources;
            table_idx_end = table_idx_start + kpt->num_op_sinks;
            section = KP_TABLE_OP_SINKS_SECTION;
            terminal_mask = TOUCHED_SINK_0 << terminal;
        }

        kp = kpt->table;

        for (i = table_idx_start; i < table_idx_end; i++)
        {
            if (kp[i].kt.op_bgint_task == task)
            {
                kp[i].t_mask |= terminal_mask;
                return TRUE;
            }
        }
        /* Getting here means that these operators aren't already connected to each other. */
        return add_new_kick_prop_table_entry(cur_op, i, section, (void *)task, terminal_mask);
    }
    else
    {
        /* Its an endpoint that wants to be kicked by the graph so add
         * that to the list.
         */
        unsigned section, terminal_mask, terminal = get_terminal_from_opidep(endpoint_id);
        ENDPOINT *endpoint_to_kick = stream_endpoint_from_extern_id(ep_id_to_kick);

        /* Step index to op_sinks length field */
        table_idx_start = kpt->num_op_sources + kpt->num_op_sinks;

        /* If this is sink on the operator then this goes in the next table section */
        if (get_is_sink_from_opidep(endpoint_id))
        {
            /* Step index to op_sources_kicking_eps length field */
            table_idx_start += kpt->num_ep_sources;
            table_idx_end = table_idx_start + kpt->num_ep_sinks;
            section = KP_TABLE_EP_SINKS_SECTION;
            terminal_mask = TOUCHED_SINK_0 << terminal;
        }
        else
        {
            table_idx_end = table_idx_start + kpt->num_ep_sources;
            section = KP_TABLE_EP_SOURCES_SECTION;
            terminal_mask = TOUCHED_SOURCE_0 << terminal;
        }

        /* This code assumes that endpoints that care about kicks are very rare
         * and are not synchronised so each one must be kicked individually.
         * which means each one of these requires a new entry, the easiest way
         * to add that is to insert it at the start of this table section. */

        return add_new_kick_prop_table_entry(cur_op, table_idx_start, section,
                                (void *)endpoint_to_kick, terminal_mask);
    }
}

/**
 * \brief This function is used when kicks are no longer required on an operator
 * terminal's connection. This may be either due to a disconnect or when scheduling
 * decides that an operator shouldn't be kicked by the rest of the chain. This
 * function removes the entry for the connection represented by the endpoint_id
 * from the underlying capability's kick propagation table.
 *
 * \param endpoint_id The ID of the endpoint associated with the operator
 * terminal which is no longer required to propagate kicks.
 */
void opmgr_kick_prop_table_remove(unsigned endpoint_id)
{
    unsigned section, terminal, table_idx_start, table_idx_end, terminal_mask, i;
    KP_ELEMENT *kp;
    KP_TABLE *kpt;

    patch_fn_shared(opmgr);

    /* Lookup the operator */

    /* Look in local oplist, as kick propagation happens in graph
     * local to OpMgr and Stream that calls this func.
     * Shadowed 'real' EPs, not OP EPs will cross the IPC barrier.
     */
    OPERATOR_DATA *cur_op = get_op_data_from_id(get_opid_from_opidep(endpoint_id));
    /* In some scenarios the operator doesn't have a kick propagation list so
     * nothing further needs to be done if that is the case.
     */
    kpt = cur_op->kick_propagation_table;
    if (NULL == kpt)
    {
        return;
    }

    terminal = get_terminal_from_opidep(endpoint_id);

    if (!get_is_sink_from_opidep(endpoint_id))
    {
        table_idx_start = 0;
        table_idx_end = kpt->num_op_sources;
        section = KP_TABLE_OP_SOURCES_SECTION;
        terminal_mask = TOUCHED_SOURCE_0 << terminal;

    }
    else
    {
        terminal_mask = TOUCHED_SINK_0 << terminal;;
        section = KP_TABLE_OP_SINKS_SECTION;
        table_idx_start = kpt->num_op_sources;
        table_idx_end = table_idx_start + kpt->num_op_sinks;
    }

    kp = kpt->table;

    for (i = table_idx_start; i < table_idx_end; i++)
    {
        if (kp[i].t_mask & terminal_mask)
        {
            /* clear the bit for this connection as we don't propagate along
             * it any longer */
            kp[i].t_mask &= ~terminal_mask;
            /* If there are still other connections to this operator
             * then the recording that this one went away is all that
             * needs to be done. */
            if (0 != kp[i].t_mask)
            {
                return;
            }

            remove_new_kick_prop_table_entry(cur_op, i, section);
            return;
        }
    }

    /* If it wasn't found by this point then it must be connected to an
     * endpoint rather than another operator. This situation is quite rare,
     * so traverse to the kicking endpoint part of the table. */
    if (KP_TABLE_OP_SOURCES_SECTION == section)
    {
        section = KP_TABLE_EP_SOURCES_SECTION;
        table_idx_start += kpt->num_op_sources +
                                kpt->num_op_sinks;
        table_idx_end = table_idx_start + kpt->num_ep_sources;
    }
    else
    {
        section = KP_TABLE_EP_SINKS_SECTION;
        table_idx_start += kpt->num_op_sinks +
                                kpt->num_ep_sources;
        table_idx_end = table_idx_start + kpt->num_ep_sinks;
    }


    for (i = table_idx_start; i < table_idx_end; i++)
    {
        if (kp[i].t_mask & terminal_mask)
        {
            /* Remove this connection as there is only ever one connection to
             * each endpoint. */

            remove_new_kick_prop_table_entry(cur_op, i, section);
            return;
        }
    }
}

/****************************************************************************
Public function definitions
*/

/****************************************************************************
 *
 * opmgr_operator_task_handler
 *
 * Handler for handling messages sent to operators
 */
void opmgr_operator_task_handler(void **msg_data)
{
    uint16 op_cmd_id;
    unsigned resp_msg_id = OPCMD_INVALID;
    void *msg_body = NULL;
    void *resp_data = NULL;
    tRoutingInfo rinfo, new_rinfo;

    patch_fn_shared(opmgr);

    OPERATOR_DATA* current_op = (OPERATOR_DATA*)*msg_data;
    if(get_message_with_routing(mkqid(current_op->task_id, 1),
            &op_cmd_id, &msg_body, &rinfo))
    {
        /* Find the appropriate function handler, if we find it call it and
         * send the response back. */
        handler_function handler = lookup_handler_for_operator_commands(current_op, op_cmd_id);

        if (handler)
        {
            bool result = handler(current_op,
                                  msg_body,
                                  &resp_msg_id,
                                  &resp_data);

            /* Swap src and dest for the return journey */
            new_rinfo.dest_id = rinfo.src_id;
            new_rinfo.src_id = rinfo.dest_id;

            if (result)
            {
                if (resp_data == NULL)
                {
                    /* The operator claimed to succeed, but didn't fill in the response.
                     * Fail now before anything else goes wrong
                     */
                    panic_diatribe(PANIC_AUDIO_OPMGR_NO_RESPONSE_FROM_OPERATOR, current_op->id);
                }
#ifdef INSTALL_OPERATOR_CREATE_PENDING
                /* The Operator Create command is allowed to be asynchronous and can
                   return HANDLER_INCOMPLETE. In this case it should return a pointer
                   to a callback function which we call immediately. The operator
                   takes responsibility for the command completing - either by resending
                   the original message, or sending an operator response itself. */
                if (   op_cmd_id == OPCMD_CREATE
                    && (PENDABLE_OP_HANDLER_RETURN)result == HANDLER_INCOMPLETE)
                {
                    pending_operator_cb pending_handler = (pending_operator_cb)resp_data;
                    L4_DBG_MSG1("opmgr. Pending operator create  Operator %x",(void*)current_op);

                    pending_handler(current_op,op_cmd_id,msg_body,&rinfo,resp_msg_id);

                    return;
                }
#endif
            }
            else    /* result = FALSE */
            {
                OP_STD_RSP* resp;
                if (resp_data != NULL)
                {
                    /* The operator shouldn't provide a response message
                     * if it returns FALSE - this would result in a memory leak
                     * so fail hard now
                     */
                    panic_diatribe(PANIC_AUDIO_OPMGR_INVALID_RESPONSE_FROM_OPERATOR, current_op->id);
                }
                /* Try to send a reply at all cost. Panic if not possible. */
                resp = xppmalloc(sizeof(OP_STD_RSP),MALLOC_PREFERENCE_SYSTEM);
                if (resp == NULL)
                {
                    panic_diatribe(PANIC_AUDIO_NOT_ENOUGH_MEMORY_FOR_OPERATOR, current_op->id);
                }
                resp->op_id = current_op->id;
                resp->resp_data.err_code = 0;
                resp->status = STATUS_CMD_FAILED;

                resp_data = resp;
                /* The response id is same as the message ID
                 * OPCMD_FROM_OPERATOR is a special case.*/
                resp_msg_id = op_cmd_id;
            }
            if (resp_data == NULL)
            {
                L2_DBG_MSG2("Operator ID 0x%04X command 0x%04X returned NULL", INT_TO_EXT_OPID(current_op->id), op_cmd_id);
            }
            else if (((OP_STD_RSP *)resp_data)->status != STATUS_OK)
            {
                if (op_cmd_id < OPCMD_ARRAY_SIZE)
                {
                    L2_DBG_MSG3("%s sent to operator ID 0x%04X returned status 0x%04X", opcmd_debug_strings[op_cmd_id], INT_TO_EXT_OPID(current_op->id), ((OP_STD_RSP* )resp_data)->status);
                }
                else
                {
                    L2_DBG_MSG3("Command ID 0x%04X sent to operator ID 0x%04X returned status 0x%04X", op_cmd_id, INT_TO_EXT_OPID(current_op->id), ((OP_STD_RSP* )resp_data)->status);
                }
            }

            put_message_with_routing(OPMGR_TASK_QUEUE_ID,
                                    /* Msgid is 16-bit in sched oxygen. Typecast necessary
                                     * for 32-bit platforms such as crescendo */
                                    (uint16) resp_msg_id,
                                    resp_data,
                                    &new_rinfo);
        }

        /* If there was a payload to the message it is safe to free it now. */
        if (msg_body != NULL)
        {
            pdelete(msg_body);
        }
    }

}

/****************************************************************************
 *
 * opmgr_kick_from_operator
 *
 * Helper function for issue kicks from an operator
 */
RUN_FROM_PM_RAM
void opmgr_kick_from_operator(OPERATOR_DATA *op_data,unsigned source_kicks,unsigned sink_kicks)
{
   KP_TABLE *kpt = op_data->kick_propagation_table;
   unsigned sect_idx, i;

   if(kpt==NULL)
   {
      return;
   }

   /* Kick downstream operators */
   /* It's highly likely that there is a downstream operator, but less
   * likely that it actually needs kicking. So check first if any sources
   * were touched, before checking if any sources want kicks. */
   if (source_kicks)
   {
      i = 0;
      sect_idx = kpt->num_op_sources;
      if (sect_idx)
      {
          for(; i < sect_idx; i++)
          {
              if(kpt->table[i].t_mask & source_kicks)
              {
                  source_kicks &= ~kpt->table[i].t_mask;
                  raise_bg_int_with_bgint(kpt->table[i].kt.op_bgint_task);
              }
          }
      }
   }
   else
   {
      /* If we didn't bother doing anything then we need to initialise the
       * looping variables to the next section in the kicktable. */
      sect_idx = kpt->num_op_sources;
      i = kpt->num_op_sources;
   }

   /* Kick upstream operators */
   /* As before its more likely that there is an upstream operator than
   * that it actually needs a kick. So check first if sinks were touched
   * before checking if any sinks want kicks.
   */
   if (sink_kicks)
   {
      if (kpt->num_op_sinks)
      {
          sect_idx += kpt->num_op_sinks;
          for(; i < sect_idx; i++)
          {
              if(kpt->table[i].t_mask & sink_kicks)
              {
                  sink_kicks &= ~kpt->table[i].t_mask;
                  raise_bg_int_with_bgint(kpt->table[i].kt.op_bgint_task);
              }
          }
      }
   }
   else
   {
      sect_idx += kpt->num_op_sinks;
      i += kpt->num_op_sinks;
   }

   /* Kick downstream endpoints */
   /* This logic for kicking connected endpoints is the opposite as for
   * connected operators as it's
   * rare that there is an endpoint connected that wants kicks. Hence
   * it is more efficient to check if there are endpoints to be kicked
   * before checking if any sources were touched. */
   if (kpt->num_ep_sources)
   {
      sect_idx += kpt->num_ep_sources;
      if (source_kicks)
      {
          for(; i < sect_idx; i++)
          {
              if(kpt->table[i].t_mask & source_kicks)
              {
                  stream_if_kick_ep(kpt->table[i].kt.ep, STREAM_KICK_FORWARDS);
              }
          }
      }
      else
      {
          i += kpt->num_ep_sources;
      }
   }

   /* Kick upstream endpoints */
   /* This logic for kicking connected endpoints is the opposite as for
   * connected operators as it's
   * rare that there is an endpoint connected that wants kicks. Hence
   * it is more efficient to check if there are endpoints to be kicked
   * before checking if any sinks were touched. */
   if (kpt->num_ep_sinks)
   {
      if (sink_kicks)
      {
          sect_idx += kpt->num_ep_sinks;
          for(; i < sect_idx; i++)
          {
              if(kpt->table[i].t_mask & sink_kicks)
              {
                  stream_if_kick_ep(kpt->table[i].kt.ep, STREAM_KICK_BACKWARDS);
              }
          }
      }
   }
}

#ifdef PROFILER_ON
/* Debug log string to allow profiler entries to be recognised by ACAT */
LOG_STRING(operator_name, "Operator");
#endif

/****************************************************************************
 *
 * opmgr_operator_bgint_handler
 *
 * Handler for all background interrupts raised on operators.
 */
RUN_FROM_PM_RAM
void opmgr_operator_bgint_handler(void **bg_data)
{
    TOUCHED_TERMINALS touched;
    OPERATOR_DATA* current_op = (OPERATOR_DATA*)*bg_data;

    PL_PRINT_P0(TR_OPMGR, "opmgr_operator_bgint_handler, bg_int received\n");

    patch_fn(opmgr_bgint_patchpoint);

    /* Check that we have a valid context. If it's NULL then we've coded
     * something badly. */
    if (current_op == NULL)
    {
        panic_diatribe(PANIC_AUDIO_INVALID_OPERATOR_CONTEXT, (DIATRIBE_TYPE)((uintptr_t)bg_data));
    }

    /* nothing to do if the operator is not in the running state*/
    if (OP_RUNNING != current_op->state)
    {
        return;
    }

    /* There's a remote chance that the pointer could be non-NULL but
     * still invalid. See the essay in opmgr_destroy_operator for an
     * explanation of why, and why we don't expect it ever to happen. */

    /* call the operator data processing function */
    /* The operator process_data handler will populate the bitfields in touched which indicate
     * which source/sink terminals to propagate kicks along.*/
    touched.sources = TOUCHED_NOTHING;
    touched.sinks = TOUCHED_NOTHING;

#ifdef PROFILER_ON
    if (current_op->profiler == NULL) 
    {
        if (get_profiler_state())
        {
            current_op->profiler = create_dynamic_profiler(operator_name, INT_TO_EXT_OPID(current_op->id));
            L2_DBG_MSG2("Created profiler %08X for Op %04X", current_op->profiler, INT_TO_EXT_OPID(current_op->id));
        }
    }
    else
    {   
        if (!get_profiler_state())
        {
            profiler *old_profiler = current_op->profiler;
            L2_DBG_MSG2("Removing profiler %08X for Op %04X", current_op->profiler, INT_TO_EXT_OPID(current_op->id));
            current_op->profiler = NULL;
            PROFILER_DEREGISTER(old_profiler);
            pdelete(old_profiler);
        }
    }

    if (current_op->profiler != NULL)
    {
        PROFILER_MEASURE(current_op->profiler, current_op->local_process_data(current_op, &touched)); 
    }
    else
#endif /* PROFILER_ON */
    {
        current_op->local_process_data(current_op, &touched);
    }

    /* A quick check to see if there is anything to do. If there is no kick table
     * or nothing was touched then there is no work to do.
     */

    /* NOTE: There might be a potential optimisation to be made here based on
     * the fact that most capabilities only kick 1 operator up/down stream. All
     * the loop setup could be avoided in these cases. The extra logic and some
     * jumps may be avoidable.
     */
    if (touched.sources || touched.sinks)
    {
#ifdef PROFILER_ON
       if ((touched.sources) && (current_op->profiler != NULL))
       {
            current_op->profiler->kick_inc++;
       }
#endif
        opmgr_kick_from_operator(current_op,touched.sources,touched.sinks);
    }
}


/****************************************************************************
Direct function call interface to operators
*/

/****************************************************************************
 *
 * opmgr_get_buffer_details
 *
 */
bool opmgr_get_buffer_details( unsigned int endpoint_id,
                               BUFFER_DETAILS *buff_details)
{
    unsigned int msg[1];
    OP_BUF_DETAILS_RSP *resp = NULL;
    unsigned int resp_id;

    patch_fn_shared(opmgr);

    msg[0] = get_op_ep_terminal_id(endpoint_id);

    if (send_cmd_from_op_ep_to_op( endpoint_id, OPCMD_BUFFER_DETAILS,(void *)msg, &resp_id, \
                                   (void **)&resp) == STATUS_OK)
    {
        /* Analyse and free the response*/
        buff_details->supplies_buffer = resp->supplies_buffer;
        buff_details->can_override  = resp->can_override;
        buff_details->wants_override = resp->needs_override;
        buff_details->runs_in_place = resp->runs_in_place;
#ifdef INSTALL_METADATA
        buff_details->supports_metadata = resp->supports_metadata;
        if (resp->supports_metadata)
        {
            buff_details->metadata_buffer = resp->metadata_buffer;
        }
#endif /* INSTALL_METADATA */
        if (resp->supplies_buffer)
        {
            buff_details->b.buffer = resp->b.buffer;
        }
        else if (resp->runs_in_place)
        {
            /* Operator can run in place.  Set the in_place_buff_params from the response. */
            buff_details->b.in_place_buff_params.in_place_terminal = resp->b.in_place_buff_params.in_place_terminal;
            buff_details->b.in_place_buff_params.size = resp->b.in_place_buff_params.size;
            buff_details->b.in_place_buff_params.buffer = resp->b.in_place_buff_params.buffer;
        }
        else
        {
            buff_details->b.buff_params.flags = BUF_DESC_SW_BUFFER;
            buff_details->b.buff_params.size = resp->b.buffer_size ;
        }

        /* Free the response. */
        pfree(resp);
        return TRUE;
    }
    else
    {
        pfree(resp);
        return FALSE;
    }
}

/****************************************************************************
 *
 * opmgr_connect_buffer_to_endpoint
 *
 */
bool opmgr_connect_buffer_to_endpoint(unsigned int endpoint_id, tCbuffer *Cbuffer_ptr, unsigned int ep_id_to_kick)
{
    uintptr_t connect_msg[3];
    unsigned int *resp = NULL, resp_id;

    patch_fn_shared(opmgr);

    /* Connect the operator to the buffer */
    connect_msg[0] = get_op_ep_terminal_id(endpoint_id);
    connect_msg[1] = (uintptr_t)Cbuffer_ptr;
    connect_msg[2] = 0; /* TODO the endpoint/buffer depends on the type of connection been made */

    if (send_cmd_from_op_ep_to_op(endpoint_id, OPCMD_CONNECT,(void *)connect_msg, &resp_id, (void **)&resp) != STATUS_OK)
    {
        if (resp)
        {
            pfree(resp);
        }
        return FALSE;
    }

    pfree(resp);
    resp = NULL;

    if (ep_id_to_kick != 0)
    {
        if(!opmgr_kick_prop_table_add(endpoint_id, ep_id_to_kick))
        {
            /* Try to disconnect, otherwise the buffer in the operator remains set.
             * The connect message can be reused for the disconnect because the last
             * two word from the message will be ignored by the operator.
             * No error handling for the disconnect because this is the most what we can do. */
            send_cmd_from_op_ep_to_op(endpoint_id, OPCMD_DISCONNECT,(void *)connect_msg, &resp_id, (void **)&resp);

            if (resp)
            {
                pfree(resp);
            }
            return FALSE;
        }
    }
    return TRUE;
}

/****************************************************************************
 *
 * opmgr_disconnect_buffer_from_endpoint
 *
 */
bool opmgr_disconnect_buffer_from_endpoint (unsigned int endpoint_id)
{
    uintptr_t msg[1];
    unsigned int *resp = NULL, resp_id;

    /* Disconnect the operator from the buffer */
    msg[0] = get_op_ep_terminal_id(endpoint_id);;

    if (send_cmd_from_op_ep_to_op(endpoint_id, OPCMD_DISCONNECT,(void *)msg, &resp_id, (void **)&resp) == STATUS_OK)
    {
        /* Free the response*/
        pfree(resp);
        /* Stop kicking along this connection */
        opmgr_kick_prop_table_remove(endpoint_id);
        return TRUE;
    }
    else
    {
        /* Free the response*/
        pfree(resp);
        return FALSE;
    }
}

/****************************************************************************
 *
 * opmgr_get_data_format
 *
 */
AUDIO_DATA_FORMAT opmgr_get_data_format(unsigned int endpoint_id)
{
    AUDIO_DATA_FORMAT format = AUDIO_DATA_FORMAT_16_BIT;

    uintptr_t msg[1];
    unsigned int *resp = NULL, resp_id;

    patch_fn_shared(opmgr);

    PL_PRINT_P1(TR_OPMGR, "Opmgr: Get data format from ep 0x%x \n", endpoint_id);

    /* Get Data format of an operator endpoint. */
    msg[0] = get_op_ep_terminal_id(endpoint_id);

    if (send_cmd_from_op_ep_to_op(endpoint_id, OPCMD_DATA_FORMAT,(void *)msg, &resp_id, (void **)&resp) == STATUS_OK)
    {
        /* Extract data format from the response. */
        format = resp[2];
    }

    /* Free the response*/
    pfree(resp);
    return format;
}

/****************************************************************************
 *
 * opmgr_get_block_size
 *
 */
void opmgr_get_block_size(unsigned int endpoint_id, unsigned int *ep_block_size)
{
    uintptr_t msg[1];
    OP_SCHED_INFO_RSP *resp = NULL;
    unsigned int  resp_id;

    patch_fn_shared(opmgr);

    PL_PRINT_P1(TR_OPMGR, "Opmgr: Get data block size from ep 0x%x \n", endpoint_id);

    /* Get scheduling information of an operator endpoint. */
    msg[0] = get_op_ep_terminal_id(endpoint_id);;

    if (send_cmd_from_op_ep_to_op(endpoint_id, OPCMD_GET_SCHED_INFO,(void *)msg, &resp_id, (void **)&resp) == STATUS_OK)
    {
        /* The block size of the operator is packed in the scheduling information.
         * Extract block size from the response. */
        *ep_block_size = resp->block_size;
    }
    pfree(resp);
}

/****************************************************************************
 *
 * opmgr_get_sched_info
 *
 */
void opmgr_get_sched_info(unsigned int endpoint_id, unsigned *ep_block_size,
        unsigned *ep_period, bool *ep_locally_clocked, bool *ep_wants_kicks)
{
    uintptr_t msg[1];
    OP_SCHED_INFO_RSP *resp = NULL;
    unsigned int  resp_id;
    OPERATOR_DATA *cur_op;

    patch_fn_shared(opmgr);

    /* Get scheduling information of an operator endpoint. */
    msg[0] = get_op_ep_terminal_id(endpoint_id);;

    if (send_cmd_from_op_ep_to_op(endpoint_id, OPCMD_GET_SCHED_INFO,(void *)msg, &resp_id, (void **)&resp) == STATUS_OK)
    {
        /* The info is packed in the response, the run period is only valid if
         * block_size is 0, locally clocked only has any worth if the endpoint
         * is real, streams handles this in the level above. */
        *ep_block_size = resp->block_size;
        *ep_locally_clocked = resp->locally_clocked;

        if (resp->block_size == 0)
        {
            *ep_period = resp->run_period;
        }
        else
        {
            *ep_period = 0;
        }

    }
    pfree(resp);

    /* Wants kicks is determined by the operators stop_chain_kicks flag, this
     * may get set after connection which is more complicated to unwind, this
     * path however is simple to indicate here. */
    /* Look in local oplist, as kick propagation happens in graph
     * local to OpMgr and Stream that calls this func.
     * Shadowed 'real' EPs, not OP EPs will cross the IPC barrier.
     */
    cur_op = get_op_data_from_id(get_opid_from_opidep(endpoint_id));

    /* Check if the operator wants kicks at the side of the endpoint */
    if (get_is_sink_from_opidep(endpoint_id))
    {
        *ep_wants_kicks = ((cur_op->stop_chain_kicks & SINK_SIDE) == 0);
    }
    else /* source endpoint */
    {
        *ep_wants_kicks = ((cur_op->stop_chain_kicks & SOURCE_SIDE) == 0);
    }
}

/****************************************************************************
 *
 * opmgr_get_operator_ep_clock_id
 *
 */
unsigned opmgr_get_operator_ep_clock_id(unsigned int endpoint_id)
{
    OP_MSG_REQ msg;
    OP_OPMSG_RSP *resp;
    unsigned clock_id = 0;
    unsigned resp_id, terminal_id = get_op_ep_terminal_id(endpoint_id);

    msg.header.cmd_header.client_id = terminal_id;
    msg.header.cmd_header.length = sizeof(OP_MSG_REQ);
    msg.header.msg_id = OPMSG_COMMON_GET_CLOCK_ID;

    if (send_cmd_from_op_ep_to_op(endpoint_id, OPCMD_MESSAGE, (void*)&msg, &resp_id, (void **)&resp) == STATUS_OK)
    {
        clock_id = resp->payload[0];
    }

    pfree(resp);
    return clock_id;
}

/****************************************************************************
 *
 * opmgr_get_config_msg_to_operator
 *
 */
bool opmgr_get_config_msg_to_operator(unsigned int endpoint_id, unsigned int key, OPMSG_GET_CONFIG_RESULT* result)
{
    OPMSG_GET_CONFIG msg;
    OP_GET_CONFIG_RSP *resp = NULL;
    unsigned int resp_id, status, terminal_id = get_op_ep_terminal_id(endpoint_id);
    bool retval = FALSE;

    patch_fn_shared(opmgr);

    PL_PRINT_P1(TR_OPMGR, "Opmgr: Get data format from ep 0x%x \n", endpoint_id);

    /* Get the details of the operator endpoint. */
    msg.header.cmd_header.client_id = terminal_id;
    msg.header.cmd_header.length = sizeof(OPMSG_GET_CONFIG) - sizeof(OPCMD_MSG_HEADER); /* length of the remaining content */
    msg.header.msg_id = OPMSG_COMMON_GET_CONFIGURATION;
    msg.key = key;
    msg.result = result;/* pointer to the result */

    status = send_cmd_from_op_ep_to_op(endpoint_id, OPCMD_MESSAGE,(void *)&msg, &resp_id, (void **)&resp);
    if (status == STATUS_OK)
    {
        retval = TRUE;
    }

    /* Free the response.*/
    pfree(resp);
    return retval;
}


/****************************************************************************
 *
 * opmgr_config_msg_to_operator
 *
 */
bool opmgr_config_msg_to_operator(unsigned int endpoint_id, unsigned int key, uint32 value)
{
    OPMSG_CONFIGURE msg;
    OP_OPMSG_RSP *resp = NULL;
    unsigned int resp_id, status, terminal_id = get_op_ep_terminal_id(endpoint_id);

    patch_fn_shared(opmgr);

    PL_PRINT_P1(TR_OPMGR, "Opmgr: send configuration message to the operator ep 0x%x \n", endpoint_id);
    /* Set the data format of the opeator. */
    msg.header.cmd_header.client_id = terminal_id;
    msg.header.cmd_header.length = sizeof(OPMSG_CONFIGURE) - sizeof(OPCMD_MSG_HEADER); /* length of the remaining content */
    msg.header.msg_id = OPMSG_COMMON_CONFIGURE;
    msg.key = key;
    msg.value = value;/* value passed to the operator.*/

    status = send_cmd_from_op_ep_to_op(endpoint_id, OPCMD_MESSAGE,(void *)&msg, &resp_id, (void **)&resp);
    /* Free the response.*/
    pfree(resp);

    if (status == STATUS_OK)
    {
        return TRUE;
    }
    return FALSE;
}



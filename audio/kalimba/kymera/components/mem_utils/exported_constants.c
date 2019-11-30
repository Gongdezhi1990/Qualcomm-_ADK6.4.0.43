/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file 
 * \ingroup exp_const
 *
 * Crescendo platform implementation for exported constants.
 *
 * Exported constants are mapped via a fixed size array of constants.
 * Other than accessing the status of a constant (i.e. in RAM)
 * all updates are via messages and a task, which runs on processor
 * 0 only.
 *
 * Other than "kick" messages to itself, all messages are simply
 * de-queued rapidly and placed in a list.
 */

#include "exported_constants_private.h"

#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT


/**
 * Extended message used for sending callback details to MEM_UTILS_QUEUE
 *
 * Message ID: EXP_CONST_MSG_CALLBACK_ID
 */
typedef struct exp_const_msg_cb
{
    qid                     q;
    uint16                  id;
    void                   *msg;
    tRoutingInfo            rinfo;
} exp_const_msg_cb;

/**
 * Base message used for messages to MEM_UTILS_QUEUE
 *
 * The message has an additional "next" field used when maintaining
 * a queue of actions.
 */
typedef struct exp_const_msg
{
        /** No need to set when sending. Used as pointer when on queue */
    struct exp_const_msg   *next;
        /** No need to set when sending. ID is saved when adding to queue */
    exp_const_msgid         id;     
        /** Address of the constants table (in ROM/SQIF) that has been 
            exported */
    void                   *const_table;
#if defined(AUDIO_SECOND_CORE) || defined(INSTALL_DUAL_CORE_SUPPORT)
        /** Processor ID, populated in the KIP IPC process if needed */
    unsigned                proc;   
#endif
        /** Reference passed by the code loading constants. In the case of
            an operator, the operator data is sufficient */
    unsigned                operator;
        /** Optional structure defining callback, normally NULL */
    exp_const_msg_cb       *callback;
} exp_const_msg;


/** Configuration for the exported constants code.
 *
 * This is exchanged in the IPC lookup table if multiple processors are
 * used.
 */
typedef struct
{
    exp_const_mapping   *constants;
    unsigned            max_used_constants;
} EXP_CONST_IPC;

/** The number of constants *ever* used in a single execution of the 
 * code.
 *  This can be less than the total number known to exist as we don't
 * expect to use 1, 2, 3 & 4 mic solutions on the same device. 
 */
#define NUMBER_CONSTANTS_SUPPORTED 16

#if !defined(AUDIO_SECOND_CORE) || defined(INSTALL_DUAL_CORE_SUPPORT)
/** Start of linked list with actions to be taken. */
static exp_const_msg *activity_queue = NULL;

/** Flag indicating if we are waiting for a file */
static bool exp_const_waiting_for_file = FALSE;

/** The table for keeping track of exported constants that have been referenced */
static DM_P0_RW_ZI exp_const_mapping exp_const_mapping_table[NUMBER_CONSTANTS_SUPPORTED];
#endif

/* The configuration for this instance of exported constants.
 *
 * This is pre-filled for P0, and populated via IPC on the other processors
 */
static EXP_CONST_IPC exp_const_config = {   exp_const_mapping_table,
                                            NUMBER_CONSTANTS_SUPPORTED
                                        };

#define FOR_USED_CONSTANTS(ix)    for (ix = 0;\
                                                ix < exp_const_config.max_used_constants\
                                             && exp_const_config.constants[ix].const_in_ROM;\
                                             ix++)

/****************************************************************************
Private function definitions
*/

#if defined(AUDIO_SECOND_CORE) || defined(INSTALL_DUAL_CORE_SUPPORT)

/** 
 * Send a message using KIP to another processor.
 *
 * Most messages should be sent to 0, but when file transfers have completed 
 * the task on P0 will send a kip message to the initiating processor to 
 * issue a callback.
 *
 * \param proc          Processor to send to
 * \param id            Our internal message ID to place in the KIP
 * \param const_table   ROM reference of the table that we are loading
 * \param operator      Reference to the operator to place in the kip
 * \param callback      Anonymous pointer sent to p0 and in time returned
 */
static void send_exp_const_kip_msg(unsigned proc,exp_const_msgid id,void *const_table,unsigned operator,void *callback)
{
    uint16 con_id = PACK_CON_ID(PACK_SEND_RECV_ID(hal_get_reg_processor_id(),0),PACK_SEND_RECV_ID(proc,0));
    uint16 kip_msg[KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_WORD_SIZE] 
                   = {KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CREATE(con_id,
                                                                   id,
                                                         (unsigned)const_table,
                                                                   operator,
                                                         (unsigned)callback)};

    kip_adaptor_send_message(proc, KIP_MSG_ID_EXPORTED_CONSTANTS_ACTION_REQ,
                                   KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_WORD_SIZE,
                                   (unsigned*)&kip_msg[0],NULL);
}

#endif

/** 
 * Send a message to the exported constants task.
 *
 * This deals with inter-process messaging if required, using KIP if we are not
 * being called on processor 0.
 *
 * If sending a message locally, the message is allocated here, and freed once
 * processed at the end of process_activity_queue().
 *
 * \param id            Our internal message ID 
 * \param const_table   ROM reference of the table that we are loading
 * \param operator      Reference to the operator 
 * \param callback      Anonymous pointer holding callback information
 */
static void send_message_to_task(exp_const_msgid id, void *const_table, unsigned operator, exp_const_msg_cb *cb_param)
{
    patch_fn_shared(exp_const);

    if (KIP_PRIMARY_CONTEXT())
    {
        exp_const_msg *msg = zpnew(exp_const_msg);

        msg->const_table = const_table;
        msg->operator = operator;
        msg->callback = cb_param;
#if defined(AUDIO_SECOND_CORE) || defined(INSTALL_DUAL_CORE_SUPPORT)
        msg->proc = hal_get_reg_processor_id();
#endif
        put_message(MEM_UTILS_QUEUE, id, msg);
    }
#if defined(AUDIO_SECOND_CORE) || defined(INSTALL_DUAL_CORE_SUPPORT)
    else
    {
        send_exp_const_kip_msg(0,id,const_table,operator,cb_param);
    }
#endif
}

/**
 * Helper function to initiate a callback on a different processor using KIP.
 *
 * \param proc              Target processor
 * \param callback_struct   Anonymous pointer as originally passed to us
 */
static void send_callback_over_kip(unsigned proc,void *callback_struct)
{
    send_exp_const_kip_msg(proc,EXP_CONST_MSG_CALLBACK_ID,NULL,(unsigned)-1,callback_struct);
}


#if defined(INSTALL_DUAL_CORE_SUPPORT)

/**
 * Function to handle KIP messages for the exported constant code, as received 
 * by P0.
 *
 * The parameter msg_data is only valid for the life of the function call, so we 
 * extract values from it before sending to the exported constants task.
 *
 * The message allocated here is handled in the same way as a message created in
 * send_message_to_task() and will be freed once processed at the end of 
 * process_activity_queue().
 *
 * \param length    length of the kip message
 * \param msg       pointer to the kip message received
 */
void external_constant_handle_kip_p0(unsigned length,uint16 *msg_data)
{
    KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ *kip_msg = (KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ *)msg_data;
    exp_const_msg *msg = zpnew(exp_const_msg);

    unsigned con_id = KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CON_ID_GET(kip_msg);
    uint16 id = KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_MI_GET(kip_msg);
    void *const_table = (void*)KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CONSTANT_P_GET(kip_msg);
    unsigned operator = KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_OPERATOR_ID_GET(kip_msg);
    void *cb_param = (void*)KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CALLBACK_INFO_GET(kip_msg);

    msg->const_table = const_table;
    msg->operator = operator;
    msg->callback = cb_param;
    msg->proc = GET_SEND_PROC_ID(con_id);
    put_message(MEM_UTILS_QUEUE,id,msg);
}
#endif

/**
 * Function to handle KIP messages for the exported constant code, as received 
 * on processors other than P0.
 *
 * Once the callback has been issued we free the callback pointer.
 *
 * \param length    length of the kip message
 * \param msg       pointer to the kip message received
 */
#if defined(AUDIO_SECOND_CORE)
void external_constant_handle_kip_px(unsigned length,uint16 *msg_data)
{
    KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ *kip_msg = (KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ *)msg_data;
    uint16 id = KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_MI_GET(kip_msg);

    PL_ASSERT(id == EXP_CONST_MSG_CALLBACK_ID);

    exp_const_msg_cb *cb = (exp_const_msg_cb*)KIP_MSG_EXPORTED_CONSTANTS_ACTION_REQ_CALLBACK_INFO_GET(kip_msg);

    put_message_with_routing(cb->q, cb->id,cb->msg, &cb->rinfo);
    pfree(cb);
}
#endif

/** 
 * Find a mapping entry for the requested constant in ROM
 *
 * Scans the list to find a matching entry, returning NULL if not found
 *
 * \param dynamic_table     Pointer to the constant table in ROM/SQIF
 *
 * \returns pointer to the mapping entry or NULL
 */
static exp_const_mapping *find_const_mapping(void *dynamic_table)
{
    unsigned index;

    FOR_USED_CONSTANTS(index)
    {
        if (exp_const_config.constants[index].const_in_ROM == dynamic_table)
        {
            return &exp_const_config.constants[index];
        }
    }
    return NULL;
}

/** 
 * Returns a mapping entry for the requested constant in ROM, creating if needed.
 *
 * If there is not already a matching entry, creates one
 *
 * \param dynamic_table     Pointer to the constant table in ROM/SQIF
 *
 * \returns pointer to the mapping entry
 */
static exp_const_mapping *find_or_add_const_mapping(void *dynamic_table)
{
    unsigned index;

    FOR_USED_CONSTANTS(index)
    {
        if (exp_const_config.constants[index].const_in_ROM == dynamic_table)
        {
            return &exp_const_config.constants[index];
        }
    }

    if (index == exp_const_config.max_used_constants)
    {
        /* Shouldn't occur in production as a single build won't use,
         * for example, CVC set-ups with 2, 3 & 4 microphones. 
         * If we don't panic then the addociated OperatorCreate will 
         * fail yet, after a restart, it would work.
         */
        panic(PANIC_AUDIO_TOO_MANY_DIFFERENT_CONSTANTS);
    }
    exp_const_config.constants[index].const_in_ROM = dynamic_table;
    return &exp_const_config.constants[index];
}

/**
 * Vary the size of the array used to store users of an exported constant.
 *
 * Checks the size of the current block and creates a larger one if required.
 * If the size could be reduced, we just return the existing block.
 *
 * The memory allocated here is either freed on a subsequent call (if re-sizing)
 * or released in remove_const_usage.
 *
 * \param old_block     Pointer to the old block
 * \param new_entries   Number of new entries in the block.
 *
 * \returns Pointer of the correct size. Failure to re-size will Panic
 */
static unsigned *realloc_uses_array(unsigned *old_block,unsigned new_entries)
{
    unsigned new_size = new_entries * sizeof(void*);
    unsigned old_size = psizeof(old_block);

    if (new_size > old_size)
    {
        unsigned *new_block = zpmalloc(new_size);
        memcpy(new_block,old_block,old_size);
        pfree(old_block);
        return new_block;
    }

    /* Never bother reducing size */
    return old_block;
}

/**
 * Add a user of the exported constant
 *
 * We do not handle nesting, so if function calls happen to be nested there
 * is a chance that the constant could be removed and then used.
 *
 * \param ecmap     Pointer to the map entry
 * \param the_user  Pointer identifying the user of the entry
 */
static void add_const_usage(exp_const_mapping *ecmap,unsigned the_user)
{
    unsigned i;

    patch_fn_shared(exp_const);

    for (i=0; i < ecmap->num_uses; i++)
    {
        if (ecmap->uses[i] == the_user)
        {
            L4_DBG_MSG2("mem_util/exp_const: Constant requested multiple times for same use. const:%p user:%p",ecmap->const_in_ROM,the_user);
            return;
        }
    }
    void *new_map = realloc_uses_array(ecmap->uses,ecmap->num_uses + 1);
    ecmap->uses = new_map;
    ecmap->uses[ecmap->num_uses++] = the_user;
}

/**
 * Remove a user of the exported constant
 *
 * We do not handle nesting, so if function calls happen to be nested there
 * is a chance that the constant could be removed and then used.
 *
 * \param ecmap     Pointer to the map entry
 * \param the_user  Pointer identifying the user of the entry
 */
static void remove_const_usage(exp_const_mapping *ecmap,unsigned the_user)
{
    unsigned i;

    for (i=0; i < ecmap->num_uses; i++)
    {
        if (ecmap->uses[i] == the_user)
        {
            /* Just copy the last entry in array to the current, matching,
             * entry. At worst we copy ourselves. */
            ecmap->uses[i] = ecmap->uses[--ecmap->num_uses];
            if (ecmap->num_uses)
            {
                ecmap->uses = realloc_uses_array(ecmap->uses,ecmap->num_uses);
            }
            else
            {
                pfree(ecmap->uses);
                ecmap->uses = NULL;
                /* Don't clear the one fatal error status */
                switch (ecmap->status)
                {
                    case EXP_CONST_STATUS_ERROR:
                        /* Fatal error state, leave unchanged */
                        break;

                    case EXP_CONST_STATUS_ERROR_MEMORY:
                        ecmap->status = EXP_CONST_STATUS_EXISTS;
                        break;

                    case EXP_CONST_STATUS_ERROR_SESSION:
                        ecmap->status = EXP_CONST_STATUS_UNKNOWN;
                        break;

                    default:
                        /* Deletable state will free RAM (if any) and
                         * reset the state to exists */
                        ecmap->status = EXP_CONST_STATUS_DELETABLE;
                }
            }
            return;
        }
    }
    L2_DBG_MSG2("mem_util/exp_const: Constant 'released' but not in use. const:%p user:%p",
                ecmap->const_in_ROM,the_user);
}

/**
 * Check for a specific user of a constant
 *
 * Match the user against the defines uses
 *
 * \param ecmap     Pointer to the map entry
 * \param the_user  Pointer identifying the user of the entry
 *
 * \returns TRUE if user is in the list, FALSE otherwise
 */
static bool constant_reserved_for_operator(exp_const_mapping *ecmap,unsigned operator_id)
{
    unsigned i;

    for (i=0; i < ecmap->num_uses; i++)
    {
        if (ecmap->uses[i] == operator_id)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/**
 * Check if the referenced table is in ROM/SQIF rather than exported 
 *
 * \param source    Address of table in ROM to be checked for an export.
 */
static bool is_constant_onchip(void *source)
{
    return (DynLoaderHasExternalRedirect(source) == FALSE);
}

/**
 * Add the message to end of activity list.
 *
 * Only iterate over the list as expected to be short, and infrequently used
 *
 * \param   msg     Message to add
 */
static void add_to_activity_queue(exp_const_msg *msg)
{
    if (!activity_queue)
    {
        activity_queue = msg;
    }
    else
    {
        exp_const_msg *p=activity_queue;

        while (p->next)
        {
            p = p->next;
        }
        p->next = msg;
    }
}

/** 
 * Check the mapped constants to see if any can be released from RAM
 *
 * This function should only be called when the queue of messages
 * to be processed is empty, ensuring no conflicts.
 */
static void free_unused_memory(void)
{
    unsigned index;

    FOR_USED_CONSTANTS(index)
    {
        if (exp_const_config.constants[index].status == EXP_CONST_STATUS_DELETABLE)
        {
            /* DEBUG ONLY - get the size of the block to wipe it when freed */
            unsigned size = psizeof(exp_const_config.constants[index].ram_copy);
            
            L3_DBG_MSG2("mem_util/exp_const: Releasing RAM for table @%p (entry %u)",
                        exp_const_config.constants[index].const_in_ROM, index);

            exp_const_config.constants[index].status = EXP_CONST_STATUS_EXISTS;
            
            /* DEBUG ONLY - wipe block so use after free should be obvious */
            memset(exp_const_config.constants[index].ram_copy, 0xaa, size);

            pfree(exp_const_config.constants[index].ram_copy);
            exp_const_config.constants[index].ram_copy = NULL;
        }
    }
}

/**
 * Process the first item in the queued messages.
 *
 * If a file transfer is in progress, then no action is taken, we expect a 
 * callback which will unblock us.
 */
static void process_activity_queue(void)
{
    exp_const_msg      *current=activity_queue;
    exp_const_msg_cb   *cb_msg;
    exp_const_mapping  *ecmap;
    bool                kick_needed_regardless = FALSE;

    patch_fn_shared(exp_const);

    if (exp_const_waiting_for_file)
    {
        L4_DBG_MSG1("mem_util/exp_const: Not processing queue, BUSY. Head %p",current);
        return;
    }

    if (!current)
    {
        L4_DBG_MSG("mem_util/exp_const: No queue. Checking deletions");
        free_unused_memory();
        return;
    }

    L2_DBG_MSG1("mem_util/exp_const: Request at head of queue is %d",current->id);

    switch (current->id)
    {
    case EXP_CONST_MSG_ADDREF_ID:
        /* Does the constant table exist in our mapping table */
        ecmap = find_or_add_const_mapping(current->const_table);
        add_const_usage(ecmap, current->operator);

        switch (ecmap->status)
        {
        case EXP_CONST_STATUS_UNKNOWN:
        case EXP_CONST_STATUS_EXISTS:
        case EXP_CONST_STATUS_LOADING:
            exp_const_load_external_table(ecmap);
            exp_const_waiting_for_file = TRUE;
            break;

        case EXP_CONST_STATUS_DELETABLE:
            /* We managed to get in before deleted */
            assert(ecmap->ram_copy);
            ecmap->status = EXP_CONST_STATUS_IN_RAM;
            break;

        /* EXP_CONST_STATUS_IN_RAM - have added reference, continue */
        /* EXP_CONST_STATUS_ERROR_xxx - shouldn't be called */
        default:
            /* Nothing to do, drop out and free the entry */
            break;
        }
        break;

    case EXP_CONST_MSG_RELEASE_ID:
        ecmap = find_const_mapping(current->const_table);
        if (ecmap)
        {
            remove_const_usage(ecmap, current->operator);
            /* If the entry is now deletable, want to make sure that
             * we are called again. */
            if (ecmap->status == EXP_CONST_STATUS_DELETABLE)
            {
                kick_needed_regardless = TRUE;
            }
        }
        break;

    default:
        break;

    case EXP_CONST_MSG_CALLBACK_ID:
        /* When we get to process this message, we call the callback regardless.
         * If the table isn't available then the operator is expected to die
         * (or error) as appropriate */
#if defined(AUDIO_SECOND_CORE) || defined(INSTALL_DUAL_CORE_SUPPORT)
        /* The callback needs to use KIP if it was not requested from P0 */
        if (current->proc != hal_get_reg_processor_id())
        {
            send_callback_over_kip(current->proc,current->callback);
            break;
        }
        /* Check that we are on P0, as we are about to send a local message */
        PL_ASSERT(KIP_PRIMARY_CONTEXT());
#endif
        cb_msg = current->callback;
        put_message_with_routing(cb_msg->q, cb_msg->id,cb_msg->msg, &cb_msg->rinfo);
        pfree(cb_msg);
        break;
    }

    /* Remove the head of the queue */
    activity_queue = current->next;
    pfree(current);

    /* We now exit the task handler, sending ourselves a message to schedule
     * the task again (if needed) */
    if (activity_queue || kick_needed_regardless)
    {
        put_message(MEM_UTILS_QUEUE, EXP_CONST_MSG_KICK_ID, NULL);
    }

}

/****************************************************************************
Public function definitions
*/

/*
 * Basic task initialisation function, needed by all static tasks
 *
 * Initialise our file transfer code.
 *
 * \param task_data Task context (unused)
 */
void mem_utils_task_init(void **data)
{
    UNUSED(data);

    patch_fn_shared(exp_const);

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
    /* For P0 get and populate the shared address in the LUT
     * and for Pn get it from the LUT
     */ 
    if( hal_get_reg_processor_id() != 0) 
    {
        EXP_CONST_IPC *lookup;

        if(ipc_get_lookup_addr(IPC_LUT_ID_EXPORTED_CONSTANTS,
                              (uintptr_t*)&lookup) != IPC_SUCCESS)
        {
            panic_diatribe(PANIC_IPC_LUT_CORRUPTED, IPC_LUT_ID_EXPORTED_CONSTANTS); 
        }
        exp_const_config = *lookup;
    }
    else
#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */
    {
        if(ipc_set_lookup_addr(IPC_LUT_ID_EXPORTED_CONSTANTS, 
                               (uintptr_t)&exp_const_config) != IPC_SUCCESS )
        {
            panic_diatribe(PANIC_IPC_LUT_CORRUPTED, IPC_LUT_ID_EXPORTED_CONSTANTS); 
        }

        /* We only need to start the file transfer on P0 */
        exp_const_file_transferer_init();
    }
}


/**
 * queue handler function for the mem_utils task
 *
 * The handling of exported constants relies on handling requests in 
 * order and working asynchronously with the FTP file transfer and the
 * callbacks registed on us.
 *
 * To achieve this, all messages are de-queued in a tight loop and 
 * added to an activity list. When all messages have been processed the
 * activity list is processed, that handler arranging to send a message
 * if needed.
 *
 * When the asynchronous file transfer completes we are sent a message
 * to wake us, and process the next entry in the activity queue which was
 * blocked until now.
 *
 * \param task_data Task context (unused)
 */
void mem_utils_task(void ** task_data)
{
    uint16          id;
    exp_const_msg  *msg;

    patch_fn_shared(exp_const);

    while (get_message(MEM_UTILS_QUEUE,&id, (void**)&msg))
    {
        L3_DBG_MSG2("mem_util/exp_const: New message id:%d msg:%p",id,msg);

        switch (id)
        {
        case EXP_CONST_MSG_KICK_ID: /* NULL message to wake us */
            break;

        /* Message from file handling code */
        case EXP_CONST_MSG_TRANSFER_OVER_KICK_ID:
            L2_DBG_MSG("mem_util/exp_const: Transfer completed");

            exp_const_waiting_for_file = FALSE;
            break;

        default:
            /* Populate msg fields for use in linked list */
            msg->next = NULL;
            msg->id = (exp_const_msgid)id;

            /* We want to handle messages in order, but as we are waiting 
             * for the file server to get back to us in some cases, we just
             * take all messages off the queue and add them to our own activity
             * list. */
            add_to_activity_queue(msg);
            break;
        }
    }

    process_activity_queue();

    UNUSED(task_data);
}


/* Update the passed descriptor to point to the equivalent table in RAM (if needed) */
bool external_constant_map_descriptor(const_data_descriptor * start)
{
    exp_const_mapping   *map;

    L4_DBG_MSG1("mem_util/exp_const: attempting to access_descriptor @ %p",start);

    map = find_const_mapping(start->address);

    /* We don't have a mapping. 
       If it turns out to be a mapped table, then it's too late to do 
       anything about it */
    if (!map)
    {
        return TRUE;
    }

    if (map->status == EXP_CONST_STATUS_IN_RAM)
    {
        /* We are mapped, so update the data descriptor to point 
           at the copied memory */
        start->address = map->ram_copy;
        L4_DBG_MSG1("mem_util/exp_const: local copy in RAM @ %p",start->address);
    }
    else
    {
        L3_DBG_MSG1("mem_util/exp_const: Not in RAM (status %d)",map->status);
        return FALSE;
    }

    return TRUE;
}

/* Reserve the requested external_constant */
bool external_constant_reserve( void *source , unsigned operator)
{
    L2_DBG_MSG2("external_constant_reserve(0x%x, 0x%x)", source, operator);

    if (is_constant_onchip(source))
    {
        return TRUE;
    }

    /* Does the constant table exist in our mapping table */
    exp_const_mapping *ecmap = find_const_mapping(source);
    if (ecmap)
    {
        if (ecmap->status >= EXP_CONST_STATUS_ERROR)
        {
            return FALSE;
        }
        if (constant_reserved_for_operator(ecmap,operator))
        {
            /* We have a reservation already, quit to avoid re-requesting.
             * This will/may need to come out if we start supporting requests
             * when an operator is running, e.g. when updating settings
             */
            return TRUE;
        }
    }

    send_message_to_task(EXP_CONST_MSG_ADDREF_ID, source, operator, NULL);
    return TRUE;

}

/* Release a reservation on the requested external_constant */
void external_constant_release( void *source , unsigned operator)
{
    L2_DBG_MSG2("external_constant_release(0x%x, 0x%x)",source,operator);

    send_message_to_task(EXP_CONST_MSG_RELEASE_ID, source, operator, NULL);
}

/* Check if the constant table required is accessible */
bool is_external_constant_available ( void *source, unsigned operator )
{
    exp_const_mapping *ecmap;

    L2_DBG_MSG2("is_external_constant_avail(0x%x, 0x%x)", source, operator);

    if (is_constant_onchip(source))
    {
        return TRUE;
    }

    ecmap = find_const_mapping(source);
    if (ecmap && (ecmap->status == EXP_CONST_STATUS_IN_RAM))
    {
        return constant_reserved_for_operator(ecmap,operator);
    }

    /* All other cases return FALSE, in the expectation that a 
     * callback will be registered */
    return FALSE;
}

/* Queue a message to be sent when the constants previously reserved are available 
 *
 * The callback information structure allocated here (exp_const_msg_cb) is released 
 * either in external_constant_handle_kip_px(), which processes the pointer 
 * being returned from P0, or in the function process_activity_queue() when 
 * processing the message EXP_CONST_MSG_CALLBACK_ID.
 */
void external_constant_callback_when_available(void *source,qid q,uint16 cmdId,void *cmdMsg,tRoutingInfo *rinfo)
{
    exp_const_msg_cb *cb = pnew(exp_const_msg_cb);

    L2_DBG_MSG1("external_constant_callback_when_available(0x%x)",source);

    cb->q =q;
    cb->id = cmdId;
    cb->msg = cmdMsg;
    cb->rinfo = *rinfo;
    send_message_to_task(EXP_CONST_MSG_CALLBACK_ID, source, (unsigned)-1, cb);
}

#endif /* INSTALL_CAPABILITY_CONSTANT_EXPORT */


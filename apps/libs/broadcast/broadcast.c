/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast.c
  @brief Implementation of the external interface to the broadcast library.
*/

#include "broadcast_private.h"
#include "broadcast_msg_client.h"
#include "broadcast_tx.h"

#include <panic.h>
#include <connection.h>
#include <message.h>

/*! API validation macro
    Determine if the broadcast instance reference passed in is valid.
 */
#define VALIDATE_INSTANCE(x)            if (!(x)) \
                                            return broadcast_bad_instance; \
                                        if (!broadcast_is_valid_instance(x)) \
                                            return broadcast_bad_instance; 

/*! API validation macro
    Determine if the role is correct for the broadcast instance.
 */
#define VALIDATE_RECEIVER_ROLE(x)       if ((x)->role != broadcast_role_receiver) \
                                            return broadcast_bad_role;

/*! API validation macro
    Determine if the role is correct for the broadcast instance.
 */
#define VALIDATE_BROADCASTER_ROLE(x)    if ((x)->role != broadcast_role_broadcaster) \
                                            return broadcast_bad_role;

/*! API validation macro
    Determine if the broadcast mode is correct for the broadcast instance.
 */
#define VALIDATE_RECEIVER_MODE(x)       if (x == broadcast_mode_broadcast_only) \
                                            return broadcast_bad_mode;

/* Global pointer to the library core, will persist throughout library
 * instance create/destroy to maintain link to GATT Broadcast Service
 * Server. */
broadcast_lib_core* lib_core = NULL;

/*! @brief Message handler for the Broadcast Library Core.
 
    SINGLE

    At present the only entity that will have a handle to the core is the
    single instance of the GATT Broadcast Service Service. It also currently
    does not send any messages to it's client task (the lib_core task).
    
    If we implement any messages from the GATT Broadcast Service Server to
    it's client task, we'll need to forward them in here to the appropriate
    instance of the Broadcast library in the Broadcaster role.
*/
static void broadcast_lib_core_msg_handler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(id);
    UNUSED(message);

    BROADCAST_DEBUG(("BL:CORE: Message RX with ID:%x\n", id));
}

/*!
    @brief Handler for Broadcast library messages.
*/
static void broadcast_msg_handler(Task task, MessageId id, Message message)
{
    /* work out which library instance this is for */
    BROADCAST* broadcast = broadcast_identify_instance(task);

    /* Broadcaster or Receiver specific messages or connection library
     * CSB-related messages go into the TX and RX state machines */
    if (    ((id >= BROADCAST_INTERNAL_MSG_BASE) &&
             (id <= BROADCAST_INTERNAL_RX_TIMEOUT_RES))
         || ((id >= CL_CSB_SET_RESERVED_LT_ADDR_CFM) &&
             (id <= CL_CSB_CHANNEL_MAP_CHANGE_IND))
       )
    {
        /* pass message/event in to the correct state machine for the role */
        if (IS_BROADCASTER_ROLE(broadcast))
        {
            broadcastTxHandleEvent(broadcast, id, message);
        }
        else
        {
            broadcastRxHandleEvent(broadcast, id, message);
        }
    }
    else
    {
        BROADCAST_DEBUG(("BL: Unknown message received 0x%x\n", id));
    }
}

/*!
    @brief Create an instance of the Broadcast library.
*/
void BroadcastInit(Task app_task, broadcast_role role)
{
    BROADCAST* new_lib = NULL;

    /* create the core if it doesn't exist, first time a library
     * instance is created. */
    if (!lib_core)
    {
        lib_core = PanicUnlessMalloc(sizeof(*lib_core));
        memset(lib_core, 0, sizeof(*lib_core));
        lib_core->core_task.handler = broadcast_lib_core_msg_handler;
        BROADCAST_DEBUG(("BL:CORE: Initialised, allocated %d words\n", sizeof(*lib_core)));
    }

    /* ensure any previous instance is fully destroyed, we support
     * switching roles, but still only one instance at a time */
    if (lib_core->lib_instances_head)
    {
        /* still something in the instances list, should be NULL */
        BROADCAST_DEBUG(("BL: Initialise failed, still destroying\n"));
        broadcast_msg_client_init_cfm(app_task, NULL, broadcast_init_failed);
        return;
    }

    /* create and setup a new broadcast library instance */
    new_lib = PanicUnlessMalloc(sizeof(*new_lib));
    if (new_lib)
    {
        memset(new_lib, 0, sizeof(*new_lib));
        new_lib->lib_task.handler = broadcast_msg_handler;
        new_lib->client_task = app_task;
        new_lib->role = role;

        /* initialise the role specific state machine */
        if (role == broadcast_role_broadcaster)
        {
            new_lib->config.broadcaster.current_state = broadcastTxStateInit;
        }
        else
        {
            new_lib->config.receiver.current_state = broadcastRxStateInit;
        }


        /* get Connection library CSB messages delivered to this instance */
        ConnectionCsbRegisterTask(&new_lib->lib_task);
        /* SINGLE
         * At some point if we support multiple concurrent broadcaster/receiver
         * instances, we'll need to register the lib_core with the connection
         * library, and forward any received messages to the appropriate
         * instance in broadcast_lib_core_msg_handler().
         */

        /* remember this instance */
        new_lib->next_instance = lib_core->lib_instances_head;
        lib_core->lib_instances_head = new_lib;

        BROADCAST_DEBUG(("BL: Initialised, allocated %d words\n", sizeof(BROADCAST)));

        /* tell the client */
        broadcast_msg_client_init_cfm(app_task, new_lib, broadcast_success);
    }
    else
    {
        BROADCAST_DEBUG(("BL: Initialise Failed\n"));
        broadcast_msg_client_init_cfm(app_task, NULL, broadcast_init_failed);
    }
}

/*!
    @brief Destroy a broadcast library instance.
*/
broadcast_status BroadcastDestroy(BROADCAST* broadcast)
{
    VALIDATE_INSTANCE(broadcast);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_DESTROY_T);
        message->broadcast = broadcast;
        /* wait on the lock so we don't attempt a destroy operation until we're 
         * out of any transition state */
        MessageSendConditionally(&broadcast->lib_task, BROADCAST_INTERNAL_DESTROY,
                                 message, &broadcast->lock);
        return broadcast_success;
    }
}

/*!
    @brief Configure the Broadcast library for a broadcast role.
*/
broadcast_status BroadcastConfigureBroadcaster(BROADCAST* broadcast,
                                   ltaddr lt_addr,
                                   broadcaster_csb_params* csb_params,
                                   broadcaster_sync_params* sync_params)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_BROADCASTER_ROLE(broadcast);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_CONFIG_BROADCASTER_T);
        message->broadcast = broadcast;
        message->addr = lt_addr;
        message->csb_config = csb_params;
        message->sync_config = sync_params;
        MessageSendConditionally(&broadcast->lib_task, BROADCAST_INTERNAL_CONFIG_BROADCASTER,
                message, &broadcast->lock);
        return broadcast_success;
    }
}

/*!
    @brief Start a broadcast.
*/
broadcast_status BroadcastStartBroadcast(BROADCAST* broadcast, broadcast_mode start_mode)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_BROADCASTER_ROLE(broadcast);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_START_BROADCAST_T);
        message->broadcast = broadcast;
        message->mode = start_mode;
        message->sync_train_confirm = TRUE;
        MessageSendConditionally(&broadcast->lib_task, BROADCAST_INTERNAL_START_BROADCAST,
                                 message, &broadcast->lock);
        return broadcast_success;
    }
}

/*!
    @brief Stop a broadcast.
*/
broadcast_status BroadcastStopBroadcast(BROADCAST* broadcast, broadcast_mode stop_mode)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_BROADCASTER_ROLE(broadcast);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_STOP_BROADCAST_T);
        message->broadcast = broadcast;
        message->mode = stop_mode;
        MessageSendConditionally(&broadcast->lib_task, BROADCAST_INTERNAL_STOP_BROADCAST,
                                 message, &broadcast->lock);
        return broadcast_success;
    }
}

/*!
    @brief Respond to broadcaster transmission timeout.
*/
broadcast_status BroadcastBroadcasterTimeoutResponse(BROADCAST* broadcast)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_BROADCASTER_ROLE(broadcast);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_TX_TIMEOUT_RES_T);
        message->broadcast = broadcast;
        MessageSend(&broadcast->lib_task, BROADCAST_INTERNAL_TX_TIMEOUT_RES, message);
        return broadcast_success;
    }
}

/*!
    @brief Configure a broadcast receiver.
*/
broadcast_status BroadcastConfigureReceiver(BROADCAST* broadcast,
                                receiver_sync_params* sync_params)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_RECEIVER_ROLE(broadcast);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_CONFIG_RECEIVER_T);
        message->broadcast = broadcast;
        message->sync_config = sync_params;
        MessageSendConditionally(&broadcast->lib_task, BROADCAST_INTERNAL_CONFIG_RECEIVER,
                                 message, &broadcast->lock);
        return broadcast_success;
    }
}

/*!
    @brief Configure a broadcast receiver out-of-band.
*/
broadcast_status BroadcastConfigureReceiverOOB(BROADCAST* broadcast,
                                   bdaddr bd_addr, ltaddr lt_addr,
                                   receiver_csb_params* csb_params)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_RECEIVER_ROLE(broadcast);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_CONFIG_RECEIVER_OOB_T);
        message->broadcast = broadcast;
        message->bt_addr = bd_addr;
        message->lt_addr = lt_addr;
        message->csb_config = csb_params;
        MessageSendConditionally(&broadcast->lib_task, BROADCAST_INTERNAL_CONFIG_RECEIVER_OOB,
                                 message, &broadcast->lock);
        return broadcast_success;
    }
}

/*!
    @brief Start a broadcast receiver.
*/
broadcast_status BroadcastStartReceiver(BROADCAST* broadcast, broadcast_mode start_mode,
                            uint16 supervision_timeout)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_RECEIVER_ROLE(broadcast);
    VALIDATE_RECEIVER_MODE(start_mode);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_START_RECEIVER_T);
        message->broadcast = broadcast;
        message->mode = start_mode;
        message->timeout = supervision_timeout;
        MessageSendConditionally(&broadcast->lib_task, BROADCAST_INTERNAL_START_RECEIVER,
                                 message, &broadcast->lock);
        return broadcast_success;
    }
}

/*!
    @brief Stop a broadcast receiver.
*/
broadcast_status BroadcastStopReceiver(BROADCAST* broadcast)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_RECEIVER_ROLE(broadcast);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_STOP_RECEIVER_T);
        message->broadcast = broadcast;
        MessageSendConditionally(&broadcast->lib_task, BROADCAST_INTERNAL_STOP_RECEIVER,
                                 message, &broadcast->lock);
        return broadcast_success;
    }
}

/*!
    @brief Respond to receiver timeout.
*/
broadcast_status BroadcastReceiverTimeoutResponse(BROADCAST* broadcast)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_RECEIVER_ROLE(broadcast);
    {
        MESSAGE_MAKE(message, BROADCAST_INTERNAL_RX_TIMEOUT_RES_T);
        message->broadcast = broadcast;
        MessageSend(&broadcast->lib_task, BROADCAST_INTERNAL_RX_TIMEOUT_RES, message);
        return broadcast_success;
    }
}

/*!
    @brief Determine if a broadcast is currently active.
*/
broadcast_status BroadcastIsBroadcasting(BROADCAST* broadcast)
{
    VALIDATE_INSTANCE(broadcast);

    if (broadcast->role == broadcast_role_broadcaster)
    {
        if ((broadcast->config.broadcaster.current_state == broadcastTxStateCsb) ||
            (broadcast->config.broadcaster.current_state == broadcastTxStateCsbSyncTrain))
            return broadcast_is_active;
    }
    else
    {
        if ((broadcast->config.receiver.current_state == broadcastRxStateSyncTrain) ||
            (broadcast->config.receiver.current_state == broadcastRxStateSyncTrainCsbRx))
            return broadcast_is_active;
    }

    return broadcast_not_active;
}

/*!
    @brief Determine if a sync train is currently active.
*/
broadcast_status BroadcastIsSyncTrainActive(BROADCAST* broadcast)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_BROADCASTER_ROLE(broadcast);

    if (broadcast->config.broadcaster.current_state == broadcastTxStateCsbSyncTrain)
        return broadcast_sync_train_is_active;

    return broadcast_sync_train_not_active;
}

/*!
    @brief Get the current configuration of the library in the broadcast role.
*/
broadcast_status BroadcastBroadcasterGetConfiguration(BROADCAST* broadcast,
                                          broadcaster_csb_params* csb_params,
                                          broadcaster_sync_params* sync_params)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_BROADCASTER_ROLE(broadcast);

    *csb_params = BROADCASTER_CSB_PARAMS(broadcast);
    *sync_params = BROADCASTER_SYNC_PARAMS(broadcast);

    return broadcast_success;
}

/*!
    @brief Get the current configuration of the library in the receiver role.
*/
broadcast_status BroadcastReceiverGetConfiguration(BROADCAST* broadcast,
                                       receiver_csb_params* csb_params,
                                       receiver_sync_params* sync_params)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_RECEIVER_ROLE(broadcast);

    *csb_params = RECEIVER_CSB_PARAMS(broadcast);
    *sync_params = RECEIVER_SYNC_PARAMS(broadcast);

    return broadcast_success;
}

/*!
    @brief Get the CSB interval of the library in the receiver role.
*/
broadcast_status BroadcastReceiverGetCSBInterval(BROADCAST *broadcast,
                                                 uint16 *interval)
{
    VALIDATE_INSTANCE(broadcast);
    VALIDATE_RECEIVER_ROLE(broadcast);
    *interval = RECEIVER_CSB_PARAMS(broadcast).interval;
    return broadcast_success;
}



/*!
    @brief Get the configured LT_ADDR
*/
uint8 BroadcastGetLtAddr(BROADCAST* broadcast)
{
    if(broadcast)
    {
        return broadcast->lt_addr;
    }
    return LT_ADDR_DEFAULT;
}



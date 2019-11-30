/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast_private.h
  @brief Internal interface within the Broadcast library.
*/

#ifndef BROADCAST_PRIVATE_H_
#define BROADCAST_PRIVATE_H_

#include "broadcast.h"
#include "broadcast_tx.h"
#include "broadcast_rx.h"
#include "gatt_broadcast_server.h"
#include "gatt_broadcast_client.h"

/****************************************************************************
 * Defines
 ****************************************************************************/
/* control debug generation */
#ifdef BROADCAST_DEBUG_LIB
#include <stdio.h>
#define BROADCAST_DEBUG(x)  printf x
#else
#define BROADCAST_DEBUG(x)
#endif

/*! Not a valid broadcast LT address. */
#define LT_ADDR_INVALID             0

/*! Base for the broadcast library internal messages, 0x0 by convention. */
#define BROADCAST_INTERNAL_MSG_BASE  (0x0)

/*! Macros to identify currently defined role */
#define IS_BROADCASTER_ROLE(x)      (((x) != NULL) && ((x)->role == broadcast_role_broadcaster))
#define IS_RECEIVER_ROLE(x)         (((x) != NULL) && ((x)->role == broadcast_role_receiver))

/*! Macros to access broadcaster configuration */
#define BROADCASTER_CSB_PARAMS(x)   ((x)->config.broadcaster.csb_params)
#define BROADCASTER_SYNC_PARAMS(x)  ((x)->config.broadcaster.sync_params)
#define BROADCASTER_STATE(x)        ((x)->config.broadcaster.current_state)

/*! Macros to access receiver configuration */
#define RECEIVER_CSB_PARAMS(x)      ((x)->config.receiver.csb_params)
#define RECEIVER_SYNC_PARAMS(x)     ((x)->config.receiver.sync_params)
#define RECEIVER_STATE(x)           ((x)->config.receiver.current_state)
#define RECEIVER_ASSOC_DATA(x)      ((x)->config.receiver.assoc_data)

/****************************************************************************
 * Typedefs, Structs and Enumerations
 ****************************************************************************/
/*! Core component of the library that always persists across instance
 * create/destroy operations.
 */
typedef struct
{
    /*! Task for the core component */
    TaskData    core_task;
    /*! Linked list of broadcast library instances. */
    BROADCAST*  lib_instances_head;
} broadcast_lib_core;

/*! Global reference to the Broadcast library core. */
extern broadcast_lib_core* lib_core;

/*! Debug structure with names of internal events for clearer debug. */
extern const char* const internal_event_names[];

/*! Core state of a Broadcast library instance. */
struct __BROADCAST
{
    /*! Task for this instance of the broadcast library */
    TaskData        lib_task;

    /*! Application to which messages for this instance will be sent */
    Task            client_task;

    /*! Link to next instance of the library in the lib_core->lib_instances_head
     * list */
    BROADCAST*      next_instance;

    /*! Broadcast role this instance is configured for */
    broadcast_role  role;

    /*! Broadcast address of this instance */
    ltaddr lt_addr;

    /*! Lock used to conditionally post messages through transitional states
     * and prevent client message arriving in transition states. */
    uint16 lock;

    /*! A library instance will be either a broadcaster or a receiver */
    union
    {
        struct
        {
            /*! Current TX FSM state */
            broadcast_tx_state current_state;

            /*! Mode in which #BroadcastStartBroadcast was called */
            broadcast_mode start_mode;
            /*! Mode in which #BroadcastStopBroadcast was called */
            broadcast_mode stop_mode;

            /*! Flag set to send start sync train confirm message to client */
            bool sync_train_confirm;

            /*! Broadcaster CSB parameters. */
            broadcaster_csb_params csb_params;
            /*! Broadcaster Sync Train parameters. */
            broadcaster_sync_params sync_params;
        } broadcaster;
        struct
        {
            /*! Current RX FSM state */
            broadcast_rx_state current_state;

            /*! Mode in which #BroadcastStartReceiver was called */
            broadcast_mode start_mode;

            /*! Association data received from server */
            broadcast_assoc_data assoc_data;

            /*! Receiver CSB parameters. */
            receiver_csb_params csb_params;
            
            /*! Receiver Sync Train params. */
            receiver_sync_params sync_params;
        } receiver;
    } config;
};

/*! Internal message types, used to decouple external interface from hamdling. */
typedef enum
{
    BROADCAST_INTERNAL_DESTROY = BROADCAST_INTERNAL_MSG_BASE,
    BROADCAST_INTERNAL_CONFIG_BROADCASTER,
    BROADCAST_INTERNAL_START_BROADCAST,
    BROADCAST_INTERNAL_STOP_BROADCAST,
    BROADCAST_INTERNAL_CONFIG_RECEIVER,
    BROADCAST_INTERNAL_CONFIG_RECEIVER_OOB,
    BROADCAST_INTERNAL_START_RECEIVER,
    BROADCAST_INTERNAL_STOP_RECEIVER,
    BROADCAST_INTERNAL_TX_TIMEOUT_RES,
    BROADCAST_INTERNAL_RX_TIMEOUT_RES,

    BROADCAST_INTERNAL_MSG_TOP
} BROADCAST_INTERNAL_MSG;

/* Internal message to destroy a library instance. */
typedef struct
{
    BROADCAST* broadcast;
} BROADCAST_INTERNAL_DESTROY_T;

/* Internal message to configure broadcaster. */
typedef struct
{
    BROADCAST* broadcast;
    ltaddr addr;
    broadcaster_csb_params* csb_config;
    broadcaster_sync_params* sync_config;
} BROADCAST_INTERNAL_CONFIG_BROADCASTER_T;

/* Internal message to start a broadcast.  */
typedef struct
{
    BROADCAST* broadcast;
    broadcast_mode mode;
    bool sync_train_confirm;
} BROADCAST_INTERNAL_START_BROADCAST_T;

/* Internal message to stop a broadcast. */
typedef struct
{
    BROADCAST* broadcast;
    broadcast_mode mode;
} BROADCAST_INTERNAL_STOP_BROADCAST_T;

/* Internal message to configure a receiver. */
typedef struct
{
    BROADCAST* broadcast;
    receiver_sync_params* sync_config;
} BROADCAST_INTERNAL_CONFIG_RECEIVER_T;

/* Internal message to configure a receiver OOB. */
typedef struct
{
    BROADCAST* broadcast;
    bdaddr bt_addr;
    ltaddr lt_addr;
    receiver_csb_params* csb_config;
} BROADCAST_INTERNAL_CONFIG_RECEIVER_OOB_T;

/* Internal message to start a receiver. */
typedef struct
{
    BROADCAST* broadcast;
    broadcast_mode mode;
    uint16 timeout;
} BROADCAST_INTERNAL_START_RECEIVER_T;

/* Internal message to stop a receiver. */
typedef struct
{
    BROADCAST* broadcast;
} BROADCAST_INTERNAL_STOP_RECEIVER_T;

/* Internal message to respond to broadcaster timeout. */
typedef struct
{
    BROADCAST* broadcast;
} BROADCAST_INTERNAL_TX_TIMEOUT_RES_T;

/* Internal message to respond to receiver timeout. */
typedef struct
{
    BROADCAST* broadcast;
} BROADCAST_INTERNAL_RX_TIMEOUT_RES_T;

/*! @brief Find the BROADCAST instance for a given library task.

    @param task [IN] The task structure of a broadcast library instance.

    @return BROADCAST* Pointer to the Broadcast instance for the task.
*/
BROADCAST* broadcast_identify_instance(Task task);

/*! @brief Validate that a BROADCAST library instance reference is valid
 
    @param broadcast [IN] Pointer to an instance of the broadcast library.

    @return bool TRUE if the reference is valid, FALSE if not.
 */
bool broadcast_is_valid_instance(BROADCAST* broadcast);

/*! @brief Free the memory associated with a broadcast library instance.
 
    @param broadcast [IN] Pointer to an instance of the broadcast library.
 */
void broadcast_destroy_lib(BROADCAST* broadcast);

#endif /* BROADCAST_PRIVATE_H_ */

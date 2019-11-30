/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast_rx.c
  @brief Implementation of the Receiver state machine.
*/

#include "broadcast_private.h"
#include "broadcast_rx.h"
#include "broadcast_msg_client.h"

#include <message.h>
#include <panic.h>
#include <stream.h>
#include <source.h>
#include <connection.h>
#include <stdlib.h>

/*! Clearer definitions of the lock that holds up messages until
 * we've passed through transition states */
#define LOCKED      1
#define UNLOCKED    0

#define CSB_RX_REMOTE_TIMING_ACCURACY  200
#define CSB_RX_SKIP                    0
#define CSB_RX_PACKET_TYPE             8

/* Accesor for CSB stream source. */
#define CSB_STREAM_SOURCE(broadcast) StreamCsbSource(&RECEIVER_SYNC_PARAMS(broadcast).bd_addr, broadcast->lt_addr)

#ifdef BROADCAST_DEBUG_LIB
const char* const rx_state_names[] =
{
    "Init",
    "UnusedTransitionState",
    "Idle",
    "SyncTrainDiscovery",
    "SyncTrain",
    "StartingCsbRx",
    "SyncTrainCsbRx",
    "StoppingCsbRx",
    "UnusedState",
    "SyncTrainDiscoveryCsbRx",
    "UnusedState",
    "SyncTrainDiscoveryStopping"
};
#endif
#ifdef BROADCAST_DEBUG_LIB
const char* const internal_event_names[] =
{
    "LIB_DESTROY",
    "CONFIG_BROADCASTER",
    "START_BROADCAST",
    "STOP_BROADCAST",
    "CONFIG_RECEIVER",
    "CONFIG_RECEIVER_OOB",
    "START_RECEIVER",
    "STOP_RECEIVER",
    "TX_TIMEOUT_RES",
    "RX_TIMEOUT_RES",
    "CONFIG_ASSOC",
    "START_ASSOC",
    "STOP_ASSOC",
    "START_ASSOC_RECV",
    "STOP_ASSOC_RECV"
};
#endif

/*! @brief Broadcast Rx state machine transition handler. */
static void broadcastRxSetState(BROADCAST* broadcast, broadcast_rx_state new_state);

/****************************************************************************
 * Utility functions
 ****************************************************************************/
/*! @brief ensure all sync parameters are valid. */
static bool validate_rx_sync_params(receiver_sync_params* sync_params)
{
    /* must have some parameters... */
    if (!sync_params)
        return FALSE;

    /* Range: 0x0000 or 0x0022-0xFFFE; only even values are valid. */
    if (   ((sync_params->sync_scan_timeout > 0) && (sync_params->sync_scan_timeout < 0x0022))
        || (sync_params->sync_scan_timeout & 0x0001))
        return FALSE;
    
    /* Range: 0x0022-0xFFFE; only even values are valid. */
    if ((sync_params->sync_scan_window < 0x0022) ||
        (sync_params->sync_scan_window & 0x0001))
        return FALSE;

    /* Range: 0x0002-0xFFFE; only even values are valid. */
    if ((sync_params->sync_scan_interval == 0x0000) ||
        (sync_params->sync_scan_interval & 0x0001))
        return FALSE;

    return TRUE;
}

/*! @brief Copy the sync train scan results into a #receiver_csb_params structure. */
static void broadcastCopySyncTrainScanResults(receiver_csb_params *csb_params,
                                              CL_CSB_SYNC_TRAIN_RECEIVED_CFM_T* cfm)
{
    csb_params->interval = cfm->interval;
    csb_params->clock_offset = cfm->clock_offset;
    csb_params->next_csb_clock = cfm->next_csb_clock;
    /* map is 10 bytes stored unpacked in uint16[10] */
    memcpy(csb_params->afh_map, cfm->afh_map, sizeof(csb_params->afh_map));
    /* not provided in DM or CL message, use defaults */
    csb_params->remote_timing_accuracy = CSB_RX_REMOTE_TIMING_ACCURACY;
    csb_params->skip = CSB_RX_SKIP;
    csb_params->packet_type = CSB_RX_PACKET_TYPE;
}

/****************************************************************************
 * State Machine State Handler Functions
 ****************************************************************************/
/*! @brief Broadcast Rx FSM broadcastRxStateInit handler.
 */
static void broadcastRxStateInitHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case BROADCAST_INTERNAL_CONFIG_RECEIVER:
        {
            BROADCAST_INTERNAL_CONFIG_RECEIVER_T* bicr = (BROADCAST_INTERNAL_CONFIG_RECEIVER_T*)message;

            if (validate_rx_sync_params(bicr->sync_config))
            {
                RECEIVER_SYNC_PARAMS(broadcast) = *(bicr)->sync_config;
                broadcast_msg_client_config_cfm(broadcast, broadcast_success);
                broadcastRxSetState(broadcast, broadcastRxStateIdle);
            }
            else
            {
                broadcast_msg_client_config_cfm(broadcast, broadcast_bad_param);
            }
        }
        break;
        case BROADCAST_INTERNAL_CONFIG_RECEIVER_OOB:
        {
            /* deprecated API */
        }
        break;
        case BROADCAST_INTERNAL_DESTROY:
        {
            /* destroy is valid here, no firmware state to release, so just
             * go straight ahead and destroy the library, no point changing
             * states we'll be gone shortly. */
            broadcast_destroy_lib(broadcast);
        }
        break;
        case BROADCAST_INTERNAL_STOP_RECEIVER:
        {
            /* we're not running, nothing to stop, tell the client */
            broadcast_msg_client_stop_receiver_cfm(broadcast, broadcast_not_active);
        }
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                            id, RECEIVER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Rx FSM broadcastRxStateIdle handler.
 */
static void broadcastRxStateIdleHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case BROADCAST_INTERNAL_START_RECEIVER:
        {
            BROADCAST_INTERNAL_START_RECEIVER_T* bisr = (BROADCAST_INTERNAL_START_RECEIVER_T*)message;

            /* remember our start mode for later messaging to client */
            broadcast->config.receiver.start_mode = bisr->mode;

            if (bisr->mode == broadcast_mode_auto)
            {
                /* auto start, so send ourselves a message to automatically
                 * start receiving the CSB after we find the sync train */
                MESSAGE_MAKE(delayed, BROADCAST_INTERNAL_START_RECEIVER_T);
                delayed->broadcast = broadcast;
                delayed->mode = broadcast_mode_auto;
                delayed->timeout = bisr->timeout;
                broadcast->lock = LOCKED;
                MessageSendConditionally(&broadcast->lib_task,
                                         BROADCAST_INTERNAL_START_RECEIVER,
                                         delayed, &broadcast->lock);
            }

            /* discover the sync train */
            ConnectionCsbReceiveSyncTrain(&RECEIVER_SYNC_PARAMS(broadcast).bd_addr,
                                          RECEIVER_SYNC_PARAMS(broadcast).sync_scan_timeout,
                                          RECEIVER_SYNC_PARAMS(broadcast).sync_scan_window,
                                          RECEIVER_SYNC_PARAMS(broadcast).sync_scan_interval);

            broadcastRxSetState(broadcast, broadcastRxStateSyncTrainDiscovery);
        }
        break;
        case BROADCAST_INTERNAL_CONFIG_RECEIVER:
        {
            BROADCAST_INTERNAL_CONFIG_RECEIVER_T* bicr = (BROADCAST_INTERNAL_CONFIG_RECEIVER_T*)message;

            if (bicr->sync_config)
            {
                RECEIVER_SYNC_PARAMS(broadcast) = *(bicr)->sync_config;
                broadcast_msg_client_config_cfm(broadcast, broadcast_success);
            }
            else
            {
                broadcast_msg_client_config_cfm(broadcast, broadcast_bad_param);
            }
        }
        break;
        case BROADCAST_INTERNAL_CONFIG_RECEIVER_OOB:
        {
            /* deprecated API */
        }
        break;
        case BROADCAST_INTERNAL_DESTROY:
        {
            /* destroy is valid here, no firmware state to release, so just
             * go straight ahead and destroy the library, no point changing
             * states we'll be gone shortly. */
            broadcast_destroy_lib(broadcast);
        }
        break;
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                            id, RECEIVER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Rx FSM broadcastRxStateSyncTrainDiscovery handler.
 */
static void broadcastRxStateSyncTrainDiscoveryHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case CL_CSB_SYNC_TRAIN_RECEIVED_CFM:
        {
            CL_CSB_SYNC_TRAIN_RECEIVED_CFM_T* cfm = (CL_CSB_SYNC_TRAIN_RECEIVED_CFM_T*)message;

            BROADCAST_DEBUG(("BL: CL_CSB_SYNC_TRAIN_RECEIVED_CFM status:%x ltaddr:%x\n", cfm->status, cfm->lt_addr));

            if (cfm->status == hci_success)
            {
                /* Remember the LT_ADDR of the broadcaster */
                broadcast->lt_addr = cfm->lt_addr;
                
                if (broadcast->config.receiver.start_mode == broadcast_mode_auto)
                {
                    /* remember the discovered csb params */
                    broadcastCopySyncTrainScanResults(&RECEIVER_CSB_PARAMS(broadcast), cfm);
                    /* go to "sync train known" state ready for the BROADCAST_INTERNAL_START_RECEIVER 
                     * message to arrive once we leave this transition state */
                    broadcastRxSetState(broadcast, broadcastRxStateSyncTrain);
                }

                /* if start mode was sync train only tell the client and we're done
                 * and go back to idle */
                if (broadcast->config.receiver.start_mode == broadcast_mode_synctrain_only)
                {
                    receiver_csb_params csb_params;
                    broadcastCopySyncTrainScanResults(&csb_params, cfm);
                    broadcast_msg_client_start_receiver_cfm(broadcast, broadcast_success,
                                                            broadcast_mode_synctrain_only,
                                                            (Source)NULL, &csb_params);
                    broadcastRxSetState(broadcast, broadcastRxStateIdle);
                }
            }
            else
            {
                /* failed to find the sync train, cancel any pending autostart of
                 * broadcast receive, tell the client, go back to idle */
                MessageCancelFirst(&broadcast->lib_task, BROADCAST_INTERNAL_START_RECEIVER);

                broadcast_msg_client_start_receiver_cfm(broadcast, broadcast_sync_train_receive_failed,
                                                        broadcast->config.receiver.start_mode,
                                                        (Source)NULL, NULL);
                broadcastRxSetState(broadcast, broadcastRxStateIdle);
            }
        }
        break;
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                            id, RECEIVER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Rx FSM broadcastRxStateSyncTrain handler.
 */
static void broadcastRxStateSyncTrainHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case BROADCAST_INTERNAL_START_RECEIVER:
        {
            BROADCAST_INTERNAL_START_RECEIVER_T* bisr = (BROADCAST_INTERNAL_START_RECEIVER_T*)message;
            
            /* remember the start mode and the CSB receive supervision timeout */
            broadcast->config.receiver.start_mode = bisr->mode;
            RECEIVER_CSB_PARAMS(broadcast).supervision_timeout = bisr->timeout;

            /* try and receive the CSB */
            ConnectionCsbReceive(TRUE, &RECEIVER_SYNC_PARAMS(broadcast).bd_addr,
                                       broadcast->lt_addr,
                                       RECEIVER_CSB_PARAMS(broadcast).interval,
                                       RECEIVER_CSB_PARAMS(broadcast).clock_offset,
                                       RECEIVER_CSB_PARAMS(broadcast).next_csb_clock,
                                       RECEIVER_CSB_PARAMS(broadcast).supervision_timeout,
                                       RECEIVER_CSB_PARAMS(broadcast).remote_timing_accuracy,
                                       RECEIVER_CSB_PARAMS(broadcast).skip,
                                       RECEIVER_CSB_PARAMS(broadcast).packet_type,
                                       RECEIVER_CSB_PARAMS(broadcast).afh_map);

            /* go to StartingCsbRx state waiting for response to the request to start
             * receiving the CSB */
            broadcastRxSetState(broadcast, broadcastRxStateStartingCsbRx);
        }
        break;
        case BROADCAST_INTERNAL_STOP_RECEIVER:
        {
            /* received a stop request before the sync train discovery completed, it was sat in
             * the message queue waiting until we exited broadcastRxStateSyncTrainDiscovery
             * state, cancel any pending auto start message, inform client and return to idle */
            MessageCancelFirst(&broadcast->lib_task, BROADCAST_INTERNAL_START_RECEIVER);
            broadcast_msg_client_stop_receiver_cfm(broadcast, broadcast_success);
            broadcastRxSetState(broadcast, broadcastRxStateIdle);
        }
        break;
        case BROADCAST_INTERNAL_DESTROY:
        {
            /* destroy is not valid here, broadcast must be stopped first */
            broadcast_msg_client_destroy_cfm(broadcast->client_task, broadcast_destroy_failed_still_active);
        }
        break;
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                            id, RECEIVER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Rx FSM broadcastRxStateStartingCsbRx handler.
 */
static void broadcastRxStateStartingCsbRxHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case CL_CSB_RECEIVE_CFM:
        {
            CL_CSB_RECEIVE_CFM_T* cfm = (CL_CSB_RECEIVE_CFM_T*)message;
            
            BROADCAST_DEBUG(("BL: CL_CSB_RECEIVE_CFM status:%x\n", cfm->status));

            if (cfm->status == hci_success)
            {
                /* get the CSB source */
                Source src = CSB_STREAM_SOURCE(broadcast);

                /* dispose any data while the source is passed to the application */
                StreamConnectDispose(src);

                /* tell client success, returning the stream source */
                broadcast_msg_client_start_receiver_cfm(broadcast, broadcast_success,
                                                        broadcast->config.receiver.start_mode,
                                                        src, &(RECEIVER_CSB_PARAMS(broadcast)));
                broadcastRxSetState(broadcast, broadcastRxStateSyncTrainCsbRx);
            }
            else
            {
                broadcast_msg_client_start_receiver_cfm(broadcast, broadcast_data_broadcast_receive_failed,
                                                        broadcast->config.receiver.start_mode,
                                                        (Source)NULL, NULL);
                broadcastRxSetState(broadcast, broadcastRxStateIdle);
            }
        }
        break;
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                            id, RECEIVER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Rx FSM broadcastRxStateStoppingCsbRx handler.
 */
static void broadcastRxStateStoppingCsbRxHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case CL_CSB_RECEIVE_CFM:
        {
            CL_CSB_RECEIVE_CFM_T* cfm = (CL_CSB_RECEIVE_CFM_T*)message;

            BROADCAST_DEBUG(("BL: CL_CSB_RECEIVE_CFM status:%x\n", cfm->status));

            if (cfm->status == hci_success)
            {
                broadcastRxSetState(broadcast, broadcastRxStateIdle);
                broadcast_msg_client_stop_receiver_cfm(broadcast, broadcast_success);
            }
            /* can't fail to stop receiving a broadcast */
        }
        break;
        case BROADCAST_INTERNAL_RX_TIMEOUT_RES:
        {
            /* application sent the response to the timeout, tell the connection
             * library and transition back to idle so we're ready to be started
             * again */
            ConnectionCsbReceiveTimeoutResponse(&RECEIVER_SYNC_PARAMS(broadcast).bd_addr,
                                                broadcast->lt_addr);
            broadcastRxSetState(broadcast, broadcastRxStateIdle);
        }
        break;
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                            id, RECEIVER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Rx FSM broadcastRxStateSyncTrainCsbRx handler.
 */
static void broadcastRxStateSyncTrainCsbRxHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    UNUSED(message);

    switch (id)
    {
        case BROADCAST_INTERNAL_STOP_RECEIVER:
        {
            /* stop receiving the broadcast, go to transition state to wait
             * for confirmation before informing the client */
            ConnectionCsbReceive(FALSE, &RECEIVER_SYNC_PARAMS(broadcast).bd_addr,
                                        broadcast->lt_addr,
                                        RECEIVER_CSB_PARAMS(broadcast).interval,
                                        RECEIVER_CSB_PARAMS(broadcast).clock_offset,
                                        RECEIVER_CSB_PARAMS(broadcast).next_csb_clock,
                                        RECEIVER_CSB_PARAMS(broadcast).supervision_timeout,
                                        RECEIVER_CSB_PARAMS(broadcast).remote_timing_accuracy,
                                        RECEIVER_CSB_PARAMS(broadcast).skip,
                                        RECEIVER_CSB_PARAMS(broadcast).packet_type,
                                        RECEIVER_CSB_PARAMS(broadcast).afh_map);
            broadcastRxSetState(broadcast, broadcastRxStateStoppingCsbRx);
        }
        break;
        case CL_CSB_RECEIVE_TIMEOUT_IND:
        {
            BROADCAST_DEBUG(("BL: CL_CSB_RECEIVE_TIMEOUT_IND\n"));

            broadcast_msg_client_stop_receiver_ind(broadcast);
            broadcastRxSetState(broadcast, broadcastRxStateStoppingCsbRx);
        }
        break;
        case BROADCAST_INTERNAL_DESTROY:
        {
            /* destroy is not valid here, broadcast must be stopped first */
            broadcast_msg_client_destroy_cfm(broadcast->client_task, broadcast_destroy_failed_still_active);
        }
        break;
        case BROADCAST_INTERNAL_START_RECEIVER:
        {
            BROADCAST_INTERNAL_START_RECEIVER_T* bisr = (BROADCAST_INTERNAL_START_RECEIVER_T*)message;
            if (bisr->mode == broadcast_mode_synctrain_only)
            {
                /* remember our start mode for later messaging to client */
                broadcast->config.receiver.start_mode = broadcast_mode_synctrain_only;

                /* don't save the CSB receive timeout, we won't be changing the
                 * current CSB receive parameters */

                /* rediscover the sync train and go to state awaiting sync train
                 * discovery while still receiving CSB */
                ConnectionCsbReceiveSyncTrain(&RECEIVER_SYNC_PARAMS(broadcast).bd_addr,
                                              RECEIVER_SYNC_PARAMS(broadcast).sync_scan_timeout,
                                              RECEIVER_SYNC_PARAMS(broadcast).sync_scan_window,
                                              RECEIVER_SYNC_PARAMS(broadcast).sync_scan_interval);
                broadcastRxSetState(broadcast, broadcastRxStateSyncTrainDiscoveryCsbRx);
            }
            else
            {
                /* whilst receiving CSB we only support another start request to
                 * discover a sync train */
                broadcast_msg_client_start_receiver_cfm(broadcast, broadcast_bad_param,
                                                        bisr->mode,
                                                        (Source)NULL, NULL);
            }
        }
        break;
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                            id, RECEIVER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Rx FSM broadcastRxStateSyncTrainDiscoveryCsbRx handler.
 
    The receiver is receiving a broadcast, but has been requested by the client
    to search for a sync train. The receiver is waiting for the results of the 
    sync train discovery.
 */
static void broadcastRxStateSyncTrainDiscoveryCsbRxHandler(BROADCAST* broadcast,
                                                           MessageId id,
                                                           Message message)
{
    switch (id)
    {
        /* received results of sync train scan */
        case CL_CSB_SYNC_TRAIN_RECEIVED_CFM:
        {
            CL_CSB_SYNC_TRAIN_RECEIVED_CFM_T* cfm = (CL_CSB_SYNC_TRAIN_RECEIVED_CFM_T*)message;

            BROADCAST_DEBUG(("BL: CL_CSB_SYNC_TRAIN_RECEIVED_CFM status:%x ltaddr:%x\n", cfm->status, cfm->lt_addr));

            if (cfm->status == hci_success)
            {
                /* don't remember the CSB params as we're already receiving a CSB,
                 * just return them to the client */
                receiver_csb_params csb_params;
                broadcastCopySyncTrainScanResults(&csb_params, cfm);
                broadcast_msg_client_start_receiver_cfm(broadcast, broadcast_success,
                                                        broadcast_mode_synctrain_only,
                                                        (Source)NULL, &csb_params);
            }
            else
            {
                /* failed to find the sync train, tell the client */
                broadcast_msg_client_start_receiver_cfm(broadcast, broadcast_sync_train_receive_failed,
                                                        broadcast_mode_synctrain_only,
                                                        (Source)NULL, NULL);
            }
            /* return to CSB RX state */
            broadcastRxSetState(broadcast, broadcastRxStateSyncTrainCsbRx);
        }
        break;
        
        case CL_CSB_RECEIVE_TIMEOUT_IND:
        {
            /* the broadcast being received has timed out, while we're waiting for
             * a sync train discovery, tell client application and go to state waiting
             * on both sync train discovery and client app response to the timeout */
            BROADCAST_DEBUG(("BL: CL_CSB_RECEIVE_TIMEOUT_IND\n"));
            broadcast_msg_client_stop_receiver_ind(broadcast);
            broadcastRxSetState(broadcast, broadcastRxStateSyncTrainDiscoveryStopping);
        }
        break;

        case BROADCAST_INTERNAL_DESTROY:
        {
            /* destroy is not valid here, broadcast must be stopped first */
            broadcast_msg_client_destroy_cfm(broadcast->client_task, broadcast_destroy_failed_still_active);
        }
        break;

        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                            id, RECEIVER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Rx FSM broadcastRxStateSyncTrainDiscoveryStopping handler.
 */
static void broadcastRxStateSyncTrainDiscoveryStoppingHandler(BROADCAST* broadcast,
                                                              MessageId id,
                                                              Message message)
{
    switch (id)
    {
        case CL_CSB_SYNC_TRAIN_RECEIVED_CFM:
        {
            CL_CSB_SYNC_TRAIN_RECEIVED_CFM_T* cfm = (CL_CSB_SYNC_TRAIN_RECEIVED_CFM_T*)message;

            BROADCAST_DEBUG(("BL: CL_CSB_SYNC_TRAIN_RECEIVED_CFM status:%x ltaddr:%x\n", cfm->status, cfm->lt_addr));

            if (cfm->status == hci_success)
            {
                /* don't remember the CSB params as we're already receiving a CSB,
                 * just return them to the client */
                receiver_csb_params csb_params;
                broadcastCopySyncTrainScanResults(&csb_params, cfm);
                broadcast_msg_client_start_receiver_cfm(broadcast, broadcast_success,
                                                        broadcast_mode_synctrain_only,
                                                        (Source)NULL, &csb_params);
            }
            else
            {
                /* failed to find the sync train, tell the client */
                broadcast_msg_client_start_receiver_cfm(broadcast, broadcast_sync_train_receive_failed,
                                                        broadcast_mode_synctrain_only,
                                                        (Source)NULL, NULL);
            }
            /* we've had the sync train details, so go to the stopping state
             * and handle the completion of the stop in the usual manner */
            broadcastRxSetState(broadcast, broadcastRxStateStoppingCsbRx);
        }
        break;

        case BROADCAST_INTERNAL_RX_TIMEOUT_RES:
        {
            /* application responded to the timeout, inform the connection library
             * to complete the timeout.
             * we're still waiting on the sync train scan, so go to the normal
             * sync train discovery state, now that we're no longer receiving CSB */
            ConnectionCsbReceiveTimeoutResponse(&RECEIVER_SYNC_PARAMS(broadcast).bd_addr,
                                                broadcast->lt_addr);
            broadcastRxSetState(broadcast, broadcastRxStateSyncTrainDiscovery);
        }
        break;

        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                            id, RECEIVER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Rx FSM Event handler.
    
    Switch on the current state and pass events in to the respective handler.
 */
void broadcastRxHandleEvent(BROADCAST* broadcast, MessageId id, Message message)
{
#ifdef  BROADCAST_DEBUG_LIB
    if ((id >= BROADCAST_INTERNAL_MSG_BASE) && (id < BROADCAST_INTERNAL_MSG_TOP))
        BROADCAST_DEBUG(("BL: RX Event (%s) State(%s)\n",
                                internal_event_names[id-BROADCAST_INTERNAL_MSG_BASE],
                                rx_state_names[RECEIVER_STATE(broadcast)]));
    else
        BROADCAST_DEBUG(("BL: RX Event (0x%x) State(%s)\n", id,
                                rx_state_names[RECEIVER_STATE(broadcast)]));
#endif

    switch (RECEIVER_STATE(broadcast))
    {
        case broadcastRxStateInit:
            broadcastRxStateInitHandler(broadcast, id, message);
            break;
        case broadcastRxStateIdle:
            broadcastRxStateIdleHandler(broadcast, id, message);
            break;
        case broadcastRxStateSyncTrainDiscovery:
            broadcastRxStateSyncTrainDiscoveryHandler(broadcast, id, message);
            break;
        case broadcastRxStateSyncTrain:
            broadcastRxStateSyncTrainHandler(broadcast, id, message);
            break;
        case broadcastRxStateStartingCsbRx:
            broadcastRxStateStartingCsbRxHandler(broadcast, id, message);
            break;
        case broadcastRxStateSyncTrainCsbRx:
            broadcastRxStateSyncTrainCsbRxHandler(broadcast, id, message);
            break;
        case broadcastRxStateStoppingCsbRx:
            broadcastRxStateStoppingCsbRxHandler(broadcast, id, message);
            break;
        case broadcastRxStateSyncTrainDiscoveryCsbRx:
            broadcastRxStateSyncTrainDiscoveryCsbRxHandler(broadcast, id, message);
            break;
        case broadcastRxStateSyncTrainDiscoveryStopping:
            broadcastRxStateSyncTrainDiscoveryStoppingHandler(broadcast, id, message);
            break;
        default:
            BROADCAST_DEBUG(("BL: TX Event unknown state (0x%x)\n",
                            RECEIVER_STATE(broadcast)));
            break;
    }
}

/*! @brief Broadcast Rx state machine transition handler. */
static void broadcastRxSetState(BROADCAST* broadcast, broadcast_rx_state new_state)
{
    BROADCAST_DEBUG(("BL: Rx State Transition %s -> %s\n",
                            rx_state_names[RECEIVER_STATE(broadcast)], rx_state_names[new_state]));

    /* if we're about to enter a transition state, set the lock */
    if (new_state & 0x01)
        broadcast->lock = LOCKED;
        
    /* change state */
    RECEIVER_STATE(broadcast) = new_state;

    /* if we've entered a non-transition state, clear the lock */
    if (!(RECEIVER_STATE(broadcast) & 0x01))
        broadcast->lock = UNLOCKED;
}


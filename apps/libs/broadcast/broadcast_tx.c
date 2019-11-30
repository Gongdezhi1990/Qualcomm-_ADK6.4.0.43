/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast_tx.c
  @brief Implementation of the Broadcaster state machine.
*/

#include "broadcast_private.h"
#include "broadcast_tx.h"
#include "broadcast_msg_client.h"

#include <message.h>
#include <panic.h>
#include <stream.h>
#include <sink.h>
#include <connection.h>
#include <stdlib.h>
#include <csb.h>

/*! Default sync train timeout of 30s in slots, used when the client
    configures a continuous sync train */
#define DEFAULT_STO 0xBB80

/*! Clearer definitions of the lock that holds up messages until
 * we've passed through transition states */
#define LOCKED      1
#define UNLOCKED    0

/* Accessor for CSB stream sink. */
#define CSB_STREAM_SINK(broadcast) StreamCsbSink(broadcast->lt_addr)

#ifdef BROADCAST_DEBUG_LIB
const char* const tx_state_names[] = 
{
    "Init",
    "Configuring",
    "Idle",
    "StartingCsb",
    "Csb",
    "StoppingCsb",
    "CsbSyncTrain",
    "CsbStartingSyncTrain",
    "",
    "DestroyingInstance"
};
#endif

/*! @brief Broadcast Tx state machine transition handler. */
static void broadcastTxSetState(BROADCAST* broadcast, broadcast_tx_state new_state);
/*! @brief Utility function to send a delayed BROADCAST_INTERNAL_START_BROADCAST message. */
static void broadcastTxSendDelayedStart(BROADCAST* broadcast, broadcast_mode mode, bool confirm);
/*! @brief Utility function to start and stop CSB. */
static void broadcastTxControlCsb(BROADCAST* broadcast, bool enable);

/*! @brief ensure all csb parameters are valid. */
static bool validate_tx_csb_params(broadcaster_csb_params* csb_params)
{
    /* there must be some parameters... */
    if (!csb_params)
        return FALSE;

    /* interval_min must be <= interval_max */
    if (csb_params->interval_min > csb_params->interval_max)
        return FALSE;

    /* Range: 0x0002-0xFFFE; only even values are valid. */
    if ((csb_params->interval_min == 0x0000) ||
        (csb_params->interval_min & 0x0001))
        return FALSE;

    /* Range: 0x0002-0xFFFE; only even values are valid. */
    if ((csb_params->interval_max == 0x0000) ||
        (csb_params->interval_max & 0x0001))
        return FALSE;

    /* Range: 0x0002-0xFFFE; only even values are valid. */
    if ((csb_params->supervision_timeout == 0x0000) ||
        (csb_params->supervision_timeout & 0x0001))
        return FALSE;

    /* got here, all valid */
    return TRUE;
}

/*! @brief ensure all sync parameters are valid. */
static bool validate_tx_sync_params(broadcaster_sync_params* sync_params)
{
    if (!sync_params)
        return FALSE;

    /* interval_min must be <= interval_max */
    if (sync_params->interval_min > sync_params->interval_max)
        return FALSE;

    /* Range: 0x0020-0xFFFE; only even values are valid. */
    if ((sync_params->interval_min < 0x0020) ||
        (sync_params->interval_min & 0x0001))
        return FALSE;

    /* Range: 0x0020-0xFFFE; only even values are valid. */
    if ((sync_params->interval_max < 0x0020) ||
        (sync_params->interval_max & 0x0001))
        return FALSE;
    
    /* Range: 0x00000000 or 0x00000002-07FFFFFE; only even values are valid. */
    if (sync_params->sync_train_timeout & 0x00000001)
        return FALSE;

    return TRUE;
}

/****************************************************************************
 * State Machine State Handler Functions
 ****************************************************************************/
/*! @brief Broadcast Tx FSM broadcastTxStateInit handler.
 */
static void broadcastTxStateInitHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case BROADCAST_INTERNAL_CONFIG_BROADCASTER:
        {
            BROADCAST_INTERNAL_CONFIG_BROADCASTER_T* bicb = (BROADCAST_INTERNAL_CONFIG_BROADCASTER_T*)message;

            /* only handle configuration if provided, applications may 
             * reconfigure later and only supply CSB or Sync parameters,
             * setting NULL for params to remain unchanged */
            if (bicb->csb_config)
            {
                /* if we do have params, ensure they're sane */
                if (validate_tx_csb_params(bicb->csb_config))
                {
                    BROADCASTER_CSB_PARAMS(broadcast) = *(bicb->csb_config);
                }
                else
                {
                    broadcast_msg_client_config_cfm(broadcast, broadcast_bad_param);
                    return;
                }
            }
            if (bicb->sync_config)
            {
                if (validate_tx_sync_params(bicb->sync_config))
                {
                    BROADCASTER_SYNC_PARAMS(broadcast) = *(bicb->sync_config);
                }
                else
                {
                    broadcast_msg_client_config_cfm(broadcast, broadcast_bad_param);
                    return;
                }
            }

            /* reserve the LTADDR, can only happen if we're
             * in the init state */
            ConnectionCsbSetReservedLtAddr(bicb->addr);

            /* Also keep update the lt_addr in the broadcaster instance */
            broadcast->lt_addr = bicb->addr;

            /* waiting on completion of the configuration now */
            broadcastTxSetState(broadcast, broadcastTxStateConfiguring);
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
                            id, BROADCASTER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Tx FSM broadcastTxStateConfiguring handler.
 */
static void broadcastTxStateConfiguringHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case CL_CSB_SET_RESERVED_LT_ADDR_CFM:
        {
            CL_CSB_SET_RESERVED_LT_ADDR_CFM_T* cfm = (CL_CSB_SET_RESERVED_LT_ADDR_CFM_T*)message;

            if (cfm->status == hci_success)
            {
                /* LT_ADDR registered ok, remember it and move to Idle, ready to
                 * start */
                broadcast->lt_addr = cfm->lt_addr;
                broadcastTxSetState(broadcast, broadcastTxStateIdle);
            }
            else
            {
                /* failed to register the LT_ADDR, go back to init */
                broadcastTxSetState(broadcast, broadcastTxStateInit);
            }

            /* tell client what happened */
            broadcast_msg_client_config_cfm(broadcast,
                            cfm->status == hci_success ? 
                            broadcast_success : broadcast_invalid_lt_addr);
        }
        break;
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                        id, BROADCASTER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Tx FSM broadcastTxStateIdle handler.
 */
static void broadcastTxStateIdleHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case BROADCAST_INTERNAL_START_BROADCAST:
        {
            BROADCAST_INTERNAL_START_BROADCAST_T* bisb = (BROADCAST_INTERNAL_START_BROADCAST_T*)message;

            if (bisb->mode == broadcast_mode_synctrain_only)
            {
                broadcast_msg_client_start_broadcast_cfm(broadcast, broadcast_bad_param,
                                                         broadcast_mode_synctrain_only, (Sink)NULL);
            }
            else
            {
                /* remember the mode we've been started in */
                broadcast->config.broadcaster.start_mode = bisb->mode;
            
                /* if the mode was auto, then post ourselves a message to
                 * automatically start the sync train if the CSB start
                 * succeeds */
                if (bisb->mode == broadcast_mode_auto)
                {
                    broadcastTxSendDelayedStart(broadcast, broadcast_mode_auto, bisb->sync_train_confirm);
                }

                /* actually try and start the broadcast */
                broadcastTxControlCsb(broadcast, TRUE);

                broadcastTxSetState(broadcast, broadcastTxStateStartingCsb);
            }
        }
        break;
        
        case BROADCAST_INTERNAL_CONFIG_BROADCASTER:
        {
            BROADCAST_INTERNAL_CONFIG_BROADCASTER_T* bicb = (BROADCAST_INTERNAL_CONFIG_BROADCASTER_T*)message;

            /* we're idle so client can configure both the CSB and sync train
             * parameters, copy each if they're provided. */
            if (bicb->csb_config)
            {
                BROADCASTER_CSB_PARAMS(broadcast) = *(bicb->csb_config);
            }
            if (bicb->sync_config)
            {
                BROADCASTER_SYNC_PARAMS(broadcast) = *(bicb->sync_config);
            }

            /* report success and stay in this state */
            broadcast_msg_client_config_cfm(broadcast, broadcast_success); 
        }
        break;
        
        case BROADCAST_INTERNAL_DESTROY:
        {
            /* destroy is valid here
                - delete the reserved LTADDR
                - transition to the destroying state to wait for confirmation
             */
            ConnectionCsbDeleteReservedLtAddr(broadcast->lt_addr);
            broadcastTxSetState(broadcast, broadcastTxStateDestroyingInstance);
        }
        break;

        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                        id, BROADCASTER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Tx FSM broadcastTxStateStartingCsb handler.
 */
static void broadcastTxStateStartingCsbHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case CL_CSB_SET_CSB_CFM:
        {
            CL_CSB_SET_CSB_CFM_T* cfm = (CL_CSB_SET_CSB_CFM_T*)message;
            /* successful start or stop of the broadcast */
            if (cfm->status == hci_success)
            {
                /* if we were only starting the CSB and not auto starting the 
                 * sync train then we're done, tell client and move to CSB
                 * active state */
                if (broadcast->config.broadcaster.start_mode == broadcast_mode_broadcast_only)
                {
                    broadcast_msg_client_start_broadcast_cfm(broadcast, broadcast_success,
                            broadcast_mode_broadcast_only, CSB_STREAM_SINK(broadcast));
                }
                /* we're now broadcasting */
                broadcastTxSetState(broadcast, broadcastTxStateCsb);
            }
            else
            {
                /* clear any pending autostart of the sync train */
                MessageCancelFirst(&broadcast->lib_task, BROADCAST_INTERNAL_START_BROADCAST);

                /* failed to start CSB, tell client */
                broadcast_msg_client_start_broadcast_cfm(broadcast,
                        broadcast_data_broadcast_start_failed,
                        broadcast->config.broadcaster.start_mode, (Sink)NULL);

                /* go back to idle */
                broadcastTxSetState(broadcast, broadcastTxStateIdle);
            }
        }
        break;
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                        id, BROADCASTER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Tx FSM broadcastTxStateCsb handler.
 */
static void broadcastTxStateCsbHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        /* this is either a manual sync train start, or an auto start message
         * we posted ourselves */
        case BROADCAST_INTERNAL_START_BROADCAST:
        {
            BROADCAST_INTERNAL_START_BROADCAST_T* bisb = (BROADCAST_INTERNAL_START_BROADCAST_T*)message;
            
            if (bisb->mode == broadcast_mode_synctrain_only ||
                bisb->mode == broadcast_mode_auto)
            {
                /* remember the mode for later client messaging */
                broadcast->config.broadcaster.start_mode = bisb->mode;
                
                /* store confirm flag so we know if a sync train start confirm message
                   needs to be sent when sync train started */
                broadcast->config.broadcaster.sync_train_confirm = bisb->sync_train_confirm;
                
                /* Configure the sync train.
                 * If the timeout is 0, then set our default value and auto
                 * restart when we get notified it completed, else use the 
                 * configured timeout */
                ConnectionCsbWriteSyncTrainParameters(
                        BROADCASTER_SYNC_PARAMS(broadcast).interval_min,
                        BROADCASTER_SYNC_PARAMS(broadcast).interval_max,
                        BROADCASTER_SYNC_PARAMS(broadcast).sync_train_timeout == 0 ?
                            DEFAULT_STO : BROADCASTER_SYNC_PARAMS(broadcast).sync_train_timeout,
                        BROADCASTER_SYNC_PARAMS(broadcast).service_data);

                broadcastTxSetState(broadcast, broadcastTxStateCsbStartingSyncTrain);
            }
            else
            {
                /* requested a broadcast only start, which we're already doing,
                 * just return success, with the current still valid csb sink */
                broadcast_msg_client_start_broadcast_cfm(broadcast, broadcast_success,
                                                         bisb->mode,
                                                         CSB_STREAM_SINK(broadcast));
            }
        }
        break;
        case BROADCAST_INTERNAL_STOP_BROADCAST:
        {
            BROADCAST_INTERNAL_STOP_BROADCAST_T* bisb = (BROADCAST_INTERNAL_STOP_BROADCAST_T*)message;

            /* synctrain_only doesn't make sense, we don't have it running,
             * so return an error. Auto and broadcast_only we'll treat the
             * same and just stop */
            if (bisb->mode == broadcast_mode_synctrain_only)
            {
                broadcast_msg_client_stop_broadcast_cfm(broadcast, bisb->mode,
                                                        broadcast_bad_param);
            }
            else
            {
                /* remember stop mode for confirmation messages to client */
                broadcast->config.broadcaster.stop_mode = bisb->mode;

                /* turn off broadcast and go to CSB transition state to
                 * await confirmation */
                broadcastTxControlCsb(broadcast, FALSE);
                broadcastTxSetState(broadcast, broadcastTxStateStoppingCsb);
            }
        }
        break;
        case BROADCAST_INTERNAL_CONFIG_BROADCASTER:
        {
            BROADCAST_INTERNAL_CONFIG_BROADCASTER_T* bicb = (BROADCAST_INTERNAL_CONFIG_BROADCASTER_T*)message;

            /* we have a broadcast running, so cannot change the CSB params */
            if (bicb->csb_config)
            {
                broadcast_msg_client_config_cfm(broadcast, broadcast_configure_failed_active);
                return;
            }
            /* but we can accept updates to the sync train params */
            if (bicb->sync_config)
            {
                BROADCASTER_SYNC_PARAMS(broadcast) = *(bicb->sync_config);
                broadcast_msg_client_config_cfm(broadcast, broadcast_success); 
            }
            /* stay in this state */
        }
        break;
        case CL_CSB_TRANSMIT_TIMEOUT_IND:
        {
            /* inform application about the timeout */
            broadcast_msg_client_stop_broadcast_ind(broadcast);

            /* go to the stopping state and await BroadcastBroadcasterTimeoutResponse() */
            broadcastTxSetState(broadcast, broadcastTxStateStoppingCsb);
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
                        id, BROADCASTER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Tx FSM broadcastTxStateStoppingCsb handler.
 */
static void broadcastTxStateStoppingCsbHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case CL_CSB_SET_CSB_CFM:
        {
            CL_CSB_SET_CSB_CFM_T* cfm = (CL_CSB_SET_CSB_CFM_T*)message;

            /* successful stop of the broadcast */
            if (cfm->status == hci_success)
            {
                /* tell the client and go to idle */
                broadcast_msg_client_stop_broadcast_cfm(broadcast, 
                                                        broadcast->config.broadcaster.stop_mode,
                                                        broadcast_success);
                broadcastTxSetState(broadcast, broadcastTxStateIdle);
            }
            /* stopping CSB can not fail, so no failure handler here */
        }
        break;

        case CL_CSB_START_SYNC_TRAIN_CFM:
            /* must be auto stopping from broadcastTxStateCsbStartingSyncTrain
             * state, just ignore this, we'll get the CL_CSB_SET_CSB_CFM
             * shortly */
        break;

        case BROADCAST_INTERNAL_TX_TIMEOUT_RES:
        {
            /* applicatiom acknowledged the transmit timeout, tell the
             * connection library and go back to idle */
            ConnectionCsbTransmitTimeoutResponse(broadcast->lt_addr);
            broadcastTxSetState(broadcast, broadcastTxStateIdle);
        }
        break;

        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                        id, BROADCASTER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Tx FSM broadcastTxStateCsbStartingSyncTrain handler.
 */
static void broadcastTxStateCsbStartingSyncTrainHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case CL_CSB_WRITE_SYNC_TRAIN_PARAMETERS_CFM:
        {
            CL_CSB_WRITE_SYNC_TRAIN_PARAMETERS_CFM_T* cfm = (CL_CSB_WRITE_SYNC_TRAIN_PARAMETERS_CFM_T*)message;
            if (cfm->status == hci_success)
            {
                /* start the sync train */
                ConnectionCsbStartSyncTrain();

                /* tell the client if requested */
                if (broadcast->config.broadcaster.sync_train_confirm)
                {
                    broadcast->config.broadcaster.sync_train_confirm = FALSE;
                    broadcast_msg_client_start_broadcast_cfm(broadcast, broadcast_success,
                                                             broadcast->config.broadcaster.start_mode,
                                                             CSB_STREAM_SINK(broadcast));
                }

                /* we're now broadcasting and running the sync train */
                broadcastTxSetState(broadcast, broadcastTxStateCsbSyncTrain);
            }
            else
            {
                if (broadcast->config.broadcaster.start_mode == broadcast_mode_auto)
                {
                    /* we were auto starting everything and failed, so go 
                     * back to idle */
                    broadcastTxControlCsb(broadcast, FALSE);

                    broadcast_msg_client_start_broadcast_cfm(broadcast,
                                                             broadcast_sync_train_start_failed,
                                                             broadcast->config.broadcaster.start_mode,
                                                             (Sink)NULL);

                    broadcastTxSetState(broadcast, broadcastTxStateIdle);
                }
                else
                {
                    /* just a sync train start failure, go back to just 
                     * broadcasting, Sink is still valid. */
                    broadcast_msg_client_start_broadcast_cfm(broadcast, broadcast_sync_train_start_failed,
                                            broadcast->config.broadcaster.start_mode, CSB_STREAM_SINK(broadcast));

                    broadcastTxSetState(broadcast, broadcastTxStateCsb);
                }
            }
        }
        break;
        
        case CL_CSB_TRANSMIT_TIMEOUT_IND:
        {
            /* inform application about the timeout */
            broadcast_msg_client_stop_broadcast_ind(broadcast);

            /* go to the stopping state and await BroadcastBroadcasterTimeoutResponse() */
            broadcastTxSetState(broadcast, broadcastTxStateStoppingCsb);
        }
        break;
#if 0
        case CL_CSB_SYNC_TRAIN_COMPLETE_IND:
        {
            if (broadcast->config.broadcaster.stop_mode == broadcast_mode_synctrain_only)
            {
                broadcast_msg_client_stop_broadcast_cfm(broadcast, 
                                                        broadcast->config.broadcaster.stop_mode,
                                                        broadcast_success);

                broadcastTxSetState(broadcast, broadcastTxStateCsb);
            }
        }
        break;
#endif
        default:
        {
            BROADCAST_DEBUG(("BL: Unhandled event (0x%x) state (0x%x)\n",
                        id, BROADCASTER_STATE(broadcast)));
        }
        break;
    }
}

/*! @brief Broadcast Tx FSM broadcastTxStateCsbSyncTrain handler.
 */
static void broadcastTxStateCsbSyncTrainHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case CL_CSB_START_SYNC_TRAIN_CFM:
        {
            CL_CSB_START_SYNC_TRAIN_CFM_T *cfm = (CL_CSB_START_SYNC_TRAIN_CFM_T *)message;
            if (cfm->status == hci_success) 
            {
                /* are we configured for continuous sync train? */
                if (BROADCASTER_SYNC_PARAMS(broadcast).sync_train_timeout == 0)
                {
                    /* post ourselves a message to start the sync train and
                    * go back to the CSB-only state, we'll reuse the sync
                    * train setup code path, which will take account of any
                    * changes in the sync train parameters the client may
                    * have made */
                    broadcastTxSendDelayedStart(broadcast, broadcast_mode_synctrain_only, FALSE);
                }
                else
                {
                    /* notify client the sync train has stopped and move back to
                    * CSB-only state */
                    broadcast_msg_client_status_ind(broadcast, broadcast_sync_train_tx_ended);
                }
            } 
            else
            {
                /* should never happen, just tell client that sync train has stopped */
                broadcast_msg_client_status_ind(broadcast, broadcast_sync_train_tx_ended);
            }
            
            /* go back to CSB-only state, if sync train is going to be restarted, we'll handle it in
               that state */ 
            broadcastTxSetState(broadcast, broadcastTxStateCsb);
        }
        break;

        case BROADCAST_INTERNAL_STOP_BROADCAST:
        {
            BROADCAST_INTERNAL_STOP_BROADCAST_T* bisb = (BROADCAST_INTERNAL_STOP_BROADCAST_T*)message;
        
            /* remember the stop mode */
            broadcast->config.broadcaster.stop_mode = bisb->mode;

            /* if the mode is sync_train_only we go back into the sync train
             * transition state and wait for it to complete */
            if (bisb->mode == broadcast_mode_synctrain_only)
            {
                broadcastTxSetState(broadcast, broadcastTxStateCsbStartingSyncTrain);
            }
            else
            {
                /* if the stop mode is auto (or broadcast_only) we don't want
                 * to wait for the sync train to complete, so we stop the CSB,
                 * and the sync train gets cancelled too. */
                broadcastTxControlCsb(broadcast, FALSE);
                broadcastTxSetState(broadcast, broadcastTxStateStoppingCsb);
            }
        }
        break;
        case BROADCAST_INTERNAL_CONFIG_BROADCASTER:
        {
            BROADCAST_INTERNAL_CONFIG_BROADCASTER_T* bicb = (BROADCAST_INTERNAL_CONFIG_BROADCASTER_T*)message;

            /* we have a broadcast running, so cannot change the CSB params */
            if (bicb->csb_config)
            {
                broadcast_msg_client_config_cfm(broadcast, broadcast_configure_failed_active); 
                return;
            }
            /* but we can accept updates to the sync train params */
            if (bicb->sync_config)
            {
                BROADCASTER_SYNC_PARAMS(broadcast) = *(bicb->sync_config);
                broadcast_msg_client_config_cfm(broadcast, broadcast_success); 
            }
            /* stay in this state */
        }
        break;
        case CL_CSB_TRANSMIT_TIMEOUT_IND:
        {
            /* inform application about the timeout */
            broadcast_msg_client_stop_broadcast_ind(broadcast);

            /* go to the stopping state and await BroadcastBroadcasterTimeoutResponse() */
            broadcastTxSetState(broadcast, broadcastTxStateStoppingCsb);
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
                        id, BROADCASTER_STATE(broadcast)));
        }
        break;
    }
}

static void broadcastTxStateDestroyingInstanceHandler(BROADCAST* broadcast, MessageId id, Message message)
{
    switch (id)
    {
        case CL_CSB_DELETE_RESERVED_LT_ADDR_CFM:
        {
            CL_CSB_DELETE_RESERVED_LT_ADDR_CFM_T* cfm = (CL_CSB_DELETE_RESERVED_LT_ADDR_CFM_T*)message;

            BROADCAST_DEBUG(("BL: CL_CSB_DELETE_RESERVED_LT_ADDR_CFM:%x\n", cfm->status));
            
            if (cfm->status != hci_success)
            {
                /* shouldn't happen, but we need to know if it does */
                Panic();
            }
            else
            {
                /* actually destroy the library, no point changing states
                 * we'll be gone shortly. We're in a transition state anyway
                 * and incoming messages will be locked out */
                broadcast_destroy_lib(broadcast);
            }
        }
        break;
        default:
        break;
    }

}

/*! @brief Handle new AFH map update.  */
static void broadcastTxHandleAfhMap(BROADCAST* broadcast, CL_CSB_AFH_MAP_IND_T *ind)
{
    BROADCAST_DEBUG(("BL: AFH New channel map available\n"));
    broadcast_msg_client_afh_update_ind(broadcast, ind->map, ind->clock);
}

/*! brief Handle AFH map applued indication. */
static void broadcastTxHandleCsbChanneMapChange(BROADCAST* broadcast, CL_CSB_CHANNEL_MAP_CHANGE_IND_T *ind)
{
    BROADCAST_DEBUG(("BL: AFH Channel map applied\n"));
    broadcast_msg_client_afh_channel_map_changed_ind(broadcast, ind->map);
}

/*! @brief Broadcast Tx FSM Event handler.
    
    Switch on the current state and pass events in to the respective handler.
 */
void broadcastTxHandleEvent(BROADCAST* broadcast, MessageId id, Message message)
{
#ifdef BROADCAST_DEBUG_LIB
    if ((id >= BROADCAST_INTERNAL_MSG_BASE) && (id < BROADCAST_INTERNAL_MSG_TOP))
    {
        BROADCAST_DEBUG(("BL: TX FSM Event (%s) State(%s)\n",
                                    internal_event_names[id-BROADCAST_INTERNAL_MSG_BASE],
                                    tx_state_names[BROADCASTER_STATE(broadcast)]));
    }
    else
    {
        BROADCAST_DEBUG(("BL: TX FSM Event (0x%x) State(%s)\n", id,
                                    tx_state_names[BROADCASTER_STATE(broadcast)]));
    }
#endif

    /* handling of AFH update messages */
    if (id == CL_CSB_AFH_MAP_IND)
    {
        broadcastTxHandleAfhMap(broadcast, (CL_CSB_AFH_MAP_IND_T *)message);
        return;
    }
    else if (id == CL_CSB_CHANNEL_MAP_CHANGE_IND)
    {
        broadcastTxHandleCsbChanneMapChange(broadcast, (CL_CSB_CHANNEL_MAP_CHANGE_IND_T*)message);
        return;
    }

    /* everything else is an input to the TX state machine */
    switch (BROADCASTER_STATE(broadcast))
    {
        case broadcastTxStateInit:
            broadcastTxStateInitHandler(broadcast, id, message);
            break;
        case broadcastTxStateConfiguring:
            broadcastTxStateConfiguringHandler(broadcast, id, message);
            break;
        case broadcastTxStateIdle:
            broadcastTxStateIdleHandler(broadcast, id, message);
            break;
        case broadcastTxStateStartingCsb:
            broadcastTxStateStartingCsbHandler(broadcast, id, message);
            break;
        case broadcastTxStateCsb:
            broadcastTxStateCsbHandler(broadcast, id, message);
            break;
        case broadcastTxStateStoppingCsb:
            broadcastTxStateStoppingCsbHandler(broadcast, id, message);
            break;
        case broadcastTxStateCsbStartingSyncTrain:
            broadcastTxStateCsbStartingSyncTrainHandler(broadcast, id, message);
            break;
        case broadcastTxStateCsbSyncTrain:
            broadcastTxStateCsbSyncTrainHandler(broadcast, id, message);
            break;
        case broadcastTxStateDestroyingInstance:
            broadcastTxStateDestroyingInstanceHandler(broadcast, id, message);
            break;
        default:
            BROADCAST_DEBUG(("BL: TX Event unknown state (0x%x)\n",
                            BROADCASTER_STATE(broadcast)));
            break;
    }
}

/*! @brief Broadcast Tx state machine transition handler. */
static void broadcastTxSetState(BROADCAST* broadcast, broadcast_tx_state new_state)
{
    BROADCAST_DEBUG(("BL: Tx State Transition %s -> %s\n",
                        tx_state_names[BROADCASTER_STATE(broadcast)],
                        tx_state_names[new_state]));

    /* if we're about to enter a transition state, set the lock */
    if (new_state & 0x01)
        broadcast->lock = LOCKED;
        
    /* change state */
    BROADCASTER_STATE(broadcast) = new_state;

    /* if we've entered a non-transition state, clear the lock */
    if (!(BROADCASTER_STATE(broadcast) & 0x01))
        broadcast->lock = UNLOCKED;
}


/*! @brief Utility function to send a delayed BROADCAST_INTERNAL_START_BROADCAST message. */
static void broadcastTxSendDelayedStart(BROADCAST* broadcast, broadcast_mode mode, bool sync_train_confirm)
{
    MESSAGE_MAKE(delayed, BROADCAST_INTERNAL_START_BROADCAST_T);
    delayed->broadcast = broadcast;
    delayed->mode = mode;
    delayed->sync_train_confirm = sync_train_confirm;
    broadcast->lock = LOCKED;
    MessageSendConditionally(&broadcast->lib_task,
                             BROADCAST_INTERNAL_START_BROADCAST,
                             delayed, &broadcast->lock);
}

/*! @brief Utility function to start and stop CSB. */
static void broadcastTxControlCsb(BROADCAST* broadcast, bool enable)
{
    ConnectionCsbSet(enable, broadcast->lt_addr,
            BROADCASTER_CSB_PARAMS(broadcast).lpo_allowed,
            BROADCASTER_CSB_PARAMS(broadcast).packet_type,
            BROADCASTER_CSB_PARAMS(broadcast).interval_min,
            BROADCASTER_CSB_PARAMS(broadcast).interval_max,
            BROADCASTER_CSB_PARAMS(broadcast).supervision_timeout);
}


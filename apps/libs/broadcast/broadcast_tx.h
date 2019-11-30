/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast_tx.h
  @brief Interface to the Broadcaster state machine.
*/

#ifndef BROADCAST_TX_H_
#define BROADCAST_TX_H_

#include "broadcast_private.h"

/*! Definition of broadcaster states.
 
  NOTE - transition states MUST have odd values, as the state machine
  event handler uses this to identify transition states and set/clear
  a message lock to control message delivery into the state machine.
  See #broadcastTxSetState().
 */
typedef enum
{
    /*! The broadcaster is initialised. */
    broadcastTxStateInit                    = 0x00,

    /*! The broadcaster is (re)configuring. Transition State. */
    broadcastTxStateConfiguring             = 0x01,

    /*! The broadcaster has no CSB or Sync Train active. */
    broadcastTxStateIdle                    = 0x02,

    /*! The broadcaster is starting a broadcast. Transition State. */
    broadcastTxStateStartingCsb             = 0x03,

    /*! The broadcaster has an active broadcast. */
    broadcastTxStateCsb                     = 0x04,

    /*! The broadcaster is stopping a broadcast. Transition State. */
    broadcastTxStateStoppingCsb             = 0x05,

    /*! The broadcaster has an active broadcast and sync train. */
    broadcastTxStateCsbSyncTrain            = 0x06,

    /*! The broadcaster has an active broadcast and is starting the sync train.
        Transition State. */
    broadcastTxStateCsbStartingSyncTrain    = 0x07,

    /*! An instance of the library in the broadcast role is being
        destroyed. Transition State. */
    broadcastTxStateDestroyingInstance      = 0x09

} broadcast_tx_state;

/*! @brief Broadcast Tx state machine event handler.

    Handler for events being passed into the broadcast tx state machine.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param id [IN] Event message type.
    @param message [IN] Event message contents.
*/
void broadcastTxHandleEvent(BROADCAST* broadcast, MessageId id, Message message);

#endif /* BROADCAST_TX_H_ */

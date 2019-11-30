/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast_rx.h
  @brief Interface to the Receiver state machine.
*/

#ifndef BROADCAST_RX_H_
#define BROADCAST_RX_H_

#include "broadcast_private.h"

/*! Definition of receiver states.
 
  NOTE - transition states MUST have odd values, as the state machine
  event handler uses this to identify transition states and set/clear
  a message lock to control message delivery into the state machine.
  See #broadcastRxSetState().
 */
typedef enum
{
    /*! The receiver is initialised. */
    broadcastRxStateInit                    = 0x00,

    /*! The receiver has no knowledge of sync train and is not
        receiving a broadcast. */
    broadcastRxStateIdle                    = 0x02,

    /*! The receiver is trying to find a sync train. Transition State. */
    broadcastRxStateSyncTrainDiscovery      = 0x03,

    /*! The receiver has knowledge of a broadcast (likely from a sync train). */
    broadcastRxStateSyncTrain               = 0x04,

    /*! The receiver is starting to receive a broadcast. Transition State. */
    broadcastRxStateStartingCsbRx           = 0x05,

    /*! The receiver has found a sync train and is receiving a broadcast. */
    broadcastRxStateSyncTrainCsbRx          = 0x06,

    /*! The receiver is stopping a broadcast. Transition State. */
    broadcastRxStateStoppingCsbRx           = 0x07,

    /*! The receiver is looking for a sync train, and receiving a broadcast. 
        Transition State. */
    broadcastRxStateSyncTrainDiscoveryCsbRx = 0x09,

    /*! The receiver is waiting for an in progress sync train discovery to finish,
     *  after the CSB RX stopped. Transition State. */
    broadcastRxStateSyncTrainDiscoveryStopping = 0x0b

} broadcast_rx_state;

/*! @brief Broadcast Rx state machine event handler.

    Handler for events being passed into the broadcast rx state machine.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param id [IN] Event message type.
    @param message [IN] Event message contents.
*/
void broadcastRxHandleEvent(BROADCAST* broadcast, MessageId id, Message message);

#endif /* BROADCAST_RX_H_ */

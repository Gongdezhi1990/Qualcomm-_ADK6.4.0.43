/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_ba_broadcaster_packetiser.h
 
DESCRIPTION
    Interface for controlling the broadcast packetiser and erasure coding for the application,
    as oppose to be controlled by the audio plugins.
    That enables sending of AFH map updates to the receivers regardless of the audio plugin states.
*/

#ifndef SINK_BA_BROADCASTER_PACKETISER_H_
#define SINK_BA_BROADCASTER_PACKETISER_H_

#include <message.h>

typedef enum
{
    /* The caller has nothing to do. */
    BROADCASTER_BP_HANDLED_INTERNALLY,

    /* The broadcast packetiser was successfully initialised,
       and the message was handled as far as the packetiser is concerned. */
    BROADCASTER_BP_INIT_SUCCESS,

    /* The broadcast packetiser initialisation failed. */
    BROADCASTER_BP_INIT_FAIL
} bp_msg_handler_result_t;

/***************************************************************************
DESCRIPTION
    Sets up and starts the erasure coding.
    Parameters are passed through BroadcastContext library.
*/
#if defined(ENABLE_BROADCAST_AUDIO) && defined(BROADCAST_PACKETISER_IS_CONTROLLED_BY_APP)
void sinkBroadcasterInitErasureCoding(void);
#else
#define sinkBroadcasterInitErasureCoding() ((void)0)
#endif

/***************************************************************************
DESCRIPTION
    Stops the erasure coding.
*/
#if defined(ENABLE_BROADCAST_AUDIO) && defined(BROADCAST_PACKETISER_IS_CONTROLLED_BY_APP)
void sinkBroadcasterDestroyErasureCoding(void);
#else
#define sinkBroadcasterDestroyErasureCoding() ((void)0)
#endif

/***************************************************************************
DESCRIPTION
    Sets up and starts the broadcast packetiser.
    Parameters are passed through BroadcastContext library.
*/
#if defined(ENABLE_BROADCAST_AUDIO) && defined(BROADCAST_PACKETISER_IS_CONTROLLED_BY_APP)
void sinkBroadcasterInitPacketiser(Task task);
#else
#define sinkBroadcasterInitPacketiser(task) ((void)0)
#endif

/***************************************************************************
DESCRIPTION
    Stops the broadcast packetiser.
*/
#if defined(ENABLE_BROADCAST_AUDIO) && defined(BROADCAST_PACKETISER_IS_CONTROLLED_BY_APP)
void sinkBroadcasterDestroyPacketiser(void);
#else
#define sinkBroadcasterDestroyPacketiser() ((void)0)
#endif

/***************************************************************************
DESCRIPTION
    Returns TRUE if a message was sent by the broadcast packetiser.
*/
#if defined(ENABLE_BROADCAST_AUDIO) && defined(BROADCAST_PACKETISER_IS_CONTROLLED_BY_APP)
bool sinkBroadcasterIsItPacketiserMessage(MessageId id);
#else
#define sinkBroadcasterIsItPacketiserMessage(id) (FALSE)
#endif

/***************************************************************************
DESCRIPTION
    Handle the broadcast packetiser messages.
    Due to separation of the packetiser code, some messages may be handled only partially.
    Caller is responsible for further handling of partially handled messages.
    Return code other than BROADCASTER_BP_HANDLED_INTERNALLY means that
    the message was handled only partially.
*/
#if defined(ENABLE_BROADCAST_AUDIO) && defined(BROADCAST_PACKETISER_IS_CONTROLLED_BY_APP)
bp_msg_handler_result_t sinkBroadcasterHandlePacketiserMessages(MessageId id, Message message);
#else
#define sinkBroadcasterHandlePacketiserMessages(id, message) (BROADCASTER_BP_HANDLED_INTERNALLY)
#endif

/***************************************************************************
DESCRIPTION
    Returns TRUE if the broadcast packetiser and erasure coding lifecycles
    should be controlled by the application.
*/
#if defined(ENABLE_BROADCAST_AUDIO) && defined(BROADCAST_PACKETISER_IS_CONTROLLED_BY_APP)
bool sinkBroadcasterIsPacketiserControlledByApp(void);
#else
#define sinkBroadcasterIsPacketiserControlledByApp() (FALSE)
#endif

#endif /* SINK_BA_BROADCASTER_PACKETISER_H_ */

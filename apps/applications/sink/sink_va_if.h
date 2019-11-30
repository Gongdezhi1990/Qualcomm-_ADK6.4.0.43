/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_va_if.h

DESCRIPTION
    Header file for the interface to the sink VA functionality

*/

/*!
@file   sink_va_if.h
@brief  Interface to the voice assistant functionality in the sink app

*/

#ifndef SINK_VA_IF_H_
#define SINK_VA_IF_H_

#include <csrtypes.h>
#include "sink_events.h"

#if (defined(ENABLE_AMA) || defined(ENABLE_BISTO))
#ifndef ENABLE_VOICE_ASSISTANT
  #define ENABLE_VOICE_ASSISTANT
#endif
#endif /* ENABLE_AMA || ENABLE_BISTO */


/*!
    @brief Initialize the Voice Assistant Application module

    @param None

    @return None
*/
#ifdef ENABLE_VOICE_ASSISTANT
void SinkVaInit(void);
#else
#define SinkVaInit() ((void)(0))
#endif


/*!
    @brief Calls for the cleaning up of transport state after Link Loss event. 

    @param None

    @return None
*/
#ifdef ENABLE_VOICE_ASSISTANT
void SinkVaHandleLinkLoss(void);
#else
#define SinkVaHandleLinkLoss() ((void)(0))
#endif


/*!
    @brief Triggers the event to start the button tap VA session

    @param None

    @return None
*/
#ifdef ENABLE_VOICE_ASSISTANT
void SinkVaTriggerTaptoTalk(void);
#else
#define SinkVaTriggerTaptoTalk() ((void)(0))
#endif

/*!
    @brief Triggers the event to start or stop the Push to talk VA session
    based on the VA session state.

    @param None

    @return None
*/
#ifdef ENABLE_VOICE_ASSISTANT
void SinkVaTriggerPushToTalkStart(void);
#else
#define SinkVaTriggerPushToTalkStart() ((void)(0))
#endif

/*!
    @brief Triggers the complete event for a VA session

    @param None

    @return None
*/
#ifdef ENABLE_VOICE_ASSISTANT
void SinkVaTriggerPushToTalkStop(void);
#else
#define SinkVaTriggerPushToTalkStop() ((void)(0))
#endif


/*!
    @brief Triggers the user event to Cancel the VA session

    @param None

    @return None
*/
#ifdef ENABLE_VOICE_ASSISTANT
void SinkVaUsrCancelSession(void);
#else
#define SinkVaUsrCancelSession() ((void)(0))
#endif

/*!
    @brief Abort the VA session when HFP incoming/outgoing call.

    @param None

    @return None
*/

#ifdef ENABLE_VOICE_ASSISTANT
void SinkVaCancelSession(void);
#else
#define SinkVaCancelSession() ((void)(0))
#endif

/*!
    @brief Abort the VA session when HFP incoming/outgoing call.

    @param None

    @return None
*/
#ifdef ENABLE_VOICE_ASSISTANT
void SinkVaResumeSession(void);
#else
#define SinkVaResumeSession() ((void)(0))
#endif /* ENABLE_VOICE_ASSISTANT */

#ifdef ENABLE_BISTO
/*
    Type definition for a VA callback function to handle sinkEvents_t.
 */
typedef bool (*sinkVaEventHandler_t)(sinkEvents_t sink_event);


/*!
    @brief Register a VA callback function to handle sinkEvents_t.

    @param callback Function to register

    @return None
*/
void sinkVaRegisterEventHandler(sinkVaEventHandler_t callback);

/*!
    @brief The VA function to handle sinkEvents_t.

    @param sink_event The sink event to be handled.

    @return bool TRUE if handledf, else FALSE.
*/
bool sinkVaEventHandler(sinkEvents_t sink_event);

#else   /* !ENABLE_BISTO */
#define sinkVaRegisterEventHandler(x) ((void)(x))
#define sinkVaEventHandler(x) ((void)(x), FALSE)
#endif  /* !ENABLE_BISTO */

#endif /* SINK_VA_IF_H_ */
